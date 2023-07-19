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

#ifdef MUSIC_FLAC_DRFLAC

#include "music_drflac.h"
#include "mp3utils.h"
#include "../utils.h"

#include "SDL.h"

#define DR_FLAC_IMPLEMENTATION
#if defined(__GNUC__) && (__GNUC__ >= 4) && \
  !(defined(_WIN32) || defined(__EMX__))
#define DRFLAC_API __attribute__((visibility("hidden")))
#elif defined(__APPLE__)
#define DRFLAC_API __private_extern__
#else
#define DRFLAC_API /* just in case.. */
#endif
#define DR_FLAC_NO_STDIO
#define DRFLAC_ASSERT(expression)
#define DRFLAC_COPY_MEMORY(dst, src, sz) SDL_memcpy((dst), (src), (sz))
#define DRFLAC_MOVE_MEMORY(dst, src, sz) SDL_memmove((dst), (src), (sz))
#define DRFLAC_ZERO_MEMORY(p, sz) SDL_memset((p), 0, (sz))
#define DRFLAC_MALLOC(sz) SDL_malloc((sz))
#define DRFLAC_REALLOC(p, sz) SDL_realloc((p), (sz))
#define DRFLAC_FREE(p) SDL_free((p))
#include "dr_libs/dr_flac.h"


typedef struct {
    struct mp3file_t file;
    drflac *dec;
    int play_count;
    int freesrc;
    int volume;
    int status;
    int sample_rate;
    int channels;
    SDL_AudioStream *stream;
    drflac_int16 *buffer;
    int buffer_size;
    int loop;
    SDL_bool loop_flag;
    Sint64 loop_start;
    Sint64 loop_end;
    Sint64 loop_len;
    Mix_MusicMetaTags tags;
} DRFLAC_Music;


static size_t DRFLAC_ReadCB(void *context, void *buf, size_t size)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    return MP3_RWread(&music->file, buf, 1, size);
}

static drflac_bool32 DRFLAC_SeekCB(void *context, int offset, drflac_seek_origin origin)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    int whence = (origin == drflac_seek_origin_start) ? RW_SEEK_SET : RW_SEEK_CUR;
    if (MP3_RWseek(&music->file, offset, whence) < 0) {
        return DRFLAC_FALSE;
    }
    return DRFLAC_TRUE;
}

