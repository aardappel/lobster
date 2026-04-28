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

// !!! FIXME: figure out `int` vs Sint64/Uint64 metrics in all of this.

#include "SDL_mixer_internal.h"

// !!! FIXME: should RAW go first (only needs to check if it was explicitly
// !!! FIXME: requested), and SINEWAVE last (must be requested, likely rare).
static const MIX_Decoder *decoders[] = {
    #ifdef DECODER_WAV
    &MIX_Decoder_WAV,
    #endif
    #ifdef DECODER_OGGVORBIS_VORBISFILE
    &MIX_Decoder_VORBIS,
    #endif
    #ifdef DECODER_OGGVORBIS_STB
    &MIX_Decoder_STBVORBIS,
    #endif
    #ifdef DECODER_OPUS
    &MIX_Decoder_OPUS,
    #endif
    #ifdef DECODER_FLAC_LIBFLAC
    &MIX_Decoder_FLAC,
    #endif
    #ifdef DECODER_FLAC_DRFLAC
    &MIX_Decoder_DRFLAC,
    #endif
    #ifdef DECODER_MIDI_FLUIDSYNTH
    &MIX_Decoder_FLUIDSYNTH,
    #endif
    #ifdef DECODER_MIDI_TIMIDITY
    &MIX_Decoder_TIMIDITY,
    #endif
    #ifdef DECODER_WAVPACK
    &MIX_Decoder_WAVPACK,
    #endif
    #ifdef DECODER_GME
    &MIX_Decoder_GME,
    #endif
    #ifdef DECODER_MOD_XMP
    &MIX_Decoder_XMP,
    #endif
    #ifdef DECODER_VOC
    &MIX_Decoder_VOC,
    #endif
    #ifdef DECODER_AIFF
    &MIX_Decoder_AIFF,
    #endif
    #ifdef DECODER_AU
    &MIX_Decoder_AU,
    #endif
    #ifdef DECODER_MP3_MPG123
    &MIX_Decoder_MPG123,
    #endif
    #ifdef DECODER_MP3_DRMP3
    &MIX_Decoder_DRMP3,
    #endif

    // these are always available.
    &MIX_Decoder_SINEWAVE,
    &MIX_Decoder_RAW
};

static const MIX_Decoder *available_decoders[SDL_arraysize(decoders)];
static int num_available_decoders = 0;
static int mixer_initialized = 0;
static MIX_Mixer *all_mixers = NULL;
static MIX_Audio *all_audios = NULL;
static MIX_AudioDecoder *all_audiodecoders = NULL;
static SDL_Mutex *global_lock = NULL;

#if defined(SDL_NEON_INTRINSICS) && SDL_MIXER_NEED_SCALAR_FALLBACK
bool MIX_HasNEON = false;
#endif


static void LockGlobal(void)
{
    SDL_LockMutex(global_lock);
}

static void UnlockGlobal(void)
{
    SDL_UnlockMutex(global_lock);
}

static void LockMixer(MIX_Mixer *mixer)
{
    SDL_LockAudioStream(mixer->output_stream);
}

static void UnlockMixer(MIX_Mixer *mixer)
{
    SDL_UnlockAudioStream(mixer->output_stream);
}

static void LockTrack(MIX_Track *track)
{
    SDL_assert(track != NULL);
    SDL_assert(track->output_stream != NULL);
    SDL_LockAudioStream(track->output_stream);
}

static void UnlockTrack(MIX_Track *track)
{
    SDL_assert(track != NULL);
    SDL_assert(track->output_stream != NULL);
    SDL_UnlockAudioStream(track->output_stream);
}

static bool CheckInitialized(void)
{
    if (!mixer_initialized) {
        return SDL_SetError("Mixer not initialized (call MIX_Init first)");
    }
    return true;
}

#define CHECKPARAMFUNC(type, name, sym) \
    static bool Check##name##Param(type *sym) { \
        if (!CheckInitialized()) { \
            return false; \
        } else if (!sym) { \
            return SDL_InvalidParamError(#sym); \
        } \
        return true; \
    } \

CHECKPARAMFUNC(MIX_Mixer, Mixer, mixer)
CHECKPARAMFUNC(MIX_Track, Track, track)
CHECKPARAMFUNC(MIX_Audio, Audio, audio)
CHECKPARAMFUNC(MIX_Group, Group, group)
CHECKPARAMFUNC(MIX_AudioDecoder, AudioDecoder, audiodecoder)

#undef CHECKPARAMFUNC

#define CHECKTAGPLUSPARAM(type, name, sym) \
    static bool Check##name##TagParam(type *sym, const char *tag) { \
        if (!CheckInitialized()) { \
            return false; \
        } else if (!sym) { \
            return SDL_InvalidParamError(#sym); \
        } else if (!tag) { \
            return SDL_InvalidParamError("tag"); \
        } \
        return true; \
    } \

CHECKTAGPLUSPARAM(MIX_Mixer, Mixer, mixer)
CHECKTAGPLUSPARAM(MIX_Track, Track, track)

#undef CHECKTAGPLUSPARAM


static bool SetTrackOutputStreamFormat(MIX_Track *track, const SDL_AudioSpec *spec)
{
    SDL_copyp(&track->output_spec, &track->mixer->spec);
    if (track->spatialization_mode == MIX_SPATIALIZATION_3D) {
        track->output_spec.channels = 1;
    } else if (track->spatialization_mode == MIX_SPATIALIZATION_STEREO) {
        track->output_spec.channels = 2;
    }

    const bool retval = SDL_SetAudioStreamFormat(track->output_stream, spec, &track->output_spec);   // input is `spec`, output is to mixer->output_stream (or, if spatializing, to mixer->output_stream but mono...if force_stereo, output_stream but stereo).
    SDL_assert(retval != false);
    return retval;
}

// catch events to see if output device format has changed. This can let us move to/from surround sound support on the fly, not to mention spend less time doing unnecessary conversions.
static bool SDLCALL AudioDeviceChangeEventWatcher(void *userdata, SDL_Event *event)
{
    MIX_Mixer *mixer = (MIX_Mixer *) userdata;
    if (event->type != SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED) {
        return true;  // don't care about this event.
    } else if (mixer->device_id != event->adevice.which) {
        return true;  // don't care about this device.
    } else if (mixer->device_id == 0) {
        return true;  // don't care about this mixer.
    }

    LockMixer(mixer);

    // adjust all our output streams to the new format.
    if (SDL_GetAudioStreamFormat(mixer->output_stream, NULL, &mixer->spec)) {
        mixer->spec.format = SDL_AUDIO_F32;
        if (SDL_SetAudioStreamFormat(mixer->output_stream, &mixer->spec, NULL)) {
            MIX_VBAP2D_Init(&mixer->vbap2d, mixer->spec.channels);  // deal with channel count changing.
            for (MIX_Track *track = mixer->all_tracks; track; track = track->next) {
                LockTrack(track);
                SetTrackOutputStreamFormat(track, NULL);   // input is from internal_stream, output is to mixer->output_stream (or, if spatializing, to mixer->output_stream but mono).
                if (track->spatialization_mode == MIX_SPATIALIZATION_3D) {  // deal with channel count changing.
                    MIX_Spatialize(&track->mixer->vbap2d, track->position3d, track->spatialization_panning, track->spatialization_speakers);
                }
                UnlockTrack(track);
            }
        }
    }

    UnlockMixer(mixer);
    return true;
}

// this assumes LockTrack(track) was called before this.
static void TrackStopped(MIX_Track *track)
{
    SDL_assert(track->state != MIX_STATE_STOPPED);  // shouldn't be already stopped at this point.
    track->state = MIX_STATE_STOPPED;
    if (track->stopped_callback) {
        track->stopped_callback(track->stopped_callback_userdata, track);
    }
    if (track->fire_and_forget) {
        SDL_assert(!track->stopped_callback);  // these shouldn't have stopped callbacks.
        SDL_assert(track->state == MIX_STATE_STOPPED);  // should not have changed, shouldn't have a stopped_callback, etc.
        SDL_assert(track->fire_and_forget_next == NULL);  // shouldn't be in the list at all right now.
        MIX_SetTrackAudio(track, NULL);
        MIX_Mixer *mixer = track->mixer;
        LockMixer(mixer);
        track->fire_and_forget_next = mixer->fire_and_forget_pool;
        mixer->fire_and_forget_pool = track;
        UnlockMixer(mixer);
    }
}

static void ApplyFade(MIX_Track *track, int channels, float *pcm, int frames)
{
    // !!! FIXME: this is probably pretty naive.

    if (track->fade_direction == 0) {
        return;  // no fade is happening, early exit.
    }

    const int to_be_faded = (int) SDL_min(track->fade_frames, frames);
    const int total_fade_frames = (int) track->total_fade_frames;
    int fade_frame_position = total_fade_frames - (int) track->fade_frames;

    // some hacks to avoid a branch on each sample frame. Might not be a good idea in practice.
    const float fade_start_gain = track->fade_start_gain;
    const float pctmult = (1.0f - fade_start_gain) * ((track->fade_direction < 0) ? 1.0f : -1.0f);
    const float pctsub = (track->fade_direction < 0) ? 1.0f : 0.0f;
    const float ftotal_fade_frames = (float) total_fade_frames;

    SDL_assert((fade_start_gain == 0.0f) || (track->fade_direction > 0));  // we only allow fade _in_ from arbitrary levels. Fade out always operates on the full signal down to zero.

    for (int i = 0; i < to_be_faded; i++) {
        const float pct = ((pctsub - (((float) fade_frame_position) / ftotal_fade_frames)) * pctmult) + fade_start_gain;
        SDL_assert(pct >= 0.0f);
        SDL_assert(pct <= 1.0f);
        fade_frame_position++;

        // use this fade percentage for the entire sample frame.
        switch (channels) {   // !!! FIXME: profile this and see if this is a dumb idea.
            case 8: *(pcm++) *= pct; SDL_FALLTHROUGH;
            case 7: *(pcm++) *= pct; SDL_FALLTHROUGH;
            case 6: *(pcm++) *= pct; SDL_FALLTHROUGH;
            case 5: *(pcm++) *= pct; SDL_FALLTHROUGH;
            case 4: *(pcm++) *= pct; SDL_FALLTHROUGH;
            case 3: *(pcm++) *= pct; SDL_FALLTHROUGH;
            case 2: *(pcm++) *= pct; SDL_FALLTHROUGH;
            case 1: *(pcm++) *= pct; break;

            default:  // catch any other number of channels.
                for (int j = 0; j < channels; j++) {
                    *(pcm++) *= pct;
                }
                break;
        }
    }

    track->fade_frames -= to_be_faded;
    SDL_assert(track->fade_frames >= 0);
    if (track->fade_frames == 0) {  // fade is done.
        if (track->fade_direction < 0) {
            track->loops_remaining = 0;  // we were fading out, don't loop anymore.
        }
        track->fade_direction = 0;
    }
}

static bool DecodeMore(MIX_Track *track, int bytes_needed)
{
    SDL_assert(track->input_audio != NULL);

    bool retval = true;
    while (SDL_GetAudioStreamAvailable(track->input_stream) < bytes_needed) {
        if (!track->input_audio->decoder->decode(track->decoder_userdata, track->input_stream)) {
            SDL_FlushAudioStream(track->input_stream);  // make sure we read _everything_ now.
            retval = false;
            break;
        }
    }

    return retval;
}

static int FillSilenceFrames(MIX_Track *track, void *buffer, int channels, int buflen)
{
    SDL_assert(track->silence_frames > 0);
    SDL_assert(buflen > 0);
    const int max_silence_bytes = (int) (track->silence_frames * channels * sizeof (float));
    const int br = SDL_min(buflen, max_silence_bytes);
    if (br) {
        SDL_memset(buffer, '\0', br);
        track->silence_frames -= br / (channels * sizeof (float));
    }
    return br;
}

// This is called every time we try to pull more from a track's output_stream.
// We generate more audio here on-demand, either from a decoder, or pulling
// from another audio stream.
// track->output_stream is locked when calling this.
static void SDLCALL TrackGetCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    MIX_Track *track = (MIX_Track *) userdata;
    SDL_assert(stream == track->output_stream);

    if (additional_amount == 0) {
        return;  // don't need to generate more audio yet.
    } else if (track->state != MIX_STATE_PLAYING) {
        return;  // paused or stopped, don't make progress.
    }

    SDL_assert(track->output_spec.format == SDL_AUDIO_F32);
    SDL_assert(track->output_spec.freq == track->mixer->spec.freq);

    SDL_AudioSpec raw_spec;
    if (track->input_stream) {
        SDL_GetAudioStreamFormat(track->input_stream, NULL, &raw_spec);
    }

    // do we need to grow our buffer?
    if ((unsigned)additional_amount > track->input_buffer_len) {
        void *ptr = SDL_realloc(track->input_buffer, additional_amount);
        if (!ptr) {   // uhoh.
            TrackStopped(track);
            return;  // not much to be done, we're out of memory!
        }
        track->input_buffer = (float *) ptr;
        track->input_buffer_len = additional_amount;
    }

    float *pcm = track->input_buffer;  // we always work in float32 format.
    const int output_framesize = SDL_AUDIO_FRAMESIZE(track->output_spec);
    int bytes_remaining = additional_amount;

    // Calling TrackStopped() might have a stopped_callback that restarts the track, so don't break the loop
    //  for simply being stopped, so we can generate audio without gaps. If not restarted, track->state will no longer be PLAYING.
    while ((track->state == MIX_STATE_PLAYING) && (bytes_remaining > 0)) {
        bool end_of_audio = false;
        int br = 0;   // bytes read.

        // make sure we're not trying to read half a sample frame.
        bytes_remaining = SDL_max(bytes_remaining, output_framesize);

        if (track->silence_frames > 0) {
            SDL_assert(track->input_stream != NULL);  // should have data bound if you landed here (we need raw_spec to be initialized).
            br = FillSilenceFrames(track, pcm, raw_spec.channels, bytes_remaining);
        } else if (track->input_stream) {
            if (track->input_audio) {
                DecodeMore(track, bytes_remaining);
            }
            br = SDL_GetAudioStreamData(track->input_stream, pcm, bytes_remaining);
        }

        // if input_audio and input_stream are both NULL, there's nothing to play (maybe they changed out the input on us?), br will be zero and we'll go to end_of_audio=true.

        if (br <= 0) {  // if 0: EOF. if < 0: decoding/input failure, we're done by default. But maybe it'll loop and play the start again...!
            end_of_audio = true;
        } else {
            SDL_assert(track->input_stream != NULL);  // should have data bound if you landed here.

            // this (probably?) shouldn't be a partial read here. It's either we completely filled the buffer or exhausted the data.
            //  as such, this does raw_callback() as likely the entire buffer, or all we're getting before a finish callback would have to fire,
            //  even if the finish callback would restart the track. As such, the outer loop is mostly here to deal with looping tracks
            //  and finish callbacks that restart the track.

            // if this would put us past the end of maxframes, or a fadeout, clamp br and set end_of_audio=true so we can do looping, etc.
            Sint64 maxpos = -1;
            if (track->max_frame >= 0) {
                maxpos = track->max_frame;
            }
            if (track->fade_direction < 0) {
                const Sint64 maxfadepos = (Sint64) (track->position + track->fade_frames);
                if ((maxpos < 0) || (maxfadepos < maxpos)) {
                    maxpos = maxfadepos;
                }
            }

            const int raw_channels = raw_spec.channels;

            int frames_read = br / (sizeof (float) * raw_channels);
            if (maxpos >= 0) {
                const Sint64 newpos = (Sint64)(track->position + frames_read);
                if (newpos >= maxpos) {  // we read past the end of the fade out or maxframes, we need to clamp.
                    br -= (int)(((newpos - maxpos) * raw_channels) * sizeof(float));
                    frames_read = br / (sizeof (float) * raw_channels);
                    end_of_audio = true;
                }
            }

            // give the app a shot at the final buffer before sending it on through transformations.
            const int samples = frames_read * raw_channels;

            if (track->raw_callback) {
                track->raw_callback(track->raw_callback_userdata, track, &raw_spec, pcm, samples);
            }

            ApplyFade(track, raw_channels, pcm, frames_read);

            const int put_bytes = samples * sizeof (float);
            SDL_PutAudioStreamData(stream, pcm, put_bytes);

            track->position += frames_read;
            bytes_remaining -= put_bytes;
        }

        // remember that the callback in TrackStopped() might restart this track,
        //  so we'll loop to see if we can fill in more audio without a gap even in that case.
        if (end_of_audio) {
            if (track->input_audio) {
                SDL_ClearAudioStream(track->input_stream);   // make sure that any extra buffered input is removed.
            }
            bool track_stopped = false;
            if (track->loops_remaining == 0) {
                if (track->silence_frames < 0) {
                    track->silence_frames = -track->silence_frames;  // time to start appending silence.
                } else {
                    track_stopped = true;  // out of data, no loops remain, no appended silence left, we're done.
                }
            } else {
                if (track->loops_remaining > 0) {  // negative means infinite loops, so don't decrement for that.
                    track->loops_remaining--;
                }
                if (!track->input_audio) {  // can't loop on a streaming input, you're done.
                    track_stopped = true;
                } else {
                    if (!track->input_audio->decoder->seek(track->decoder_userdata, track->loop_start)) {
                        track_stopped = true;  // uhoh, can't seek! Abandon ship!
                    } else {
                        track->position = track->loop_start;
                    }
                }
            }

            if (track_stopped) {
                if (track->halt_when_exhausted) {
                    TrackStopped(track);
                } else {
                    break;  // done with this track for now, but don't halt the track. We'll try it again later.
                }
            }
        }
    }
}

