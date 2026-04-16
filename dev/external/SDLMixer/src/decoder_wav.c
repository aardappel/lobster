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

#ifdef DECODER_WAV

#include "SDL_mixer_internal.h"


// this is originally SDL2_mixer's music_wav.c, which was probably
// SDL's SDL_wave.c at some point. It's been heavily modified for this.


// !!! FIXME: some of this is duplicated in decoder_aiff.c; if we end up
// !!! FIXME: supporting more formats that need it, we should generalize it.


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
#define ID3x        0x20336469      /* "id3 " */
#define ID3X        0x20334449      /* "ID3 " */
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


// channel mask bits in WAV file
#define WAV_SPEAKER_FRONT_LEFT            (1 << 0)
#define WAV_SPEAKER_FRONT_RIGHT           (1 << 1)
#define WAV_SPEAKER_FRONT_CENTER          (1 << 2)
#define WAV_SPEAKER_LOW_FREQUENCY         (1 << 3)
#define WAV_SPEAKER_BACK_LEFT             (1 << 4)
#define WAV_SPEAKER_BACK_RIGHT            (1 << 5)
#define WAV_SPEAKER_FRONT_LEFT_OF_CENTER  (1 << 6)
#define WAV_SPEAKER_FRONT_RIGHT_OF_CENTER (1 << 7)
#define WAV_SPEAKER_BACK_CENTER           (1 << 8)
#define WAV_SPEAKER_SIDE_LEFT             (1 << 9)
#define WAV_SPEAKER_SIDE_RIGHT            (1 << 10)


#pragma pack(push, 1)
typedef struct {
    // Not saved in the chunk we read:
    //Uint32  chunkID;
    //Uint32  chunkLen;
    Uint16  encoding;
    Uint16  channels;       // 1 = mono, 2 = stereo
    Uint32  frequency;      // One of 11025, 22050, or 44100 Hz
    Uint32  byterate;       // Average bytes per second
    Uint16  blockalign;     // Bytes per sample block
    Uint16  bitspersample;      // One of 8, 12, 16, or 4 for ADPCM
} WaveFMT;

typedef struct {
    WaveFMT format;
    Uint16  cbSize;
    union {
        Uint16 validbitspersample; // bits of precision
        Uint16 samplesperblock;   // valid if wBitsPerSample==0
        Uint16 reserved;         // If neither applies, set to zero.
    } Samples;
    Uint32 channelsmask;
    // GUID subFormat 16 bytes
    Uint32 subencoding;
    Uint16 sub_data2;
    Uint16 sub_data3;
    Uint8  sub_data[8];
} WaveFMTEx;

typedef struct {
    Uint32 identifier;
    Uint32 type;
    Uint32 start;  // this is SAMPLE FRAMES, not byte offset!
    Uint32 end;    // this is SAMPLE FRAMES, not byte offset!
    Uint32 fraction;
    Uint32 play_count;
} SampleLoop;

typedef struct {
    // Not saved in the chunk we read:
    //Uint32  chunkID;
    //Uint32  chunkLen;
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

typedef struct ADPCM_DecoderInfo
{
    Uint32 channels;        // Number of channels.
    size_t blocksize;       // Size of an ADPCM block in bytes.
    size_t blockheadersize; // Size of an ADPCM block header in bytes.
    size_t samplesperblock; // Number of samples per channel in an ADPCM block.
    void *ddata;            // Decoder data from initialization.
} ADPCM_DecoderInfo;

typedef struct MS_ADPCM_CoeffData
{
    Uint16 coeffcount;
    Sint16 *coeff;
    Sint16 aligndummy; // Has to be last member.
} MS_ADPCM_CoeffData;

typedef struct ADPCM_DecoderState
{
    const ADPCM_DecoderInfo *info;

    void *cstate;           // Decoding state for each channel.

    // Current ADPCM block
    struct
    {
        Uint8 *data;
        size_t size;
        size_t pos;
    } block;

    // Decoded 16-bit PCM data.
    struct
    {
        Sint16 *data;
        size_t size;
        size_t pos;
        size_t read;
    } output;
} ADPCM_DecoderState;

typedef struct MS_ADPCM_ChannelState
{
    Uint16 delta;
    Sint16 coeff1;
    Sint16 coeff2;
} MS_ADPCM_ChannelState;

typedef struct WAVLoopPoint
{
    Uint32 start;
    Uint32 stop;
    Uint32 iterations;
} WAVLoopPoint;

// precalc some positional things to make seeking and looping easier.
typedef struct WAVSeekBlock
{
    Sint64 frame_start;
    Sint64 num_frames;
    Sint64 iterations;
    Sint64 seek_position;     // for compressed formats, this is the start of the block. For uncompressed, it's the start of the sample frame.
} WAVSeekBlock;

typedef struct WAV_TrackData WAV_TrackData;

typedef int (*WAV_FetchFn)(WAV_TrackData *tdata, Uint8 *buffer, int buflen);

typedef struct WAV_AudioData
{
    ADPCM_DecoderInfo adpcm_info;
    Uint16 encoding;
    Sint64 start;
    Sint64 stop;
    int framesize;
    int decoded_framesize;
    WAV_FetchFn fetch;
    unsigned int numloops;
    WAVLoopPoint *loops;
    unsigned int num_seekblocks;
    WAVSeekBlock *seekblocks;
    Uint32 channelmask;
} WAV_AudioData;

struct WAV_TrackData
{
    const WAV_AudioData *adata;
    SDL_IOStream *io;
    ADPCM_DecoderState adpcm_state;
    const WAVSeekBlock *seekblock;  // current seekblock we're decoding.
    Uint32 current_iteration;        // current loop iteration in seekblock
    Uint32 current_iteration_frames;  // current framecount into seekblock.
    int channels;
    bool must_set_channel_map;
};

static bool IsADPCM(const Uint16 encoding)
{
    return ((encoding == MS_ADPCM_CODE) || (encoding == IMA_ADPCM_CODE));
}

static bool MS_ADPCM_Init(ADPCM_DecoderInfo *info, const Uint8 *chunk_data, Uint32 chunk_length)
{
    const WaveFMTEx *fmt = (WaveFMTEx *)chunk_data;
    const Uint16 channels = SDL_Swap16LE(fmt->format.channels);
    const Uint16 blockalign = SDL_Swap16LE(fmt->format.blockalign);
    const Uint16 bitspersample = SDL_Swap16LE(fmt->format.bitspersample);
    const size_t blockheadersize = (size_t)channels * 7;
    const size_t blockdatasize = (size_t)blockalign - blockheadersize;
    const size_t blockframebitsize = (size_t)bitspersample * channels;
    const size_t blockdatasamples = (blockdatasize * 8) / blockframebitsize;

    // While it's clear how IMA ADPCM handles more than two channels, the nibble
    // order of MS ADPCM makes it awkward. The Standards Update does not talk
    // about supporting more than stereo anyway.
    if (channels > 2) {
        return SDL_SetError("WAV: ADPCM Invalid number of channels");
    } else if (bitspersample != 4) {
        return SDL_SetError("WAV: Invalid MS ADPCM bits per sample of %u", (unsigned int)bitspersample);
    } else if (blockalign < blockheadersize) {  // The block size must be big enough to contain the block header.
        return SDL_SetError("WAV: Invalid MS ADPCM block size (nBlockAlign)");
    }

    const Uint16 cbExtSize = SDL_Swap16LE(fmt->cbSize);
    Uint16 samplesperblock = SDL_Swap16LE(fmt->Samples.samplesperblock);

    // Number of coefficient pairs. A pair has two 16-bit integers.
    size_t coeffcount = (size_t)chunk_data[20] | ((size_t)chunk_data[21] << 8);

    // bPredictor, the integer offset into the coefficients array, is only
    // 8 bits. It can only address the first 256 coefficients. Let's limit
    // the count number here.
    if (coeffcount > 256) {
        coeffcount = 256;
    }

    // There are wSamplesPerBlock, wNumCoef, and at least 7 coefficient pairs in the extended part of the header.
    if (chunk_length < 22 + coeffcount * 4) {
        return SDL_SetError("WAV: Chunk size too small for MS ADPCM format header");
    } else if (cbExtSize < 4 + coeffcount * 4) {
        return SDL_SetError("WAV: Invalid MS ADPCM format header (too small)");
    } else if (coeffcount < 7) {
        return SDL_SetError("Missing required coefficients in MS ADPCM format header");
    }

    // Technically, wSamplesPerBlock is required, but we have all the
    // information in the other fields to calculate it, if it's zero.
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
        return SDL_SetError("WAV: Invalid number of samples per MS ADPCM block (wSamplesPerBlock)");
    }

    MS_ADPCM_CoeffData *coeffdata = (MS_ADPCM_CoeffData *)SDL_malloc(sizeof(MS_ADPCM_CoeffData) + coeffcount * 4);
    if (coeffdata == NULL) {
        return false;
    }
    coeffdata->coeff = &coeffdata->aligndummy;
    coeffdata->coeffcount = (Uint16)coeffcount;
    info->ddata = coeffdata;  // Freed in cleanup.

    // Copy the 16-bit pairs.
    for (size_t i = 0; i < coeffcount * 2; i++) {
        Sint32 c = chunk_data[22 + i * 2] | ((Sint32)chunk_data[23 + i * 2] << 8);
        if (c >= 0x8000) {
            c -= 0x10000;
        }
        static const Sint16 presetcoeffs[14] = { 256, 0, 512, -256, 0, 0, 192, 64, 240, 0, 460, -208, 392, -232 };
        if ((i < 14) && (c != presetcoeffs[i])) {
            return SDL_SetError("WAV: Wrong preset coefficients in MS ADPCM format header");
        }
        coeffdata->coeff[i] = (Sint16)c;
    }

    info->blocksize = blockalign;
    info->channels = channels;
    info->blockheadersize = blockheadersize;
    info->samplesperblock = samplesperblock;

    return true;
}

