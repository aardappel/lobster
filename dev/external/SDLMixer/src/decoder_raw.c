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

// (this decoder is always enabled, as it is used internally.)

#include "SDL_mixer_internal.h"

// !!! FIXME: change track interface to provide the stream when seeking, and a means to see if we're backed by a memory SDL_IOStream, then we could use SDL_AudioStreamPutDataNoCopy to push the whole buffer upfront for free, and clear/push a subset when seeking.

typedef struct RAW_TrackData
{
    SDL_IOStream *io;
    int framesize;
    const Uint8 *const_data;
    size_t const_datalen;
    Sint64 position;
} RAW_TrackData;

static bool SDLCALL RAW_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    const char *decoder_name = SDL_GetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, NULL);
    if (!decoder_name || (SDL_strcasecmp(decoder_name, "raw") != 0)) {
        return false;
    }

    const Sint64 si64fmt = SDL_GetNumberProperty(props, MIX_PROP_DECODER_FORMAT_NUMBER, -1);
    const Sint64 si64channels = SDL_GetNumberProperty(props, MIX_PROP_DECODER_CHANNELS_NUMBER, -1);
    const Sint64 si64freq = SDL_GetNumberProperty(props, MIX_PROP_DECODER_FREQ_NUMBER, -1);

    if ((si64fmt <= 0) || (si64channels <= 0) || (si64freq <= 0)) {
        return SDL_SetError("Requested RAW decoder but didn't provide PCM format information");
    }

    spec->format = (SDL_AudioFormat) si64fmt;
    spec->channels = (int) si64channels;
    spec->freq = (int) si64freq;

    // we don't have to inspect the data, we treat anything as valid.

    const Sint64 framesize = (Sint64) (SDL_AUDIO_FRAMESIZE(*spec));
    const Sint64 iolen = SDL_GetIOSize(io);

    *duration_frames = (iolen >= 0) ? (iolen / framesize) : MIX_DURATION_UNKNOWN;
    *audio_userdata = NULL;  // no state.

    return true;
}

static bool SDLCALL RAW_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    SDL_assert(audio_userdata == NULL);  // no state.
    RAW_TrackData *tdata = (RAW_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    tdata->io = io;
    tdata->framesize = (int) (SDL_AUDIO_FRAMESIZE(*spec));
    tdata->const_data = (const Uint8 *) MIX_GetConstIOBuffer(tdata->io, &tdata->const_datalen);

    *track_userdata = tdata;

    return true;
}

static bool SDLCALL RAW_decode(void *track_userdata, SDL_AudioStream *stream)
{
    RAW_TrackData *tdata = (RAW_TrackData *) track_userdata;
    Uint8 buffer[256];

    if (tdata->const_data) {
        size_t readlen = (size_t) (((Sint64) tdata->const_datalen) - tdata->position);
        readlen = SDL_min(sizeof (buffer), readlen);
        readlen -= (readlen % tdata->framesize);
        if (readlen == 0) {
            return false;  // nothing else to read.
        }
        SDL_PutAudioStreamDataNoCopy(stream, tdata->const_data + tdata->position, (int) readlen, NULL, NULL);
        tdata->position += readlen;
    } else {
        const size_t readlen = sizeof (buffer) - (sizeof (buffer) % tdata->framesize);
        size_t br = SDL_ReadIO(tdata->io, buffer, readlen);
        br -= (br % tdata->framesize);
        if (br == 0) {
            return false;  // eof or error, can't supply more data.
        }
        SDL_PutAudioStreamData(stream, buffer, (int) br);
        tdata->position += br;
    }

    return true;
}

static bool SDLCALL RAW_seek(void *track_userdata, Uint64 frame)
{
    RAW_TrackData *tdata = (RAW_TrackData *) track_userdata;
    const Sint64 offset = (Sint64) (frame * tdata->framesize);
    if (SDL_SeekIO(tdata->io, offset, SDL_IO_SEEK_SET) != offset) {
        return false;
    }
    tdata->position = offset;
    return true;
}

static void SDLCALL RAW_quit_track(void *track_userdata)
{
    SDL_free(track_userdata);
}

static void SDLCALL RAW_quit_audio(void *audio_userdata)
{
    SDL_assert(audio_userdata == NULL);  // no state.
}

const MIX_Decoder MIX_Decoder_RAW = {
    "RAW",
    NULL,  // init
    RAW_init_audio,
    RAW_init_track,
    RAW_decode,
    RAW_seek,
    NULL,  // jump_to_order
    RAW_quit_track,
    RAW_quit_audio,
    NULL  // quit
};

