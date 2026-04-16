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

// This file supports Ogg Vorbis audio streams using libvorbisfile (or the integer-only implementation, "tremor").

#ifdef DECODER_OGGVORBIS_VORBISFILE

#include "SDL_mixer_internal.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#if defined(VORBIS_HEADER)
#include VORBIS_HEADER
#elif defined(VORBIS_USE_TREMOR)
#include <tremor/ivorbisfile.h>
#else
#include <vorbis/vorbisfile.h>
#endif

#if defined(MPG123_DYNAMIC) && defined(SDL_ELF_NOTE_DLOPEN)
SDL_ELF_NOTE_DLOPEN(
    "vorbis",
    "Support for VORBIS audio", // vorbisfile or tremor
    SDL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
    VORBIS_DYNAMIC
)
#endif

#ifdef VORBIS_DYNAMIC
#define MIX_LOADER_DYNAMIC VORBIS_DYNAMIC
#endif

#define MIX_LOADER_FUNCTIONS_vorbisbase \
    MIX_LOADER_FUNCTION(true,int,ov_clear,(OggVorbis_File *vf)) \
    MIX_LOADER_FUNCTION(true,vorbis_info *,ov_info,(OggVorbis_File *vf,int link)) \
    MIX_LOADER_FUNCTION(true,vorbis_comment *,ov_comment,(OggVorbis_File *vf,int link)) \
    MIX_LOADER_FUNCTION(true,int,ov_test_callbacks,(void *datasource, OggVorbis_File *vf, const char *initial, long ibytes, ov_callbacks callbacks)) \
    MIX_LOADER_FUNCTION(true,int,ov_open_callbacks,(void *datasource, OggVorbis_File *vf, const char *initial, long ibytes, ov_callbacks callbacks)) \
    MIX_LOADER_FUNCTION(true,ogg_int64_t,ov_pcm_total,(OggVorbis_File *vf,int i)) \
    MIX_LOADER_FUNCTION(true,int,ov_pcm_seek,(OggVorbis_File *vf, ogg_int64_t pos)) \
    MIX_LOADER_FUNCTION(true,int,ov_raw_seek,(OggVorbis_File *vf, ogg_int64_t pos)) \
    MIX_LOADER_FUNCTION(true,ogg_int64_t,ov_pcm_tell,(OggVorbis_File *vf)) \

#ifdef VORBIS_USE_TREMOR
    #define MIX_LOADER_FUNCTIONS \
        MIX_LOADER_FUNCTIONS_vorbisbase \
        MIX_LOADER_FUNCTION(true,long,ov_read,(OggVorbis_File *vf,char *buffer,int length, int *bitstream))
#else
    #define MIX_LOADER_FUNCTIONS \
        MIX_LOADER_FUNCTIONS_vorbisbase \
        MIX_LOADER_FUNCTION(true,long,ov_read_float,(OggVorbis_File *vf, float ***pcm_channels, int samples, int *bitstream))
#endif

#define MIX_LOADER_MODULE vorbis
#include "SDL_mixer_loader.h"


#ifdef VORBIS_USE_TREMOR
#define VORBIS_AUDIO_FORMAT SDL_AUDIO_S16
#else
#define VORBIS_AUDIO_FORMAT SDL_AUDIO_F32
#endif

typedef struct VORBIS_AudioData
{
    size_t framesize;
    MIX_OggLoop loop;
} VORBIS_AudioData;

typedef struct VORBIS_TrackData
{
    const VORBIS_AudioData *adata;
    OggVorbis_File vf;
    int current_channels;
    int current_freq;
    int current_bitstream;
    Sint64 current_iteration;
    Sint64 current_iteration_frames;
} VORBIS_TrackData;


static bool SDLCALL VORBIS_init(void)
{
    return LoadModule_vorbis();
}

static void SDLCALL VORBIS_quit(void)
{
    UnloadModule_vorbis();
}

static bool SetOggVorbisError(const char *function, int error)
{
    switch (error) {
        #define HANDLE_ERROR_CASE(X) case X: return SDL_SetError("%s: %s", function, #X)
        HANDLE_ERROR_CASE(OV_FALSE);
        HANDLE_ERROR_CASE(OV_EOF);
        HANDLE_ERROR_CASE(OV_HOLE);
        HANDLE_ERROR_CASE(OV_EREAD);
        HANDLE_ERROR_CASE(OV_EFAULT);
        HANDLE_ERROR_CASE(OV_EIMPL);
        HANDLE_ERROR_CASE(OV_EINVAL);
        HANDLE_ERROR_CASE(OV_ENOTVORBIS);
        HANDLE_ERROR_CASE(OV_EBADHEADER);
        HANDLE_ERROR_CASE(OV_EVERSION);
        HANDLE_ERROR_CASE(OV_ENOTAUDIO);
        HANDLE_ERROR_CASE(OV_EBADPACKET);
        HANDLE_ERROR_CASE(OV_EBADLINK);
        HANDLE_ERROR_CASE(OV_ENOSEEK);
        #undef HANDLE_ERROR_CASE
        default: break;
    }
    return SDL_SetError("%s: unknown error %d", function, error);
}