static void MixSpatializedFloat32Audio(float *dst, const float *src, const int samples, const int output_channels, const float *panning, const int *speakers, const float gain)
{
    const float panning0 = panning[0] * gain;
    const float panning1 = panning[1] * gain;
    const int speaker0 = speakers[0];
    const int speaker1 = speakers[1];

    // !!! FIXME: a common case (output_channels==2, speaker0=0, speaker1=1) can be easily SIMD'd.
    // !!! FIXME: unroll this loop?
    if ((panning0 == 0.0f) && (panning1 == 0.0f)) {
        return;  // don't mix silence.
    } else if ((panning0 == 1.0f) && (panning1 == 1.0f)) {  // no modulation.
        for (int i = 0; i < samples; i++, dst += output_channels, src++) {
            const float sample = *src;
            dst[speaker0] += sample;
            dst[speaker1] += sample;
        }
    } else {
        for (int i = 0; i < samples; i++, dst += output_channels, src++) {
            const float sample = *src;
            dst[speaker0] += sample * panning0;
            dst[speaker1] += sample * panning1;
        }
    }
}

static void MixForcedStereoFloat32Audio(float *dst, const float *src, const int sample_frames, const int output_channels, const float *panning, const float gain)
{
    const float panning0 = panning[0] * gain;
    const float panning1 = panning[1] * gain;

    // !!! FIXME: a common case (output_channels==2) can be easily SIMD'd.
    // !!! FIXME: unroll this loop?
    if ((panning0 == 0.0f) && (panning1 == 0.0f)) {
        return;  // don't mix silence.
    } else if ((panning0 == 1.0f) && (panning1 == 1.0f)) {  // no modulation.
        for (int i = 0; i < sample_frames; i++, dst += output_channels, src += 2) {
            dst[0] += src[0];
            dst[1] += src[1];
        }
    } else {
        for (int i = 0; i < sample_frames; i++, dst += output_channels, src += 2) {
            dst[0] += src[0] * panning0;
            dst[1] += src[1] * panning1;
        }
    }
}

static void MixFloat32Audio(float *dst, const float *src, const int buffer_size, const float gain)
{
    if (gain == 0.0f) {
        return;  // don't mix silence.
    } else if (!SDL_MixAudio((Uint8 *) dst, (const Uint8 *) src, SDL_AUDIO_F32, buffer_size, gain)) {
        SDL_assert(!"This shouldn't happen.");
    }
}

// SDL calls this function from the audio device thread as more data is needed the mixer.
static void SDLCALL MixerCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    MIX_Mixer *mixer = (MIX_Mixer *) userdata;
    mixer->actual_mixed_bytes = 0;

    if (additional_amount == 0) {
        return;  // nothing to actually do yet. This was a courtesy call; the stream still has enough buffered.
    }

    // it should be asking for float data...
    SDL_assert((additional_amount % sizeof (float)) == 0);

    // !!! FIXME: maybe we should do a consistent buffer size, to make this easier for app callbacks
    // !!! FIXME:  and save some trouble on systems that want to do like 200 samples at a time.

    // do we need to grow our buffer?
    const bool skip_group_mixing = !mixer->all_groups || !mixer->all_groups->next;
    const int alloc_multiplier = skip_group_mixing ? 2 : 3;
    const int alloc_size = additional_amount * alloc_multiplier;
    if ((unsigned)alloc_size > mixer->mix_buffer_allocation) {
        void *ptr = SDL_realloc(mixer->mix_buffer, alloc_size);
        if (!ptr) {   // uhoh.
            return;  // not much to be done, we're out of memory!
        }
        mixer->mix_buffer = (float *) ptr;
        mixer->mix_buffer_allocation = alloc_size;
    }

    float *getbuf = mixer->mix_buffer;
    float *final_mixbuf = getbuf + (additional_amount / sizeof (float));
    float *group_mixbuf = skip_group_mixing ? final_mixbuf : (final_mixbuf + (additional_amount / sizeof (float)));

    SDL_memset(final_mixbuf, '\0', additional_amount);

    MIX_Group *next_group = NULL;
    for (MIX_Group *group = mixer->all_groups; group; group = next_group) {
        next_group = group->next;  // this won't save you from a callback going totally rogue, but it'll deal with the current group changing.
        if (!skip_group_mixing) {
            SDL_memset(group_mixbuf, '\0', additional_amount);  // if skip_group_mixing, this is final_mixbuf, which we just zero'd out.
        }

        int group_bytes = 0;
        MIX_Track *next_track = NULL;
        for (MIX_Track *track = group->tracks; track; track = next_track) {
            next_track = track->group_next;  // this won't save you from a callback going totally rogue, but it'll deal with the current track leaving the group.
            const int to_be_read = (additional_amount / SDL_AUDIO_FRAMESIZE(mixer->spec)) * SDL_AUDIO_FRAMESIZE(track->output_spec);
            const int br = SDL_GetAudioStreamData(track->output_stream, getbuf, to_be_read);
            if (br > 0) {
                if (track->cooked_callback) {
                    track->cooked_callback(track->cooked_callback_userdata, track, &track->output_spec, getbuf, br / sizeof (float));
                }

                switch (track->spatialization_mode) {
                    case MIX_SPATIALIZATION_NONE:
                        SDL_assert(track->output_spec.channels == mixer->spec.channels);
                        MixFloat32Audio(group_mixbuf, getbuf, br, mixer->gain);
                        group_bytes = SDL_max(group_bytes, br);
                        break;

                    case MIX_SPATIALIZATION_3D:
                        SDL_assert(track->output_spec.channels == 1);
                        MixSpatializedFloat32Audio(group_mixbuf, getbuf, br / sizeof (float), mixer->spec.channels, track->spatialization_panning, track->spatialization_speakers, mixer->gain);
                        group_bytes = SDL_max(group_bytes, br * mixer->spec.channels);
                        break;

                    case MIX_SPATIALIZATION_STEREO:
                        SDL_assert(track->output_spec.channels == 2);
                        MixForcedStereoFloat32Audio(group_mixbuf, getbuf, br / (sizeof (float) * 2), mixer->spec.channels, track->spatialization_panning, mixer->gain);
                        group_bytes = SDL_max(group_bytes, (br / 2) * mixer->spec.channels);
                        break;

                    default:
                        SDL_assert(!"Unexpected spatialization mode");
                        break;
                }
            }
        }

        if (group_bytes > mixer->actual_mixed_bytes) {
            mixer->actual_mixed_bytes = group_bytes;
        }

        if (group->postmix_callback) {
            group->postmix_callback(group->postmix_callback_userdata, group, &mixer->spec, group_mixbuf, additional_amount / sizeof (float));
        }

        if (!skip_group_mixing) {
            MixFloat32Audio(final_mixbuf, group_mixbuf, group_bytes, 1.0f);  // we adjusted for mixer->gain for each track, don't adjust gain here, too.
        }
    }

    if (mixer->postmix_callback) {
        mixer->postmix_callback(mixer->postmix_callback_userdata, mixer, &mixer->spec, final_mixbuf, additional_amount / sizeof (float));
    }

    SDL_PutAudioStreamData(stream, final_mixbuf, additional_amount);
}

int MIX_Generate(MIX_Mixer *mixer, void *buffer, int buflen)
{
    SDL_AudioSpec output_spec;
    if (!CheckMixerParam(mixer)) {
        return -1;
    } else if (mixer->device_id) {
        SDL_SetError("Can't use MIX_Generate with a MIX_Mixer from MIX_CreateMixerDevice");
        return -1;
    } else if (!SDL_GetAudioStreamFormat(mixer->output_stream, NULL, &output_spec)) {
        return -1;
    } else if (!SDL_GetAudioStreamData(mixer->output_stream, buffer, buflen)) {  // will fire MixerCallback() to generate audio.
        return -1;
    }

    return (mixer->actual_mixed_bytes / sizeof (float)) * SDL_AUDIO_BYTESIZE(output_spec.format);
}

static void InitDecoders(void)
{
    for (size_t i = 0; i < SDL_arraysize(decoders); i++) {
        const MIX_Decoder *decoder = decoders[i];
        if (!decoder->init || decoder->init()) {
            available_decoders[num_available_decoders++] = decoder;
        }
    }
}

static void QuitDecoders(void)
{
    for (int i = 0; i < num_available_decoders; i++) {
        if (available_decoders[i]->quit) {
            available_decoders[i]->quit();
        }
        available_decoders[i] = NULL;
    }
    num_available_decoders = 0;
}

int MIX_Version(void)
{
    return SDL_MIXER_VERSION;
}

bool MIX_Init(void)
{
    if (!mixer_initialized) {
        #if defined(SDL_SSE_INTRINSICS)   // we assume you have SSE if you're on an Intel CPU.
        if (!SDL_HasSSE()) {
            return SDL_SetError("Need SSE instructions but this CPU doesn't offer it");  // whoa! Better order a new Pentium III from Gateway 2000!
        }
        #endif

        #if defined(SDL_NEON_INTRINSICS) && !SDL_MIXER_NEED_SCALAR_FALLBACK
        if (!SDL_HasNEON()) {
            return SDL_SetError("Need NEON instructions but this CPU doesn't offer it");  // :(
        }
        #elif defined(SDL_NEON_INTRINSICS) && SDL_MIXER_NEED_SCALAR_FALLBACK
        MIX_HasNEON = SDL_HasNEON();
        #endif

        global_lock = SDL_CreateMutex();
        if (!global_lock) {
            return false;
        }
        InitDecoders();
    }
    mixer_initialized++;
    return true;
}

void MIX_Quit(void)
{
    SDL_assert(mixer_initialized >= 0);

    if (mixer_initialized <= 0) {
        return;   // not mixer_initialized
    }

    if (mixer_initialized > 1) {
        mixer_initialized--;
        return;  // more refcounts to go.
    }

    // actually shutting down now.
    while (all_audiodecoders) {
        MIX_DestroyAudioDecoder(all_audiodecoders);
    }

    while (all_mixers) {
        MIX_DestroyMixer(all_mixers);
    }

    while (all_audios) {
        MIX_DestroyAudio(all_audios);
    }

    QuitDecoders();

    SDL_DestroyMutex(global_lock);
    global_lock = NULL;

    mixer_initialized = 0;
}

int MIX_GetNumAudioDecoders(void)
{
    return CheckInitialized() ? num_available_decoders : -1;
}

const char *MIX_GetAudioDecoder(int index)
{
    if (!CheckInitialized()) {
        return NULL;
    } else if ((index < 0) || (index >= num_available_decoders)) {
        SDL_InvalidParamError("index");
        return NULL;
    }
    return available_decoders[index]->name;
}

static MIX_Mixer *CreateMixer(SDL_AudioStream *stream)
{
    if (!stream) {
        return NULL;
    }

    SDL_assert(CheckInitialized());

    MIX_Mixer *mixer = (MIX_Mixer *) SDL_calloc(1, sizeof (*mixer));
    if (!mixer) {
        goto failed;
    }

    SDL_AudioSpec output_spec;
    if (!SDL_GetAudioStreamFormat(stream, &mixer->spec, &output_spec)) {
        goto failed;
    }

    // on our end, we always work in float32 format.
    if (mixer->spec.format != SDL_AUDIO_F32) {
        mixer->spec.format = SDL_AUDIO_F32;
        if (!SDL_SetAudioStreamFormat(stream, &mixer->spec, NULL)) {
            goto failed;
        }
    }

    mixer->gain = 1.0f;
    mixer->output_stream = stream;

    mixer->props = SDL_CreateProperties();
    if (!mixer->props) {
        goto failed;
    }

    mixer->track_tags = SDL_CreateProperties();
    if (!mixer->track_tags) {
        goto failed;
    }

    mixer->default_group = MIX_CreateGroup(mixer);
    if (!mixer->default_group) {
        goto failed;
    }

    SDL_SetNumberProperty(mixer->props, MIX_PROP_MIXER_DEVICE_NUMBER, SDL_GetAudioStreamDevice(stream));

    SDL_SetAudioStreamGetCallback(stream, MixerCallback, mixer);

    MIX_VBAP2D_Init(&mixer->vbap2d, output_spec.channels);

    LockGlobal();
    mixer->next = all_mixers;
    if (all_mixers) {
        all_mixers->prev = mixer;
    }
    all_mixers = mixer;
    UnlockGlobal();

    return mixer;

failed:
    SDL_DestroyAudioStream(stream);
    if (mixer) {
        if (mixer->default_group) { MIX_DestroyGroup(mixer->default_group); }
        if (mixer->track_tags) { SDL_DestroyProperties(mixer->track_tags); }
        SDL_free(mixer);
    }
    return NULL;
}

