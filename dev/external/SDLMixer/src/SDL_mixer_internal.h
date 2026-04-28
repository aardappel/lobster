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

/* This is for debugging and/or pulling the fire alarm. */
#ifndef SDL_MIXER_FORCE_SCALAR_FALLBACK
#  define SDL_MIXER_FORCE_SCALAR_FALLBACK 0
#endif
#if SDL_MIXER_FORCE_SCALAR_FALLBACK
#  define SDL_DISABLE_SSE
#  define SDL_DISABLE_NEON
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_intrin.h>

#if defined(SDL_SSE_INTRINSICS)  /* if you are on x86 or x86-64, we assume you have SSE1 by now. */
#define SDL_MIXER_NEED_SCALAR_FALLBACK 0
#elif defined(SDL_NEON_INTRINSICS) && (defined(__ARM_ARCH) && (__ARM_ARCH >= 8))  /* ARMv8 always has NEON. */
#define SDL_MIXER_NEED_SCALAR_FALLBACK 0
#elif defined(SDL_NEON_INTRINSICS) && (defined(__APPLE__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7))   /* All ARMv7 chips from Apple have NEON. */
#define SDL_MIXER_NEED_SCALAR_FALLBACK 0
#elif defined(SDL_NEON_INTRINSICS) && (defined(__WINDOWS__) || defined(__WINRT__)) && defined(_M_ARM)  /* all WinRT-level Microsoft devices have NEON */
#define SDL_MIXER_NEED_SCALAR_FALLBACK 0
#else
#define SDL_MIXER_NEED_SCALAR_FALLBACK 1
#endif

#if defined(SDL_SSE_INTRINSICS)   /* we assume you always have this on x86/x86-64 chips. SSE1 is 20+ years old! */
#define MIX_HasSSE 1
#endif

#if defined(SDL_NEON_INTRINSICS)
#if SDL_MIXER_NEED_SCALAR_FALLBACK
extern bool MIX_HasNEON;
#else
#define MIX_HasNEON 1
#endif
#endif

#include "SDL3_mixer/SDL_mixer.h"

typedef enum MIX_SpatializationMode
{
    MIX_SPATIALIZATION_NONE,
    MIX_SPATIALIZATION_STEREO,
    MIX_SPATIALIZATION_3D
} MIX_SpatializationMode;

// Vector Based Amplitude Panning stuff, for surround sound positional audio.
// VBAP code originally from https://github.com/drbafflegab/vbap/ ... CC0 license (public domain).
#define MIX_VBAP2D_MAX_RESOLUTION 3600
#define MIX_VBAP2D_MAX_SPEAKER_COUNT 8   // original code had 64, assumed you'd use less, but we're hardcoding our current maximum.
#define MIX_VBAP2D_RESOLUTION 36   // 10 degrees per division

typedef struct MIX_VBAP2D_Bucket { Uint8 speaker_pair; } MIX_VBAP2D_Bucket;
typedef struct MIX_VBAP2D_Matrix { float a00, a01, a10, a11; } MIX_VBAP2D_Matrix;

typedef struct MIX_VBAP2D
{
    int speaker_count;
    MIX_VBAP2D_Bucket buckets[MIX_VBAP2D_RESOLUTION];
    MIX_VBAP2D_Matrix matrices[MIX_VBAP2D_MAX_SPEAKER_COUNT-1];   // the upper ones all have an LFE channel, which we don't track here, so minus one.
} MIX_VBAP2D;

void MIX_VBAP2D_Init(MIX_VBAP2D *vbap2d, int speaker_count);


// Clamp an IOStream to a subset of its available data...this is used to cut ID3 (etc) tags off
//  both ends of an audio file, making it look like the file just doesn't have those bytes.

typedef struct MIX_IoClamp
{
    SDL_IOStream *io;
    Sint64 start;
    Sint64 length;
    Sint64 pos;
} MIX_IoClamp;

