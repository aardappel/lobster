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

#ifdef DECODER_MP3_MPG123

#include "SDL_mixer_internal.h"

#if defined(MPG123_DYNAMIC) && defined(SDL_ELF_NOTE_DLOPEN)
SDL_ELF_NOTE_DLOPEN(
    "mp3",
    "Support for MP3 audio using mpg123",
    SDL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
    MPG123_DYNAMIC
)
#endif

#define MPG123_ENUM_API /* for mpg123_param() */
#include <stdio.h>  // SEEK_SET
#include <mpg123.h>

#ifdef _MSC_VER
#define Mpg123SSizeType ptrdiff_t
#else
#define Mpg123SSizeType ssize_t
#endif
#if (MPG123_API_VERSION >= 45) /* api (but not abi) change as of mpg123-1.26.0 */
#define Mpg123OutMemoryType void *
#else
#define Mpg123OutMemoryType unsigned char *
#endif

#ifdef MPG123_DYNAMIC
#define MIX_LOADER_DYNAMIC MPG123_DYNAMIC
#endif

#define MIX_LOADER_FUNCTIONS_mpg123base \
    MIX_LOADER_FUNCTION(true,int,mpg123_close,(mpg123_handle *mh)) \
    MIX_LOADER_FUNCTION(true,void,mpg123_delete,(mpg123_handle *mh)) \
    MIX_LOADER_FUNCTION(true,void,mpg123_exit,(void)) \
    MIX_LOADER_FUNCTION(true,int,mpg123_format,(mpg123_handle *mh, long rate, int channels, int encodings)) \
    MIX_LOADER_FUNCTION(true,int,mpg123_format_none,(mpg123_handle *mh)) \
    MIX_LOADER_FUNCTION(true,int,mpg123_getformat,(mpg123_handle *mh, long *rate, int *channels, int *encoding)) \
    MIX_LOADER_FUNCTION(true,int,mpg123_init,(void)) \
    MIX_LOADER_FUNCTION(true,mpg123_handle*,mpg123_new,(const char* decoder, int *error)) \
    MIX_LOADER_FUNCTION(true,const char*,mpg123_plain_strerror,(int errcode)) \
    MIX_LOADER_FUNCTION(true,void,mpg123_rates,(const long **list, size_t *number)) \
    MIX_LOADER_FUNCTION(true,int,mpg123_read,(mpg123_handle *mh, Mpg123OutMemoryType outmemory, size_t outmemsize, size_t *done)) \
    MIX_LOADER_FUNCTION(true,int,mpg123_scan,(mpg123_handle *mh)) \
    MIX_LOADER_FUNCTION(true,const char*,mpg123_strerror,(mpg123_handle *mh)) \

#if (MPG123_API_VERSION >= 49)
    #define MIX_LOADER_FUNCTIONS \
        MIX_LOADER_FUNCTIONS_mpg123base \
        MIX_LOADER_FUNCTION(true,int,mpg123_param2,(mpg123_handle *mh, int type, long value, double fvalue)) \
        MIX_LOADER_FUNCTION(true,int,mpg123_open_handle64,(mpg123_handle *mh, void *iohandle)) \
        MIX_LOADER_FUNCTION(true,int,mpg123_reader64,(mpg123_handle *mh, int (*r_read)(void*, void*, size_t, size_t*), int64_t (*r_lseek)(void*, int64_t, int), void (*cleanup)(void*))) \
        MIX_LOADER_FUNCTION(true,int64_t,mpg123_seek64,(mpg123_handle *mh, int64_t sampleoff, int whence)) \
        MIX_LOADER_FUNCTION(true,int64_t,mpg123_tell64,(mpg123_handle *mh)) \
        MIX_LOADER_FUNCTION(true,int64_t,mpg123_length64,(mpg123_handle *mh))
