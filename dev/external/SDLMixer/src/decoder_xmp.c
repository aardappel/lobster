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

#ifdef DECODER_MOD_XMP

#include "SDL_mixer_internal.h"

#if defined(XMP_DYNAMIC) && defined(SDL_ELF_NOTE_DLOPEN)
SDL_ELF_NOTE_DLOPEN(
    "midi-xmp",
    "Support for MIDI audio using libxmp",
    SDL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
    XMP_DYNAMIC
)
#endif

#ifdef LIBXMP_HEADER
#include LIBXMP_HEADER
#else
#include <xmp.h>
#endif

// libxmp >= 4.5.0 constified several funcs and also added load using file callbacks
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

#ifdef XMP_DYNAMIC
#define MIX_LOADER_DYNAMIC XMP_DYNAMIC
#endif

#define MIX_LOADER_FUNCTIONS \
    MIX_LOADER_FUNCTION(true,xmp_context,xmp_create_context,(void)) \
    MIX_LOADER_FUNCTION(false,int,xmp_test_module_from_callbacks,(void *, struct xmp_callbacks, struct xmp_test_info *)) \
    MIX_LOADER_FUNCTION(true,int,xmp_load_module_from_callbacks,(xmp_context, void *, struct xmp_callbacks)) \
    MIX_LOADER_FUNCTION(true,int,xmp_start_player,(xmp_context, int, int)) \
    MIX_LOADER_FUNCTION(true,void,xmp_end_player,(xmp_context)) \
    MIX_LOADER_FUNCTION(true,void,xmp_get_module_info,(xmp_context, struct xmp_module_info *)) \
    MIX_LOADER_FUNCTION(true,int,xmp_play_frame,(xmp_context)) \
    MIX_LOADER_FUNCTION(true,int,xmp_play_buffer,(xmp_context, void *, int, int)) \
    MIX_LOADER_FUNCTION(true,int,xmp_seek_time,(xmp_context, int)) \
    MIX_LOADER_FUNCTION(true,int,xmp_set_position,(xmp_context, int)) \
    MIX_LOADER_FUNCTION(true,void,xmp_get_frame_info,(xmp_context, struct xmp_frame_info *)) \
    MIX_LOADER_FUNCTION(true,void,xmp_stop_module,(xmp_context)) \
    MIX_LOADER_FUNCTION(true,void,xmp_release_module,(xmp_context)) \
    MIX_LOADER_FUNCTION(true,void,xmp_free_context,(xmp_context)) \

#define MIX_LOADER_MODULE libxmp
#include "SDL_mixer_loader.h"


static unsigned long XMP_IoRead(void *dst, unsigned long len, unsigned long nmemb, void *src)
{
    return ((len > 0) && (nmemb > 0)) ? (SDL_ReadIO((SDL_IOStream *) src, dst, len * nmemb) / len) : 0;
}

static int XMP_IoSeek(void *src, long offset, int whence)
{
    return (SDL_SeekIO((SDL_IOStream *) src, (Sint64) offset, whence) < 0) ? -1 : 0;
}

static long XMP_IoTell(void *src)
{
    return (long)SDL_TellIO((SDL_IOStream *) src);
}

static const struct xmp_callbacks XMP_IoCallbacks = { XMP_IoRead, XMP_IoSeek, XMP_IoTell, NULL };


static bool SetLibXmpError(const char *function, int error)
{
    switch (error) {
        #define HANDLE_ERROR_CASE(X) case -X: return SDL_SetError("%s: %s", function, #X)
        HANDLE_ERROR_CASE(XMP_ERROR_INTERNAL);
        HANDLE_ERROR_CASE(XMP_ERROR_FORMAT);
        HANDLE_ERROR_CASE(XMP_ERROR_LOAD);
        HANDLE_ERROR_CASE(XMP_ERROR_DEPACK);
        HANDLE_ERROR_CASE(XMP_ERROR_SYSTEM);
        HANDLE_ERROR_CASE(XMP_ERROR_INVALID);
        HANDLE_ERROR_CASE(XMP_ERROR_STATE);
        #undef HANDLE_ERROR_CASE
        default: break;
    }
    return SDL_SetError("%s: unknown error %d", function, error);
}

typedef struct XMP_TrackData
{
    int freq;
    xmp_context ctx;
} XMP_TrackData;


static bool SDLCALL XMP_init(void)
{
    return LoadModule_libxmp();
}

static void SDLCALL XMP_quit(void)
{
    UnloadModule_libxmp();
}

