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

#ifdef DECODER_MP3_DRMP3

#include "SDL_mixer_internal.h"

#define DR_MP3_IMPLEMENTATION
#if defined(__GNUC__) && (__GNUC__ >= 4) && \
  !(defined(_WIN32) || defined(__EMX__))
#define DRMP3_API __attribute__((visibility("hidden")))
#elif defined(__APPLE__)
#define DRMP3_API __private_extern__
#else
#define DRMP3_API /* just in case.. */
#endif
#define DR_MP3_FLOAT_OUTPUT
#define DR_MP3_NO_STDIO
#define DRMP3_ASSERT(expression) SDL_assert((expression))
#define DRMP3_COPY_MEMORY(dst, src, sz) SDL_memcpy((dst), (src), (sz))
#define DRMP3_MOVE_MEMORY(dst, src, sz) SDL_memmove((dst), (src), (sz))
#define DRMP3_ZERO_MEMORY(p, sz) SDL_memset((p), 0, (sz))
#define DRMP3_MALLOC(sz) SDL_malloc((sz))
#define DRMP3_REALLOC(p, sz) SDL_realloc((p), (sz))
#define DRMP3_FREE(p) SDL_free((p))

// !!! FIXME: we need a DRMP3_NO_PARSE_METADATA_TAGS option to remove the ID3/APE checks, since we filtered them elsewhere.

#include "dr_libs/dr_mp3.h"

typedef struct DRMP3_AudioData
{
    size_t framesize;
    drmp3_seek_point *seek_points;
    drmp3_uint32 num_seek_points;
} DRMP3_AudioData;

typedef struct DRMP3_TrackData
{
    const DRMP3_AudioData *adata;
    drmp3 decoder;
} DRMP3_TrackData;


// drmp3 -> SDL_IOStream bridge...

static size_t DRMP3_IoRead(void *context, void *buf, size_t size)
{
    return SDL_ReadIO((SDL_IOStream *) context, buf, size);
}

static drmp3_bool32 DRMP3_IoSeek(void *context, int offset, drmp3_seek_origin origin)
{
    // SDL_IOWhence and drmp3_seek_origin happen to match up.
    return (SDL_SeekIO((SDL_IOStream *) context, offset, (SDL_IOWhence) origin) < 0) ? DRMP3_FALSE : DRMP3_TRUE;
}

static drmp3_bool32 DRMP3_IoTell(void *context, drmp3_int64 *pos)
{
    *pos = (drmp3_int64) SDL_TellIO((SDL_IOStream *) context);
    return (*pos < 0) ? DRMP3_FALSE : DRMP3_TRUE;
}


static bool SDLCALL DRMP3_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    drmp3 decoder;
    // do an initial load from the IOStream to get metadata.
    if (!drmp3_init(&decoder, DRMP3_IoRead, DRMP3_IoSeek, DRMP3_IoTell, NULL, io, NULL)) {
        return false;  // probably not an MP3 file.
    }

    DRMP3_AudioData *adata = (DRMP3_AudioData *) SDL_calloc(1, sizeof(*adata));
    if (!adata) {
        drmp3_uninit(&decoder);
        return false;
    }

    // I don't know if this is a great idea, as this is allegedly inefficient, but let's precalculate a seek table at load time, so each track can reuse it.
    // (If any of this fails, we go on without it.)
    drmp3_uint64 num_mp3_frames = 0;
    drmp3_uint64 num_pcm_frames = 0;
    if (drmp3_get_mp3_and_pcm_frame_count(&decoder, &num_mp3_frames, &num_pcm_frames)) {
        adata->num_seek_points = (drmp3_uint32) num_mp3_frames;
        adata->seek_points = (drmp3_seek_point *) SDL_calloc(num_mp3_frames, sizeof (*adata->seek_points));
        if (adata->seek_points) {
            if (drmp3_calculate_seek_points(&decoder, &adata->num_seek_points, adata->seek_points)) {
                // shrink the array if possible.
                if (adata->num_seek_points < ((drmp3_uint32) num_mp3_frames)) {
                    void *ptr = SDL_realloc(adata->seek_points, adata->num_seek_points * sizeof (*adata->seek_points));
                    if (ptr) {
                        adata->seek_points = (drmp3_seek_point *) ptr;
                    }
                }
            } else {  // failed, oh well. Live without.
                SDL_free(adata->seek_points);
                adata->seek_points = NULL;
                adata->num_seek_points = 0;
            }
        }
    }

    spec->format = SDL_AUDIO_F32;
    spec->channels = (int) decoder.channels;
    spec->freq = (int) decoder.sampleRate;

    drmp3_uninit(&decoder);

    adata->framesize = SDL_AUDIO_FRAMESIZE(*spec);

    *duration_frames = num_pcm_frames;
    *audio_userdata = adata;

    return true;
}

static bool SDLCALL DRMP3_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    const DRMP3_AudioData *adata = (const DRMP3_AudioData *) audio_userdata;
    DRMP3_TrackData *tdata = (DRMP3_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    if (!drmp3_init(&tdata->decoder, DRMP3_IoRead, DRMP3_IoSeek, DRMP3_IoTell, NULL, io, NULL)) {
        SDL_free(tdata);
        return false;
    }

    if (adata->seek_points) {
        drmp3_bind_seek_table(&tdata->decoder, adata->num_seek_points, adata->seek_points);
    }

    tdata->adata = adata;
    *track_userdata = tdata;

    return true;
}

static bool SDLCALL DRMP3_decode(void *track_userdata, SDL_AudioStream *stream)
{
    DRMP3_TrackData *tdata = (DRMP3_TrackData *) track_userdata;
    const int framesize = tdata->adata->framesize;
    float samples[256];
    const drmp3_uint64 rc = drmp3_read_pcm_frames_f32(&tdata->decoder, sizeof (samples) / framesize, samples);
    if (!rc) {
        return false;  // done decoding.
    }
    SDL_PutAudioStreamData(stream, samples, rc * framesize);
    return true;
}

static bool SDLCALL DRMP3_seek(void *track_userdata, Uint64 frame)
{
    DRMP3_TrackData *tdata = (DRMP3_TrackData *) track_userdata;
    return !!drmp3_seek_to_pcm_frame(&tdata->decoder, (drmp3_uint64) frame);
}

static void SDLCALL DRMP3_quit_track(void *track_userdata)
{
    DRMP3_TrackData *tdata = (DRMP3_TrackData *) track_userdata;
    drmp3_uninit(&tdata->decoder);
    SDL_free(tdata);
}

static void SDLCALL DRMP3_quit_audio(void *audio_userdata)
{
    DRMP3_AudioData *adata = (DRMP3_AudioData *) audio_userdata;
    SDL_free(adata->seek_points);
    SDL_free(adata);
}

const MIX_Decoder MIX_Decoder_DRMP3 = {
    "DRMP3",
    NULL,  // init
    DRMP3_init_audio,
    DRMP3_init_track,
    DRMP3_decode,
    DRMP3_seek,
    NULL,  // jump_to_order
    DRMP3_quit_track,
    DRMP3_quit_audio,
    NULL  // quit
};

#endif
