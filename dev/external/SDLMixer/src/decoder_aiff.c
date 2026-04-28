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

/*
  This is the source needed to decode an AIFF file into a waveform.
  It's pretty straightforward once you get going.

  This file by TorbjÃÂ¶rn Andersson (torbjorn.andersson@eurotime.se)
  8SVX file support added by Marc Le Douarain (mavati@club-internet.fr)
  in december 2002.
*/

#ifdef DECODER_AIFF

#include "SDL_mixer_internal.h"

// (This code is originally from SDL2_mixer, heavily modified here.)

/*********************************************/
/* Define values for AIFF (IFF audio) format */
/*********************************************/
#define FORM        0x4d524f46      /* "FORM" */
#define AIFF        0x46464941      /* "AIFF" */
#define AIFC        0x43464941      /* "AIFÃÂ¡" */
#define FVER        0x52455646      /* "FVER" */
#define SSND        0x444e5353      /* "SSND" */
#define COMM        0x4d4d4f43      /* "COMM" */
#define AIFF_ID3_   0x20334449      /* "ID3 " */
#define MARK        0x4B52414D      /* "MARK" */
#define INST        0x54534E49      /* "INST" */
#define AUTH        0x48545541      /* "AUTH" */
#define NAME        0x454D414E      /* "NAME" */
#define _c__        0x20296328      /* "(c) " */
#define ANNO        0x4F4E4E41      /* "ANNO" */

/* Supported compression types */
#define NONE        0x454E4F4E      /* "NONE" */
#define sowt        0x74776F73      /* "sowt" */
#define raw_        0x20776172      /* "raw " */
#define ulaw        0x77616C75      /* "ulaw" */
#define alaw        0x77616C61      /* "alaw" */
#define ULAW        0x57414C55      /* "ULAW" */
#define ALAW        0x57414C41      /* "ALAW" */
#define fl32        0x32336C66      /* "fl32" */
#define fl64        0x34366C66      /* "fl64" */
#define FL32        0x32334C46      /* "FL32" */

typedef struct AIFF_TrackData AIFF_TrackData;

typedef int (*AIFF_FetchFn)(AIFF_TrackData *tdata, Uint8 *buffer, int buflen);

typedef struct AIFF_AudioData
{
    Sint64 start;
    Sint64 stop;
    int framesize;
    int decoded_framesize;
    AIFF_FetchFn fetch;
    Sint64 num_pcm_frames;
} AIFF_AudioData;

struct AIFF_TrackData
{
    const AIFF_AudioData *adata;
    SDL_IOStream *io;
};

static int FetchXLaw(AIFF_TrackData *tdata, Uint8 *buffer, int buflen, const float *lut)
{
    int length = buflen;
    length = (int) SDL_ReadIO(tdata->io, buffer, (size_t)(length / 4));
    if (length % tdata->adata->framesize != 0) {
        length -= length % tdata->adata->framesize;
    }
    float *out = (float *) &buffer[(length - 1) * 4];
    for (int i = length - 1; i >= 0; i--) {
        out[i] = lut[buffer[i]];
    }
    return length * 4;
}

static int FetchULaw(AIFF_TrackData *tdata, Uint8 *buffer, int buflen)
{
    return FetchXLaw(tdata, buffer, buflen, MIX_ulawToFloat);
}

static int FetchALaw(AIFF_TrackData *tdata, Uint8 *buffer, int buflen)
{
    return FetchXLaw(tdata, buffer, buflen, MIX_alawToFloat);
}

static int FetchPCM(AIFF_TrackData *tdata, Uint8 *buffer, int buflen)
{
    return SDL_ReadIO(tdata->io, buffer, buflen);
}