extern SDL_IOStream *MIX_OpenIoClamp(MIX_IoClamp *clamp, SDL_IOStream *io);


typedef struct MIX_Decoder
{
    const char *name;
    bool (SDLCALL *init)(void);   // initialize the decoder (load external libraries, etc).
    bool (SDLCALL *init_audio)(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata);  // see if it's a supported format, init spec, set metadata in props, allocate static userdata and payload.
    bool (SDLCALL *init_track)(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata);  // init decoder instance data for a single track.
    bool (SDLCALL *decode)(void *track_userdata, SDL_AudioStream *stream);
    bool (SDLCALL *seek)(void *track_userdata, Uint64 frame);
    bool (SDLCALL *jump_to_order)(void *track_userdata, int order);
    void (SDLCALL *quit_track)(void *track_userdata);
    void (SDLCALL *quit_audio)(void *audio_userdata);
    void (SDLCALL *quit)(void);   // deinitialize the decoder (unload external libraries, etc).
} MIX_Decoder;

typedef enum MIX_TrackState
{
    MIX_STATE_STOPPED,
    MIX_STATE_PAUSED,
    MIX_STATE_PLAYING
} MIX_TrackState;

struct MIX_Audio
{
    SDL_AtomicInt refcount;
    SDL_PropertiesID props;
    SDL_AudioSpec spec;
    const MIX_Decoder *decoder;
    void *decoder_userdata;
    const void *precache;    // non-NULL if this cached the audio data (might be NULL if we're feeding from an external SDL_IOStream).
    size_t precachelen;
    bool free_precache;
    Sint64 duration_frames;
    Sint64 clamp_offset;
    Sint64 clamp_length;
    MIX_Audio *prev;  // double-linked list for all_audios.
    MIX_Audio *next;
};

struct MIX_Track
{
    float SDL_ALIGNED(16) position3d[4];   // we only need the X, Y, and Z coords, but the 4th element makes this SIMD-friendly.
    MIX_SpatializationMode spatialization_mode;
    float spatialization_panning[2];
    int spatialization_speakers[2];
    MIX_Mixer *mixer;
    SDL_PropertiesID props;
    float *input_buffer;  // a place to process audio as it progresses through the callback.
    size_t input_buffer_len;  // number of bytes allocated to input_buffer.
    MIX_Audio *input_audio;    // non-NULL if used with MIX_SetTrackAudioStream. Holds a reference.
    SDL_IOStream *io;  // used for MIX_SetTrackAudio and MIX_SetTrackIOStream. Might be owned by us (SDL_IOFromConstMem of MIX_Audio::precache), or owned by the app.
    MIX_IoClamp ioclamp;  // used for MIX_SetTrackAudio and MIX_SetTrackIOStream.
    bool closeio;  // true if we should close `io` when changing track data.
    bool halt_when_exhausted;  // true if we should stop the track when input runs out.
    SDL_AudioStream *input_stream;  // used for both MIX_SetTrackAudio and MIX_SetTrackAudioStream. Maybe not owned by SDL_mixer!
    SDL_AudioStream *internal_stream;  // used with MIX_SetTrackAudio, where it is also assigned to input_stream. Owned by SDL_mixer!
    void *decoder_userdata;  // MIX_Decoder-specific data for this run, if any.
    SDL_AudioSpec output_spec;  // processed data we send to SDL is in this format.
    SDL_AudioStream *output_stream;  // the stream that is bound to the audio device.
    MIX_TrackState state;  // playing, paused, stopped.
    Uint64 position;   // sample frames played from start of file.
    Sint64 silence_frames;  // number of frames of silence to mix at the end of the track.
    Sint64 max_frame;   // consider audio at EOF at this many sample frame position.
    bool fire_and_forget;  // true if this is a MIX_Track managed internally for fire-and-forget playback.
    Sint64 total_fade_frames;  // fade in or out for this many sample frames.
    Sint64 fade_frames;  // remaining frames to fade.
    int fade_direction;  // -1: fade out  0: don't fade  1: fade in
    float fade_start_gain;  // between 0.0f and 1.0f. Fade with this volume as the starting point (fade-in only).
    int loops_remaining;  // seek to loop_start and continue this many more times at end of input. Negative to loop forever.
    int loop_start;      // sample frame position for loops to begin, so you can play an intro once and then loop from an internal point thereafter.
    SDL_PropertiesID tags;  // lookup tags to see if they are currently applied to this track (true or false).
    MIX_TrackMixCallback raw_callback;
    void *raw_callback_userdata;
    MIX_TrackMixCallback cooked_callback;
    void *cooked_callback_userdata;
    MIX_TrackStoppedCallback stopped_callback;
    void *stopped_callback_userdata;
    MIX_Group *group; // might be default_group, which should not be returned to the app (the app sees that as a NULL group).
    MIX_Track *prev;  // double-linked list for all_tracks.
    MIX_Track *next;
    MIX_Track *group_prev;  // double-linked list for the owning group.
    MIX_Track *group_next;
    MIX_Track *fire_and_forget_next;  // linked list for the fire-and-forget pool.
};

