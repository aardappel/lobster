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

  This file is used to support SDL_LoadMUS playback of FLAC files.
    ~ Austen Dicken (admin@cvpcs.org)
*/

#ifdef MUSIC_FLAC_LIBFLAC

#include "SDL_loadso.h"
#include "SDL_assert.h"

#include "music_flac.h"
#include "utils.h"

#include <FLAC/stream_decoder.h>


typedef struct {
    int loaded;
    void *handle;
    FLAC__StreamDecoder *(*FLAC__stream_decoder_new)(void);
    void (*FLAC__stream_decoder_delete)(FLAC__StreamDecoder *decoder);
    FLAC__StreamDecoderInitStatus (*FLAC__stream_decoder_init_stream)(
                        FLAC__StreamDecoder *decoder,
                        FLAC__StreamDecoderReadCallback read_callback,
                        FLAC__StreamDecoderSeekCallback seek_callback,
                        FLAC__StreamDecoderTellCallback tell_callback,
                        FLAC__StreamDecoderLengthCallback length_callback,
                        FLAC__StreamDecoderEofCallback eof_callback,
                        FLAC__StreamDecoderWriteCallback write_callback,
                        FLAC__StreamDecoderMetadataCallback metadata_callback,
                        FLAC__StreamDecoderErrorCallback error_callback,
                        void *client_data);
    FLAC__bool (*FLAC__stream_decoder_finish)(FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_flush)(FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_process_single)(
                        FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_process_until_end_of_metadata)(
                        FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_process_until_end_of_stream)(
                        FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_seek_absolute)(
                        FLAC__StreamDecoder *decoder,
                        FLAC__uint64 sample);
    FLAC__StreamDecoderState (*FLAC__stream_decoder_get_state)(
                        const FLAC__StreamDecoder *decoder);
    FLAC__uint64 (*FLAC__stream_decoder_get_total_samples)(
                        const FLAC__StreamDecoder *decoder);
    FLAC__bool (*FLAC__stream_decoder_set_metadata_respond)(
                        FLAC__StreamDecoder *decoder,
                        FLAC__MetadataType type);
} flac_loader;

static flac_loader flac;

