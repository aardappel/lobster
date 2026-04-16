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

// This file supports Wavpack audio streams using libwavpack.

#ifdef DECODER_WAVPACK

#include "SDL_mixer_internal.h"

#if defined(WAVPACK_DYNAMIC) && defined(SDL_ELF_NOTE_DLOPEN)
SDL_ELF_NOTE_DLOPEN(
    "wavpack",
    "Support for WAVPACK audio using WavPack",
    SDL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
    WAVPACK_DYNAMIC
)
#endif

#define WAVPACK_DBG 0

// This file supports WavPack music streams

#if defined(WAVPACK_HEADER)
#include WAVPACK_HEADER
#elif defined(HAVE_WAVPACK_H)
#include <wavpack.h>
#else
#include <wavpack/wavpack.h>
#endif
#include <stdio.h>  // SEEK_SET, ...

#define DECODE_FRAMES 4096

#ifndef OPEN_DSD_NATIVE
#define OPEN_DSD_NATIVE 0x100
#define OPEN_DSD_AS_PCM 0x200
#define WAVPACK4_OR_OLDER
#endif

#ifdef WAVPACK_DYNAMIC
#define MIX_LOADER_DYNAMIC WAVPACK_DYNAMIC
#endif

#ifdef WAVPACK4_OR_OLDER
typedef struct {
    int32_t (*read_bytes)(void *id, void *data, int32_t bcount);
    int32_t (*write_bytes)(void *id, void *data, int32_t bcount);
    int64_t (*get_pos)(void *id);
    int (*set_pos_abs)(void *id, int64_t pos);
    int (*set_pos_rel)(void *id, int64_t delta, int mode);
    int (*push_back_byte)(void *id, int c);
    int64_t (*get_length)(void *id);
    int (*can_seek)(void *id);
    int (*truncate_here)(void *id);
    int (*close)(void *id);
} WavpackStreamReader64;
#endif

#define MIX_LOADER_FUNCTIONS_wavpackbase \
    MIX_LOADER_FUNCTION(true,uint32_t,WavpackGetLibraryVersion,(void)) \
    MIX_LOADER_FUNCTION(true,char*,WavpackGetErrorMessage,(WavpackContext*)) \
    MIX_LOADER_FUNCTION(true,WavpackContext*,WavpackOpenFileInputEx,(WavpackStreamReader *, void *, void *, char *, int, int)) \
    MIX_LOADER_FUNCTION(true,WavpackContext*,WavpackCloseFile,(WavpackContext*)) \
    MIX_LOADER_FUNCTION(true,int,WavpackGetMode,(WavpackContext*)) \
    MIX_LOADER_FUNCTION(true,int,WavpackGetBytesPerSample,(WavpackContext*)) \
    MIX_LOADER_FUNCTION(true,int,WavpackGetNumChannels,(WavpackContext*)) \
    MIX_LOADER_FUNCTION(true,uint32_t,WavpackGetNumSamples,(WavpackContext*)) \
    MIX_LOADER_FUNCTION(true,uint32_t,WavpackGetSampleRate,(WavpackContext*)) \
    MIX_LOADER_FUNCTION(true,uint32_t,WavpackUnpackSamples,(WavpackContext*, int32_t *buffer, uint32_t samples)) \
    MIX_LOADER_FUNCTION(true,int,WavpackSeekSample,(WavpackContext*, uint32_t sample)) \
    MIX_LOADER_FUNCTION(true,uint32_t,WavpackGetSampleIndex,(WavpackContext*))

#if defined(WAVPACK4_OR_OLDER) && !defined(WAVPACK_DYNAMIC)
#define MIX_LOADER_FUNCTIONS MIX_LOADER_FUNCTIONS_wavpackbase
#else
/* WavPack 5.x functions with 64 bit support: */
#define MIX_LOADER_FUNCTIONS \
    MIX_LOADER_FUNCTIONS_wavpackbase \
    MIX_LOADER_FUNCTION(false,WavpackContext*,WavpackOpenFileInputEx64,(WavpackStreamReader64 *reader, void *wv_id, void *wvc_id, char *error, int flags, int norm_offset)) \
    MIX_LOADER_FUNCTION(false,int64_t,WavpackGetNumSamples64,(WavpackContext*)) \
    MIX_LOADER_FUNCTION(false,int64_t,WavpackGetSampleIndex64,(WavpackContext*)) \
    MIX_LOADER_FUNCTION(false,int,WavpackSeekSample64,(WavpackContext*, int64_t sample))