struct MIX_Group
{
    MIX_Mixer *mixer;
    MIX_Track *tracks;
    SDL_PropertiesID props;
    MIX_GroupMixCallback postmix_callback;
    void *postmix_callback_userdata;
    MIX_Group *prev;  // double-linked list for all_groups.
    MIX_Group *next;
};

struct MIX_Mixer
{
    SDL_AudioStream *output_stream;
    SDL_AudioSpec spec;
    SDL_AudioDeviceID device_id;  // can be zero if created from MIX_CreateMixer instead of MIX_CreateMixerDevice.
    SDL_PropertiesID props;
    SDL_PropertiesID track_tags;
    MIX_Group *default_group;
    MIX_Track *all_tracks;
    MIX_Track *fire_and_forget_pool;  // these are also listed in all_tracks.
    MIX_Group *all_groups;
    MIX_PostMixCallback postmix_callback;
    void *postmix_callback_userdata;
    float *mix_buffer;
    size_t mix_buffer_allocation;
    int actual_mixed_bytes;   // on each iteration of the mixer, number of bytes of real mixed audio, ignoring silence at end if no audio was available to mix there.
    float gain;
    MIX_VBAP2D vbap2d;
    MIX_Mixer *prev;  // double-linked list for all_mixers.
    MIX_Mixer *next;
};

// these are not (currently) available in the public API, and may change names or functionality, or be removed.
#define MIX_PROP_DECODER_NAME_STRING "SDL_mixer.decoder.name"
#define MIX_PROP_DECODER_FORMAT_NUMBER "SDL_mixer.decoder.format"
#define MIX_PROP_DECODER_CHANNELS_NUMBER "SDL_mixer.decoder.channels"
#define MIX_PROP_DECODER_FREQ_NUMBER "SDL_mixer.decoder.freq"
#define MIX_PROP_DECODER_SINEWAVE_HZ_NUMBER "SDL_mixer.decoder.sinewave.hz"
#define MIX_PROP_DECODER_SINEWAVE_AMPLITUDE_FLOAT "SDL_mixer.decoder.sinewave.amplitude"
#define MIX_PROP_DECODER_SINEWAVE_MS_NUMBER "SDL_mixer.decoder.sinewave.ms"
#define MIX_PROP_DECODER_WAVPACK_WVC_IOSTREAM_POINTER "SDL_mixer.decoder.wavpack.wvc_iostream"
#define MIX_PROP_DECODER_WAVPACK_WVC_PATH_STRING "SDL_mixer.decoder.wavpack.wvc_path"
#define MIX_PROP_DECODER_FLUIDSYNTH_SOUNDFONT_IOSTREAM_POINTER "SDL_mixer.decoder.fluidsynth.soundfont_iostream"
#define MIX_PROP_DECODER_FLUIDSYNTH_SOUNDFONT_PATH_STRING "SDL_mixer.decoder.fluidsynth.soundfont_path"
#define MIX_PROP_DECODER_FLUIDSYNTH_PROPS_NUMBER "SDL_mixer.decoder.fluidsynth.props"
#define MIX_PROP_AUDIO_LOAD_PATH_STRING "SDL_mixer.audio.load.path"
#define MIX_PROP_AUDIO_LOAD_ONDEMAND_BOOLEAN "SDL_mixer.audio.load.ondemand"