#ifdef FLAC_DYNAMIC
#define FUNCTION_LOADER(FUNC, SIG) \
    flac.FUNC = (SIG) SDL_LoadFunction(flac.handle, #FUNC); \
    if (flac.FUNC == NULL) { SDL_UnloadObject(flac.handle); return -1; }
#else
#define FUNCTION_LOADER(FUNC, SIG) \
    flac.FUNC = FUNC; \
    if (flac.FUNC == NULL) { Mix_SetError("Missing FLAC.framework"); return -1; }
#endif

static int FLAC_Load(void)
{
    if (flac.loaded == 0) {
#ifdef FLAC_DYNAMIC
        flac.handle = SDL_LoadObject(FLAC_DYNAMIC);
        if (flac.handle == NULL) {
            return -1;
        }
#endif

        FUNCTION_LOADER(FLAC__stream_decoder_new, FLAC__StreamDecoder *(*)(void))
        FUNCTION_LOADER(FLAC__stream_decoder_delete, void (*)(FLAC__StreamDecoder *))
        FUNCTION_LOADER(FLAC__stream_decoder_init_stream, FLAC__StreamDecoderInitStatus (*)(
                        FLAC__StreamDecoder *,
                        FLAC__StreamDecoderReadCallback,
                        FLAC__StreamDecoderSeekCallback,
                        FLAC__StreamDecoderTellCallback,
                        FLAC__StreamDecoderLengthCallback,
                        FLAC__StreamDecoderEofCallback,
                        FLAC__StreamDecoderWriteCallback,
                        FLAC__StreamDecoderMetadataCallback,
                        FLAC__StreamDecoderErrorCallback,
                        void *))
        FUNCTION_LOADER(FLAC__stream_decoder_finish, FLAC__bool (*)(FLAC__StreamDecoder *))
        FUNCTION_LOADER(FLAC__stream_decoder_flush, FLAC__bool (*)(FLAC__StreamDecoder *))
        FUNCTION_LOADER(FLAC__stream_decoder_process_single, FLAC__bool (*)(FLAC__StreamDecoder *))
        FUNCTION_LOADER(FLAC__stream_decoder_process_until_end_of_metadata, FLAC__bool (*)(FLAC__StreamDecoder *))
        FUNCTION_LOADER(FLAC__stream_decoder_process_until_end_of_stream, FLAC__bool (*)(FLAC__StreamDecoder *))
        FUNCTION_LOADER(FLAC__stream_decoder_seek_absolute, FLAC__bool (*)(FLAC__StreamDecoder *, FLAC__uint64))
        FUNCTION_LOADER(FLAC__stream_decoder_get_state, FLAC__StreamDecoderState (*)(const FLAC__StreamDecoder *decoder))
        FUNCTION_LOADER(FLAC__stream_decoder_get_total_samples,
                        FLAC__uint64 (*)(const FLAC__StreamDecoder *))
        FUNCTION_LOADER(FLAC__stream_decoder_set_metadata_respond,
                        FLAC__bool (*)(FLAC__StreamDecoder *,
                                       FLAC__MetadataType))
    }
    ++flac.loaded;

    return 0;
}

static void FLAC_Unload(void)
{
    if (flac.loaded == 0) {
        return;
    }
    if (flac.loaded == 1) {
#ifdef FLAC_DYNAMIC
        SDL_UnloadObject(flac.handle);
#endif
    }
    --flac.loaded;
}


typedef struct {
    int volume;
    int play_count;
    FLAC__StreamDecoder *flac_decoder;
    unsigned sample_rate;
    unsigned channels;
    unsigned bits_per_sample;
    SDL_RWops *src;
    int freesrc;
    SDL_AudioStream *stream;
    int loop;
    FLAC__int64 pcm_pos;
    FLAC__int64 full_length;
    SDL_bool loop_flag;
    FLAC__int64 loop_start;
    FLAC__int64 loop_end;
    FLAC__int64 loop_len;
    Mix_MusicMetaTags tags;
} FLAC_Music;


static int FLAC_Seek(void *context, double position);

static FLAC__StreamDecoderReadStatus flac_read_music_cb(
                                    const FLAC__StreamDecoder *decoder,
                                    FLAC__byte buffer[],
                                    size_t *bytes,
                                    void *client_data)
{
    FLAC_Music *data = (FLAC_Music*)client_data;

    (void)decoder;

    /* make sure there is something to be reading */
    if (*bytes > 0) {
        *bytes = SDL_RWread (data->src, buffer, sizeof (FLAC__byte), *bytes);

        if (*bytes == 0) { /* error or no data was read (EOF) */
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        } else { /* data was read, continue */
            return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
        }
    } else {
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
}

static FLAC__StreamDecoderSeekStatus flac_seek_music_cb(
                                    const FLAC__StreamDecoder *decoder,
                                    FLAC__uint64 absolute_byte_offset,
                                    void *client_data)
{
    FLAC_Music *data = (FLAC_Music*)client_data;

    (void)decoder;

    if (SDL_RWseek(data->src, (Sint64)absolute_byte_offset, RW_SEEK_SET) < 0) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    } else {
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    }
}

static FLAC__StreamDecoderTellStatus flac_tell_music_cb(
                                    const FLAC__StreamDecoder *decoder,
                                    FLAC__uint64 *absolute_byte_offset,
                                    void *client_data)
{
    FLAC_Music *data = (FLAC_Music*)client_data;

    Sint64 pos = SDL_RWtell(data->src);

    (void)decoder;

    if (pos < 0) {
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
    } else {
        *absolute_byte_offset = (FLAC__uint64)pos;
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }
}

static FLAC__StreamDecoderLengthStatus flac_length_music_cb(
                                    const FLAC__StreamDecoder *decoder,
                                    FLAC__uint64 *stream_length,
                                    void *client_data)
{
    FLAC_Music *data = (FLAC_Music*)client_data;

    Sint64 pos = SDL_RWtell(data->src);
    Sint64 length = SDL_RWseek(data->src, 0, RW_SEEK_END);

    (void)decoder;

    if (SDL_RWseek(data->src, pos, RW_SEEK_SET) != pos || length < 0) {
        /* there was an error attempting to return the stream to the original
         * position, or the length was invalid. */
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    } else {
        *stream_length = (FLAC__uint64)length;
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }
}

static FLAC__bool flac_eof_music_cb(
                                const FLAC__StreamDecoder *decoder,
                                void *client_data)
{
    FLAC_Music *data = (FLAC_Music*)client_data;

    Sint64 pos = SDL_RWtell(data->src);
    Sint64 end = SDL_RWseek(data->src, 0, RW_SEEK_END);

    (void)decoder;

    /* was the original position equal to the end (a.k.a. the seek didn't move)? */
    if (pos == end) {
        /* must be EOF */
        return true;
    } else {
        /* not EOF, return to the original position */
        SDL_RWseek(data->src, pos, RW_SEEK_SET);
        return false;
    }
}

static FLAC__StreamDecoderWriteStatus flac_write_music_cb(
                                    const FLAC__StreamDecoder *decoder,
                                    const FLAC__Frame *frame,
                                    const FLAC__int32 *const buffer[],
                                    void *client_data)
{
    FLAC_Music *music = (FLAC_Music *)client_data;
    Sint16 *data;
    unsigned int i, j, channels;
    int shift_amount = 0, amount;

    (void)decoder;

    if (!music->stream) {
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    switch (music->bits_per_sample) {
    case 16:
        shift_amount = 0;
        break;
    case 20:
        shift_amount = 4;
        break;
    case 24:
        shift_amount = 8;
        break;
    default:
        SDL_SetError("FLAC decoder doesn't support %d bits_per_sample", music->bits_per_sample);
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    if (music->channels == 3) {
        /* We'll just drop the center channel for now */
        channels = 2;
    } else {
        channels = music->channels;
    }

    data = SDL_stack_alloc(Sint16, (frame->header.blocksize * channels));
    if (!data) {
        SDL_SetError("Couldn't allocate %d bytes stack memory", (int)(frame->header.blocksize * channels * sizeof(*data)));
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    if (music->channels == 3) {
        Sint16 *dst = data;
        for (i = 0; i < frame->header.blocksize; ++i) {
            Sint16 FL = (Sint16)(buffer[0][i] >> shift_amount);
            Sint16 FR = (Sint16)(buffer[1][i] >> shift_amount);
            Sint16 FCmix = (Sint16)((buffer[2][i] >> shift_amount) * 0.5f);
            int sample;

            sample = (FL + FCmix);
            if (sample > SDL_MAX_SINT16) {
                *dst = SDL_MAX_SINT16;
            } else if (sample < SDL_MIN_SINT16) {
                *dst = SDL_MIN_SINT16;
            } else {
                *dst = (Sint16)sample;
            }
            ++dst;

            sample = (FR + FCmix);
            if (sample > SDL_MAX_SINT16) {
                *dst = SDL_MAX_SINT16;
            } else if (sample < SDL_MIN_SINT16) {
                *dst = SDL_MIN_SINT16;
            } else {
                *dst = (Sint16)sample;
            }
            ++dst;
        }
    } else {
        for (i = 0; i < channels; ++i) {
            Sint16 *dst = data + i;
            for (j = 0; j < frame->header.blocksize; ++j) {
                *dst = (Sint16)(buffer[i][j] >> shift_amount);
                dst += channels;
            }
        }
    }
    amount = (int)(frame->header.blocksize * channels * sizeof(*data));
    music->pcm_pos += (FLAC__int64) frame->header.blocksize;
    if (music->loop && (music->play_count != 1) &&
        (music->pcm_pos >= music->loop_end)) {
        amount -= (music->pcm_pos - music->loop_end) * channels * (int)sizeof(*data);
        music->loop_flag = SDL_TRUE;
    }

    SDL_AudioStreamPut(music->stream, data, amount);
    SDL_stack_free(data);

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void flac_metadata_music_cb(
                    const FLAC__StreamDecoder *decoder,
                    const FLAC__StreamMetadata *metadata,
                    void *client_data)
{
    FLAC_Music *music = (FLAC_Music *)client_data;
    const FLAC__StreamMetadata_VorbisComment *vc;
    int channels;
    unsigned rate;
    char *param, *argument, *value;
    SDL_bool is_loop_length = SDL_FALSE;

    (void)decoder;

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        music->sample_rate = metadata->data.stream_info.sample_rate;
        music->channels = metadata->data.stream_info.channels;
        music->bits_per_sample = metadata->data.stream_info.bits_per_sample;
        /*printf("FLAC: Sample rate = %d, channels = %d, bits_per_sample = %d\n", music->sample_rate, music->channels, music->bits_per_sample);*/

        /* SDL's channel mapping and FLAC channel mapping are the same,
           except for 3 channels: SDL is FL FR LFE and FLAC is FL FR FC
         */
        if (music->channels == 3) {
            channels = 2;
        } else {
            channels = (int)music->channels;
        }

        /* We check for NULL stream later when we get data */
        SDL_assert(!music->stream);
        music->stream = SDL_NewAudioStream(AUDIO_S16SYS, (Uint8)channels, (int)music->sample_rate,
                                          music_spec.format, music_spec.channels, music_spec.freq);
    } else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        FLAC__uint32 i;

        vc  = &metadata->data.vorbis_comment;
        rate = music->sample_rate;

        for (i = 0; i < vc->num_comments; ++i) {
            param = SDL_strdup((const char *) vc->comments[i].entry);
            argument = param;
            value = SDL_strchr(param, '=');

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
}

static void flac_error_music_cb(
                const FLAC__StreamDecoder *decoder,
                FLAC__StreamDecoderErrorStatus status,
                void *client_data)
{
    (void)decoder;
    (void)client_data;

    /* print an SDL error based on the error status */
    switch (status) {
    case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
        SDL_SetError("Error processing the FLAC file [LOST_SYNC].");
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
        SDL_SetError("Error processing the FLAC file [BAD_HEADER].");
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
        SDL_SetError("Error processing the FLAC file [CRC_MISMATCH].");
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
        SDL_SetError("Error processing the FLAC file [UNPARSEABLE].");
        break;
    default:
        SDL_SetError("Error processing the FLAC file [UNKNOWN].");
        break;
    }
}

/* Load an FLAC stream from an SDL_RWops object */
static void *FLAC_CreateFromRW(SDL_RWops *src, int freesrc)
{
    FLAC_Music *music;
    int init_stage = 0;
    int was_error = 1;
    FLAC__int64 full_length;

    music = (FLAC_Music *)SDL_calloc(1, sizeof(*music));
    if (!music) {
        SDL_OutOfMemory();
        return NULL;
    }
    music->src = src;
    music->volume = MIX_MAX_VOLUME;

    music->flac_decoder = flac.FLAC__stream_decoder_new();
    if (music->flac_decoder) {
        init_stage++; /* stage 1! */
        flac.FLAC__stream_decoder_set_metadata_respond(music->flac_decoder,
                    FLAC__METADATA_TYPE_VORBIS_COMMENT);

        if (flac.FLAC__stream_decoder_init_stream(
                    music->flac_decoder,
                    flac_read_music_cb, flac_seek_music_cb,
                    flac_tell_music_cb, flac_length_music_cb,
                    flac_eof_music_cb, flac_write_music_cb,
                    flac_metadata_music_cb, flac_error_music_cb,
                    music) == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
            init_stage++; /* stage 2! */

            if (flac.FLAC__stream_decoder_process_until_end_of_metadata(music->flac_decoder)) {
                was_error = 0;
            } else {
                SDL_SetError("FLAC__stream_decoder_process_until_end_of_metadata() failed");
            }
        } else {
            SDL_SetError("FLAC__stream_decoder_init_stream() failed");
        }
    } else {
        SDL_SetError("FLAC__stream_decoder_new() failed");
    }

    if (was_error) {
        switch (init_stage) {
            case 2:
                flac.FLAC__stream_decoder_finish(music->flac_decoder); /* fallthrough */
            case 1:
                flac.FLAC__stream_decoder_delete(music->flac_decoder); /* fallthrough */
            case 0:
                SDL_free(music);
                break;
        }
        return NULL;
    }

    /* loop_start, loop_end and loop_len get set by metadata callback if tags
     * are present in metadata.
     */
    full_length = (FLAC__int64) flac.FLAC__stream_decoder_get_total_samples(music->flac_decoder);
    if ((music->loop_end > 0) && (music->loop_end <= full_length) &&
        (music->loop_start < music->loop_end)) {
        music->loop = 1;
    }

    music->full_length = full_length;
    music->freesrc = freesrc;
    return music;
}

static const char* FLAC_GetMetaTag(void *context, Mix_MusicMetaTag tag_type)
{
    FLAC_Music *music = (FLAC_Music *)context;
    return meta_tags_get(&music->tags, tag_type);
}


/* Set the volume for an FLAC stream */
static void FLAC_SetVolume(void *context, int volume)
{
    FLAC_Music *music = (FLAC_Music *)context;
    music->volume = volume;
}

/* Get the volume for an FLAC stream */
static int FLAC_GetVolume(void *context)
{
    FLAC_Music *music = (FLAC_Music *)context;
    return music->volume;
}

/* Start playback of a given FLAC stream */
static int FLAC_Play(void *context, int play_count)
{
    FLAC_Music *music = (FLAC_Music *)context;
    music->play_count = play_count;
    return FLAC_Seek(music, 0.0);
}

static void FLAC_Stop(void *context)
{
    FLAC_Music *music = (FLAC_Music *)context;
    SDL_AudioStreamClear(music->stream);
}

/* Read some FLAC stream data and convert it for output */
static int FLAC_GetSome(void *context, void *data, int bytes, SDL_bool *done)
{
    FLAC_Music *music = (FLAC_Music *)context;
    int filled;

    filled = SDL_AudioStreamGet(music->stream, data, bytes);
    if (filled != 0) {
        return filled;
    }

    if (!music->play_count) {
        /* All done */
        *done = SDL_TRUE;
        return 0;
    }

    if (!flac.FLAC__stream_decoder_process_single(music->flac_decoder)) {
        SDL_SetError("FLAC__stream_decoder_process_single() failed");
        return -1;
    }

    if (music->loop_flag) {
        music->pcm_pos = music->loop_start;
        if (flac.FLAC__stream_decoder_seek_absolute(music->flac_decoder, (FLAC__uint64)music->loop_start) ==
                FLAC__STREAM_DECODER_SEEK_ERROR) {
            SDL_SetError("FLAC__stream_decoder_seek_absolute() failed");
            flac.FLAC__stream_decoder_flush(music->flac_decoder);
            return -1;
        } else {
            int play_count = -1;
            if (music->play_count > 0) {
                play_count = (music->play_count - 1);
            }
            music->play_count = play_count;
            music->loop_flag = SDL_FALSE;
        }
    }

    if (flac.FLAC__stream_decoder_get_state(music->flac_decoder) == FLAC__STREAM_DECODER_END_OF_STREAM) {
        if (music->play_count == 1) {
            music->play_count = 0;
            SDL_AudioStreamFlush(music->stream);
        } else {
            int play_count = -1;
            if (music->play_count > 0) {
                play_count = (music->play_count - 1);
            }
            if (FLAC_Play(music, play_count) < 0) {
                return -1;
            }
        }
    }
    return 0;
}

/* Play some of a stream previously started with FLAC_play() */
static int FLAC_GetAudio(void *context, void *data, int bytes)
{
    FLAC_Music *music = (FLAC_Music *)context;
    return music_pcm_getaudio(context, data, bytes, music->volume, FLAC_GetSome);
}

/* Jump (seek) to a given position (position is in seconds) */
static int FLAC_Seek(void *context, double position)
{
    FLAC_Music *music = (FLAC_Music *)context;
    FLAC__uint64 seek_sample = (FLAC__uint64) (music->sample_rate * position);

    SDL_AudioStreamClear(music->stream);

    music->pcm_pos = (FLAC__int64) seek_sample;
    if (!flac.FLAC__stream_decoder_seek_absolute(music->flac_decoder, seek_sample)) {
        if (flac.FLAC__stream_decoder_get_state(music->flac_decoder) == FLAC__STREAM_DECODER_SEEK_ERROR) {
            flac.FLAC__stream_decoder_flush(music->flac_decoder);
        }

        SDL_SetError("Seeking of FLAC stream failed: libFLAC seek failed.");
        return -1;
    }
    return 0;
}

static double FLAC_Tell(void *context)
{
    FLAC_Music *music = (FLAC_Music *)context;
    return (double)music->pcm_pos / music->sample_rate;
}

/* Return music duration in seconds */
static double FLAC_Duration(void *context)
{
    FLAC_Music *music = (FLAC_Music *)context;
    return (double)music->full_length / music->sample_rate;
}

static double FLAC_LoopStart(void *music_p)
{
    FLAC_Music *music = (FLAC_Music *)music_p;
    if (music->loop > 0) {
        return (double)music->loop_start / music->sample_rate;
    }
    return -1.0;
}

static double FLAC_LoopEnd(void *music_p)
{
    FLAC_Music *music = (FLAC_Music *)music_p;
    if (music->loop > 0) {
        return (double)music->loop_end / music->sample_rate;
    }
    return -1.0;
}

static double FLAC_LoopLength(void *music_p)
{
    FLAC_Music *music = (FLAC_Music *)music_p;
    if (music->loop > 0) {
        return (double)music->loop_len / music->sample_rate;
    }
    return -1.0;
}

/* Close the given FLAC_Music object */
static void FLAC_Delete(void *context)
{
    FLAC_Music *music = (FLAC_Music *)context;
    if (music) {
        meta_tags_clear(&music->tags);
        if (music->flac_decoder) {
            flac.FLAC__stream_decoder_finish(music->flac_decoder);
            flac.FLAC__stream_decoder_delete(music->flac_decoder);
        }
        if (music->stream) {
            SDL_FreeAudioStream(music->stream);
        }
        if (music->freesrc) {
            SDL_RWclose(music->src);
        }
        SDL_free(music);
    }
}

Mix_MusicInterface Mix_MusicInterface_FLAC =
{
    "FLAC",
    MIX_MUSIC_FLAC,
    MUS_FLAC,
    SDL_FALSE,
    SDL_FALSE,

    FLAC_Load,
    NULL,   /* Open */
    FLAC_CreateFromRW,
    NULL,   /* CreateFromFile */
    FLAC_SetVolume,
    FLAC_GetVolume,
    FLAC_Play,
    NULL,   /* IsPlaying */
    FLAC_GetAudio,
    NULL,   /* Jump */
    FLAC_Seek,
    FLAC_Tell,
    FLAC_Duration,
    FLAC_LoopStart,
    FLAC_LoopEnd,
    FLAC_LoopLength,
    FLAC_GetMetaTag,/* GetMetaTag */
    NULL,   /* Pause */
    NULL,   /* Resume */
    FLAC_Stop,   /* Stop */
    FLAC_Delete,
    NULL,   /* Close */
    FLAC_Unload
};

#endif /* MUSIC_FLAC_LIBFLAC */

/* vi: set ts=4 sw=4 expandtab: */
