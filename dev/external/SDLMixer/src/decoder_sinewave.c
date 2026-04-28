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

// (this decoder is always enabled, since an external API uses it.)

// !!! FIXME:
// change track interface to provide the stream when seeking, then we could:
//   - Generate one whole iteration of the waveform upfront
//   - Push the same buffer twice with SDL_AudioStreamPutDataNoCopy
//   - decode just needs to see if one of the buffers is complete and push it again.
//   - clear/push a subset when seeking.

#include <stddef.h>
#include "SDL_mixer_internal.h"

typedef struct SINEWAVE_AudioData
{
    int hz;
    float amplitude;
    int sample_rate;
    Sint64 total_frames;
} SINEWAVE_AudioData;

typedef struct SINEWAVE_TrackData
{
    const SINEWAVE_AudioData *adata;
    int current_sine_sample;
    Sint64 position;
} SINEWAVE_TrackData;

static bool SDLCALL SINEWAVE_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    const char *decoder_name = SDL_GetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, NULL);
    if (!decoder_name || (SDL_strcasecmp(decoder_name, "sinewave") != 0)) {
        return false;
    }

    const Sint64 si64hz = SDL_GetNumberProperty(props, MIX_PROP_DECODER_SINEWAVE_HZ_NUMBER, -1);
    const float famp = SDL_GetFloatProperty(props, MIX_PROP_DECODER_SINEWAVE_AMPLITUDE_FLOAT, -1.0f);
    const Sint64 ms = SDL_GetNumberProperty(props, MIX_PROP_DECODER_SINEWAVE_MS_NUMBER, -1);

    if ((si64hz <= 0) || (famp <= 0.0f)) {
        return false;
    }

    spec->format = SDL_AUDIO_F32;
    spec->channels = 1;
    // we use the existing spec->freq to match the device sample rate, avoiding unnecessary resampling.

    SINEWAVE_AudioData *adata = (SINEWAVE_AudioData *) SDL_malloc(sizeof (*adata));
    if (!adata) {
        return false;
    }

    adata->hz = (int) si64hz;
    adata->amplitude = famp;
    adata->sample_rate = spec->freq;
    adata->total_frames = ms < 0 ? MIX_DURATION_INFINITE : MIX_MSToFrames(spec->freq, ms);

    *duration_frames = adata->total_frames;
    *audio_userdata = adata;

    return true;
}

static bool SDLCALL SINEWAVE_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    SINEWAVE_TrackData *tdata = (SINEWAVE_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    tdata->adata = (const SINEWAVE_AudioData *) audio_userdata;
    *track_userdata = tdata;

    return true;
}

static bool SDLCALL SINEWAVE_decode(void *track_userdata, SDL_AudioStream *stream)
{
    SINEWAVE_TrackData *tdata = (SINEWAVE_TrackData *) track_userdata;
    const SINEWAVE_AudioData *adata = tdata->adata;
    const int sample_rate = adata->sample_rate;
    const float fsample_rate = (float) sample_rate;
    const int hz = adata->hz;
    const float amplitude = adata->amplitude;
    int current_sine_sample = tdata->current_sine_sample;
    float samples[256];
    const bool infinite_sine = (adata->total_frames < 0);
    const ptrdiff_t total_frames = infinite_sine ? (int)SDL_arraysize(samples) : SDL_min(adata->total_frames - tdata->position, (int)SDL_arraysize(samples));

    if (total_frames <= 0) {
        return false;
    }

    for (ptrdiff_t i = 0; i < total_frames; i++) {
        const float phase = current_sine_sample * hz / fsample_rate;
        samples[i] = SDL_sinf(phase * 2.0f * SDL_PI_F) * amplitude;
        current_sine_sample++;
    }

    // wrapping around to avoid floating-point errors
    tdata->current_sine_sample = current_sine_sample % sample_rate;

    if (!infinite_sine) {
        tdata->position += total_frames;
    }

    SDL_PutAudioStreamData(stream, samples, sizeof (samples));

    return true;
}

static bool SDLCALL SINEWAVE_seek(void *track_userdata, Uint64 frame)
{
    SINEWAVE_TrackData *tdata = (SINEWAVE_TrackData *) track_userdata;
    const SINEWAVE_AudioData *adata = tdata->adata;
    if (adata->total_frames >= 0) {
        if (frame > (Uint64) adata->total_frames) {
            return SDL_SetError("Past end of sinewave");
        }
        tdata->position = frame;
    }
    tdata->current_sine_sample = frame % adata->sample_rate;
    return true;
}

static void SDLCALL SINEWAVE_quit_track(void *track_userdata)
{
    SDL_free(track_userdata);
}

static void SDLCALL SINEWAVE_quit_audio(void *audio_userdata)
{
    SDL_free(audio_userdata);
}

const MIX_Decoder MIX_Decoder_SINEWAVE = {
    "SINEWAVE",
    NULL,  // init
    SINEWAVE_init_audio,
    SINEWAVE_init_track,
    SINEWAVE_decode,
    SINEWAVE_seek,
    NULL,  // jump_to_order
    SINEWAVE_quit_track,
    SINEWAVE_quit_audio,
    NULL  // quit
};

