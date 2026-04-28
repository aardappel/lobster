/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifdef DECODER_VOC

// https://moddingwiki.shikadi.net/wiki/VOC_Format
// https://www.manualslib.com/manual/547219/Creative-Sb0350.html?page=125#manual

#include "SDL_mixer_internal.h"

#define VOC_TERM     0
#define VOC_DATA     1
#define VOC_CONT     2
#define VOC_SILENCE  3
#define VOC_MARKER   4
#define VOC_TEXT     5
#define VOC_LOOP     6
#define VOC_LOOPEND  7
#define VOC_EXTENDED 8
#define VOC_DATA_16  9

typedef struct VOC_Block
{
    int loop_count;  // 0=data or silence block, >0=loop block of X loops, -1=infinite loop block, -2=endloop block.
    Sint64 iopos;  // byte position in i/o stream of this block's data. Might be 0 for things like loop and silence blocks.
    SDL_AudioSpec spec;
    Uint64 frames;
} VOC_Block;

typedef struct VOC_AudioData
{
    VOC_Block *blocks;
    size_t num_blocks;
} VOC_AudioData;

typedef struct VOC_TrackData
{
    const VOC_AudioData *adata;
    SDL_IOStream *io;
    SDL_AudioSpec spec;
    size_t current_block;
    size_t frame_pos;
    const VOC_Block *loop_start;
    int loop_count;
} VOC_TrackData;


static VOC_Block *AddVocBlock(VOC_AudioData *adata)
{
    void *ptr = SDL_realloc(adata->blocks, (adata->num_blocks + 1) * sizeof (*adata->blocks));
    if (!ptr) {
        return NULL;
    }

    adata->blocks = (VOC_Block *) ptr;
    VOC_Block *block = &adata->blocks[adata->num_blocks];
    SDL_zerop(block);
    adata->num_blocks++;
    return block;
}

static VOC_Block *AddVocDataBlock(VOC_AudioData *adata, Sint64 iopos, const SDL_AudioSpec *spec, Uint64 frames)
{
    if (iopos < 0) {  // SDL_TellIO failed?
        return NULL;
    }

    VOC_Block *block = AddVocBlock(adata);
    if (block) {
        block->iopos = iopos;
        block->loop_count = 0;
        SDL_copyp(&block->spec, spec);
        block->frames = frames;
    }
    return block;
}

static VOC_Block *AddVocLoopBlock(VOC_AudioData *adata, int loop_count)
{
    VOC_Block *block = AddVocBlock(adata);
    if (block) {
        block->loop_count = loop_count;
    }
    return block;
}


