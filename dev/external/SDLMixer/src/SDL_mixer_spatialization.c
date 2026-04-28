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

#include "SDL_mixer_internal.h"

// Most of this code was originally from MojoAL (https://github.com/icculus/mojoAL),
// either written by me under the zlib license, or offered by others in the public domain.

#define FIXME(what)

// VBAP code originally from https://github.com/drbafflegab/vbap/ ... CC0 license (public domain).

static SDL_INLINE float MIX_VBAP2D_division_to_angle(int const division)
{
    return (float)division * (2.0f * SDL_PI_F) / (float)MIX_VBAP2D_RESOLUTION;
}

static SDL_INLINE int MIX_VBAP2D_angle_to_span(float const angle)
{
    return (int)SDL_floorf(angle * (float)MIX_VBAP2D_RESOLUTION / (2.0f * SDL_PI_F));
}

static SDL_INLINE bool MIX_VBAP2D_contains(int division, int last_division, int next_division)
{
    if (last_division < next_division) {
        return last_division <= division && division < next_division;
    } else {
        const bool cond_a = 0 <= division && division < next_division;
        const bool cond_b = last_division <= division && division < MIX_VBAP2D_RESOLUTION;
        return cond_a || cond_b;
    }
}

static SDL_INLINE void MIX_VBAP2D_unpack_speaker_pair(int speaker_pair, int speaker_count, int *speakers)
{
    speakers[0] = (speaker_pair == 0 ? speaker_count : speaker_pair) - 1;
    speakers[1] = speaker_pair;
}

typedef struct MIX_VBAP2D_SpeakerPosition
{
    const Uint8 division;  // this is in degrees--positive to the left--divided by the resolution. RESOLUTION MUST BE AT LEAST 2 TO FIT IN UINT8!
    const Uint8 sdl_channel;  // the channel in SDL's layout (in stereo: {left=0, right=1}...etc).
} MIX_VBAP2D_SpeakerPosition;

typedef struct MIX_VBAP2D_SpeakerLayout
{
    const MIX_VBAP2D_SpeakerPosition *positions;
    const int lfe_channel;
} MIX_VBAP2D_SpeakerLayout;

// these have to go from smallest to largest angle, I think...
#define P(angle) ( (Uint8) ((angle / 360.0) * MIX_VBAP2D_RESOLUTION ) )
static const MIX_VBAP2D_SpeakerPosition MIX_VBAP2D_SpeakerPositions_quad[] = { { P(45), 1 }, { P(135), 0 }, { P(225), 2 }, { P(315), 3 } };
static const MIX_VBAP2D_SpeakerPosition MIX_VBAP2D_SpeakerPositions_4_1[] = { { P(45), 1 }, { P(135), 0 }, { P(225), 3 }, { P(315), 4 } };
static const MIX_VBAP2D_SpeakerPosition MIX_VBAP2D_SpeakerPositions_5_1[] = { { P(60), 1 }, { P(90), 2 }, { P(120), 0 }, { P(240), 4 }, { P(300), 5 } };
static const MIX_VBAP2D_SpeakerPosition MIX_VBAP2D_SpeakerPositions_6_1[] = { { P(60), 1 }, { P(90), 2 }, { P(120), 0 }, { P(190), 5 }, { P(270), 4 }, { P(350), 6 } };
static const MIX_VBAP2D_SpeakerPosition MIX_VBAP2D_SpeakerPositions_7_1[] = { { P(0), 7 }, { P(60), 1 }, { P(90), 2 }, { P(120), 0 }, { P(200), 6 }, { P(240), 4 }, { P(300), 5 } };
static const MIX_VBAP2D_SpeakerLayout MIX_VBAP2D_SpeakerLayouts[MIX_VBAP2D_MAX_SPEAKER_COUNT-3] = {  // -3 to skip mono/stereo/2.1
    { MIX_VBAP2D_SpeakerPositions_quad, -1 },
    { MIX_VBAP2D_SpeakerPositions_4_1, 2 },
    { MIX_VBAP2D_SpeakerPositions_5_1, 3 },
    { MIX_VBAP2D_SpeakerPositions_6_1, 3 },
    { MIX_VBAP2D_SpeakerPositions_7_1, 3 }
};
#undef P