MIX_Mixer *MIX_CreateMixer(const SDL_AudioSpec *spec)
{
    if (!CheckInitialized()) {
        return NULL;
    } else if (!spec) {
        SDL_InvalidParamError("spec");
        return NULL;
    }

    SDL_AudioStream *stream = SDL_CreateAudioStream(spec, spec);
    if (!stream) {
        return NULL;
    }

    // we want this stream to survive SDL_Quit(), since it's not attached to an audio device.
    SDL_SetBooleanProperty(SDL_GetAudioStreamProperties(stream), SDL_PROP_AUDIOSTREAM_AUTO_CLEANUP_BOOLEAN, false);

    return CreateMixer(stream);
}

MIX_Mixer *MIX_CreateMixerDevice(SDL_AudioDeviceID devid, const SDL_AudioSpec *spec)
{
    if (!CheckInitialized()) {
        return NULL;
    } else if (!SDL_Init(SDL_INIT_AUDIO)) {
        return NULL;
    }

    MIX_Mixer *mixer = CreateMixer(SDL_OpenAudioDeviceStream(devid, spec, NULL, NULL));
    if (!mixer) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    } else {
        mixer->device_id = SDL_GetAudioStreamDevice(mixer->output_stream);
        SDL_AddEventWatch(AudioDeviceChangeEventWatcher, mixer);
        SDL_ResumeAudioStreamDevice(mixer->output_stream);
    }

    return mixer;
}

void MIX_DestroyMixer(MIX_Mixer *mixer)
{
    if (!mixer) {
        return;  // harmless no-op.
    }

    LockGlobal();
    if (mixer->prev) {
        mixer->prev->next = mixer->next;
    } else {
        all_mixers = mixer->next;
    }
    if (mixer->next) {
        mixer->next->prev = mixer->prev;
    }
    UnlockGlobal();

    MIX_StopAllTracks(mixer, 0);

    while (mixer->all_tracks) {
        MIX_DestroyTrack(mixer->all_tracks);
    }

    while (mixer->all_groups) {
        MIX_DestroyGroup(mixer->all_groups);
    }

    if (mixer->device_id) {
        SDL_RemoveEventWatch(AudioDeviceChangeEventWatcher, mixer);
    }

    SDL_DestroyAudioStream(mixer->output_stream);
    SDL_DestroyProperties(mixer->track_tags);
    SDL_DestroyProperties(mixer->props);
    SDL_free(mixer->mix_buffer);

    if (mixer->device_id) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    SDL_free(mixer);
}

SDL_PropertiesID MIX_GetMixerProperties(MIX_Mixer *mixer)
{
    if (!CheckMixerParam(mixer)) {
        return 0;
    }

    if (mixer->props == 0) {
        mixer->props = SDL_CreateProperties();
    }
    return mixer->props;
}

bool MIX_GetMixerFormat(MIX_Mixer *mixer, SDL_AudioSpec *spec)
{
    if (!CheckMixerParam(mixer)) {
        if (spec) {
            SDL_zerop(spec);
        }
        return false;
    } else if (!spec) {
        return SDL_InvalidParamError("spec");
    }
    return SDL_GetAudioStreamFormat(mixer->output_stream, NULL, spec);
}

void MIX_LockMixer(MIX_Mixer *mixer)
{
    if (mixer) {
        LockMixer(mixer);
    }
}

void MIX_UnlockMixer(MIX_Mixer *mixer)
{
    if (mixer) {
        UnlockMixer(mixer);
    }
}

static const MIX_Decoder *PrepareDecoder(SDL_IOStream *io, MIX_Audio *audio)
{
    const char *decoder_name = SDL_GetStringProperty(audio->props, MIX_PROP_AUDIO_DECODER_STRING, NULL);

    SDL_AudioSpec original_spec;
    SDL_copyp(&original_spec, &audio->spec);

    for (int i = 0; i < num_available_decoders; i++) {
        const MIX_Decoder *decoder = available_decoders[i];
        if (!decoder_name || (SDL_strcasecmp(decoder->name, decoder_name) == 0)) {
            if (decoder->init_audio(io, &audio->spec, audio->props, &audio->duration_frames, &audio->decoder_userdata)) {
                audio->decoder = decoder;
                return decoder;
            } else if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {   // note this seeks to offset 0, because we're using an IoClamp.
                SDL_SetError("Can't seek in stream to find proper decoder");
                return NULL;
            }
            SDL_copyp(&audio->spec, &original_spec);  // reset this, in case init_audio changed it and then failed.
        }
    }

    SDL_SetError("Audio data is in unknown/unsupported/corrupt format");
    return NULL;
}

static void *DecodeWholeFile(MIX_Audio *audio, SDL_IOStream *io, size_t *decoded_len)
{
    size_t bytes_decoded = 0;
    Uint8 *decoded = NULL;
    SDL_AudioStream *stream = SDL_CreateAudioStream(&audio->spec, &audio->spec);   // !!! FIXME: if we're decoding up front, we might as well convert to float here too, right?
    if (stream) {
        const MIX_Decoder *decoder = audio->decoder;
        void *track_userdata = NULL;
        if (decoder->init_track(audio->decoder_userdata, io, &audio->spec, audio->props, &track_userdata)) {
            if (decoder->seek(track_userdata, 0)) {
                while (decoder->decode(track_userdata, stream)) {
                    // spin.
                }
            }
            decoder->quit_track(track_userdata);

            SDL_FlushAudioStream(stream);
            const int available = SDL_GetAudioStreamAvailable(stream);
            decoded = (Uint8 *) SDL_malloc(available);   // !!! FIXME: SIMD align?
            if (decoded) {
                const int rc = SDL_GetAudioStreamData(stream, decoded, available);
                SDL_assert((rc < 0) || (rc == available));
                if (rc < 0) {
                    SDL_free(decoded);
                    decoded = NULL;
                } else {
                    bytes_decoded = (size_t) available;
                }
            }
        }
        SDL_DestroyAudioStream(stream);
    }

    *decoded_len = bytes_decoded;
    return decoded;
}

MIX_Audio *MIX_LoadAudioWithProperties(SDL_PropertiesID props)  // lets you specify things like "here's a path to MIDI instrument data outside of this file", etc.
{
    if (!CheckInitialized()) {
        return NULL;
    }

    SDL_IOStream *origio = (SDL_IOStream *) SDL_GetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, NULL);
    MIX_Mixer *mixer = (MIX_Mixer *) SDL_GetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, NULL);
    const bool predecode = SDL_GetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_PREDECODE_BOOLEAN, false);
    const bool closeio = SDL_GetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, false);
    const bool ondemand = SDL_GetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_ONDEMAND_BOOLEAN, false);
    const bool skip_metadata_tags = SDL_GetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_SKIP_METADATA_TAGS_BOOLEAN, false);
    void *audio_userdata = NULL;
    const MIX_Decoder *decoder = NULL;
    SDL_IOStream *io = NULL;
    SDL_IOStream *ioclamp = NULL;
    MIX_IoClamp clamp;
    SDL_AudioSpec recommended_spec = { SDL_AUDIO_F32, 2, 48000 };  // a reasonable default if no mixer specified.
    if (mixer) {
        SDL_copyp(&recommended_spec, &mixer->spec);
    }

    MIX_Audio *audio = (MIX_Audio *) SDL_calloc(1, sizeof (*audio));
    if (!audio) {
        goto failed;
    }

    audio->duration_frames = MIX_DURATION_UNKNOWN;
    audio->props = SDL_CreateProperties();
    if (!audio->props) {
        goto failed;
    }

    if (props && !SDL_CopyProperties(props, audio->props)) {
        goto failed;
    }

    io = origio;  // we'll replace this if parsing metadata tags.

    // check for ID3/APE/MusicMatch/whatever tags here, in case they were slapped onto the edge of any random file format.
    audio->clamp_offset = -1;
    audio->clamp_length = -1;
    if (origio && !skip_metadata_tags) {
        ioclamp = io = MIX_OpenIoClamp(&clamp, origio);
        if (!io) {
            goto failed;
        }

        const Sint64 orig_filelen = clamp.length;

        // !!! FIXME: currently we're ignoring return values from this function (see FIXME at the top of its code).
        MIX_ReadMetadataTags(io, audio->props, &clamp);
        if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {
            goto failed;
        }

        // will we need to apply an IoClamp when reading the real data later, too?
        if ((clamp.start != 0) || (clamp.length != orig_filelen)) {
            audio->clamp_offset = clamp.start;
            audio->clamp_length = clamp.length;
        }
    }

    // the decoder sets audio->spec to whatever it's actually providing, but we pass the current hardware setting in, in case that's useful for
    // things that generate audio in whatever format (for example, a MIDI decoder is going to generate PCM from "notes", so it can do it at any
    // sample rate, so it might as well do it at device format to avoid an unnecessary resample later).
    SDL_copyp(&audio->spec, &recommended_spec);

    decoder = PrepareDecoder(io, audio);
    if (!decoder) {
        goto failed;
    }

    audio_userdata = audio->decoder_userdata;  // less wordy access to this pointer.  :)

    // Go back to start of the SDL_IOStream, since we're either precaching, predecoding, or maybe just getting ready to actually play the thing.
    if (io && (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0)) {   // note this seeks to offset 0, because we're using an IoClamp.
        goto failed;
    }

    // set this before predecoding might change `decoder` to the RAW implementation.
    SDL_SetStringProperty(audio->props, MIX_PROP_AUDIO_DECODER_STRING, decoder->name);

    // if this is already raw data, predecoding is just going to make a copy of it, so skip it.
    if (predecode && (decoder != &MIX_Decoder_RAW) && (audio->duration_frames != MIX_DURATION_INFINITE)) {
        audio->precache = DecodeWholeFile(audio, io, &audio->precachelen);
        if (!audio->precache) {
            goto failed;
        }
        audio->free_precache = true;

        decoder->quit_audio(audio_userdata);
        decoder = audio->decoder = &MIX_Decoder_RAW;
        audio_userdata = audio->decoder_userdata = NULL;  // no audio_userdata state in the RAW decoder (so we can cheat here and not do a full init_audio().)
        audio->duration_frames = audio->precachelen / SDL_AUDIO_FRAMESIZE(audio->spec);
        audio->clamp_offset = -1;   // we're raw data now, any existing clamp is just nonsense now.
        audio->clamp_length = -1;
    } else if (!ondemand) {  // precache the audio data, so all decoding happens from a single buffer in RAM shared between tracks.
        if ((audio->precache = SDL_LoadFile_IO(io, &audio->precachelen, false)) == NULL) {
            goto failed;
        }
        audio->free_precache = true;
        audio->clamp_offset = -1;   // precache is already clamped
        audio->clamp_length = -1;
    }

    if (ioclamp) {
        SDL_CloseIO(ioclamp);  // IoClamp's close doesn't close the original stream, but we still need to free its resources here.
        io = ioclamp = NULL;
    }

    if (closeio) {
        SDL_CloseIO(origio);
        origio = NULL;
        SDL_ClearProperty(audio->props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER);
    }

    if (audio->duration_frames >= 0) {
        SDL_SetNumberProperty(audio->props, MIX_PROP_METADATA_DURATION_FRAMES_NUMBER, audio->duration_frames);
    } else if (audio->duration_frames == MIX_DURATION_INFINITE) {
        SDL_SetBooleanProperty(audio->props, MIX_PROP_METADATA_DURATION_INFINITE_BOOLEAN, true);
    }

    SDL_AtomicIncRef(&audio->refcount);

    LockGlobal();
    audio->next = all_audios;
    if (all_audios) {
        all_audios->prev = audio;
    }
    all_audios = audio;
    UnlockGlobal();

    return audio;

failed:
    if (decoder) {
        decoder->quit_audio(audio_userdata);
    }

    if (audio) {
        if (audio->precache) {
            SDL_free((void *) audio->precache);
        }
        if (audio->props) {
            SDL_DestroyProperties(audio->props);
        }
        SDL_free(audio);
    }

    if (ioclamp) {
        SDL_CloseIO(ioclamp);  // IoClamp's close doesn't close the original stream, but we still need to free its resources here.
    }

    if (origio && closeio) {
        SDL_CloseIO(origio);
    }

    return NULL;
}

MIX_Audio *MIX_LoadAudio_IO(MIX_Mixer *mixer, SDL_IOStream *io, bool predecode, bool closeio)
{
    if (!io) {
        SDL_InvalidParamError("io");
        return NULL;
    }

    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, mixer);
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_PREDECODE_BOOLEAN, predecode);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, closeio);
    MIX_Audio *audio = MIX_LoadAudioWithProperties(props);
    SDL_DestroyProperties(props);
    return audio;
}

MIX_Audio *MIX_LoadAudio(MIX_Mixer *mixer, const char *path, bool predecode)
{
    if (!path) {
        SDL_InvalidParamError("path");
        return NULL;
    }

    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    MIX_Audio *retval = NULL;
    if (io) {
        const SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, mixer);
        SDL_SetStringProperty(props, MIX_PROP_AUDIO_LOAD_PATH_STRING, path);
        SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
        SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_PREDECODE_BOOLEAN, predecode);
        SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, true);
        retval = MIX_LoadAudioWithProperties(props);
        SDL_DestroyProperties(props);
    }

    return retval;
}

MIX_Audio *MIX_LoadAudioNoCopy(MIX_Mixer *mixer, const void *data, size_t datalen, bool free_when_done)
{
    if (!data) {
        SDL_InvalidParamError("data");
        return NULL;
    }

    SDL_IOStream *io = SDL_IOFromConstMem(data, datalen);
    if (!io) {
        return NULL;
    }

    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, mixer);
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, true);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_ONDEMAND_BOOLEAN, true);  // so it doesn't make a copy to precache
    MIX_Audio *audio = MIX_LoadAudioWithProperties(props);
    SDL_DestroyProperties(props);

    if (!audio) {
        return NULL;
    }

    audio->precache = data;
    audio->precachelen = datalen;
    audio->free_precache = free_when_done;

    return audio;
}

MIX_Audio *MIX_LoadRawAudio_IO(MIX_Mixer *mixer, SDL_IOStream *io, const SDL_AudioSpec *spec, bool closeio)
{
    if (!CheckInitialized()) {
        if (io && closeio) { SDL_CloseIO(io); }
        return NULL;
    } else if (!io) {
        SDL_InvalidParamError("io");
        return NULL;
    } else if (!spec) {
        if (io && closeio) { SDL_CloseIO(io); }
        SDL_InvalidParamError("spec");
        return NULL;
    }

    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, mixer);
    SDL_SetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, "RAW");
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_FORMAT_NUMBER, (Sint64) spec->format);
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_CHANNELS_NUMBER, (Sint64) spec->channels);
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_FREQ_NUMBER, (Sint64) spec->freq);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_SKIP_METADATA_TAGS_BOOLEAN, true);
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, closeio);

    MIX_Audio *audio = MIX_LoadAudioWithProperties(props);
    SDL_DestroyProperties(props);
    return audio;
}


