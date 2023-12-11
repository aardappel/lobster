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

#if defined(MUSIC_OGG) && defined(OGG_USE_STB)

/* This file supports Ogg Vorbis music streams using a modified stb_vorbis module */

#include "music_ogg.h"
#include "utils.h"
#include "SDL_assert.h"
#include "SDL_version.h"
#if !SDL_VERSION_ATLEAST(2, 0, 9)
#include <math.h> /* for missing exp() */
#endif

#define STB_VORBIS_SDL 1 /* for SDL_mixer-specific stuff. */
#define STB_VORBIS_NO_STDIO 1
#define STB_VORBIS_NO_CRT 1
#define STB_VORBIS_NO_PUSHDATA_API 1
#define STB_VORBIS_MAX_CHANNELS 32   /* For 7.1 surround sound */
#define STB_FORCEINLINE SDL_FORCE_INLINE
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define STB_VORBIS_BIG_ENDIAN 1
#endif
#define STBV_CDECL SDLCALL /* for SDL_qsort */

#ifdef assert
#undef assert
#endif
#ifdef memset
#undef memset
#endif
#ifdef memcpy
#undef memcpy
#endif
#define assert SDL_assert
#define memset SDL_memset
#define memcmp SDL_memcmp
#define memcpy SDL_memcpy
#define qsort SDL_qsort
#define malloc SDL_malloc
#define realloc SDL_realloc
#define free SDL_free

#define pow SDL_pow
#define floor SDL_floor
#define ldexp(v, e) SDL_scalbn((v), (e))
#define abs(x) SDL_abs(x)
#define cos(x) SDL_cos(x)
#define sin(x) SDL_sin(x)
#define log(x) SDL_log(x)
#if SDL_VERSION_ATLEAST(2, 0, 9)
#define exp(x) SDL_exp(x) /* Available since SDL 2.0.9 */
#endif

/* Workaround to don't conflict with another statically-linked stb-vorbis */
#define stb_vorbis_get_info _mix_stb_vorbis_get_info
#define stb_vorbis_get_comment _mix_stb_vorbis_get_comment
#define stb_vorbis_get_error _mix_stb_vorbis_get_error
#define stb_vorbis_close _mix_stb_vorbis_close
#define stb_vorbis_get_sample_offset _mix_stb_vorbis_get_sample_offset
#define stb_vorbis_get_playback_sample_offset _mix_stb_vorbis_get_playback_sample_offset
#define stb_vorbis_get_file_offset _mix_stb_vorbis_get_file_offset
#define stb_vorbis_open_pushdata _mix_stb_vorbis_open_pushdata
#define stb_vorbis_decode_frame_pushdata _mix_stb_vorbis_decode_frame_pushdata
#define stb_vorbis_flush_pushdata _mix_stb_vorbis_flush_pushdata
#define stb_vorbis_decode_filename _mix_stb_vorbis_decode_filename
#define stb_vorbis_decode_memory _mix_stb_vorbis_decode_memory
#define stb_vorbis_open_memory _mix_stb_vorbis_open_memory
#define stb_vorbis_open_filename _mix_stb_vorbis_open_filename
#define stb_vorbis_open_file _mix_stb_vorbis_open_file
#define stb_vorbis_open_file_section _mix_stb_vorbis_open_file_section
#define stb_vorbis_open_rwops_section _mix_stb_vorbis_open_rwops_section
#define stb_vorbis_open_rwops _mix_stb_vorbis_open_rwops
#define stb_vorbis_seek_frame _mix_stb_vorbis_seek_frame
#define stb_vorbis_seek _mix_stb_vorbis_seek
#define stb_vorbis_seek_start _mix_stb_vorbis_seek_start
#define stb_vorbis_stream_length_in_samples _mix_stb_vorbis_stream_length_in_samples
#define stb_vorbis_stream_length_in_seconds _mix_stb_vorbis_stream_length_in_seconds
#define stb_vorbis_get_frame_float _mix_stb_vorbis_get_frame_float
#define stb_vorbis_get_frame_short_interleaved _mix_stb_vorbis_get_frame_short_interleaved
#define stb_vorbis_get_frame_short _mix_stb_vorbis_get_frame_short
#define stb_vorbis_get_samples_float_interleaved _mix_stb_vorbis_get_samples_float_interleaved
#define stb_vorbis_get_samples_float _mix_stb_vorbis_get_samples_float
#define stb_vorbis_get_samples_short_interleaved _mix_stb_vorbis_get_samples_short_interleaved
#define stb_vorbis_get_samples_short _mix_stb_vorbis_get_samples_short
/* Workaround to don't conflict with another statically-linked stb-vorbis: END */