void MIX_VBAP2D_Init(MIX_VBAP2D *vbap2d, int speaker_count)
{
    SDL_assert(speaker_count > 0);
    SDL_assert(speaker_count <= MIX_VBAP2D_MAX_SPEAKER_COUNT);
    SDL_assert(MIX_VBAP2D_RESOLUTION <= MIX_VBAP2D_MAX_RESOLUTION);

    vbap2d->speaker_count = speaker_count;

    if (speaker_count < 4) {
        return;  // no VBAP for mono, stereo, or 2.1.
    }

    const MIX_VBAP2D_SpeakerLayout *speaker_layout = &MIX_VBAP2D_SpeakerLayouts[speaker_count - 4];  // offset to zero, skip mono/stereo/2.1
    const MIX_VBAP2D_SpeakerPosition *speaker_positions = speaker_layout->positions;

    if (speaker_layout->lfe_channel >= 0) {
        speaker_count--;  // for our purposes, collapse out the subwoofer channel
    }

    MIX_VBAP2D_Bucket *buckets = vbap2d->buckets;
    for (int division = 0, speaker_pair = 0; division < MIX_VBAP2D_RESOLUTION; division++) {
        int speakers[2];
        MIX_VBAP2D_unpack_speaker_pair(speaker_pair, speaker_count, speakers);
        const int last_division = speaker_positions[speakers[0]].division;
        const int next_division = speaker_positions[speakers[1]].division;

        if (!MIX_VBAP2D_contains(division, last_division, next_division)) {
            speaker_pair = (speaker_pair + 1) % speaker_count;
        }

        buckets[division].speaker_pair = speaker_pair;
    }

    MIX_VBAP2D_Matrix *matrices = vbap2d->matrices;
    for (int speaker_pair = 0; speaker_pair < speaker_count; speaker_pair++) {
        int speakers[2];
        MIX_VBAP2D_unpack_speaker_pair(speaker_pair, speaker_count, speakers);
        const int last_division = speaker_positions[speakers[0]].division;
        const int next_division = speaker_positions[speakers[1]].division;
        const float last_angle = MIX_VBAP2D_division_to_angle(last_division);
        const float next_angle = MIX_VBAP2D_division_to_angle(next_division);
        const float a00 = SDL_cosf(last_angle), a01 = SDL_cosf(next_angle);
        const float a10 = SDL_sinf(last_angle), a11 = SDL_sinf(next_angle);
        const float det = 1.0f / (a00 * a11 - a01 * a10);

        matrices[speaker_pair].a00 = +a11 * det;
        matrices[speaker_pair].a01 = -a01 * det;
        matrices[speaker_pair].a10 = -a10 * det;
        matrices[speaker_pair].a11 = +a00 * det;
    }
}

static void MIX_VBAP2D_CalculateGains(const MIX_VBAP2D *vbap2d, float source_angle, float *gains, int *speakers)
{
    int speaker_count = vbap2d->speaker_count;
    SDL_assert(speaker_count >= 4);

    const MIX_VBAP2D_SpeakerLayout *speaker_layout = &MIX_VBAP2D_SpeakerLayouts[speaker_count - 4];  // offset to zero, skip mono/stereo/2.1

    if (speaker_layout->lfe_channel >= 0) {
        speaker_count--;  // for our purposes, collapse out the subwoofer channel
    }

    // shift so angle 0 is due east instead of due north, and normalize it to the 0 to 2pi range.
    source_angle += SDL_PI_F / 2.0f;

    while (source_angle < 0.0f) {
        source_angle += 2.0f * SDL_PI_F;
    }
    while (source_angle > (2.0f * SDL_PI_F)) {
        source_angle -= 2.0f * SDL_PI_F;
    }

    const float source_x = SDL_cosf(source_angle);
    const float source_y = SDL_sinf(source_angle);
    const int span = MIX_VBAP2D_angle_to_span(source_angle);
    const int speaker_pair = vbap2d->buckets[span].speaker_pair;
    int vbap_speakers[2];

    MIX_VBAP2D_unpack_speaker_pair(speaker_pair, speaker_count, vbap_speakers);

    const MIX_VBAP2D_Matrix *matrix = &vbap2d->matrices[speaker_pair];
    const float gain_a = source_x * matrix->a00 + source_y * matrix->a01;
    const float gain_b = source_x * matrix->a10 + source_y * matrix->a11;

    const float scale = 1.0f / SDL_sqrtf(gain_a * gain_a + gain_b * gain_b);

    const float gain_a_normalized = gain_a * scale;
    const float gain_b_normalized = gain_b * scale;

    speakers[0] = speaker_layout->positions[vbap_speakers[0]].sdl_channel;
    speakers[1] = speaker_layout->positions[vbap_speakers[1]].sdl_channel;
    gains[0] = gain_a_normalized;
    gains[1] = gain_b_normalized;
}

