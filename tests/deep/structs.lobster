
import vec

// Numeric structs (int2/float3/...): operators, indexing, and the
// vector-math builtins that operate on them.

// Structs used across the tests below.
struct DSA:
    x:int
    y:int

struct DSB:
    a:DSA  // Inline nested struct.
    b:int

struct DS5:
    a:float
    b:float
    c:float
    d:float
    e:float

def structs_tests():
    suite("structs")

    // Value semantics: assignment copies.
    var sa = int2 { 3, 4 }
    let sb = sa
    sa.y = 5
    test_eq(sa, int2 { 3, 5 })
    test_eq(sb, int2 { 3, 4 })

    // Field and index access; indexing struct fields.
    test_eq(sa.x, 3)
    test_eq(sa[1], 5)
    sa[1] = 4
    test_eq(sa.y, 4)
    test_eq(int3 { 5, 6, 7 }[2], 7)

    // Operators with structs and scalars, both ways.
    test_eq(int2 { 1, 2 } + int2 { 4, 5 }, int2 { 5, 7 })
    test_eq(int2 { 1, 2 } * int2 { 4, 5 }, int2 { 4, 10 })
    test_eq(int2 { 10, 20 } - int2 { 1, 2 }, int2 { 9, 18 })
    test_eq(int2 { 10, 20 } / int2 { 2, 5 }, int2 { 5, 4 })
    test_eq(int2 { 10, 20 } % int2 { 3, 6 }, int2 { 1, 2 })
    test_eq(int2 { 10, 20 } % 3, int2 { 1, 2 })
    test_eq(int2 { 2, 3 } * 5, int2 { 10, 15 })
    test_eq(5 * int2 { 2, 3 }, int2 { 10, 15 })
    test_eq(5 - int2 { 2, 3 }, int2 { 3, 2 })
    test_eq(int2 { 2, 3 } - 5, int2 { -3, -2 })
    test_eq(10 / int3 { 5, 10, 1 }, int3 { 2, 1, 10 })
    test_eq(-int2 { 1, -2 }, int2 { -1, 2 })
    test_eq(float2 { 1.5, 2.5 } * 2.0, float2 { 3.0, 5.0 })
    test_approx(float2 { 10.1, 20.1 } % 3.0, float2 { 1.1, 2.1 })
    // Assignment operators on struct vars and elements in vectors/fields.
    var sacc = int2 { 1, 2 }
    sacc += int2 { 10, 10 }
    test_eq(sacc, int2 { 11, 12 })
    sacc -= 1
    test_eq(sacc, int2 { 10, 11 })
    sacc *= 2
    test_eq(sacc, int2 { 20, 22 })
    let vsv = [ int2 { 1, 2 } ]
    vsv[0] += 1
    test_eq(vsv[0], int2 { 2, 3 })
    vsv[0][0] = 9
    test_eq(vsv[0], int2 { 9, 3 })

    // Comparison operators on structs return int structs; any/all reduce them.
    test_eq(int2 { 2, 5 } < 5, int2 { 1, 0 })
    test_eq(int2 { 2, 5 } <= 5, int2 { 1, 1 })
    test_eq(int2 { 2, 5 } > int2 { 1, 9 }, int2 { 1, 0 })
    test_eq(any(int2 { 2, 5 } < 5), true)
    test_eq(all(int2 { 2, 5 } < 5), false)
    test_eq(all(float2 { 0.5, 0.6 } < 1.0), true)
    // == on structs is full value equality (a single bool).
    test_true(int3 { 1, 2, 3 } == int3 { 1, 2, 3 })
    test_true(int3 { 1, 2, 3 } != int3 { 1, 2, 4 })

    // Generic vec2<T> works for non-numeric types too.
    let strvec = vec2<string> { "a", "b" }
    test_eq(strvec.x + strvec.y, "ab")

    // Nested structs, in classes and vectors.
    let dsb = DSB { DSA { 5, 6 }, 7 }
    test_eq(dsb.a.x + dsb.a.y + dsb.b, 18)
    let dsbv = [ DSB { DSA { 1, 2 }, 3 } ]
    test_eq(dsbv[0].a.y, 2)

    // Structs of size 1 and > 4 support math too.
    test_eq(DS5 { 1.0, 2.0, 3.0, 4.0, 5.0 } * 2.0, DS5 { 2.0, 4.0, 6.0, 8.0, 10.0 })
    test_eq(dot(DS5 { 1.0, 2.0, 3.0, 4.0, 5.0 }, DS5 { 1.0, 1.0, 1.0, 1.0, 1.0 }), 15.0)

    // Vector math builtins.
    test_eq(dot(float2 { 1.0, 2.0 }, float2 { 3.0, 4.0 }), 11.0)
    test_eq(magnitude(float2 { 3.0, 4.0 }), 5.0)
    test_eq(magnitude_squared(float2 { 3.0, 4.0 }), 25.0)
    test_eq(magnitude_squared(int2 { 3, 4 }), 25)
    test_eq(manhattan(int3 { 1, -2, 3 }), 6)
    test_eq(cross(float3 { 1.0, 0.0, 0.0 }, float3 { 0.0, 1.0, 0.0 }), float3 { 0.0, 0.0, 1.0 })
    test_eq(volume(int3 { 2, 3, 4 }), 24)
    test_eq(volume(float2 { 2.5, 4.0 }), 10.0)
    test_approx(normalize(float2 { 3.0, 4.0 }), float2 { 0.6, 0.8 })
    test_eq(normalize(float2_0), float2_0)  // Zero vector passes through.
    test_approx(magnitude(normalize(float3 { 1.0, 2.0, 3.0 })), 1.0)

    // Component-wise builtins.
    test_eq(abs(int2 { -1, 2 }), int2 { 1, 2 })
    test_eq(abs(float2 { -1.5, 2.5 }), float2 { 1.5, 2.5 })
    test_eq(sign(float3 { -6.0, 0.0, 6.0 }), int3 { -1, 0, 1 })
    test_eq(sign(int2 { -9, 9 }), int2 { -1, 1 })
    test_eq(int(float2 { 1.9, -1.9 }), int2 { 1, -1 })
    test_eq(float(int2 { 1, 2 }), float2 { 1.0, 2.0 })
    test_eq(floor(float2 { 1.9, -1.1 }), int2 { 1, -2 })
    test_eq(ceiling(float2 { 1.1, -1.9 }), int2 { 2, -1 })
    test_eq(round(float2 { 1.5, -1.5 }), int2 { 2, -2 })
    test_eq(fraction(float2 { 1.25, 3.5 }), float2 { 0.25, 0.5 })
    // FIXME: fraction on float structs computes f - int(f) per component, but
    // scalar fraction computes f - floor(f), so they disagree for negative
    // inputs: fraction(-1.25) == 0.75 but fraction(float2 { -1.25, .. }).x == -0.25.
    //test_eq(fraction(float2 { -1.25, 0.0 }).x, fraction(-1.25))
    test_approx(pow(float2 { 2.0, 3.0 }, 2.0), float2 { 4.0, 9.0 })

    // min/max: pairwise on two structs, or reducing one struct.
    test_eq(min(int2 { 1, 5 }, int2 { 2, 4 }), int2 { 1, 4 })
    test_eq(max(int2 { 1, 5 }, int2 { 2, 4 }), int2 { 2, 5 })
    test_eq(min(float2 { 1.5, 0.5 }, float2 { 0.7, 0.8 }), float2 { 0.7, 0.5 })
    test_eq(max(float2 { 1.5, 0.5 }, float2 { 0.7, 0.8 }), float2 { 1.5, 0.8 })
    test_eq(min(int3 { 3, 1, 2 }), 1)
    test_eq(max(int3 { 3, 1, 2 }), 3)
    test_eq(min(float2 { 1.5, 0.5 }), 0.5)
    test_eq(max(float2 { 1.5, 0.5 }), 1.5)
    // FIXME: same 32-bit sentinel problem as the vector versions: max() of an
    // all-negative float struct returns FLT_MIN, and components beyond 32-bit
    // int / float32 range are clamped to the sentinel.
    //test_eq(max(float2 { -5.0, -6.0 }), -5.0)
    //test_eq(min(int2 { 3000000000000000000, 4000000000000000000 }), 3000000000000000000)

    // clamp / in_range on structs.
    test_eq(clamp(int2 { -5, 9 }, int2_0, int2 { 3, 3 }), int2 { 0, 3 })
    test_eq(clamp(float2 { -0.5, 0.5 }, float2_0, float2_1), float2 { 0.0, 0.5 })
    test_true(in_range(int2 { 1, 2 }, int2 { 5, 5 }))
    test_true(not in_range(int2 { 5, 2 }, int2 { 5, 5 }))
    test_true(in_range(int3 { 1, 2, 3 }, int3 { 5, 5, 5 }))
    test_true(in_range(float2 { 0.5, 0.5 }, float2_1))
    test_true(in_range(float3 { 0.5, 0.5, 0.5 }, float3_1))

    // lerp on float structs.
    test_approx(lerp(float2_0, float2 { 10.0, 20.0 }, 0.5), float2 { 5.0, 10.0 })

    // Predefined constants from the vec module.
    test_eq(int2_0, int2 { 0, 0 })
    test_eq(int2_1, int2 { 1, 1 })
    test_eq(float3_0, float3 { 0.0, 0.0, 0.0 })
    test_eq(int2_x, int2 { 1, 0 })
    test_eq(int2_y, int2 { 0, 1 })

    // Struct values in control flow.
    test_eq((if sa.x > 0: int2 { 1, 2 } else: int2 { 3, 4 }).y, 2)
    let sw = switch manhattan(int2 { 1, 1 }):
        case 2: int2 { 1, 2 }
        default: int2 { 3, 4 }
    test_eq(sw, int2 { 1, 2 })
    for([ int2 { 1, 2 }, int2 { 0, 3 } ]) st:
        test_eq(st.x + st.y, 3)

    // Structs with reference-counted members.
    let rs = vec2<string> { "a", "b" }
    let rsv = [ rs, vec2<string> { "c", "d" } ]
    test_eq(rsv[1].y, "d")
    test_true(rsv[0] == rs)  // == on structs is value equality, also for string fields.

structs_tests()