static void DRFLAC_MetaCB(void *context, drflac_metadata *metadata)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;

    if (metadata->type == DRFLAC_METADATA_BLOCK_TYPE_STREAMINFO) {
        music->sample_rate = metadata->data.streaminfo.sampleRate;
        music->channels = metadata->data.streaminfo.channels;
    } else if (metadata->type == DRFLAC_METADATA_BLOCK_TYPE_VORBIS_COMMENT) {
        drflac_uint32 i;
        char *param, *argument, *value;
        SDL_bool is_loop_length = SDL_FALSE;
        const char *pRunningData = (const char *)metadata->data.vorbis_comment.pComments;

        for (i = 0; i < metadata->data.vorbis_comment.commentCount; ++i) {
            drflac_uint32 commentLength = drflac__le2host_32_ptr_unaligned(pRunningData); pRunningData += 4;

            param = (char *)SDL_malloc(commentLength + 1);
            if (param) {
                SDL_memcpy(param, pRunningData, commentLength);
                param[commentLength] = '\0';
                argument = param;
                value = SDL_strchr(param, '=');

                if (value == NULL) {
                    value = param + SDL_strlen(param);
                } else {
                    *(value++) = '\0';
                }

                /* Want to match LOOP-START, LOOP_START, etc. Remove - or _ from
                 * string if it is present at position 4. */
                if (_Mix_IsLoopTag(argument) && ((argument[4] == '_') || (argument[4] == '-'))) {
                    SDL_memmove(argument + 4, argument + 5, SDL_strlen(argument) - 4);
                }

                if (SDL_strcasecmp(argument, "LOOPSTART") == 0)
                    music->loop_start = _Mix_ParseTime(value, music->sample_rate);
                else if (SDL_strcasecmp(argument, "LOOPLENGTH") == 0) {
                    music->loop_len = SDL_strtoll(value, NULL, 10);
                    is_loop_length = SDL_TRUE;
                } else if (SDL_strcasecmp(argument, "LOOPEND") == 0) {
                    music->loop_end = _Mix_ParseTime(value, music->sample_rate);
                    is_loop_length = SDL_FALSE;
                } else if (SDL_strcasecmp(argument, "TITLE") == 0) {
                    meta_tags_set(&music->tags, MIX_META_TITLE, value);
                } else if (SDL_strcasecmp(argument, "ARTIST") == 0) {
                    meta_tags_set(&music->tags, MIX_META_ARTIST, value);
                } else if (SDL_strcasecmp(argument, "ALBUM") == 0) {
                    meta_tags_set(&music->tags, MIX_META_ALBUM, value);
                } else if (SDL_strcasecmp(argument, "COPYRIGHT") == 0) {
                    meta_tags_set(&music->tags, MIX_META_COPYRIGHT, value);
                }
                SDL_free(param);
            }
            pRunningData += commentLength;
        }

        if (is_loop_length) {
            music->loop_end = music->loop_start + music->loop_len;
        } else {
            music->loop_len = music->loop_end - music->loop_start;
        }

        /* Ignore invalid loop tag */
        if (music->loop_start < 0 || music->loop_len < 0 || music->loop_end < 0) {
            music->loop_start = 0;
            music->loop_len = 0;
            music->loop_end = 0;
        }
    }
}

static int DRFLAC_Seek(void *context, double position);

static void *DRFLAC_CreateFromRW(SDL_RWops *src, int freesrc)
{
    DRFLAC_Music *music;

    music = (DRFLAC_Music *)SDL_calloc(1, sizeof(DRFLAC_Music));
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

    music->dec = drflac_open_with_metadata(DRFLAC_ReadCB, DRFLAC_SeekCB, DRFLAC_MetaCB, music, NULL);
    if (!music->dec) {
        SDL_free(music);
        Mix_SetError("music_drflac: corrupt flac file (bad stream).");
        return NULL;
    }

    /* We should have channels and sample rate set up here */
    music->stream = SDL_NewAudioStream(AUDIO_S16SYS,
                                       (Uint8)music->channels,
                                       music->sample_rate,
                                       music_spec.format,
                                       music_spec.channels,
                                       music_spec.freq);
    if (!music->stream) {
        SDL_OutOfMemory();
        drflac_close(music->dec);
        SDL_free(music);
        return NULL;
    }

    music->buffer_size = music_spec.samples * sizeof(drflac_int16) * music->channels;
    music->buffer = (drflac_int16*)SDL_calloc(1, music->buffer_size);
    if (!music->buffer) {
        drflac_close(music->dec);
        SDL_OutOfMemory();
        SDL_free(music);
        return NULL;
    }

    /* loop_start, loop_end and loop_len get set by metadata callback if tags
     * are present in metadata.
     */
    if ((music->loop_end > 0) && (music->loop_end <= (Sint64)music->dec->totalPCMFrameCount) &&
        (music->loop_start < music->loop_end)) {
        music->loop = 1;
    }

    music->freesrc = freesrc;
    return music;
}

static void DRFLAC_SetVolume(void *context, int volume)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    music->volume = volume;
}

static int DRFLAC_GetVolume(void *context)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    return music->volume;
}

/* Starts the playback. */
static int DRFLAC_Play(void *context, int play_count)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    music->play_count = play_count;
    return DRFLAC_Seek(music, 0.0);
}

static void DRFLAC_Stop(void *context)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    SDL_AudioStreamClear(music->stream);
}