// end VBAP code.


// All the 3D math here is way overcommented because I HAVE NO IDEA WHAT I'M
//  DOING and had to research the hell out of what are probably pretty simple
//  concepts. Pay attention in math class, kids.

// The scalar versions have explanitory comments and links. The SIMD versions don't.

static float calculate_distance_attenuation(const float distance)
{
    // we use the OpenAL default distance model (AL_INVERSE_DISTANCE_CLAMPED), with a reference distance and rolloff factor of 1.0f (the defaults).
    // this collapses a ton of work out of this code that MojoAL had to do.
    return 1.0f / (1.0f + (SDL_max(distance, 1.0f) - 1.0f));
}

static const float SDL_ALIGNED(16) listener_at[4] = { 0.0f, 0.0, -1.0f, 0.0f };  // default "at" for OpenAL listener orientation matrix.
static const float SDL_ALIGNED(16) listener_up[4] = { 0.0f, 1.0, 0.0f, 0.0f };  // default "up" for OpenAL listener orientation matrix.


#if SDL_MIXER_NEED_SCALAR_FALLBACK
//  XYZZY!! https://en.wikipedia.org/wiki/Cross_product#Mnemonic
//
//  Calculates cross product. https://en.wikipedia.org/wiki/Cross_product
//  Basically takes two vectors and gives you a vector that's perpendicular
//  to both.
static void xyzzy(float *v, const float *a, const float *b)
{
    v[0] = (a[1] * b[2]) - (a[2] * b[1]);
    v[1] = (a[2] * b[0]) - (a[0] * b[2]);
    v[2] = (a[0] * b[1]) - (a[1] * b[0]);
}

// calculate dot product (multiply each element of two vectors, sum them)
static float dotproduct(const float *a, const float *b)
{
    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

// calculate distance ("magnitude") in 3D space:
//  https://math.stackexchange.com/questions/42640/calculate-distance-in-3d-space
//  assumes vector starts at (0,0,0).
static float magnitude(const float *v)
{
    // technically, the inital part on this is just a dot product of itself.
    return SDL_sqrtf((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
}

static void calculate_distance_attenuation_and_angle_scalar(const float *position, float *_gain, float *_radians)
{
    // Remove upwards component so it lies completely within the horizontal plane.
    const float a = dotproduct(position, listener_up);
    float V[3];
    V[0] = position[0] - (a * listener_up[0]);
    V[1] = position[1] - (a * listener_up[1]);
    V[2] = position[2] - (a * listener_up[2]);

    // Calculate angle
    const float mags = magnitude(listener_at) * magnitude(V);
    float radians;
    if (mags == 0.0f) {
        radians = 0.0f;
    } else {
        const float cosangle = dotproduct(listener_at, V) / mags;
        radians = SDL_acosf(SDL_clamp(cosangle, -1.0f, 1.0f));
    }

    // Get "right" vector
    float R[3];
    xyzzy(R, listener_at, listener_up);

    // make it negative to the left, positive to the right.
    if (dotproduct(R, V) < 0.0f) {
        radians = -radians;
    }

    *_gain = calculate_distance_attenuation(magnitude(position));
    *_radians = radians;
}
#endif


#if defined(SDL_SSE_INTRINSICS)
static __m128 SDL_TARGETING("sse") xyzzy_sse(const __m128 a, const __m128 b)
{
    // http://fastcpp.blogspot.com/2011/04/vector-cross-product-using-sse-code.html
    //    this is the "three shuffle" version in the comments, plus the variables swapped around for handedness in the later comment.
    const __m128 v = _mm_sub_ps(
        _mm_mul_ps(a, _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1))),
        _mm_mul_ps(b, _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)))
    );
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 2, 1));
}

static float SDL_TARGETING("sse") dotproduct_sse(const __m128 a, const __m128 b)
{
    const __m128 prod = _mm_mul_ps(a, b);
    const __m128 sum1 = _mm_add_ps(prod, _mm_shuffle_ps(prod, prod, _MM_SHUFFLE(1, 0, 3, 2)));
    const __m128 sum2 = _mm_add_ps(sum1, _mm_shuffle_ps(sum1, sum1, _MM_SHUFFLE(2, 2, 0, 0)));
    FIXME("this can use _mm_hadd_ps in SSE3, or _mm_dp_ps in SSE4.1");
    return _mm_cvtss_f32(_mm_shuffle_ps(sum2, sum2, _MM_SHUFFLE(3, 3, 3, 3)));
}