MIX_Audio *MIX_LoadRawAudio(MIX_Mixer *mixer, const void *data, size_t datalen, const SDL_AudioSpec *spec)
{
    return MIX_LoadRawAudio_IO(mixer, SDL_IOFromConstMem(data, datalen), spec, true);
}

MIX_Audio *MIX_LoadRawAudioNoCopy(MIX_Mixer *mixer, const void *data, size_t datalen, const SDL_AudioSpec *spec, bool free_when_done)
{
    if (!CheckInitialized()) {
        return NULL;
    } else if (!data) {
        SDL_InvalidParamError("data");
        return NULL;
    } else if (!spec) {
        SDL_InvalidParamError("spec");
        return NULL;
    }

    SDL_IOStream *io = SDL_IOFromConstMem(data, datalen);
    if (!io) {
        return NULL;
    }

    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, mixer);
    SDL_SetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, "RAW");
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_FORMAT_NUMBER, (Sint64) spec->format);
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_CHANNELS_NUMBER, (Sint64) spec->channels);
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_FREQ_NUMBER, (Sint64) spec->freq);
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_SKIP_METADATA_TAGS_BOOLEAN, true);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_ONDEMAND_BOOLEAN, true);  // so it doesn't make a copy to precache
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, true);
    MIX_Audio *audio = MIX_LoadAudioWithProperties(props);
    SDL_DestroyProperties(props);

    if (!audio) {
        return NULL;
    }

    audio->precache = data;
    audio->precachelen = datalen;
    audio->free_precache = free_when_done;

    return audio;
}

MIX_Audio *MIX_CreateSineWaveAudio(MIX_Mixer *mixer, int hz, float amplitude, Sint64 ms)
{
    if (!CheckInitialized()) {
        return NULL;
    } else if (hz <= 0) {
        SDL_InvalidParamError("hz");
        return NULL;
    } else if ((amplitude < 0.0f) || (amplitude > 1.0f)) {
        SDL_InvalidParamError("amplitude");
        return NULL;
    }

    const SDL_PropertiesID props = SDL_CreateProperties();
    if (!props) {
        return NULL;
    }

    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, mixer);
    SDL_SetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, "SINEWAVE");
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_SINEWAVE_HZ_NUMBER, hz);
    SDL_SetFloatProperty(props, MIX_PROP_DECODER_SINEWAVE_AMPLITUDE_FLOAT, amplitude);
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_SINEWAVE_MS_NUMBER, ms);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_ONDEMAND_BOOLEAN, true);
    MIX_Audio *audio = MIX_LoadAudioWithProperties(props);
    SDL_DestroyProperties(props);
    return audio;
}

SDL_PropertiesID MIX_GetAudioProperties(MIX_Audio *audio)
{
    if (!CheckAudioParam(audio)) {
        return 0;
    }

    if (audio->props == 0) {
        audio->props = SDL_CreateProperties();
    }
    return audio->props;
}

bool MIX_GetAudioFormat(MIX_Audio *audio, SDL_AudioSpec *spec)
{
    if (!CheckAudioParam(audio)) {
        return false;
    } else if (!spec) {
        return SDL_InvalidParamError("spec");
    }
    SDL_copyp(spec, &audio->spec);
    return true;
}

Sint64 MIX_GetAudioDuration(MIX_Audio *audio)
{
    return CheckAudioParam(audio) ? audio->duration_frames : -1;
}

static void RefAudio(MIX_Audio *audio)
{
    if (audio) {
        SDL_AtomicIncRef(&audio->refcount);
    }
}

static void UnrefAudio(MIX_Audio *audio)
{
    if (audio && SDL_AtomicDecRef(&audio->refcount)) {
        LockGlobal();
        if (audio->prev) {
            audio->prev->next = audio->next;
        } else {
            all_audios = audio->next;
        }
        if (audio->next) {
            audio->next->prev = audio->prev;
        }
        UnlockGlobal();

        if (audio->decoder) {
            audio->decoder->quit_audio(audio->decoder_userdata);
        }
        if (audio->props) {
            SDL_DestroyProperties(audio->props);
        }
        if (audio->free_precache) {
            SDL_free((void *) audio->precache);
        }
        SDL_free(audio);
    }
}

void MIX_DestroyAudio(MIX_Audio *audio)
{
    if (CheckAudioParam(audio)) {
        UnrefAudio(audio);
    }
}

MIX_Track *MIX_CreateTrack(MIX_Mixer *mixer)
{
    if (!CheckMixerParam(mixer)) {
        return NULL;
    }

    // this makes sure track->position is aligned for SIMD access.
    MIX_Track *track = (MIX_Track *) SDL_aligned_alloc(SDL_GetSIMDAlignment(), sizeof (*track));
    if (!track) {
        return NULL;
    }
    SDL_zerop(track);

    track->tags = SDL_CreateProperties();
    if (!track->tags) {
        SDL_free(track);
        return NULL;
    }

    track->output_stream = SDL_CreateAudioStream(&mixer->spec, &mixer->spec);
    if (!track->output_stream) {
        SDL_DestroyProperties(track->tags);
        SDL_free(track);
        return NULL;
    }

    // we want this stream to survive SDL_Quit(), since it's not attached to an audio device.
    SDL_SetBooleanProperty(SDL_GetAudioStreamProperties(track->output_stream), SDL_PROP_AUDIOSTREAM_AUTO_CLEANUP_BOOLEAN, false);

    SDL_SetAudioStreamGetCallback(track->output_stream, TrackGetCallback, track);

    track->mixer = mixer;
    track->halt_when_exhausted = true;

    LockMixer(mixer);
    track->next = mixer->all_tracks;
    if (mixer->all_tracks) {
        mixer->all_tracks->prev = track;
    }
    mixer->all_tracks = track;
    UnlockMixer(mixer);

    MIX_SetTrackGroup(track, NULL);  // this sets up state and updates linked lists. Should not fail!

    return track;
}

// Take `track` out of `mixer`'s list of tracks tagged with `tag`.
static void RemoveTrackFromMixerTagList(MIX_Mixer *mixer, MIX_Track *track, const char *tag)
{
    const SDL_PropertiesID track_tags = track->mixer->track_tags;
    MIX_TagList *list = (MIX_TagList *) SDL_GetPointerProperty(track_tags, tag, NULL);
    if (list) {
        SDL_LockRWLockForWriting(list->rwlock);
        for (size_t i = 0; i < list->num_tracks; i++) {
            if (list->tracks[i] == track) {
                const size_t cpy = (list->num_tracks - (i+1)) * sizeof (*list->tracks);
                if (cpy) {
                    SDL_memmove(&list->tracks[i], &list->tracks[i+1], cpy);
                }
                list->tracks[--list->num_tracks] = NULL;
                break;
            }
        }
        SDL_UnlockRWLock(list->rwlock);
    }
}

// this is an enumerator; call it multiple times to _actually_ remove from all tag lists.
static void SDLCALL RemoveTrackFromAllMixerTagLists(void *userdata, SDL_PropertiesID props, const char *tag)
{
    // this only removes the track from the mixer's tag lists, as we're
    //  enumerating track->tags and don't want the hash to change during that.
    // After this is done, we'll destroy the hash outright anyhow.
    MIX_Track *track = (MIX_Track *) userdata;
    SDL_assert(track->mixer != NULL);
    SDL_assert(track->tags == props);
    if (SDL_GetBooleanProperty(props, tag, false)) {  // these still exist in track->tags once untagged, so only bother if set to true.
        RemoveTrackFromMixerTagList(track->mixer, track, tag);
    }
}

MIX_Mixer *MIX_GetTrackMixer(MIX_Track *track)
{
    return CheckTrackParam(track) ? track->mixer : NULL;
}

void MIX_DestroyTrack(MIX_Track *track)
{
    if (!CheckTrackParam(track)) {
        return;
    }

    MIX_Mixer *mixer = track->mixer;

    LockMixer(mixer);
    if (track->prev) {
        track->prev->next = track->next;
    } else {
        mixer->all_tracks = track->next;
    }
    if (track->next) {
        track->next->prev = track->prev;
    }

    // we don't check the fire-and-forget pool because that is only free'd, with mixer->all_tracks, when closing the mixer.
    // !!! FIXME: maybe we _shouldn't_ keep the fire-and-forget pool in all_tracks, so we can skip processing them everywhere, and just explicitly free the pool in MIX_DestroyMixer.

    SDL_assert(track->group != NULL);
    if (track->group_prev) {
        track->group_prev->group_next = track->group_next;
    } else {
        track->group->tracks = track->group_next;
    }
    if (track->group_next) {
        track->group_next->group_prev = track->group_prev;
    }
    track->group = NULL;
    UnlockMixer(mixer);

    SDL_DestroyAudioStream(track->output_stream);

    if (track->input_audio) {
        track->input_audio->decoder->quit_track(track->decoder_userdata);
    }

    SDL_DestroyAudioStream(track->internal_stream);

    UnrefAudio(track->input_audio);
    SDL_EnumerateProperties(track->tags, RemoveTrackFromAllMixerTagLists, track);
    SDL_DestroyProperties(track->tags);
    SDL_DestroyProperties(track->props);
    SDL_free(track->input_buffer);
    if (track->ioclamp.io) {  // if we applied an i/o clamp to the stream, close that unconditionally.
        SDL_CloseIO(track->io);   // this is the clamp, not the actual stream.
        track->io = track->ioclamp.io;  // this is the actual stream.
    }
    if (track->io && track->closeio) {
        SDL_CloseIO(track->io);
    }
    SDL_aligned_free(track);
}

SDL_PropertiesID MIX_GetTrackProperties(MIX_Track *track)
{
    if (!CheckTrackParam(track)) {
        return 0;
    }

    if (track->props == 0) {
        track->props = SDL_CreateProperties();
    }
    return track->props;
}

static bool MIX_SetTrackAudio_internal(MIX_Track *track, MIX_Audio *audio, SDL_IOStream *io, bool closeio)
{
    SDL_assert(CheckTrackParam(track));
    SDL_assert(audio || !io);  // if audio==NULL, io must be NULL, too.
    SDL_assert(io || !closeio);  // if io==NULL, closeio must be false.

    SDL_IOStream *origio = io;

    SDL_AudioSpec spec;
    if (audio) {
        SDL_copyp(&spec, &audio->spec);
    } else {
        // make this reasonable, but in theory we shouldn't touch it again.
        spec.freq = 44100;
        spec.channels = 2;
    }
    spec.format = SDL_AUDIO_F32;  // we always work in float32.

    LockTrack(track);

    if (audio && (track->internal_stream == NULL)) {
        track->internal_stream = SDL_CreateAudioStream(&audio->spec, &spec);
        if (!track->internal_stream) {
            UnlockTrack(track);
            if (closeio) {
                SDL_CloseIO(io);
            }
            return false;
        }

        // we want this stream to survive SDL_Quit(), since it's not attached to an audio device.
        SDL_SetBooleanProperty(SDL_GetAudioStreamProperties(track->internal_stream), SDL_PROP_AUDIOSTREAM_AUTO_CLEANUP_BOOLEAN, false);
    }

    if (track->input_audio) {
        track->input_audio->decoder->quit_track(track->decoder_userdata);
        UnrefAudio(track->input_audio);
        if (track->ioclamp.io) {  // if we applied an i/o clamp to the stream, close that unconditionally.
            SDL_CloseIO(track->io);   // this is the clamp, not the actual stream.
            track->io = track->ioclamp.io;  // this is the actual stream.
        }
        if (track->io && track->closeio) {
            SDL_CloseIO(track->io);
        }
        SDL_zero(track->ioclamp);
        track->io = NULL;
        track->closeio = false;
    }

    track->input_audio = NULL;
    track->input_stream = NULL;

    bool retval = true;
    if (audio) {
        if (audio->clamp_offset >= 0) {   // clamp i/o so decoders don't see ID3 tags, etc.
            SDL_IOStream *clampio = MIX_OpenIoClamp(&track->ioclamp, io);
            if (!clampio) {
                retval = false;
            } else {
                io = clampio;
                track->ioclamp.start = audio->clamp_offset;
                track->ioclamp.length = audio->clamp_length;
            }
        }

        if (retval) {
            retval = audio->decoder->init_track(audio->decoder_userdata, io, &audio->spec, audio->props, &track->decoder_userdata);
            if (!retval) {
                if (track->ioclamp.io) {
                    SDL_CloseIO(io);  // this was the IoClamp, not the real data stream.
                    io = origio;
                }
            } else {
                RefAudio(audio);
                SDL_SetAudioStreamFormat(track->internal_stream, &audio->spec, &spec);   // input is from decoded audio, output is to output_stream
                SetTrackOutputStreamFormat(track, &spec);   // input is from internal_stream, output is to mixer->output_stream (or, if spatializing, to mixer->output_stream but mono).
                track->input_audio = audio;
                track->input_stream = track->internal_stream;
                SDL_ClearAudioStream(track->input_stream);   // make sure that any extra buffered input from before is removed.
                track->position = 0;
                track->io = io;
                track->closeio = closeio;
            }
        }
    }

    UnlockTrack(track);

    if (!retval && closeio) {
        SDL_CloseIO(origio);
    }

    return retval;
}

bool MIX_SetTrackAudio(MIX_Track *track, MIX_Audio *audio)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    SDL_IOStream *io = NULL;
    if (audio) {
        // external MIX_Audios shouldn't be able to get into a state where they aren't precached (except SINEWAVE,
        // which is generated on the fly). Make this assert more generic if we add another thing like SINEWAVE later!
        SDL_assert((audio->precache != NULL) || (audio->decoder == &MIX_Decoder_SINEWAVE));

        if (audio->precache) {
            io = SDL_IOFromConstMem(audio->precache, audio->precachelen);
            if (!io) {
                return false;
            }
        }
    }

    return MIX_SetTrackAudio_internal(track, audio, io, io != NULL);
}

bool MIX_SetTrackAudioStream(MIX_Track *track, SDL_AudioStream *stream)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    LockTrack(track);

    if (track->input_audio) {
        track->input_audio->decoder->quit_track(track->decoder_userdata);
        UnrefAudio(track->input_audio);
        track->input_audio = NULL;
        if (track->ioclamp.io) {  // if we applied an i/o clamp to the stream, close that unconditionally.
            SDL_CloseIO(track->io);   // this is the clamp, not the actual stream.
            track->io = track->ioclamp.io;  // this is the actual stream.
        }
        if (track->io && track->closeio) {
            SDL_CloseIO(track->io);
        }
        SDL_zero(track->ioclamp);
        track->io = NULL;
        track->closeio = false;
        if (track->internal_stream) {
            SDL_ClearAudioStream(track->internal_stream);   // make sure that any extra buffered input is removed.
        }
    }

    if (stream) {
        SDL_AudioSpec spec;
        SDL_GetAudioStreamFormat(stream, &spec, NULL);
        spec.format = SDL_AUDIO_F32;  // we always work in float32.
        SDL_SetAudioStreamFormat(stream, NULL, &spec);                 // input is whatever, output is whatever in float format.
        SetTrackOutputStreamFormat(track, &spec);   // input is whatever in float format, output is to mixer->output_stream (or, if spatializing, to mixer->output_stream but mono).
    }

    track->input_stream = stream;
    track->position = 0;
    UnlockTrack(track);

    return true;
}

