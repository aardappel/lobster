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

#ifdef DECODER_FLAC_LIBFLAC

#include "SDL_mixer_internal.h"

#include <FLAC/stream_decoder.h>

#if defined(FLAC_DYNAMIC) && defined(SDL_ELF_NOTE_DLOPEN)
SDL_ELF_NOTE_DLOPEN(
    "flac",
    "Support for FLAC audio using libFLAC",
    SDL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
    FLAC_DYNAMIC
)
#endif

#ifdef FLAC_DYNAMIC
#define MIX_LOADER_DYNAMIC FLAC_DYNAMIC
#endif

#define MIX_LOADER_FUNCTIONS \
    MIX_LOADER_FUNCTION(true,FLAC__StreamDecoder*,FLAC__stream_decoder_new,(void)) \
    MIX_LOADER_FUNCTION(true,void,FLAC__stream_decoder_delete,(FLAC__StreamDecoder *decoder)) \
    MIX_LOADER_FUNCTION(true,FLAC__StreamDecoderInitStatus,FLAC__stream_decoder_init_stream,(FLAC__StreamDecoder *,FLAC__StreamDecoderReadCallback,FLAC__StreamDecoderSeekCallback,FLAC__StreamDecoderTellCallback,FLAC__StreamDecoderLengthCallback,FLAC__StreamDecoderEofCallback,FLAC__StreamDecoderWriteCallback,FLAC__StreamDecoderMetadataCallback,FLAC__StreamDecoderErrorCallback,void *)) \
    MIX_LOADER_FUNCTION(true,FLAC__StreamDecoderInitStatus,FLAC__stream_decoder_init_ogg_stream,(FLAC__StreamDecoder *,FLAC__StreamDecoderReadCallback,FLAC__StreamDecoderSeekCallback,FLAC__StreamDecoderTellCallback,FLAC__StreamDecoderLengthCallback,FLAC__StreamDecoderEofCallback,FLAC__StreamDecoderWriteCallback,FLAC__StreamDecoderMetadataCallback,FLAC__StreamDecoderErrorCallback,void *)) \
    MIX_LOADER_FUNCTION(true,FLAC__bool,FLAC__stream_decoder_finish,(FLAC__StreamDecoder *decoder)) \
    MIX_LOADER_FUNCTION(true,FLAC__bool,FLAC__stream_decoder_flush,(FLAC__StreamDecoder *decoder)) \
    MIX_LOADER_FUNCTION(true,FLAC__bool,FLAC__stream_decoder_process_single,(FLAC__StreamDecoder *decoder)) \
    MIX_LOADER_FUNCTION(true,FLAC__bool,FLAC__stream_decoder_process_until_end_of_metadata,(FLAC__StreamDecoder *decoder)) \
    MIX_LOADER_FUNCTION(true,FLAC__bool,FLAC__stream_decoder_process_until_end_of_stream,(FLAC__StreamDecoder *decoder)) \
    MIX_LOADER_FUNCTION(true,FLAC__bool,FLAC__stream_decoder_seek_absolute,(FLAC__StreamDecoder *decoder,FLAC__uint64 sample)) \
    MIX_LOADER_FUNCTION(true,FLAC__StreamDecoderState,FLAC__stream_decoder_get_state,(const FLAC__StreamDecoder *decoder)) \
    MIX_LOADER_FUNCTION(true,FLAC__uint64,FLAC__stream_decoder_get_total_samples,(const FLAC__StreamDecoder *decoder)) \
    MIX_LOADER_FUNCTION(true,FLAC__bool,FLAC__stream_decoder_set_metadata_respond,(FLAC__StreamDecoder *decoder,FLAC__MetadataType type)) \

#define MIX_LOADER_MODULE flac
#include "SDL_mixer_loader.h"

typedef struct FLAC_AudioData
{
    bool is_ogg_stream;
    MIX_OggLoop loop;
    SDL_PropertiesID props;
} FLAC_AudioData;

typedef struct FLAC_TrackData
{
    const FLAC_AudioData *adata;
    SDL_IOStream *io;
    FLAC__StreamDecoder *decoder;
    SDL_AudioStream *stream;
    SDL_AudioSpec spec;
    int bits_per_sample;
    Uint8 *cvtbuf;
    size_t cvtbuflen;
    Sint64 current_iteration;
    Sint64 current_iteration_frames;
} FLAC_TrackData;


