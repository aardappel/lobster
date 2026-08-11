
import vec

// Scalar math builtins, and random number generation.

// A trivial class for hashing a reference value.
class DHashObj:
    x = 1

def math_tests():
    suite("math")

    // abs / sign.
    test_eq(abs(-5), 5)
    test_eq(abs(5), 5)
    test_eq(abs(-2.5), 2.5)
    test_eq(sign(-42), -1)
    test_eq(sign(0), 0)
    test_eq(sign(42), 1)
    test_eq(sign(-0.001), -1)
    test_eq(sign(0.0), 0)
    test_eq(sign(123.4), 1)
    test_true(sign(1.5) is int)

    // min / max on two scalars; mixed int/float coerces to float.
    test_eq(min(1, 2), 1)
    test_eq(max(1, 2), 2)
    test_eq(min(-1.5, 1.5), -1.5)
    test_eq(max(-1.5, 1.5), 1.5)
    let mm1 = 1
    test_eq(min(mm1, 2.5), 1.0)
    test_true(min(mm1, 2.5) is float)
    test_eq(max(mm1, 0.5), 1.0)

    // pow: ints (exponentiation by squaring, negative exponent yields 0),
    // and floats.
    test_eq(pow(2, 10), 1024)
    test_eq(pow(3, 0), 1)
    test_eq(pow(0, 0), 1)
    test_eq(pow(-2, 3), -8)
    test_eq(pow(2, -1), 0)
    test_eq(pow(2, 63), 0x8000000000000000)  // Wraps like * does.
    test_eq(pow(2, 64), 0)
    test_approx(pow(2.0, 0.5), sqrt(2.0))
    test_approx(pow(10.0, -2.0), 0.01)
    test_eq(pow(2.0, 10.0), 1024.0)

    // Roots and logarithms.
    test_eq(sqrt(9.0), 3.0)
    test_eq(sqrt(0.0), 0.0)
    test_approx(log(exp_e()), 1.0)
    test_eq(log(1.0), 0.0)
    test_eq(log2(8.0), 3.0)
    test_eq(log2(1.0), 0.0)
    test_approx(log2(10.0), 3.321928094887362)

    // Rounding family. int() truncates, floor/ceiling to nearest int
    // down/up, round is half-away-from-zero.
    test_eq(int(1.9), 1)
    test_eq(int(-1.9), -1)
    test_eq(floor(1.9), 1)
    test_eq(floor(-1.1), -2)
    test_eq(floor(2.0), 2)
    test_eq(ceiling(1.1), 2)
    test_eq(ceiling(-1.9), -1)
    test_eq(ceiling(2.0), 2)
    test_eq(round(1.4), 1)
    test_eq(round(1.5), 2)
    test_eq(round(-1.5), -2)
    test_eq(round(2.5), 3)
    test_eq(round(0.5), 1)
    test_eq(round(-0.5), -1)
    test_eq(fraction(1.25), 0.25)
    test_eq(fraction(-1.25), 0.75)  // f - floor(f), so always >= 0.
    test_eq(fraction(2.0), 0.0)
    test_eq(float(3), 3.0)
    test_true(float(3) is float)

    // Trigonometry, in degrees.
    test_approx(sin(0.0), 0.0)
    test_approx(sin(90.0), 1.0)
    test_approx(cos(180.0), -1.0)
    test_approx(tan(45.0), 1.0)
    test_approx(asin(1.0), 90.0)
    test_approx(atan(1.0), 45.0)
    test_approx(radians(180.0), 3.14159265358979)
    test_approx(acos(-1.0), 180.0)
    test_approx(degrees(3.14159265358979), 180.0)
    test_true(abs(acos(-1.0) - 180.0) < 0.0001)
    test_true(abs(degrees(3.14159265358979) - 180.0) < 0.0001)
    let sc = sincos(90.0)
    test_approx(sc, float2 { 0.0, 1.0 })  // sincos yields (cos, sin).
    test_approx(sincos(0.0), float2 { 1.0, 0.0 })
    // atan2 covers all four quadrants.
    test_approx(atan2(float2 { 1.0, 1.0 }), 45.0)
    test_approx(atan2(float2 { -1.0, 1.0 }), 135.0)
    test_approx(atan2(float2 { 1.0, -1.0 }), -45.0)
    test_approx(atan2(float2 { -1.0, -1.0 }), -135.0)
    // Trig also applies component-wise to float structs.
    test_approx(sin(float2 { 90.0, 0.0 }), float2 { 1.0, 0.0 })
    test_approx(cos(float2 { 0.0, 180.0 }), float2 { 1.0, -1.0 })
    test_approx(tan(float2 { 45.0, 0.0 }), float2 { 1.0, 0.0 })

    // div forces float division on ints.
    test_eq(div(7, 2), 3.5)
    test_true(div(4, 2) is float)

    // clamp / in_range.
    test_eq(clamp(5, 0, 3), 3)
    test_eq(clamp(-5, 0, 3), 0)
    test_eq(clamp(2, 0, 3), 2)
    test_eq(clamp(0.5, 1.0, 2.0), 1.0)
    test_true(in_range(0, 1))
    test_true(in_range(5, 10))
    test_true(not in_range(10, 10))   // Range is exclusive.
    test_true(in_range(5, 10, 5))     // Bias is inclusive.
    test_true(not in_range(4, 10, 5))
    test_true(in_range(5.0, 10.0, 1.0))
    test_true(not in_range(11.0, 10.0, 1.0))

    // popcount.
    test_eq(popcount(0), 0)
    test_eq(popcount(255), 8)
    test_eq(popcount(-1), 64)
    test_eq(popcount(0x8000000000000000), 1)

    // Interpolation helpers. lerp extrapolates outside 0..1.
    test_eq(lerp(0.0, 10.0, 0.5), 5.0)
    test_eq(lerp(0.0, 10.0, 0.0), 0.0)
    test_eq(lerp(0.0, 10.0, 1.0), 10.0)
    test_eq(lerp(0.0, 10.0, 1.5), 15.0)
    test_eq(lerp(0.0, 10.0, -0.5), -5.0)
    test_eq(lerp(10.0, 0.0, 0.25), 7.5)
    // The 1-arg smoothstep does not clamp (input must be 0..1, outside just
    // evaluates the polynomial), but the 3-arg version clamps per the GL spec.
    test_eq(smoothstep(0.0), 0.0)
    test_eq(smoothstep(1.0), 1.0)
    test_eq(smoothstep(0.5), 0.5)
    test_eq(smoothstep(-1.0), 5.0)
    test_eq(smoothstep(2.0), -4.0)
    test_eq(smoothstep(0.0, 2.0, 1.0), 0.5)
    test_eq(smoothstep(0.0, 2.0, -1.0), 0.0)
    test_eq(smoothstep(0.0, 2.0, 3.0), 1.0)
    test_eq(smootherstep(0.5), 0.5)
    test_eq(smootherstep(1.0), 1.0)
    test_eq(smootherstep(0.0), 0.0)
    test_true(smoothmin(1.0, 5.0, 0.5) <= 1.0)

    // Float precision: classic 0.1 + 0.2 is only approximately 0.3.
    test_approx(0.1 + 0.2, 0.3)
    test_true(0.1 + 0.2 != 0.3)

    // IEEE754 specials from float division/modulo by zero.
    test_eq("{1.0 / 0.0}", "inf")
    test_eq("{-1.0 / 0.0}", "-inf")
    test_true("{0.0 / 0.0}".find_string("nan") >= 0)
    test_true("{1.0 % 0.0}".find_string("nan") >= 0)
    test_true("{log(-1.0)}".find_string("nan") >= 0)
    test_eq("{log(0.0)}", "-inf")
    test_true("{sqrt(-1.0)}".find_string("nan") >= 0)

    // Noise functions: deterministic, roughly in [-1, 1].
    let sxv = simplex_raw(float2 { 0.3, 0.7 })
    test_eq(sxv, simplex_raw(float2 { 0.3, 0.7 }))
    test_true(sxv >= -1.0 and sxv <= 1.0)
    let sxv3 = simplex_raw(float3 { 0.3, 0.7, 0.1 })
    test_true(sxv3 >= -1.0 and sxv3 <= 1.0)
    let sxo = simplex(float2 { 0.3, 0.7 }, 2, 1.0, 0.5)
    test_true(sxo >= -1.0 and sxo <= 1.0)
    let fnv = fast_noise(float2 { 0.3, 0.7 }, 2, 1.0, 0.5, 0, 0)
    test_eq(fnv, fast_noise(float2 { 0.3, 0.7 }, 2, 1.0, 0.5, 0, 0))

    // Randomness: seeded, so deterministic.
    rnd_seed(1234)
    let r1 = rnd(1000000)
    rnd_seed(1234)
    let r2 = rnd(1000000)
    test_eq(r1, r2)
    rnd_seed(55)
    let rg1 = rnd_gaussian()
    rnd_seed(55)
    test_eq(rnd_gaussian(), rg1)
    test_true(r1 >= 0 and r1 < 1000000)
    test_eq(rnd(1), 0)
    test_eq(rnd(-5), 0)  // Max is clamped to at least 1.
    let rf = rnd_float()
    test_true(rf >= 0.0 and rf < 1.0)
    rnd_gaussian()  // Just check it runs.
    let rv = rnd(int2 { 4, 4 })
    test_true(rv.x >= 0 and rv.x < 4 and rv.y >= 0 and rv.y < 4)
    // Separate generators via rnd_select don't disturb each other.
    rnd_seed(42)
    let old_rng = rnd_select(1)
    test_eq(old_rng, 0)
    rnd_seed(9999)
    rnd(100)
    rnd_select(0)
    let r3 = rnd(1000000)
    rnd_seed(42)
    test_eq(rnd(1000000), r3)
    // Deprecated mersenne twister is also seeded/deterministic.
    rndm_seed(7)
    let rm1 = rndm(1000)
    rndm_seed(7)
    test_eq(rndm(1000), rm1)

    // hash: any value hashes to a positive int.
    test_true(hash(123) >= 0)
    test_true(hash(-123) >= 0)
    test_true(hash(1.5) >= 0)
    test_true(hash("abc") >= 0)
    test_true(hash([ 1, 2 ]) >= 0)
    test_true(hash(int2 { 1, 2 }) >= 0)
    test_true(hash(float2 { 1.5, 2.5 }) >= 0)
    test_true(hash(true) >= 0)          // Enum (ref-free) value.
    test_true(hash(DHashObj {}) >= 0)   // Class instance.
    // Value-typed things (strings, scalars, numeric structs) hash by value:
    // equal values hash equally.
    test_eq(hash("abc"), hash("abc"))
    test_eq(hash(int2 { 1, 2 }), hash(int2 { 1, 2 }))
    test_eq(hash(1.5), hash(1.5))
    // Vectors and classes hash by identity (address), so two equal-but-distinct
    // objects generally hash differently, while an object hashes equal to
    // itself.
    let hv = [ 1, 2 ]
    test_eq(hash(hv), hash(hv))
    test_neq(hash([ 1, 2 ]), hash([ 1, 2 ]))
    test_true(hash(nil::string) >= 0)

    // line_intersect: crossing and parallel segments.
    let isect, ipoint = line_intersect(float2 { -1.0, 0.0 }, float2 { 1.0, 0.0 },
                                       float2 { 0.0, -1.0 }, float2 { 0.0, 1.0 })
    test_eq(isect, 1)
    test_approx(ipoint, float2_0)
    let isect2, ipoint2 = line_intersect(float2 { 0.0, 0.0 }, float2 { 1.0, 0.0 },
                                         float2 { 0.0, 1.0 }, float2 { 1.0, 1.0 })
    test_eq(isect2, 0)
    test_approx(ipoint2, float2_0)

// Not a builtin: e as a value, for the log test.
private def exp_e(): return 2.718281828459045

math_tests()