static float SDL_TARGETING("sse") magnitude_sse(const __m128 v)
{
    return SDL_sqrtf(dotproduct_sse(v, v));
}

static void SDL_TARGETING("sse") calculate_distance_attenuation_and_angle_sse(const float *position, float *_gain, float *_radians)
{
    const __m128 position_sse = _mm_load_ps(position);
    const __m128 at_sse = _mm_load_ps(listener_at);
    const __m128 up_sse = _mm_load_ps(listener_up);

    const float a = dotproduct_sse(position_sse, up_sse);
    const __m128 V_sse = _mm_sub_ps(position_sse, _mm_mul_ps(_mm_set1_ps(a), up_sse));
    const float mags = magnitude_sse(at_sse) * magnitude_sse(V_sse);
    float radians;
    if (mags == 0.0f) {
        radians = 0.0f;
    } else {
        const float cosangle = dotproduct_sse(at_sse, V_sse) / mags;
        radians = SDL_acosf(SDL_clamp(cosangle, -1.0f, 1.0f));
    }

    const __m128 R_sse = xyzzy_sse(at_sse, up_sse);
    if (dotproduct_sse(R_sse, V_sse) < 0.0f) {
        radians = -radians;
    }

    *_gain = calculate_distance_attenuation(magnitude_sse(position_sse));
    *_radians = radians;
}
#endif

#if defined(SDL_NEON_INTRINSICS)
// Some versions of arm_neon.h don't have vcopyq_laneq_f32() available
#ifndef vcopyq_laneq_f32
#define vcopyq_laneq_f32(a1, __b1, c1, __d1) __extension__ ({ \
  float32x4_t __a1 = (a1); float32x4_t __c1 = (c1); \
  float32_t __c2 = vgetq_lane_f32(__c1, __d1); \
  vsetq_lane_f32(__c2, __a1, __b1); })
#endif

static float32x4_t xyzzy_neon(const float32x4_t a, const float32x4_t b)
{
    const float32x4_t a_yzx = vcopyq_laneq_f32(vextq_f32(a, a, 1), 2, a, 0);
    const float32x4_t b_yzx = vcopyq_laneq_f32(vextq_f32(b, b, 1), 2, b, 0);
    const float32x4_t c = vsubq_f32(vmulq_f32(a, b_yzx), vmulq_f32(b, a_yzx));
    const float32x4_t r = vcopyq_laneq_f32(vextq_f32(c, c, 1), 2, c, 0);
    return vsetq_lane_f32(0, r, 3);
}

static float dotproduct_neon(const float32x4_t a, const float32x4_t b)
{
    const float32x4_t prod = vmulq_f32(a, b);
    const float32x4_t sum1 = vaddq_f32(prod, vrev64q_f32(prod));
    const float32x4_t sum2 = vaddq_f32(sum1, vcombine_f32(vget_high_f32(sum1), vget_low_f32(sum1)));
    return vgetq_lane_f32(sum2, 3);
}

static float magnitude_neon(const float32x4_t v)
{
    return SDL_sqrtf(dotproduct_neon(v, v));
}

static void calculate_distance_attenuation_and_angle_neon(const float *position, float *_gain, float *_radians)
{
    const float32x4_t position_neon = vld1q_f32(position);
    const float32x4_t at_neon = vld1q_f32(listener_at);
    const float32x4_t up_neon = vld1q_f32(listener_up);

    const float a = dotproduct_neon(position_neon, up_neon);
    const float32x4_t V_neon = vsubq_f32(position_neon, vmulq_f32(vdupq_n_f32(a), up_neon));
    const float mags = magnitude_neon(at_neon) * magnitude_neon(V_neon);
    float radians;
    if (mags == 0.0f) {
        radians = 0.0f;
    } else {
        const float cosangle = dotproduct_neon(at_neon, V_neon) / mags;
        radians = SDL_acosf(SDL_clamp(cosangle, -1.0f, 1.0f));
    }

    const float32x4_t R_neon = xyzzy_neon(at_neon, up_neon);
    if (dotproduct_neon(R_neon, V_neon) < 0.0f) {
        radians = -radians;
    }

    *_gain = calculate_distance_attenuation(magnitude_neon(position_neon));
    *_radians = radians;
}
#endif

