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

#ifdef MUSIC_MOD_XMP

#include "SDL_loadso.h"

#include "music_xmp.h"

#ifdef LIBXMP_HEADER
#include LIBXMP_HEADER
#else
#include <xmp.h>
#endif

/* libxmp >= 4.5.0 constified several funcs */
/* and also added load using file callbacks */
#if (XMP_VERCODE < 0x040500)
struct xmp_callbacks {
    unsigned long (*read_func)(void *, unsigned long, unsigned long, void *);
    int           (*seek_func)(void *, long, int);
    long          (*tell_func)(void *);
    int           (*close_func)(void*);
};
#define LIBXMP_CONST
#else
#define LIBXMP_CONST const
#endif

typedef struct {
    int loaded;
    void *handle;

    xmp_context (*xmp_create_context)(void);
    int (*xmp_load_module_from_memory)(xmp_context, LIBXMP_CONST void *, long);
    int (*xmp_load_module_from_callbacks)(xmp_context, void *, struct xmp_callbacks);
    int (*xmp_start_player)(xmp_context, int, int);
    void (*xmp_end_player)(xmp_context);
    void (*xmp_get_module_info)(xmp_context, struct xmp_module_info *);
    int (*xmp_play_buffer)(xmp_context, void *, int, int);
    int (*xmp_set_position)(xmp_context, int);
    int (*xmp_seek_time)(xmp_context, int);
    void (*xmp_get_frame_info)(xmp_context, struct xmp_frame_info *);
    void (*xmp_stop_module)(xmp_context);
    void (*xmp_release_module)(xmp_context);
    void (*xmp_free_context)(xmp_context);
} xmp_loader;

static xmp_loader libxmp;