static size_t VORBIS_IoRead(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    if (size > 0 && nmemb > 0) {
        return SDL_ReadIO((SDL_IOStream*)datasource, ptr, size * nmemb) / size;
    }
    return 0;
}

static int VORBIS_IoSeek(void *datasource, ogg_int64_t offset, int whence)
{
    return (SDL_SeekIO((SDL_IOStream*)datasource, offset, whence) < 0) ? -1 : 0;
}

static long VORBIS_IoTell(void *datasource)
{
    return (long) SDL_TellIO((SDL_IOStream*)datasource);
}

static int VORBIS_IoClose(void *datasource)
{
    (void)datasource;
    return 0;
}

static const ov_callbacks VORBIS_IoCallbacks = { VORBIS_IoRead, VORBIS_IoSeek, VORBIS_IoClose, VORBIS_IoTell };


static bool SDLCALL VORBIS_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    // just load the bare minimum from the IOStream to verify it's an Ogg Vorbis file.
    // !!! FIXME: is ov_open_callbacks going to return more slowly if this isn't an Opus file? It's probably better to just do the full open.
    OggVorbis_File vf;
    if (vorbis.ov_test_callbacks(io, &vf, NULL, 0, VORBIS_IoCallbacks) < 0) {
        return SDL_SetError("Not an Ogg Vorbis audio stream");
    }
    vorbis.ov_clear(&vf);

    // Go back and do a proper load now to get metadata.
    if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {
        return false;
    }

    VORBIS_AudioData *adata = (VORBIS_AudioData *) SDL_calloc(1, sizeof (*adata));
    if (!adata) {
        return false;
    }

    // now open the stream for serious processing.
    const int rc = vorbis.ov_open_callbacks(io, &vf, NULL, 0, VORBIS_IoCallbacks);
    if (rc < 0) {
        SDL_free(adata);
        return SetOggVorbisError("ov_open_callbacks", rc);
    }

    const vorbis_info *vi = vorbis.ov_info(&vf, -1);
    if (!vi) {
        vorbis.ov_clear(&vf);
        SDL_free(adata);
        return SDL_SetError("Couldn't get Ogg Vorbis info; corrupt data?");
    }

    spec->format = VORBIS_AUDIO_FORMAT;
    spec->channels = vi->channels;
    spec->freq = vi->rate;

    adata->framesize = SDL_AUDIO_FRAMESIZE(*spec);

    vorbis_comment *vc = vorbis.ov_comment(&vf, -1);
    if (vc != NULL) {
        MIX_ParseOggComments(props, spec->freq, vc->vendor, (const char * const *) vc->user_comments, vc->comments, &adata->loop);
    }

    vorbis.ov_raw_seek(&vf, 0);  // !!! FIXME: it's not clear if this seek is necessary, but https://stackoverflow.com/a/72482773 suggests it might be, at least on older libvorbisfile releases...
    const Sint64 full_length = (Sint64) vorbis.ov_pcm_total(&vf, -1);

    if (adata->loop.end > full_length) {
        adata->loop.active = false;
    }

    vorbis.ov_clear(&vf);  // done with this instance. Tracks will maintain their own OggVorbis_File object.

    if (adata->loop.active) {
        *duration_frames = (adata->loop.count < 0) ? MIX_DURATION_INFINITE : (full_length * adata->loop.count);
    } else {
        *duration_frames = full_length;
    }

    *audio_userdata = adata;

    return true;
}

static bool SDLCALL VORBIS_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    VORBIS_TrackData *tdata = (VORBIS_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    const VORBIS_AudioData *adata = (const VORBIS_AudioData *) audio_userdata;

    // now open the stream for serious processing.
    int rc = vorbis.ov_open_callbacks(io, &tdata->vf, NULL, 0, VORBIS_IoCallbacks);
    if (rc < 0) {
        SDL_free(tdata);
        return SetOggVorbisError("ov_open_callbacks", rc);
    }

    tdata->current_channels = spec->channels;
    tdata->current_freq = spec->freq;
    tdata->current_bitstream = -1;
    tdata->current_iteration = -1;
    tdata->adata = adata;

    *track_userdata = tdata;

    return true;
}