static bool SDLCALL XMP_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    int err;

    if (libxmp.xmp_test_module_from_callbacks) {
        if (libxmp.xmp_test_module_from_callbacks(io, XMP_IoCallbacks, NULL) < 0) {
            return SDL_SetError("Not a libxmp-supported format");
        }
    }

    // If we're still here, we either have valid data, or we're on libxmp < 4.5. In either case, just load the whole thing and see what happens.

    // Go back and do a proper load now to get metadata.
    if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {
        return false;
    }

    xmp_context ctx = libxmp.xmp_create_context();
    if (!ctx) {
        return SDL_OutOfMemory();
    }

    err = libxmp.xmp_load_module_from_callbacks(ctx, io, XMP_IoCallbacks);
    if (err) {
        libxmp.xmp_free_context(ctx);
        return SetLibXmpError("xmp_load_module_from_memory", err);
    }

    struct xmp_module_info info;
    libxmp.xmp_get_module_info(ctx, &info);

    if (info.comment && *info.comment) {
        SDL_SetStringProperty(props, "SDL_mixer.metadata.xmp.comment", info.comment);
        if (!SDL_HasProperty(props, MIX_PROP_METADATA_COPYRIGHT_STRING)) {
            SDL_SetStringProperty(props, MIX_PROP_METADATA_COPYRIGHT_STRING, info.comment);
        }
    }

    if (info.mod) {
        if (info.mod->name[0]) {
            SDL_SetStringProperty(props, "SDL_mixer.metadata.xmp.name", info.mod->name);
            if (!SDL_HasProperty(props, MIX_PROP_METADATA_TITLE_STRING)) {
                SDL_SetStringProperty(props, MIX_PROP_METADATA_TITLE_STRING, info.mod->name);
            }
        }
        if (info.mod->type[0]) {
            SDL_SetStringProperty(props, "SDL_mixer.metadata.xmp.type", info.mod->type);
        }
    }

    // !!! FIXME: is this necessary?
    err = libxmp.xmp_start_player(ctx, spec->freq, 0);  // start this running at device sample rate, so we can calculate total_time.
    if (err) {
        libxmp.xmp_release_module(ctx);
        libxmp.xmp_free_context(ctx);
        return SetLibXmpError("xmp_start_player", err);
    }

    struct xmp_frame_info frame_info;
    libxmp.xmp_get_frame_info(ctx, &frame_info);

    *duration_frames = MIX_MSToFrames(spec->freq, (Sint64) frame_info.total_time);   // closest we can get.
    if (*duration_frames < 0) {
        *duration_frames = 0;
    }

    libxmp.xmp_stop_module(ctx);
    libxmp.xmp_end_player(ctx);
    libxmp.xmp_release_module(ctx);
    libxmp.xmp_free_context(ctx);

    // libxmp prefers to generate Sint16, stereo data.
    spec->format = SDL_AUDIO_S16;
    spec->channels = 2;
    // libxmp generates in whatever sample rate, so use the current device spec->freq.

    *audio_userdata = NULL;  // no state.

    return true;
}

static bool SDLCALL XMP_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    SDL_assert(audio_userdata == NULL);  // no state.

    int err;

    XMP_TrackData *tdata = (XMP_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    tdata->freq = spec->freq;

    tdata->ctx = libxmp.xmp_create_context();
    if (!tdata->ctx) {
        SDL_free(tdata);
        return SDL_OutOfMemory();
    }

    err = libxmp.xmp_load_module_from_callbacks(tdata->ctx, io, XMP_IoCallbacks);
    if (err) {
        libxmp.xmp_free_context(tdata->ctx);
        SDL_free(tdata);
        return SetLibXmpError("xmp_load_module_from_memory", err);
    }

    err = libxmp.xmp_start_player(tdata->ctx, spec->freq, 0);
    if (err) {
        libxmp.xmp_release_module(tdata->ctx);
        libxmp.xmp_free_context(tdata->ctx);
        SDL_free(tdata);
        return SetLibXmpError("xmp_start_player", err);
    }

    *track_userdata = tdata;

    return true;
}

static bool SDLCALL XMP_decode(void *track_userdata, SDL_AudioStream *stream)
{
    XMP_TrackData *tdata = (XMP_TrackData *) track_userdata;

    if (libxmp.xmp_play_frame(tdata->ctx) < 0) {
        return false;  // either an error or EOF, either way we're done.
    }

    struct xmp_frame_info info;
    libxmp.xmp_get_frame_info(tdata->ctx, &info);

    if (info.loop_count > 0) {
        return false;  // if we looped, we're at the EOF. !!! FIXME: do _all_ formats loop, or should we honor this?
    }

    SDL_PutAudioStreamData(stream, info.buffer, info.buffer_size);

    return true;  // had more data to decode.
}

static bool SDLCALL XMP_seek(void *track_userdata, Uint64 frame)
{
    XMP_TrackData *tdata = (XMP_TrackData *) track_userdata;
    Sint64 ms = MIX_FramesToMS(tdata->freq, (Sint64) frame);
    if (ms < 0) {
        ms = 0;
    }
    const int err = libxmp.xmp_seek_time(tdata->ctx, (int) ms);
    libxmp.xmp_play_buffer(tdata->ctx, NULL, 0, 0); // reset the internal state.
    return err < 0 ? SetLibXmpError("xmp_seek_time", err) : true;
}

static bool SDLCALL XMP_jump_to_order(void *track_userdata, int order)
{
    XMP_TrackData *tdata = (XMP_TrackData *) track_userdata;
    int err = libxmp.xmp_set_position(tdata->ctx, order);
    switch (err) {
    case -XMP_ERROR_STATE:
    case -XMP_ERROR_INVALID:
        return SetLibXmpError("xmp_set_position", err);
    }
    return true;
}

static void SDLCALL XMP_quit_track(void *track_userdata)
{
    XMP_TrackData *tdata = (XMP_TrackData *) track_userdata;
    libxmp.xmp_stop_module(tdata->ctx);
    libxmp.xmp_end_player(tdata->ctx);
    libxmp.xmp_release_module(tdata->ctx);
    libxmp.xmp_free_context(tdata->ctx);
    SDL_free(tdata);
}

static void SDLCALL XMP_quit_audio(void *audio_userdata)
{
    SDL_assert(audio_userdata == NULL);   // no state.
}

const MIX_Decoder MIX_Decoder_XMP = {
    "XMP",
    XMP_init,
    XMP_init_audio,
    XMP_init_track,
    XMP_decode,
    XMP_seek,
    XMP_jump_to_order,
    XMP_quit_track,
    XMP_quit_audio,
    XMP_quit
};

#endif