#include "stb_vorbis/stb_vorbis.h"

/* Global flags which are applying on initializing of Vorbis player with a file */
typedef struct {
    int multitrack;
    int channels_per_track;
    int total_tracks;
    double speed;
} OGGVorbis_Setup;

static OGGVorbis_Setup oggvorbis_setup = {
    0, 0, 0, 1.0
};

static void OGGVorbis_SetDefault(OGGVorbis_Setup *setup)
{
    setup->multitrack = 0;
    setup->channels_per_track = 0;
    setup->total_tracks = 0;
    setup->speed = 1.0;
}

typedef struct {
    SDL_RWops *src;
    int freesrc;
    int play_count;
    int volume;
    stb_vorbis *vf;
    stb_vorbis_info vi;
    int section;
    SDL_AudioStream *stream;
    char *buffer;
    int buffer_size;
    int loop;
    Sint64 loop_start;
    Sint64 loop_end;
    Sint64 loop_len;
    Sint64 full_length;
    int computed_src_rate;
    double speed;

    SDL_bool multitrack;
    float *multitrack_buffer[STB_VORBIS_MAX_CHANNELS];
    int multitrack_mute[STB_VORBIS_MAX_CHANNELS];
    int multitrack_buffer_samples;
    int multitrack_channels;
    int multitrack_tracks;

    Mix_MusicMetaTags tags;
} OGG_music;

static int set_ov_error(const char *function, int error)
{
#define HANDLE_ERROR_CASE(X) case X: Mix_SetError("%s: %s", function, #X); break;
    switch (error) {
    HANDLE_ERROR_CASE(VORBIS_need_more_data)
    HANDLE_ERROR_CASE(VORBIS_invalid_api_mixing)
    HANDLE_ERROR_CASE(VORBIS_outofmem)
    HANDLE_ERROR_CASE(VORBIS_feature_not_supported)
    HANDLE_ERROR_CASE(VORBIS_too_many_channels)
    HANDLE_ERROR_CASE(VORBIS_file_open_failure)
    HANDLE_ERROR_CASE(VORBIS_seek_without_length)
    HANDLE_ERROR_CASE(VORBIS_unexpected_eof)
    HANDLE_ERROR_CASE(VORBIS_seek_invalid)
    HANDLE_ERROR_CASE(VORBIS_invalid_setup)
    HANDLE_ERROR_CASE(VORBIS_invalid_stream)
    HANDLE_ERROR_CASE(VORBIS_missing_capture_pattern)
    HANDLE_ERROR_CASE(VORBIS_invalid_stream_structure_version)
    HANDLE_ERROR_CASE(VORBIS_continued_packet_flag_invalid)
    HANDLE_ERROR_CASE(VORBIS_incorrect_stream_serial_number)
    HANDLE_ERROR_CASE(VORBIS_invalid_first_page)
    HANDLE_ERROR_CASE(VORBIS_bad_packet_type)
    HANDLE_ERROR_CASE(VORBIS_cant_find_last_page)
    HANDLE_ERROR_CASE(VORBIS_seek_failed)
    HANDLE_ERROR_CASE(VORBIS_ogg_skeleton_not_supported)
    default:
        Mix_SetError("%s: unknown error %d\n", function, error);
        break;
    }
    return -1;
}

static int OGG_Seek(void *context, double time);
static void OGG_Delete(void *context);

static int OGG_UpdateSpeed(OGG_music *music)
{
    if (music->computed_src_rate != -1) {
        return 0;
    }

    if (music->stream) {
        SDL_AudioStreamFlush(music->stream);
        if (SDL_AudioStreamAvailable(music->stream) > 0) {
            return -1;
        }
        SDL_FreeAudioStream(music->stream);
        music->stream = NULL;
    }

    music->computed_src_rate = music->vi.sample_rate * music->speed;
    if (music->computed_src_rate < 1000) {
        music->computed_src_rate = 1000;
    }

    music->stream = SDL_NewAudioStream(AUDIO_F32SYS, (Uint8)(music->multitrack ? music->multitrack_channels : music->vi.channels), music->computed_src_rate,
                                       music_spec.format, music_spec.channels, music_spec.freq);
    if (!music->stream) {
        return -2;
    }

    return 0;
}