static void UpdateVorbisStreamFormat(SDL_AudioStream *stream, const SDL_AudioSpec *spec)
{
    static const int vorbis_chmap_3[] = { 0, 2, 1, -1, -1, -1 };  // SDL=FL,FR,FC,LFE,RL,RR  VORBIS=FL,FC,FR
    static const int vorbis_chmap_5[] = { 0, 2, 1, 4, 5, -1};  // SDL=FL,FR,FC,LFE,RL,RR  VORBIS=FL,FC,FR,RL,RR
    static const int vorbis_chmap_6[] = { 0, 2, 1, 4, 5, 3 };  // SDL=FL,FR,FC,LFE,RL,RR  VORBIS=FL,FC,FR,RL,RR,LFE
    static const int vorbis_chmap_7[] = { 0, 2, 1, 5, 6, 4, 3 };  // SDL=FL,FR,FC,LFE,RC,SL,SR  VORBIS=FL,FC,FR,SL,SR,RC,LFE
    static const int vorbis_chmap_8[] = { 0, 2, 1, 6, 7, 4, 5, 3 };  // SDL=FL,FR,FC,LFE,RL,RR,SL,SR  VORBIS=FL,FC,FR,SL,SR,RL,RR,LFE

    SDL_AudioSpec bumped_spec;
    const int *chmap = NULL;
    switch (spec->channels) {
        // 3 and 5 channel audio offers a Front Center channel, which SDL doesn't have until 5.1 (6 channels), so bump the spec
        //  to 5.1 and provide NULL planar channels where appropriate.
        // !!! FIXME: Since Tremor doesn't provide planar data, we just pass this through in the wrong channel order there,
        // !!! FIXME: which is dumb but who is using these formats in the first place, let alone with low-powered Tremor?
        #ifndef VORBIS_USE_TREMOR
        case 3:
            SDL_copyp(&bumped_spec, spec);
            bumped_spec.channels = 6;
            spec = &bumped_spec;
            chmap = vorbis_chmap_3;
            break;
        case 5:
            SDL_copyp(&bumped_spec, spec);
            bumped_spec.channels = 6;
            spec = &bumped_spec;
            chmap = vorbis_chmap_5;
            break;
        #endif

        case 6: chmap = vorbis_chmap_6; break;
        case 7: chmap = vorbis_chmap_7; break;
        case 8: chmap = vorbis_chmap_8; break;
        default:
            // no special mapping, pass through as-is.
            break;
    }

    SDL_SetAudioStreamFormat(stream, spec, NULL);
    SDL_SetAudioStreamInputChannelMap(stream, chmap, spec->channels);
}


static bool SDLCALL VORBIS_seek(void *track_userdata, Uint64 frame);