// this runs during VOC_audio_init to walk the whole .VOC for metadata and sanity checks.
static bool ParseVocFile(SDL_IOStream *io, VOC_AudioData *adata, SDL_PropertiesID props, SDL_AudioSpec *spec, Sint64 *duration_frames)
{
    Sint64 total_frames = 0;
    VOC_Block *loop_start = NULL;
    int loop_start_loop_count = 0;
    Sint64 loop_frames = 0;
    SDL_AudioSpec original_spec;
    SDL_AudioSpec current_spec;
    int text_count = 0;

    Sint64 pos = SDL_TellIO(io);
    if (pos < 0) {
        return false;
    }

    SDL_copyp(&original_spec, spec);
    SDL_copyp(&current_spec, spec);
    spec->format = SDL_AUDIO_UNKNOWN;

    while (true) {
        Uint8 block;
        Uint32 blen = 0;
        if (SDL_ReadIO(io, &block, 1) != 1) {
            break;   // assume that's the end of the file.
        } else if (block == VOC_TERM) {
            break;  // that's the (optional) end.
        } else if (block != VOC_LOOPEND) {  // TERM and LOOPEND don't have a size field.
            Uint8 bits24[3];
            if (SDL_ReadIO(io, bits24, sizeof(bits24)) != sizeof(bits24)) {
                return false;
            }
            // Size is a 24-bit value. Ugh.
            blen = (Uint32)((bits24[0]) | (bits24[1] << 8) | (bits24[2] << 16));
        }

        pos++;
        if ((block != VOC_TERM) && (block != VOC_LOOPEND)) {
            pos += 3;  // size fields on everything but VOC_TERM and VOC_LOOPEND.
        }

        switch (block) {
            case VOC_DATA: {
                Uint8 codec, rateu8;
                if (SDL_ReadIO(io, &rateu8, 1) != 1) {
                    return false;
                } else if (SDL_ReadIO(io, &codec, 1) != 1) {
                    return false;
                }

                if ((codec != 0) && (codec != 4)) {  // !!! FIXME: there are other formats (adpcm, etc), but we don't support them at the moment.
                    return SDL_SetError("Unsupported VOC data format");
                }

                current_spec.freq = 1000000 / (256 - rateu8);
                current_spec.channels = 1;
                current_spec.format = (codec == 0) ? SDL_AUDIO_U8 : SDL_AUDIO_S16LE;

                const int framelen = SDL_AUDIO_FRAMESIZE(current_spec);
                const Uint64 frames = (Uint64) ((blen - 2) / framelen);
                if (!AddVocDataBlock(adata, SDL_TellIO(io), &current_spec, frames)) {
                    return false;
                }

                if (spec->format == SDL_AUDIO_UNKNOWN) {  // take the first thing as the "official" spec, but this can change later.
                    SDL_copyp(spec, &current_spec);
                }

                if (total_frames != -1) {
                    loop_frames += frames;
                }

                break;
            }

            case VOC_DATA_16: {
                Uint32 rate32;
                if (!SDL_ReadU32LE(io, &rate32)) {
                    return false;
                } else if (rate32 == 0) {
                    return SDL_SetError("VOC sample rate is zero?");
                }

                Uint8 bits, channels, codec;
                if (SDL_ReadIO(io, &bits, 1) != 1) {
                    return false;
                } else if ((bits != 8) && (bits != 16)) {
                    return SDL_SetError("Unsupported VOC data format");
                } else if (SDL_ReadIO(io, &channels, 1) != 1) {   // I assume you have mono or stereo, but we'll let you go wild with whatever.
                    return false;
                } else if (SDL_ReadIO(io, &codec, 1) != 1) {
                    return false;
                } else if ((codec != 0) && (codec != 4)) {  // !!! FIXME: there are other formats (adpcm, etc), but we don't support them at the moment.
                    return SDL_SetError("Unsupported VOC data format");
                } else if (((codec == 0) && (bits != 8)) || ((codec == 4) && (bits != 16))) {
                    return SDL_SetError("Corrupt VOC data");
                }

                current_spec.freq = (int) rate32;
                current_spec.channels = (int) channels;
                current_spec.format = (codec == 0) ? SDL_AUDIO_U8 : SDL_AUDIO_S16LE;

                const int framelen = SDL_AUDIO_FRAMESIZE(current_spec);
                const Uint64 frames = (Uint64) ((blen - 12) / framelen);
                if (!AddVocDataBlock(adata, SDL_TellIO(io), &current_spec, frames)) {
                    return false;
                }

                if (spec->format == SDL_AUDIO_UNKNOWN) {  // take the first thing as the "official" spec, but this can change later.
                    SDL_copyp(spec, &current_spec);
                }

                if (total_frames != -1) {
                    loop_frames += frames;
                }

                break;
            }

            case VOC_CONT: {
                if (spec->format == SDL_AUDIO_UNKNOWN) {
                    return SDL_SetError("VOC continuation block before a data type is set.");
                }

                const int framelen = SDL_AUDIO_FRAMESIZE(current_spec);
                const Uint64 frames = (Uint64) (blen / framelen);
                if (!AddVocDataBlock(adata, SDL_TellIO(io), &current_spec, frames)) {
                    return false;
                }

                if (total_frames != -1) {
                    loop_frames += frames;
                }
                break;
            }

            case VOC_SILENCE: {  // it's okay if we haven't set an initial format yet, we'll know it when we get there later.
                // technically we could try to add this to an existing silence block, but in practice these are rare, and duplicate blocks probably more so.
                Uint16 frames;
                if (!SDL_ReadU16LE(io, &frames)) {
                    return false;
                } else if (!AddVocDataBlock(adata, 0, &current_spec, frames + 1)) {
                    return false;
                }

                if (total_frames != -1) {
                    loop_frames += frames + 1;
                }
                break;
            }

            case VOC_LOOP: {
                Uint16 iterations = 0;

                // !!! FIXME: can LOOP/LOOPEND sections nest? https://moddingwiki.shikadi.net/wiki/VOC_Format suggests no, saying LOOPEND goes back to _most recent_ LOOP start.
                if (loop_start) {
                    return SDL_SetError("VOC has nested loop");
                } else if (!SDL_ReadU16LE(io, &iterations)) {
                    return false;
                }

                int loop_count = -1;
                if (iterations == 0xFFFF) {
                    total_frames = -1;   // it's infinite.
                } else {
                    loop_count = ((int) iterations) + 1;
                }

                if (total_frames != -1) {
                    total_frames += loop_frames;   // add in anything that's accumulated until now.
                }

                loop_frames = 0;

                loop_start = AddVocLoopBlock(adata, loop_count);
                if (!loop_start) {
                    return false;
                }
                loop_start_loop_count = loop_start->loop_count;
                break;
            }

            case VOC_LOOPEND: {
                if (!loop_start) {
                    return SDL_SetError("VOC has a LOOPEND without a matching LOOP");
                }

                VOC_Block *block = AddVocLoopBlock(adata, -2);
                if (!block) {
                    return false;
                }

                if (total_frames != -1) {
                    total_frames += loop_frames * loop_start_loop_count;
                }

                loop_start = NULL;
                loop_start_loop_count = 0;
                loop_frames = 0;
                break;
            }

            case VOC_EXTENDED: {
                Uint16 rateu16;
                Uint8 codec, channelsu8;
                if (!SDL_ReadU16LE(io, &rateu16)) {
                    return false;
                } else if (SDL_ReadIO(io, &codec, 1) != 1) {
                    return false;
                } else if ((codec != 0) && (codec != 4)) {  // !!! FIXME: there are other formats (adpcm, etc), but we don't support them at the moment.
                    return SDL_SetError("Unsupported VOC data format");
                } else if (SDL_ReadIO(io, &channelsu8, 1) != 1) {
                    return false;
                }

                const int channels = ((int) channelsu8) + 1;
                current_spec.freq = (256000000 / (channels * (65536 - rateu16)));
                current_spec.channels = channels;
                current_spec.format = (codec == 0) ? SDL_AUDIO_U8 : SDL_AUDIO_S16LE;

                if (spec->format == SDL_AUDIO_UNKNOWN) {
                    SDL_copyp(spec, &current_spec);
                }

                break;
            }

            case VOC_TEXT: {
                char *value = (char *) SDL_malloc(blen);
                if (value) {  // oh well if we ran out of memory.
                    if (SDL_ReadIO(io, value, blen) != blen) {
                        return false;
                    }
                    char *utf8 = SDL_iconv_string("UTF-8", "ISO-8859-1", value, blen);
                    if (utf8) {
                        char key[64];
                        SDL_snprintf(key, sizeof (key), "SDL_mixer.metadata.voc.text%d", text_count);
                        SDL_SetStringProperty(props, key, utf8);
                        SDL_free(utf8);
                        text_count++;
                    }
                    SDL_free(value);
                }
                break;
            }

            // just skip these.
            //case VOC_MARKER:
            default:
                break;
        }

        pos += blen;
        if (SDL_SeekIO(io, pos, SDL_IO_SEEK_SET) < 0) {
            return false;
        }
    }

    if (loop_start) {  // !!! FIXME: should we just treat EOF as the loop end in this case?
        return SDL_SetError("VOC has a LOOP without a matching LOOPEND");
    }

    if (spec->format == SDL_AUDIO_UNKNOWN) {  // theoretically this can happen if you only have VOC_SILENCE blocks. Set it to the original device format.
        SDL_copyp(spec, &original_spec);
    }

    if (total_frames != -1) {
        total_frames += loop_frames;
    }

    *duration_frames = total_frames;

    return true;
}

