#include "lobster/stdafx.h"

#include "lobster/natreg.h"

#define FNL_IMPL
#include "FastNoiseLite.h"
fnl_state fast_lite_default_noise_state = fnlCreateState();

using namespace lobster;

float SimplexNoise(const int octaves, const float persistence, const float scale, const float3 &v) {
    auto state = fast_lite_default_noise_state;
    state.gain = persistence;
    state.frequency = scale;
    state.octaves = octaves;
    state.fractal_type = FNL_FRACTAL_FBM;
    return fnlGetNoise3D(&state, v.x, v.y, v.z);
}

namespace lobster {

BuiltinGroup noise_builtins;
#define BUILTIN_GROUP noise_builtins
#define BUILTIN_SYM(name) builtin_##name

// The noise builtins exist in a 2D and a 3D form, which the generated code passes the values
// of a struct to in exactly as many stack slots, with no length to go with them.
#define NOISE2D(state, v) fnlGetNoise2D(&state, v.x, v.y)
#define NOISE3D(state, v) fnlGetNoise3D(&state, v.x, v.y, v.z)

#define SIMPLEXW(W) \
    BUILTIN_V_OVERLOAD(simplex_f##W, "simplex", "pos,octaves,scale,persistence", \
        "F}:" #W "IFF", "F", \
        "returns a simplex noise value [-1..1] given a 2D/3D location, the number of octaves" \
        " (try 6), a scale (try 0.01), and persistence from one octave to the next (try 0.5)." \
        " This function is the same as calling fast_noise with simplex/fbm flags.") \
    (StackPtr &sp, VM &, Value *pos, iint octaves, double scale, double persistence) { \
        auto state = fast_lite_default_noise_state; \
        state.gain = (float)persistence; \
        state.frequency = (float)scale; \
        state.octaves = (int)octaves; \
        state.fractal_type = FNL_FRACTAL_FBM; \
        auto v = ToVec<vec<float, W>>(pos, W); \
        Push(sp, NOISE##W##D(state, v)); \
    }
SIMPLEXW(2) SIMPLEXW(3)
#undef SIMPLEXW

#define SIMPLEXRAWW(W) \
    BUILTIN_V_OVERLOAD(simplex_raw_f##W, "simplex_raw", "pos", "F}:" #W, "F", \
        "returns a simplex noise value [-1..1] given a 2D/3D location") \
    (StackPtr &sp, VM &, Value *pos) { \
        auto state = fast_lite_default_noise_state; \
        auto v = ToVec<vec<float, W>>(pos, W); \
        Push(sp, NOISE##W##D(state, v)); \
    }
SIMPLEXRAWW(2) SIMPLEXRAWW(3)
#undef SIMPLEXRAWW

#define FASTNOISEW(W) \
    BUILTIN_V_OVERLOAD(fast_noise_f##W, "fast_noise", \
        "pos,octaves,scale,persistence,noise_type,fractal_type", "F}:" #W "IFFII", "F", \
        "returns a noise value [-1..1] given a 2D/3Dlocation, the number of octaves" \
        " (try 6), a scale (try 0.01), and persistence from one octave to the next (try 0.5)." \
        " see noise.lobster for constants for the type params.") \
    (StackPtr &sp, VM &, Value *pos, iint octaves, double scale, double persistence, \
     iint noise_type, iint fractal_type) { \
        auto state = fast_lite_default_noise_state; \
        state.fractal_type = (fnl_fractal_type)fractal_type; \
        state.noise_type = (fnl_noise_type)noise_type; \
        state.gain = (float)persistence; \
        state.frequency = (float)scale; \
        state.octaves = (int)octaves; \
        auto v = ToVec<vec<float, W>>(pos, W); \
        Push(sp, NOISE##W##D(state, v)); \
    }
FASTNOISEW(2) FASTNOISEW(3)
#undef FASTNOISEW
#undef NOISE2D
#undef NOISE3D

}  // namespace lobster