typedef struct MIX_TagList
{
    MIX_Track **tracks;
    size_t num_tracks;
    size_t num_allocated;
    SDL_RWLock *rwlock;
} MIX_TagList;

struct MIX_AudioDecoder
{
    MIX_Audio *audio;
    void *track_userdata;
    SDL_IOStream *io;
    bool closeio;
    SDL_AudioStream *stream;
    MIX_AudioDecoder *prev;  // double-linked list for all_audiodecoders.
    MIX_AudioDecoder *next;
};


// Parse through an SDL_IOStream for tags (ID3, APE, MusicMatch, etc), and add metadata to props.
// !!! FIXME: see FIXME in the function's implementation; just ignore return values from this function for now.
extern bool MIX_ReadMetadataTags(SDL_IOStream *io, SDL_PropertiesID props, MIX_IoClamp *clamp);

// Various Ogg-based decoders use this (Vorbis, FLAC, Opus, etc).
typedef struct MIX_OggLoop
{
    Sint64 start;
    Sint64 end;
    Sint64 len;
    Sint64 count;
    bool active;
} MIX_OggLoop;

void MIX_ParseOggComments(SDL_PropertiesID props, int freq, const char *vendor, const char * const *user_comments, int num_comments, MIX_OggLoop *loop);

// `panning` and `speakers` need to be arrays of 2 elements each, to be filled in with what speakers to write to, and at what gain. `position` must be 16 bytes (only 12 are used), aligned to 16 bytes.
void MIX_Spatialize(const MIX_VBAP2D *vbap2d, const float *position, float *panning, int *speakers);

// if we think `io` is backed by a memory buffer, return its pointer and buffer length for direct access.
void *MIX_GetConstIOBuffer(SDL_IOStream *io, size_t *datalen);

// Slurp in all the data from an SDL_IOStream; if it appears to be memory-based, return the pointer with no allocation or copy made.
void *MIX_SlurpConstIO(SDL_IOStream *io, size_t *datalen, bool *copied);


// mu-Law and a-Law lookup tables.
extern const float MIX_alawToFloat[256];
extern const float MIX_ulawToFloat[256];

// these might not all be available, but they are all declared here as if they are.
extern const MIX_Decoder MIX_Decoder_AU;
extern const MIX_Decoder MIX_Decoder_VOC;
extern const MIX_Decoder MIX_Decoder_WAV;
extern const MIX_Decoder MIX_Decoder_AIFF;
extern const MIX_Decoder MIX_Decoder_MPG123;
extern const MIX_Decoder MIX_Decoder_DRMP3;
extern const MIX_Decoder MIX_Decoder_VORBIS;
extern const MIX_Decoder MIX_Decoder_STBVORBIS;
extern const MIX_Decoder MIX_Decoder_OPUS;
extern const MIX_Decoder MIX_Decoder_FLAC;
extern const MIX_Decoder MIX_Decoder_DRFLAC;
extern const MIX_Decoder MIX_Decoder_TIMIDITY;
extern const MIX_Decoder MIX_Decoder_FLUIDSYNTH;
extern const MIX_Decoder MIX_Decoder_WAVPACK;
extern const MIX_Decoder MIX_Decoder_GME;
extern const MIX_Decoder MIX_Decoder_XMP;
extern const MIX_Decoder MIX_Decoder_SINEWAVE;
extern const MIX_Decoder MIX_Decoder_RAW;

