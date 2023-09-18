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

#if defined(MUSIC_OGG) && !defined(OGG_USE_STB)

/* This file supports Ogg Vorbis music streams */

#include "SDL_loadso.h"

#include "music_ogg.h"
#include "utils.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#if defined(OGG_HEADER)
#include OGG_HEADER
#elif defined(OGG_USE_TREMOR)
#include <tremor/ivorbisfile.h>
#else
#include <vorbis/vorbisfile.h>
#endif


typedef struct {
    int loaded;
    void *handle;
    int (*ov_clear)(OggVorbis_File *vf);
    vorbis_info *(*ov_info)(OggVorbis_File *vf,int link);
    vorbis_comment *(*ov_comment)(OggVorbis_File *vf,int link);
    int (*ov_open_callbacks)(void *datasource, OggVorbis_File *vf, const char *initial, long ibytes, ov_callbacks callbacks);
    ogg_int64_t (*ov_pcm_total)(OggVorbis_File *vf,int i);
#ifdef OGG_USE_TREMOR
    long (*ov_read)(OggVorbis_File *vf,char *buffer,int length, int *bitstream);
    int (*ov_time_seek)(OggVorbis_File *vf,ogg_int64_t pos);
    ogg_int64_t (*ov_time_tell)(OggVorbis_File *vf);
    ogg_int64_t (*ov_time_total)(OggVorbis_File *vf, int i);
#else
    long (*ov_read)(OggVorbis_File *vf,char *buffer,int length, int bigendianp,int word,int sgned,int *bitstream);
    int (*ov_time_seek)(OggVorbis_File *vf,double pos);
    double (*ov_time_tell)(OggVorbis_File *vf);
    double (*ov_time_total)(OggVorbis_File *vf, int i);
#endif
    int (*ov_pcm_seek)(OggVorbis_File *vf, ogg_int64_t pos);
    ogg_int64_t (*ov_pcm_tell)(OggVorbis_File *vf);
} vorbis_loader;

static vorbis_loader vorbis;