static FLAC__StreamDecoderReadStatus FLAC_IoRead(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *userdata)
{
    if (*bytes > 0) {
        FLAC_TrackData *tdata = (FLAC_TrackData *) userdata;
        *bytes = SDL_ReadIO(tdata->io, buffer, *bytes);
        const SDL_IOStatus status = SDL_GetIOStatus(tdata->io);
        if (status == SDL_IO_STATUS_READY || status == SDL_IO_STATUS_NOT_READY) {
            return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
        } else if (status == SDL_IO_STATUS_EOF) {
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }
    }
    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}

static FLAC__StreamDecoderSeekStatus FLAC_IoSeek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *userdata)
{
    FLAC_TrackData *tdata = (FLAC_TrackData *) userdata;
    if (SDL_SeekIO(tdata->io, (Sint64)absolute_byte_offset, SDL_IO_SEEK_SET) < 0) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

static FLAC__StreamDecoderTellStatus FLAC_IoTell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *userdata)
{
    FLAC_TrackData *tdata = (FLAC_TrackData *) userdata;
    const Sint64 pos = SDL_TellIO(tdata->io);
    if (pos < 0) {
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
    }
    *absolute_byte_offset = (FLAC__uint64)pos;
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

static FLAC__StreamDecoderLengthStatus FLAC_IoLength(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *userdata)
{
    FLAC_TrackData *tdata = (FLAC_TrackData *) userdata;
    const Sint64 iolen = SDL_GetIOSize(tdata->io);
    if (iolen < 0) {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    }
    *stream_length = (FLAC__uint64)iolen;
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

static FLAC__bool FLAC_IoEOF(const FLAC__StreamDecoder *decoder, void *userdata)
{
    FLAC_TrackData *tdata = (FLAC_TrackData *) userdata;
    return SDL_TellIO(tdata->io) >= SDL_GetIOSize(tdata->io);
}

static FLAC__StreamDecoderWriteStatus FLAC_IoWriteNoOp(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *userdata)
{
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;  // we don't need this data at this moment.
}

static bool SDLCALL FLAC_seek(void *track_userdata, Uint64 frame);

static FLAC__StreamDecoderWriteStatus FLAC_IoWrite(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *userdata)
{
    FLAC_TrackData *tdata = (FLAC_TrackData *) userdata;
    SDL_AudioStream *stream = tdata->stream;
    const int channels = (int) frame->header.channels;

    // !!! FIXME: this is kinda gross, but FLAC 3-channel is FL, FR, FC, whereas SDL is FL, FR, LFE...we don't have a "front center" channel until 5.1 output.  :/  The channel mask thing Sam wants in SDL3 would fix this.
    const int sdlchannels = (channels == 3) ? 6 : channels;

    // change the stream format if we're suddenly getting data in a different format. I assume this can happen if you chain FLAC files together.
    if ((tdata->spec.freq != (int) frame->header.sample_rate) || (tdata->spec.channels != sdlchannels) || (tdata->bits_per_sample != (int) frame->header.bits_per_sample)) {
        tdata->bits_per_sample = (int) frame->header.bits_per_sample;
        tdata->spec.freq = (int) frame->header.sample_rate;
        tdata->spec.channels = sdlchannels;
        // decoded FLAC data is always int, from 4 to 32 bits, apparently.
        if (tdata->bits_per_sample <= 8) {
            tdata->spec.format = SDL_AUDIO_S8;
        } else if (tdata->bits_per_sample <= 16) {
            tdata->spec.format = SDL_AUDIO_S16;
        } else if (tdata->bits_per_sample <= 32) {
            tdata->spec.format = SDL_AUDIO_S32;
        }
        SDL_SetAudioStreamFormat(stream, &tdata->spec, NULL);
    }

    size_t amount = (size_t) frame->header.blocksize;
    const size_t onebuflen = ((size_t) SDL_AUDIO_BYTESIZE(tdata->spec.format)) * amount;
    const size_t buflen = onebuflen * channels;
    if (tdata->cvtbuflen < buflen) {
        void *ptr = SDL_realloc(tdata->cvtbuf, buflen);
        if (!ptr) {
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        tdata->cvtbuf = (Uint8 *) ptr;
        tdata->cvtbuflen = buflen;
    }

    const MIX_OggLoop *loop = &tdata->adata->loop;
    if (tdata->current_iteration < 0) {
        if (loop->active && ((tdata->current_iteration_frames + (Sint64)amount) >= loop->start)) {
            tdata->current_iteration = 0;  // we've hit the start of the loop point.
            tdata->current_iteration_frames = (tdata->current_iteration_frames - loop->start);  // so adding `amount` corrects this later.
        }
    }

    if (tdata->current_iteration >= 0) {
        SDL_assert(loop->active);
        SDL_assert(tdata->current_iteration_frames <= loop->len);
        const Sint64 available = loop->len - tdata->current_iteration_frames;
        if ((Sint64)amount > available) {
            amount = available;
        }

        SDL_assert(tdata->current_iteration_frames <= loop->len);
        if ((tdata->current_iteration_frames + (Sint64)amount) >= loop->len) {  // time to loop?
            bool should_loop = false;
            if (loop->count < 0) {  // negative==infinite loop
                tdata->current_iteration = 0;
                should_loop = true;
            } else {
                tdata->current_iteration++;
                SDL_assert(tdata->current_iteration <= loop->count);
                if (tdata->current_iteration < loop->count) {
                    should_loop = true;
                }
            }

            if (should_loop) {
                const Uint64 nextframe = ((Uint64) loop->start) + ( ((Uint64) loop->len) * ((Uint64) tdata->current_iteration) );
                if (!FLAC_seek(tdata, nextframe)) {
                    return false;
                }
            } else {
                tdata->current_iteration = -1;
            }
            tdata->current_iteration_frames = 0;
        }
    }

    if (amount > 0) {
        void *channel_arrays[32];
        SDL_assert(SDL_arraysize(channel_arrays) >= (size_t)channels);

        // (If we padded out to 5.1 to get a front-center channel, the unnecessary channels happen
        //   to be at the end, so it'll assume those padded channels are silent.)
        for (int i = 0; i < channels; i++) {
            channel_arrays[i] = tdata->cvtbuf + (i * onebuflen);
        }

        const int shift = SDL_AUDIO_BITSIZE(tdata->spec.format) - tdata->bits_per_sample;

        #define PREP_CHANNEL_ARRAY(typ) { \
            for (int channel = 0; channel < channels; channel++) { \
                const Sint32 *src = (const Sint32 *) buffer[channel]; \
                typ *dst = (typ *) channel_arrays[channel]; \
                for (size_t i = 0; i < amount; i++) { \
                    dst[i] = (typ) (src[i] << shift); \
                } \
            } \
        }

        switch (tdata->spec.format) {
            case SDL_AUDIO_S8: PREP_CHANNEL_ARRAY(Sint8); break;
            case SDL_AUDIO_S16: PREP_CHANNEL_ARRAY(Sint16); break;
            case SDL_AUDIO_S32: PREP_CHANNEL_ARRAY(Sint32); break;
            default: SDL_assert(!"Unexpected audio data type"); return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }

        #undef PREP_CHANNEL_ARRAY

        SDL_PutAudioStreamPlanarData(stream, (const void * const *) channel_arrays, channels, amount);
        tdata->current_iteration_frames += amount;
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void FLAC_IoMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *userdata)
{
    FLAC_TrackData *tdata = (FLAC_TrackData *) userdata;
    FLAC_AudioData *adata = (FLAC_AudioData *) tdata->adata;  // cast away constness here. This metadata callback is when we're initializing adata.

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        tdata->spec.freq = metadata->data.stream_info.sample_rate;
        tdata->spec.channels = metadata->data.stream_info.channels;   // (if we need the 3-channel map magic, it'll notice spec.channels is wrong when we get to FLAC_IoWrite and set it up.)
        tdata->bits_per_sample = (int) metadata->data.stream_info.bits_per_sample;

        // decoded FLAC data is always int, from 4 to 32 bits, apparently.
        if (tdata->bits_per_sample <= 8) {
            tdata->spec.format = SDL_AUDIO_S8;
        } else if (tdata->bits_per_sample <= 16) {
            tdata->spec.format = SDL_AUDIO_S16;
        } else if (tdata->bits_per_sample <= 32) {
            tdata->spec.format = SDL_AUDIO_S32;
        }
    } else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        const FLAC__StreamMetadata_VorbisComment *vc = &metadata->data.vorbis_comment;
        const int num_comments = (int) vc->num_comments;
        char **comments = (char **) SDL_malloc(num_comments * sizeof (*comments));
        if (comments) {
            for (int i = 0; i < num_comments; i++) {
            	comments[i] = (char *) vc->comments[i].entry;
            }
            MIX_ParseOggComments(adata->props, tdata->spec.freq, (const char *) vc->vendor_string.entry, (const char * const *) comments, num_comments, &adata->loop);
            SDL_free(comments);
        }
    }
}

static void FLAC_IoError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    // (currently) don't care.
}