bool MIX_SetTrackIOStream(MIX_Track *track, SDL_IOStream *io, bool closeio)
{
    if (!CheckTrackParam(track)) {
        if (io && closeio) { SDL_CloseIO(io); }
        return false;
    } else if (!io) {
        return MIX_SetTrackAudio(track, NULL);  // just drop the current input.
    }

    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, track->mixer);
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_ONDEMAND_BOOLEAN, true);
    MIX_Audio *audio = MIX_LoadAudioWithProperties(props);
    SDL_DestroyProperties(props);
    if (!audio) {
        if (closeio) { SDL_CloseIO(io); }
        return false;
    }

    const bool retval = MIX_SetTrackAudio_internal(track, audio, io, closeio);

    // Drop our reference to `audio` after the track accepts it, so when the track is
    //  done with it, it'll unref it, and `audio` will be cleaned up. If the track failed
    //  to accept the audio, this will clean it up right now.
    UnrefAudio(audio);

    return retval;
}

bool MIX_SetTrackRawIOStream(MIX_Track *track, SDL_IOStream *io, const SDL_AudioSpec *spec, bool closeio)
{
    if (!CheckTrackParam(track)) {
        if (io && closeio) { SDL_CloseIO(io); }
        return false;
    } else if (!io) {
        return MIX_SetTrackAudio(track, NULL);  // just drop the current input.
    } else if (!spec) {
        if (io && closeio) { SDL_CloseIO(io); }
        return SDL_InvalidParamError("spec");
    }

    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, track->mixer);
    SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_ONDEMAND_BOOLEAN, true);
    SDL_SetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, "RAW");
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_FORMAT_NUMBER, (Sint64) spec->format);
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_CHANNELS_NUMBER, (Sint64) spec->channels);
    SDL_SetNumberProperty(props, MIX_PROP_DECODER_FREQ_NUMBER, (Sint64) spec->freq);
    SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_SKIP_METADATA_TAGS_BOOLEAN, true);
    MIX_Audio *audio = MIX_LoadAudioWithProperties(props);
    SDL_DestroyProperties(props);
    if (!audio) {
        if (io && closeio) { SDL_CloseIO(io); }
        return false;
    }

    const bool retval = MIX_SetTrackAudio_internal(track, audio, io, closeio);

    // Drop our reference to `audio` after the track accepts it, so when the track is
    //  done with it, it'll unref it, and `audio` will be cleaned up. If the track failed
    //  to accept the audio, this will clean it up right now.
    UnrefAudio(audio);

    return retval;
}

static void SDLCALL CleanupTagList(void *userdata, void *value)
{
    MIX_TagList *list = (MIX_TagList *) value;
    SDL_DestroyRWLock(list->rwlock);
    SDL_free(list->tracks);
    SDL_free(list);
}

// assumes inputs are valid and mixer->track_tags's lock is held.
static MIX_TagList *CreateTagList(MIX_Mixer *mixer, const char *tag)
{
    const SDL_PropertiesID track_tags = mixer->track_tags;
    SDL_assert(track_tags != 0);

    SDL_LockProperties(track_tags);

    MIX_TagList *list = (MIX_TagList *) SDL_GetPointerProperty(track_tags, tag, NULL);  // check that something didn't beat us here while we waited on the lock.
    if (!list) {
        list = (MIX_TagList *) SDL_calloc(1, sizeof (*list));
        if (list) {
            list->num_allocated = 4;
            list->tracks = (MIX_Track **) SDL_calloc(list->num_allocated, sizeof (*list->tracks));
            list->rwlock = SDL_CreateRWLock();
            if (!list->tracks || !list->rwlock) {
                SDL_free(list->tracks);
                if (list->rwlock) {
                    SDL_DestroyRWLock(list->rwlock);
                }
                SDL_free(list);
                list = NULL;
            }
        }

        if (list && !SDL_SetPointerPropertyWithCleanup(track_tags, tag, list, CleanupTagList, NULL)) {
            SDL_DestroyRWLock(list->rwlock);
            SDL_free(list->tracks);
            SDL_free(list);
            list = NULL;
        }
    }

    SDL_UnlockProperties(track_tags);

    return list;
}

bool MIX_TagTrack(MIX_Track *track, const char *tag)
{
    if (!CheckTrackTagParam(track, tag)) {
        return false;
    }

    const SDL_PropertiesID tags = track->tags;

    SDL_LockProperties(tags);
    if (!SDL_GetBooleanProperty(tags, tag, false)) {
        if (!SDL_SetBooleanProperty(tags, tag, true)) {
            SDL_UnlockProperties(tags);
            return false;
        }

        const SDL_PropertiesID track_tags = track->mixer->track_tags;
        SDL_assert(track_tags != 0);
        MIX_TagList *list = (MIX_TagList *) SDL_GetPointerProperty(track_tags, tag, NULL);
        if (!list) {
            list = CreateTagList(track->mixer, tag);
            if (!list) {
                SDL_UnlockProperties(tags);
                SDL_SetBooleanProperty(tags, tag, false);
                return false;
            }
        }

        SDL_LockRWLockForWriting(list->rwlock);
        if (list->num_tracks >= list->num_allocated) {
            void *ptr = SDL_realloc(list->tracks, sizeof (*list->tracks) * (list->num_allocated * 2));
            if (!ptr) {
                SDL_UnlockRWLock(list->rwlock);
                SDL_SetBooleanProperty(tags, tag, false);
                SDL_UnlockProperties(tags);
                return false;
            }
            list->tracks = ptr;
            list->num_allocated *= 2;
        }
        list->tracks[list->num_tracks++] = track;
        SDL_UnlockRWLock(list->rwlock);
    }
    SDL_UnlockProperties(tags);

    return true;
}

void MIX_UntagTrack(MIX_Track *track, const char *tag)
{
    if (!CheckTrackParam(track)) {
        return;  // do nothing.
    }

    const SDL_PropertiesID tags = track->tags;
    if (!tag) {  // untag everything on the track.
        SDL_EnumerateProperties(tags, RemoveTrackFromAllMixerTagLists, track);
        const SDL_PropertiesID new_tags = SDL_CreateProperties();
        LockTrack(track);
        track->tags = new_tags;
        UnlockTrack(track);
        SDL_DestroyProperties(tags);  // just nuke all the tags and start over.
    } else {
        SDL_LockProperties(tags);
        if (SDL_GetBooleanProperty(tags, tag, false)) {  // if tag isn't there, nothing to do.
            SDL_assert(SDL_GetPointerProperty(track->mixer->track_tags, tag, NULL) != NULL);  // shouldn't be NULL, there's definitely a track with this tag!
            RemoveTrackFromMixerTagList(track->mixer, track, tag);
            SDL_SetBooleanProperty(tags, tag, false);
        }
        SDL_UnlockProperties(tags);
    }
}

typedef struct GetTrackTagsCallbackData
{
    const char *struct_tags[4];  // hopefully mostly fits in here, no allocations.
    const char **allocated_tags;
    int count;
    bool failed;
} GetTrackTagsCallbackData;

static void SDLCALL GetTrackTagsCallback(void *userdata, SDL_PropertiesID props, const char *tag)
{
    // just store the tag to the array; since we have the properties locked, we can copy it after enumeration is done.
    GetTrackTagsCallbackData *data = (GetTrackTagsCallbackData *) userdata;
    if (data->failed) {
        return;  // just get out if we previously failed.
    } else if (SDL_GetBooleanProperty(props, tag, false)) {   // if false, tag _was_ here, but has since been untagged. Skip it.
        if (data->count < (int)SDL_arraysize(data->struct_tags)) {
            data->struct_tags[data->count++] = tag;
        } else {
            void *ptr = SDL_realloc(data->allocated_tags, sizeof (char *) * (data->count - SDL_arraysize(data->struct_tags) + 1));
            if (!ptr) {
                data->failed = true;
            } else {
                data->allocated_tags = (const char **) ptr;
                data->allocated_tags[data->count - SDL_arraysize(data->struct_tags)] = tag;
                data->count++;
            }
        }
    }
}

char **MIX_GetTrackTags(MIX_Track *track, int *count)
{
    char **retval = NULL;
    int dummycount;
    if (!count) {
        count = &dummycount;
    }
    *count = 0;

    if (!CheckTrackParam(track)) {
        return NULL;
    }

    GetTrackTagsCallbackData data;
    SDL_zero(data);
    SDL_LockProperties(track->tags);
    SDL_EnumerateProperties(track->tags, GetTrackTagsCallback, &data);
    if (!data.failed) {
        size_t allocation = sizeof (char *);  // one extra pointer for the list's NULL terminator.
        for (int i = 0; i < data.count; i++) {
            const char *str = (i < (int)SDL_arraysize(data.struct_tags)) ? data.struct_tags[i] : data.allocated_tags[i - SDL_arraysize(data.struct_tags)];
            allocation += sizeof (char *) + SDL_strlen(str) + 1;
        }
        retval = (char **) SDL_malloc(allocation);
        if (retval) {
            char *strptr = ((char *) retval) + (sizeof (char *) * (data.count + 1));
            for (int i = 0; i < data.count; i++) {
                const char *str = (i < (int)SDL_arraysize(data.struct_tags)) ? data.struct_tags[i] : data.allocated_tags[i - SDL_arraysize(data.struct_tags)];
                const size_t slen = SDL_strlen(str) + 1;
                SDL_memcpy(strptr, str, slen);
                retval[i] = strptr;
                strptr += slen;
            }
            retval[data.count] = NULL;
            *count = data.count;
        }
        SDL_free(data.allocated_tags);
    }
    SDL_UnlockProperties(track->tags);

    return retval;
}

MIX_Track **MIX_GetTaggedTracks(MIX_Mixer *mixer, const char *tag, int *count)
{
    int dummycount;
    if (!count) {
        count = &dummycount;
    }
    *count = 0;

    if (!CheckMixerTagParam(mixer, tag)) {
        return NULL;
    }

    MIX_TagList *list = (MIX_TagList *) SDL_GetPointerProperty(mixer->track_tags, tag, NULL);
    if (!list) {  // nothing is using this tag?
        return (MIX_Track **) SDL_calloc(1, sizeof (MIX_Track *));  // just a single NULL entry (or a NULL return if out of memory, works either way).
    }

    MIX_Track **retval = NULL;
    SDL_LockRWLockForReading(list->rwlock);
    const size_t total = list->num_tracks;
    retval = (MIX_Track **) SDL_malloc(sizeof (*retval) * (total + 1));
    if (retval) {
        SDL_memcpy(retval, list->tracks, sizeof (*retval) * total);
        retval[total] = NULL;
        *count = (int) total;
    }
    SDL_UnlockRWLock(list->rwlock);

    return retval;
}


bool MIX_SetTrackPlaybackPosition(MIX_Track *track, Sint64 frames)
{
    if (!CheckTrackParam(track)) {
        return false;
    } else if (frames < 0) {
        return SDL_InvalidParamError("frames");
    }

    bool retval = true;

    // !!! FIXME: should it be legal to seek past the end of an track (so it just stops immediately, or maybe stops on next callback)?
    LockTrack(track);
    if (!track->input_audio) {
        if (track->input_stream) {  // can't seek a stream that was set up with MIX_SetTrackAudioStream.
            retval = SDL_SetError("Can't seek a streaming track");
        } else {
            retval = SDL_SetError("No audio currently assigned to this track");
        }
    } else {
        retval = track->input_audio->decoder->seek(track->decoder_userdata, (Uint64) frames);
        if (retval) {
            SDL_ClearAudioStream(track->input_stream);   // make sure that any extra buffered input from before the seek is removed.
            track->position = (Uint64) frames;
        }
    }
    UnlockTrack(track);

    return retval;
}

Sint64 MIX_GetTrackPlaybackPosition(MIX_Track *track)
{
    Sint64 retval = -1;
    if (CheckTrackParam(track)) {
        LockTrack(track);
        retval = (Sint64) track->position;
        UnlockTrack(track);
    }
    return retval;
}

Sint64 MIX_GetTrackRemaining(MIX_Track *track)
{
    Sint64 retval = -1;
    if (CheckTrackParam(track)) {
        LockTrack(track);
        const Sint64 position = (Sint64) track->position;
        const Sint64 duration = track->input_audio ? track->input_audio->duration_frames : -1;
        const bool stopped = (track->state == MIX_STATE_STOPPED);
        UnlockTrack(track);
        if (stopped) {
            retval = 0;
        } else if ((duration >= 0) && (position >= 0) && (position <= duration)) {
            retval = duration - position;
        }
    }
    return retval;
}

Sint64 MIX_GetTrackFadeFrames(MIX_Track *track)
{
    Sint64 retval = 0;
    if (CheckTrackParam(track)) {
        LockTrack(track);
        if (track->state != MIX_STATE_STOPPED) {
            retval = track->fade_frames * track->fade_direction;
        }
        UnlockTrack(track);
    }
    return retval;
}

int MIX_GetTrackLoops(MIX_Track *track)
{
    int retval = 0;
    if (CheckTrackParam(track)) {
        LockTrack(track);
        if (track->state != MIX_STATE_STOPPED) {
            retval = track->loops_remaining;
        }
        UnlockTrack(track);
    }
    return retval;
}

bool MIX_SetTrackLoops(MIX_Track *track, int num_loops)
{
    bool retval = false;
    if (CheckTrackParam(track)) {
        if (num_loops < -1) {
            num_loops = -1;  // keep this value consistent if we're looping infinitely.
        }
        LockTrack(track);
        track->loops_remaining = num_loops;
        UnlockTrack(track);
        retval = true;
    }
    return retval;
}

MIX_Audio *MIX_GetTrackAudio(MIX_Track *track)
{
    MIX_Audio *retval = NULL;
    if (CheckTrackParam(track)) {
        LockTrack(track);
        retval = (track->input_audio && track->input_audio->precache) ? track->input_audio : NULL;  // don't allow access to the MIX_Audio if this was a temporary one created for MIX_SetTrackIOStream.
        UnlockTrack(track);
    }
    return retval;
}

SDL_AudioStream *MIX_GetTrackAudioStream(MIX_Track *track)
{
    SDL_AudioStream *retval = NULL;
    if (CheckTrackParam(track)) {
        LockTrack(track);
        retval = (track->input_stream != track->internal_stream) ? track->input_stream : NULL;
        UnlockTrack(track);
    }
    return retval;
}

Sint64 MIX_MSToFrames(int sample_rate, Sint64 ms)
{
    if (sample_rate <= 0) {
        SDL_InvalidParamError("sample_rate");
        return -1;
    } else if (ms < 0) {
        SDL_InvalidParamError("ms");
        return -1;
    }
    return (Sint64) ((((double) ms) / 1000.0) * ((double) sample_rate));
}