static int DRFLAC_GetSome(void *context, void *data, int bytes, SDL_bool *done)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    int filled;
    drflac_uint64 amount;

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

    if (music->loop_flag) {
        if (!drflac_seek_to_pcm_frame(music->dec, music->loop_start)) {
            SDL_SetError("drflac_seek_to_pcm_frame() failed");
            return -1;
        } else {
            int play_count = -1;
            if (music->play_count > 0) {
                play_count = (music->play_count - 1);
            }
            music->play_count = play_count;
            music->loop_flag = SDL_FALSE;
        }
    }

    amount = drflac_read_pcm_frames_s16(music->dec, music_spec.samples, music->buffer);
    if (amount > 0) {
        if (music->loop && (music->play_count != 1) &&
            ((Sint64)music->dec->currentPCMFrame >= music->loop_end)) {
            amount -= (music->dec->currentPCMFrame - music->loop_end) * sizeof(drflac_int16) * music->channels;
            music->loop_flag = SDL_TRUE;
        }
        if (SDL_AudioStreamPut(music->stream, music->buffer, (int)amount * sizeof(drflac_int16) * music->channels) < 0) {
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
            if (DRFLAC_Play(music, play_count) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

static int DRFLAC_GetAudio(void *context, void *data, int bytes)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    return music_pcm_getaudio(context, data, bytes, music->volume, DRFLAC_GetSome);
}

static int DRFLAC_Seek(void *context, double position)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    drflac_uint64 destpos = (drflac_uint64)(position * music->sample_rate);
    drflac_seek_to_pcm_frame(music->dec, destpos);
    return 0;
}

static double DRFLAC_Tell(void *context)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    return (double)music->dec->currentPCMFrame / music->sample_rate;
}

static double DRFLAC_Duration(void *context)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    drflac_uint64 samples = music->dec->totalPCMFrameCount;
    return (double)samples / music->sample_rate;
}

static double DRFLAC_LoopStart(void *context)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    if (music->loop > 0) {
        return (double)music->loop_start / music->sample_rate;
    }
    return -1.0;
}

static double DRFLAC_LoopEnd(void *context)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    if (music->loop > 0) {
        return (double)music->loop_end / music->sample_rate;
    }
    return -1.0;
}

static double DRFLAC_LoopLength(void *context)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    if (music->loop > 0) {
        return (double)music->loop_len / music->sample_rate;
    }
    return -1.0;
}

static const char* DRFLAC_GetMetaTag(void *context, Mix_MusicMetaTag tag_type)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;
    return meta_tags_get(&music->tags, tag_type);
}

static void DRFLAC_Delete(void *context)
{
    DRFLAC_Music *music = (DRFLAC_Music *)context;

    drflac_close(music->dec);
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

Mix_MusicInterface Mix_MusicInterface_DRFLAC =
{
    "DRFLAC",
    MIX_MUSIC_DRFLAC,
    MUS_FLAC,
    SDL_FALSE,
    SDL_FALSE,

    NULL,   /* Load */
    NULL,   /* Open */
    DRFLAC_CreateFromRW,
    NULL,   /* CreateFromFile */
    DRFLAC_SetVolume,
    DRFLAC_GetVolume,
    DRFLAC_Play,
    NULL,   /* IsPlaying */
    DRFLAC_GetAudio,
    NULL,   /* Jump */
    DRFLAC_Seek,
    DRFLAC_Tell,
    DRFLAC_Duration,
    DRFLAC_LoopStart,
    DRFLAC_LoopEnd,
    DRFLAC_LoopLength,
    DRFLAC_GetMetaTag,
    NULL,   /* Pause */
    NULL,   /* Resume */
    DRFLAC_Stop,
    DRFLAC_Delete,
    NULL,   /* Close */
    NULL    /* Unload */
};

#endif /* MUSIC_FLAC_DRFLAC */

/* vi: set ts=4 sw=4 expandtab: */
