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

// This file supports Ogg Opus audio streams using libopusfile.

#ifdef DECODER_OPUS

#include "SDL_mixer_internal.h"

#include <opusfile.h>

#if defined(OPUS_DYNAMIC) && defined(SDL_ELF_NOTE_DLOPEN)
SDL_ELF_NOTE_DLOPEN(
    "opus",
    "Support for OPUS audio using opusfile",
    SDL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
    OPUS_DYNAMIC
)
#endif

#ifdef OPUS_DYNAMIC
#define MIX_LOADER_DYNAMIC OPUS_DYNAMIC
#endif

#define MIX_LOADER_FUNCTIONS \
    MIX_LOADER_FUNCTION(true,const OpusTags *,op_tags,(const OggOpusFile *,int)) \
    MIX_LOADER_FUNCTION(true,OggOpusFile *,op_open_callbacks,(void *,const OpusFileCallbacks *,const unsigned char *,size_t,int *)) \
    MIX_LOADER_FUNCTION(true,OggOpusFile *,op_test_callbacks,(void *,const OpusFileCallbacks *,const unsigned char *,size_t,int *)) \
    MIX_LOADER_FUNCTION(true,void,op_free,(OggOpusFile *)) \
    MIX_LOADER_FUNCTION(true,const OpusHead *,op_head,(const OggOpusFile *,int)) \
    MIX_LOADER_FUNCTION(true,int,op_seekable,(const OggOpusFile *)) \
    MIX_LOADER_FUNCTION(true,int,op_read_float,(OggOpusFile *, float *,int,int *)) \
    MIX_LOADER_FUNCTION(true,int,op_raw_seek,(OggOpusFile *,opus_int64)) \
    MIX_LOADER_FUNCTION(true,int,op_pcm_seek,(OggOpusFile *,ogg_int64_t)) \
    MIX_LOADER_FUNCTION(true,ogg_int64_t,op_pcm_tell,(const OggOpusFile *)) \
    MIX_LOADER_FUNCTION(true,ogg_int64_t,op_pcm_total,(const OggOpusFile *, int)) \

#define MIX_LOADER_MODULE opus
#include "SDL_mixer_loader.h"


typedef struct OPUS_AudioData
{
    MIX_OggLoop loop;
} OPUS_AudioData;

typedef struct OPUS_TrackData
{
    const OPUS_AudioData *adata;
    OggOpusFile *of;
    int current_channels;
    int current_bitstream;
    Sint64 current_iteration;
    Sint64 current_iteration_frames;
} OPUS_TrackData;


static bool SDLCALL OPUS_init(void)
{
    return LoadModule_opus();
}

static void SDLCALL OPUS_quit(void)
{
    UnloadModule_opus();
}

static bool set_op_error(const char *function, int error)
{
    #define HANDLE_ERROR_CASE(X) case X: return SDL_SetError("%s: %s", function, #X)
    switch (error) {
        HANDLE_ERROR_CASE(OP_FALSE);
        HANDLE_ERROR_CASE(OP_EOF);
        HANDLE_ERROR_CASE(OP_HOLE);
        HANDLE_ERROR_CASE(OP_EREAD);
        HANDLE_ERROR_CASE(OP_EFAULT);
        HANDLE_ERROR_CASE(OP_EIMPL);
        HANDLE_ERROR_CASE(OP_EINVAL);
        HANDLE_ERROR_CASE(OP_ENOTFORMAT);
        HANDLE_ERROR_CASE(OP_EBADHEADER);
        HANDLE_ERROR_CASE(OP_EVERSION);
        HANDLE_ERROR_CASE(OP_ENOTAUDIO);
        HANDLE_ERROR_CASE(OP_EBADPACKET);
        HANDLE_ERROR_CASE(OP_EBADLINK);
        HANDLE_ERROR_CASE(OP_ENOSEEK);
        HANDLE_ERROR_CASE(OP_EBADTIMESTAMP);
    #undef HANDLE_ERROR_CASE
    default: break;
    }
    return SDL_SetError("%s: unknown error %d", function, error);
}

static int OPUS_IoRead(void *datasource, unsigned char *ptr, int size)
{
    return SDL_ReadIO((SDL_IOStream*)datasource, ptr, (size_t) size);
}

static int OPUS_IoSeek(void *datasource, opus_int64 offset, int whence)
{
    return (SDL_SeekIO((SDL_IOStream*)datasource, offset, whence) < 0) ? -1 : 0;
}

static opus_int64 OPUS_IoTell(void *datasource)
{
    return SDL_TellIO((SDL_IOStream*)datasource);
}

static int OPUS_IoClose(void *datasource)
{
    (void)datasource;
    return 0;
}

static const OpusFileCallbacks OPUS_IoCallbacks = { OPUS_IoRead, OPUS_IoSeek, OPUS_IoTell, OPUS_IoClose };