#ifdef XMP_DYNAMIC
#define FUNCTION_LOADER(FUNC, SIG) \
    libxmp.FUNC = (SIG) SDL_LoadFunction(libxmp.handle, #FUNC); \
    if (libxmp.FUNC == NULL) { SDL_UnloadObject(libxmp.handle); return -1; }
#else
#define FUNCTION_LOADER(FUNC, SIG) \
    libxmp.FUNC = FUNC; \
    if (libxmp.FUNC == NULL) { Mix_SetError("Missing xmp.framework"); return -1; }
#endif

static int XMP_Load(void)
{
    if (libxmp.loaded == 0) {
#ifdef XMP_DYNAMIC
        libxmp.handle = SDL_LoadObject(XMP_DYNAMIC);
        if (libxmp.handle == NULL) {
            return -1;
        }
#endif
        FUNCTION_LOADER(xmp_create_context, xmp_context(*)(void))
        FUNCTION_LOADER(xmp_load_module_from_memory, int(*)(xmp_context,LIBXMP_CONST void *,long))
        FUNCTION_LOADER(xmp_start_player, int(*)(xmp_context,int,int))
        FUNCTION_LOADER(xmp_end_player, void(*)(xmp_context))
        FUNCTION_LOADER(xmp_get_module_info, void(*)(xmp_context,struct xmp_module_info*))
        FUNCTION_LOADER(xmp_play_buffer, int(*)(xmp_context,void*,int,int))
        FUNCTION_LOADER(xmp_set_position, int(*)(xmp_context,int))
        FUNCTION_LOADER(xmp_seek_time, int(*)(xmp_context,int))
        FUNCTION_LOADER(xmp_get_frame_info, void(*)(xmp_context,struct xmp_frame_info*))
        FUNCTION_LOADER(xmp_stop_module, void(*)(xmp_context))
        FUNCTION_LOADER(xmp_release_module, void(*)(xmp_context))
        FUNCTION_LOADER(xmp_free_context, void(*)(xmp_context))
#if defined(XMP_DYNAMIC)
        libxmp.xmp_load_module_from_callbacks = (int (*)(xmp_context,void*,struct xmp_callbacks)) SDL_LoadFunction(libxmp.handle, "xmp_load_module_from_callbacks");
#elif (XMP_VERCODE >= 0x040500)
        libxmp.xmp_load_module_from_callbacks = xmp_load_module_from_callbacks;
#else
        libxmp.xmp_load_module_from_callbacks = NULL;
#endif
    }
    ++libxmp.loaded;

    return 0;
}

static void XMP_Unload(void)
{
    if (libxmp.loaded == 0) {
        return;
    }
    if (libxmp.loaded == 1) {
#ifdef XMP_DYNAMIC
        SDL_UnloadObject(libxmp.handle);
#endif
    }
    --libxmp.loaded;
}


typedef struct
{
    int volume;
    int play_count;
    struct xmp_module_info mi;
    struct xmp_frame_info fi;
    xmp_context ctx;
    SDL_AudioStream *stream;
    void *buffer;
    int buffer_size;
    Mix_MusicMetaTags tags;
} XMP_Music;


static int XMP_Seek(void *ctx, double pos);
static void XMP_Delete(void *ctx);

static void libxmp_set_error(int e)
{
    const char *msg;
    switch (e) {
    case -XMP_ERROR_INTERNAL:
        msg = "Internal error in libxmp";
        break;
    case -XMP_ERROR_FORMAT:
        msg = "Unrecognized file format";
        break;
    case -XMP_ERROR_LOAD:
        msg = "Error loading file";
        break;
    case -XMP_ERROR_DEPACK:
        msg = "Error depacking file";
        break;
    case -XMP_ERROR_SYSTEM:
        msg = "System error in libxmp";
        break;
    case -XMP_ERROR_INVALID:
        msg = "Invalid parameter";
        break;
    case -XMP_ERROR_STATE:
        msg = "Invalid player state";
        break;
    default:
        msg = "Unknown error";
        break;
    }
    Mix_SetError("XMP: %s", msg);
}

static unsigned long xmp_fread(void *dst, unsigned long len, unsigned long nmemb, void *src) {
    return SDL_RWread((SDL_RWops*)src, dst, len, nmemb);
}
static int xmp_fseek(void *src, long offset, int whence) {
    return (SDL_RWseek((SDL_RWops*)src, offset, whence) < 0)? -1 : 0;
}
static long xmp_ftell(void *src) {
    return SDL_RWtell((SDL_RWops*)src);
}

/* Load a libxmp stream from an SDL_RWops object */
void *XMP_CreateFromRW(SDL_RWops *src, int freesrc)
{
    XMP_Music *music;
    struct xmp_callbacks file_callbacks = {
           xmp_fread, xmp_fseek, xmp_ftell, NULL
    };
    int err = 0;

    music = (XMP_Music *)SDL_calloc(1, sizeof(*music));
    if (!music) {
        SDL_OutOfMemory();
        return NULL;
    }

    music->ctx = libxmp.xmp_create_context();
    if (!music->ctx) {
        SDL_OutOfMemory();
        goto e0;
    }

    music->buffer_size = music_spec.samples * 2 * 2;
    music->buffer = SDL_malloc((size_t)music->buffer_size);
    if (!music->buffer) {
        SDL_OutOfMemory();
        goto e1;
    }

    if (libxmp.xmp_load_module_from_callbacks) {
        err = libxmp.xmp_load_module_from_callbacks(music->ctx, src, file_callbacks);
    } else {
        size_t size;
        void *mem = SDL_LoadFile_RW(src, &size, SDL_FALSE);
        if (!mem) {
            SDL_OutOfMemory();
            goto e1;
        }
        err = libxmp.xmp_load_module_from_memory(music->ctx, mem, (long)size);
        SDL_free(mem);
    }

    if (err < 0) {
        libxmp_set_error(err);
        goto e1;
    }

    err = libxmp.xmp_start_player(music->ctx, music_spec.freq, 0);
    if (err < 0) {
        libxmp_set_error(err);
        goto e2;
    }

    music->volume = MIX_MAX_VOLUME;
    music->stream = SDL_NewAudioStream(AUDIO_S16SYS, 2, music_spec.freq,
                                       music_spec.format, music_spec.channels, music_spec.freq);
    if (!music->stream) {
        goto e3;
    }

    meta_tags_init(&music->tags);
    libxmp.xmp_get_module_info(music->ctx, &music->mi);
    if (music->mi.mod->name[0]) {
        meta_tags_set(&music->tags, MIX_META_TITLE, music->mi.mod->name);
    }
    if (music->mi.comment) {
        meta_tags_set(&music->tags, MIX_META_COPYRIGHT, music->mi.comment);
    }

    if (freesrc) {
        SDL_RWclose(src);
    }
    return music;

e3: libxmp.xmp_end_player(music->ctx);
e2: libxmp.xmp_release_module(music->ctx);
e1: libxmp.xmp_free_context(music->ctx);
e0: SDL_free(music->buffer); SDL_free(music);
    return NULL;
}

/* Set the volume for a libxmp stream */
static void XMP_SetVolume(void *context, int volume)
{
    XMP_Music *music = (XMP_Music *)context;
    music->volume = volume;
}

/* Get the volume for a libxmp stream */
static int XMP_GetVolume(void *context)
{
    XMP_Music *music = (XMP_Music *)context;
    return music->volume;
}

/* Start playback of a given libxmp stream */
static int XMP_Play(void *context, int play_count)
{
    XMP_Music *music = (XMP_Music *)context;
    music->play_count = play_count;
    return XMP_Seek(music, 0);
}

/* Clean-up the output buffer */
static void XMP_Stop(void *context)
{
    XMP_Music *music = (XMP_Music *)context;
    SDL_AudioStreamClear(music->stream);
}

/* Play some of a stream previously started with xmp_play() */
static int XMP_GetSome(void *context, void *data, int bytes, SDL_bool *done)
{
    XMP_Music *music = (XMP_Music *)context;
    int filled, amount, ret;

    filled = SDL_AudioStreamGet(music->stream, data, bytes);
    if (filled != 0) {
        return filled;
    }

    if (!music->play_count) {
        /* All done */
        *done = SDL_TRUE;
        return 0;
    }

    /* if the data write is partial, rest of the buffer will be zero-filled.
     * the loop param is the max number that the current sequence of song
     * will be looped, or 0 to disable loop checking:  0 for play_count < 0
     * for an endless loop, or 1 for our own loop checks to do their job. */
    ret = libxmp.xmp_play_buffer(music->ctx, music->buffer, music->buffer_size, (music->play_count > 0));
    amount = music->buffer_size;

    if (ret == 0) {
        if (SDL_AudioStreamPut(music->stream, music->buffer, amount) < 0) {
            return -1;
        }
    } else {
        if (ret != -XMP_END) {
            return -1;
        }
        if (music->play_count == 1) {
            music->play_count = 0;
            SDL_AudioStreamFlush(music->stream);
        } else {
            int play_count = -1;
            if (music->play_count > 0) {
                play_count = (music->play_count - 1);
            }
            if (XMP_Play(music, play_count) < 0) {
                return -1;
            }
        }
    }
    return 0;
}
static int XMP_GetAudio(void *context, void *data, int bytes)
{
    XMP_Music *music = (XMP_Music *)context;
    return music_pcm_getaudio(context, data, bytes, music->volume, XMP_GetSome);
}

/* Jump to a given order */
static int XMP_Jump(void *context, int order)
{
    XMP_Music *music = (XMP_Music *)context;
    return libxmp.xmp_set_position(music->ctx, order);
}

/* Jump (seek) to a given position */
static int XMP_Seek(void *context, double pos)
{
    XMP_Music *music = (XMP_Music *)context;
    libxmp.xmp_seek_time(music->ctx, (int)(pos * 1000));
    libxmp.xmp_play_buffer(music->ctx, NULL, 0, 0); /* reset internal state. */
    return 0;
}

static double XMP_Tell(void *context)
{
    XMP_Music *music = (XMP_Music *)context;
    libxmp.xmp_get_frame_info(music->ctx, &music->fi);
    return music->fi.time / 1000.0;
}

static double XMP_Duration(void *context)
{
    XMP_Music *music = (XMP_Music *)context;
    libxmp.xmp_get_frame_info(music->ctx, &music->fi);
    return music->fi.total_time / 1000.0;
}

static const char* XMP_GetMetaTag(void *context, Mix_MusicMetaTag tag_type)
{
    XMP_Music *music = (XMP_Music *)context;
    return meta_tags_get(&music->tags, tag_type);
}

/* Close the given libxmp stream */
static void XMP_Delete(void *context)
{
    XMP_Music *music = (XMP_Music *)context;
    meta_tags_clear(&music->tags);
    if (music->ctx) {
        libxmp.xmp_stop_module(music->ctx);
        libxmp.xmp_end_player(music->ctx);
        libxmp.xmp_release_module(music->ctx);
        libxmp.xmp_free_context(music->ctx);
    }
    if (music->stream) {
        SDL_FreeAudioStream(music->stream);
    }
    if (music->buffer) {
        SDL_free(music->buffer);
    }
    SDL_free(music);
}

Mix_MusicInterface Mix_MusicInterface_XMP =
{
    "XMP",
    MIX_MUSIC_LIBXMP,
    MUS_MOD,
    SDL_FALSE,
    SDL_FALSE,

    XMP_Load,
    NULL,   /* Open */
    XMP_CreateFromRW,
    NULL,   /* CreateFromFile */
    XMP_SetVolume,
    XMP_GetVolume,
    XMP_Play,
    NULL,   /* IsPlaying */
    XMP_GetAudio,
    XMP_Jump,
    XMP_Seek,
    XMP_Tell,
    XMP_Duration,
    NULL,   /* LoopStart */
    NULL,   /* LoopEnd */
    NULL,   /* LoopLength */
    XMP_GetMetaTag,
    NULL,   /* Pause */
    NULL,   /* Resume */
    XMP_Stop,
    XMP_Delete,
    NULL,   /* Close */
    XMP_Unload
};

#endif /* MUSIC_MOD_XMP */

/* vi: set ts=4 sw=4 expandtab: */