static void FLAC_IoMetadataNoOp(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *userdata)
{
    // don't care about metadata at this point.
}

static bool SDLCALL FLAC_init(void)
{
    if (!LoadModule_flac()) {
        return false;
    }
    return true;
}

static void SDLCALL FLAC_quit(void)
{
    UnloadModule_flac();
}

static bool SDLCALL FLAC_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    // just load the bare minimum from the IOStream to verify it's a FLAC file (if it's an Ogg stream, we'll let libFLAC try to parse it out).
    bool is_ogg_stream = false;

    char magic[4];
    if (SDL_ReadIO(io, magic, 4) != 4) {
        return false;
    } else if (SDL_memcmp(magic, "OggS", 4) == 0) {
        is_ogg_stream = true;  // MAYBE flac, might be vorbis, etc.
    } else if (SDL_memcmp(magic, "fLaC", 4) != 0) {
        return SDL_SetError("Not a FLAC audio stream");
    } else if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {  // rewind, let libFLAC process through the metadata, so we know it's definitely a FLAC file and we have the props.
        return false;
    }

    FLAC_TrackData tdata;
    SDL_zero(tdata);
    tdata.io = io;

    tdata.decoder = flac.FLAC__stream_decoder_new();
    if (!tdata.decoder) {
        return SDL_SetError("FLAC__stream_decoder_new() failed");
    }

    flac.FLAC__stream_decoder_set_metadata_respond(tdata.decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);

    FLAC_AudioData *adata = (FLAC_AudioData *) SDL_calloc(1, sizeof (*adata));
    if (!adata) {
        flac.FLAC__stream_decoder_delete(tdata.decoder);
        return false;
    }

    adata->is_ogg_stream = is_ogg_stream;
    adata->props = props;

    tdata.adata = adata;

    FLAC__StreamDecoderInitStatus ret;
    if (is_ogg_stream) {
        ret = flac.FLAC__stream_decoder_init_ogg_stream(tdata.decoder, FLAC_IoRead, FLAC_IoSeek, FLAC_IoTell, FLAC_IoLength, FLAC_IoEOF, FLAC_IoWriteNoOp, FLAC_IoMetadata, FLAC_IoError, &tdata);
    } else {
        ret = flac.FLAC__stream_decoder_init_stream(tdata.decoder, FLAC_IoRead, FLAC_IoSeek, FLAC_IoTell, FLAC_IoLength, FLAC_IoEOF, FLAC_IoWriteNoOp, FLAC_IoMetadata, FLAC_IoError, &tdata);
    }

    if (ret != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        flac.FLAC__stream_decoder_delete(tdata.decoder);
        return SDL_SetError("FLAC__stream_decoder_init_stream() failed");
    }

    const bool rc = !!flac.FLAC__stream_decoder_process_until_end_of_metadata(tdata.decoder);
    const Sint64 total_frames = rc ? (Sint64) flac.FLAC__stream_decoder_get_total_samples(tdata.decoder) : -1;

    // Dump this stream no matter what. Either we failed, or we know it's a real FLAC and have its metadata pushed to `props`.
    flac.FLAC__stream_decoder_finish(tdata.decoder);
    flac.FLAC__stream_decoder_delete(tdata.decoder);

    if (!rc) {
        SDL_free(adata);
        return SDL_SetError("FLAC__stream_decoder_process_until_end_of_metadata() failed");
    }

    // metadata processing should have filled in several things in `adata`.
    adata->props = 0;  // metadata callbacks needed this, but don't store this past this function.

    SDL_copyp(spec, &tdata.spec);

    if (adata->loop.end > total_frames) {
        adata->loop.active = false;
    }

    if (adata->loop.active) {
        *duration_frames = (adata->loop.count < 0) ? MIX_DURATION_INFINITE : (total_frames * adata->loop.count);
    } else {
        *duration_frames = total_frames;
    }

    *audio_userdata = adata;

    return true;
}