Sint64 MIX_FramesToMS(int sample_rate, Sint64 frames)
{
    if (sample_rate <= 0) {
        SDL_InvalidParamError("sample_rate");
        return -1;
    } else if (frames < 0) {
        SDL_InvalidParamError("frames");
        return -1;
    }
    return (Sint64) ((((double) frames) / ((double) sample_rate)) * 1000.0);
}

Sint64 MIX_TrackMSToFrames(MIX_Track *track, Sint64 ms)
{
    Sint64 retval = -1;
    if (CheckTrackParam(track)) {
        LockTrack(track);
        SDL_AudioSpec spec;
        if (track->input_stream) {
            SDL_GetAudioStreamFormat(track->input_stream, &spec, NULL);
        } else {
            spec.freq = 0;
        }
        UnlockTrack(track);
        if (spec.freq) {
            retval = MIX_MSToFrames(spec.freq, ms);
        }
    }
    return retval;
}

Sint64 MIX_TrackFramesToMS(MIX_Track *track, Sint64 frames)
{
    Sint64 retval = -1;
    if (CheckTrackParam(track)) {
        LockTrack(track);
        SDL_AudioSpec spec;
        if (track->input_stream) {
            SDL_GetAudioStreamFormat(track->input_stream, &spec, NULL);
        } else {
            spec.freq = 0;
        }
        UnlockTrack(track);
        if (spec.freq) {
            retval = MIX_FramesToMS(spec.freq, frames);
        }
    }
    return retval;
}

Sint64 MIX_AudioMSToFrames(MIX_Audio *audio, Sint64 ms)
{
    if (!CheckAudioParam(audio)) {
        return -1;
    }
    return MIX_MSToFrames(audio->spec.freq, ms);
}

Sint64 MIX_AudioFramesToMS(MIX_Audio *audio, Sint64 frames)
{
    if (!CheckAudioParam(audio)) {
        return -1;
    }
    return MIX_FramesToMS(audio->spec.freq, frames);
}

static Sint64 GetTrackOptionFramesOrTicks(MIX_Track *track, SDL_PropertiesID options, const char *framesprop, const char *msprop, Sint64 defval)
{
    if (SDL_HasProperty(options, framesprop)) {
        return SDL_GetNumberProperty(options, framesprop, defval);
    } else if (SDL_HasProperty(options, msprop)) {
        const Sint64 val = SDL_GetNumberProperty(options, msprop, defval);
        Sint64 val_frames = MIX_TrackMSToFrames(track, val);
        if (val_frames < 0) {
            val_frames = 0;
        }
        return (val < 0) ? val : val_frames;
    }
    return defval;
}

bool MIX_PlayTrack(MIX_Track *track, SDL_PropertiesID options)
{
    if (!CheckTrackParam(track)) {
        return false;
    } else if (!track->input_audio && !track->input_stream) {
        return SDL_SetError("No audio currently assigned to this track");
    }

    int loops = 0;
    Sint64 max_frame = -1;
    Sint64 start_pos = 0;
    Sint64 loop_start = 0;
    Sint64 fade_in = 0;
    Sint64 append_silence_frames = 0;
    int start_order = -1;
    float fade_start_gain = 0.0f;
    bool halt_when_exhausted = true;

    LockTrack(track);
    if (options) {
        loops = (int) SDL_GetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        max_frame = GetTrackOptionFramesOrTicks(track, options, MIX_PROP_PLAY_MAX_FRAME_NUMBER, MIX_PROP_PLAY_MAX_MILLISECONDS_NUMBER, max_frame);
        start_pos = GetTrackOptionFramesOrTicks(track, options, MIX_PROP_PLAY_START_FRAME_NUMBER, MIX_PROP_PLAY_START_MILLISECOND_NUMBER, start_pos);
        loop_start = GetTrackOptionFramesOrTicks(track, options, MIX_PROP_PLAY_LOOP_START_FRAME_NUMBER, MIX_PROP_PLAY_LOOP_START_MILLISECOND_NUMBER, loop_start);
        fade_in = GetTrackOptionFramesOrTicks(track, options, MIX_PROP_PLAY_FADE_IN_FRAMES_NUMBER, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fade_in);
        fade_start_gain = SDL_GetFloatProperty(options, MIX_PROP_PLAY_FADE_IN_START_GAIN_FLOAT, fade_start_gain);
        append_silence_frames = GetTrackOptionFramesOrTicks(track, options, MIX_PROP_PLAY_APPEND_SILENCE_FRAMES_NUMBER, MIX_PROP_PLAY_APPEND_SILENCE_MILLISECONDS_NUMBER, append_silence_frames);
        halt_when_exhausted = SDL_GetBooleanProperty(options, MIX_PROP_PLAY_HALT_WHEN_EXHAUSTED_BOOLEAN, halt_when_exhausted);
        start_order = (int) SDL_GetNumberProperty(options, MIX_PROP_PLAY_START_ORDER_NUMBER, start_order);

        if (start_pos < 0) {
            start_pos = 0;
        }

        if (loop_start < 0) {
            loop_start = 0;
        }

        if (append_silence_frames < 0) {
            append_silence_frames = 0;
        }

        fade_start_gain = SDL_clamp(fade_start_gain, 0.0f, 1.0f);
    }

    if ((start_order >= 0) && (!track->input_audio || !track->input_audio->decoder->jump_to_order)) {
        start_order = -1;  // ignore this option, it doesn't mean anything on this decoder.
    }

    if ((start_order >= 0) && !track->input_audio->decoder->jump_to_order(track->decoder_userdata, start_order)) {
        UnlockTrack(track);
        return false;
    } else if (track->input_audio && (!track->input_audio->decoder->seek(track->decoder_userdata, start_pos))) {
        UnlockTrack(track);
        return false;
    } else if (!track->input_audio && (start_pos != 0)) {
        UnlockTrack(track);
        return SDL_SetError("Playing an input stream (not MIX_Audio) with a non-zero start position");  // !!! FIXME: should we just read off this many frames right now instead?
    }

    track->max_frame = max_frame;
    track->loops_remaining = loops;
    track->loop_start = (int) loop_start;
    track->total_fade_frames = (fade_in > 0) ? fade_in : 0;
    track->fade_frames = track->total_fade_frames;
    track->fade_direction = (fade_in > 0) ? 1 : 0;
    track->fade_start_gain = fade_start_gain;
    track->silence_frames = (append_silence_frames > 0) ? -append_silence_frames : 0;  // negative means "there is still actual audio data to play", positive means "we're done with actual data, feed silence now." Zero means no silence (left) to feed.
    track->state = MIX_STATE_PLAYING;
    track->position = start_pos;
    track->halt_when_exhausted = halt_when_exhausted;

    UnlockTrack(track);
    return true;
}

bool MIX_PlayTag(MIX_Mixer *mixer, const char *tag, SDL_PropertiesID options)
{
    if (!CheckMixerTagParam(mixer, tag)) {
        return false;
    }

    MIX_TagList *list = (MIX_TagList *) SDL_GetPointerProperty(mixer->track_tags, tag, NULL);
    if (!list) {
        return true;  // nothing is using this tag, do nothing (but not an error).
    }

    bool retval = true;
    SDL_LockRWLockForReading(list->rwlock);
    const size_t total = list->num_tracks;
    LockMixer(mixer);  // so all tracks start at the same time.

    for (size_t i = 0; i < total; i++) {
        MIX_Track *track = list->tracks[i];
        if (track->input_audio || track->input_stream) {  // don't treat it as an error if no audio is available, just don't play it.
            if (!MIX_PlayTrack(track, options)) {
                retval = false;
            }
        }
    }

    UnlockMixer(mixer);
    SDL_UnlockRWLock(list->rwlock);

    return retval;
}

bool MIX_PlayAudio(MIX_Mixer *mixer, MIX_Audio *audio)
{
    if (!CheckMixerParam(mixer)) {
        return false;
    } else if (!CheckAudioParam(audio)) {
        return false;
    }

    // grab an existing fire-and-forget track from the available pool.
    LockMixer(mixer);
    MIX_Track *track = mixer->fire_and_forget_pool;
    if (track) {
        mixer->fire_and_forget_pool = track->fire_and_forget_next;
        track->fire_and_forget_next = NULL;
    }
    UnlockMixer(mixer);

    if (!track) {  // make a new item if the pool was empty.
        track = MIX_CreateTrack(mixer);
        if (!track) {
            return false;
        }
        track->fire_and_forget = true;
    }

    if (!MIX_SetTrackAudio(track, audio)) {
        // !!! FIXME: put the track back in the fire/forget pool.
        return false;
    }

    const bool retval = MIX_PlayTrack(track, 0);

    // !!! FIXME: MIX_PlayTrack should only fail for things we already validated here...but if this assertion fires, we need to put this track back in the fire/forget pool.
    SDL_assert(retval);

    return retval;
}

static void StopTrack(MIX_Track *track, Sint64 fadeOut)
{
    LockTrack(track);
    if (track->state != MIX_STATE_STOPPED) {
        if (fadeOut <= 0) {  // stop immediately.
            if (track->internal_stream) {
                SDL_ClearAudioStream(track->internal_stream);  // make sure we don't leave old data hanging around.
            }
            TrackStopped(track);
        } else {
            track->total_fade_frames = fadeOut;
            track->fade_frames = fadeOut;
            track->fade_direction = -1;
            track->fade_start_gain = 0.0f;  // only used for fade-ins.
        }
    }
    UnlockTrack(track);
}

bool MIX_StopTrack(MIX_Track *track, Sint64 fade_out_frames)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    StopTrack(track, fade_out_frames);
    return true;
}

bool MIX_StopAllTracks(MIX_Mixer *mixer, Sint64 fade_out_ms)
{
    if (!CheckMixerParam(mixer)) {
        return false;
    }

    LockMixer(mixer);  // lock the mixer so all tracks stop at the same time.

    for (MIX_Track *track = mixer->all_tracks; track != NULL; track = track->next) {
        Sint64 fade_out_frames = MIX_TrackMSToFrames(track, fade_out_ms);
        if (fade_out_frames < 0) {
            fade_out_frames = 0;
        }
        StopTrack(track, (fade_out_ms > 0) ? fade_out_frames : -1);
    }

    UnlockMixer(mixer);

    return true;
}

bool MIX_StopTag(MIX_Mixer *mixer, const char *tag, Sint64 fade_out_ms)
{
    if (!CheckMixerTagParam(mixer, tag)) {
        return false;
    }

    MIX_TagList *list = (MIX_TagList *) SDL_GetPointerProperty(mixer->track_tags, tag, NULL);
    if (!list) {
        return true;  // nothing is using this tag, do nothing (but not an error).
    }

    SDL_LockRWLockForReading(list->rwlock);

    const size_t total = list->num_tracks;
    for (size_t i = 0; i < total; i++) {
        Sint64 fade_out_frames = MIX_TrackMSToFrames(list->tracks[i], fade_out_ms);
        if (fade_out_frames < 0) {
            fade_out_frames = 0;
        }
        StopTrack(list->tracks[i], (fade_out_ms > 0) ? fade_out_frames : -1);
    }

    SDL_UnlockRWLock(list->rwlock);

    return true;
}

static void PauseTrack(MIX_Track *track)
{
    LockTrack(track);
    if (track->state == MIX_STATE_PLAYING) {
        track->state = MIX_STATE_PAUSED;
    }
    UnlockTrack(track);
}

bool MIX_PauseTrack(MIX_Track *track)
{
    if (!CheckTrackParam(track)) {
        return false;
    }
    PauseTrack(track);
    return true;
}

bool MIX_PauseAllTracks(MIX_Mixer *mixer)
{
    if (!CheckMixerParam(mixer)) {
        return false;
    }

    LockMixer(mixer);  // lock the mixer so all tracks pause at the same time.

    for (MIX_Track *track = mixer->all_tracks; track != NULL; track = track->next) {
        PauseTrack(track);
    }

    UnlockMixer(mixer);

    return true;
}

bool MIX_PauseTag(MIX_Mixer *mixer, const char *tag)
{
    if (!CheckMixerTagParam(mixer, tag)) {
        return false;
    }

    MIX_TagList *list = (MIX_TagList *) SDL_GetPointerProperty(mixer->track_tags, tag, NULL);
    if (!list) {
        return true;  // nothing is using this tag, do nothing (but not an error).
    }

    LockMixer(mixer);  // lock the mixer so all tracks pause at the same time.
    SDL_LockRWLockForReading(list->rwlock);

    const size_t total = list->num_tracks;
    for (size_t i = 0; i < total; i++) {
        PauseTrack(list->tracks[i]);
    }

    SDL_UnlockRWLock(list->rwlock);
    UnlockMixer(mixer);

    return true;
}

static void ResumeTrack(MIX_Track *track)
{
    LockTrack(track);
    if (track->state == MIX_STATE_PAUSED) {
        track->state = MIX_STATE_PLAYING;
    }
    UnlockTrack(track);
}

bool MIX_ResumeTrack(MIX_Track *track)
{
    if (!CheckTrackParam(track)) {
        return false;
    }
    ResumeTrack(track);
    return true;
}

bool MIX_ResumeAllTracks(MIX_Mixer *mixer)
{
    if (!CheckMixerParam(mixer)) {
        return false;
    }

    LockMixer(mixer);  // lock the mixer so all tracks resume at the same time.

    for (MIX_Track *track = mixer->all_tracks; track != NULL; track = track->next) {
        ResumeTrack(track);
    }

    UnlockMixer(mixer);
    return true;
}

bool MIX_ResumeTag(MIX_Mixer *mixer, const char *tag)
{
    if (!CheckMixerTagParam(mixer, tag)) {
        return false;
    }

    MIX_TagList *list = (MIX_TagList *) SDL_GetPointerProperty(mixer->track_tags, tag, NULL);
    if (!list) {
        return true;  // nothing is using this tag, do nothing (but not an error).
    }

    LockMixer(mixer);  // lock the mixer so all tracks resume at the same time.
    SDL_LockRWLockForReading(list->rwlock);

    const size_t total = list->num_tracks;
    for (size_t i = 0; i < total; i++) {
        ResumeTrack(list->tracks[i]);
    }

    SDL_UnlockRWLock(list->rwlock);
    UnlockMixer(mixer);

    return true;
}

bool MIX_TrackPlaying(MIX_Track *track)
{
    if (!CheckTrackParam(track)) {
        return false;
    }
    LockTrack(track);
    const bool retval = (track->state == MIX_STATE_PLAYING);
    UnlockTrack(track);
    return retval;
}


bool MIX_TrackPaused(MIX_Track *track)
{
    if (!CheckTrackParam(track)) {
        return false;
    }
    LockTrack(track);
    const bool retval = (track->state == MIX_STATE_PAUSED);
    UnlockTrack(track);
    return retval;
}

bool MIX_SetTrackStoppedCallback(MIX_Track *track, MIX_TrackStoppedCallback cb, void *userdata)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    LockTrack(track);
    track->stopped_callback = cb;
    track->stopped_callback_userdata = userdata;
    UnlockTrack(track);

    return true;
}