static int OGG_UpdateSection(OGG_music *music)
{
    stb_vorbis_info vi;
    Uint8 in_channels;
    int i;

    vi = stb_vorbis_get_info(music->vf);

    if (vi.channels == music->vi.channels && vi.sample_rate == music->vi.sample_rate) {
        return 0;
    }
    SDL_memcpy(&music->vi, &vi, sizeof(vi));

    music->computed_src_rate = music->vi.sample_rate * music->speed;
    if (music->computed_src_rate < 1000) {
        music->computed_src_rate = 1000;
    }

    if (music->buffer) {
        SDL_free(music->buffer);
        music->buffer = NULL;
    }

    for (i = 0; i < STB_VORBIS_MAX_CHANNELS; ++i) {
        if (music->multitrack_buffer[i]) {
            SDL_free(music->multitrack_buffer[i]);
            music->multitrack_buffer[i] = NULL;
        }
    }

    if (music->stream) {
        SDL_FreeAudioStream(music->stream);
        music->stream = NULL;
    }

    if (music->multitrack) {
        in_channels = (Uint8)music->multitrack_channels;
    } else {
        in_channels = (Uint8)vi.channels;
    }

    music->stream = SDL_NewAudioStream(AUDIO_F32SYS, in_channels, music->computed_src_rate,
                                       music_spec.format, music_spec.channels, music_spec.freq);
    if (!music->stream) {
        return -1;
    }

    music->buffer_size = music_spec.samples * (int)sizeof(float) * vi.channels;
    if (music->buffer_size <= 0) {
        return -1;
    }

    music->buffer = (char *)SDL_malloc((size_t)music->buffer_size);
    if (!music->buffer) {
        return -1;
    }

    if (music->multitrack) {
        if (music->multitrack_channels * music->multitrack_tracks > music->vi.channels) {
            Mix_SetError("Invalid multitrack setup: product of channels and tracks must not be bigger than actual channels number at this file.");
            return -1;
        }

        music->multitrack_buffer_samples = music_spec.samples;
        for (i = 0; i < music->multitrack_channels * music->multitrack_tracks; ++i) {
            music->multitrack_buffer[i] = (float *)SDL_malloc((size_t)music->multitrack_buffer_samples * sizeof(float));
            if (!music->multitrack_buffer[i]) {
                return -1;
            }
        }
    }

    return 0;
}

static void process_args(const char *args, OGGVorbis_Setup *setup)
{
#define ARG_BUFFER_SIZE    1024
    char arg[ARG_BUFFER_SIZE];
    char type = '-';
    size_t maxlen = 0;
    size_t i, j = 0;
    int value_opened = 0;
    if (args == NULL) {
        return;
    }
    maxlen = SDL_strlen(args);
    if (maxlen == 0) {
        return;
    }

    maxlen += 1;
    OGGVorbis_SetDefault(setup);

    for (i = 0; i < maxlen; i++) {
        char c = args[i];
        if (value_opened == 1) {
            if ((c == ';') || (c == '\0')) {
                int value = 0;
                arg[j] = '\0';
                value = SDL_atoi(arg);
                switch(type)
                {
                case 'm':
                    setup->multitrack = value;
                    break;
                case 'c':
                    setup->channels_per_track = value;
                    break;
                case 'r':
                    setup->total_tracks = value;
                    break;
                case 's':
                    if (arg[0] == '=') {
                        setup->speed = SDL_strtod(arg + 1, NULL);
                        if (setup->speed < 0.0) {
                            setup->speed = 1.0;
                        }
                    }
                    break;
                case '\0':
                    break;
                default:
                    break;
                }
                value_opened = 0;
            }
            arg[j++] = c;
        } else {
            if (c == '\0') {
                return;
            }
            type = c;
            value_opened = 1;
            j = 0;
        }
    }
#undef ARG_BUFFER_SIZE
}