#else
    #define MIX_LOADER_FUNCTIONS \
        MIX_LOADER_FUNCTIONS_mpg123base \
        MIX_LOADER_FUNCTION(true,int,mpg123_param,(mpg123_handle *mh, enum mpg123_parms type, long value, double fvalue)) \
        MIX_LOADER_FUNCTION(true,int,mpg123_open_handle,(mpg123_handle *mh, void *iohandle)) \
        MIX_LOADER_FUNCTION(true,int,mpg123_replace_reader_handle,(mpg123_handle *mh, Mpg123SSizeType (*r_read)(void *, void *, size_t), off_t (*r_lseek)(void *, off_t, int), void (*cleanup)(void*))) \
        MIX_LOADER_FUNCTION(true,off_t,mpg123_seek,(mpg123_handle *mh, off_t sampleoff, int whence)) \
        MIX_LOADER_FUNCTION(true,off_t,mpg123_tell,(mpg123_handle *mh)) \
        MIX_LOADER_FUNCTION(true,off_t,mpg123_length,(mpg123_handle *mh))
#endif

#define MIX_LOADER_MODULE mpg123
#include "SDL_mixer_loader.h"


static bool SDLCALL MPG123_init(void)
{
    if (!LoadModule_mpg123()) {
        return false;
    }

    mpg123.mpg123_init();

    return true;
}

static void SDLCALL MPG123_quit(void)
{
    if (mpg123.mpg123_exit) {
        mpg123.mpg123_exit();
    }
    UnloadModule_mpg123();
}

#if (MPG123_API_VERSION >= 49)
static int MPG123_IoRead(void* p, void* dst, size_t n, size_t *b)
{
    SDL_IOStream *io = (SDL_IOStream *) p;
    *b = SDL_ReadIO(io, dst, n);
    if (!*b && (SDL_GetIOStatus(io) != SDL_IO_STATUS_EOF)) {
        return -1;
    }
    return 0;
}

static int64_t MPG123_IoSeek(void *p, int64_t offset, int whence)
{
    return SDL_SeekIO((SDL_IOStream *)p, offset, whence);
}
#else
static Mpg123SSizeType MPG123_IoRead(void *p, void *dst, size_t n)
{
    SDL_IOStream *io = (SDL_IOStream *) p;
    const Mpg123SSizeType r = (Mpg123SSizeType) SDL_ReadIO(io, dst, n);
    if (!r && (SDL_GetIOStatus(io) != SDL_IO_STATUS_EOF)) {
        return -1;
    }
    return r < 0 ? -1 : r;
}

static off_t MPG123_IoSeek(void *p, off_t offset, int whence)
{
    return (off_t)SDL_SeekIO((SDL_IOStream *)p, (Sint64)offset, whence);
}
#endif

static void MPG123_IoClose(void *p)
{
    (void)p;  // do nothing, we will free the file later
}

static SDL_AudioFormat FormatMPG123ToSDL(int fmt)
{
    switch (fmt) {
        case MPG123_ENC_SIGNED_8: return SDL_AUDIO_S8;
        case MPG123_ENC_UNSIGNED_8: return SDL_AUDIO_U8;
        case MPG123_ENC_SIGNED_16: return SDL_AUDIO_S16;
        case MPG123_ENC_SIGNED_32: return SDL_AUDIO_S32;
        case MPG123_ENC_FLOAT_32: return SDL_AUDIO_F32;
        default: break;
    }
    return SDL_AUDIO_UNKNOWN;
}

/*#define DEBUG_MPG123*/
#ifdef DEBUG_MPG123
static const char *MPG123FormatString(int fmt)
{
    switch (fmt) {
#define f(x) case x: return #x;
        f(MPG123_ENC_UNSIGNED_8)
        f(MPG123_ENC_SIGNED_8)
        f(MPG123_ENC_SIGNED_16)
        f(MPG123_ENC_SIGNED_32)
        f(MPG123_ENC_FLOAT_32)
#undef f
    }
    return "unknown";
}
#endif

