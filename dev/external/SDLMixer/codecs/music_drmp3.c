/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

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

#ifdef MUSIC_MP3_DRMP3

#include "music_drmp3.h"
#include "mp3utils.h"
#include "SDL.h"

#define DR_MP3_IMPLEMENTATION
#if defined(__GNUC__) && (__GNUC__ >= 4) && \
  !(defined(_WIN32) || defined(__EMX__))
#define DRMP3_API __attribute__((visibility("hidden")))
#elif defined(__APPLE__)
#define DRMP3_API __private_extern__
#else
#define DRMP3_API /* just in case.. */
#endif
#define DR_MP3_NO_STDIO
#define DRMP3_ASSERT(expression)
#define DRMP3_COPY_MEMORY(dst, src, sz) SDL_memcpy((dst), (src), (sz))
#define DRMP3_MOVE_MEMORY(dst, src, sz) SDL_memmove((dst), (src), (sz))
#define DRMP3_ZERO_MEMORY(p, sz) SDL_memset((p), 0, (sz))
#define DRMP3_MALLOC(sz) SDL_malloc((sz))
#define DRMP3_REALLOC(p, sz) SDL_realloc((p), (sz))
#define DRMP3_FREE(p) SDL_free((p))
#include "dr_libs/dr_mp3.h"


typedef struct {
    struct mp3file_t file;
    drmp3 dec;
    int play_count;
    int freesrc;
    int volume;
    int status;
    SDL_AudioStream *stream;
    drmp3_int16 *buffer;
    int buffer_size;
    int channels;

    Mix_MusicMetaTags tags;
} DRMP3_Music;


static size_t DRMP3_ReadCB(void *context, void *buf, size_t size)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    return MP3_RWread(&music->file, buf, 1, size);
}

static drmp3_bool32 DRMP3_SeekCB(void *context, int offset, drmp3_seek_origin origin)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    int whence = (origin == drmp3_seek_origin_start) ? RW_SEEK_SET : RW_SEEK_CUR;
    if (MP3_RWseek(&music->file, offset, whence) < 0) {
        return DRMP3_FALSE;
    }
    return DRMP3_TRUE;
}

static int DRMP3_Seek(void *context, double position);

static void *DRMP3_CreateFromRW(SDL_RWops *src, int freesrc)
{
    DRMP3_Music *music;

    music = (DRMP3_Music *)SDL_calloc(1, sizeof(DRMP3_Music));
    if (!music) {
        SDL_OutOfMemory();
        return NULL;
    }
    music->volume = MIX_MAX_VOLUME;

    if (MP3_RWinit(&music->file, src) < 0) {
        SDL_free(music);
        return NULL;
    }

    meta_tags_init(&music->tags);
    if (mp3_read_tags(&music->tags, &music->file, SDL_FALSE) < 0) {
        SDL_free(music);
        Mix_SetError("music_drmp3: corrupt mp3 file (bad tags).");
        return NULL;
    }

    MP3_RWseek(&music->file, 0, RW_SEEK_SET);

    if (!drmp3_init(&music->dec, DRMP3_ReadCB, DRMP3_SeekCB, music, NULL)) {
        SDL_free(music);
        Mix_SetError("music_drmp3: corrupt mp3 file (bad stream).");
        return NULL;
    }

    music->channels = music->dec.channels;
    music->stream = SDL_NewAudioStream(AUDIO_S16SYS,
                                       (Uint8)music->channels,
                                       (int)music->dec.sampleRate,
                                       music_spec.format,
                                       music_spec.channels,
                                       music_spec.freq);
    if (!music->stream) {
        SDL_OutOfMemory();
        drmp3_uninit(&music->dec);
        SDL_free(music);
        return NULL;
    }

    music->buffer_size = music_spec.samples * sizeof(drmp3_int16) * music->channels;
    music->buffer = (drmp3_int16*)SDL_calloc(1, music->buffer_size);
    if (!music->buffer) {
        drmp3_uninit(&music->dec);
        SDL_OutOfMemory();
        SDL_free(music);
        return NULL;
    }

    music->freesrc = freesrc;
    return music;
}

