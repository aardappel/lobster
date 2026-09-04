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

BUILTIN_V(simplex, "pos,octaves,scale,persistence", "F}IFF", "F",
    "returns a simplex noise value [-1..1] given a 2D/3D location, the number of octaves"
    " (try 6), a scale (try 0.01), and persistence from one octave to the next (try 0.5)."
    " This function is the same as calling fast_noise with simplex/fbm flags.")
(StackPtr &sp, VM &vm, Value *pos, iint pos_len, iint octaves, double scale, double persistence) {
    auto state = fast_lite_default_noise_state;
    state.gain = (float)persistence;
    state.frequency = (float)scale;
    state.octaves = (int)octaves;
    auto v = ToVec<float4>(pos, pos_len);
    state.fractal_type = FNL_FRACTAL_FBM;
    switch (pos_len) {
        case 2:
            Push(sp, fnlGetNoise2D(&state, v.x, v.y));
            break;
        case 3:
            Push(sp, fnlGetNoise3D(&state, v.x, v.y, v.z));
            break;
        default:
            vm.BuiltinError("simplex: only 2D/3D supported");
    }
}

BUILTIN_V(simplex_raw, "pos", "F}", "F",
    "returns a simplex noise value [-1..1] given a 2D/3D location")
(StackPtr &sp, VM &vm, Value *pos, iint pos_len) {
    auto state = fast_lite_default_noise_state;
    auto v = ToVec<float4>(pos, pos_len);
    switch (pos_len) {
        case 2:
            Push(sp, fnlGetNoise2D(&state, v.x, v.y));
            break;
        case 3:
            Push(sp, fnlGetNoise3D(&state, v.x, v.y, v.z));
            break;
        default:
            vm.BuiltinError("simplex_raw: only 2D/3D supported");
    }
}

BUILTIN_V(fast_noise, "pos,octaves,scale,persistence,noise_type,fractal_type", "F}IFFII", "F",
    "returns a noise value [-1..1] given a 2D/3Dlocation, the number of octaves"
    " (try 6), a scale (try 0.01), and persistence from one octave to the next (try 0.5)."
    " see noise.lobster for constants for the type params.")
(StackPtr &sp, VM &vm, Value *pos, iint pos_len, iint octaves, double scale, double persistence,
 iint noise_type, iint fractal_type) {
    auto state = fast_lite_default_noise_state;
    state.fractal_type = (fnl_fractal_type)fractal_type;
    state.noise_type = (fnl_noise_type)noise_type;
    state.gain = (float)persistence;
    state.frequency = (float)scale;
    state.octaves = (int)octaves;
    auto v = ToVec<float4>(pos, pos_len);
    switch (pos_len) {
        case 2:
            Push(sp, fnlGetNoise2D(&state, v.x, v.y));
            break;
        case 3:
            Push(sp, fnlGetNoise3D(&state, v.x, v.y, v.z));
            break;
        default:
            vm.BuiltinError("fast_noise: only 2D/3D supported");
    }
}

}  // namespace lobster