static const char *mpg_err(mpg123_handle* mpg, int result)
{
    const char *err = "unknown error";

    if (mpg && result == MPG123_ERR) {
        err = mpg123.mpg123_strerror(mpg);
    } else {
        err = mpg123.mpg123_plain_strerror(result);
    }
    return err;
}


// stolen from dr_mp3's drmp3_hdr_valid() function.
static int IsMp3Header(const Uint8 *h)
{
    #define MP3_HDR_GET_LAYER(h)            (((h[1]) >> 1) & 3)
    #define MP3_HDR_GET_BITRATE(h)          ((h[2]) >> 4)
    #define MP3_HDR_GET_SAMPLE_RATE(h)      (((h[2]) >> 2) & 3)
    return h[0] == 0xff && ((h[1] & 0xF0) == 0xf0 || (h[1] & 0xFE) == 0xe2) &&
        (MP3_HDR_GET_LAYER(h) != 0) &&
        (MP3_HDR_GET_BITRATE(h) != 15) &&
        (MP3_HDR_GET_SAMPLE_RATE(h) != 3);
    #undef MP3_HDR_GET_LAYER
    #undef MP3_HDR_GET_BITRATE
    #undef MP3_HDR_GET_SAMPLE_RATE
}


static bool SDLCALL MPG123_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    // libmpg123 will accept almost any binary data and try to treat it as MPEG audio, which causes
    //  problems. Sometimes it will reject invalid data, but it appears to accept lots of things that
    //  are definitely not MPEG audio.
    // To limit it from accepting things that aren't MP3s, we do an extremely simple check to see if we
    //  see something kinda like a valid MPEG frame header at the start.
    // If this decoder was explicitly requested, we skip this check and let libmpg123 run with anything it
    //  thinks it can (overconfidently) handle.
    const char *decoder_name = SDL_GetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, NULL);
    const bool accept_anything = decoder_name && (SDL_strcasecmp(decoder_name, "MPG123") == 0);
    if (!accept_anything) {
        // if it has an ID3 tag, we'll assume it's worth trying without further tests.
        if ( !SDL_HasProperty(props, "SDL_mixer.metadata.id3v1.title") &&
             !SDL_HasProperty(props, "SDL_mixer.metadata.id3v2.TIT2") &&
             !SDL_HasProperty(props, "SDL_mixer.metadata.id3v2.TT2") ) {

            Uint8 maybe_frame_header[4];
            if (SDL_ReadIO(io, maybe_frame_header, sizeof (maybe_frame_header)) != sizeof (maybe_frame_header)) {
                return false;
            } else if (!IsMp3Header(maybe_frame_header)) {
                return SDL_SetError("mpg123: (Probably) not MPEG audio data");
            } else if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {  // roll back and let mpg123 have it all.
                return false;
            }
        }
    }

    const long *rates = NULL;
    size_t num_rates = 0;
    int encoding = 0;
    long rate = 0;
    int result = 0;

    mpg123_handle *handle = mpg123.mpg123_new(NULL, &result);
    if (result != MPG123_OK) {
        return SDL_SetError("mpg123_new failed");
    }

    #if (MPG123_API_VERSION >= 49)
    mpg123.mpg123_param2(handle, MPG123_ADD_FLAGS, MPG123_QUIET, 0.0);  // don't log errors to stderr.
    result = mpg123.mpg123_reader64(handle, MPG123_IoRead, MPG123_IoSeek, MPG123_IoClose);
    #else
    mpg123.mpg123_param(handle, MPG123_ADD_FLAGS, MPG123_QUIET, 0.0);   // don't log errors to stderr.
    result = mpg123.mpg123_replace_reader_handle(handle, MPG123_IoRead, MPG123_IoSeek, MPG123_IoClose);
    #endif
    if (result != MPG123_OK) {
        SDL_SetError("mpg123_replace_reader_handle: %s", mpg_err(handle, result));
        goto failed;
    }

    result = mpg123.mpg123_format_none(handle);
    if (result != MPG123_OK) {
        SDL_SetError("mpg123_format_none: %s", mpg_err(handle, result));
        goto failed;
    }

    mpg123.mpg123_rates(&rates, &num_rates);
    for (size_t i = 0; i < num_rates; ++i) {
        const int channels = (MPG123_MONO|MPG123_STEREO);
        const int formats = (MPG123_ENC_SIGNED_8 |
                             MPG123_ENC_UNSIGNED_8 |
                             MPG123_ENC_SIGNED_16 |
                             MPG123_ENC_SIGNED_32 |
                             MPG123_ENC_FLOAT_32);
        mpg123.mpg123_format(handle, rates[i], channels, formats);
    }

    #if (MPG123_API_VERSION >= 49)
    result = mpg123.mpg123_open_handle64(handle, io);
    #else
    result = mpg123.mpg123_open_handle(handle, io);
    #endif
    if (result != MPG123_OK) {
        SDL_SetError("mpg123_open_handle: %s", mpg_err(handle, result));
        goto failed;
    }

    result = mpg123.mpg123_getformat(handle, &rate, &spec->channels, &encoding);
    if (result != MPG123_OK) {
        SDL_SetError("mpg123_getformat: %s", mpg_err(handle, result));
        goto failed;
    }