static bool SDLCALL VOC_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    // just load the bare minimum from the IOStream to verify it's a .VOC file.
    char magic[20];
    if (SDL_ReadIO(io, magic, 20) != 20) {
        return false;
    } else if (SDL_memcmp(magic, "Creative Voice File\032", 20) != 0) {
        return SDL_SetError("Not a VOC audio stream");
    }

    // now jump to the data and prepare to parse everything out.
    Uint16 datablockpos;
    if (!SDL_ReadU16LE(io, &datablockpos)) {
        return false;
    } else if (SDL_SeekIO(io, datablockpos, SDL_IO_SEEK_SET) < 0) {
        return false;
    }

    VOC_AudioData *adata = (VOC_AudioData *) SDL_calloc(1, sizeof(*adata));
    if (!adata) {
        return false;
    } else if (!ParseVocFile(io, adata, props, spec, duration_frames)) {
        SDL_free(adata->blocks);
        SDL_free(adata);
        return false;
    }

    *audio_userdata = adata;

    return true;
}

static bool SDLCALL VOC_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **userdata)
{
    VOC_TrackData *tdata = (VOC_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    const VOC_AudioData *adata = (const VOC_AudioData *) audio_userdata;

    tdata->adata = adata;
    tdata->io = io;
    SDL_copyp(&tdata->spec, spec);

    *userdata = tdata;
    return true;
}

static bool SDLCALL VOC_decode(void *userdata, SDL_AudioStream *stream)
{
    VOC_TrackData *tdata = (VOC_TrackData *) userdata;

    if (tdata->current_block >= tdata->adata->num_blocks) {
        return false;  // EOF.
    }

    const VOC_Block *block = &tdata->adata->blocks[tdata->current_block];

    if (tdata->frame_pos == 0) {  // starting a new block, see what we're doing...
        if (block->loop_count == 0) {  // it's data.
            if (block->iopos) {  // have to read actual file data from this block.
                if (SDL_SeekIO(tdata->io, block->iopos, SDL_IO_SEEK_SET) != block->iopos) {
                    return false;  // uhoh.
                }
            }
            if (SDL_memcmp(&tdata->spec, &block->spec, sizeof (tdata->spec)) != 0) {
                tdata->spec.format = SDL_AUDIO_UNKNOWN;  // we'll set it later.
            }
        } else if (block->loop_count == -2) {   // it's an endloop block.
            SDL_assert(tdata->loop_start != NULL);
            const size_t start_block = (tdata->loop_start - tdata->adata->blocks) + 1;
            if (tdata->loop_count < 0) {   // infinite loop
                tdata->current_block = start_block;
            } else if (tdata->loop_count == 0) {  // last iteration
                tdata->current_block++;
                tdata->loop_start = NULL;
            } else {
                tdata->loop_count--;
                tdata->current_block = start_block;
            }
            return true;  // try again on new block.
        } else {  // it's a loop block.
            SDL_assert(tdata->loop_start == NULL);
            tdata->loop_start = block;
            tdata->loop_count = block->loop_count;
            tdata->current_block++;
            return true;  // try again on new block.
        }
    }
            
    // still here? Feed data.
    SDL_assert(tdata->frame_pos <= block->frames);
    const Uint64 available = block->frames - tdata->frame_pos;
    Uint8 buffer[512 * sizeof (Uint32)];
    const int framesize = SDL_AUDIO_FRAMESIZE(block->spec);
    const Uint64 frames = SDL_min(available, sizeof (buffer) / framesize);

    if (frames == 0) {  // finished this block.
        tdata->frame_pos = 0;
        tdata->current_block++;
        return true;  // try again, there might be more data available.
    }

    if (tdata->spec.format == SDL_AUDIO_UNKNOWN) {
        SDL_copyp(&tdata->spec, &block->spec);
        SDL_SetAudioStreamFormat(stream, &tdata->spec, NULL);
    }

    if (block->iopos == 0) {  // zero position means write silence (you can't have a data block at position 0 because of headers, etc).
        const void *nullp = NULL;
        SDL_PutAudioStreamPlanarData(stream, &nullp, 1, (int) frames);   // push silence to the stream.
        tdata->frame_pos += frames;
    } else {
        const size_t total = (size_t) (frames * framesize);
        const size_t br = SDL_ReadIO(tdata->io, buffer, total);
        const int frames_read = (int) (br / framesize);
        if (frames_read == 0) {
            return false;  // uhoh.
        }
        SDL_PutAudioStreamData(stream, buffer, frames_read * framesize);
        tdata->frame_pos += frames_read;
    }

    return true;
}

static bool SDLCALL VOC_seek(void *userdata, Uint64 frame)
{
    VOC_TrackData *tdata = (VOC_TrackData *) userdata;
    VOC_TrackData cpy;

    SDL_copyp(&cpy, tdata);

    tdata->loop_start = NULL;
    tdata->loop_count = 0;
    tdata->frame_pos = 0;
    tdata->current_block = 0;

    if (frame == 0) {
        tdata->spec.format = SDL_AUDIO_UNKNOWN;  // we'll set it later.
        return true;  // easy seek to start.
    }

    const size_t num_blocks = tdata->adata->num_blocks;
    Uint64 framepos = 0;
    while (tdata->current_block < num_blocks) {
        const VOC_Block *block = &tdata->adata->blocks[tdata->current_block];
        if (block->loop_count == 0) {  // it's data.
            const int framesize = SDL_AUDIO_FRAMESIZE(block->spec);
            if (frame < (framepos + block->frames)) {
                SDL_assert(frame >= framepos);
                tdata->frame_pos = frame - framepos;
                if (block->iopos) {  // have to read actual file data from this block.
                    if (SDL_SeekIO(tdata->io, block->iopos + ((Sint64)tdata->frame_pos * framesize), SDL_IO_SEEK_SET) < 0) {
                        return false;  // uhoh.
                    }
                }
                if (SDL_memcmp(&tdata->spec, &block->spec, sizeof (tdata->spec)) != 0) {
                    tdata->spec.format = SDL_AUDIO_UNKNOWN;  // we'll set it later.
                }
                return true;  // we're ready!
            } else {
                framepos += block->frames;  // not there yet, keep walking through blocks.
                tdata->current_block++;
            }
        } else if (block->loop_count == -2) {   // it's an endloop block.
            SDL_assert(tdata->loop_start != NULL);
            const size_t start_block = (tdata->loop_start - tdata->adata->blocks) + 1;
            if (tdata->loop_count < 0) {   // infinite loop
                tdata->current_block = start_block;
            } else if (tdata->loop_count == 0) {  // last iteration
                tdata->current_block++;
                tdata->loop_start = NULL;
            } else {
                tdata->loop_count--;
                tdata->current_block = start_block;
            }
            // try again on new block.
        } else {  // it's a loop block.
            SDL_assert(tdata->loop_start == NULL);
            tdata->loop_start = block;
            tdata->loop_count = block->loop_count;
            tdata->current_block++;
            // try again on new block.
        }
    }

    // still here? Seek was past EOF.
    return false;
}

static void SDLCALL VOC_quit_track(void *userdata)
{
    VOC_TrackData *tdata = (VOC_TrackData *) userdata;
    SDL_free(tdata);
}

static void SDLCALL VOC_quit_audio(void *audio_userdata)
{
    VOC_AudioData *tdata = (VOC_AudioData *) audio_userdata;
    SDL_free(tdata->blocks);
    SDL_free(tdata);
}

const MIX_Decoder MIX_Decoder_VOC = {
    "VOC",
    NULL, // init
    VOC_init_audio,
    VOC_init_track,
    VOC_decode,
    VOC_seek,
    NULL,  // jump_to_order
    VOC_quit_track,
    VOC_quit_audio,
    NULL  // quit
};

#endif