#endif

#define MIX_LOADER_MODULE wavpack
#include "SDL_mixer_loader.h"


// i/o callbacks ...

static int32_t WAVPACK_IoReadBytes(void *id, void *data, int32_t bcount)
{
    return (int32_t) SDL_ReadIO((SDL_IOStream*)id, data, (size_t)bcount);
}

static uint32_t WAVPACK_IoGetPos32(void *id)
{
    return (uint32_t) SDL_TellIO((SDL_IOStream*)id);
}

static int64_t WAVPACK_IoGetPos64(void *id)
{
    return SDL_TellIO((SDL_IOStream*)id);
}

static int WAVPACK_IoSetPosRel64(void *id, int64_t delta, int mode)
{
    switch (mode) { // just in case SDL_IO doesn't match stdio..
        case SEEK_SET: mode = SDL_IO_SEEK_SET; break;
        case SEEK_CUR: mode = SDL_IO_SEEK_CUR; break;
        case SEEK_END: mode = SDL_IO_SEEK_END; break;
        default: return -1;
    }
    return (SDL_SeekIO((SDL_IOStream*)id, delta, mode) < 0) ? -1 : 0;
}

static int WAVPACK_IoSetPosRel32(void *id, int32_t delta, int mode)
{
    return WAVPACK_IoSetPosRel64(id, delta, mode);
}

static int WAVPACK_IoSetPosAbs64(void *id, int64_t pos)
{
    return (SDL_SeekIO((SDL_IOStream*)id, pos, SDL_IO_SEEK_SET) < 0) ? -1 : 0;
}

static int WAVPACK_IoSetPosAbs32(void *id, uint32_t pos)
{
    return (SDL_SeekIO((SDL_IOStream*)id, pos, SDL_IO_SEEK_SET) < 0) ? -1 : 0;
}

static int WAVPACK_IoPushbackByte(void *id, int c)
{
    (void)c;
    // libwavpack calls ungetc(), but doesn't really modify buffer.
    return (SDL_SeekIO((SDL_IOStream*)id, -1, SDL_IO_SEEK_CUR) < 0) ? -1 : 0;
}

static uint32_t WAVPACK_IoGetLength32(void *id)
{
    return (uint32_t) SDL_GetIOSize((SDL_IOStream*)id);
}

static int64_t WAVPACK_IoGetLength64(void *id)
{
    return SDL_GetIOSize((SDL_IOStream*)id);
}

static int WAVPACK_IoCanSeek(void *id)
{
    return (SDL_SeekIO((SDL_IOStream*)id, 0, SDL_IO_SEEK_CUR) < 0) ? 0 : 1;
}

static WavpackStreamReader WAVPACK_IoReader32 = {
    WAVPACK_IoReadBytes,
    WAVPACK_IoGetPos32,
    WAVPACK_IoSetPosAbs32,
    WAVPACK_IoSetPosRel32,
    WAVPACK_IoPushbackByte,
    WAVPACK_IoGetLength32,
    WAVPACK_IoCanSeek,
    NULL  // write_bytes
};

static WavpackStreamReader64 WAVPACK_IoReader64 = {
    WAVPACK_IoReadBytes,
    NULL, // write_bytes
    WAVPACK_IoGetPos64,
    WAVPACK_IoSetPosAbs64,
    WAVPACK_IoSetPosRel64,
    WAVPACK_IoPushbackByte,
    WAVPACK_IoGetLength64,
    WAVPACK_IoCanSeek,
    NULL, // truncate_here
    NULL  // close
};


#define FLAGS_DSD 0

#ifdef MUSIC_WAVPACK_DSD
#undef FLAGS_DSD
#define FLAGS_DSD OPEN_DSD_AS_PCM