/* Load an OGG stream from an SDL_RWops object */
static void *OGG_CreateFromRWex(SDL_RWops *src, int freesrc, const char *args)
{
    OGG_music *music;
    stb_vorbis_comment vc;
    long rate;
    SDL_bool is_loop_length = SDL_FALSE;
    int i, error;
    OGGVorbis_Setup setup = oggvorbis_setup;

    music = (OGG_music *)SDL_calloc(1, sizeof *music);
    if (!music) {
        SDL_OutOfMemory();
        return NULL;
    }
    music->src = src;
    music->volume = MIX_MAX_VOLUME;
    music->computed_src_rate = -1;
    music->section = -1;

    process_args(args, &setup);

    music->speed = setup.speed;

    music->vf = stb_vorbis_open_rwops(src, 0, &error, NULL);

    if (music->vf == NULL) {
        set_ov_error("stb_vorbis_open_rwops", error);
        SDL_free(music);
        return NULL;
    }

    if (setup.multitrack > 0) {
        music->multitrack = SDL_TRUE;
        music->multitrack_channels = setup.channels_per_track;
        music->multitrack_tracks = setup.total_tracks;
    }

    if (OGG_UpdateSection(music) < 0) {
        OGG_Delete(music);
        return NULL;
    }

    music->vi = stb_vorbis_get_info(music->vf);
    if ((int)music->vi.sample_rate <= 0) {
        Mix_SetError("Invalid sample rate value");
        OGG_Delete(music);
        return NULL;
    }

    rate = music->vi.sample_rate;
    vc = stb_vorbis_get_comment(music->vf);
    if (vc.comment_list != NULL) {
        for (i = 0; i < vc.comment_list_length; i++) {
            char *param = SDL_strdup(vc.comment_list[i]);
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

    music->full_length = stb_vorbis_stream_length_in_samples(music->vf);
    if ((music->loop_end > 0) && (music->loop_end <= music->full_length) &&
        (music->loop_start < music->loop_end)) {
        music->loop = 1;
    }

    OGG_Seek(music, 0.0);

    music->freesrc = freesrc;
    return music;
}

static void *OGG_CreateFromRW(struct SDL_RWops *src, int freesrc)
{
    return OGG_CreateFromRWex(src, freesrc, NULL);
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
    int filled, amount, channels, result, i, j, k;
    int section;
    Sint64 pcmPos;
    float *cur;
    float *cur_src[STB_VORBIS_MAX_CHANNELS];

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

    if (music->multitrack) {
        channels = music->multitrack_channels;
        amount = stb_vorbis_get_samples_float(music->vf,
                                              music->multitrack_channels * music->multitrack_tracks,
                                              music->multitrack_buffer,
                                              music_spec.samples);

        cur = (float *)music->buffer;
        SDL_memcpy(cur_src, music->multitrack_buffer, sizeof(float *) * STB_VORBIS_MAX_CHANNELS);
        SDL_memset(music->buffer, 0, music->buffer_size);

        for (i = 0; i < music_spec.samples; ++i) {
            for (j = 0; j < music->multitrack_tracks; ++j) {
                if (music->multitrack_mute[j]) {
                    continue;
                }

                for (k = 0; k < music->multitrack_channels; ++k) {
                    cur[k] += *(cur_src[(j * music->multitrack_channels) + k]++);
                }
            }

            cur += music->multitrack_channels;
        }
    } else {
        channels = music->vi.channels;
        amount = stb_vorbis_get_samples_float_interleaved(music->vf,
                                                          music->vi.channels,
                                                          (float *)music->buffer,
                                                          music_spec.samples * music->vi.channels);
    }

    amount *= channels * sizeof(float);

    if (section != music->section) {
        music->section = section;
        if (OGG_UpdateSection(music) < 0) {
            return -1;
        }
    }

    if (music->computed_src_rate < 0) {
        result = OGG_UpdateSpeed(music);
        if (result == -1) {
            return 0; /* Has data to be flush */
        } else if (result == -2) {
            return -1; /* Error has occured */
        }
    }

    pcmPos = stb_vorbis_get_playback_sample_offset(music->vf);
    if (music->loop && (music->play_count != 1) && (pcmPos >= music->loop_end)) {
        amount -= (int)((pcmPos - music->loop_end) * channels) * (int)sizeof(float);
        result = stb_vorbis_seek(music->vf, music->loop_start);
        if (!result) {
            set_ov_error("stb_vorbis_seek", stb_vorbis_get_error(music->vf));
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

    result = stb_vorbis_seek(music->vf, (time * music->vi.sample_rate));
    if (!result) {
        set_ov_error("stb_vorbis_seek", stb_vorbis_get_error(music->vf));
        return -1;
    }
    return 0;
}

static double OGG_Tell(void *context)
{
    OGG_music *music = (OGG_music *)context;
    return (double)stb_vorbis_get_playback_sample_offset(music->vf) / music->vi.sample_rate;
}

/* Return music duration in seconds */
static double OGG_Duration(void *context)
{
    OGG_music *music = (OGG_music *)context;
    return (double)music->full_length / music->vi.sample_rate;
}

static int OGG_SetSpeed(void *context, double speed)
{
    OGG_music *music = (OGG_music *)context;
    if (speed <= 0.01) {
        speed = 0.01;
    }
    music->speed = speed;
    music->computed_src_rate = -1;
    return 0;
}

static double OGG_GetSpeed(void *context)
{
    OGG_music *music = (OGG_music *)context;
    return music->speed;
}


static double   OGG_LoopStart(void *music_p)
{
    OGG_music *music = (OGG_music *)music_p;
    if (music->loop > 0) {
        return (double)music->loop_start / music->vi.sample_rate;
    }
    return -1.0;
}

static double   OGG_LoopEnd(void *music_p)
{
    OGG_music *music = (OGG_music *)music_p;
    if (music->loop > 0) {
        return (double)music->loop_end / music->vi.sample_rate;
    }
    return -1.0;
}

static double   OGG_LoopLength(void *music_p)
{
    OGG_music *music = (OGG_music *)music_p;
    if (music->loop > 0) {
        return (double)music->loop_len / music->vi.sample_rate;
    }
    return -1.0;
}

static int OGG_GetTracksCount(void *music_p)
{
    OGG_music *music = (OGG_music *)music_p;
    if (music->multitrack) {
        return music->multitrack_tracks;
    }
    return -1;
}

static int OGG_SetTrackMute(void *music_p, int track, int mute)
{
    OGG_music *music = (OGG_music *)music_p;
    if (music->multitrack && track >= 0 && track < music->multitrack_tracks) {
        music->multitrack_mute[track] = mute;
        return 0;
    }
    return -1;
}

/* Close the given OGG stream */
static void OGG_Delete(void *context)
{
    int i;
    OGG_music *music = (OGG_music *)context;
    meta_tags_clear(&music->tags);
    stb_vorbis_close(music->vf);
    if (music->stream) {
        SDL_FreeAudioStream(music->stream);
    }
    if (music->buffer) {
        SDL_free(music->buffer);
    }
    for (i = 0; i < STB_VORBIS_MAX_CHANNELS; ++i) {
        if (music->multitrack_buffer[i]) {
            SDL_free(music->multitrack_buffer[i]);
            music->multitrack_buffer[i] = NULL;
        }
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

    NULL,   /* Load */
    NULL,   /* Open */
    OGG_CreateFromRW,
    OGG_CreateFromRWex, /* [MIXER-X] */
    NULL,   /* CreateFromFile */
    NULL,   /* CreateFromFileEx [MIXER-X]*/
    OGG_SetVolume,
    OGG_GetVolume,
    OGG_Play,
    NULL,   /* IsPlaying */
    OGG_GetAudio,
    NULL,   /* Jump */
    OGG_Seek,
    OGG_Tell,
    OGG_Duration,
    NULL,   /* SetTempo [MIXER-X] */
    NULL,   /* GetTempo [MIXER-X] */
    OGG_SetSpeed,
    OGG_GetSpeed,
    NULL,   /* SetPitch [MIXER-X] */
    NULL,   /* GetPitch [MIXER-X] */
    OGG_GetTracksCount, /* [MIXER-X] */
    OGG_SetTrackMute, /* [MIXER-X] */
    OGG_LoopStart,
    OGG_LoopEnd,
    OGG_LoopLength,
    OGG_GetMetaTag,   /* GetMetaTag */
    NULL,   /* GetNumTracks */
    NULL,   /* StartTrack */
    NULL,   /* Pause */
    NULL,   /* Resume */
    OGG_Stop,
    OGG_Delete,
    NULL,   /* Close */
    NULL    /* Unload */
};

#endif /* MUSIC_OGG */

/* vi: set ts=4 sw=4 expandtab: */
