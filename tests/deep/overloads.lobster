import std

import quaternion
import mat2x2

// Operator overloading in full: every overloadable operator on a class and
// on a struct, plus the quaternion and mat2x2 modules which are the stdlib's
// operator-overload showcases.

// A class overloading every overloadable operator. Assignment operators can
// mutate `this` in place, which only makes sense on classes.
class OvC:
    a:int

    def operator+(o:OvC): return OvC { a + o.a }
    def operator-(o:OvC): return OvC { a - o.a }
    def operator*(o:OvC): return OvC { a * o.a }
    def operator/(o:OvC): return OvC { a / o.a }
    def operator%(o:OvC): return OvC { a % o.a }
    def operator&(o:OvC): return OvC { a & o.a }
    def operator|(o:OvC): return OvC { a | o.a }
    def operator^(o:OvC): return OvC { a ^ o.a }
    def operator<<(o:OvC): return OvC { a << o.a }
    def operator>>(o:OvC): return OvC { a >> o.a }

    def operator+=(o:OvC): a += o.a
    def operator-=(o:OvC): a -= o.a
    def operator*=(o:OvC): a *= o.a
    def operator/=(o:OvC): a /= o.a
    def operator%=(o:OvC): a %= o.a
    def operator&=(o:OvC): a &= o.a
    def operator|=(o:OvC): a |= o.a
    def operator^=(o:OvC): a ^= o.a
    def operator<<=(o:OvC): a <<= o.a
    def operator>>=(o:OvC): a >>= o.a

    def operator==(o:OvC): return a == o.a
    def operator!=(o:OvC): return a != o.a
    def operator<(o:OvC): return a < o.a
    def operator>(o:OvC): return a > o.a
    def operator<=(o:OvC): return a <= o.a
    def operator>=(o:OvC): return a >= o.a

    def operator~(): return OvC { ~a }
    def operator-(): return OvC { -a }
    def operator++(): return a++
    def operator--(): return a--

    def operator=(o:OvC): a = o.a

    def operator[](i:int): return a * 10 + i

// A struct with the non-mutating subset (structs are copied by value, so
// assignment operators are not applicable).
struct OvS:
    a:int

    def operator+(o:OvS): return OvS { a + o.a }
    def operator-(o:OvS): return OvS { a - o.a }
    def operator*(o:OvS): return OvS { a * o.a }
    def operator/(o:OvS): return OvS { a / o.a }
    def operator%(o:OvS): return OvS { a % o.a }
    def operator&(o:OvS): return OvS { a & o.a }
    def operator|(o:OvS): return OvS { a | o.a }
    def operator^(o:OvS): return OvS { a ^ o.a }
    def operator<<(o:OvS): return OvS { a << o.a }
    def operator>>(o:OvS): return OvS { a >> o.a }

    def operator==(o:OvS): return a == o.a
    def operator!=(o:OvS): return a != o.a
    def operator<(o:OvS): return a < o.a
    def operator>(o:OvS): return a > o.a
    def operator<=(o:OvS): return a <= o.a
    def operator>=(o:OvS): return a >= o.a

    def operator~(): return OvS { ~a }
    def operator-(): return OvS { -a }

    def operator[](i:int): return a * 100 + i