// Decimation code for playing DSD (which comes from the library already decimated 8x)
// Code provided by David Bryant.
// sinc low-pass filter, cutoff = fs/12, 80 terms
#define NUM_TERMS 80
static const int32_t filter[NUM_TERMS] = {
         50,     464,     968,     711,   -1203,   -5028,   -9818,  -13376,
     -12870,   -6021,    7526,   25238,   41688,   49778,   43050,   18447,
     -21428,  -67553, -105876, -120890, -100640,  -41752,   47201,  145510,
     224022,  252377,  208224,   86014,  -97312, -301919, -470919, -541796,
    -461126, -199113,  239795,  813326, 1446343, 2043793, 2509064, 2763659,
    2763659, 2509064, 2043793, 1446343,  813326,  239795, -199113, -461126,
    -541796, -470919, -301919,  -97312,   86014,  208224,  252377,  224022,
     145510,   47201,  -41752, -100640, -120890, -105876,  -67553,  -21428,
      18447,   43050,   49778,   41688,   25238,    7526,   -6021,  -12870,
     -13376,   -9818,   -5028,   -1203,     711,     968,     464,      50
};

typedef struct chan_state {
    int32_t delay[NUM_TERMS];
    int index, num_channels, ratio;
} ChanState;

static void *decimation_init(int num_channels, int ratio)
{
    ChanState *sp = (ChanState *)SDL_calloc(num_channels, sizeof(ChanState));

    if (sp) {
        int i = 0;
        for (; i < num_channels; ++i) {
            sp[i].num_channels = num_channels;
            sp[i].index = NUM_TERMS - ratio;
            sp[i].ratio = ratio;
        }
    }

    return sp;
}

// FIXME: This isn't particularly easy on the CPU !
static int decimation_run(void *context, int32_t *samples, int num_samples)
{
    ChanState *sp = (ChanState *)context;
    int32_t *in_samples = samples;
    int32_t *out_samples = samples;
    const int num_channels = sp->num_channels;
    const int ratio = sp->ratio;
    int chan = 0;

    while (num_samples) {
        sp = (ChanState *)context + chan;

        sp->delay[sp->index++] = *in_samples++;

        if (sp->index == NUM_TERMS) {
            int64_t sum = 0;
            int i = 0;
            for (; i < NUM_TERMS; ++i) {
                sum += (int64_t)filter[i] * sp->delay[i];
            }
            *out_samples++ = (int32_t)(sum >> 24);
            SDL_memmove(sp->delay, sp->delay + ratio, sizeof(sp->delay[0]) * (NUM_TERMS - ratio));
            sp->index = NUM_TERMS - ratio;
        }

        if (++chan == num_channels) {
            num_samples--;
            chan = 0;
        }
    }

    return (int)(out_samples - samples) / num_channels;
}

static void decimation_reset(void *context)
{
    ChanState *sp = (ChanState *)context;
    const int num_channels = sp->num_channels;
    const int ratio = sp->ratio;
    int i = 0;

    SDL_memset(sp, 0, sizeof(ChanState) * num_channels);
    for (; i < num_channels; ++i) {
        sp[i].num_channels = num_channels;
        sp[i].index = NUM_TERMS - ratio;
        sp[i].ratio = ratio;
    }
}
#endif // MUSIC_WAVPACK_DSD


typedef struct WAVPACK_AudioData
{
    const Uint8 *wvcdata;
    size_t wvcdatalen;
    int64_t numsamples;
    uint32_t samplerate;
    int bps;
    int channels;
    int mode;
    int decimation;
    SDL_AudioFormat format;
} WAVPACK_AudioData;

typedef struct WAVPACK_TrackData
{
    const WAVPACK_AudioData *adata;
    SDL_IOStream *io;  // an IOStream for accessing the .wv file data.
    SDL_IOStream *wvcio;  // a const-mem IOStream for accessing the adata's correction data.
    WavpackContext *ctx;
    void *decimation_ctx;
    void *decode_buffer;
} WAVPACK_TrackData;


static bool SDLCALL WAVPACK_init(void)
{
    if (!LoadModule_wavpack()) {
        return false;
    }
    #if WAVPACK_DBG
    SDL_Log("WavPack library version: 0x%x", (unsigned int) wavpack.WavpackGetLibraryVersion());
    #endif
    return true;
}

static void SDLCALL WAVPACK_quit(void)
{
    UnloadModule_wavpack();
}

