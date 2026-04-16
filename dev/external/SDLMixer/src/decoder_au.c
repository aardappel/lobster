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

// This code originally came from SDL_sound, and was originally written by
// Mattias Engdegård. It's been heavily modified for SDL3_mixer, so don't
// bother Mattias about bugs, they're probably not his fault.  :)

// Sun/NeXT .au decoder for SDL_sound.
// Formats supported: 8 and 16 bit linear PCM, 8 bit mu-law.
// Files without valid header are assumed to be 8 bit mu-law, 8kHz, mono.

#ifdef DECODER_AU

#include "SDL_mixer_internal.h"

#define AU_MAGIC 0x2E736E64  // ".snd", in ASCII (bigendian number)

typedef struct AU_file_hdr
{
    Uint32 magic;
    Uint32 hdr_size;
    Uint32 data_size;
    Uint32 encoding;
    Uint32 sample_rate;
    Uint32 channels;
} AU_file_hdr;

typedef enum AU_Encoding
{
    AU_ENC_ULAW_8       = 1,        // 8-bit ISDN mu-law
    AU_ENC_LINEAR_8     = 2,        // 8-bit linear PCM
    AU_ENC_LINEAR_16    = 3,        // 16-bit linear PCM

    // the rest are unsupported (I have never seen them in the wild)
    AU_ENC_LINEAR_24    = 4,        // 24-bit linear PCM
    AU_ENC_LINEAR_32    = 5,        // 32-bit linear PCM 
    AU_ENC_FLOAT        = 6,        // 32-bit IEEE floating point
    AU_ENC_DOUBLE       = 7,        // 64-bit IEEE floating point
    // more Sun formats, not supported either
    AU_ENC_ADPCM_G721   = 23,
    AU_ENC_ADPCM_G722   = 24,
    AU_ENC_ADPCM_G723_3 = 25,
    AU_ENC_ADPCM_G723_5 = 26,
    AU_ENC_ALAW_8       = 27
} AU_Encoding;

typedef struct AU_AudioData
{
    Uint32 start_offset;
    int framesize;    // encoded frame size! So ulaw will produce float32 samples, but the framesize is 1 byte * channels.
    AU_Encoding encoding;
} AU_AudioData;

typedef struct AU_TrackData
{
    const AU_AudioData *adata;
    SDL_IOStream *io;
} AU_TrackData;

// Read in the AU header from disk. This makes this process safe
//  regardless of the processor's byte order or how the AU_file_hdr
//  structure is packed.
static bool ReadAUHeader(SDL_IOStream *io, AU_file_hdr *hdr)
{
    return SDL_ReadU32BE(io, &hdr->magic) &&
           SDL_ReadU32BE(io, &hdr->hdr_size) &&
           SDL_ReadU32BE(io, &hdr->data_size) &&
           SDL_ReadU32BE(io, &hdr->encoding) &&
           SDL_ReadU32BE(io, &hdr->sample_rate) &&
           SDL_ReadU32BE(io, &hdr->channels);
}

static bool SDLCALL AU_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    AU_AudioData adata;
    SDL_zero(adata);

    // ReadAUHeader() will do byte order swapping.
    AU_file_hdr hdr;
    if (!ReadAUHeader(io, &hdr)) {
        return SDL_SetError("AU: bad header");
    }

    Uint32 total_frames = 0;
    if (hdr.magic == AU_MAGIC) {  // valid magic?
        switch (hdr.encoding) {
            case AU_ENC_ULAW_8:
                spec->format = SDL_AUDIO_F32;  // might as well go straight to float...
                total_frames = hdr.data_size;
                adata.framesize = 1;
                break;
            case AU_ENC_LINEAR_8:
                spec->format = SDL_AUDIO_S8;
                total_frames = hdr.data_size;
                adata.framesize = 1;
                break;
            case AU_ENC_LINEAR_16:
                spec->format = SDL_AUDIO_S16BE;
                total_frames = hdr.data_size / 2;
                adata.framesize = 2;
                break;
            default:
                return SDL_SetError("AU: Unsupported .au encoding");
        }

        spec->freq = (int) hdr.sample_rate;
        spec->channels = (int) hdr.channels;

        adata.encoding = hdr.encoding;
        adata.framesize *= hdr.channels;
        total_frames /= hdr.channels;
        adata.start_offset = hdr.hdr_size;
    } else {
        // A number of files in the wild have the .au extension but no valid
        // header; these are traditionally assumed to be 8kHz mu-law. Handle
        // them here only if the extension is recognized or this decoder was
        // explicitly requested.
        bool assume_au_data = false;
        const char *decoder_name = SDL_GetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, NULL);
        if (decoder_name && (SDL_strcasecmp(decoder_name, "au") == 0)) {
            assume_au_data = true;
        } else {
            const char *origpath = SDL_GetStringProperty(props, MIX_PROP_AUDIO_LOAD_PATH_STRING, NULL);
            if (origpath) {
                const char *ext = SDL_strrchr(origpath, '.');
                if (ext) {
                    assume_au_data = (SDL_strcasecmp(ext + 1, "au") == 0);
                }
            }
        }

        if (!assume_au_data) {
            return SDL_SetError("AU: Not .au audio");
        }

        SDL_zero(hdr);

        spec->format = SDL_AUDIO_S16;
        spec->freq = 8000;
        spec->channels = 1;

        total_frames = SDL_GetIOSize(io);
        adata.start_offset = 0;
        adata.framesize = 1;
        adata.encoding = AU_ENC_ULAW_8;
    }

    // skip remaining part of header
    if (SDL_SeekIO(io, (Sint64) hdr.hdr_size, SDL_IO_SEEK_SET) < 0) {
        return false;
    }

    AU_AudioData *padata = (AU_AudioData *) SDL_calloc(1, sizeof (*padata));
    if (!padata) {
        return false;
    }
    SDL_copyp(padata, &adata);

    *audio_userdata = padata;
    *duration_frames = total_frames;

    return true;
}