static int FetchPCM24LE(AIFF_TrackData *tdata, Uint8 *buffer, int buflen)
{
    int length = buflen;
    length = (int) SDL_ReadIO(tdata->io, buffer, (size_t)((length / 4) * 3));
    if ((length % tdata->adata->framesize) != 0) {
        length -= length % tdata->adata->framesize;
    }
    int i = 0, o = 0;
    for (i = length - 3, o = ((length - 3) / 3) * 4; i >= 0; i -= 3, o -= 4) {
        const Uint8 *x = &buffer[i];
        const Sint32 in = ((Sint32)(Sint8)x[2] << 16) | ((int32_t)x[1] << 8) | x[0];
        float *out = (float *) &buffer[o];
        *out = ((float) in) / 8388608.0f;
    }
    return (length / 3) * 4;
}

static int FetchPCM24BE(AIFF_TrackData *tdata, Uint8 *buffer, int buflen)
{
    int length = buflen;
    length = (int) SDL_ReadIO(tdata->io, buffer, (size_t)((length / 4) * 3));
    if ((length % tdata->adata->framesize) != 0) {
        length -= length % tdata->adata->framesize;
    }
    int i = 0, o = 0;
    for (i = length - 3, o = ((length - 3) / 3) * 4; i >= 0; i -= 3, o -= 4) {
        const Uint8 *x = &buffer[i];
        const Sint32 in = ((Sint32)(Sint8)x[0] << 16) | ((int32_t)x[1] << 8) | x[2];
        float *out = (float *) &buffer[o];
        *out = ((float) in) / 8388608.0f;
    }
    return (length / 3) * 4;
}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
static double MIX_SwapDoubleBE(double x)
{
    union
    {
        double f;
        Uint64 ui64;
    } swapper;
    swapper.f = x;
    swapper.ui64 = SDL_Swap64(swapper.ui64);
    return swapper.f;
}
#else
#define MIX_SwapDoubleBE(X)  (X)
#endif

static int FetchFloat64BE(AIFF_TrackData *tdata, Uint8 *buffer, int buflen)
{
    int length = buflen;
    length = (int) SDL_ReadIO(tdata->io, buffer, (size_t)length);
    if (length % tdata->adata->framesize != 0) {
        length -= length % tdata->adata->framesize;
    }
    float *out = (float *) buffer;
    int i, o;
    for (i = 0, o = 0; i < length; i += 8, o++) {
        out[o] = (float)MIX_SwapDoubleBE(*(double*)(buffer + i));
    }
    return length / 2;
}



// I couldn't get SANE_to_double() to work, so I stole this from libsndfile.
// I don't pretend to fully understand it.
static Uint32 SANE_to_Uint32(const Uint8 *sanebuf)
{
    const Uint8 sb0 = sanebuf[0];
    const Uint8 sb1 = sanebuf[1];
    if (sb0 & 0x80) {  // Negative number?
        return 0;
    } else if (sb0 <= 0x3F) {  // Less than 1?
        return 1;
    } else if (sb0 > 0x40) {  // Way too big?
        return 0x4000000;
    } else if ((sb0 == 0x40) && (sb1 > 0x1C)) {  // Still too big?
        return 800000000;
    }
    return (Uint32) (((sanebuf[2] << 23) | (sanebuf[3] << 15) | (sanebuf[4] << 7) | (sanebuf[5] >> 1)) >> (29 - sb1));
}

static bool ParseAIFFID3(SDL_IOStream *io, SDL_PropertiesID props, Uint32 chunk_length)
{
    MIX_IoClamp clamp;
    SDL_IOStream *ioclamp = MIX_OpenIoClamp(&clamp, io);
    if (!ioclamp) {
        return false;
    }
    clamp.length = (Sint64) chunk_length;
    MIX_ReadMetadataTags(ioclamp, props, &clamp);
    SDL_CloseIO(ioclamp);
    return true;
}