#ifdef DEBUG_MPG123
    SDL_Log("MPG123 format: %s, channels: %d, rate: %ld\n", MPG123FormatString(encoding), channels, rate);
#endif

    spec->format = FormatMPG123ToSDL(encoding);
    SDL_assert(spec->format != SDL_AUDIO_UNKNOWN);
    spec->freq = rate;

    result = mpg123.mpg123_scan(handle);  // parse through whole file; it makes mpg123_length() accurate even if MP3 metadata is missing.
    if (result != MPG123_OK) {
        SDL_SetError("mpg123_scan: %s", mpg_err(handle, result));
        goto failed;
    }

    // mpg123_length() returns sample frames, or MPG123_ERR, which happens to be -1, which we use for "don't know" here.
    #if (MPG123_API_VERSION >= 49)
    *duration_frames = mpg123.mpg123_length64(handle);
    #else
    *duration_frames = (Sint64) mpg123.mpg123_length(handle);
    #endif
    
    mpg123.mpg123_close(handle);
    mpg123.mpg123_delete(handle);
    handle = NULL;

    *audio_userdata = NULL;  // no state.

    return true;

failed:
    if (handle) {
        mpg123.mpg123_close(handle);
        mpg123.mpg123_delete(handle);
    }
    return false;
}

static bool SDLCALL MPG123_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    SDL_assert(audio_userdata == NULL);  // no state.

    int result = 0;
    mpg123_handle *handle = mpg123.mpg123_new(NULL, &result);
    if (result != MPG123_OK) {
        return SDL_SetError("mpg123_new failed");
    }

    #if (MPG123_API_VERSION >= 49)
    mpg123.mpg123_param2(handle, MPG123_ADD_FLAGS, MPG123_QUIET, 0.0);  // don't log errors to stderr.
    result = mpg123.mpg123_reader64(handle, MPG123_IoRead, MPG123_IoSeek, MPG123_IoClose);
    #else
    mpg123.mpg123_param(handle, MPG123_ADD_FLAGS, MPG123_QUIET, 0.0);   // don't log errors to stderr.
    result = mpg123.mpg123_replace_reader_handle(handle, MPG123_IoRead, MPG123_IoSeek, MPG123_IoClose);
    #endif
    if (result != MPG123_OK) {
        SDL_SetError("mpg123_replace_reader_handle: %s", mpg_err(handle, result));
        mpg123.mpg123_delete(handle);
        return false;
    }

    // !!! FIXME: can I just force the `spec` format here, or does that cause problems if we have a "frankenstein" mp3 that changes formats later?
    result = mpg123.mpg123_format_none(handle);
    if (result != MPG123_OK) {
        SDL_SetError("mpg123_format_none: %s", mpg_err(handle, result));
        mpg123.mpg123_delete(handle);
        return false;
    }

    const long *rates = NULL;
    size_t num_rates = 0;
    mpg123.mpg123_rates(&rates, &num_rates);
    for (size_t i = 0; i < num_rates; ++i) {
        const int channels = (MPG123_MONO|MPG123_STEREO);
        const int formats = (MPG123_ENC_SIGNED_8 |
                             MPG123_ENC_UNSIGNED_8 |
                             MPG123_ENC_SIGNED_16 |
                             MPG123_ENC_SIGNED_32 |
                             MPG123_ENC_FLOAT_32);
        mpg123.mpg123_format(handle, rates[i], channels, formats);
    }

    #if (MPG123_API_VERSION >= 49)
    result = mpg123.mpg123_open_handle64(handle, io);
    #else
    result = mpg123.mpg123_open_handle(handle, io);
    #endif
    if (result != MPG123_OK) {
        SDL_SetError("mpg123_open_handle: %s", mpg_err(handle, result));
        mpg123.mpg123_delete(handle);
        return false;
    }

    // !!! FIXME: should we do this? It makes seeking better, maybe?  mpg123.mpg123_scan(handle);

    *track_userdata = handle;

    return true;
}