#ifdef OGG_DYNAMIC
#define FUNCTION_LOADER(FUNC, SIG) \
    vorbis.FUNC = (SIG) SDL_LoadFunction(vorbis.handle, #FUNC); \
    if (vorbis.FUNC == NULL) { SDL_UnloadObject(vorbis.handle); return -1; }
#else
#define FUNCTION_LOADER(FUNC, SIG) \
    vorbis.FUNC = FUNC; \
    if (vorbis.FUNC == NULL) { Mix_SetError("Missing vorbis.framework or tremor.framework"); return -1; }
#endif

static int OGG_Load(void)
{
    if (vorbis.loaded == 0) {
#ifdef OGG_DYNAMIC
        vorbis.handle = SDL_LoadObject(OGG_DYNAMIC);
        if (vorbis.handle == NULL) {
            return -1;
        }
#endif
        FUNCTION_LOADER(ov_clear, int (*)(OggVorbis_File *))
        FUNCTION_LOADER(ov_info, vorbis_info *(*)(OggVorbis_File *,int))
        FUNCTION_LOADER(ov_comment, vorbis_comment *(*)(OggVorbis_File *,int))
        FUNCTION_LOADER(ov_open_callbacks, int (*)(void *,OggVorbis_File *,const char *,long,ov_callbacks))
        FUNCTION_LOADER(ov_pcm_total, ogg_int64_t (*)(OggVorbis_File *,int))
#ifdef OGG_USE_TREMOR
        FUNCTION_LOADER(ov_read, long (*)(OggVorbis_File *,char *,int,int *))
        FUNCTION_LOADER(ov_time_seek, int (*)(OggVorbis_File *,ogg_int64_t))
        FUNCTION_LOADER(ov_time_tell, ogg_int64_t (*)(OggVorbis_File *))
        FUNCTION_LOADER(ov_time_total, ogg_int64_t (*)(OggVorbis_File *, int))
#else
        FUNCTION_LOADER(ov_read, long (*)(OggVorbis_File *,char *,int,int,int,int,int *))
        FUNCTION_LOADER(ov_time_seek, int (*)(OggVorbis_File *,double))
        FUNCTION_LOADER(ov_time_tell, double (*)(OggVorbis_File *))
        FUNCTION_LOADER(ov_time_total, double (*)(OggVorbis_File *, int))
#endif
        FUNCTION_LOADER(ov_pcm_seek, int (*)(OggVorbis_File *,ogg_int64_t))
        FUNCTION_LOADER(ov_pcm_tell, ogg_int64_t (*)(OggVorbis_File *))
    }
    ++vorbis.loaded;

    return 0;
}

static void OGG_Unload(void)
{
    if (vorbis.loaded == 0) {
        return;
    }
    if (vorbis.loaded == 1) {
#ifdef OGG_DYNAMIC
        SDL_UnloadObject(vorbis.handle);
#endif
    }
    --vorbis.loaded;
}


typedef struct {
    SDL_RWops *src;
    int freesrc;
    int play_count;
    int volume;
    OggVorbis_File vf;
    vorbis_info vi;
    int section;
    SDL_AudioStream *stream;
    char *buffer;
    int buffer_size;
    int loop;
    ogg_int64_t loop_start;
    ogg_int64_t loop_end;
    ogg_int64_t loop_len;
    Mix_MusicMetaTags tags;
} OGG_music;


static int set_ov_error(const char *function, int error)
{
#define HANDLE_ERROR_CASE(X) case X: Mix_SetError("%s: %s", function, #X); break;
    switch (error) {
    HANDLE_ERROR_CASE(OV_FALSE)
    HANDLE_ERROR_CASE(OV_EOF)
    HANDLE_ERROR_CASE(OV_HOLE)
    HANDLE_ERROR_CASE(OV_EREAD)
    HANDLE_ERROR_CASE(OV_EFAULT)
    HANDLE_ERROR_CASE(OV_EIMPL)
    HANDLE_ERROR_CASE(OV_EINVAL)
    HANDLE_ERROR_CASE(OV_ENOTVORBIS)
    HANDLE_ERROR_CASE(OV_EBADHEADER)
    HANDLE_ERROR_CASE(OV_EVERSION)
    HANDLE_ERROR_CASE(OV_ENOTAUDIO)
    HANDLE_ERROR_CASE(OV_EBADPACKET)
    HANDLE_ERROR_CASE(OV_EBADLINK)
    HANDLE_ERROR_CASE(OV_ENOSEEK)
    default:
        Mix_SetError("%s: unknown error %d\n", function, error);
        break;
    }
    return -1;
}

static size_t sdl_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    return SDL_RWread((SDL_RWops*)datasource, ptr, size, nmemb);
}

static int sdl_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
    return (SDL_RWseek((SDL_RWops*)datasource, offset, whence) < 0)? -1 : 0;
}

static long sdl_tell_func(void *datasource)
{
    return (long)SDL_RWtell((SDL_RWops*)datasource);
}

static int OGG_Seek(void *context, double time);
static void OGG_Delete(void *context);

static int OGG_UpdateSection(OGG_music *music)
{
    vorbis_info *vi;

    vi = vorbis.ov_info(&music->vf, -1);
    if (!vi) {
        Mix_SetError("ov_info returned NULL");
        return -1;
    }

    if (vi->channels == music->vi.channels && vi->rate == music->vi.rate) {
        return 0;
    }
    SDL_memcpy(&music->vi, vi, sizeof(*vi));

    if (music->buffer) {
        SDL_free(music->buffer);
        music->buffer = NULL;
    }

    if (music->stream) {
        SDL_FreeAudioStream(music->stream);
        music->stream = NULL;
    }

    music->stream = SDL_NewAudioStream(AUDIO_S16SYS, (Uint8)vi->channels, (int)vi->rate,
                                       music_spec.format, music_spec.channels, music_spec.freq);
    if (!music->stream) {
        return -1;
    }

    music->buffer_size = music_spec.samples * (int)sizeof(Sint16) * vi->channels;
    music->buffer = (char *)SDL_malloc((size_t)music->buffer_size);
    if (!music->buffer) {
        return -1;
    }
    return 0;
}

