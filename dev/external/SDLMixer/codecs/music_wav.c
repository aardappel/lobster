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

#ifdef MUSIC_WAV

/* This file supports streaming WAV files */

#include "music_wav.h"
#include "mp3utils.h"

typedef struct ADPCM_DecoderState
{
    Uint32 channels;        /* Number of channels. */
    size_t blocksize;       /* Size of an ADPCM block in bytes. */
    size_t blockheadersize; /* Size of an ADPCM block header in bytes. */
    size_t samplesperblock; /* Number of samples per channel in an ADPCM block. */
    void *ddata;            /* Decoder data from initialization. */
    void *cstate;           /* Decoding state for each channel. */

    /* Current ADPCM block */
    struct
    {
        Uint8 *data;
        size_t size;
        size_t pos;
    } block;

    /* Decoded 16-bit PCM data. */
    struct
    {
        Sint16 *data;
        size_t size;
        size_t pos;
        size_t read;
    } output;

} ADPCM_DecoderState;

typedef struct MS_ADPCM_CoeffData
{
    Uint16 coeffcount;
    Sint16 *coeff;
    Sint16 aligndummy; /* Has to be last member. */
} MS_ADPCM_CoeffData;

typedef struct MS_ADPCM_ChannelState
{
    Uint16 delta;
    Sint16 coeff1;
    Sint16 coeff2;
} MS_ADPCM_ChannelState;

typedef struct {
    SDL_bool active;
    Uint32 start;
    Uint32 stop;
    Uint32 initial_play_count;
    Uint32 current_play_count;
} WAVLoopPoint;

typedef struct {
    SDL_RWops *src;
    int freesrc;
    SDL_AudioSpec spec;
    int volume;
    int play_count;
    Sint64 start;
    Sint64 stop;
    Sint64 samplesize;
    Uint8 *buffer;
    size_t buflen;
    size_t buffered;
    SDL_AudioStream *stream;
    ADPCM_DecoderState adpcm_state;
    unsigned int numloops;
    WAVLoopPoint *loops;
    Mix_MusicMetaTags tags;
    Uint16 encoding;
    int (*decode)(void *music, int length);
} WAV_Music;

/*
    Taken with permission from SDL_wave.h, part of the SDL library,
    available at: http://www.libsdl.org/
    and placed under the same license as this mixer library.
*/

/* WAVE files are little-endian */

/*******************************************/
/* Define values for Microsoft WAVE format */
/*******************************************/
#define RIFF        0x46464952      /* "RIFF" */
#define WAVE        0x45564157      /* "WAVE" */
#define FMT         0x20746D66      /* "fmt " */
#define DATA        0x61746164      /* "data" */
#define SMPL        0x6c706d73      /* "smpl" */
#define LIST        0x5453494c      /* "LIST" */
#define ID3_        0x20336469      /* "id3 " */
#define UNKNOWN_CODE    0x0000
#define PCM_CODE        0x0001      /* WAVE_FORMAT_PCM */
#define MS_ADPCM_CODE   0x0002      /* WAVE_FORMAT_ADPCM */
#define IEEE_FLOAT_CODE 0x0003      /* WAVE_FORMAT_IEEE_FLOAT */
#define ALAW_CODE       0x0006      /* WAVE_FORMAT_ALAW */
#define MULAW_CODE      0x0007      /* WAVE_FORMAT_MULAW */
#define IMA_ADPCM_CODE  0x0011
#define MPEG_CODE       0x0050
#define MPEGLAYER3_CODE 0x0055
#define EXTENSIBLE_CODE 0xFFFE
#define WAVE_MONO   1
#define WAVE_STEREO 2

#pragma pack(push, 1)
typedef struct {
/* Not saved in the chunk we read:
    Uint32  chunkID;
    Uint32  chunkLen;
*/
    Uint16  encoding;
    Uint16  channels;       /* 1 = mono, 2 = stereo */
    Uint32  frequency;      /* One of 11025, 22050, or 44100 Hz */
    Uint32  byterate;       /* Average bytes per second */
    Uint16  blockalign;     /* Bytes per sample block */
    Uint16  bitspersample;      /* One of 8, 12, 16, or 4 for ADPCM */
} WaveFMT;

typedef struct {
    WaveFMT format;
    Uint16  cbSize;
    union {
        Uint16 validbitspersample; /* bits of precision */
        Uint16 samplesperblock;   /* valid if wBitsPerSample==0 */
        Uint16 reserved;         /* If neither applies, set to zero. */
    } Samples;
    Uint32 channelsmask;
    /* GUID subFormat 16 bytes */
    Uint32 subencoding;
    Uint16 sub_data2;
    Uint16 sub_data3;
    Uint8  sub_data[8];
} WaveFMTEx;

typedef struct {
    Uint32 identifier;
    Uint32 type;
    Uint32 start;
    Uint32 end;
    Uint32 fraction;
    Uint32 play_count;
} SampleLoop;

typedef struct {
/* Not saved in the chunk we read:
    Uint32  chunkID;
    Uint32  chunkLen;
*/
    Uint32  manufacturer;
    Uint32  product;
    Uint32  sample_period;
    Uint32  MIDI_unity_note;
    Uint32  MIDI_pitch_fraction;
    Uint32  SMTPE_format;
    Uint32  SMTPE_offset;
    Uint32  sample_loops;
    Uint32  sampler_data;
    SampleLoop loops[1];
} SamplerChunk;
#pragma pack(pop)

/*********************************************/
/* Define values for AIFF (IFF audio) format */
/*********************************************/
#define FORM        0x4d524f46      /* "FORM" */
#define AIFF        0x46464941      /* "AIFF" */
#define AIFC        0x43464941      /* "AIFС" */
#define FVER        0x52455646      /* "FVER" */
#define SSND        0x444e5353      /* "SSND" */
#define COMM        0x4d4d4f43      /* "COMM" */
#define AIFF_ID3_   0x20334449      /* "ID3 " */
#define MARK        0x4B52414D      /* "MARK" */
#define INST        0x54534E49      /* "INST" */
#define AUTH        0x48545541      /* "AUTH" */
#define NAME        0x454D414E      /* "NAME" */
#define _c__        0x20296328      /* "(c) " */

#define _8SVX       0x58565338      /* "8SVX" */
#define VHDR        0x52444856      /* "VHDR" */
#define BODY        0x59444F42      /* "BODY" */

/* Supported compression types */
#define NONE        0x454E4F4E      /* "NONE" */
#define sowt        0x74776F73      /* "sowt" */
#define raw_        0x20776172      /* "raw " */
#define ulaw        0x77616C75      /* "ulaw" */
#define alaw        0x77616C61      /* "alaw" */
#define ULAW        0x57414C55      /* "ULAW" */
#define ALAW        0x57414C41      /* "ALAW" */
#define fl32        0x32336C66      /* "fl32" */
#define fl64        0x34366C66      /* "fl64" */
#define FL32        0x32334C46      /* "FL32" */

/* Function to load the WAV/AIFF stream */
static SDL_bool LoadWAVMusic(WAV_Music *wave);
static SDL_bool LoadAIFFMusic(WAV_Music *wave);

static void WAV_Delete(void *context);

static int fetch_pcm(void *context, int length);

/* Load a WAV stream from the given RWops object */
static void *WAV_CreateFromRW(SDL_RWops *src, int freesrc)
{
    WAV_Music *music;
    Uint32 magic;
    SDL_bool loaded = SDL_FALSE;

    music = (WAV_Music *)SDL_calloc(1, sizeof(*music));
    if (!music) {
        Mix_OutOfMemory();
        return NULL;
    }
    music->src = src;
    music->volume = MIX_MAX_VOLUME;
    /* Default decoder is PCM */
    music->decode = fetch_pcm;
    music->encoding = PCM_CODE;

    magic = SDL_ReadLE32(src);
    if (magic == RIFF || magic == WAVE) {
        loaded = LoadWAVMusic(music);
    } else if (magic == FORM) {
        loaded = LoadAIFFMusic(music);
    } else {
        Mix_SetError("Unknown WAVE format");
    }
    if (!loaded) {
        WAV_Delete(music);
        return NULL;
    }

    music->buflen = SDL_AUDIO_BITSIZE(music->spec.format) / 8;
    music->buflen *= music->spec.channels;
    music->buflen *= 4096;       /* Good default sample frame count */

    music->buffer = (Uint8*)SDL_malloc(music->buflen);
    if (!music->buffer) {
        Mix_OutOfMemory();
        WAV_Delete(music);
        return NULL;
    }
    music->stream = SDL_NewAudioStream(
        music->spec.format, music->spec.channels, music->spec.freq,
        music_spec.format, music_spec.channels, music_spec.freq);
    if (!music->stream) {
        WAV_Delete(music);
        return NULL;
    }

    music->freesrc = freesrc;
    return music;
}

static void WAV_SetVolume(void *context, int volume)
{
    WAV_Music *music = (WAV_Music *)context;
    music->volume = volume;
}

static int WAV_GetVolume(void *context)
{
    WAV_Music *music = (WAV_Music *)context;
    return music->volume;
}