static bool CheckAIFFMetadataField(const char *propname, SDL_IOStream *io, SDL_PropertiesID props, Uint32 chunk_type, Uint32 chunk_length, int dupcount)
{
    char *chunk_buffer = (char*)SDL_malloc(chunk_length + 1);
    if (!chunk_buffer) {
        return false;
    } else if (SDL_ReadIO(io, chunk_buffer, chunk_length) != chunk_length) {
        SDL_free(chunk_buffer);
        return false;
    }

    chunk_buffer[chunk_length] = '\0';

    char dupstr[16] = { '\0' };
    if (dupcount >= 0) {
        SDL_snprintf(dupstr, sizeof (dupstr), "%d", dupcount);
    }
    char key[64];
    SDL_snprintf(key, sizeof (key), "SDL_mixer.metadata.aiff.%c%c%c%c%s", (chunk_type >> 0) & 0xFF, (chunk_type >> 8) & 0xFF, (chunk_type >> 16) & 0xFF, (chunk_type >> 24) & 0xFF, dupstr);
    SDL_SetStringProperty(props, key, chunk_buffer);
    if (propname && !SDL_HasProperty(props, propname)) {
        SDL_SetStringProperty(props, propname, chunk_buffer);
    }

    SDL_free(chunk_buffer);
    return true;
}

