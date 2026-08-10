import std

import vec

// Geometry-flavored builtins that need no engine: matrix.*, splines,
// quaternion lerp, circles_within_range, wave_function_collapse.

def geometry_tests():
    suite("geometry")

    // 4x4 matrices as length-16 float vectors.
    let identity = [ 1.0, 0.0, 0.0, 0.0,
                     0.0, 1.0, 0.0, 0.0,
                     0.0, 0.0, 1.0, 0.0,
                     0.0, 0.0, 0.0, 1.0 ]
    test_seq(matrix.multiply(identity, identity), identity)
    let trans = matrix.translation(float3 { 1.0, 2.0, 3.0 })
    test_eq(trans.length, 16)
    // Translation lives in the last column (column-major storage).
    test_eq(trans[12], 1.0)
    test_eq(trans[13], 2.0)
    test_eq(trans[14], 3.0)
    test_eq(trans[15], 1.0)
    test_seq(matrix.multiply(identity, trans), trans)
    // Rotating by 0 degrees is the identity.
    let rot0 = matrix.rotate_x(sincos(0.0))
    test_seq(rot0, identity)
    test_eq(matrix.rotate_y(sincos(0.0)).length, 16)
    test_eq(matrix.rotate_z(sincos(0.0)).length, 16)
    // Rotation about x by 90 degrees maps y to z.
    let rot90 = matrix.rotate_x(sincos(90.0))
    test_approx(rot90[5], 0.0)   // cos in the y/z block.
    test_approx(rot90[6], 1.0)

    // cardinal_spline: at f == 0 and 1 it passes through the control points.
    let sp0 = cardinal_spline(float3_0, float3_1, float3 { 2.0, 2.0, 2.0 },
                              float3 { 3.0, 3.0, 3.0 }, 0.0, 0.5)
    test_approx(sp0, float3_1)
    let sp1 = cardinal_spline(float3_0, float3_1, float3 { 2.0, 2.0, 2.0 },
                              float3 { 3.0, 3.0, 3.0 }, 1.0, 0.5)
    test_approx(sp1, float3 { 2.0, 2.0, 2.0 })

    // spherical_lerp between quaternions: endpoints are exact.
    let qa = float4 { 0.0, 0.0, 0.0, 1.0 }  // Identity quaternion.
    let qb = float4 { 0.0, 0.0, 1.0, 0.0 }  // 180 degrees around z.
    test_approx(spherical_lerp(qa, qb, 0.0), qa)
    test_approx(spherical_lerp(qa, qb, 1.0), qb)

    // circles_within_range: 3 circles on a line, radius 1, within distance 1
    // of each other's radius only for adjacent ones.
    let positions = [ float2 { 0.0, 0.0 }, float2 { 3.0, 0.0 }, float2 { 6.0, 0.0 } ]
    let radiuses = [ 1.0, 1.0, 1.0 ]
    let neighbors = circles_within_range(1.5, positions, radiuses, [], [], int2_0)
    test_eq(neighbors.length, 3)
    test_seq(neighbors[0].qsort(): _a < _b, [ 1 ])
    test_seq(neighbors[1].qsort(): _a < _b, [ 0, 2 ])
    test_seq(neighbors[2].qsort(): _a < _b, [ 1 ])

    // wave_function_collapse: output has the requested size and only uses
    // tiles from the input.
    rnd_seed(0)
    let wfcin = [ "ab", "ba" ]
    let wfcout, contradictions = wave_function_collapse(wfcin, int2 { 4, 3 })
    test_eq(wfcout.length, 3)
    test_eq(wfcout[0].length, 4)
    test_true(contradictions >= 0)
    for(wfcout) row:
        for(row) c:
            test_true(c == 'a' or c == 'b', "unexpected tile {c}")

geometry_tests()