/* Start playback of a given WAV stream */
static int WAV_Play(void *context, int play_count)
{
    WAV_Music *music = (WAV_Music *)context;
    unsigned int i;
    for (i = 0; i < music->numloops; ++i) {
        WAVLoopPoint *loop = &music->loops[i];
        loop->active = SDL_TRUE;
        loop->current_play_count = loop->initial_play_count;
    }
    music->play_count = play_count;
    if (SDL_RWseek(music->src, music->start, RW_SEEK_SET) < 0) {
        return -1;
    }
    return 0;
}

static void WAV_Stop(void *context)
{
    WAV_Music *music = (WAV_Music *)context;
    SDL_AudioStreamClear(music->stream);
}

static int fetch_pcm(void *context, int length)
{
    WAV_Music *music = (WAV_Music *)context;
    return (int)SDL_RWread(music->src, music->buffer, 1, (size_t)length);
}

static Uint32 PCM_S24_to_S32_BE(Uint8 *x) {
    const Uint32 bits = 24;
    Uint32 in = (((Uint32)x[0] << 0)  & 0x0000FF) |
                (((Uint32)x[1] << 8)  & 0x00FF00) |
                (((Uint32)x[2] << 16) & 0xFF0000);
    Uint32 m = 1u << (bits - 1);
    return (in ^ m) - m;
}

static Uint32 PCM_S24_to_S32_LE(Uint8 *x) {
    const Uint32 bits = 24;
    Uint32 in = (((Uint32)x[2] << 0)  & 0x0000FF) |
                (((Uint32)x[1] << 8)  & 0x00FF00) |
                (((Uint32)x[0] << 16) & 0xFF0000);
    Uint32 m = 1u << (bits - 1);
    return (in ^ m) - m;
}

static int fetch_pcm24be(void *context, int length)
{
    WAV_Music *music = (WAV_Music *)context;
    int i = 0, o = 0;
    length = (int)SDL_RWread(music->src, music->buffer, 1, (size_t)((length / 4) * 3));
    if (length % music->samplesize != 0) {
        length -= length % music->samplesize;
    }
    for (i = length - 3, o = ((length - 3) / 3) * 4; i >= 0; i -= 3, o -= 4) {
        Uint32 decoded = PCM_S24_to_S32_BE(music->buffer + i);
        music->buffer[o + 0] = (decoded >> 0) & 0xFF;
        music->buffer[o + 1] = (decoded >> 8) & 0xFF;
        music->buffer[o + 2] = (decoded >> 16) & 0xFF;
        music->buffer[o + 3] = (decoded >> 24) & 0xFF;
    }
    return (length / 3) * 4;
}

static int fetch_pcm24le(void *context, int length)
{
    WAV_Music *music = (WAV_Music *)context;
    int i = 0, o = 0;
    length = (int)SDL_RWread(music->src, music->buffer, 1, (size_t)((length / 4) * 3));
    if (length % music->samplesize != 0) {
        length -= length % music->samplesize;
    }
    for (i = length - 3, o = ((length - 3) / 3) * 4; i >= 0; i -= 3, o -= 4) {
        Uint32 decoded = PCM_S24_to_S32_LE(music->buffer + i);
        music->buffer[o + 3] = (decoded >> 0) & 0xFF;
        music->buffer[o + 2] = (decoded >> 8) & 0xFF;
        music->buffer[o + 1] = (decoded >> 16) & 0xFF;
        music->buffer[o + 0] = (decoded >> 24) & 0xFF;
    }
    return (length / 3) * 4;
}

SDL_FORCE_INLINE double
Mix_SwapDouble(double x)
{
    union
    {
        double f;
        Uint64 ui64;
    } swapper;
    swapper.f = x;
    swapper.ui64 = SDL_Swap64(swapper.ui64);
    return swapper.f;
}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define Mix_SwapDoubleLE(X)  (X)
#define Mix_SwapDoubleBE(X)  Mix_SwapDouble(X)
#else
#define Mix_SwapDoubleLE(X)  Mix_SwapDouble(X)
#define Mix_SwapDoubleBE(X)  (X)
#endif

static int fetch_float64be(void *context, int length)
{
    WAV_Music *music = (WAV_Music *)context;
    int i = 0, o = 0;
    length = (int)SDL_RWread(music->src, music->buffer, 1, (size_t)(length));
    if (length % music->samplesize != 0) {
        length -= length % music->samplesize;
    }
    for (i = 0, o = 0; i < length; i += 8, o += 4) {
        union
        {
            float f;
            Uint32 ui32;
        } sample;
        sample.f = (float)Mix_SwapDoubleBE(*(double*)(music->buffer + i));
        music->buffer[o + 0] = (sample.ui32 >> 0) & 0xFF;
        music->buffer[o + 1] = (sample.ui32 >> 8) & 0xFF;
        music->buffer[o + 2] = (sample.ui32 >> 16) & 0xFF;
        music->buffer[o + 3] = (sample.ui32 >> 24) & 0xFF;
    }
    return length / 2;
}

static int fetch_float64le(void *context, int length)
{
    WAV_Music *music = (WAV_Music *)context;
    int i = 0, o = 0;
    length = (int)SDL_RWread(music->src, music->buffer, 1, (size_t)(length));
    if (length % music->samplesize != 0) {
        length -= length % music->samplesize;
    }
    for (i = 0, o = 0; i < length; i += 8, o += 4) {
        union
        {
            float f;
            Uint32 ui32;
        } sample;
        sample.f = (float)Mix_SwapDoubleLE(*(double*)(music->buffer + i));
        music->buffer[o + 0] = (sample.ui32 >> 0) & 0xFF;
        music->buffer[o + 1] = (sample.ui32 >> 8) & 0xFF;
        music->buffer[o + 2] = (sample.ui32 >> 16) & 0xFF;
        music->buffer[o + 3] = (sample.ui32 >> 24) & 0xFF;
    }
    return length / 2;
}

static int MS_ADPCM_Init(ADPCM_DecoderState *state, const Uint8 *chunk_data, Uint32 chunk_length)
{
    const WaveFMTEx *fmt = (WaveFMTEx *)chunk_data;
    const Uint16 channels = SDL_SwapLE16(fmt->format.channels);
    const Uint16 blockalign = SDL_SwapLE16(fmt->format.blockalign);
    const Uint16 bitspersample = SDL_SwapLE16(fmt->format.bitspersample);
    const size_t blockheadersize = (size_t)channels * 7;
    const size_t blockdatasize = (size_t)blockalign - blockheadersize;
    const size_t blockframebitsize = (size_t)bitspersample * channels;
    const size_t blockdatasamples = (blockdatasize * 8) / blockframebitsize;
    const Sint16 presetcoeffs[14] = { 256, 0, 512, -256, 0, 0, 192, 64, 240, 0, 460, -208, 392, -232 };
    Uint16 cbExtSize, samplesperblock;
    size_t i, coeffcount;
    MS_ADPCM_CoeffData *coeffdata;

    /* Sanity checks. */

    /* While it's clear how IMA ADPCM handles more than two channels, the nibble
     * order of MS ADPCM makes it awkward. The Standards Update does not talk
     * about supporting more than stereo anyway.
     */
    if (channels > 2) {
        return Mix_SetError("Invalid number of channels");
    }

    if (bitspersample != 4) {
        return Mix_SetError("Invalid MS ADPCM bits per sample of %u", (unsigned int)bitspersample);
    }

    /* The block size must be big enough to contain the block header. */
    if (blockalign < blockheadersize) {
        return Mix_SetError("Invalid MS ADPCM block size (nBlockAlign)");
    }

    /* There are wSamplesPerBlock, wNumCoef, and at least 7 coefficient pairs in
     * the extended part of the header.
     */
    if (chunk_length < 22) {
        return Mix_SetError("Could not read MS ADPCM format header");
    }

    cbExtSize = SDL_SwapLE16(fmt->cbSize);
    samplesperblock = SDL_SwapLE16(fmt->Samples.samplesperblock);
    /* Number of coefficient pairs. A pair has two 16-bit integers. */
    coeffcount = chunk_data[20] | ((size_t)chunk_data[21] << 8);
    /* bPredictor, the integer offset into the coefficients array, is only
     * 8 bits. It can only address the first 256 coefficients. Let's limit
     * the count number here.
     */
    if (coeffcount > 256) {
        coeffcount = 256;
    }

    if (chunk_length < 22 + coeffcount * 4) {
        return Mix_SetError("Could not read custom coefficients in MS ADPCM format header");
    } else if (cbExtSize < 4 + coeffcount * 4) {
        return Mix_SetError("Invalid MS ADPCM format header (too small)");
    } else if (coeffcount < 7) {
        return Mix_SetError("Missing required coefficients in MS ADPCM format header");
    }

    coeffdata = (MS_ADPCM_CoeffData *)SDL_malloc(sizeof(MS_ADPCM_CoeffData) + coeffcount * 4);
    if (coeffdata == NULL) {
        return Mix_OutOfMemory();
    }
    coeffdata->coeff = &coeffdata->aligndummy;
    coeffdata->coeffcount = (Uint16)coeffcount;
    state->ddata = coeffdata; /* Freed in cleanup. */

    /* Copy the 16-bit pairs. */
    for (i = 0; i < coeffcount * 2; i++) {
        Sint32 c = chunk_data[22 + i * 2] | ((Sint32)chunk_data[23 + i * 2] << 8);
        if (c >= 0x8000) {
            c -= 0x10000;
        }
        if (i < 14 && c != presetcoeffs[i]) {
            return Mix_SetError("Wrong preset coefficients in MS ADPCM format header");
        }
        coeffdata->coeff[i] = (Sint16)c;
    }

    /* Technically, wSamplesPerBlock is required, but we have all the
     * information in the other fields to calculate it, if it's zero.
     */
    if (samplesperblock == 0) {
        /* Let's be nice to the encoders that didn't know how to fill this.
         * The Standards Update calculates it this way:
         *
         *   x = Block size (in bits) minus header size (in bits)
         *   y = Bit depth multiplied by channel count
         *   z = Number of samples per channel in block header
         *   wSamplesPerBlock = x / y + z
         */
        samplesperblock = (Uint16)(blockdatasamples + 2);
    }

    /* nBlockAlign can be in conflict with wSamplesPerBlock. For example, if
     * the number of samples doesn't fit into the block. The Standards Update
     * also describes wSamplesPerBlock with a formula that makes it necessary to
     * always fill the block with the maximum amount of samples, but this is not
     * enforced here as there are no compatibility issues.
     * A truncated block header with just one sample is not supported.
     */
    if (samplesperblock == 1 || blockdatasamples < (size_t)(samplesperblock - 2)) {
        return Mix_SetError("Invalid number of samples per MS ADPCM block (wSamplesPerBlock)");
    }

    state->blocksize = blockalign;
    state->channels = channels;
    state->blockheadersize = blockheadersize;
    state->samplesperblock = samplesperblock;

    state->cstate = SDL_calloc(channels, sizeof(MS_ADPCM_ChannelState));
    if (!state->cstate) {
        return Mix_OutOfMemory();
    }

    state->block.pos = 0;
    state->block.size = blockalign;
    state->block.data = (Uint8 *)SDL_malloc(state->block.size);
    if (!state->block.data) {
        return Mix_OutOfMemory();
    }

    state->output.read = 0;
    state->output.pos = 0;
    state->output.size = state->samplesperblock * state->channels;
    state->output.data = (Sint16 *)SDL_malloc(state->output.size * sizeof(Sint16));
    if (!state->output.data) {
        return Mix_OutOfMemory();
    }

    return 0;
}