static bool AIFF_init_audio_internal(AIFF_AudioData *adata, SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props)
{
    Uint32 offset = 0;
    Uint32 blocksize = 0;

    Uint16 channels = 0;
    Uint32 numsamples = 0;
    Uint16 samplesize = 0;
    Uint32 frequency = 0;
    Uint32 AIFCVersion1 = 0;
    Uint32 compressionType = 0;
    Uint32 chunk_length = 0;
    Uint32 chunk_type = 0;
    int anno_count = 0;

    const Sint64 flen = SDL_GetIOSize(io);

    // Check the magic header
    if (!SDL_ReadU32LE(io, &chunk_type) || !SDL_ReadU32BE(io, &chunk_length)) {
        return false;
    } else if (chunk_type != FORM) {
        return SDL_SetError("AIFF: Unrecognized file type (not FORM chunk)");
    } else if (chunk_length > (flen-8)) {
        return SDL_SetError("AIFF: Corrupt file (primary chunk larger than file)");
    } else if (!SDL_ReadU32LE(io, &chunk_type)) {
        return false;
    } else if ((chunk_type != AIFF) && (chunk_type != AIFC)) {
        return SDL_SetError("AIFF: Unrecognized file type (not AIFF or AIFC)");
    }

    const bool is_AIFC = (chunk_type == AIFC);

    // From what I understand of the specification, chunks may appear in
    // any order, and we should just ignore unknown ones.
    //
    // TODO: Better sanity-checking. E.g. what happens if the AIFF file contains compressed sound data?

    bool found_SSND = false;
    bool found_COMM = false;
    bool found_FVER = false;

    Sint64 next_chunk = 0;
    do {
        if (!SDL_ReadU32LE(io, &chunk_type) || !SDL_ReadU32BE(io, &chunk_length)) {
            return false;
        }

        const Sint64 chunk_start_position = SDL_TellIO(io);
        next_chunk = chunk_start_position + chunk_length;
        if (chunk_start_position < 0) {
            return false;
        } else if (next_chunk > flen) {
            return SDL_SetError("AIFF: Corrupt AIFF file (chunk goes past EOF)");
        }

        if (chunk_length % 2) {
            next_chunk++;  // pad to 16-bit word size.
        }

        switch (chunk_type) {
        case SSND:
            found_SSND = true;
            if (!SDL_ReadU32BE(io, &offset) || !SDL_ReadU32BE(io, &blocksize)) {
                return false;
            }
            adata->start = SDL_TellIO(io) + offset;
            (void)blocksize; // unused
            break;

        case FVER:
            found_FVER = true;
            if (!SDL_ReadU32BE(io, &AIFCVersion1)) {
                return false;
            }
            (void)AIFCVersion1; // unused
            break;

        case AIFF_ID3_:
            if (!ParseAIFFID3(io, props, chunk_length)) {
                return false;
            }
            break;

        case NAME:
            if (!CheckAIFFMetadataField(MIX_PROP_METADATA_TITLE_STRING, io, props, chunk_type, chunk_length, -1)) {
                return false;
            }
            break;

        case AUTH:
            if (!CheckAIFFMetadataField(MIX_PROP_METADATA_ARTIST_STRING, io, props, chunk_type, chunk_length, -1)) {
                return false;
            }
            break;

        case _c__:
            if (!CheckAIFFMetadataField(MIX_PROP_METADATA_COPYRIGHT_STRING, io, props, chunk_type, chunk_length, -1)) {
                return false;
            }
            break;

        case ANNO:
            if (!CheckAIFFMetadataField(NULL, io, props, chunk_type, chunk_length, anno_count++)) {
                return false;
            }
            break;

        case COMM: {
            found_COMM = true;

            // Read the audio data format chunk
            Uint8 sane_freq[10];
            if (!SDL_ReadU16BE(io, &channels) ||
                !SDL_ReadU32BE(io, &numsamples) ||
                !SDL_ReadU16BE(io, &samplesize) ||
                SDL_ReadIO(io, sane_freq, sizeof(sane_freq)) != sizeof(sane_freq)) {
                return false;
            }
            frequency = SANE_to_Uint32(sane_freq);
            if (is_AIFC) {
                if (!SDL_ReadU32LE(io, &compressionType)) {
                    return false;
                }
                // here must be a "compressionName" which is a padded string
            }
            break;
        }

        default:
            break;  // Unknown/unsupported chunk: we just skip over it.
        }
    } while ((next_chunk < flen) && (SDL_SeekIO(io, next_chunk, SDL_IO_SEEK_SET) >= 0));

    if (!found_SSND) {
        return SDL_SetError("AIFF: Bad AIFF/AIFF-C file (no SSND chunk)");
    } else if (!found_COMM) {
        return SDL_SetError("AIFF: Bad AIFF/AIFF-C file (no COMM chunk)");
    } else if (is_AIFC && !found_FVER) {
        return SDL_SetError("AIFF: Bad AIFF-C file (no FVER chunk)");
    }

    adata->framesize = channels * (samplesize / 8);
    adata->stop = adata->start + channels * numsamples * (samplesize / 8);
    adata->fetch = FetchPCM;
    adata->num_pcm_frames = (Sint64) numsamples;

    // Decode the audio data format
    spec->freq = (int)frequency;
    bool unsupported_format = false;
    switch (samplesize) {
    case 8:
        if (!is_AIFC) {
            spec->format = SDL_AUDIO_S8;
        } else {
            switch (compressionType) {
            case raw_: spec->format = SDL_AUDIO_U8; break;
            case sowt: spec->format = SDL_AUDIO_S8; break;
            case ulaw:
                spec->format = SDL_AUDIO_F32;
                adata->fetch = FetchULaw;
                break;
            case alaw:
                spec->format = SDL_AUDIO_F32;
                adata->fetch = FetchALaw;
                break;
            default: unsupported_format = true; break;
            }
        }
        break;
    case 16:
        if (!is_AIFC) {
            spec->format = SDL_AUDIO_S16BE;
        } else {
            switch (compressionType) {
            case sowt: spec->format = SDL_AUDIO_S16LE; break;
            case NONE: spec->format = SDL_AUDIO_S16BE; break;
            case ULAW:
                spec->format = SDL_AUDIO_F32;
                adata->fetch = FetchULaw;
                break;
            case ALAW:
                spec->format = SDL_AUDIO_F32;
                adata->fetch = FetchALaw;
                break;
            default: unsupported_format = true; break;
            }
        }
        break;
    case 24:
        adata->fetch = FetchPCM24BE;
        spec->format = SDL_AUDIO_F32; 
        if (is_AIFC) {
            switch (compressionType) {
            case sowt: adata->fetch = FetchPCM24LE; break;
            case NONE: break;
            default: unsupported_format = true; break;
            }
        }
        break;
    case 32:
        if (!is_AIFC) {
            spec->format = SDL_AUDIO_S32BE;
        } else {
            switch (compressionType) {
            case sowt: spec->format = SDL_AUDIO_S32LE; break;
            case NONE: spec->format = SDL_AUDIO_S32BE; break;
            case fl32:
            case FL32: spec->format = SDL_AUDIO_F32BE; break;
            default: unsupported_format = true; break;
            }
        }
        break;
    case 64:
        adata->fetch = FetchFloat64BE;
        if (!is_AIFC) {
            spec->format = SDL_AUDIO_F32;
        } else {
            switch (compressionType) {
            case fl64:
                spec->format = SDL_AUDIO_F32;
                break;
            default: unsupported_format = true; break;
            }
        }
        break;
    default:
        unsupported_format = true;
        break;
    }

    if (unsupported_format) {
        return SDL_SetError("AIFF: unsupported data format");
    }

    spec->channels = (Uint8) channels;
    adata->decoded_framesize = SDL_AUDIO_FRAMESIZE(*spec);

    return true;
}