static bool SDLCALL MPG123_decode(void *track_userdata, SDL_AudioStream *stream)
{
    mpg123_handle *handle = (mpg123_handle *) track_userdata;
    SDL_AudioSpec spec;
    int result;
    size_t amount = 0;
    long rate;
    int channels, encoding;
    Uint8 buffer[1024];

    result = mpg123.mpg123_read(handle, buffer, sizeof (buffer), &amount);

    if (result == MPG123_NEW_FORMAT) {
        result = mpg123.mpg123_getformat(handle, &rate, &channels, &encoding);
        if (result != MPG123_OK) {
            return SDL_SetError("mpg123_getformat: %s", mpg_err(handle, result));
        }

        #ifdef DEBUG_MPG123
        SDL_Log("MPG123 format: %s, channels: %d, rate: %ld\n", MPG123FormatString(encoding), channels, rate);
        #endif

        spec.format = FormatMPG123ToSDL(encoding);
        spec.channels = channels;
        spec.freq = (int)rate;
        SDL_assert(spec.format != SDL_AUDIO_UNKNOWN);

        SDL_SetAudioStreamFormat(stream, &spec, NULL);
    }

    if (amount > 0) {
        SDL_PutAudioStreamData(stream, buffer, (int)amount);
    }

    if ((result != MPG123_OK) && (result != MPG123_DONE)) {
        SDL_SetError("mpg123_read: %s", mpg_err(handle, result));
    }

    return (result == MPG123_OK);
}

static bool SDLCALL MPG123_seek(void *track_userdata, Uint64 frame)
{
    mpg123_handle *handle = (mpg123_handle *) track_userdata;
    #if (MPG123_API_VERSION >= 49)
    const int64_t rc = mpg123.mpg123_seek64(handle, (int64_t) frame, SEEK_SET);
    #else
    const off_t rc = mpg123.mpg123_seek(handle, (off_t) frame, SEEK_SET);
    #endif
    return (rc < 0) ? SDL_SetError("mpg123_seek:%s", mpg_err(handle, rc)) : true;
}

static void SDLCALL MPG123_quit_track(void *track_userdata)
{
    mpg123_handle *handle = (mpg123_handle *) track_userdata;
    mpg123.mpg123_close(handle);
    mpg123.mpg123_delete(handle);
}

static void SDLCALL MPG123_quit_audio(void *audio_userdata)
{
    SDL_assert(audio_userdata == NULL);  // no state.
}

const MIX_Decoder MIX_Decoder_MPG123 = {
    "MPG123",
    MPG123_init,
    MPG123_init_audio,
    MPG123_init_track,
    MPG123_decode,
    MPG123_seek,
    NULL,  // jump_to_order
    MPG123_quit_track,
    MPG123_quit_audio,
    MPG123_quit
};

#endif