static Sint16 MS_ADPCM_ProcessNibble(MS_ADPCM_ChannelState *cstate, Sint32 sample1, Sint32 sample2, Uint8 nybble)
{
    const Sint32 max_audioval = 32767;
    const Sint32 min_audioval = -32768;
    const Uint16 max_deltaval = 65535;
    const Uint16 adaptive[] = {
        230, 230, 230, 230, 307, 409, 512, 614,
        768, 614, 512, 409, 307, 230, 230, 230
    };
    Sint32 new_sample;
    Sint32 errordelta;
    Uint32 delta = cstate->delta;

    new_sample = (sample1 * cstate->coeff1 + sample2 * cstate->coeff2) / 256;
    /* The nibble is a signed 4-bit error delta. */
    errordelta = (Sint32)nybble - (nybble >= 0x08 ? 0x10 : 0);
    new_sample += (Sint32)delta * errordelta;
    if (new_sample < min_audioval) {
        new_sample = min_audioval;
    } else if (new_sample > max_audioval) {
        new_sample = max_audioval;
    }
    delta = (delta * adaptive[nybble]) / 256;
    if (delta < 16) {
        delta = 16;
    } else if (delta > max_deltaval) {
        /* This issue is not described in the Standards Update and therefore
         * undefined. It seems sensible to prevent overflows with a limit.
         */
        delta = max_deltaval;
    }

    cstate->delta = (Uint16)delta;
    return (Sint16)new_sample;
}

static int MS_ADPCM_DecodeBlockHeader(ADPCM_DecoderState *state)
{
    Uint8 coeffindex;
    const Uint32 channels = state->channels;
    Sint32 sample;
    Uint32 c;
    MS_ADPCM_ChannelState *cstate = (MS_ADPCM_ChannelState *)state->cstate;
    MS_ADPCM_CoeffData *ddata = (MS_ADPCM_CoeffData *)state->ddata;

    if (state->block.size < state->blockheadersize) {
        return Mix_SetError("Invalid ADPCM header");
    }

    for (c = 0; c < channels; c++) {
        size_t o = c;

        /* Load the coefficient pair into the channel state. */
        coeffindex = state->block.data[o];
        if (coeffindex > ddata->coeffcount) {
            return Mix_SetError("Invalid MS ADPCM coefficient index in block header");
        }
        cstate[c].coeff1 = ddata->coeff[coeffindex * 2];
        cstate[c].coeff2 = ddata->coeff[coeffindex * 2 + 1];

        /* Initial delta value. */
        o = (size_t)channels + c * 2;
        cstate[c].delta = state->block.data[o] | ((Uint16)state->block.data[o + 1] << 8);

        /* Load the samples from the header. Interestingly, the sample later in
         * the output stream comes first.
         */
        o = (size_t)channels * 3 + c * 2;
        sample = state->block.data[o] | ((Sint32)state->block.data[o + 1] << 8);
        if (sample >= 0x8000) {
            sample -= 0x10000;
        }
        state->output.data[state->output.pos + channels] = (Sint16)sample;

        o = (size_t)channels * 5 + c * 2;
        sample = state->block.data[o] | ((Sint32)state->block.data[o + 1] << 8);
        if (sample >= 0x8000) {
            sample -= 0x10000;
        }
        state->output.data[state->output.pos] = (Sint16)sample;

        state->output.pos++;
    }

    state->block.pos += state->blockheadersize;

    /* Skip second sample frame that came from the header. */
    state->output.pos += state->channels;

    return 0;
}

/* Decodes the data of the MS ADPCM block. Decoding will stop if a block is too
 * short, returning with none or partially decoded data. The partial data
 * will always contain full sample frames (same sample count for each channel).
 * Incomplete sample frames are discarded.
 */
static int MS_ADPCM_DecodeBlockData(ADPCM_DecoderState *state)
{
    Uint16 nybble = 0;
    Sint16 sample1, sample2;
    const Uint32 channels = state->channels;
    Uint32 c;
    MS_ADPCM_ChannelState *cstate = (MS_ADPCM_ChannelState *)state->cstate;

    size_t blockpos = state->block.pos;
    size_t blocksize = state->block.size;

    size_t outpos = state->output.pos;

    size_t blockframesleft = state->samplesperblock - 2;

    while (blockframesleft > 0) {
        for (c = 0; c < channels; c++) {
            if (nybble & 0x4000) {
                nybble <<= 4;
            } else if (blockpos < blocksize) {
                nybble = state->block.data[blockpos++] | 0x4000;
            } else {
                /* Out of input data. Drop the incomplete frame and return. */
                state->output.pos = outpos - c;
                return -1;
            }

            /* Load previous samples which may come from the block header. */
            sample1 = state->output.data[outpos - channels];
            sample2 = state->output.data[outpos - channels * 2];

            sample1 = MS_ADPCM_ProcessNibble(cstate + c, sample1, sample2, (nybble >> 4) & 0x0f);
            state->output.data[outpos++] = sample1;
        }

        blockframesleft--;
    }

    state->output.pos = outpos;

    return 0;
}

static int IMA_ADPCM_Init(ADPCM_DecoderState *state, const Uint8 *chunk_data, Uint32 chunk_length)
{
    const WaveFMTEx *fmt = (WaveFMTEx *)chunk_data;
    const Uint16 formattag = SDL_SwapLE16(fmt->format.encoding);
    const Uint16 channels = SDL_SwapLE16(fmt->format.channels);
    const Uint16 blockalign = SDL_SwapLE16(fmt->format.blockalign);
    const Uint16 bitspersample = SDL_SwapLE16(fmt->format.bitspersample);
    const size_t blockheadersize = (size_t)channels * 4;
    const size_t blockdatasize = (size_t)blockalign - blockheadersize;
    const size_t blockframebitsize = (size_t)bitspersample * channels;
    const size_t blockdatasamples = (blockdatasize * 8) / blockframebitsize;
    Uint16 samplesperblock = 0;

    /* Sanity checks. */

    /* IMA ADPCM can also have 3-bit samples, but it's not supported by SDL at this time. */
    if (bitspersample == 3) {
        return Mix_SetError("3-bit IMA ADPCM currently not supported");
    } else if (bitspersample != 4) {
        return Mix_SetError("Invalid IMA ADPCM bits per sample of %u", (unsigned int)bitspersample);
    }

    /* The block size is required to be a multiple of 4 and it must be able to
     * hold a block header.
     */
    if (blockalign < blockheadersize || blockalign % 4) {
        return Mix_SetError("Invalid IMA ADPCM block size (nBlockAlign)");
    }

    if (formattag == EXTENSIBLE_CODE) {
        /* There's no specification for this, but it's basically the same
         * format because the extensible header has wSampePerBlocks too.
         */
    } else if (chunk_length >= 20) {
        Uint16 cbExtSize = SDL_SwapLE16(fmt->cbSize);
        if (cbExtSize >= 2) {
            samplesperblock = SDL_SwapLE16(fmt->Samples.samplesperblock);
        }
    }

    if (samplesperblock == 0) {
        /* Field zero? No problem. We just assume the encoder packed the block.
         * The specification calculates it this way:
         *
         *   x = Block size (in bits) minus header size (in bits)
         *   y = Bit depth multiplied by channel count
         *   z = Number of samples per channel in header
         *   wSamplesPerBlock = x / y + z
         */
        samplesperblock = (Uint16)(blockdatasamples + 1);
    }

    /* nBlockAlign can be in conflict with wSamplesPerBlock. For example, if
     * the number of samples doesn't fit into the block. The Standards Update
     * also describes wSamplesPerBlock with a formula that makes it necessary
     * to always fill the block with the maximum amount of samples, but this is
     * not enforced here as there are no compatibility issues.
     */
    if (blockdatasamples < (size_t)(samplesperblock - 1)) {
        return Mix_SetError("Invalid number of samples per IMA ADPCM block (wSamplesPerBlock)");
    }

    state->blocksize = blockalign;
    state->channels = channels;
    state->blockheadersize = blockheadersize;
    state->samplesperblock = samplesperblock;

    state->cstate = SDL_calloc(channels, sizeof(Sint8));
    if (!state->cstate) {
        return Mix_OutOfMemory();
    }

    state->block.pos = 0;
    state->block.size = blockalign;
    state->block.data = (Uint8 *)SDL_malloc(state->block.size);
    if (!state->block.data) {
        return Mix_OutOfMemory();
    }

    state->output.read = 0;
    state->output.pos = 0;
    state->output.size = state->samplesperblock * state->channels;
    state->output.data = (Sint16 *)SDL_malloc(state->output.size * sizeof(Sint16));
    if (!state->output.data) {
        return Mix_OutOfMemory();
    }

    return 0;
}