static bool SDLCALL AIFF_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    // quick rejection before we allocate anything.
    Uint32 AIFFmagic;
    if (!SDL_ReadU32LE(io, &AIFFmagic)) {
        return false;
    } else if (AIFFmagic != FORM) {
        return SDL_SetError("AIFF: Not an AIFF file");
    } else if (!SDL_ReadU32BE(io, &AIFFmagic)) {   // we don't care about the length of the FORM field here.
        return false;
    } else if (!SDL_ReadU32LE(io, &AIFFmagic)) {
        return false;
    } else if ((AIFFmagic != AIFF) && (AIFFmagic != AIFC)) {
        return SDL_SetError("AIFF: Not an AIFF file");
    } else if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {
        return false;
    }

    AIFF_AudioData *adata = (AIFF_AudioData *) SDL_calloc(1, sizeof (*adata));
    if (!adata) {
        return false;
    }

    const bool rc = AIFF_init_audio_internal(adata, io, spec, props);
    if (!rc) {
        SDL_free(adata);
        return false;
    }

    *duration_frames = adata->num_pcm_frames;
    *audio_userdata = adata;

    return true;
}

static bool SDLCALL AIFF_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    const AIFF_AudioData *adata = (const AIFF_AudioData *) audio_userdata;
    AIFF_TrackData *tdata = (AIFF_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    tdata->adata = adata;
    tdata->io = io;

    *track_userdata = tdata;

    return true;
}

static bool SDLCALL AIFF_decode(void *track_userdata, SDL_AudioStream *stream)
{
    AIFF_TrackData *tdata = (AIFF_TrackData *) track_userdata;

    Uint8 buffer[1024];
    int buflen = (int) sizeof (buffer);
    const int mod = buflen % tdata->adata->decoded_framesize;
    if (mod) {
        buflen -= mod;
    }
    const int br = tdata->adata->fetch(tdata, buffer, buflen);  // this will deal with different formats that might need decompression or conversion.
    if (br <= 0) {
        return false;
    }

    SDL_PutAudioStreamData(stream, buffer, br);
    return true;
}

static bool SDLCALL AIFF_seek(void *track_userdata, Uint64 frame)
{
    AIFF_TrackData *tdata = (AIFF_TrackData *) track_userdata;
    const AIFF_AudioData *adata = tdata->adata;
    const Sint64 dest_offset = (Sint64)frame * adata->framesize;
    const Sint64 destpos = adata->start + dest_offset;
    if (destpos > adata->stop) {
        return false;
    } else if (SDL_SeekIO(tdata->io, destpos, SDL_IO_SEEK_SET) < 0) {
        return false;
    }

    return true;
}

static void SDLCALL AIFF_quit_track(void *track_userdata)
{
    AIFF_TrackData *tdata = (AIFF_TrackData *) track_userdata;
    SDL_free(tdata);
}

static void SDLCALL AIFF_quit_audio(void *audio_userdata)
{
    AIFF_AudioData *adata = (AIFF_AudioData *) audio_userdata;
    SDL_free(adata);
}

const MIX_Decoder MIX_Decoder_AIFF = {
    "AIFF",
    NULL,  // init
    AIFF_init_audio,
    AIFF_init_track,
    AIFF_decode,
    AIFF_seek,
    NULL,  // jump_to_order
    AIFF_quit_track,
    AIFF_quit_audio,
    NULL  // quit
};

#endif