static bool SDLCALL FLAC_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    FLAC_TrackData *tdata = (FLAC_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    const FLAC_AudioData *adata = (const FLAC_AudioData *) audio_userdata;
    tdata->adata = adata;
    tdata->io = io;
    tdata->decoder = flac.FLAC__stream_decoder_new();
    tdata->current_iteration = -1;

    if (!tdata->decoder) {
        SDL_free(tdata);
        return SDL_SetError("FLAC__stream_decoder_new() failed");
    }

    FLAC__StreamDecoderInitStatus ret;
    if (adata->is_ogg_stream) {
        ret = flac.FLAC__stream_decoder_init_ogg_stream(tdata->decoder, FLAC_IoRead, FLAC_IoSeek, FLAC_IoTell, FLAC_IoLength, FLAC_IoEOF, FLAC_IoWrite, FLAC_IoMetadataNoOp, FLAC_IoError, tdata);
    } else {
        ret = flac.FLAC__stream_decoder_init_stream(tdata->decoder, FLAC_IoRead, FLAC_IoSeek, FLAC_IoTell, FLAC_IoLength, FLAC_IoEOF, FLAC_IoWrite, FLAC_IoMetadataNoOp, FLAC_IoError, tdata);
    }

    if (ret != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        flac.FLAC__stream_decoder_delete(tdata->decoder);
        SDL_free(tdata);
        return SDL_SetError("FLAC__stream_decoder_init_stream() failed");
    }

    if (!flac.FLAC__stream_decoder_process_until_end_of_metadata(tdata->decoder)) {
        flac.FLAC__stream_decoder_finish(tdata->decoder);
        flac.FLAC__stream_decoder_delete(tdata->decoder);
        SDL_free(tdata);
        SDL_SetError("FLAC__stream_decoder_process_until_end_of_metadata() failed");
    }

    SDL_copyp(&tdata->spec, spec);

    *track_userdata = tdata;
    return true;
}