static Sint16 IMA_ADPCM_ProcessNibble(Sint8 *cindex, Sint16 lastsample, Uint8 nybble)
{
    const Sint32 max_audioval = 32767;
    const Sint32 min_audioval = -32768;
    const Sint8 index_table_4b[16] = {
        -1, -1, -1, -1,
        2, 4, 6, 8,
        -1, -1, -1, -1,
        2, 4, 6, 8
    };
    const Uint16 step_table[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
        34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
        143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
        449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
        1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
        3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
        9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
        22385, 24623, 27086, 29794, 32767
    };
    Uint32 step;
    Sint32 sample, delta;
    Sint8 index = *cindex;

    /* Clamp index into valid range. */
    if (index > 88) {
        index = 88;
    } else if (index < 0) {
        index = 0;
    }

    /* explicit cast to avoid gcc warning about using 'char' as array index */
    step = step_table[(size_t)index];

    /* Update index value */
    *cindex = index + index_table_4b[nybble];

    /* This calculation uses shifts and additions because multiplications were
     * much slower back then. Sadly, this can't just be replaced with an actual
     * multiplication now as the old algorithm drops some bits. The closest
     * approximation I could find is something like this:
     * (nybble & 0x8 ? -1 : 1) * ((nybble & 0x7) * step / 4 + step / 8)
     */
    delta = step >> 3;
    if (nybble & 0x04) {
        delta += step;
    }
    if (nybble & 0x02) {
        delta += step >> 1;
    }
    if (nybble & 0x01) {
        delta += step >> 2;
    }
    if (nybble & 0x08) {
        delta = -delta;
    }

    sample = lastsample + delta;

    /* Clamp output sample */
    if (sample > max_audioval) {
        sample = max_audioval;
    } else if (sample < min_audioval) {
        sample = min_audioval;
    }

    return (Sint16)sample;
}

static int IMA_ADPCM_DecodeBlockHeader(ADPCM_DecoderState *state)
{
    Sint16 step;
    Uint32 c;
    Uint8 *cstate = (Uint8 *)state->cstate;

    if (state->block.size < state->blockheadersize) {
        return Mix_SetError("Invalid ADPCM header");
    }

    for (c = 0; c < state->channels; c++) {
        size_t o = state->block.pos + c * 4;

        /* Extract the sample from the header. */
        Sint32 sample = state->block.data[o] | ((Sint32)state->block.data[o + 1] << 8);
        if (sample >= 0x8000) {
            sample -= 0x10000;
        }
        state->output.data[state->output.pos++] = (Sint16)sample;

        /* Channel step index. */
        step = (Sint16)state->block.data[o + 2];
        cstate[c] = (Sint8)(step > 0x80 ? step - 0x100 : step);

        /* Reserved byte in block header, should be 0. */
        if (state->block.data[o + 3] != 0) {
            /* Uh oh, corrupt data?  Buggy code? */;
        }
    }

    state->block.pos += state->blockheadersize;

    return 0;
}

/* Decodes the data of the IMA ADPCM block. Decoding will stop if a block is too
 * short, returning with none or partially decoded data. The partial data always
 * contains full sample frames (same sample count for each channel).
 * Incomplete sample frames are discarded.
 */
static int IMA_ADPCM_DecodeBlockData(ADPCM_DecoderState *state)
{
    size_t i;
    int retval = 0;
    const Uint32 channels = state->channels;
    const size_t subblockframesize = (size_t)channels * 4;
    Uint64 bytesrequired;
    Uint32 c;

    size_t blockpos = state->block.pos;
    size_t blocksize = state->block.size;
    size_t blockleft = blocksize - blockpos;

    size_t outpos = state->output.pos;

    Sint64 blockframesleft = state->samplesperblock - 1;

    bytesrequired = (blockframesleft + 7) / 8 * subblockframesize;
    if (blockleft < bytesrequired) {
        /* Data truncated. Calculate how many samples we can get out if it. */
        const size_t guaranteedframes = blockleft / subblockframesize;
        const size_t remainingbytes = blockleft % subblockframesize;
        blockframesleft = guaranteedframes;
        if (remainingbytes > subblockframesize - 4) {
            blockframesleft += (remainingbytes % 4) * 2;
        }
        /* Signal the truncation. */
        retval = -1;
    }

    /* Each channel has their nibbles packed into 32-bit blocks. These blocks
     * are interleaved and make up the data part of the ADPCM block. This loop
     * decodes the samples as they come from the input data and puts them at
     * the appropriate places in the output data.
     */
    while (blockframesleft > 0) {
        const size_t subblocksamples = blockframesleft < 8 ? (size_t)blockframesleft : 8;

        for (c = 0; c < channels; c++) {
            Uint8 nybble = 0;
            /* Load previous sample which may come from the block header. */
            Sint16 sample = state->output.data[outpos + c - channels];

            for (i = 0; i < subblocksamples; i++) {
                if (i & 1) {
                    nybble >>= 4;
                } else {
                    nybble = state->block.data[blockpos++];
                }

                sample = IMA_ADPCM_ProcessNibble((Sint8 *)state->cstate + c, sample, nybble & 0x0f);
                state->output.data[outpos + c + i * channels] = sample;
            }
        }

        outpos += channels * subblocksamples;
        blockframesleft -= subblocksamples;
    }

    state->block.pos = blockpos;
    state->output.pos = outpos;

    return retval;
}

static void ADPCM_Cleanup(ADPCM_DecoderState *state)
{
    if (state->ddata) {
        SDL_free(state->ddata);
        state->ddata = NULL;
    }
    if (state->cstate) {
        SDL_free(state->cstate);
        state->cstate = NULL;
    }
    if (state->block.data) {
        SDL_free(state->block.data);
        SDL_zero(state->block);
    }
    if (state->output.data) {
        SDL_free(state->output.data);
        SDL_zero(state->output);
    }
}

static int fetch_adpcm(void *context, int length, int (*DecodeBlockHeader)(ADPCM_DecoderState *state), int (*DecodeBlockData)(ADPCM_DecoderState *state))
{
    WAV_Music *music = (WAV_Music *)context;
    ADPCM_DecoderState *state = &music->adpcm_state;
    size_t len, left = (size_t)length;
    Uint8 *dst = music->buffer;

    while (left > 0) {
        if (state->output.read == state->output.pos) {
            size_t bytesread = SDL_RWread(music->src, state->block.data, 1, state->blocksize);
            if (bytesread == 0) {
                break;
            }

            state->block.size = bytesread < state->blocksize ? bytesread : state->blocksize;
            state->block.pos = 0;
            state->output.pos = 0;
            state->output.read = 0;

            if (DecodeBlockHeader(state) < 0) {
                return -1;
            }

            if (DecodeBlockData(state) < 0) {
                return -1;
            }
        }
        len = SDL_min(left, (state->output.pos - state->output.read) * sizeof(Sint16));
        SDL_memcpy(dst, &state->output.data[state->output.read], len);
        state->output.read += (len / sizeof(Sint16));
        dst += len;
        left -= len;
    }
    music->buffered = (state->output.pos - state->output.read) * sizeof(Sint16);

    return length;
}

static int fetch_ms_adpcm(void *context, int length)
{
    return fetch_adpcm(context, length, MS_ADPCM_DecodeBlockHeader, MS_ADPCM_DecodeBlockData);
}

static int fetch_ima_adpcm(void *context, int length)
{
    return fetch_adpcm(context, length, IMA_ADPCM_DecodeBlockHeader, IMA_ADPCM_DecodeBlockData);
}