static void calculate_distance_attenuation_and_angle(const float *position, float *_gain, float *_radians)
{
    SDL_assert( (((size_t) listener_at) % 16) == 0 );  // must be aligned for SIMD access.
    SDL_assert( (((size_t) listener_up) % 16) == 0 );  // must be aligned for SIMD access.

    // this goes through most of the steps the AL spec dictates for gain and distance attenuation...
    #if defined(SDL_SSE_INTRINSICS)
    if (MIX_HasSSE) {
        calculate_distance_attenuation_and_angle_sse(position, _gain, _radians);
    } else
    #elif defined(SDL_NEON_INTRINSICS)
    if (MIX_HasNEON) {
        calculate_distance_attenuation_and_angle_neon(position, _gain, _radians);
    } else
    #endif

    {
    #if SDL_MIXER_NEED_SCALAR_FALLBACK
        calculate_distance_attenuation_and_angle_scalar(position, _gain, _radians);
    #endif
    }
}

// Get the sin(angle) and cos(angle) at the same time. Ideally, with one
//  instruction, like what is offered on the x86.
//  angle is in radians, not degrees.
static void calculate_sincos(const float angle, float *_sin, float *_cos)
{
    // (of course, FSINCOS uses the floating point registers, so we're
    //  currently opting to favor portability by using the SDL_* functions.)
    *_sin = SDL_sinf(angle);
    *_cos = SDL_cosf(angle);
}

void MIX_Spatialize(const MIX_VBAP2D *vbap2d, const float *position, float *panning, int *speakers)
{
    const int output_channels = vbap2d->speaker_count;

    SDL_assert( (((size_t) position) % 16) == 0 );  // must be aligned for SIMD access.
    SDL_assert(output_channels > 0);

    float gain, radians;
    calculate_distance_attenuation_and_angle(position, &gain, &radians);

    if (output_channels == 1) {  // no positioning for mono output, just distance attenuation.
        speakers[0] = speakers[1] = 0;
        panning[0] = gain;
        panning[1] = 0.0f;
    } else if ((output_channels == 2) || (output_channels == 3)) {  // stereo (and 2.1) output uses Constant Power Panning.
        speakers[0] = 0;
        speakers[1] = 1;

        // here comes the Constant Power Panning magic...
        #define SQRT2_DIV2 0.7071067812f  // sqrt(2.0) / 2.0 ...

        // this might be a terrible idea, which is totally my own doing here,
        // but here you go: Constant Power Panning only works from -45 to 45
        // degrees in front of the listener. So we split this into 4 quadrants.

        //   - from -45 to 45: standard panning.
        //   - from 45 to 135: pan full right.
        //   - from 135 to 225: flip angle so it works like standard panning.
        //   - from 225 to -45: pan full left.

        #define RADIANS_45_DEGREES 0.7853981634f
        #define RADIANS_135_DEGREES 2.3561944902f
        if ((radians >= -RADIANS_45_DEGREES) && (radians <= RADIANS_45_DEGREES)) {
            float sine, cosine;
            calculate_sincos(radians, &sine, &cosine);
            panning[0] = (SQRT2_DIV2 * (cosine - sine));
            panning[1] = (SQRT2_DIV2 * (cosine + sine));
        } else if ((radians >= RADIANS_45_DEGREES) && (radians <= RADIANS_135_DEGREES)) {
            panning[0] = 0.0f;
            panning[1] = 1.0f;
        } else if ((radians >= -RADIANS_135_DEGREES) && (radians <= -RADIANS_45_DEGREES)) {
            panning[0] = 1.0f;
            panning[1] = 0.0f;
        } else if (radians < 0.0f) {  // back left
            float sine, cosine;
            calculate_sincos(-(radians + SDL_PI_F), &sine, &cosine);
            panning[0] = (SQRT2_DIV2 * (cosine - sine));
            panning[1] = (SQRT2_DIV2 * (cosine + sine));
        } else { // back right
            float sine, cosine;
            calculate_sincos(-(radians - SDL_PI_F), &sine, &cosine);
            panning[0] = (SQRT2_DIV2 * (cosine - sine));
            panning[1] = (SQRT2_DIV2 * (cosine + sine));
        }

        // apply distance attenuation and gain to positioning.
        panning[0] *= gain;
        panning[1] *= gain;
    } else {  // surround-sound (output_channels >= 4)
        // we're going negative to the _right_ here, at the moment, so negative radians.
        MIX_VBAP2D_CalculateGains(vbap2d, -radians, panning, speakers);

        // apply distance attenuation and gain to positioning.
        panning[0] *= gain;
        panning[1] *= gain;
    }
}