bool MIX_SetMixerGain(MIX_Mixer *mixer, float gain)
{
    if (!CheckMixerParam(mixer)) {
        return false;
    } else if (gain < 0.0f) {
        return SDL_InvalidParamError("gain");
    }

    LockMixer(mixer);
    mixer->gain = gain;
    UnlockMixer(mixer);
    return true;
}

float MIX_GetMixerGain(MIX_Mixer *mixer)
{
    if (!CheckMixerParam(mixer)) {
        return 1.0f;
    }

    LockMixer(mixer);
    const float retval = mixer->gain;
    UnlockMixer(mixer);
    return retval;
}

static bool SetTrackGain(MIX_Track *track, float gain)
{
    // don't have to LockTrack, as SDL_SetAudioStreamGain will do that.
    //LockTrack(track);
    const bool retval = SDL_SetAudioStreamGain(track->output_stream, gain);
    //UnlockTrack(track);
    return retval;
}

bool MIX_SetTrackGain(MIX_Track *track, float gain)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    if (gain < 0.0f) {
        gain = 0.0f;  // !!! FIXME: this clamps, but should it fail instead?
    }

    return SetTrackGain(track, gain);
}

float MIX_GetTrackGain(MIX_Track *track)
{
    if (!CheckTrackParam(track)) {
        return 1.0f;
    }

    // don't have to LockTrack, as SDL_GetAudioStreamGain will do that.
    //LockTrack(track);
    const float retval = SDL_GetAudioStreamGain(track->output_stream);
    //UnlockTrack(track);

    return retval;
}

bool MIX_SetTagGain(MIX_Mixer *mixer, const char *tag, float gain)
{
    if (!CheckMixerTagParam(mixer, tag)) {
        return false;
    }

    if (gain < 0.0f) {
        gain = 0.0f;  // !!! FIXME: this clamps, but should it fail instead?
    }

    MIX_TagList *list = (MIX_TagList *) SDL_GetPointerProperty(mixer->track_tags, tag, NULL);
    if (!list) {
        return true;  // nothing is using this tag, do nothing (but not an error).
    }

    LockMixer(mixer);  // lock the mixer so all tracks adust gain at the same time.
    SDL_LockRWLockForReading(list->rwlock);

    const size_t total = list->num_tracks;
    for (size_t i = 0; i < total; i++) {
        SetTrackGain(list->tracks[i], gain);
    }

    SDL_UnlockRWLock(list->rwlock);
    UnlockMixer(mixer);

    return true;
}

bool MIX_SetMixerFrequencyRatio(MIX_Mixer *mixer, float ratio)
{
    if (!CheckMixerParam(mixer)) {
        return false;
    }

    ratio = SDL_clamp(ratio, 0.01f, 100.0f);   // !!! FIXME: this clamps, but should it fail instead?

    // don't have to LockMixer, as SDL_SetAudioStreamFrequencyRatio will do that.
    return SDL_SetAudioStreamFrequencyRatio(mixer->output_stream, ratio);
}

float MIX_GetMixerFrequencyRatio(MIX_Mixer *mixer)
{
    if (!CheckMixerParam(mixer)) {
        return 0.0f;
    }

    // don't have to LockMixer, as SDL_GetAudioStreamFrequencyRatio will do that.
    //LockTrack(track);
    const float retval = SDL_GetAudioStreamFrequencyRatio(mixer->output_stream);
    //UnlockTrack(track);

    return retval;
}

bool MIX_SetTrackFrequencyRatio(MIX_Track *track, float ratio)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    ratio = SDL_clamp(ratio, 0.01f, 100.0f);   // !!! FIXME: this clamps, but should it fail instead?

    // don't have to LockTrack, as SDL_SetAudioStreamFrequencyRatio will do that.
    return SDL_SetAudioStreamFrequencyRatio(track->output_stream, ratio);
}

float MIX_GetTrackFrequencyRatio(MIX_Track *track)
{
    if (!CheckTrackParam(track)) {
        return 0.0f;
    }

    // don't have to LockTrack, as SDL_GetAudioStreamFrequencyRatio will do that.
    //LockTrack(track);
    const float retval = SDL_GetAudioStreamFrequencyRatio(track->output_stream);
    //UnlockTrack(track);

    return retval;
}

bool MIX_SetTrackOutputChannelMap(MIX_Track *track, const int *chmap, int count)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    // don't have to LockTrack, as SDL_SetAudioStreamOutputChannelMap will do that.
    //LockTrack(track);
    const bool retval = SDL_SetAudioStreamOutputChannelMap(track->output_stream, chmap, count);
    //UnlockTrack(track);

    return retval;
}


bool MIX_SetTrackStereo(MIX_Track *track, const MIX_StereoGains *gains)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    LockTrack(track);

    const bool wants_stereo = (gains != NULL);
    const MIX_SpatializationMode new_mode = wants_stereo ? MIX_SPATIALIZATION_STEREO : MIX_SPATIALIZATION_NONE;
    if (track->spatialization_mode != new_mode) {
        track->spatialization_mode = new_mode;
        SetTrackOutputStreamFormat(track, NULL);   // change output format to stereo (or back to normal) if necessary.
    }

    track->position3d[0] = track->position3d[1] = track->position3d[2] = 0.0f;

    if (wants_stereo) {
        const float left = SDL_max(0.0f, gains->left);
        const float right = SDL_max(0.0f, gains->right);
        if (track->mixer->spec.channels == 1) {  // mono output
            track->spatialization_speakers[0] = track->spatialization_speakers[1] = 0;
            track->spatialization_panning[0] = left * 0.5f;
            track->spatialization_panning[1] = right * 0.5f;
        } else {
            track->spatialization_speakers[0] = 0;
            track->spatialization_speakers[1] = 1;
            track->spatialization_panning[0] = left;
            track->spatialization_panning[1] = right;
        }
    }

    UnlockTrack(track);

    return true;

}

bool MIX_SetTrack3DPosition(MIX_Track *track, const MIX_Point3D *position)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    LockTrack(track);

    const bool wants_spatialization = (position != NULL);
    const MIX_SpatializationMode new_mode = wants_spatialization ? MIX_SPATIALIZATION_3D : MIX_SPATIALIZATION_NONE;
    const bool toggling = (track->spatialization_mode != new_mode);
    if (toggling) {
        track->spatialization_mode = new_mode;
        SetTrackOutputStreamFormat(track, NULL);   // change output format to stereo (or back to normal) if necessary.
    }

    if (!wants_spatialization) {
        track->position3d[0] = track->position3d[1] = track->position3d[2] = 0.0f;
    } else {
        float *tposition3d = track->position3d;
        if (toggling || ((tposition3d[0] != position->x) || (tposition3d[2] != position->y) || (tposition3d[2] != position->z))) {
            tposition3d[0] = position->x;
            tposition3d[1] = position->y;
            tposition3d[2] = position->z;
            MIX_Spatialize(&track->mixer->vbap2d, tposition3d, track->spatialization_panning, track->spatialization_speakers);
        }
    }

    UnlockTrack(track);

    return true;
}

bool MIX_GetTrack3DPosition(MIX_Track *track, MIX_Point3D *position)
{
    if (!CheckTrackParam(track)) {
        return false;
    } else if (!position) {
        return SDL_InvalidParamError("position");
    }

    LockTrack(track);
    const float *tposition3d = track->position3d;
    position->x = tposition3d[0];
    position->y = tposition3d[1];
    position->z = tposition3d[2];
    UnlockTrack(track);

    return true;

}

bool MIX_SetPostMixCallback(MIX_Mixer *mixer, MIX_PostMixCallback cb, void *userdata)
{
    if (!CheckMixerParam(mixer)) {
        return false;
    }

    LockMixer(mixer);
    mixer->postmix_callback = cb;
    mixer->postmix_callback_userdata = userdata;
    UnlockMixer(mixer);

    return true;
}

bool MIX_SetTrackRawCallback(MIX_Track *track, MIX_TrackMixCallback cb, void *userdata)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    LockTrack(track);
    track->raw_callback = cb;
    track->raw_callback_userdata = userdata;
    UnlockTrack(track);
    return true;
}

bool MIX_SetTrackCookedCallback(MIX_Track *track, MIX_TrackMixCallback cb, void *userdata)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    LockTrack(track);
    track->cooked_callback = cb;
    track->cooked_callback_userdata = userdata;
    UnlockTrack(track);
    return true;
}

MIX_Group *MIX_CreateGroup(MIX_Mixer *mixer)
{
    if (!CheckMixerParam(mixer)) {
        return NULL;
    }

    MIX_Group *group = (MIX_Group *) SDL_calloc(1, sizeof (*group));
    if (!group) {
        return NULL;
    }

    group->mixer = mixer;

    LockMixer(mixer);
    group->next = mixer->all_groups;
    if (mixer->all_groups) {
        mixer->all_groups->prev = group;
    }
    mixer->all_groups = group;
    UnlockMixer(mixer);

    return group;
}

void MIX_DestroyGroup(MIX_Group *group)
{
    if (!CheckGroupParam(group)) {
        return;
    }

    MIX_Mixer *mixer = group->mixer;

    LockMixer(mixer);
    if (group->prev) {
        group->prev->next = group->next;
    } else {
        mixer->all_groups = group->next;
    }
    if (group->next) {
        group->next->prev = group->prev;
    }

    MIX_Track *next = NULL;
    for (MIX_Track *track = group->tracks; track; track = next) {
        next = track->group_next;  // track->group_next will change in SetTrackGroup, so save it off.
        MIX_SetTrackGroup(track, NULL);
    }
    UnlockMixer(mixer);

    SDL_DestroyProperties(group->props);
    SDL_free(group);
}

SDL_PropertiesID MIX_GetGroupProperties(MIX_Group *group)
{
    if (!CheckGroupParam(group)) {
        return 0;
    }

    if (group->props == 0) {
        group->props = SDL_CreateProperties();
    }
    return group->props;
}

MIX_Mixer *MIX_GetGroupMixer(MIX_Group *group)
{
    return CheckGroupParam(group) ? group->mixer : NULL;
}

bool MIX_SetTrackGroup(MIX_Track *track, MIX_Group *group)
{
    if (!CheckTrackParam(track)) {
        return false;
    }

    if (!group) {
        group = track->mixer->default_group;
    } else if (track->mixer != group->mixer) {
        return SDL_SetError("Track and group are not from the same MIX_Mixer.");
    }

    LockMixer(track->mixer);
    LockTrack(track);
    MIX_Group *oldgroup = track->group;
    if (group != oldgroup) {
        if (oldgroup) {   // remove from current group, if in one.
            if (track->group_prev) {
                track->group_prev->group_next = track->group_next;
                track->group_prev = NULL;
            } else {
                oldgroup->tracks = track->group_next;
            }
            if (track->group_next) {
                track->group_next->group_prev = track->group_prev;
            }
        }

        track->group_next = group->tracks;
        if (group->tracks) {
            group->tracks->group_prev = track;
        }
        group->tracks = track;
        track->group = group;
    }
    UnlockTrack(track);
    UnlockMixer(track->mixer);

    return true;
}

bool MIX_SetGroupPostMixCallback(MIX_Group *group, MIX_GroupMixCallback cb, void *userdata)
{
    if (!CheckGroupParam(group)) {
        return false;
    }

    LockMixer(group->mixer);
    group->postmix_callback = cb;
    group->postmix_callback_userdata = userdata;
    UnlockMixer(group->mixer);

    return true;
}

MIX_AudioDecoder * MIX_CreateAudioDecoder_IO(SDL_IOStream *io, bool closeio, SDL_PropertiesID props)
{
    if (!CheckInitialized()) {
        return NULL;
    } else if (!io) {
        SDL_InvalidParamError("io");
        return NULL;
    }

    MIX_AudioDecoder *audiodecoder = (MIX_AudioDecoder *) SDL_calloc(1, sizeof (*audiodecoder));
    if (!audiodecoder) {
        if (closeio) {
            SDL_CloseIO(io);
        }
        return NULL;
    }

    audiodecoder->io = io;
    audiodecoder->closeio = closeio;

    const SDL_PropertiesID tmpprops = SDL_CreateProperties();
    if (!tmpprops) {
        if (closeio) {
            SDL_CloseIO(io);
        }
        SDL_free(audiodecoder);
        return NULL;
    }

    if (props) {
        if (!SDL_CopyProperties(props, tmpprops)) {
            SDL_DestroyProperties(tmpprops);
            if (closeio) {
                SDL_CloseIO(io);
            }
            SDL_free(audiodecoder);
            return NULL;
        }
    }

    SDL_SetPointerProperty(tmpprops, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
    SDL_SetBooleanProperty(tmpprops, MIX_PROP_AUDIO_LOAD_ONDEMAND_BOOLEAN, true);
    audiodecoder->audio = MIX_LoadAudioWithProperties(tmpprops);
    SDL_DestroyProperties(tmpprops);

    if (!audiodecoder->audio) {
        SDL_free(audiodecoder);
        return NULL;
    }

    if (!audiodecoder->audio->decoder->init_track(audiodecoder->audio->decoder_userdata, io, &audiodecoder->audio->spec, audiodecoder->audio->props, &audiodecoder->track_userdata)) {
        MIX_DestroyAudio(audiodecoder->audio);
        SDL_free(audiodecoder);
        return NULL;
    }

    audiodecoder->stream = SDL_CreateAudioStream(&audiodecoder->audio->spec, &audiodecoder->audio->spec);
    if (!audiodecoder->stream) {
        audiodecoder->audio->decoder->quit_track(audiodecoder->track_userdata);
        MIX_DestroyAudio(audiodecoder->audio);
        SDL_free(audiodecoder);
        return NULL;
    }

    // we want this stream to survive SDL_Quit(), since it's not attached to an audio device.
    SDL_SetBooleanProperty(SDL_GetAudioStreamProperties(audiodecoder->stream), SDL_PROP_AUDIOSTREAM_AUTO_CLEANUP_BOOLEAN, false);

    LockGlobal();
    audiodecoder->next = all_audiodecoders;
    if (all_audiodecoders) {
        all_audiodecoders->prev = audiodecoder;
    }
    all_audiodecoders = audiodecoder;
    UnlockGlobal();

    return audiodecoder;
}

MIX_AudioDecoder *MIX_CreateAudioDecoder(const char *path, SDL_PropertiesID props)
{
    if (!CheckInitialized()) {
        return NULL;
    } else if (!path) {
        SDL_InvalidParamError("path");
        return NULL;
    }

    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) {
        return NULL;
    }

    return MIX_CreateAudioDecoder_IO(io, true, props);
}

void MIX_DestroyAudioDecoder(MIX_AudioDecoder *audiodecoder)
{
    if (CheckAudioDecoderParam(audiodecoder)) {
        LockGlobal();
        if (audiodecoder->prev) {
            audiodecoder->prev->next = audiodecoder->next;
        } else {
            all_audiodecoders = audiodecoder->next;
        }
        if (audiodecoder->next) {
            audiodecoder->next->prev = audiodecoder->prev;
        }
        UnlockGlobal();

        audiodecoder->audio->decoder->quit_track(audiodecoder->track_userdata);
        MIX_DestroyAudio(audiodecoder->audio);
        SDL_DestroyAudioStream(audiodecoder->stream);
        if (audiodecoder->closeio) {
            SDL_CloseIO(audiodecoder->io);
        }
        SDL_free(audiodecoder);
    }
}