/*
    G711 decode tables taken from SDL2 (src/audio/SDL_wave.c)
*/
#ifdef SDL_WAVE_LAW_LUT
static const Sint16 alaw_lut[256] = {
    -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736, -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784, -2752,
    -2624, -3008, -2880, -2240, -2112, -2496, -2368, -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392, -22016,
    -20992, -24064, -23040, -17920, -16896, -19968, -18944, -30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136, -11008,
    -10496, -12032, -11520, -8960, -8448, -9984, -9472, -15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568, -344,
    -328, -376, -360, -280, -264, -312, -296, -472, -456, -504, -488, -408, -392, -440, -424, -88,
    -72, -120, -104, -24, -8, -56, -40, -216, -200, -248, -232, -152, -136, -184, -168, -1376,
    -1312, -1504, -1440, -1120, -1056, -1248, -1184, -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696, -688,
    -656, -752, -720, -560, -528, -624, -592, -944, -912, -1008, -976, -816, -784, -880, -848, 5504,
    5248, 6016, 5760, 4480, 4224, 4992, 4736, 7552, 7296, 8064, 7808, 6528, 6272, 7040, 6784, 2752,
    2624, 3008, 2880, 2240, 2112, 2496, 2368, 3776, 3648, 4032, 3904, 3264, 3136, 3520, 3392, 22016,
    20992, 24064, 23040, 17920, 16896, 19968, 18944, 30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136, 11008,
    10496, 12032, 11520, 8960, 8448, 9984, 9472, 15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568, 344,
    328, 376, 360, 280, 264, 312, 296, 472, 456, 504, 488, 408, 392, 440, 424, 88,
    72, 120, 104, 24, 8, 56, 40, 216, 200, 248, 232, 152, 136, 184, 168, 1376,
    1312, 1504, 1440, 1120, 1056, 1248, 1184, 1888, 1824, 2016, 1952, 1632, 1568, 1760, 1696, 688,
    656, 752, 720, 560, 528, 624, 592, 944, 912, 1008, 976, 816, 784, 880, 848
};

static const Sint16 mulaw_lut[256] = {
    -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956, -23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764, -15996,
    -15484, -14972, -14460, -13948, -13436, -12924, -12412, -11900, -11388, -10876, -10364, -9852, -9340, -8828, -8316, -7932,
    -7676, -7420, -7164, -6908, -6652, -6396, -6140, -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092, -3900,
    -3772, -3644, -3516, -3388, -3260, -3132, -3004, -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980, -1884,
    -1820, -1756, -1692, -1628, -1564, -1500, -1436, -1372, -1308, -1244, -1180, -1116, -1052, -988, -924, -876,
    -844, -812, -780, -748, -716, -684, -652, -620, -588, -556, -524, -492, -460, -428, -396, -372,
    -356, -340, -324, -308, -292, -276, -260, -244, -228, -212, -196, -180, -164, -148, -132, -120,
    -112, -104, -96, -88, -80, -72, -64, -56, -48, -40, -32, -24, -16, -8, 0, 32124,
    31100, 30076, 29052, 28028, 27004, 25980, 24956, 23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764, 15996,
    15484, 14972, 14460, 13948, 13436, 12924, 12412, 11900, 11388, 10876, 10364, 9852, 9340, 8828, 8316, 7932,
    7676, 7420, 7164, 6908, 6652, 6396, 6140, 5884, 5628, 5372, 5116, 4860, 4604, 4348, 4092, 3900,
    3772, 3644, 3516, 3388, 3260, 3132, 3004, 2876, 2748, 2620, 2492, 2364, 2236, 2108, 1980, 1884,
    1820, 1756, 1692, 1628, 1564, 1500, 1436, 1372, 1308, 1244, 1180, 1116, 1052, 988, 924, 876,
    844, 812, 780, 748, 716, 684, 652, 620, 588, 556, 524, 492, 460, 428, 396, 372,
    356, 340, 324, 308, 292, 276, 260, 244, 228, 212, 196, 180, 164, 148, 132, 120,
    112, 104, 96, 88, 80, 72, 64, 56, 48, 40, 32, 24, 16, 8, 0
};
#endif

static Sint16 uLAW_To_PCM16(Uint8 u_val)
{
#ifdef SDL_WAVE_LAW_LUT
    return mulaw_lut[u_val];
#else
    Uint8 nibble = ~u_val;
    Sint16 mantissa = nibble & 0xf;
    Uint8 exponent = (nibble >> 4) & 0x7;
    Sint16 step = (Sint16)(4 << (exponent + 1));

    mantissa = (Sint16)(0x80 << exponent) + step * mantissa + step / 2 - 132;

    return nibble & 0x80 ? -mantissa : mantissa;
#endif
}

static Sint16 ALAW_To_PCM16(Uint8 a_val)
{
#ifdef SDL_WAVE_LAW_LUT
    return alaw_lut[a_val];
#else
    Uint8 nibble = a_val;
    Uint8 exponent = (nibble & 0x7f) ^ 0x55;
    Sint16 mantissa = exponent & 0xf;

    exponent >>= 4;
    if (exponent > 0) {
        mantissa |= 0x10;
    }
    mantissa = (Sint16)(mantissa << 4) | 0x8;
    if (exponent > 1) {
        mantissa <<= exponent - 1;
    }

    return nibble & 0x80 ? mantissa : -mantissa;
#endif
}

static int fetch_xlaw(Sint16 (*decode_sample)(Uint8), void *context, int length)
{
    WAV_Music *music = (WAV_Music *)context;
    int i = 0, o = 0;
    length = (int)SDL_RWread(music->src, music->buffer, 1, (size_t)(length / 2));
    if (length % music->samplesize != 0) {
        length -= length % music->samplesize;
    }
    for (i = length - 1, o = (length - 1) * 2; i >= 0; i--, o -= 2) {
        Uint16 decoded = (Uint16)decode_sample(music->buffer[i]);
        music->buffer[o] = decoded & 0xFF;
        music->buffer[o + 1] = (decoded >> 8) & 0xFF;
    }
    return length * 2;
}

static int fetch_ulaw(void *context, int length)
{
    return fetch_xlaw(uLAW_To_PCM16, context, length);
}

static int fetch_alaw(void *context, int length)
{
    return fetch_xlaw(ALAW_To_PCM16, context, length);
}

static Sint64 WAV_Position(WAV_Music *music)
{
    return SDL_RWtell(music->src) - music->buffered;
}

/* Play some of a stream previously started with WAV_Play() */
static int WAV_GetSome(void *context, void *data, int bytes, SDL_bool *done)
{
    WAV_Music *music = (WAV_Music *)context;
    Sint64 pos, stop;
    WAVLoopPoint *loop;
    Sint64 loop_start = music->start;
    Sint64 loop_stop = music->stop;
    SDL_bool looped = SDL_FALSE;
    SDL_bool at_end = SDL_FALSE;
    unsigned int i;
    int filled, amount, result;

    filled = SDL_AudioStreamGet(music->stream, data, bytes);
    if (filled != 0) {
        return filled;
    }

    if (!music->play_count) {
        /* All done */
        *done = SDL_TRUE;
        return 0;
    }

    pos = WAV_Position(music);
    stop = music->stop;
    loop = NULL;
    for (i = 0; i < music->numloops; ++i) {
        loop = &music->loops[i];
        if (loop->active) {
            const int bytes_per_sample = (SDL_AUDIO_BITSIZE(music->spec.format) / 8) * music->spec.channels;
            loop_start = music->start + loop->start * (Uint32)bytes_per_sample;
            loop_stop = music->start + (loop->stop + 1) * (Uint32)bytes_per_sample;
            if (pos >= loop_start && pos < loop_stop) {
                stop = loop_stop;
                break;
            }
        }
        loop = NULL;
    }

    amount = (int)music->buflen;
    if ((stop - pos) < amount) {
        amount = (int)(stop - pos);
    }

    amount = music->decode(music, amount);
    if (amount > 0) {
        result = SDL_AudioStreamPut(music->stream, music->buffer, amount);
        if (result < 0) {
            return -1;
        }
    } else {
        /* We might be looping, continue */
        at_end = SDL_TRUE;
    }

    if (loop && SDL_RWtell(music->src) >= stop) {
        if (loop->current_play_count == 1) {
            loop->active = SDL_FALSE;
        } else {
            if (loop->current_play_count > 0) {
                --loop->current_play_count;
            }
            if (SDL_RWseek(music->src, loop_start, RW_SEEK_SET) < 0)
                return -1;
            looped = SDL_TRUE;
        }
    }

    if (!looped && (at_end || WAV_Position(music) >= music->stop)) {
        if (music->play_count == 1) {
            music->play_count = 0;
            SDL_AudioStreamFlush(music->stream);
        } else {
            int play_count = -1;
            if (music->play_count > 0) {
                play_count = (music->play_count - 1);
            }
            if (WAV_Play(music, play_count) < 0) {
                return -1;
            }
        }
    }

    /* We'll get called again in the case where we looped or have more data */
    return 0;
}

static int WAV_GetAudio(void *context, void *data, int bytes)
{
    WAV_Music *music = (WAV_Music *)context;
    return music_pcm_getaudio(context, data, bytes, music->volume, WAV_GetSome);
}