static bool SDLCALL WAVPACK_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    char err[80];

    // Try to load a correction file if we can. This turns a lossy file into a lossless one.
    //  If the app provided an IOStream for it, use it. If not, see if they provided a path. If not, see if we can load a ".wvc" version of the original path.
    bool closewvcio = false;
    SDL_IOStream *wvcio = SDL_GetPointerProperty(props, MIX_PROP_DECODER_WAVPACK_WVC_IOSTREAM_POINTER, NULL);
    if (wvcio) {
        closewvcio = SDL_GetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, false);
    } else {
        // Try to load a correction file if available.
        closewvcio = true;  // if we open it, we close it.
        const char *wvcpath = SDL_GetStringProperty(props, MIX_PROP_DECODER_WAVPACK_WVC_PATH_STRING, NULL);
        char *str = NULL;
        if (!wvcpath) {
            const char *origpath = SDL_GetStringProperty(props, MIX_PROP_AUDIO_LOAD_PATH_STRING, NULL);
            if (origpath) {
                const int slen = SDL_asprintf(&str, "%sc", origpath);
                if ((slen > 4) && (SDL_strcasecmp(&str[slen - 4], ".wvc") == 0)) {  // only accept it if the original file ended in ".wv" (so `str` is ".wvc").
                    wvcpath = str;
                }
            }
        }
        if (wvcpath) {
            wvcio = SDL_IOFromFile(wvcpath, "rb");
            #if WAVPACK_DBG
            if (wvcio) {
                SDL_Log("Loaded WavPack correction file %s", wvcpath);
            }
            #endif
        }
        SDL_free(str);
    }

    if (!SDL_ReadIO(io, err, 4)) {
        if (wvcio && closewvcio) { SDL_CloseIO(wvcio); }
        return false;
    } else if (SDL_memcmp(err, "wvpk", 4) != 0) {  // just a quick check before we load the whole thing into RAM.
        if (wvcio && closewvcio) { SDL_CloseIO(wvcio); }
        return SDL_SetError("Not a WavPack file.");
    }

    // Go back and do a proper load now to get metadata.
    if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {
        if (wvcio && closewvcio) { SDL_CloseIO(wvcio); }
        return false;
    }

    // !!! FIXME: it would be better to not pull this all in upfront; move the thing where we decide how to access the data to a common place, make an IOStream per-track.
    size_t wvcdatalen = 0;
    Uint8 *wvcdata = NULL;
    if (wvcio) {
        wvcdata = (Uint8 *) SDL_LoadFile_IO(wvcio, &wvcdatalen, false);
        if (closewvcio) {
            SDL_CloseIO(wvcio);
        }
        if (!wvcdata) {
            return false;
        }
    }

    WavpackContext *ctx = NULL;
    WAVPACK_AudioData *adata = (WAVPACK_AudioData *) SDL_calloc(1, sizeof (*adata));
    if (!adata) {
        goto failed;
    }

    if (wvcdata) {
        wvcio = SDL_IOFromConstMem(wvcdata, wvcdatalen);  // switch over to a memory IOStream.
        if (!wvcio) {  // uhoh.
            goto failed;
        }
    }

    // now open the memory buffers for serious processing.
    ctx =
    #if !defined(WAVPACK4_OR_OLDER) || defined(WAVPACK_DYNAMIC)
      (wavpack.WavpackOpenFileInputEx64 != NULL) ?
       wavpack.WavpackOpenFileInputEx64(&WAVPACK_IoReader64, io, wvcio, err, OPEN_NORMALIZE|OPEN_TAGS|FLAGS_DSD, 0) :
    #endif
       wavpack.WavpackOpenFileInputEx(&WAVPACK_IoReader32, io, wvcio, err, OPEN_NORMALIZE|OPEN_TAGS, 0);
    if (!ctx) {
        SDL_SetError("%s", err);
        goto failed;
    }

    adata->wvcdata = wvcdata;
    adata->wvcdatalen = wvcdatalen;
    adata->numsamples =
    #if !defined(WAVPACK4_OR_OLDER) || defined(WAVPACK_DYNAMIC)
      wavpack.WavpackGetNumSamples64 ? wavpack.WavpackGetNumSamples64(ctx) :
    #endif
      wavpack.WavpackGetNumSamples(ctx);
    adata->bps = wavpack.WavpackGetBytesPerSample(ctx) << 3;
    adata->mode = wavpack.WavpackGetMode(ctx);
    adata->channels = wavpack.WavpackGetNumChannels(ctx);
    adata->samplerate = wavpack.WavpackGetSampleRate(ctx);
    adata->decimation = 1;

    #ifdef MUSIC_WAVPACK_DSD
    // for very high sample rates (including DSD, which will normally be 352,800 Hz) decimate 4x here before sending on
    if (adata->samplerate >= 256000) {
        adata->decimation = 4;
    }
    #endif

    // library returns the samples in 8, 16, 24, or 32 bit depth, but
    // always in an int32_t[] buffer, in signed host-endian format.
    switch (adata->bps) {
        case 8: adata->format = SDL_AUDIO_S8; break;
        case 16: adata->format = SDL_AUDIO_S16; break;
        case 24: adata->format = SDL_AUDIO_S32; break;
        case 32: adata->format = (adata->mode & MODE_FLOAT) ? SDL_AUDIO_F32 : SDL_AUDIO_S32; break;
        default: SDL_SetError("Unsupported WavPack bitdepth"); goto failed;  // uhoh.
    }

    spec->format = adata->format;
    spec->freq = (int) adata->samplerate / adata->decimation;
    spec->channels = adata->channels;

    #if WAVPACK_DBG
    SDL_Log("WavPack loader:");
    SDL_Log(" correction data: %s", wvcio ? "yes" : "no");
    SDL_Log(" numsamples: %" SDL_PRIs64, (Sint64)adata->numsamples);
    SDL_Log(" samplerate: %d", spec->freq);
    SDL_Log(" bitspersample: %d", adata->bps);
    SDL_Log(" channels: %d", spec->channels);
    SDL_Log(" mode: 0x%x", adata->mode);
    SDL_Log(" lossy: %s", (adata->mode & MODE_LOSSLESS) ? "false" : "true");
    SDL_Log(" decimation: %d", adata->decimation);
    SDL_Log(" duration: %f", adata->numsamples / (double)adata->samplerate);
    #endif

    // WavPack files use other tag standards; their docs seem to favor APEv2 tags, but apparently
    // ID3 can be used as well. Since we handle those elsewhere, I've removed the WavpackGetTagItem
    // calls from in here.

    wavpack.WavpackCloseFile(ctx);
    SDL_CloseIO(wvcio);  // close our memory i/o.

    *duration_frames = (Sint64) adata->numsamples;
    *audio_userdata = adata;

    return true;