static bool SDLCALL VORBIS_decode(void *track_userdata, SDL_AudioStream *stream)
{
    VORBIS_TrackData *tdata = (VORBIS_TrackData *) track_userdata;
    int bitstream = tdata->current_bitstream;

#ifdef VORBIS_USE_TREMOR
    const size_t framesize = tdata->adata->framesize;
    Uint8 samples[256];
    int amount = (int)vorbis.ov_read(&tdata->vf, (char *) samples, sizeof (samples), &bitstream);
    if (amount < 0) {
        return SetOggVorbisError("ov_read", amount);
    }
    amount *= framesize;
#else
    float **pcm_channels = NULL;
    int amount = (int)vorbis.ov_read_float(&tdata->vf, &pcm_channels, 256, &bitstream);
    if (amount < 0) {
        return SetOggVorbisError("ov_read_float", amount);
    }
#endif

    if (bitstream != tdata->current_bitstream) {
        const vorbis_info *vi = vorbis.ov_info(&tdata->vf, -1);
        if (vi) {  // this _shouldn't_ be NULL, but if it is, we're just going on without it and hoping the stream format didn't change.
            if ((tdata->current_channels != vi->channels) || (tdata->current_freq != vi->rate)) {
                const SDL_AudioSpec spec = { VORBIS_AUDIO_FORMAT, vi->channels, vi->rate };
                tdata->current_channels = vi->channels;
                tdata->current_freq = vi->rate;
                UpdateVorbisStreamFormat(stream, &spec);
            }
        }
        tdata->current_bitstream = bitstream;
    }

    if (amount == 0) {
        return false;  // EOF
    }

    const MIX_OggLoop *loop = &tdata->adata->loop;
    if (tdata->current_iteration < 0) {
        if (loop->active && ((tdata->current_iteration_frames + amount) >= loop->start)) {
            tdata->current_iteration = 0;  // we've hit the start of the loop point.
            tdata->current_iteration_frames = (tdata->current_iteration_frames - loop->start);  // so adding `amount` corrects this later.
        }
    }

    if (tdata->current_iteration >= 0) {
        SDL_assert(loop->active);
        SDL_assert(tdata->current_iteration_frames <= loop->len);
        const Sint64 available = loop->len - tdata->current_iteration_frames;
        if (amount > available) {
            amount = available;
        }

        SDL_assert(tdata->current_iteration_frames <= loop->len);
        if ((tdata->current_iteration_frames + amount) >= loop->len) {  // time to loop?
            bool should_loop = false;
            if (loop->count < 0) {  // negative==infinite loop
                tdata->current_iteration = 0;
                should_loop = true;
            } else {
                tdata->current_iteration++;
                SDL_assert(tdata->current_iteration <= loop->count);
                if (tdata->current_iteration < loop->count) {
                    should_loop = true;
                }
            }

            if (should_loop) {
                const Uint64 nextframe = ((Uint64) loop->start) + ( ((Uint64) loop->len) * ((Uint64) tdata->current_iteration) );
                if (!VORBIS_seek(tdata, nextframe)) {
                    return false;
                }
            } else {
                tdata->current_iteration = -1;
            }
            tdata->current_iteration_frames = 0;
        }
    }

    if (amount > 0) {
        #ifdef VORBIS_USE_TREMOR
        SDL_PutAudioStreamData(stream, samples, amount / framesize);
        #else
        SDL_PutAudioStreamPlanarData(stream, (const void * const *) pcm_channels, tdata->current_channels, amount);
        #endif
        tdata->current_iteration_frames += amount;
    }

    return true;  // had more data to decode.
}

static bool SDLCALL VORBIS_seek(void *track_userdata, Uint64 frame)
{
    VORBIS_TrackData *tdata = (VORBIS_TrackData *) track_userdata;
    const MIX_OggLoop *loop = &tdata->adata->loop;
    Sint64 final_iteration = -1;
    Sint64 final_iteration_frames = 0;

    // frame has hit the loop point?
    if (loop->active && ((Sint64)frame >= loop->start)) {
        // figure out the _actual_ frame in the vorbis file we're aiming for.
        if ((loop->count < 0) || ((Sint64)frame < (loop->len * loop->count))) {  // literally in the loop right now.
            frame -= loop->start;  // make logical frame index relative to start of loop.
            final_iteration = (loop->count < 0) ? 0 : (frame / loop->len);  // decide what iteration of the loop we're on (stays at zero for infinite loops).
            frame %= loop->len;  // drop iterations so we're an offset into the loop.
            final_iteration_frames = frame;
            frame += loop->start;  // convert back into physical frame index.
        } else {  // past the loop point?
            SDL_assert(loop->count > 0);  // can't be infinite loop if we passed it.
            frame -= loop->len * loop->count;  // drop the iterations to get the physical frame index.
        }
    }

    // !!! FIXME: I assume ov_raw_seek is faster if we're seeking to start, but I could be wrong.
    const int rc = (frame == 0) ? vorbis.ov_raw_seek(&tdata->vf, 0) : vorbis.ov_pcm_seek(&tdata->vf, (ogg_int64_t) frame);
    if (rc != 0) {
        return SetOggVorbisError("ov_pcm_seek", rc);
    }

    tdata->current_iteration = final_iteration;
    tdata->current_iteration_frames = final_iteration_frames;

    return true;
}

static void SDLCALL VORBIS_quit_track(void *track_userdata)
{
    VORBIS_TrackData *tdata = (VORBIS_TrackData *) track_userdata;
    vorbis.ov_clear(&tdata->vf);
    SDL_free(tdata);
}

static void SDLCALL VORBIS_quit_audio(void *audio_userdata)
{
    SDL_free(audio_userdata);
}

const MIX_Decoder MIX_Decoder_VORBIS = {
    "VORBIS",
    VORBIS_init,
    VORBIS_init_audio,
    VORBIS_init_track,
    VORBIS_decode,
    VORBIS_seek,
    NULL,  // jump_to_order
    VORBIS_quit_track,
    VORBIS_quit_audio,
    VORBIS_quit
};

#endif