static int WAV_Seek(void *context, double position)
{
    WAV_Music *music = (WAV_Music *)context;
    Sint64 destpos;
    if (music->encoding == MS_ADPCM_CODE || music->encoding == IMA_ADPCM_CODE) {
        Sint64 dest_offset = (Sint64)(position * music->spec.freq * ((double)music->adpcm_state.blocksize / music->adpcm_state.samplesperblock));
        int remainder = (int)(dest_offset % music->adpcm_state.blocksize);
        dest_offset -= remainder;
        destpos = music->start + dest_offset;
        if (destpos > music->stop) {
            return -1;
        }
        if (SDL_RWseek(music->src, destpos, RW_SEEK_SET) < 0) {
            return -1;
        }

        music->buffered = 0;
        music->adpcm_state.output.read = music->adpcm_state.output.pos;
        if (remainder > 0) {
            music->decode(music, remainder);
        }
    } else {
        Sint64 sample_size = music->spec.freq * music->samplesize;
        Sint64 dest_offset = (Sint64)(position * music->spec.freq * music->samplesize);
        destpos = music->start + dest_offset;
        destpos -= dest_offset % sample_size;
        if (destpos > music->stop) {
            return -1;
        }
        if (SDL_RWseek(music->src, destpos, RW_SEEK_SET) < 0) {
            return -1;
        }
    }
    return 0;
}

static double WAV_Tell(void *context)
{
    WAV_Music *music = (WAV_Music *)context;
    Sint64 byte_pos = WAV_Position(music) - music->start;
    Sint64 sample_pos;
    if (music->encoding == MS_ADPCM_CODE || music->encoding == IMA_ADPCM_CODE) {
        sample_pos = ((byte_pos * music->adpcm_state.samplesperblock) / music->adpcm_state.blocksize);
    } else {
        sample_pos = byte_pos / music->samplesize;
    }
    return (double)sample_pos / music->spec.freq;
}

/* Return music duration in seconds */
static double WAV_Duration(void *context)
{
    WAV_Music *music = (WAV_Music *)context;
    Sint64 samples;
    if (music->encoding == MS_ADPCM_CODE || music->encoding == IMA_ADPCM_CODE) {
        samples = (((music->stop - music->start) * music->adpcm_state.samplesperblock) / music->adpcm_state.blocksize);
    } else {
        samples = (music->stop - music->start) / music->samplesize;
    }
    return (double)samples / music->spec.freq;
}

static const char* WAV_GetMetaTag(void *context, Mix_MusicMetaTag tag_type)
{
    WAV_Music *music = (WAV_Music *)context;
    return meta_tags_get(&music->tags, tag_type);
}

/* Close the given WAV stream */
static void WAV_Delete(void *context)
{
    WAV_Music *music = (WAV_Music *)context;

    /* Clean up associated data */
    meta_tags_clear(&music->tags);
    if (music->loops) {
        SDL_free(music->loops);
    }
    if (music->stream) {
        SDL_FreeAudioStream(music->stream);
    }
    if (music->buffer) {
        SDL_free(music->buffer);
    }
    if (music->freesrc) {
        SDL_RWclose(music->src);
    }
    ADPCM_Cleanup(&music->adpcm_state);
    SDL_free(music);
}

static SDL_bool ParseFMT(WAV_Music *wave, Uint32 chunk_length)
{
    SDL_AudioSpec *spec = &wave->spec;
    WaveFMTEx fmt;
    size_t size;
    int bits;
    Uint8 *chunk;

    if (chunk_length < sizeof(fmt.format)) {
        Mix_SetError("Wave format chunk too small");
        return SDL_FALSE;
    }

    chunk = (Uint8 *)SDL_malloc(chunk_length);
    if (!chunk) {
        Mix_OutOfMemory();
        return SDL_FALSE;
    }

    if (SDL_RWread(wave->src, chunk, 1, chunk_length) != chunk_length) {
        Mix_SetError("Couldn't read %u bytes from WAV file", (unsigned)chunk_length);
        SDL_free(chunk);
        return SDL_FALSE;
    }
    size = (chunk_length >= sizeof(fmt)) ? sizeof(fmt) : sizeof(fmt.format);
    SDL_memset(&fmt, 0, sizeof(fmt));
    SDL_memcpy(&fmt, chunk, size);

    wave->encoding = SDL_SwapLE16(fmt.format.encoding);

    if (wave->encoding == EXTENSIBLE_CODE) {
        if (size < sizeof(fmt)) {
            Mix_SetError("Wave format chunk too small");
            SDL_free(chunk);
            return SDL_FALSE;
        }
        wave->encoding = (Uint16)SDL_SwapLE32(fmt.subencoding);
    }

    /* Decode the audio data format */
    switch (wave->encoding) {
        case PCM_CODE:
        case IEEE_FLOAT_CODE:
            wave->decode = fetch_pcm;
            break;
        case MULAW_CODE:
            wave->decode = fetch_ulaw;
            break;
        case ALAW_CODE:
            wave->decode = fetch_alaw;
            break;
        case MS_ADPCM_CODE:
            wave->decode = fetch_ms_adpcm;
            if (MS_ADPCM_Init(&wave->adpcm_state, chunk, chunk_length) < 0) {
                SDL_free(chunk);
                return SDL_FALSE;
            }
            break;
        case IMA_ADPCM_CODE:
            wave->decode = fetch_ima_adpcm;
            if (IMA_ADPCM_Init(&wave->adpcm_state, chunk, chunk_length) < 0) {
                SDL_free(chunk);
                return SDL_FALSE;
            }
            break;
        default:
            /* but NOT this */
            Mix_SetError("Unknown WAVE data format");
            SDL_free(chunk);
            return SDL_FALSE;
    }
    SDL_free(chunk);

    spec->freq = (int)SDL_SwapLE32(fmt.format.frequency);
    bits = (int)SDL_SwapLE16(fmt.format.bitspersample);
    switch (bits) {
        case 4:
            switch(wave->encoding) {
            case MS_ADPCM_CODE: spec->format = AUDIO_S16; break;
            case IMA_ADPCM_CODE: spec->format = AUDIO_S16; break;
            default: goto unknown_bits;
            }
            break;
        case 8:
            switch(wave->encoding) {
            case PCM_CODE:  spec->format = AUDIO_U8; break;
            case ALAW_CODE: spec->format = AUDIO_S16; break;
            case MULAW_CODE: spec->format = AUDIO_S16; break;
            default: goto unknown_bits;
            }
            break;
        case 16:
            switch(wave->encoding) {
            case PCM_CODE: spec->format = AUDIO_S16; break;
            default: goto unknown_bits;
            }
            break;
        case 24:
            switch(wave->encoding) {
            case PCM_CODE:
                wave->decode = fetch_pcm24le;
                spec->format = AUDIO_S32;
                break;
            default: goto unknown_bits;
            }
            break;
        case 32:
            switch(wave->encoding) {
            case PCM_CODE:   spec->format = AUDIO_S32; break;
            case IEEE_FLOAT_CODE: spec->format = AUDIO_F32; break;
            default: goto unknown_bits;
            }
            break;
        case 64:
            switch(wave->encoding) {
            case IEEE_FLOAT_CODE:
                wave->decode = fetch_float64le;
                spec->format = AUDIO_F32;
                break;
            default: goto unknown_bits;
            }
            break;
        default:
            unknown_bits:
            Mix_SetError("Unknown PCM format with %d bits", bits);
            return SDL_FALSE;
    }
    spec->channels = (Uint8) SDL_SwapLE16(fmt.format.channels);
    wave->samplesize = spec->channels * (bits / 8);
    /* SDL_CalculateAudioSpec */
    wave->buflen = SDL_AUDIO_BITSIZE(spec->format) / 8;
    wave->buflen *= spec->channels;
    wave->buflen *= 4096;  /* reasonable sample frame count */

    return SDL_TRUE;
}

static SDL_bool ParseDATA(WAV_Music *wave, Uint32 chunk_length)
{
    wave->start = SDL_RWtell(wave->src);
    wave->stop = wave->start + chunk_length;
    if (SDL_RWseek(wave->src, chunk_length, RW_SEEK_CUR) < 0)
        return SDL_FALSE;
    return SDL_TRUE;
}

static SDL_bool AddLoopPoint(WAV_Music *wave, Uint32 play_count, Uint32 start, Uint32 stop)
{
    WAVLoopPoint *loop;
    WAVLoopPoint *loops = SDL_realloc(wave->loops, (wave->numloops + 1) * sizeof(*wave->loops));
    if (!loops) {
        Mix_OutOfMemory();
        return SDL_FALSE;
    }

    loop = &loops[ wave->numloops ];
    loop->start = start;
    loop->stop = stop;
    loop->initial_play_count = play_count;
    loop->current_play_count = play_count;

    wave->loops = loops;
    ++wave->numloops;
    return SDL_TRUE;
}