failed:
    if (ctx) { wavpack.WavpackCloseFile(ctx); }
    SDL_CloseIO(wvcio);
    SDL_free(wvcdata);
    SDL_free(adata);
    return false;
}

static bool SDLCALL WAVPACK_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    WAVPACK_TrackData *tdata = (WAVPACK_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    const WAVPACK_AudioData *adata = (const WAVPACK_AudioData *) audio_userdata;

    char err[80];

    if (adata->wvcdata) {
        tdata->wvcio = SDL_IOFromConstMem(adata->wvcdata, adata->wvcdatalen);
        if (!tdata->wvcio) {  // uhoh.
            goto failed;
        }
    }

    tdata->decode_buffer = SDL_malloc(DECODE_FRAMES * spec->channels * sizeof(int32_t) * adata->decimation);
    if (!tdata->decode_buffer) {
        goto failed;
    }

    // now open the memory buffers for serious processing.
    tdata->ctx =
    #if !defined(WAVPACK4_OR_OLDER) || defined(WAVPACK_DYNAMIC)
      (wavpack.WavpackOpenFileInputEx64 != NULL) ?
       wavpack.WavpackOpenFileInputEx64(&WAVPACK_IoReader64, io, tdata->wvcio, err, OPEN_NORMALIZE|OPEN_TAGS|FLAGS_DSD, 0) :
    #endif
       wavpack.WavpackOpenFileInputEx(&WAVPACK_IoReader32, io, tdata->wvcio, err, OPEN_NORMALIZE|OPEN_TAGS, 0);

    if (!tdata->ctx) {
        goto failed;
    }

    tdata->adata = adata;

    *track_userdata = tdata;

    return true;

failed:
    if (tdata) {
        SDL_assert(tdata->ctx == NULL);
        SDL_free(tdata->decode_buffer);
        SDL_CloseIO(tdata->wvcio);
        SDL_free(tdata);
    }
    return false;
}