static bool SDLCALL OPUS_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    // just load the bare minimum from the IOStream to verify it's an Opus file.
    // !!! FIXME: is op_open_callbacks going to return more slowly if this isn't an Opus file? It's probably better to just do the full open.
    int rc = 0;
    OggOpusFile *of = opus.op_test_callbacks(io, &OPUS_IoCallbacks, NULL, 0, &rc);
    if (!of) {
        return SDL_SetError("Not an Opus audio stream");
    }
    opus.op_free(of);

    // Go back and do a proper load now to get metadata.
    if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {
        return false;
    }

    OPUS_AudioData *adata = (OPUS_AudioData *) SDL_calloc(1, sizeof (*adata));
    if (!adata) {
        return false;
    }

    // now open the stream for serious processing.
    of = opus.op_open_callbacks(io, &OPUS_IoCallbacks, NULL, 0, &rc);
    if (!of) {
        SDL_free(adata);
        return set_op_error("ov_open_callbacks", rc);
    }

    const OpusHead *info = opus.op_head(of, -1);
    if (!info) {
        opus.op_free(of);
        SDL_free(adata);
        return SDL_SetError("Couldn't get Opus info; corrupt data?");
    }

    spec->format = SDL_AUDIO_F32;
    spec->channels = info->channel_count;
    spec->freq = 48000;  // libopus only outputs at 48kHz.

    const OpusTags *tags = opus.op_tags(of, -1);
    if (tags != NULL) {
        MIX_ParseOggComments(props, spec->freq, tags->vendor, (const char * const *) tags->user_comments, tags->comments, &adata->loop);
    }

    opus.op_raw_seek(of, 0);  // !!! FIXME: it's not clear if this seek is necessary, but https://stackoverflow.com/a/72482773 suggests it might be, at least on older libvorbisfile releases...
    const Sint64 full_length = (Sint64) opus.op_pcm_total(of, -1);

    if (adata->loop.end > full_length) {
        adata->loop.active = false;
    }

    opus.op_free(of);  // done with this instance. Tracks will maintain their own OggOpusFile object.

    if (adata->loop.active) {
        *duration_frames = (adata->loop.count < 0) ? MIX_DURATION_INFINITE : (full_length * adata->loop.count);
    } else {
        *duration_frames = full_length;
    }

    *audio_userdata = adata;

    return true;
}

static bool SDLCALL OPUS_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    OPUS_TrackData *tdata = (OPUS_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    const OPUS_AudioData *adata = (const OPUS_AudioData *) audio_userdata;

    // now open the stream for serious processing.
    int rc = 0;
    tdata->of = opus.op_open_callbacks(io, &OPUS_IoCallbacks, NULL, 0, &rc);
    if (!tdata->of) {
        SDL_free(tdata);
        return set_op_error("op_open_callbacks", rc);
    }

    tdata->current_channels = spec->channels;
    tdata->current_bitstream = -1;
    tdata->current_iteration = -1;
    tdata->adata = adata;

    *track_userdata = tdata;

    return true;
}

static bool SDLCALL OPUS_seek(void *track_userdata, Uint64 frame);

static bool SDLCALL OPUS_decode(void *track_userdata, SDL_AudioStream *stream)
{
    OPUS_TrackData *tdata = (OPUS_TrackData *) track_userdata;
    int bitstream = tdata->current_bitstream;
    float samples[256];

    const int channels = tdata->current_channels;
    int amount = opus.op_read_float(tdata->of, samples, SDL_arraysize(samples), &bitstream);
    if (amount < 0) {
        return set_op_error("op_read_float", amount);
    } else if (amount == 0) {
        return false;  // EOF
    }

    SDL_assert((amount * channels) <= (int)SDL_arraysize(samples));

    if (bitstream != tdata->current_bitstream) {
        const OpusHead *info = opus.op_head(tdata->of, -1);
        if (info) {  // this _shouldn't_ be NULL, but if it is, we're just going on without it and hoping the stream format didn't change.
            if (tdata->current_channels != info->channel_count) {
                const SDL_AudioSpec spec = { SDL_AUDIO_F32, info->channel_count, 48000 };
                SDL_SetAudioStreamFormat(stream, &spec, NULL);
                tdata->current_channels = info->channel_count;
            }
        }
        tdata->current_bitstream = bitstream;
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
                if (!OPUS_seek(tdata, nextframe)) {
                    return false;
                }
            } else {
                tdata->current_iteration = -1;
            }
            tdata->current_iteration_frames = 0;
        }
    }

    if (amount > 0) {
        SDL_PutAudioStreamData(stream, samples, amount * tdata->current_channels * sizeof (float));
        tdata->current_iteration_frames += amount;
    }

    return true;  // had more data to decode.
}

static bool SDLCALL OPUS_seek(void *track_userdata, Uint64 frame)
{
    OPUS_TrackData *tdata = (OPUS_TrackData *) track_userdata;
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

    // !!! FIXME: I assume op_raw_seek is faster if we're seeking to start, but I could be wrong.
    const int rc = (frame == 0) ? opus.op_raw_seek(tdata->of, 0) : opus.op_pcm_seek(tdata->of, (Sint64) frame);
    if (rc != 0) {
        return set_op_error("op_pcm_seek", rc);
    }

    tdata->current_iteration = final_iteration;
    tdata->current_iteration_frames = final_iteration_frames;

    return true;
}

static void SDLCALL OPUS_quit_track(void *track_userdata)
{
    OPUS_TrackData *tdata = (OPUS_TrackData *) track_userdata;
    opus.op_free(tdata->of);
    SDL_free(tdata);
}

static void SDLCALL OPUS_quit_audio(void *audio_userdata)
{
    OPUS_AudioData *adata = (OPUS_AudioData *) audio_userdata;
    SDL_free(adata);
}

const MIX_Decoder MIX_Decoder_OPUS = {
    "OPUS",
    OPUS_init,
    OPUS_init_audio,
    OPUS_init_track,
    OPUS_decode,
    OPUS_seek,
    NULL,  // jump_to_order
    OPUS_quit_track,
    OPUS_quit_audio,
    OPUS_quit
};

#endif