static Sint16 MS_ADPCM_ProcessNibble(MS_ADPCM_ChannelState *cstate, Sint32 sample1, Sint32 sample2, Uint8 nybble)
{
    const Sint32 max_audioval = 32767;
    const Sint32 min_audioval = -32768;
    const Uint16 max_deltaval = 65535;
    static const Uint16 adaptive[] = {
        230, 230, 230, 230, 307, 409, 512, 614,
        768, 614, 512, 409, 307, 230, 230, 230
    };

    Sint32 new_sample = (sample1 * cstate->coeff1 + sample2 * cstate->coeff2) / 256;
    // The nibble is a signed 4-bit error delta.
    Sint32 errordelta = (Sint32)nybble - (nybble >= 0x08 ? 0x10 : 0);
    Uint32 delta = cstate->delta;
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
        // This issue is not described in the Standards Update and therefore
        // undefined. It seems sensible to prevent overflows with a limit.
        delta = max_deltaval;
    }

    cstate->delta = (Uint16)delta;
    return (Sint16)new_sample;
}

static bool MS_ADPCM_DecodeBlockHeader(ADPCM_DecoderState *state)
{
    const ADPCM_DecoderInfo *info = state->info;
    const Uint32 channels = info->channels;
    MS_ADPCM_ChannelState *cstate = (MS_ADPCM_ChannelState *)state->cstate;
    MS_ADPCM_CoeffData *ddata = (MS_ADPCM_CoeffData *)info->ddata;

    if (state->block.size < info->blockheadersize) {
        return SDL_SetError("Invalid ADPCM header");
    }

    for (Uint32 c = 0; c < channels; c++) {
        size_t o = c;

        // Load the coefficient pair into the channel state.
        const Uint8 coeffindex = state->block.data[o];
        if (coeffindex > ddata->coeffcount) {
            return SDL_SetError("Invalid MS ADPCM coefficient index in block header");
        }
        cstate[c].coeff1 = ddata->coeff[coeffindex * 2];
        cstate[c].coeff2 = ddata->coeff[coeffindex * 2 + 1];

        // Initial delta value.
        o = (size_t)channels + c * 2;
        cstate[c].delta = state->block.data[o] | ((Uint16)state->block.data[o + 1] << 8);

        // Load the samples from the header. Interestingly, the sample later in
        //the output stream comes first.
        o = (size_t)channels * 3 + c * 2;
        Sint32 sample = state->block.data[o] | ((Sint32)state->block.data[o + 1] << 8);
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

    state->block.pos += info->blockheadersize;

    // Skip second sample frame that came from the header.
    state->output.pos += info->channels;

    return true;
}

// Decodes the data of the MS ADPCM block. Decoding will stop if a block is too
// short, returning with none or partially decoded data. The partial data
// will always contain full sample frames (same sample count for each channel).
// Incomplete sample frames are discarded.
static bool MS_ADPCM_DecodeBlockData(ADPCM_DecoderState *state)
{
    const ADPCM_DecoderInfo *info = state->info;
    const Uint32 channels = info->channels;
    MS_ADPCM_ChannelState *cstate = (MS_ADPCM_ChannelState *)state->cstate;
    Uint16 nybble = 0;
    Sint16 sample1, sample2;
    size_t blockpos = state->block.pos;
    size_t blocksize = state->block.size;
    size_t outpos = state->output.pos;
    size_t blockframesleft = info->samplesperblock - 2;

    while (blockframesleft > 0) {
        for (Uint32 c = 0; c < channels; c++) {
            if (nybble & 0x4000) {
                nybble <<= 4;
            } else if (blockpos < blocksize) {
                nybble = state->block.data[blockpos++] | 0x4000;
            } else {
                // Out of input data. Drop the incomplete frame and return.
                state->output.pos = outpos - c;
                return false;
            }

            // Load previous samples which may come from the block header.
            sample1 = state->output.data[outpos - channels];
            sample2 = state->output.data[outpos - channels * 2];

            sample1 = MS_ADPCM_ProcessNibble(cstate + c, sample1, sample2, (nybble >> 4) & 0x0f);
            state->output.data[outpos++] = sample1;
        }

        blockframesleft--;
    }

    state->output.pos = outpos;

    return true;
}

static bool IMA_ADPCM_Init(ADPCM_DecoderInfo *info, const Uint8 *chunk_data, Uint32 chunk_length)
{
    const WaveFMTEx *fmt = (WaveFMTEx *)chunk_data;
    const Uint16 formattag = SDL_Swap16LE(fmt->format.encoding);
    const Uint16 channels = SDL_Swap16LE(fmt->format.channels);
    const Uint16 blockalign = SDL_Swap16LE(fmt->format.blockalign);
    const Uint16 bitspersample = SDL_Swap16LE(fmt->format.bitspersample);
    const size_t blockheadersize = (size_t)channels * 4;
    const size_t blockdatasize = (size_t)blockalign - blockheadersize;
    const size_t blockframebitsize = (size_t)bitspersample * channels;
    const size_t blockdatasamples = (blockdatasize * 8) / blockframebitsize;
    Uint16 samplesperblock = 0;

    // IMA ADPCM can also have 3-bit samples, but it's not supported by SDL at this time.
    if (bitspersample == 3) {
        return SDL_SetError("WAV: 3-bit IMA ADPCM currently not supported");
    } else if (bitspersample != 4) {
        return SDL_SetError("WAV: Invalid IMA ADPCM bits per sample of %u", (unsigned int)bitspersample);
    } else if ((blockalign < blockheadersize) || (blockalign % 4)) {  // The block size is required to be a multiple of 4 and it must be able to hold a block header.
        return SDL_SetError("WAV: Invalid IMA ADPCM block size (nBlockAlign)");
    }

    if (formattag == EXTENSIBLE_CODE) {
        // There's no specification for this, but it's basically the same
        // format because the extensible header has wSampePerBlocks too.
    } else if (chunk_length >= 20) {
        Uint16 cbExtSize = SDL_Swap16LE(fmt->cbSize);
        if (cbExtSize >= 2) {
            samplesperblock = SDL_Swap16LE(fmt->Samples.samplesperblock);
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
        return SDL_SetError("WAV: Invalid number of samples per IMA ADPCM block (wSamplesPerBlock)");
    }

    info->blocksize = blockalign;
    info->channels = channels;
    info->blockheadersize = blockheadersize;
    info->samplesperblock = samplesperblock;

    return true;
}

static Sint16 IMA_ADPCM_ProcessNibble(Sint8 *cindex, Sint16 lastsample, Uint8 nybble)
{
    const Sint32 max_audioval = 32767;
    const Sint32 min_audioval = -32768;
    static const Sint8 index_table_4b[16] = {
        -1, -1, -1, -1,
        2, 4, 6, 8,
        -1, -1, -1, -1,
        2, 4, 6, 8
    };
    static const Uint16 step_table[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
        34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
        143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
        449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
        1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
        3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
        9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
        22385, 24623, 27086, 29794, 32767
    };

    Sint8 index = *cindex;

    // Clamp index into valid range.
    if (index > 88) {
        index = 88;
    } else if (index < 0) {
        index = 0;
    }

    // explicit cast to avoid gcc warning about using 'char' as array index
    const Uint32 step = step_table[(size_t)index];

    // Update index value
    *cindex = index + index_table_4b[nybble];

    /* This calculation uses shifts and additions because multiplications were
     * much slower back then. Sadly, this can't just be replaced with an actual
     * multiplication now as the old algorithm drops some bits. The closest
     * approximation I could find is something like this:
     * (nybble & 0x8 ? -1 : 1) * ((nybble & 0x7) * step / 4 + step / 8)
     */
    Sint32 delta = step >> 3;
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

    Sint32 sample = lastsample + delta;

    // Clamp output sample
    if (sample > max_audioval) {
        sample = max_audioval;
    } else if (sample < min_audioval) {
        sample = min_audioval;
    }

    return (Sint16)sample;
}

static bool IMA_ADPCM_DecodeBlockHeader(ADPCM_DecoderState *state)
{
    const ADPCM_DecoderInfo *info = state->info;
    Uint8 *cstate = (Uint8 *)state->cstate;

    if (state->block.size < info->blockheadersize) {
        return SDL_SetError("Invalid ADPCM header");
    }

    for (Uint32 c = 0; c < info->channels; c++) {
        const size_t o = state->block.pos + c * 4;

        // Extract the sample from the header.
        Sint32 sample = state->block.data[o] | ((Sint32)state->block.data[o + 1] << 8);
        if (sample >= 0x8000) {
            sample -= 0x10000;
        }
        state->output.data[state->output.pos++] = (Sint16)sample;

        // Channel step index.
        const Sint16 step = (Sint16)state->block.data[o + 2];
        cstate[c] = (Sint8)(step > 0x80 ? step - 0x100 : step);

        // Reserved byte in block header, should be 0.
        if (state->block.data[o + 3] != 0) {
            // Uh oh, corrupt data?  Buggy code?
        }
    }

    state->block.pos += info->blockheadersize;

    return true;
}

// Decodes the data of the IMA ADPCM block. Decoding will stop if a block is too
// short, returning with none or partially decoded data. The partial data always
// contains full sample frames (same sample count for each channel).
// Incomplete sample frames are discarded.
static bool IMA_ADPCM_DecodeBlockData(ADPCM_DecoderState *state)
{
    bool retval = true;
    const ADPCM_DecoderInfo *info = state->info;
    const Uint32 channels = info->channels;
    const size_t subblockframesize = (size_t)channels * 4;
    size_t blockpos = state->block.pos;
    const size_t blocksize = state->block.size;
    const size_t blockleft = blocksize - blockpos;
    size_t outpos = state->output.pos;
    Sint64 blockframesleft = info->samplesperblock - 1;
    const Uint64 bytesrequired = (blockframesleft + 7) / 8 * subblockframesize;

    if (blockleft < bytesrequired) {
        // Data truncated. Calculate how many samples we can get out if it.
        const size_t guaranteedframes = blockleft / subblockframesize;
        const size_t remainingbytes = blockleft % subblockframesize;
        blockframesleft = guaranteedframes;
        if (remainingbytes > subblockframesize - 4) {
            blockframesleft += (remainingbytes % 4) * 2;
        }
        // Signal the truncation.
        retval = false;
    }

    /* Each channel has their nibbles packed into 32-bit blocks. These blocks
     * are interleaved and make up the data part of the ADPCM block. This loop
     * decodes the samples as they come from the input data and puts them at
     * the appropriate places in the output data.
     */
    while (blockframesleft > 0) {
        const size_t subblocksamples = blockframesleft < 8 ? (size_t)blockframesleft : 8;

        for (Uint32 c = 0; c < channels; c++) {
            Uint8 nybble = 0;
            // Load previous sample which may come from the block header.
            Sint16 sample = state->output.data[outpos + c - channels];

            for (size_t i = 0; i < subblocksamples; i++) {
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

static void ADPCM_InfoCleanup(ADPCM_DecoderInfo *info)
{
    SDL_free(info->ddata);
}

static void ADPCM_StateCleanup(ADPCM_DecoderState *state)
{
    SDL_free(state->cstate);
    SDL_free(state->block.data);
    SDL_free(state->output.data);
}

typedef bool (*ADPCM_DecodeBlockFn)(ADPCM_DecoderState *state);

static int FetchADPCM(WAV_TrackData *tdata, Uint8 *buffer, int buflen, ADPCM_DecodeBlockFn DecodeBlockHeader, ADPCM_DecodeBlockFn DecodeBlockData)
{
    ADPCM_DecoderState *state = &tdata->adpcm_state;
    const ADPCM_DecoderInfo *info = state->info;
    size_t left = (size_t)buflen;
    Uint8 *dst = buffer;

    while (left > 0) {
        if (state->output.read == state->output.pos) {
            size_t bytesread = SDL_ReadIO(tdata->io, state->block.data, info->blocksize);
            if (bytesread == 0) {
                return buflen - left;
            }

            state->block.size = (bytesread < info->blocksize) ? bytesread : info->blocksize;
            state->block.pos = 0;
            state->output.pos = 0;
            state->output.read = 0;

            if (!DecodeBlockHeader(state) || !DecodeBlockData(state)) {
                return -1;
            }
        }
        const size_t len = SDL_min(left, (state->output.pos - state->output.read) * sizeof(Sint16));
        SDL_memcpy(dst, &state->output.data[state->output.read], len);
        state->output.read += (len / sizeof(Sint16));
        dst += len;
        left -= len;
    }

    return buflen;
}

static int FetchMSADPCM(WAV_TrackData *tdata, Uint8 *buffer, int buflen)
{
    return FetchADPCM(tdata, buffer, buflen, MS_ADPCM_DecodeBlockHeader, MS_ADPCM_DecodeBlockData);
}

static int FetchIMAADPCM(WAV_TrackData *tdata, Uint8 *buffer, int buflen)
{
    return FetchADPCM(tdata, buffer, buflen, IMA_ADPCM_DecodeBlockHeader, IMA_ADPCM_DecodeBlockData);
}

static int FetchXLaw(WAV_TrackData *tdata, Uint8 *buffer, int buflen, const float *lut)
{
    int length = buflen;
    length = (int) SDL_ReadIO(tdata->io, buffer, (size_t)(length / 4));
    if (length % tdata->adata->framesize != 0) {
        length -= length % tdata->adata->framesize;
    }
    float *out = (float *) &buffer[(length - 1) * 4];
    for (int i = length - 1; i >= 0; i--) {
        out[i] = lut[buffer[i]];
    }
    return length * 4;
}

static int FetchULaw(WAV_TrackData *tdata, Uint8 *buffer, int buflen)
{
    return FetchXLaw(tdata, buffer, buflen, MIX_ulawToFloat);
}

static int FetchALaw(WAV_TrackData *tdata, Uint8 *buffer, int buflen)
{
    return FetchXLaw(tdata, buffer, buflen, MIX_alawToFloat);
}

static int FetchPCM(WAV_TrackData *tdata, Uint8 *buffer, int buflen)
{
    return SDL_ReadIO(tdata->io, buffer, buflen);
}

static int FetchPCM24LE(WAV_TrackData *tdata, Uint8 *buffer, int buflen)
{
    int length = buflen;
    length = (int) SDL_ReadIO(tdata->io, buffer, (size_t)((length / 4) * 3));
    if ((length % tdata->adata->framesize) != 0) {
        length -= length % tdata->adata->framesize;
    }
    int i = 0, o = 0;
    for (i = length - 3, o = ((length - 3) / 3) * 4; i >= 0; i -= 3, o -= 4) {
        const Uint8 *x = &buffer[i];
        const Sint32 in = ((Sint32)(Sint8)x[2] << 16) | ((int32_t)x[1] << 8) | x[0];
        float *out = (float *) &buffer[o];
        *out = ((float) in) / 8388608.0f;
    }
    return (length / 3) * 4;
}


#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define MIX_SwapDoubleLE(X)  (X)
#else
static double MIX_SwapDoubleLE(double x)
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
#endif

static int FetchFloat64LE(WAV_TrackData *tdata, Uint8 *buffer, int buflen)
{
    int length = buflen;
    length = (int) SDL_ReadIO(tdata->io, buffer, (size_t)length);
    if (length % tdata->adata->framesize != 0) {
        length -= length % tdata->adata->framesize;
    }
    float *out = (float *) buffer;
    int i, o;
    for (i = 0, o = 0; i < length; i += 8, o++) {
        out[o] = (float)MIX_SwapDoubleLE(*(double*)(buffer + i));
    }
    return length / 2;
}

static Uint32 StandardSDLWavChannelMask(int channels)
{
    switch (channels) {
        case 1: return WAV_SPEAKER_FRONT_CENTER;
        case 2: return WAV_SPEAKER_FRONT_LEFT|WAV_SPEAKER_FRONT_RIGHT;
        case 3: return WAV_SPEAKER_FRONT_LEFT|WAV_SPEAKER_FRONT_RIGHT|WAV_SPEAKER_LOW_FREQUENCY;
        case 4: return WAV_SPEAKER_FRONT_LEFT|WAV_SPEAKER_FRONT_RIGHT|WAV_SPEAKER_BACK_LEFT|WAV_SPEAKER_BACK_RIGHT;
        case 5: return WAV_SPEAKER_FRONT_LEFT|WAV_SPEAKER_FRONT_RIGHT|WAV_SPEAKER_BACK_LEFT|WAV_SPEAKER_BACK_RIGHT|WAV_SPEAKER_LOW_FREQUENCY;
        case 6: return WAV_SPEAKER_FRONT_LEFT|WAV_SPEAKER_FRONT_RIGHT|WAV_SPEAKER_FRONT_CENTER|WAV_SPEAKER_BACK_LEFT|WAV_SPEAKER_BACK_RIGHT|WAV_SPEAKER_LOW_FREQUENCY;
        case 7: return WAV_SPEAKER_FRONT_LEFT|WAV_SPEAKER_FRONT_RIGHT|WAV_SPEAKER_FRONT_CENTER|WAV_SPEAKER_BACK_CENTER|WAV_SPEAKER_SIDE_LEFT|WAV_SPEAKER_SIDE_RIGHT|WAV_SPEAKER_LOW_FREQUENCY;
        case 8: return WAV_SPEAKER_FRONT_LEFT|WAV_SPEAKER_FRONT_RIGHT|WAV_SPEAKER_FRONT_CENTER|WAV_SPEAKER_BACK_LEFT|WAV_SPEAKER_BACK_RIGHT|WAV_SPEAKER_SIDE_LEFT|WAV_SPEAKER_SIDE_RIGHT|WAV_SPEAKER_LOW_FREQUENCY;
        default: return (channels < 32) ? (Uint32)((1 << channels) - 1) : (Uint32) 0xFFFFFFFF;   // mark all available channels as used by default.
    }
    SDL_assert(!"shouldn't hit this.");
    return 0;
}

static bool ParseFMT(WAV_AudioData *adata, SDL_IOStream *io, SDL_AudioSpec *spec, Uint32 chunk_length)
{
    WaveFMTEx fmt;

    if (chunk_length < sizeof(fmt.format)) {
        return SDL_SetError("Wave format chunk too small");
    }

    Uint8 *chunk = (Uint8 *)SDL_malloc(chunk_length);
    if (!chunk) {
        return false;
    }

    if (SDL_ReadIO(io, chunk, chunk_length) != chunk_length) {
        SDL_free(chunk);
        return SDL_SetError("Couldn't read %" SDL_PRIu32 " bytes from WAV file", chunk_length);
    }

    size_t size = (chunk_length >= sizeof(fmt)) ? sizeof(fmt) : sizeof(fmt.format);
    SDL_zero(fmt);
    SDL_memcpy(&fmt, chunk, size);

    adata->encoding = SDL_Swap16LE(fmt.format.encoding);

    if (adata->encoding == EXTENSIBLE_CODE) {
        if (size < sizeof(fmt)) {
            SDL_free(chunk);
            return SDL_SetError("Wave format chunk too small");
        }
        adata->encoding = (Uint16)SDL_Swap32LE(fmt.subencoding);
        adata->channelmask = SDL_Swap32LE(fmt.channelsmask);
    } else {
        adata->channelmask = StandardSDLWavChannelMask((int) SDL_Swap16LE(fmt.format.channels));
    }

    // Decode the audio data format
    switch (adata->encoding) {
        case PCM_CODE:
        case IEEE_FLOAT_CODE:
            adata->fetch = FetchPCM;
            break;
        case MULAW_CODE:
            adata->fetch = FetchULaw;
            break;
        case ALAW_CODE:
            adata->fetch = FetchALaw;
            break;
        case MS_ADPCM_CODE:
            adata->fetch = FetchMSADPCM;
            if (!MS_ADPCM_Init(&adata->adpcm_info, chunk, chunk_length)) {
                SDL_free(chunk);
                return false;
            }
            break;
        case IMA_ADPCM_CODE:
            adata->fetch = FetchIMAADPCM;
            if (!IMA_ADPCM_Init(&adata->adpcm_info, chunk, chunk_length)) {
                SDL_free(chunk);
                return false;
            }
            break;

        // !!! FIXME: pass off embedded MP3 data to drmp3/mpg123?

        default:
            SDL_free(chunk);
            return SDL_SetError("Unknown WAVE data format");
    }

    SDL_free(chunk);

    spec->freq = (int)SDL_Swap32LE(fmt.format.frequency);
    const int bits = (int)SDL_Swap16LE(fmt.format.bitspersample);
    bool unknown_bits = false;
    switch (bits) {
        case 4:
            switch(adata->encoding) {
            case MS_ADPCM_CODE: spec->format = SDL_AUDIO_S16; break;
            case IMA_ADPCM_CODE: spec->format = SDL_AUDIO_S16; break;
            default: unknown_bits = true; break;
            }
            break;
        case 8:
            switch(adata->encoding) {
            case PCM_CODE:  spec->format = SDL_AUDIO_U8; break;
            case ALAW_CODE: spec->format = SDL_AUDIO_F32; break;
            case MULAW_CODE: spec->format = SDL_AUDIO_F32; break;
            default: unknown_bits = true; break;
            }
            break;
        case 16:
            switch(adata->encoding) {
            case PCM_CODE: spec->format = SDL_AUDIO_S16LE; break;
            default: unknown_bits = true; break;
            }
            break;
        case 24:
            switch(adata->encoding) {
            case PCM_CODE:
                adata->fetch = FetchPCM24LE;
                spec->format = SDL_AUDIO_F32;
                break;
            default: unknown_bits = true; break;
            }
            break;
        case 32:
            switch(adata->encoding) {
            case PCM_CODE:   spec->format = SDL_AUDIO_S32LE; break;
            case IEEE_FLOAT_CODE: spec->format = SDL_AUDIO_F32LE; break;
            default: unknown_bits = true; break;
            }
            break;
        case 64:
            switch(adata->encoding) {
            case IEEE_FLOAT_CODE:
                adata->fetch = FetchFloat64LE;
                spec->format = SDL_AUDIO_F32;
                break;
            default: unknown_bits = true; break;
            }
            break;
        default: unknown_bits = true; break;
    }

    if (unknown_bits) {
        return SDL_SetError("WAV: Unknown PCM format with %d bits", bits);
    }

    spec->channels = (Uint8) SDL_Swap16LE(fmt.format.channels);
    adata->framesize = spec->channels * (bits / 8);
    adata->decoded_framesize = SDL_AUDIO_FRAMESIZE(*spec);

    return true;
}

static bool ParseDATA(WAV_AudioData *adata, SDL_IOStream *io, Uint32 chunk_length)
{
    adata->start = SDL_TellIO(io);
    adata->stop = adata->start + chunk_length;
    return true;
}

static bool AddLoopPoint(WAV_AudioData *adata, Uint32 play_count, Uint32 start, Uint32 stop)
{
    // ignore the loop if it's bogus but carry on.
    if (start >= stop) {
        return true;
    }

    WAVLoopPoint *loop;
    WAVLoopPoint *loops = SDL_realloc(adata->loops, (adata->numloops + 1) * sizeof(*adata->loops));
    if (!loops) {
        return false;
    }

    //SDL_Log("LOOP: count=%d start=%d stop=%d", (int) play_count, (int) start, (int) stop);

    loop = &loops[adata->numloops];
    loop->start = start;
    loop->stop = stop;
    loop->iterations = play_count;

    adata->loops = loops;
    ++adata->numloops;

    return true;
}

static bool ParseSMPL(WAV_AudioData *adata, SDL_IOStream *io, Uint32 chunk_length)
{
    SamplerChunk *chunk;
    Uint8 *data;
    Uint32 i;
    bool loaded = false;

    data = (Uint8 *)SDL_malloc(chunk_length);
    if (!data) {
        return false;
    }
    if (SDL_ReadIO(io, data, chunk_length) != chunk_length) {
        SDL_SetError("Couldn't read %" SDL_PRIu32 " bytes from WAV file", chunk_length);
        SDL_free(data);
        return false;
    }
    chunk = (SamplerChunk *)data;

    for (i = 0; i < SDL_Swap32LE(chunk->sample_loops); ++i) {
        const Uint32 LOOP_TYPE_FORWARD = 0;
        const Uint32 loop_type = SDL_Swap32LE(chunk->loops[i].type);
        if (loop_type == LOOP_TYPE_FORWARD) {
            AddLoopPoint(adata, SDL_Swap32LE(chunk->loops[i].play_count), SDL_Swap32LE(chunk->loops[i].start), SDL_Swap32LE(chunk->loops[i].end) + 1);  // +1 because the end field is inclusive.
        }
    }

    loaded = true;
    SDL_free(data);

    // !!! FIXME: sort loops so they go from start to finish.
    // !!! FIXME: eliminate loops that overlap.
    // !!! FIXME: eliminate loops that go past EOF.

    return loaded;
}

static bool CheckWAVMetadataField(const char *wantedtag, const char *propname, SDL_PropertiesID props, size_t *i, Uint32 chunk_length, Uint8 *data)
{
    SDL_assert(SDL_strlen(wantedtag) == 4);

    const char *tag = (const char *) (data + *i);
    if (SDL_strncmp(tag, wantedtag, 4) != 0) {
        return false;
    }

    Uint32 len = 0;
    char *field = NULL;
    *i += 4;
    len = SDL_Swap32LE(*((Uint32 *)(data + *i)));  // LIST
    if (len > chunk_length) {
        *i -= 4;  // move back so we can resync.
        return false; // Do nothing due to broken length
    }
    *i += 4;
    field = (char *)SDL_calloc(1, len + 1);
    if (!field) {  // can't set it, but move on as if we did.
        *i += len;
        return true;
    }
    SDL_strlcpy(field, (char *)(data + *i), len);
    *i += len;

    char key[64];
    SDL_snprintf(key, sizeof (key), "SDL_mixer.metadata.wavLIST.%s", wantedtag);
    SDL_SetStringProperty(props, key, field);
    if (!SDL_HasProperty(props, propname)) {
        SDL_SetStringProperty(props, propname, field);
    }

    SDL_free(field);
    return true;
}

static bool ParseLIST(WAV_AudioData *adata, SDL_IOStream *io, SDL_PropertiesID props, Uint32 chunk_length)
{
    Uint8 *data = (Uint8 *)SDL_malloc(chunk_length);
    if (!data) {
        return false;
    }

    if (SDL_ReadIO(io, data, chunk_length) != chunk_length) {
        SDL_free(data);
        return SDL_SetError("Couldn't read %" SDL_PRIu32 " bytes from WAV file", chunk_length);
    }

    if (SDL_strncmp((const char *)data, "INFO", 4) == 0) {
        for (size_t i = 4; i < chunk_length - 4;) {
            if (CheckWAVMetadataField("INAM", MIX_PROP_METADATA_TITLE_STRING, props, &i, chunk_length, data)) {
                continue;
            } else if (CheckWAVMetadataField("IART", MIX_PROP_METADATA_ARTIST_STRING, props, &i, chunk_length, data)) {
                continue;
            } else if (CheckWAVMetadataField("IALB", MIX_PROP_METADATA_ALBUM_STRING, props, &i, chunk_length, data)) {
                continue;
            } else if (CheckWAVMetadataField("BCPR", MIX_PROP_METADATA_COPYRIGHT_STRING, props, &i, chunk_length, data)) {
                continue;
            }
            i++;
        }
    }

    SDL_free(data);

    return true;
}

static bool ParseWAVID3(SDL_IOStream *io, SDL_PropertiesID props, Uint32 chunk_length)
{
    MIX_IoClamp clamp;
    SDL_IOStream *ioclamp = MIX_OpenIoClamp(&clamp, io);
    if (!ioclamp) {
        return false;
    }
    clamp.length = (Sint64) chunk_length;
    MIX_ReadMetadataTags(ioclamp, props, &clamp);
    SDL_CloseIO(ioclamp);
    return true;
}

static void CalcSeekBlockSeek(const WAV_AudioData *adata, Uint32 actual_frame, WAVSeekBlock *seekblock)
{
    if (IsADPCM(adata->encoding)) {
        seekblock->seek_position = (Sint64) (adata->start + ((actual_frame / adata->adpcm_info.samplesperblock) * adata->adpcm_info.blocksize));
    } else {
        seekblock->seek_position = adata->start + (actual_frame * adata->framesize);
    }
}

static bool BuildSeekBlocks(WAV_AudioData *adata)
{
    const Sint64 all_bytes_in_file = adata->stop - adata->start;

    const unsigned int numloops = adata->numloops;
    if (numloops == 0) {
        WAVSeekBlock *seekblocks = (WAVSeekBlock *) SDL_calloc(1, sizeof (*seekblocks));
        if (!seekblocks) {
            return false;
        }
        adata->seekblocks = seekblocks;
        adata->num_seekblocks = 1;

        seekblocks->frame_start = 0;
        seekblocks->iterations = 1;
        seekblocks->seek_position = adata->start;

        if (IsADPCM(adata->encoding)) {
            // if for some reason the final block isn't completely present, this number might be wrong; we'll presumably decode to the real EOF later.
            seekblocks->num_frames = (all_bytes_in_file / adata->adpcm_info.blocksize) * adata->adpcm_info.samplesperblock;
        } else {
            seekblocks->num_frames = all_bytes_in_file / adata->framesize;
        }
    } else {
        const unsigned int num_seekblocks = (numloops * 2) + 1;
        WAVSeekBlock *seekblocks = (WAVSeekBlock *) SDL_calloc(num_seekblocks, sizeof (*seekblocks));
        if (!seekblocks) {
            return false;
        }
        adata->seekblocks = seekblocks;
        adata->num_seekblocks = num_seekblocks;

        Sint64 current_frame = 0;

        // first seekblock is start of audio data, before any loop.
        seekblocks->frame_start = current_frame;
        seekblocks->num_frames = adata->loops[0].start;
        seekblocks->iterations = 1;
        CalcSeekBlockSeek(adata, 0, seekblocks);

        current_frame += seekblocks->num_frames;
        seekblocks++;

        for (unsigned int i = 0; i < numloops; i++) {
            // space covered by a loop...
            const WAVLoopPoint *loop = &adata->loops[i];
            seekblocks->frame_start = current_frame;
            seekblocks->num_frames = loop->stop - loop->start;
            CalcSeekBlockSeek(adata, loop->start, seekblocks);

            if (loop->iterations == 0) {  // it's an infinite loop!
                seekblocks->iterations = -1;
                current_frame += seekblocks->num_frames;
            } else {
                seekblocks->iterations = loop->iterations;
                current_frame += seekblocks->num_frames * loop->iterations;
            }

            // space covered between loops (or after last loop to EOF)...
            seekblocks++;
            seekblocks->frame_start = current_frame;
            if (i < (numloops - 1)) {
                seekblocks->num_frames = adata->loops[i + 1].start - loop->stop;
            } else {
                if (IsADPCM(adata->encoding)) {
                    // this is kinda wordy, but I wanted to reason through all the math.
                    const Sint64 loop_stop_frame = (Sint64) loop->stop;
                    const Sint64 frames_per_block = (Sint64) adata->adpcm_info.samplesperblock;
                    const Sint64 stop_frame_block = loop_stop_frame / frames_per_block;
                    const Sint64 offset_into_stop_block = loop_stop_frame % frames_per_block;
                    const Sint64 frames_left_in_stop_block = frames_per_block - offset_into_stop_block;
                    const Sint64 all_blocks_in_file = (adata->stop - adata->start) / frames_per_block;
                    const Sint64 blocks_left_after_stop_block = all_blocks_in_file - stop_frame_block;
                    seekblocks->num_frames = frames_left_in_stop_block + (blocks_left_after_stop_block * frames_per_block);
                } else {
                    const Sint64 all_frames_in_file = all_bytes_in_file / adata->framesize;
                    seekblocks->num_frames = all_frames_in_file - ((Sint64) loop->stop);
                }
            }

            seekblocks->iterations = 1;
            CalcSeekBlockSeek(adata, loop->stop, seekblocks);

            current_frame += seekblocks->num_frames;
            seekblocks++;
        }
    }

    #if 0
    for (unsigned int i = 0; i < adata->num_seekblocks; i++) {
        const WAVSeekBlock *seekblock = &adata->seekblocks[i];
        SDL_Log("SEEK BLOCK #%u: frame_start=%d, num_frames=%d, iterations=%d, seek_position=%d",
                i,
                (int) seekblock->frame_start,
                (int) seekblock->num_frames,
                (int) seekblock->iterations,
                (int) seekblock->seek_position);
    }
    #endif

    return true;
}

static bool WAV_init_audio_internal(WAV_AudioData *adata, SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props)
{
    Sint64 flen = SDL_GetIOSize(io);
    bool found_FMT = false;
    bool found_DATA = false;

    // Check the magic header
    Uint32 wavelen, WAVEmagic;
    if (!SDL_ReadU32LE(io, &WAVEmagic) || !SDL_ReadU32LE(io, &wavelen)) {
        return false;
    } else if (((Sint64) wavelen) > flen) {
        return SDL_SetError("Corrupt WAV file (wavlength goes past EOF)");
    }

    flen = wavelen;   // clamp the believed file length, in case there's something appended to the WAV file.

    if (WAVEmagic == RIFF) {
        // there's a 4-byte "form type" that must be WAVE, and then the first chunk follows directly after.
        if (!SDL_ReadU32LE(io, &WAVEmagic)) {
            return false;
        }
    }

    if (WAVEmagic != WAVE) {
        return SDL_SetError("Not a WAV file");
    }

    // Read the chunks
    while (true) {
        Uint32 chunk_type, chunk_length;
        if (!SDL_ReadU32LE(io, &chunk_type) || !SDL_ReadU32LE(io, &chunk_length)) {
            if (SDL_GetIOStatus(io) == SDL_IO_STATUS_EOF) {
                break;
            }
            return false;
        }

        const Sint64 chunk_start_position = SDL_TellIO(io);
        if (chunk_start_position < 0) {
            return false;
        }

        if (chunk_length == 0) {
            break;
        } else if (((chunk_start_position + (Sint64)chunk_length) - (Sint64)(sizeof(Uint32) * 2)) > flen) {
            return SDL_SetError("Corrupt WAV file (chunk goes past EOF)");
        }

        bool chunk_okay = true;
        switch (chunk_type) {
        case FMT:
            found_FMT = true;
            chunk_okay = ParseFMT(adata, io, spec, chunk_length);
            break;
        case DATA:
            found_DATA = true;
            chunk_okay = ParseDATA(adata, io, chunk_length);
            break;
        case SMPL:
            chunk_okay = ParseSMPL(adata, io, chunk_length);
            break;
        case LIST:
            chunk_okay = ParseLIST(adata, io, props, chunk_length);
            break;
        case ID3x:
        case ID3X:
            chunk_okay = ParseWAVID3(io, props, chunk_length);
            break;
        default:
            // unknown or unsupported chunk, we'll just skip it.
            break;
        }

        if (!chunk_okay) {
            return false;
        }

        // move to start of next chunk.
        Sint64 next_chunk = chunk_start_position + chunk_length;
        // RIFF chunks have a 2-byte alignment. Skip padding byte.
        if (chunk_length & 1) {
            next_chunk++;
        }

        if (next_chunk >= flen) {
            break;  // we're done.
        } else if (SDL_SeekIO(io, next_chunk, SDL_IO_SEEK_SET) < 0) {
            return false;
        }
    }

    if (!found_FMT) {
        return SDL_SetError("Bad WAV file (no FMT chunk)");
    } else if (!found_DATA) {
        return SDL_SetError("Bad WAV file (no DATA chunk)");
    }

    if (!BuildSeekBlocks(adata)) {
        return false;
    }

    return true;
}

static void SDLCALL WAV_quit_audio(void *audio_userdata);

static bool SDLCALL WAV_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    // Quick rejection before we allocate anything.
    Uint32 WAVEmagic;
    if (!SDL_ReadU32LE(io, &WAVEmagic)) {
        return false;
    } else if ((WAVEmagic != WAVE) && (WAVEmagic != RIFF)) {
        return SDL_SetError("WAV: not a wav file");
    } else if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {
        return false;
    }

    WAV_AudioData *adata = (WAV_AudioData *) SDL_calloc(1, sizeof (*adata));
    if (!adata) {
        return false;
    }

    const bool rc = WAV_init_audio_internal(adata, io, spec, props);
    if (!rc) {
        WAV_quit_audio(adata);
        return false;
    }

    Sint64 num_frames = 0;

    // figure out if loops increase play length...
    for (unsigned int i = 0; i < adata->num_seekblocks; i++) {
        const WAVSeekBlock *seekblock = &adata->seekblocks[i];
        if (seekblock->iterations < 0) {  // infinite loop
            num_frames = MIX_DURATION_INFINITE;
            break;
        }
        num_frames += seekblock->num_frames * seekblock->iterations;
    }

    *duration_frames = num_frames;
    *audio_userdata = adata;

    return true;
}

static int SpeakerBitToSDLChannelIndex(int channels, Uint32 newbit)
{
    switch (channels) {
        case 1:
            return 0;  // always speaker 0, I dunno.

        case 2:
            switch (newbit) {
                case WAV_SPEAKER_FRONT_LEFT: return 0;
                case WAV_SPEAKER_FRONT_RIGHT: return 1;
                default: break;
            }
            return -1;

        case 3:
            switch (newbit) {
                case WAV_SPEAKER_FRONT_LEFT: return 0;
                case WAV_SPEAKER_FRONT_RIGHT: return 1;
                case WAV_SPEAKER_LOW_FREQUENCY: return 2;
                default: break;
            }
            return -1;

        case 4:
            switch (newbit) {
                case WAV_SPEAKER_FRONT_LEFT: return 0;
                case WAV_SPEAKER_FRONT_RIGHT: return 1;
                case WAV_SPEAKER_BACK_LEFT: return 2;
                case WAV_SPEAKER_BACK_RIGHT: return 3;
                default: break;
            }
            return -1;

        case 5:
            switch (newbit) {
                case WAV_SPEAKER_FRONT_LEFT: return 0;
                case WAV_SPEAKER_FRONT_RIGHT: return 1;
                case WAV_SPEAKER_LOW_FREQUENCY: return 2;
                case WAV_SPEAKER_BACK_LEFT: return 3;
                case WAV_SPEAKER_BACK_RIGHT: return 4;
                default: break;
            }
            return -1;

        case 6:
            switch (newbit) {
                case WAV_SPEAKER_FRONT_LEFT: return 0;
                case WAV_SPEAKER_FRONT_RIGHT: return 1;
                case WAV_SPEAKER_FRONT_CENTER: return 2;
                case WAV_SPEAKER_LOW_FREQUENCY: return 3;
                case WAV_SPEAKER_BACK_LEFT: return 4;
                case WAV_SPEAKER_BACK_RIGHT: return 5;
                default: break;
            }
            return -1;

        case 7:
            switch (newbit) {
                case WAV_SPEAKER_FRONT_LEFT: return 0;
                case WAV_SPEAKER_FRONT_RIGHT: return 1;
                case WAV_SPEAKER_FRONT_CENTER: return 2;
                case WAV_SPEAKER_LOW_FREQUENCY: return 3;
                case WAV_SPEAKER_BACK_CENTER: return 4;
                case WAV_SPEAKER_SIDE_LEFT: return 5;
                case WAV_SPEAKER_SIDE_RIGHT: return 6;
                default: break;
            }
            return -1;

        case 8:
            switch (newbit) {
                case WAV_SPEAKER_FRONT_LEFT: return 0;
                case WAV_SPEAKER_FRONT_RIGHT: return 1;
                case WAV_SPEAKER_FRONT_CENTER: return 2;
                case WAV_SPEAKER_LOW_FREQUENCY: return 3;
                case WAV_SPEAKER_BACK_LEFT: return 4;
                case WAV_SPEAKER_BACK_RIGHT: return 5;
                case WAV_SPEAKER_SIDE_LEFT: return 6;
                case WAV_SPEAKER_SIDE_RIGHT: return 7;
                default: break;
            }
            return -1;

        default: break;
    }

    return -1;
}


static void SetAudioStreamChannelMapForWav(SDL_AudioStream *stream, int channels, Uint32 channelmask)
{
    // WAV files can provide whatever channels they want, setting the provided channels in a bitmask,
    // but they have to provide the data for those channels a specific order.
    // Generally this lines up with SDL, but we need to make sure that unexpected channels in the
    // bitmask are handled.

    if (channels == 1) {
        return; // don't remap mono stream (what would you remap it to!?), in case something reports front-center vs front-left or whatever.
    }

    Uint32 standardmap = StandardSDLWavChannelMask(channels);
    if (channelmask == standardmap) {
        return;  // no remapping needed!
    }

    int chmap[32];
    channels = SDL_min(channels, (int)SDL_arraysize(chmap));

    int current_channel = 0;
    for (int i = 0; i < 32; i++) {
        const Uint32 channelbit = channelmask & (1 << i);

        if (channelbit) {  // wav file uses this speaker?
            int remapping = -1;   // drop it by default.
            if (standardmap & (1 << i)) {  // SDL offers this same speaker.
                remapping = SpeakerBitToSDLChannelIndex(channels, channelbit);
            } else {
                // see if we can bump this channel into something unused that we _do_ have (like, side_left can become back_left, better than nothing).
                Uint32 newbit = 0;  // no remapping by default.
                switch (channelbit) {
                    case WAV_SPEAKER_BACK_LEFT: if (!(standardmap & WAV_SPEAKER_SIDE_LEFT)) { newbit = WAV_SPEAKER_SIDE_LEFT; } break;
                    case WAV_SPEAKER_BACK_RIGHT: if (!(standardmap & WAV_SPEAKER_SIDE_RIGHT)) { newbit = WAV_SPEAKER_SIDE_RIGHT; } break;
                    case WAV_SPEAKER_FRONT_LEFT_OF_CENTER: if (!(standardmap & WAV_SPEAKER_FRONT_LEFT)) { newbit = WAV_SPEAKER_FRONT_LEFT; } break;
                    case WAV_SPEAKER_FRONT_RIGHT_OF_CENTER: if (!(standardmap & WAV_SPEAKER_FRONT_RIGHT)) { newbit = WAV_SPEAKER_FRONT_RIGHT; } break;
                    case WAV_SPEAKER_SIDE_LEFT: if (!(standardmap & WAV_SPEAKER_BACK_LEFT)) { newbit = WAV_SPEAKER_BACK_LEFT; } break;
                    case WAV_SPEAKER_SIDE_RIGHT: if (!(standardmap & WAV_SPEAKER_BACK_RIGHT)) { newbit = WAV_SPEAKER_BACK_RIGHT; } break;
                    default: break;
                }
                if (newbit) {
                    remapping = SpeakerBitToSDLChannelIndex(channels, newbit);
                    standardmap |= newbit;  // mark it as used so we don't try to use it for a later missing speaker.
                }
            }

            chmap[current_channel++] = remapping;
            if (current_channel >= channels) {
                break;  // we got them all.
            }
        }
    }

    while (current_channel < channels) {
        chmap[current_channel++] = -1;  // dump anything that wasn't set up for some reason.
    }

    SDL_SetAudioStreamInputChannelMap(stream, chmap, channels);
}


static bool SDLCALL WAV_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    const WAV_AudioData *adata = (const WAV_AudioData *) audio_userdata;
    WAV_TrackData *tdata = (WAV_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    tdata->adata = adata;
    tdata->io = io;
    tdata->seekblock = &adata->seekblocks[0];
    tdata->current_iteration = 0;
    tdata->current_iteration_frames = 0;
    tdata->must_set_channel_map = true;     // !!! FIXME: why aren't we passing the AudioStream in during init_track, so we don't have to do this check?
    tdata->channels = spec->channels;     // !!! FIXME: why aren't we passing the AudioStream in during init_track, so we don't have to do this check?

    ADPCM_DecoderState *state = &tdata->adpcm_state;
    state->info = &adata->adpcm_info;
    if (IsADPCM(adata->encoding)) {
        if (adata->encoding == MS_ADPCM_CODE) {
            state->cstate = SDL_calloc(state->info->channels, sizeof(MS_ADPCM_ChannelState));
        } else if (adata->encoding == IMA_ADPCM_CODE) {
            state->cstate = SDL_calloc(state->info->channels, sizeof(Sint8));
        } else {
            SDL_assert(!"WAV: Unexpected ADPCM encoding");
        }

        if (!state->cstate) {
            SDL_free(tdata);
            return false;
        }

        state->block.size = adata->adpcm_info.blocksize;
        state->block.data = (Uint8 *)SDL_calloc(1, state->block.size);
        if (!state->block.data) {
            SDL_free(state->cstate);
            SDL_free(tdata);
            return false;
        }

        state->output.size = state->info->samplesperblock * state->info->channels;
        state->output.data = (Sint16 *)SDL_calloc(state->output.size, sizeof(Sint16));
        if (!state->output.data) {
            SDL_free(state->block.data);
            SDL_free(state->cstate);
            SDL_free(tdata);
            return false;
        }
    }

    *track_userdata = tdata;

    return true;
}

static bool SDLCALL WAV_seek(void *track_userdata, Uint64 frame);

static bool SDLCALL WAV_decode(void *track_userdata, SDL_AudioStream *stream)
{
    WAV_TrackData *tdata = (WAV_TrackData *) track_userdata;
    const WAVSeekBlock *seekblock = tdata->seekblock;

    // !!! FIXME: why aren't we passing the AudioStream in during init_track, so we don't have to do this check?
    if (tdata->must_set_channel_map) {
        SetAudioStreamChannelMapForWav(stream, tdata->channels, tdata->adata->channelmask);
        tdata->must_set_channel_map = false;
    }

    // see if we are at the end of a loop, etc.
    SDL_assert(tdata->current_iteration_frames <= seekblock->num_frames);
    while (tdata->current_iteration_frames == seekblock->num_frames) {
        //SDL_Log("Decoded to the end of a seekblock! (iteration %d of %d)", (int) tdata->current_iteration, (int) seekblock->iterations);

        bool should_loop = false;
        if (seekblock->iterations < 0) {  // negative==infinite loop
            tdata->current_iteration = 0;
            should_loop = true;
        } else {
            tdata->current_iteration++;
            SDL_assert(tdata->current_iteration <= seekblock->iterations);
            if (tdata->current_iteration < seekblock->iterations) {
                should_loop = true;
            }
        }

        if (should_loop) {
            const Uint64 nextframe = ((Uint64) seekblock->frame_start) + ( ((Uint64) seekblock->num_frames) * ((Uint64) tdata->current_iteration) );
            const Sint64 current_iteration = tdata->current_iteration;
            //SDL_Log("Moving back to the start of seekblock for next iteration!");
            //SDL_Log("Loop seek to frame=%d byte=%d", (int) nextframe, (int) seekblock->seek_position);
            if (!WAV_seek(tdata, nextframe)) {
                //SDL_Log("SEEK FAILED");
                return false;
            }
            SDL_assert(tdata->seekblock == seekblock);  // should not have changed.
            SDL_assert(tdata->current_iteration == current_iteration);  // should not have changed.
            SDL_assert(tdata->current_iteration_frames == 0);  // should be at start of loop.
        } else {
            //SDL_Log("That was the last iteration, moving to next seekblock!");
            seekblock++;
            if ((seekblock - tdata->adata->seekblocks) >= (Sint64)tdata->adata->num_seekblocks) {  // ran out of blocks! EOF!!
                //SDL_Log("That was the last seekblock, too!");
                tdata->current_iteration--;
                return false;
            }
            tdata->seekblock = seekblock;
            tdata->current_iteration = 0;
        }
        tdata->current_iteration_frames = 0;
    }

    const int decoded_framesize = tdata->adata->decoded_framesize;
    const Uint64 available_bytes = (seekblock->num_frames - tdata->current_iteration_frames) * decoded_framesize;

    // !!! FIXME: looping.
    Uint8 buffer[1024];
    int buflen = (int) sizeof (buffer);
    const int mod = buflen % decoded_framesize;
    if (mod) {
        buflen -= mod;
    }

    buflen = SDL_min(buflen, (Sint64)available_bytes);
    SDL_assert(buflen > 0);  // we should have caught this in the seekblock code.

    const int br = tdata->adata->fetch(tdata, buffer, buflen);  // this will deal with different formats that might need decompression or conversion.
    //SDL_Log("Requested %d bytes, read %d bytes (%d frames)!", buflen, br, br / decoded_framesize);
    if (br <= 0) {
        return false;
    }

    // update framecount, but we'll actually move to the next seekblock if necessary on the next decode, since we definitely have data to return now.
    tdata->current_iteration_frames += (br / decoded_framesize);
    SDL_assert(tdata->current_iteration_frames <= seekblock->num_frames);

    SDL_PutAudioStreamData(stream, buffer, br);
    return true;
}

static bool FindWAVSeekBlock(const WAVSeekBlock *seekblocks, int num_seekblocks, Uint64 ui64frame, const WAVSeekBlock **result)
{
    SDL_assert(seekblocks != NULL);
    SDL_assert(result != NULL);

    const Sint64 frame = (Sint64) ui64frame;
    for (int i = 0; i < num_seekblocks; i++) {
        const WAVSeekBlock *seekblock = &seekblocks[i];
        const Sint64 frame_start = seekblock->frame_start;
        const Sint64 num_frames = seekblock->num_frames;
        if (num_frames < 0) {  // infinite loop?
            *result = seekblock;
            return true;  // it's in here.
        } else if ((frame >= frame_start) && (frame < (frame_start + num_frames))) {   //  is the target!
            *result = seekblock;
            return true;  // it's in here.
        }
    }

    *result = NULL;
    return SDL_SetError("Seek past end of file");
}

static bool SDLCALL WAV_seek(void *track_userdata, Uint64 frame)
{
    WAV_TrackData *tdata = (WAV_TrackData *) track_userdata;
    const WAV_AudioData *adata = tdata->adata;

    // figure out if loops change final position...
    const WAVSeekBlock *seekblock = NULL;
    if (!FindWAVSeekBlock(adata->seekblocks, adata->num_seekblocks, frame, &seekblock)) {
        return false;
    }

    SDL_assert(seekblock != NULL);

    // make frame relative to the start of this seekblock.
    frame -= seekblock->frame_start;

    const Uint64 current_iteration = (seekblock->iterations < 0) ? 0 : (frame / seekblock->num_frames);
    const Uint64 current_iteration_frames = (frame % seekblock->num_frames);

    SDL_assert((seekblock->iterations < 0) || (current_iteration < (Uint64) seekblock->iterations));

    // Deal with loop iterations, offset by the modulus of total frames in the loop; frame_start should have already
    //  dealt with iterations, so this shouldn't matter how many iterations there are or if it's an infinite loop.
    frame = current_iteration_frames;

    if (IsADPCM(adata->encoding)) {
        const Sint64 dest_offset = ((Sint64)(frame / adata->adpcm_info.samplesperblock)) * adata->adpcm_info.blocksize;  // the start of the correct ADPCM block within this seekblock.
        const Sint64 destpos = seekblock->seek_position + dest_offset;  // seek_position is aligned to the start of an ADPCM block.
        SDL_assert(destpos >= adata->start);  // FindWAVSeekBlock should have made sure we're in the range.
        SDL_assert(destpos <= adata->stop);
        if (SDL_SeekIO(tdata->io, destpos, SDL_IO_SEEK_SET) < 0) {
            return false;
        }

        tdata->adpcm_state.output.pos = tdata->adpcm_state.output.read = 0;  // reset this for the new block.

        // We're at the start of the right ADPCM block now; decode and throw away until we hit the exact frame we want to seek to.
        int remainder = ((int)(frame % adata->adpcm_info.samplesperblock)) * adata->decoded_framesize;
        while (remainder > 0) {
            Uint8 buffer[1024];
            const int br = adata->fetch(tdata, buffer, SDL_min(remainder, (int)sizeof(buffer)));
            if (br <= 0) {
                return false;
            }
            remainder -= br;
        }
    } else {
        const Sint64 dest_offset = (Sint64)frame * adata->framesize;
        const Sint64 destpos = seekblock->seek_position + dest_offset;  // seek_position is aligned to start of a sample frame.
        SDL_assert(destpos >= adata->start);  // FindWAVSeekBlock should have made sure we're in the range.
        SDL_assert(destpos <= adata->stop);
        if (SDL_SeekIO(tdata->io, destpos, SDL_IO_SEEK_SET) < 0) {
            return false;
        }
    }

    tdata->seekblock = seekblock;
    tdata->current_iteration = current_iteration;
    tdata->current_iteration_frames = current_iteration_frames;

    return true;
}

static void SDLCALL WAV_quit_track(void *track_userdata)
{
    WAV_TrackData *tdata = (WAV_TrackData *) track_userdata;
    ADPCM_StateCleanup(&tdata->adpcm_state);
    SDL_free(tdata);
}

static void SDLCALL WAV_quit_audio(void *audio_userdata)
{
    WAV_AudioData *adata = (WAV_AudioData *) audio_userdata;
    ADPCM_InfoCleanup(&adata->adpcm_info);
    SDL_free(adata->seekblocks);
    SDL_free(adata->loops);
    SDL_free(adata);
}

const MIX_Decoder MIX_Decoder_WAV = {
    "WAV",
    NULL,  // init
    WAV_init_audio,
    WAV_init_track,
    WAV_decode,
    WAV_seek,
    NULL,  // jump_to_order
    WAV_quit_track,
    WAV_quit_audio,
    NULL  // quit
};

#endif