static bool SDLCALL FLAC_decode(void *track_userdata, SDL_AudioStream *stream)
{
    FLAC_TrackData *tdata = (FLAC_TrackData *) track_userdata;
    tdata->stream = stream;

    if (!flac.FLAC__stream_decoder_process_single(tdata->decoder)) {  // write callback will fill in stream. Might fill 0 if it hit a metadata block, but the higher level loops to get what it needs.
        return false;
    } else if (flac.FLAC__stream_decoder_get_state(tdata->decoder) == FLAC__STREAM_DECODER_END_OF_STREAM) {
        return false;  // we're done.
    }

    return true;
}

static bool SDLCALL FLAC_seek(void *track_userdata, Uint64 frame)
{
    FLAC_TrackData *tdata = (FLAC_TrackData *) track_userdata;
    const MIX_OggLoop *loop = &tdata->adata->loop;
    Sint64 final_iteration = -1;
    Sint64 final_iteration_frames = 0;

    // frame has hit the loop point?
    if (loop->active && ((Sint64)frame >= loop->start)) {
        // figure out the _actual_ frame in the vorbis file we're aiming for.
        if ((loop->count < 0) || ((Sint64)frame < (loop->len * loop->count))) {  // literally in the loop right now.
            frame -= loop->start;  // make logical frame index relative to start of loop.
            final_iteration = (loop->count < 0) ? 0 : (frame / loop->len);  // decide what iteration of the loop we're on (stays at zero for infinite loops).
            frame %= loop->len;  // drop iterations so we're an offset into the loop.
            final_iteration_frames = frame;
            frame += loop->start;  // convert back into physical frame index.
        } else {  // past the loop point?
            SDL_assert(loop->count > 0);  // can't be infinite loop if we passed it.
            frame -= loop->len * loop->count;  // drop the iterations to get the physical frame index.
        }
    }

    if (!flac.FLAC__stream_decoder_seek_absolute(tdata->decoder, frame)) {
        if (flac.FLAC__stream_decoder_get_state(tdata->decoder) == FLAC__STREAM_DECODER_SEEK_ERROR) {
            flac.FLAC__stream_decoder_flush(tdata->decoder);
        }
        return SDL_SetError("Seeking of FLAC stream failed: libFLAC seek failed.");
    }

    tdata->current_iteration = final_iteration;
    tdata->current_iteration_frames = final_iteration_frames;

    return true;
}

static void SDLCALL FLAC_quit_track(void *track_userdata)
{
    FLAC_TrackData *tdata = (FLAC_TrackData *) track_userdata;
    tdata->stream = NULL;
    flac.FLAC__stream_decoder_finish(tdata->decoder);
    flac.FLAC__stream_decoder_delete(tdata->decoder);
    SDL_free(tdata->cvtbuf);
    SDL_free(tdata);
}

static void SDLCALL FLAC_quit_audio(void *audio_userdata)
{
    FLAC_AudioData *adata = (FLAC_AudioData *) audio_userdata;
    SDL_free(adata);
}

const MIX_Decoder MIX_Decoder_FLAC = {
    "FLAC",
    FLAC_init,
    FLAC_init_audio,
    FLAC_init_track,
    FLAC_decode,
    FLAC_seek,
    NULL,  // jump_to_order
    FLAC_quit_track,
    FLAC_quit_audio,
    FLAC_quit
};

#endif