static SDL_bool ParseSMPL(WAV_Music *wave, Uint32 chunk_length)
{
    SamplerChunk *chunk;
    Uint8 *data;
    Uint32 i;
    SDL_bool loaded = SDL_FALSE;

    data = (Uint8 *)SDL_malloc(chunk_length);
    if (!data) {
        Mix_OutOfMemory();
        return SDL_FALSE;
    }
    if (SDL_RWread(wave->src, data, 1, chunk_length) != chunk_length) {
        Mix_SetError("Couldn't read %u bytes from WAV file", (unsigned)chunk_length);
        SDL_free(data);
        return SDL_FALSE;
    }
    chunk = (SamplerChunk *)data;

    for (i = 0; i < SDL_SwapLE32(chunk->sample_loops); ++i) {
        const Uint32 LOOP_TYPE_FORWARD = 0;
        Uint32 loop_type = SDL_SwapLE32(chunk->loops[i].type);
        if (loop_type == LOOP_TYPE_FORWARD) {
            AddLoopPoint(wave, SDL_SwapLE32(chunk->loops[i].play_count), SDL_SwapLE32(chunk->loops[i].start), SDL_SwapLE32(chunk->loops[i].end));
        }
    }

    loaded = SDL_TRUE;
    SDL_free(data);
    return loaded;
}

static void read_meta_field(Mix_MusicMetaTags *tags, Mix_MusicMetaTag tag_type, size_t *i, Uint32 chunk_length, Uint8 *data, size_t fieldOffset)
{
    Uint32 len = 0;
    int isID3 = fieldOffset == 7;
    char *field = NULL;
    *i += 4;
    len = isID3 ?
          SDL_SwapBE32(*((Uint32 *)(data + *i))) : /* ID3  */
          SDL_SwapLE32(*((Uint32 *)(data + *i))); /* LIST */
    if (len > chunk_length) {
        return; /* Do nothing due to broken lenght */
    }
    *i += fieldOffset;
    field = (char *)SDL_malloc(len + 1);
    SDL_memset(field, 0, (len + 1));
    SDL_strlcpy(field, (char *)(data + *i), isID3 ? len - 1 : len);
    *i += len;
    meta_tags_set(tags, tag_type, field);
    SDL_free(field);
}

static SDL_bool ParseLIST(WAV_Music *wave, Uint32 chunk_length)
{
    Uint8 *data;
    Uint32 got;
    Sint64 file_size, pos;

    data = (Uint8 *)SDL_calloc(1, chunk_length);
    if (!data) {
        Mix_OutOfMemory();
        return SDL_FALSE;
    }

    got = (Uint32)SDL_RWread(wave->src, data, 1, chunk_length);

    if (got == 0) {
        Mix_SetError("Couldn't read %u bytes from WAV file", (unsigned)chunk_length);
        SDL_free(data);
        return SDL_FALSE;
    } else if (got < chunk_length) {
        /* Workaround: Don't treat files with the LIST tag at end of file, but
         * they has an invalid chunk length. Mostly, files generated by MFAudio program. */
        file_size = SDL_RWsize(wave->src);
        pos = SDL_RWtell(wave->src);

        if (got < chunk_length && file_size != pos) {
            Mix_SetError("Couldn't read %u bytes from WAV file", (unsigned)chunk_length);
            SDL_free(data);
            return SDL_FALSE;
        }

        if (file_size == pos && chunk_length & 1) { /* Seek back one byte to avoid the error */
            SDL_RWseek(wave->src, -1, RW_SEEK_CUR);
        }
    }

    if (SDL_strncmp((char *)data, "INFO", 4) == 0) {
        size_t i = 4;
        for (i = 4; i < got - 4;) {
            if(SDL_strncmp((char *)(data + i), "INAM", 4) == 0) {
                read_meta_field(&wave->tags, MIX_META_TITLE, &i, got, data, 4);
                continue;
            } else if(SDL_strncmp((char *)(data + i), "IART", 4) == 0) {
                read_meta_field(&wave->tags, MIX_META_ARTIST, &i, got, data, 4);
                continue;
            } else if(SDL_strncmp((char *)(data + i), "IALB", 4) == 0) {
                read_meta_field(&wave->tags, MIX_META_ALBUM, &i, got, data, 4);
                continue;
            } else if (SDL_strncmp((char *)(data + i), "BCPR", 4) == 0) {
                read_meta_field(&wave->tags, MIX_META_COPYRIGHT, &i, got, data, 4);
                continue;
            }
            i++;
        }
    }

    /* done: */
    SDL_free(data);

    return SDL_TRUE;
}

static SDL_bool ParseID3(WAV_Music *wave, Uint32 chunk_length)
{
    SDL_bool loaded = SDL_TRUE;

    Uint8 *data;
    data = (Uint8 *)SDL_malloc(chunk_length);

    if (!data) {
        SDL_OutOfMemory();
        return SDL_FALSE;
    }

    if (SDL_RWread(wave->src, data, 1, chunk_length) != chunk_length) {
        Mix_SetError("Couldn't read %u bytes from WAV file", (unsigned)chunk_length);
        loaded = SDL_FALSE;
    }

    if (loaded) {
        read_id3v2_from_mem(&wave->tags, data, chunk_length);
    }

    /* done: */
    SDL_free(data);

    return loaded;
}