static bool SDLCALL AU_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    const AU_AudioData *adata = (const AU_AudioData *) audio_userdata;
    AU_TrackData *tdata = (AU_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    tdata->adata = adata;
    tdata->io = io;

    *track_userdata = tdata;

    return true;
}

static bool SDLCALL AU_decode(void *track_userdata, SDL_AudioStream *stream)
{
    AU_TrackData *tdata = (AU_TrackData *) track_userdata;
    const int framesize = tdata->adata->framesize;

    #define MAX_SAMPS 512

    switch (tdata->adata->encoding) {
        case AU_ENC_ULAW_8: {
            float buffer[MAX_SAMPS];
            const int max_read = MAX_SAMPS - (MAX_SAMPS % framesize);
            Uint8 *ulaw_buf = (((Uint8 *) buffer) + sizeof (buffer)) - max_read;
            int br = (int) SDL_ReadIO(tdata->io, ulaw_buf, max_read);
            br -= (br % framesize);
            if (br == 0) {
                return false;  // nothing else to read.
            }
            for (int i = 0; i < br; i++) {
                buffer[i] = MIX_ulawToFloat[ulaw_buf[i]];
            }
            SDL_PutAudioStreamData(stream, buffer, br * sizeof (float));
            return true;
        }

        case AU_ENC_LINEAR_8:
        case AU_ENC_LINEAR_16: {
            Sint16 buffer[MAX_SAMPS];
            int max_read = MAX_SAMPS * ((tdata->adata->encoding == AU_ENC_LINEAR_16) ? 2 : 1);
            max_read -= (max_read % framesize);
            int br = (int) SDL_ReadIO(tdata->io, buffer, sizeof (buffer) - (sizeof (buffer) % framesize));
            br -= (br % framesize);
            if (br == 0) {
                return false;  // nothing else to read.
            }
            SDL_PutAudioStreamData(stream, buffer, br);
            return true;
        }

        default: break;
    }

    #undef MAX_SAMPS

    SDL_assert(!"Unexpected AU encoding!");
    return false;
}

static bool SDLCALL AU_seek(void *track_userdata, Uint64 frame)
{
    AU_TrackData *tdata = (AU_TrackData *) track_userdata;
    const AU_AudioData *adata = tdata->adata;
    return (SDL_SeekIO(tdata->io, adata->start_offset + (frame * adata->framesize), SDL_IO_SEEK_SET) != -1);
}

static void SDLCALL AU_quit_track(void *track_userdata)
{
    AU_TrackData *tdata = (AU_TrackData *) track_userdata;
    SDL_free(tdata);
}

static void SDLCALL AU_quit_audio(void *audio_userdata)
{
    AU_AudioData *adata = (AU_AudioData *) audio_userdata;
    SDL_free(adata);
}

const MIX_Decoder MIX_Decoder_AU = {
    "AU",
    NULL,  // init
    AU_init_audio,
    AU_init_track,
    AU_decode,
    AU_seek,
    NULL,  // jump_to_order
    AU_quit_track,
    AU_quit_audio,
    NULL  // quit
};

#endif