SDL_PropertiesID MIX_GetAudioDecoderProperties(MIX_AudioDecoder *audiodecoder)
{
    return CheckAudioDecoderParam(audiodecoder) ? MIX_GetAudioProperties(audiodecoder->audio) : 0;
}

bool MIX_GetAudioDecoderFormat(MIX_AudioDecoder *audiodecoder, SDL_AudioSpec *spec)
{
    return CheckAudioDecoderParam(audiodecoder) ? MIX_GetAudioFormat(audiodecoder->audio, spec) : false;
}

int MIX_DecodeAudio(MIX_AudioDecoder *audiodecoder, void *buffer, int buflen, const SDL_AudioSpec *spec)
{
    if (!CheckAudioDecoderParam(audiodecoder)) {
        return -1;
    }

    SDL_SetAudioStreamFormat(audiodecoder->stream, NULL, spec);

    while (SDL_GetAudioStreamAvailable(audiodecoder->stream) < buflen) {
        if (!audiodecoder->audio->decoder->decode(audiodecoder->track_userdata, audiodecoder->stream)) {
            SDL_FlushAudioStream(audiodecoder->stream);  // make sure we read _everything_ now.
            break;
        }
    }

    return SDL_GetAudioStreamData(audiodecoder->stream, buffer, buflen);
}


// Clamp an IOStream to a subset of its available data.
static Sint64 MIX_IoClamp_size(void *userdata)
{
    return ((const MIX_IoClamp *) userdata)->length;
}

static Sint64 MIX_IoClamp_seek(void *userdata, Sint64 offset, SDL_IOWhence whence)
{
    MIX_IoClamp *clamp = (MIX_IoClamp *) userdata;

    if (whence == SDL_IO_SEEK_CUR) {
        offset += clamp->pos;
    } else if (whence == SDL_IO_SEEK_END) {
        offset += clamp->length;
    }

    if (offset < 0) {
        SDL_SetError("Seek before start of data");
        return -1;
    } else if (offset > clamp->length) {
        offset = clamp->length;
    }

    if (clamp->pos != offset) {
        const Sint64 ret = SDL_SeekIO(clamp->io, clamp->start + offset, SDL_IO_SEEK_SET);
        if (ret < 0) {
            return ret;
        }
        clamp->pos = offset;
    }

    return offset;
}

static size_t MIX_IoClamp_read(void *userdata, void *ptr, size_t size, SDL_IOStatus *status)
{
    MIX_IoClamp *clamp = (MIX_IoClamp *) userdata;
    const size_t remaining = (size_t)(clamp->length - clamp->pos);
    const size_t ret = SDL_ReadIO(clamp->io, ptr, SDL_min(size, remaining));
    if (ret < size) {
        *status = (ret == remaining) ? SDL_IO_STATUS_EOF : SDL_GetIOStatus(clamp->io);
    }
    clamp->pos += ret;
    return ret;
}

SDL_IOStream *MIX_OpenIoClamp(MIX_IoClamp *clamp, SDL_IOStream *io)
{
    /* Don't use SDL_GetIOSize() here -- see SDL bug #4026 */
    SDL_zerop(clamp);
    clamp->io = io;
    clamp->start = SDL_TellIO(io);
    clamp->length = SDL_SeekIO(io, 0, SDL_IO_SEEK_END) - clamp->start;
    clamp->pos = 0;
    if (clamp->start < 0 || clamp->length < 0 || (SDL_SeekIO(io, clamp->start, SDL_IO_SEEK_SET) < 0)) {
        SDL_SetError("Error seeking in datastream");
        return NULL;
    }

    SDL_IOStreamInterface iface;
    SDL_INIT_INTERFACE(&iface);
    iface.size = MIX_IoClamp_size;
    iface.seek = MIX_IoClamp_seek;
    iface.read = MIX_IoClamp_read;
    return SDL_OpenIO(&iface, clamp);
}

void *MIX_GetConstIOBuffer(SDL_IOStream *io, size_t *datalen)
{
    void *buffer = NULL;
    const SDL_PropertiesID ioprops = SDL_GetIOProperties(io);
    if (ioprops) {
        buffer = SDL_GetPointerProperty(ioprops, SDL_PROP_IOSTREAM_MEMORY_POINTER, NULL);
        if (!buffer) {
            buffer = SDL_GetPointerProperty(ioprops, SDL_PROP_IOSTREAM_DYNAMIC_MEMORY_POINTER, NULL);
        }
    }

    if (buffer) {  // we got a pointer to const memory?
        *datalen = SDL_GetIOSize(io);
    }
    return buffer;
}

void *MIX_SlurpConstIO(SDL_IOStream *io, size_t *datalen, bool *copied)
{
    void *buffer = MIX_GetConstIOBuffer(io, datalen);
    if (buffer) {  // we got a pointer to const memory?
        *copied = false;
    } else if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {   // no? Slurp the whole file in.
        return NULL;
    } else if ((buffer = SDL_LoadFile_IO(io, datalen, false)) == NULL) {
        return NULL;
    } else {
        *copied = true;
    }

    return buffer;
}


// table to convert from mu-law encoding to floating point samples,
// generated by a throwaway perl script
#define S2F(s) ( ((float) s) / 32767.0f )  // short to float.
const float MIX_ulawToFloat[256] = {
    S2F(-32124),S2F(-31100),S2F(-30076),S2F(-29052),S2F(-28028),S2F(-27004),S2F(-25980),S2F(-24956),
    S2F(-23932),S2F(-22908),S2F(-21884),S2F(-20860),S2F(-19836),S2F(-18812),S2F(-17788),S2F(-16764),
    S2F(-15996),S2F(-15484),S2F(-14972),S2F(-14460),S2F(-13948),S2F(-13436),S2F(-12924),S2F(-12412),
    S2F(-11900),S2F(-11388),S2F(-10876),S2F(-10364), S2F(-9852), S2F(-9340), S2F(-8828), S2F(-8316),
     S2F(-7932), S2F(-7676), S2F(-7420), S2F(-7164), S2F(-6908), S2F(-6652), S2F(-6396), S2F(-6140),
     S2F(-5884), S2F(-5628), S2F(-5372), S2F(-5116), S2F(-4860), S2F(-4604), S2F(-4348), S2F(-4092),
     S2F(-3900), S2F(-3772), S2F(-3644), S2F(-3516), S2F(-3388), S2F(-3260), S2F(-3132), S2F(-3004),
     S2F(-2876), S2F(-2748), S2F(-2620), S2F(-2492), S2F(-2364), S2F(-2236), S2F(-2108), S2F(-1980),
     S2F(-1884), S2F(-1820), S2F(-1756), S2F(-1692), S2F(-1628), S2F(-1564), S2F(-1500), S2F(-1436),
     S2F(-1372), S2F(-1308), S2F(-1244), S2F(-1180), S2F(-1116), S2F(-1052),  S2F(-988),  S2F(-924),
      S2F(-876),  S2F(-844),  S2F(-812),  S2F(-780),  S2F(-748),  S2F(-716),  S2F(-684),  S2F(-652),
      S2F(-620),  S2F(-588),  S2F(-556),  S2F(-524),  S2F(-492),  S2F(-460),  S2F(-428),  S2F(-396),
      S2F(-372),  S2F(-356),  S2F(-340),  S2F(-324),  S2F(-308),  S2F(-292),  S2F(-276),  S2F(-260),
      S2F(-244),  S2F(-228),  S2F(-212),  S2F(-196),  S2F(-180),  S2F(-164),  S2F(-148),  S2F(-132),
      S2F(-120),  S2F(-112),  S2F(-104),   S2F(-96),   S2F(-88),   S2F(-80),   S2F(-72),   S2F(-64),
       S2F(-56),   S2F(-48),   S2F(-40),   S2F(-32),   S2F(-24),   S2F(-16),    S2F(-8),     S2F(0),
     S2F(32124), S2F(31100), S2F(30076), S2F(29052), S2F(28028), S2F(27004), S2F(25980), S2F(24956),
     S2F(23932), S2F(22908), S2F(21884), S2F(20860), S2F(19836), S2F(18812), S2F(17788), S2F(16764),
     S2F(15996), S2F(15484), S2F(14972), S2F(14460), S2F(13948), S2F(13436), S2F(12924), S2F(12412),
     S2F(11900), S2F(11388), S2F(10876), S2F(10364),  S2F(9852),  S2F(9340),  S2F(8828),  S2F(8316),
      S2F(7932),  S2F(7676),  S2F(7420),  S2F(7164),  S2F(6908),  S2F(6652),  S2F(6396),  S2F(6140),
      S2F(5884),  S2F(5628),  S2F(5372),  S2F(5116),  S2F(4860),  S2F(4604),  S2F(4348),  S2F(4092),
      S2F(3900),  S2F(3772),  S2F(3644),  S2F(3516),  S2F(3388),  S2F(3260),  S2F(3132),  S2F(3004),
      S2F(2876),  S2F(2748),  S2F(2620),  S2F(2492),  S2F(2364),  S2F(2236),  S2F(2108),  S2F(1980),
      S2F(1884),  S2F(1820),  S2F(1756),  S2F(1692),  S2F(1628),  S2F(1564),  S2F(1500),  S2F(1436),
      S2F(1372),  S2F(1308),  S2F(1244),  S2F(1180),  S2F(1116),  S2F(1052),   S2F(988),   S2F(924),
       S2F(876),   S2F(844),   S2F(812),   S2F(780),   S2F(748),   S2F(716),   S2F(684),   S2F(652),
       S2F(620),   S2F(588),   S2F(556),   S2F(524),   S2F(492),   S2F(460),   S2F(428),   S2F(396),
       S2F(372),   S2F(356),   S2F(340),   S2F(324),   S2F(308),   S2F(292),   S2F(276),   S2F(260),
       S2F(244),   S2F(228),   S2F(212),   S2F(196),   S2F(180),   S2F(164),   S2F(148),   S2F(132),
       S2F(120),   S2F(112),   S2F(104),    S2F(96),    S2F(88),    S2F(80),    S2F(72),    S2F(64),
        S2F(56),    S2F(48),    S2F(40),    S2F(32),    S2F(24),    S2F(16),     S2F(8),     S2F(0)
};


const float MIX_alawToFloat[256] = {
    S2F(-5504), S2F(-5248), S2F(-6016), S2F(-5760), S2F(-4480), S2F(-4224), S2F(-4992), S2F(-4736), S2F(-7552), S2F(-7296), S2F(-8064), S2F(-7808), S2F(-6528), S2F(-6272), S2F(-7040), S2F(-6784), S2F(-2752),
    S2F(-2624), S2F(-3008), S2F(-2880), S2F(-2240), S2F(-2112), S2F(-2496), S2F(-2368), S2F(-3776), S2F(-3648), S2F(-4032), S2F(-3904), S2F(-3264), S2F(-3136), S2F(-3520), S2F(-3392), S2F(-22016),
    S2F(-20992), S2F(-24064), S2F(-23040), S2F(-17920), S2F(-16896), S2F(-19968), S2F(-18944), S2F(-30208), S2F(-29184), S2F(-32256), S2F(-31232), S2F(-26112), S2F(-25088), S2F(-28160), S2F(-27136), S2F(-11008),
    S2F(-10496), S2F(-12032), S2F(-11520), S2F(-8960), S2F(-8448), S2F(-9984), S2F(-9472), S2F(-15104), S2F(-14592), S2F(-16128), S2F(-15616), S2F(-13056), S2F(-12544), S2F(-14080), S2F(-13568), S2F(-344),
    S2F(-328), S2F(-376), S2F(-360), S2F(-280), S2F(-264), S2F(-312), S2F(-296), S2F(-472), S2F(-456), S2F(-504), S2F(-488), S2F(-408), S2F(-392), S2F(-440), S2F(-424), S2F(-88),
    S2F(-72), S2F(-120), S2F(-104), S2F(-24), S2F(-8), S2F(-56), S2F(-40), S2F(-216), S2F(-200), S2F(-248), S2F(-232), S2F(-152), S2F(-136), S2F(-184), S2F(-168), S2F(-1376),
    S2F(-1312), S2F(-1504), S2F(-1440), S2F(-1120), S2F(-1056), S2F(-1248), S2F(-1184), S2F(-1888), S2F(-1824), S2F(-2016), S2F(-1952), S2F(-1632), S2F(-1568), S2F(-1760), S2F(-1696), S2F(-688),
    S2F(-656), S2F(-752), S2F(-720), S2F(-560), S2F(-528), S2F(-624), S2F(-592), S2F(-944), S2F(-912), S2F(-1008), S2F(-976), S2F(-816), S2F(-784), S2F(-880), S2F(-848), S2F(5504),
    S2F(5248), S2F(6016), S2F(5760), S2F(4480), S2F(4224), S2F(4992), S2F(4736), S2F(7552), S2F(7296), S2F(8064), S2F(7808), S2F(6528), S2F(6272), S2F(7040), S2F(6784), S2F(2752),
    S2F(2624), S2F(3008), S2F(2880), S2F(2240), S2F(2112), S2F(2496), S2F(2368), S2F(3776), S2F(3648), S2F(4032), S2F(3904), S2F(3264), S2F(3136), S2F(3520), S2F(3392), S2F(22016),
    S2F(20992), S2F(24064), S2F(23040), S2F(17920), S2F(16896), S2F(19968), S2F(18944), S2F(30208), S2F(29184), S2F(32256), S2F(31232), S2F(26112), S2F(25088), S2F(28160), S2F(27136), S2F(11008),
    S2F(10496), S2F(12032), S2F(11520), S2F(8960), S2F(8448), S2F(9984), S2F(9472), S2F(15104), S2F(14592), S2F(16128), S2F(15616), S2F(13056), S2F(12544), S2F(14080), S2F(13568), S2F(344),
    S2F(328), S2F(376), S2F(360), S2F(280), S2F(264), S2F(312), S2F(296), S2F(472), S2F(456), S2F(504), S2F(488), S2F(408), S2F(392), S2F(440), S2F(424), S2F(88),
    S2F(72), S2F(120), S2F(104), S2F(24), S2F(8), S2F(56), S2F(40), S2F(216), S2F(200), S2F(248), S2F(232), S2F(152), S2F(136), S2F(184), S2F(168), S2F(1376),
    S2F(1312), S2F(1504), S2F(1440), S2F(1120), S2F(1056), S2F(1248), S2F(1184), S2F(1888), S2F(1824), S2F(2016), S2F(1952), S2F(1632), S2F(1568), S2F(1760), S2F(1696), S2F(688),
    S2F(656), S2F(752), S2F(720), S2F(560), S2F(528), S2F(624), S2F(592), S2F(944), S2F(912), S2F(1008), S2F(976), S2F(816), S2F(784), S2F(880), S2F(848)
};

#undef S2F