static bool SDLCALL WAVPACK_decode(void *track_userdata, SDL_AudioStream *stream)
{
    WAVPACK_TrackData *tdata = (WAVPACK_TrackData *) track_userdata;
    const WAVPACK_AudioData *adata = tdata->adata;

    int amount = (int) wavpack.WavpackUnpackSamples(tdata->ctx, tdata->decode_buffer, DECODE_FRAMES * adata->decimation);
    #ifdef MUSIC_WAVPACK_DSD
    if (amount && tdata->decimation_ctx) {
        amount = decimation_run(tdata->decimation_ctx, tdata->decode_buffer, amount);
    }
    #endif

    if (!amount) {
        return false;  // EOF.
    }

    amount *= adata->channels;  // move from sample frames to samples.

    // library returns the samples in 8, 16, 24, or 32 bit depth, but
    // always in an int32_t[] buffer, in signed host-endian format.
    const SDL_AudioFormat format = adata->format;
    if (format == SDL_AUDIO_S8) {
        const Sint32 *src = (const Sint32 *) tdata->decode_buffer;
        Sint8 *dst = (Sint8 *) tdata->decode_buffer;
        for (int i = 0; i < amount; i++) {
            dst[i] = (Sint8) src[i];  // data is 8-bit audio in an int32 array, shrink out unused bits in-place.
        }
    } else if (format == SDL_AUDIO_S16) {
        const Sint32 *src = (const Sint32 *) tdata->decode_buffer;
        Sint16 *dst = (Sint16 *) tdata->decode_buffer;
        for (int i = 0; i < amount; i++) {
            dst[i] = (Sint16) src[i];  // data is 16-bit audio in an int32 array, shrink out unused bits in-place.
        }
    } else if (adata->bps == 24) {
        SDL_assert(format == SDL_AUDIO_S32);
        const Sint32 *src = (const Sint32 *) tdata->decode_buffer;
        Sint32 *dst = (Sint32 *) tdata->decode_buffer;
        for (int i = 0; i < amount; i++) {
            dst[i] = src[i] << 8;  // data is 24-bit audio in an int32 array, slide bits over so most significant bits scale up to full 32-bit range.
        }
    } else {
        SDL_assert((format == SDL_AUDIO_F32) || (format == SDL_AUDIO_S32));  // these just copy through as-is.
    }

    SDL_PutAudioStreamData(stream, tdata->decode_buffer, amount * SDL_AUDIO_BYTESIZE(adata->format));
    return true;
}

static bool SDLCALL WAVPACK_seek(void *track_userdata, Uint64 frame)
{
    WAVPACK_TrackData *tdata = (WAVPACK_TrackData *) track_userdata;
    const int success =
    #if !defined(WAVPACK4_OR_OLDER) || defined(WAVPACK_DYNAMIC)
      (wavpack.WavpackSeekSample64 != NULL) ?
       wavpack.WavpackSeekSample64(tdata->ctx, frame) :
    #endif
       wavpack.WavpackSeekSample(tdata->ctx, (uint32_t) frame);
    if (!success) {
        return SDL_SetError("%s", wavpack.WavpackGetErrorMessage(tdata->ctx));
    }

    #ifdef MUSIC_WAVPACK_DSD
    if (music->decimation_ctx) {
        decimation_reset(music->decimation_ctx);
    }
    #endif

    return true;
}

static void SDLCALL WAVPACK_quit_track(void *track_userdata)
{
    WAVPACK_TrackData *tdata = (WAVPACK_TrackData *) track_userdata;

    #ifdef MUSIC_WAVPACK_DSD
    SDL_free(tdata->decimation_ctx);
    #endif

    wavpack.WavpackCloseFile(tdata->ctx);
    SDL_CloseIO(tdata->wvcio);
    SDL_free(tdata->decode_buffer);
    SDL_free(tdata);
}

static void SDLCALL WAVPACK_quit_audio(void *audio_userdata)
{
    WAVPACK_AudioData *adata = (WAVPACK_AudioData *) audio_userdata;
    SDL_free((void *) adata->wvcdata);
    SDL_free(adata);
}

const MIX_Decoder MIX_Decoder_WAVPACK = {
    "WAVPACK",
    WAVPACK_init,
    WAVPACK_init_audio,
    WAVPACK_init_track,
    WAVPACK_decode,
    WAVPACK_seek,
    NULL,  // jump_to_order
    WAVPACK_quit_track,
    WAVPACK_quit_audio,
    WAVPACK_quit
};

#endif

