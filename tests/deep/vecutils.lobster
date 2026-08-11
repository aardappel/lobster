import std

import vec

// The vec module: swizzles, conversions, loop helpers and direction tables
// built on top of the numeric struct types.

def vecutils_tests():
    suite("vecutils")

    // Shortening: xy relies on struct subtyping (chops fields), the others
    // construct new structs.
    test_eq(xy(float3 { 1.0, 2.0, 3.0 }), float2 { 1.0, 2.0 })
    test_eq(xy(float4 { 1.0, 2.0, 3.0, 4.0 }), float2 { 1.0, 2.0 })
    test_eq(xz(float3 { 1.0, 2.0, 3.0 }), float2 { 1.0, 3.0 })
    test_eq(yz(float3 { 1.0, 2.0, 3.0 }), float2 { 2.0, 3.0 })
    test_eq(xw(float4 { 1.0, 2.0, 3.0, 4.0 }), float2 { 1.0, 4.0 })
    test_eq(yw(float4 { 1.0, 2.0, 3.0, 4.0 }), float2 { 2.0, 4.0 })
    test_eq(zw(float4 { 1.0, 2.0, 3.0, 4.0 }), float2 { 3.0, 4.0 })
    test_eq(xyz(float4 { 1.0, 2.0, 3.0, 4.0 }), float3 { 1.0, 2.0, 3.0 })
    test_eq(xy(int3 { 1, 2, 3 }), int2 { 1, 2 })

    // Lengthening.
    test_eq(int3(int2 { 1, 2 }, 3), int3 { 1, 2, 3 })
    test_eq(float3(float2 { 1.0, 2.0 }, 3.0), float3 { 1.0, 2.0, 3.0 })
    test_eq(int4(int3 { 1, 2, 3 }, 4), int4 { 1, 2, 3, 4 })
    test_eq(float4(float3 { 1.0, 2.0, 3.0 }, 4.0), float4 { 1.0, 2.0, 3.0, 4.0 })

    // To/from vectors.
    test_eq(vec2_v([ 5, 6 ]), int2 { 5, 6 })
    test_eq(vec3_v([ 5.0, 6.0, 7.0 ]), float3 { 5.0, 6.0, 7.0 })
    test_eq(vec4_v([ 1, 2, 3, 4 ]), int4 { 1, 2, 3, 4 })
    test_seq(v_vec2(int2 { 5, 6 }), [ 5, 6 ])
    test_seq(v_vec3(float3 { 1.0, 2.0, 3.0 }), [ 1.0, 2.0, 3.0 ])
    test_seq(v_vec4(int4 { 1, 2, 3, 4 }), [ 1, 2, 3, 4 ])

    // Flips.
    test_eq(yx(int2 { 1, 2 }), int2 { 2, 1 })
    test_eq(zyx(int3 { 1, 2, 3 }), int3 { 3, 2, 1 })
    test_eq(wzyx(int4 { 1, 2, 3, 4 }), int4 { 4, 3, 2, 1 })

    // Direction tables.
    test_eq(cardinal_directions.length, 4)
    test_eq(diagonal_directions.length, 4)
    test_eq(positive_directions.length, 4)
    test_eq(octant_directions.length, 8)
    var dirsum = int2_0
    for(cardinal_directions) d:
        dirsum += d
        test_eq(manhattan(d), 1)
    test_eq(dirsum, int2_0)
    for(diagonal_directions) d:
        test_eq(manhattan(d), 2)

    // Loop helpers: forxy iterates x fastest, foryx y fastest.
    var trace = ""
    forxy(int2 { 2, 2 }) v:
        trace += "{v.x}{v.y} "
    test_eq(trace, "00 10 01 11 ")
    trace = ""
    foryx(int2 { 2, 2 }) v:
        trace += "{v.x}{v.y} "
    test_eq(trace, "00 01 10 11 ")
    let grid = mapxy(int2 { 2, 2 }) v: v.x + v.y * 10
    test_seq(grid, [ [ 0, 1 ], [ 10, 11 ] ])
    var cells = 0
    forxyz(int3 { 2, 3, 4 }):
        cells++
    test_eq(cells, 24)
    let grid3 = mapxyz(int3 { 2, 1, 1 }) v: v.x
    test_seq(grid3, [ [ [ 0, 1 ] ] ])

    // Random helpers are seeded and in range.
    rnd_seed(11)
    let ri2 = rnd_int2(int2 { 3, 5 })
    test_true(ri2.x >= 0 and ri2.x < 3 and ri2.y >= 0 and ri2.y < 5)
    let ri3 = rnd_int3(4)
    test_true(in_range(ri3, int3 { 4, 4, 4 }))
    let rf2 = rnd_float2()
    test_true(rf2.x >= 0.0 and rf2.x < 1.0 and rf2.y >= 0.0 and rf2.y < 1.0)
    test_approx(magnitude(rnd_float2_norm()), 1.0)
    test_approx(magnitude(rnd_float3_norm()), 1.0)

    // rotate2D rotates counter-clockwise, in degrees.
    test_approx(rotate2D(float2_x, 90.0), float2_y)
    test_approx(rotate2D(float2_y, 90.0), -float2_x)
    test_approx(rotate2D(float2 { 3.0, 4.0 }, 360.0), float2 { 3.0, 4.0 })

    // vecfromyawpitch: yaw 0, pitch 0, move 1 looks down +y.
    test_approx(vecfromyawpitch(0.0, 0.0, 1.0, 0.0), float3_y)
    // Strafe only: moves along +x.
    test_approx(vecfromyawpitch(0.0, 0.0, 0.0, 1.0), float3_x)
    // Pitch straight up.
    test_approx(vecfromyawpitch(0.0, 90.0, 1.0, 0.0), float3_z)

vecutils_tests()