static void DRMP3_SetVolume(void *context, int volume)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    music->volume = volume;
}

static int DRMP3_GetVolume(void *context)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    return music->volume;
}

/* Starts the playback. */
static int DRMP3_Play(void *context, int play_count)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    music->play_count = play_count;
    return DRMP3_Seek(music, 0.0);
}

static void DRMP3_Stop(void *context)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    SDL_AudioStreamClear(music->stream);
}

static int DRMP3_GetSome(void *context, void *data, int bytes, SDL_bool *done)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    int filled;
    drmp3_uint64 amount;

    if (music->stream) {
        filled = SDL_AudioStreamGet(music->stream, data, bytes);
        if (filled != 0) {
            return filled;
        }
    }

    if (!music->play_count) {
        /* All done */
        *done = SDL_TRUE;
        return 0;
    }

    amount = drmp3_read_pcm_frames_s16(&music->dec, music_spec.samples, music->buffer);
    if (amount > 0) {
        if (SDL_AudioStreamPut(music->stream, music->buffer, (int)amount * sizeof(drmp3_int16) * music->channels) < 0) {
            return -1;
        }
    } else {
        if (music->play_count == 1) {
            music->play_count = 0;
            SDL_AudioStreamFlush(music->stream);
        } else {
            int play_count = -1;
            if (music->play_count > 0) {
                play_count = (music->play_count - 1);
            }
            if (DRMP3_Play(music, play_count) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

static int DRMP3_GetAudio(void *context, void *data, int bytes)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    return music_pcm_getaudio(context, data, bytes, music->volume, DRMP3_GetSome);
}

static int DRMP3_Seek(void *context, double position)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    drmp3_uint64 destpos = (drmp3_uint64)(position * music->dec.sampleRate);
    drmp3_seek_to_pcm_frame(&music->dec, destpos);
    return 0;
}

static double DRMP3_Tell(void *context)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    return (double)music->dec.currentPCMFrame / music->dec.sampleRate;
}

static double DRMP3_Duration(void *context)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    drmp3_uint64 samples = drmp3_get_pcm_frame_count(&music->dec);
    return (double)samples / music->dec.sampleRate;
}

static const char* DRMP3_GetMetaTag(void *context, Mix_MusicMetaTag tag_type)
{
    DRMP3_Music *music = (DRMP3_Music *)context;
    return meta_tags_get(&music->tags, tag_type);
}

static void DRMP3_Delete(void *context)
{
    DRMP3_Music *music = (DRMP3_Music *)context;

    drmp3_uninit(&music->dec);
    meta_tags_clear(&music->tags);

    if (music->stream) {
        SDL_FreeAudioStream(music->stream);
    }
    if (music->buffer) {
        SDL_free(music->buffer);
    }
    if (music->freesrc) {
        SDL_RWclose(music->file.src);
    }
    SDL_free(music);
}

Mix_MusicInterface Mix_MusicInterface_DRMP3 =
{
    "DRMP3",
    MIX_MUSIC_DRMP3,
    MUS_MP3,
    SDL_FALSE,
    SDL_FALSE,

    NULL,   /* Load */
    NULL,   /* Open */
    DRMP3_CreateFromRW,
    NULL,   /* CreateFromFile */
    DRMP3_SetVolume,
    DRMP3_GetVolume,
    DRMP3_Play,
    NULL,   /* IsPlaying */
    DRMP3_GetAudio,
    NULL,   /* Jump */
    DRMP3_Seek,
    DRMP3_Tell,
    DRMP3_Duration,
    NULL,   /* LoopStart */
    NULL,   /* LoopEnd */
    NULL,   /* LoopLength */
    DRMP3_GetMetaTag,
    NULL,   /* Pause */
    NULL,   /* Resume */
    DRMP3_Stop,
    DRMP3_Delete,
    NULL,   /* Close */
    NULL    /* Unload */
};

#endif /* MUSIC_MP3_DRMP3 */

/* vi: set ts=4 sw=4 expandtab: */