/* Load an OGG stream from an SDL_RWops object */
static void *OGG_CreateFromRW(SDL_RWops *src, int freesrc)
{
    OGG_music *music;
    ov_callbacks callbacks;
    vorbis_comment *vc;
    long rate;
    ogg_int64_t full_length;
    SDL_bool is_loop_length = SDL_FALSE;
    int i;

    music = (OGG_music *)SDL_calloc(1, sizeof *music);
    if (!music) {
        SDL_OutOfMemory();
        return NULL;
    }
    music->src = src;
    music->volume = MIX_MAX_VOLUME;
    music->section = -1;

    SDL_zero(callbacks);
    callbacks.read_func = sdl_read_func;
    callbacks.seek_func = sdl_seek_func;
    callbacks.tell_func = sdl_tell_func;

    if (vorbis.ov_open_callbacks(src, &music->vf, NULL, 0, callbacks) < 0) {
        SDL_SetError("Not an Ogg Vorbis audio stream");
        SDL_free(music);
        return NULL;
    }

    if (OGG_UpdateSection(music) < 0) {
        OGG_Delete(music);
        return NULL;
    }

    rate = music->vi.rate;
    vc = vorbis.ov_comment(&music->vf, -1);
    if (vc != NULL) {
        for (i = 0; i < vc->comments; i++) {
            char *param = SDL_strdup(vc->user_comments[i]);
            char *argument = param;
            char *value = SDL_strchr(param, '=');
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
                music->loop_start = _Mix_ParseTime(value, rate);
            else if (SDL_strcasecmp(argument, "LOOPLENGTH") == 0) {
                music->loop_len = SDL_strtoll(value, NULL, 10);
                is_loop_length = SDL_TRUE;
            } else if (SDL_strcasecmp(argument, "LOOPEND") == 0) {
                music->loop_end = _Mix_ParseTime(value, rate);
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

    full_length = vorbis.ov_pcm_total(&music->vf, -1);
    if ((music->loop_end > 0) && (music->loop_end <= full_length) &&
        (music->loop_start < music->loop_end)) {
        music->loop = 1;
    }

    music->freesrc = freesrc;
    return music;
}

static const char* OGG_GetMetaTag(void *context, Mix_MusicMetaTag tag_type)
{
    OGG_music *music = (OGG_music *)context;
    return meta_tags_get(&music->tags, tag_type);
}

/* Set the volume for an OGG stream */
static void OGG_SetVolume(void *context, int volume)
{
    OGG_music *music = (OGG_music *)context;
    music->volume = volume;
}

/* Get the volume for an OGG stream */
static int OGG_GetVolume(void *context)
{
    OGG_music *music = (OGG_music *)context;
    return music->volume;
}

/* Start playback of a given OGG stream */
static int OGG_Play(void *context, int play_count)
{
    OGG_music *music = (OGG_music *)context;
    music->play_count = play_count;
    return OGG_Seek(music, 0.0);
}

static void OGG_Stop(void *context)
{
    OGG_music *music = (OGG_music *)context;
    SDL_AudioStreamClear(music->stream);
}

/* Play some of a stream previously started with OGG_play() */
static int OGG_GetSome(void *context, void *data, int bytes, SDL_bool *done)
{
    OGG_music *music = (OGG_music *)context;
    SDL_bool looped = SDL_FALSE;
    int filled, amount, result;
    int section;
    ogg_int64_t pcmPos;

    filled = SDL_AudioStreamGet(music->stream, data, bytes);
    if (filled != 0) {
        return filled;
    }

    if (!music->play_count) {
        /* All done */
        *done = SDL_TRUE;
        return 0;
    }

    section = music->section;
#ifdef OGG_USE_TREMOR
    amount = (int)vorbis.ov_read(&music->vf, music->buffer, music->buffer_size, &section);
#else
    amount = (int)vorbis.ov_read(&music->vf, music->buffer, music->buffer_size, SDL_BYTEORDER == SDL_BIG_ENDIAN, 2, 1, &section);
#endif
    if (amount < 0) {
        set_ov_error("ov_read", amount);
        return -1;
    }

    if (section != music->section) {
        music->section = section;
        if (OGG_UpdateSection(music) < 0) {
            return -1;
        }
    }

    pcmPos = vorbis.ov_pcm_tell(&music->vf);
    if (music->loop && (music->play_count != 1) && (pcmPos >= music->loop_end)) {
        amount -= (int)((pcmPos - music->loop_end) * music->vi.channels) * (int)sizeof(Sint16);
        result = vorbis.ov_pcm_seek(&music->vf, music->loop_start);
        if (result < 0) {
            set_ov_error("ov_pcm_seek", result);
            return -1;
        } else {
            int play_count = -1;
            if (music->play_count > 0) {
                play_count = (music->play_count - 1);
            }
            music->play_count = play_count;
        }
        looped = SDL_TRUE;
    }

    if (amount > 0) {
        if (SDL_AudioStreamPut(music->stream, music->buffer, amount) < 0) {
            return -1;
        }
    } else if (!looped) {
        if (music->play_count == 1) {
            music->play_count = 0;
            SDL_AudioStreamFlush(music->stream);
        } else {
            int play_count = -1;
            if (music->play_count > 0) {
                play_count = (music->play_count - 1);
            }
            if (OGG_Play(music, play_count) < 0) {
                return -1;
            }
        }
    }
    return 0;
}
static int OGG_GetAudio(void *context, void *data, int bytes)
{
    OGG_music *music = (OGG_music *)context;
    return music_pcm_getaudio(context, data, bytes, music->volume, OGG_GetSome);
}

/* Jump (seek) to a given position (time is in seconds) */
static int OGG_Seek(void *context, double time)
{
    OGG_music *music = (OGG_music *)context;
    int result;
#ifdef OGG_USE_TREMOR
    result = vorbis.ov_time_seek(&music->vf, (ogg_int64_t)(time * 1000.0));
#else
    result = vorbis.ov_time_seek(&music->vf, time);
#endif
    if (result < 0) {
        return set_ov_error("ov_time_seek", result);
    }
    return 0;
}

static double OGG_Tell(void *context)
{
    OGG_music *music = (OGG_music *)context;
#ifdef OGG_USE_TREMOR
    return vorbis.ov_time_tell(&music->vf) / 1000.0;
#else
    return vorbis.ov_time_tell(&music->vf);
#endif
}

/* Return music duration in seconds */
static double OGG_Duration(void *context)
{
    OGG_music *music = (OGG_music *)context;
#ifdef OGG_USE_TREMOR
    return vorbis.ov_time_total(&music->vf, -1) / 1000.0;
#else
    return vorbis.ov_time_total(&music->vf, -1);
#endif
}

static double   OGG_LoopStart(void *music_p)
{
    OGG_music *music = (OGG_music *)music_p;
    if (music->loop > 0) {
        return (double)music->loop_start / music->vi.rate;
    }
    return -1.0;
}

static double   OGG_LoopEnd(void *music_p)
{
    OGG_music *music = (OGG_music *)music_p;
    if (music->loop > 0) {
        return (double)music->loop_end / music->vi.rate;
    }
    return -1.0;
}

static double   OGG_LoopLength(void *music_p)
{
    OGG_music *music = (OGG_music *)music_p;
    if (music->loop > 0) {
        return (double)music->loop_len / music->vi.rate;
    }
    return -1.0;
}


/* Close the given OGG stream */
static void OGG_Delete(void *context)
{
    OGG_music *music = (OGG_music *)context;
    meta_tags_clear(&music->tags);
    vorbis.ov_clear(&music->vf);
    if (music->stream) {
        SDL_FreeAudioStream(music->stream);
    }
    if (music->buffer) {
        SDL_free(music->buffer);
    }
    if (music->freesrc) {
        SDL_RWclose(music->src);
    }
    SDL_free(music);
}

Mix_MusicInterface Mix_MusicInterface_OGG =
{
    "OGG",
    MIX_MUSIC_OGG,
    MUS_OGG,
    SDL_FALSE,
    SDL_FALSE,

    OGG_Load,
    NULL,   /* Open */
    OGG_CreateFromRW,
    NULL,   /* CreateFromFile */
    OGG_SetVolume,
    OGG_GetVolume,
    OGG_Play,
    NULL,   /* IsPlaying */
    OGG_GetAudio,
    NULL,   /* Jump */
    OGG_Seek,
    OGG_Tell,
    OGG_Duration,
    OGG_LoopStart,
    OGG_LoopEnd,
    OGG_LoopLength,
    OGG_GetMetaTag,   /* GetMetaTag */
    NULL,   /* Pause */
    NULL,   /* Resume */
    OGG_Stop,
    OGG_Delete,
    NULL,   /* Close */
    OGG_Unload
};

#endif /* MUSIC_OGG */

/* vi: set ts=4 sw=4 expandtab: */