static SDL_bool LoadWAVMusic(WAV_Music *wave)
{
    SDL_RWops *src = wave->src;
    Uint32 chunk_type;
    Uint32 chunk_length;
    SDL_bool found_FMT = SDL_FALSE;
    SDL_bool found_DATA = SDL_FALSE;
    /* WAV magic header */
    Uint32 wavelen;
    Uint32 WAVEmagic;

    meta_tags_init(&wave->tags);

    /* Check the magic header */
    wavelen = SDL_ReadLE32(src);
    WAVEmagic = SDL_ReadLE32(src);

    (void)wavelen;   /* unused */
    (void)WAVEmagic; /* unused */

    /* Read the chunks */
    for (; ;) {
        chunk_type = SDL_ReadLE32(src);
        chunk_length = SDL_ReadLE32(src);

        if (chunk_length == 0)
            break;

        switch (chunk_type) {
        case FMT:
            found_FMT = SDL_TRUE;
            if (!ParseFMT(wave, chunk_length))
                return SDL_FALSE;
            break;
        case DATA:
            found_DATA = SDL_TRUE;
            if (!ParseDATA(wave, chunk_length))
                return SDL_FALSE;
            break;
        case SMPL:
            if (!ParseSMPL(wave, chunk_length))
                return SDL_FALSE;
            break;
        case LIST:
            if (!ParseLIST(wave, chunk_length))
                return SDL_FALSE;
            break;
        case ID3_:
            if (!ParseID3(wave, chunk_length))
                return SDL_FALSE;
            break;
        default:
            if (SDL_RWseek(src, chunk_length, RW_SEEK_CUR) < 0)
                return SDL_FALSE;
            break;
        }

        /* RIFF chunks have a 2-byte alignment. Skip padding byte. */
        if (chunk_length & 1) {
            if (SDL_RWseek(src, 1, RW_SEEK_CUR) < 0)
                return SDL_FALSE;
        }
    }

    if (!found_FMT) {
        Mix_SetError("Bad WAV file (no FMT chunk)");
        return SDL_FALSE;
    }

    if (!found_DATA) {
        Mix_SetError("Bad WAV file (no DATA chunk)");
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

/* I couldn't get SANE_to_double() to work, so I stole this from libsndfile.
 * I don't pretend to fully understand it.
 */

static Uint32 SANE_to_Uint32 (Uint8 *sanebuf)
{
    /* Negative number? */
    if (sanebuf[0] & 0x80)
        return 0;

    /* Less than 1? */
    if (sanebuf[0] <= 0x3F)
        return 1;

    /* Way too big? */
    if (sanebuf[0] > 0x40)
        return 0x4000000;

    /* Still too big? */
    if (sanebuf[0] == 0x40 && sanebuf[1] > 0x1C)
        return 800000000;

    return (Uint32)(((sanebuf[2] << 23) | (sanebuf[3] << 15) | (sanebuf[4] << 7) |
            (sanebuf[5] >> 1)) >> (29 - sanebuf[1]));
}

static SDL_bool LoadAIFFMusic(WAV_Music *wave)
{
    SDL_RWops *src = wave->src;
    SDL_AudioSpec *spec = &wave->spec;
    SDL_bool found_SSND = SDL_FALSE;
    SDL_bool found_COMM = SDL_FALSE;
    SDL_bool found_FVER = SDL_FALSE;
    SDL_bool found_VHDR = SDL_FALSE;
    SDL_bool found_BODY = SDL_FALSE;
    SDL_bool is_AIFF = SDL_FALSE;
    SDL_bool is_AIFC = SDL_FALSE;
    SDL_bool is_8SVX = SDL_FALSE;

    Uint32 chunk_type;
    Uint32 chunk_length;
    Sint64 next_chunk = 0;
    Sint64 file_length;

    /* AIFF magic header */
    Uint32 AIFFmagic;
    /* SSND chunk        */
    Uint32 offset;
    Uint32 blocksize;
    /* COMM format chunk */
    Uint16 channels = 0;
    Uint32 numsamples = 0;
    Uint16 samplesize = 0;
    Uint8 sane_freq[10];
    Uint32 frequency = 0;
    Uint32 AIFCVersion1 = 0;
    Uint32 compressionType = 0;
    char *chunk_buffer;

    file_length = SDL_RWsize(src);

    /* Check the magic header */
    chunk_length = SDL_ReadBE32(src);
    AIFFmagic = SDL_ReadLE32(src);
    if (AIFFmagic != AIFF && AIFFmagic != AIFC && AIFFmagic != _8SVX) {
        Mix_SetError("Unrecognized file type (not AIFF, AIFC, nor 8SVX)");
        return SDL_FALSE;
    }
    if (AIFFmagic == AIFF) {
        is_AIFF = SDL_TRUE;
    }
    if (AIFFmagic == AIFC) {
        is_AIFC = SDL_TRUE;
    }
    else if (AIFFmagic == _8SVX) {
        is_8SVX = SDL_TRUE;
    }

    /* From what I understand of the specification, chunks may appear in
     * any order, and we should just ignore unknown ones.
     *
     * TODO: Better sanity-checking. E.g. what happens if the AIFF file
     *       contains compressed sound data?
     */
    do {
        chunk_type      = SDL_ReadLE32(src);
        chunk_length    = SDL_ReadBE32(src);
        next_chunk      = SDL_RWtell(src) + chunk_length;

        if (chunk_length % 2) {
            next_chunk++;
        }

        switch (chunk_type) {
        case SSND:
            found_SSND = SDL_TRUE;
            offset = SDL_ReadBE32(src);
            blocksize = SDL_ReadBE32(src);
            wave->start = SDL_RWtell(src) + offset;
            (void)blocksize; /* unused */
            break;

        case FVER:
            found_FVER = SDL_TRUE;
            AIFCVersion1 = SDL_ReadBE32(src);
            (void)AIFCVersion1; /* unused */
            break;

        case AIFF_ID3_:
            if (!ParseID3(wave, chunk_length))
                return SDL_FALSE;
            break;

        case MARK:
        case INST:
            /* Just skip those chunks */
            break;

        case NAME:
        case AUTH:
        case _c__:
            chunk_buffer = (char*)SDL_calloc(1, chunk_length + 1);
            if (SDL_RWread(src, chunk_buffer, 1, chunk_length) != chunk_length) {
                SDL_free(chunk_buffer);
                return SDL_FALSE;
            }
            meta_tags_set(&wave->tags,
                          chunk_type == NAME ? MIX_META_TITLE :
                          chunk_type == AUTH ? MIX_META_ARTIST :
                          chunk_type == _c__ ? MIX_META_COPYRIGHT : 0,
                          chunk_buffer);
            SDL_free(chunk_buffer);
            break;

        case COMM:
            found_COMM = SDL_TRUE;

            /* Read the audio data format chunk */
            channels = SDL_ReadBE16(src);
            numsamples = SDL_ReadBE32(src);
            samplesize = SDL_ReadBE16(src);
            SDL_RWread(src, sane_freq, sizeof(sane_freq), 1);
            frequency = SANE_to_Uint32(sane_freq);
            if (frequency == 0) {
                Mix_SetError("Bad AIFF sample frequency");
                return SDL_FALSE;
            }
            if (is_AIFC) {
                compressionType = SDL_ReadLE32(src);
                /* here must be a "compressionName" which is a padded string */
            }
            break;

        case VHDR:
            found_VHDR  = SDL_TRUE;
            SDL_ReadBE32(src);
            SDL_ReadBE32(src);
            SDL_ReadBE32(src);
            frequency = SDL_ReadBE16(src);
            channels = 1;
            samplesize = 8;
            break;

        case BODY:
            found_BODY  = 1;
            numsamples  = chunk_length;
            wave->start = SDL_RWtell(src);
            break;

        default:
            /* Unknown/unsupported chunk: we just skip over */
            break;
        }
    } while (next_chunk < file_length && SDL_RWseek(src, next_chunk, RW_SEEK_SET) >= 0);

    if ((is_AIFF || is_AIFC) && !found_SSND) {
        Mix_SetError("Bad AIFF/AIFF-C file (no SSND chunk)");
        return SDL_FALSE;
    }

    if ((is_AIFF || is_AIFC) && !found_COMM) {
        Mix_SetError("Bad AIFF/AIFF-C file (no COMM chunk)");
        return SDL_FALSE;
    }

    if (is_8SVX && !found_VHDR) {
        Mix_SetError("Bad 8SVX (no VHDR chunk)");
        return SDL_FALSE;
    }

    if (is_8SVX && !found_BODY) {
        Mix_SetError("Bad 8SVX (no BODY chunk)");
        return SDL_FALSE;
    }

    if (is_AIFC && !found_FVER) {
        Mix_SetError("Bad AIFF-C file (no FVER chunk)");
        return SDL_FALSE;
    }

    wave->samplesize = channels * (samplesize / 8);
    wave->stop = wave->start + channels * numsamples * (samplesize / 8);

    /* Decode the audio data format */
    SDL_memset(spec, 0, (sizeof *spec));
    spec->freq = (int)frequency;
    switch (samplesize) {
    case 8:
        if (!is_AIFC)
            spec->format = AUDIO_S8;
        else switch (compressionType) {
        case raw_: spec->format = AUDIO_U8; break;
        case sowt: spec->format = AUDIO_S8; break;
        case ulaw:
            spec->format = AUDIO_S16LSB;
            wave->encoding = MULAW_CODE;
            wave->decode = fetch_ulaw;
            break;
        case alaw:
            spec->format = AUDIO_S16LSB;
            wave->encoding = ALAW_CODE;
            wave->decode = fetch_alaw;
            break;
        default: goto unsupported_format;
        }
        break;
    case 16:
        if (!is_AIFC)
            spec->format = AUDIO_S16MSB;
        else switch (compressionType) {
        case sowt: spec->format = AUDIO_S16LSB; break;
        case NONE: spec->format = AUDIO_S16MSB; break;
        case ULAW:
            spec->format = AUDIO_S16LSB;
            wave->encoding = MULAW_CODE;
            wave->decode = fetch_ulaw;
            break;
        case ALAW:
            spec->format = AUDIO_S16LSB;
            wave->encoding = ALAW_CODE;
            wave->decode = fetch_alaw;
            break;
        default: goto unsupported_format;
        }
        break;
    case 24:
        wave->encoding = PCM_CODE;
        wave->decode = fetch_pcm24be;
        if (!is_AIFC)
            spec->format = AUDIO_S32MSB;
        else switch (compressionType) {
        case sowt: spec->format = AUDIO_S32LSB; break;
        case NONE: spec->format = AUDIO_S32MSB; break;
        default: goto unsupported_format;
        }
        break;
    case 32:
        if (!is_AIFC)
            spec->format = AUDIO_S32MSB;
        else switch (compressionType) {
        case sowt: spec->format = AUDIO_S32LSB; break;
        case NONE: spec->format = AUDIO_S32MSB; break;
        case fl32:
        case FL32: spec->format = AUDIO_F32MSB; break;
        default: goto unsupported_format;
        }
        break;
    case 64:
        wave->encoding = IEEE_FLOAT_CODE;
        wave->decode = fetch_float64be;
        if (!is_AIFC)
            spec->format = AUDIO_F32;
        else switch (compressionType) {
        case fl64:
            spec->format = AUDIO_F32;
            break;
        default: goto unsupported_format;
        }
        break;
    default:
    unsupported_format:
        Mix_SetError("Unknown samplesize in data format");
        return SDL_FALSE;
    }
    spec->channels = (Uint8) channels;

    return SDL_TRUE;
}

Mix_MusicInterface Mix_MusicInterface_WAV =
{
    "WAVE",
    MIX_MUSIC_WAVE,
    MUS_WAV,
    SDL_FALSE,
    SDL_FALSE,

    NULL,   /* Load */
    NULL,   /* Open */
    WAV_CreateFromRW,
    NULL,   /* CreateFromRWex [MIXER-X]*/
    NULL,   /* CreateFromFile */
    NULL,   /* CreateFromFileEx [MIXER-X]*/
    WAV_SetVolume,
    WAV_GetVolume,
    WAV_Play,
    NULL,   /* IsPlaying */
    WAV_GetAudio,
    NULL,       /* Jump */
    WAV_Seek,   /* Seek */
    WAV_Tell,   /* Tell */
    WAV_Duration,
    NULL,   /* SetTempo [MIXER-X] */
    NULL,   /* GetTempo [MIXER-X] */
    NULL,   /* SetSpeed [MIXER-X] */
    NULL,   /* GetSpeed [MIXER-X] */
    NULL,   /* SetPitch [MIXER-X] */
    NULL,   /* GetPitch [MIXER-X] */
    NULL,   /* GetTracksCount [MIXER-X] */
    NULL,   /* SetTrackMute [MIXER-X] */
    NULL,   /* LoopStart */
    NULL,   /* LoopEnd */
    NULL,   /* LoopLength */
    WAV_GetMetaTag,   /* GetMetaTag */
    NULL,   /* GetNumTracks */
    NULL,   /* StartTrack */
    NULL,   /* Pause */
    NULL,   /* Resume */
    WAV_Stop, /* Stop */
    WAV_Delete,
    NULL,   /* Close */
    NULL    /* Unload */
};

#endif /* MUSIC_WAV */

/* vi: set ts=4 sw=4 expandtab: */