def overloads_tests():
    suite("overloads")

    // Class: arithmetic operators produce new instances.
    let c2 = OvC { 2 }
    let c3 = OvC { 3 }
    test_eq((c2 + c3).a, 5)
    test_eq((c3 - c2).a, 1)
    test_eq((c2 * c3).a, 6)
    test_eq((OvC { 7 } / c2).a, 3)
    test_eq((OvC { 7 } % c2).a, 1)
    test_eq((c2 & c3).a, 2)
    test_eq((c2 | c3).a, 3)
    test_eq((c2 ^ c3).a, 1)
    test_eq((c3 << c2).a, 12)
    test_eq((OvC { 12 } >> c2).a, 3)
    // Unary.
    test_eq((-c3).a, -3)
    test_eq((~c3).a, -4)
    test_eq((- -c3).a, 3)
    // Comparisons.
    test_true(c2 == OvC { 2 })      // Structural via the overload...
    test_true(c2 != c3)
    test_true(c2 < c3)
    test_true(c3 > c2)
    test_true(c2 <= OvC { 2 })
    test_true(c3 >= c3)
    test_eq(c2 > c3, false)
    // Indexing with the [] overload.
    test_eq(c3[7], 37)
    // Assignment operators mutate the object in place.
    var cacc = OvC { 1 }
    let calias = cacc                // Same object, observes the mutations.
    cacc += c3
    test_eq(cacc.a, 4)
    cacc -= OvC { 1 }
    test_eq(cacc.a, 3)
    cacc *= c3
    test_eq(cacc.a, 9)
    cacc /= c2
    test_eq(cacc.a, 4)
    cacc %= c3
    test_eq(cacc.a, 1)
    cacc |= c2
    test_eq(cacc.a, 3)
    cacc &= c2
    test_eq(cacc.a, 2)
    cacc ^= c3
    test_eq(cacc.a, 1)
    cacc <<= c3
    test_eq(cacc.a, 8)
    cacc >>= c2
    test_eq(cacc.a, 2)
    // operator= also overwrites contents rather than rebinding the variable.
    cacc = OvC { 42 }
    test_eq(calias.a, 42)
    test_true(cacc == calias)
    // ++/-- overloads (as statements).
    cacc++
    test_eq(cacc.a, 43)
    cacc--
    test_eq(cacc.a, 42)

    // Struct: same operators, value semantics.
    let s2 = OvS { 2 }
    let s3 = OvS { 3 }
    test_eq((s2 + s3).a, 5)
    test_eq((s3 - s2).a, 1)
    test_eq((s2 * s3).a, 6)
    test_eq((OvS { 7 } / s2).a, 3)
    test_eq((OvS { 7 } % s2).a, 1)
    test_eq((s2 & s3).a, 2)
    test_eq((s2 | s3).a, 3)
    test_eq((s2 ^ s3).a, 1)
    test_eq((s3 << s2).a, 12)
    test_eq((OvS { 12 } >> s2).a, 3)
    test_eq((-s3).a, -3)
    test_eq((~s3).a, -4)
    test_true(s2 == OvS { 2 })
    test_true(s2 != s3)
    test_true(s2 < s3 and s3 > s2 and s2 <= s2 and s3 >= s2)
    test_eq(s3[9], 309)
    // Mixed expression as in the old operators test.
    test_true((s2 + s3 - s2) * s3 / s2 == OvS { 4 })

    // Quaternions from the quaternion module (struct quat : float4 with
    // overloads; + etc come from the built-in float4 math).
    test_eq(-quat { 1.0, 1.0, 1.0, 1.0 }, quat { -1.0, -1.0, -1.0, 1.0 })
    test_eq(quat { 1.0, 1.0, 1.0, 1.0 } * quat { 2.0, 2.0, 2.0, 1.0 },
            quat { 3.0, 3.0, 3.0, -5.0 })
    test_eq(quat { 1.0, 1.0, 1.0, 1.0 } * 2.0, quat { 2.0, 2.0, 2.0, 2.0 })
    test_eq(quat_unit * quat_unit, quat_unit)
    // Rotating a point around z by 180 degrees.
    let q180 = quat_from_angle_axis(180.0, float3_z)
    test_approx(q180.transform(float3 { 1.0, 0.0, 0.0 }), float3 { -1.0, 0.0, 0.0 })
    // The identity quat transforms any point to itself.
    test_approx(quat_unit.transform(float3 { 1.0, 2.0, 3.0 }), float3 { 1.0, 2.0, 3.0 })
    // quat_from_yaw/pitch/euler at angle 0 are the identity.
    test_approx(quat_from_yaw(0.0), quat_unit)
    test_approx(quat_from_pitch(0.0), quat_unit)
    test_approx(quat_from_euler(0.0, 0.0), quat_unit)
    // A quat between two identical vectors is the identity.
    test_approx(quat_from_2vecs(float3_x, float3_x), quat_unit)
    // Between x and y: rotates x onto y.
    let qxy = quat_from_2vecs(float3_x, float3_y)
    test_approx(qxy.transform(float3_x), float3_y)

    // mat2x2 (struct mat2x2 : float4): matrix multiply, determinant etc.
    let ma = mat2x2 { 1.0, 2.0, 3.0, 4.0 }
    let mb = mat2x2 { 5.0, 6.0, 7.0, 8.0 }
    test_eq(ma + 5.0, mat2x2 { 6.0, 7.0, 8.0, 9.0 })      // Built-in float4 math.
    test_eq(ma * mb, mat2x2 { 19.0, 22.0, 43.0, 50.0 })   // Overloaded matrix mul.
    test_eq(mb * ma, mat2x2 { 23.0, 34.0, 31.0, 46.0 })
    test_eq(ma.determinant(), -2.0)
    test_eq(ma.trace(), 4.0)  // As defined in the module: x*w.
    test_eq(ma.adjugate(), mat2x2 { 4.0, -2.0, -3.0, 1.0 })
    test_eq(ma.inverse(), mat2x2 { -2.0, 1.0, 1.5, -0.5 })
    test_eq(ma * ma.inverse(), mat2x2_identity)
    test_eq(ma * float2 { 2.0, 3.0 }, float2 { 8.0, 18.0 })
    test_eq(mat2x2_identity * float2 { 5.0, 7.0 }, float2 { 5.0, 7.0 })

overloads_tests()
