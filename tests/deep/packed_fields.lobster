import std
import vec

// Fields stored in part of a slot: `x:int<8>`, `f:float<32>`, `b:bool<8>`. Only the storage
// is narrow: reading such a field gives a plain int/float (sign extended for ints), writing
// it stores the low bits. Fields declared next to each other share a slot while they fit,
// so a struct or object holding them takes fewer slots, which is otherwise invisible.

struct PFSmall:  // 1 slot: 8 + 8 + 16 + 32 bits.
    a:int<8>
    b:int<8>
    c:int<16>
    d:int<32>

struct PFMixed:  // 4 slots: f and g, i (32 + 32 + 8 does not fit), j, k and l.
    f:float<32>
    g:float<32>
    i:int<8>
    j:int
    k:int<24>
    l:int<40>

// The program says who gets the bits left over: 32 + 32 fill a slot, then 8 + 8.
struct PFFloats:  // 2 slots: x and y, z and w.
    x:float<32>
    y:float<32>
    z:int<8>
    w:int<8>

// 64 is just a whole slot.
struct PFWhole:  // 3 slots, one per field.
    a:int<64>
    b:float<64>
    c:int<8>

enum PFColor:
    pf_red
    pf_green
    pf_blue

struct PFEnums:  // 1 slot: 8 + 8 + 16 bits.
    on:bool<8>
    col:PFColor<8>
    n:int<16>

// A struct field that is itself packed, next to packed fields of its own.
struct PFNested:  // 3 slots: p, s, q and r.
    p:int<8>
    s:PFSmall
    q:int<8>
    r:int<8>

class PFObj:  // 4 slots: a and b, s, f and g, v.
    attribute serializable
    a:int<8>
    b:int<8>
    s:string
    f:float<32>
    g:float<32>
    v:[int]

class PFSub : PFObj  // 5 slots: the 4 of PFObj, then c and d.
    attribute serializable
    c:int<8>
    d:int<16>

class PFSer:  // 3 slots: s, o, f and n.
    attribute serializable
    s:PFSmall
    o:PFObj
    f:float<32>
    n:int<8>

// Defaults, for construction with `{}` and for deserialization.
class PFDefaults:  // 3 slots: a, b and f, s, p.
    a:int<8> = 3
    b:int<8> = -4
    f:float<32> = 1.5
    s:string = "x"
    p:PFSmall = PFSmall { 1, 2, 3, 4 }

// Packed fields in an abstract struct family: members with different such fields share the
// family's int slots, and the type slot, whose family index takes only its low 8 bits, holds
// the first fields of a member when they fit in the other 56.
abstract struct PFShape  // 4 slots for every member: the type slot, 2 int slots, 1 reference slot.

struct PFCircle : PFShape  // r and filled share the type slot, the other three are padding.
    r:float<32>
    filled:bool<8>

struct PFRect : PFShape  // w, h and filled share the type slot, name has the reference slot.
    w:int<16>
    h:int<16>
    filled:bool<8>
    name:string

struct PFTri : PFShape  // a needs a whole int slot, b and c the second.
    a:int
    b:int<32>
    c:int<32>

def pf_area(s:PFShape) -> float:
    switch s:
        case PFCircle: return 3.0 * s.r * s.r
        case PFRect: return float(s.w * s.h)
        case PFTri: return float(s.a + s.b + s.c)

// A whole family in one slot: every member fits next to the family index.
abstract struct PFTiny  // 1 slot for every member: the family index in its low 8 bits.

struct PFTinyA : PFTiny  // x, y and z in bits 8 to 55.
    x:int<16>
    y:int<16>
    z:int<16>

struct PFTinyB : PFTiny  // f in bits 8 to 39, on in 40 to 47.
    f:float<32>
    on:bool<8>

struct PFTinyC : PFTiny  // Only the family index.

def pf_tiny_str(t:PFTiny) -> string:
    switch t:
        case PFTinyA: return "A {t.x} {t.y} {t.z}"
        case PFTinyB: return "B {t.f} {t.on}"
        case PFTinyC: return "C"

class PFTinyDef:  // 2 slots: t, n.
    t:PFTiny = PFTinyA { 7, -8, 9 }
    n:int<8> = 1

// The slot counts above, as the runtime sees them (an object's field count is its slots).
class PFSlotCounts:  // 1 + 4 + 2 + 3 + 1 + 3 + 4 + 1 = 19 slots.
    a:PFSmall
    b:PFMixed
    c:PFFloats
    d:PFWhole
    e:PFEnums
    f:PFNested
    g:PFShape
    h:PFTiny

class PFWithMember:  // 2 slots: x, then m.
    x:int

    def bump_m(v:int):
        member m:int<8> = 7
        m += v
        return m

def packed_fields_tests():
    suite("packed_fields")

    // Struct: construction, reading (with sign extension), and writing.
    var s = PFSmall { 1, -2, 300, -70000 }
    test_eq(s.a, 1)
    test_eq(s.b, -2)
    test_eq(s.c, 300)
    test_eq(s.d, -70000)
    test_eq("{s}", "PFSmall\{1, -2, 300, -70000\}")
    s.a = -128
    s.b = 127
    s.c = -32768
    s.d = 2147483647
    test_eq(s.a, -128)
    test_eq(s.b, 127)
    test_eq(s.c, -32768)
    test_eq(s.d, 2147483647)
    // Writing only stores the low bits, and does not disturb the neighbours.
    s.a = 256 + 5
    test_eq(s.a, 5)
    test_eq(s.b, 127)
    s.b = -1
    test_eq(s.a, 5)
    test_eq(s.c, -32768)
    // Compound assignment and increments read and write the bits.
    s.a += 10
    test_eq(s.a, 15)
    s.a -= 20
    test_eq(s.a, -5)
    s.c *= 2
    test_eq(s.c, 0)  // -65536 truncated to 16 bits.
    s.d = 5
    s.d++
    test_eq(s.d, 6)
    s.d--
    s.d--
    test_eq(s.d, 4)
    s.d |= 8
    test_eq(s.d, 12)
    s.d &= 4
    test_eq(s.d, 4)
    s.d <<= 2
    test_eq(s.d, 16)
    let pi = s.d++
    test_eq(pi, 16)
    test_eq(s.d, 17)

    // Struct equality compares the stored bits.
    test_true(PFSmall { 1, 2, 3, 4 } == PFSmall { 1, 2, 3, 4 })
    test_true(PFSmall { 1, 2, 3, 4 } != PFSmall { 1, 2, 3, 5 })
    test_true(PFSmall { -1, 2, 3, 4 } != PFSmall { 1, 2, 3, 4 })
    let m1 = PFMixed { 1.5, -2.25, -7, 1000000000000, 100000, 549755813887 }
    test_eq(m1.f, 1.5)
    test_eq(m1.g, -2.25)
    test_eq(m1.i, -7)
    test_eq(m1.j, 1000000000000)
    test_eq(m1.k, 100000)
    test_eq(m1.l, 549755813887)
    test_true(m1 == PFMixed { 1.5, -2.25, -7, 1000000000000, 100000, 549755813887 })
    test_true(m1 != PFMixed { 1.5, -2.25, -7, 1000000000000, 100000, 549755813886 })
    // A 32-bit float rounds.
    var fl = PFFloats { 0.1, -1.0e10, 1, 2 }
    test_approx(fl.x, 0.1)
    test_true(fl.x != 0.1)
    test_eq(fl.y, -1.0e10)
    test_eq(fl.z, 1)
    test_eq(fl.w, 2)
    fl.x = 2.5
    fl.z = -3
    test_eq(fl.x, 2.5)
    test_eq(fl.y, -1.0e10)
    test_eq(fl.z, -3)
    test_eq(fl.w, 2)
    fl.y += 1.0e10
    test_eq(fl.y, 0.0)
    fl.x *= 2.0
    test_eq(fl.x, 5.0)
    test_eq("{fl}", "PFFloats\{5.0, 0.0, -3, 2\}")
    let wh = PFWhole { 1234567890123, 2.5, -1 }
    test_eq(wh.a, 1234567890123)
    test_eq(wh.b, 2.5)
    test_eq(wh.c, -1)

    // Enums and bool.
    var en = PFEnums { true, pf_blue, 500 }
    test_eq(en.on, true)
    test_eq(en.col, pf_blue)
    test_eq(en.n, 500)
    test_eq("{en}", "PFEnums\{true, pf_blue, 500\}")
    en.on = false
    en.col = pf_green
    test_eq(en.on, false)
    test_eq(en.col, pf_green)
    test_eq(en.n, 500)
    if en.on: test_true(false, "bool<8> false is true")
    let sw = switch en.col:
        case pf_red: 1
        case pf_green: 2
        case pf_blue: 3
    test_eq(sw, 2)

    // Nested.
    var ns = PFNested { 1, PFSmall { 2, 3, 4, 5 }, 6, 7 }
    test_eq(ns.p, 1)
    test_eq(ns.s.b, 3)
    test_eq(ns.s.d, 5)
    test_eq(ns.q, 6)
    test_eq(ns.r, 7)
    ns.s.c = -9
    ns.q = -1
    test_eq(ns.s.c, -9)
    test_eq(ns.s, PFSmall { 2, 3, -9, 5 })
    test_eq(ns.q, -1)
    test_eq(ns.r, 7)
    test_eq("{ns}", "PFNested\{1, PFSmall\{2, 3, -9, 5\}, -1, 7\}")
    let ns2 = ns
    test_true(ns2 == ns)
    ns.r = 0
    test_true(ns2 != ns)

    // Passed to and returned from functions, and as struct fields of a local copied around.
    def pf_sum(x:PFSmall) -> int: return x.a + x.b + x.c + x.d
    def pf_make(a, b) -> PFSmall: return PFSmall { a, b, a + b, a - b }
    test_eq(pf_sum(PFSmall { 1, 2, 3, 4 }), 10)
    test_eq(pf_sum(pf_make(-1, -2)), -1 - 2 - 3 + 1)
    let mk = pf_make(100, 27)
    test_eq(mk.c, 127)
    test_eq(mk.d, 73)
    def pf_multi() -> PFSmall, PFFloats: return PFSmall { 1, 2, 3, 4 }, PFFloats { 1.0, 2.0, 3, 4 }
    let ma, mb = pf_multi()
    test_eq(ma.c, 3)
    test_eq(mb.y, 2.0)
    test_eq(mb.w, 4)

    // Vectors of them: elements are the packed slots.
    let v = [ PFSmall { 1, 2, 3, 4 }, PFSmall { -1, -2, -3, -4 } ]
    test_eq(v[0].c, 3)
    test_eq(v[1].a, -1)
    test_eq(v[1].d, -4)
    v[1].b = 20
    v[0].d += 100
    test_eq(v[1], PFSmall { -1, 20, -3, -4 })
    test_eq(v[0].d, 104)
    v.push(PFSmall { 7, 8, 9, 10 })
    test_eq(v.length, 3)
    test_eq(v[2].b, 8)
    var total = 0
    for(v) x: total += x.a
    test_eq(total, 7)
    test_eq("{v}", "[PFSmall\{1, 2, 3, 104\}, PFSmall\{-1, 20, -3, -4\}, PFSmall\{7, 8, 9, 10\}]")
    let popped = v.pop()
    test_eq(popped.c, 9)
    test_true(equal(v, [ PFSmall { 1, 2, 3, 104 }, PFSmall { -1, 20, -3, -4 } ]))
    let fv = [ PFFloats { 1.5, 2.5, 3, 4 } ]
    fv[0].y = -0.5
    fv[0].w = -4
    test_eq(fv[0].x, 1.5)
    test_eq(fv[0].y, -0.5)
    test_eq(fv[0].z, 3)
    test_eq(fv[0].w, -4)

    // Objects.
    let o = PFObj { 1, -2, "s", 0.5, -0.25, [ 3 ] }
    test_eq(o.a, 1)
    test_eq(o.b, -2)
    test_eq(o.s, "s")
    test_eq(o.f, 0.5)
    test_eq(o.g, -0.25)
    test_eq(o.v[0], 3)
    o.a = -100
    o.b += 5
    o.f = 8.0
    o.g *= 4.0
    test_eq(o.a, -100)
    test_eq(o.b, 3)
    test_eq(o.f, 8.0)
    test_eq(o.g, -1.0)
    test_eq(o.s, "s")
    test_eq("{o}", "PFObj\{-100, 3, \"s\", 8.0, -1.0, [3]\}")
    let o2 = copy(o)
    test_true(equal(o, o2))
    o2.a = 1
    test_true(not equal(o, o2))
    test_eq(o.a, -100)
    // A subclass adds its own packed fields behind the inherited ones.
    let sub = PFSub { 1, 2, "t", 3.0, 4.0, []::int, 5, 600 }
    test_eq(sub.a, 1)
    test_eq(sub.b, 2)
    test_eq(sub.c, 5)
    test_eq(sub.d, 600)
    sub.c = -6
    sub.a = 9
    test_eq(sub.a, 9)
    test_eq(sub.b, 2)
    test_eq(sub.c, -6)
    test_eq(sub.d, 600)
    let asbase:PFObj = sub
    test_eq(asbase.a, 9)
    asbase.b = 11
    test_eq(sub.b, 11)
    test_eq("{sub}", "PFSub\{9, 11, \"t\", 3.0, 4.0, [], -6, 600\}")
    let os = [ o, sub ]
    test_eq(os[1].b, 11)

    // Defaults.
    let d = PFDefaults {}
    test_eq(d.a, 3)
    test_eq(d.b, -4)
    test_eq(d.f, 1.5)
    test_eq(d.s, "x")
    test_eq(d.p, PFSmall { 1, 2, 3, 4 })
    let d2 = PFDefaults { b: 20 }
    test_eq(d2.a, 3)
    test_eq(d2.b, 20)

    // member declarations.
    let wm = PFWithMember { 1 }
    test_eq(wm.bump_m(0), 7)
    test_eq(wm.bump_m(-9), -2)
    test_eq(wm.bump_m(-200), 54)  // -202 truncated to 8 bits.

    // Abstract struct family.
    let shapes = [ PFCircle { 2.0, true }, PFRect { 3, 4, false, "r" }, PFTri { 1, 2, 3 } ]
    test_eq(pf_area(shapes[0]), 12.0)
    test_eq(pf_area(shapes[1]), 12.0)
    test_eq(pf_area(shapes[2]), 6.0)
    var sh:PFShape = PFRect { -5, 6, true, "q" }
    test_true(sh is PFRect)
    switch sh:
        case PFRect:
            test_eq(sh.w, -5)
            test_eq(sh.h, 6)
            test_eq(sh.filled, true)
            test_eq(sh.name, "q")
            sh.w = 7
            sh.filled = false
            test_eq(sh.w, 7)
            test_eq(sh.h, 6)
            test_eq(sh.filled, false)
        default:
            test_true(false, "not a rect")
    test_eq("{sh}", "PFRect\{7, 6, false, \"q\"\}")
    sh = PFCircle { 1.5, false }
    test_eq("{sh}", "PFCircle\{1.5, false\}")
    test_true(sh == PFCircle { 1.5, false })
    test_true(sh != PFCircle { 1.5, true })
    test_true(sh != shapes[0])
    test_eq("{shapes}", "[PFCircle\{2.0, true\}, PFRect\{3, 4, false, \"r\"\}, PFTri\{1, 2, 3\}]")

    // A family that shares the type slot with its fields.
    let tinies = [ PFTinyA { 1, -2, 3 }, PFTinyB { 0.5, true }, PFTinyC {} ]
    test_eq(pf_tiny_str(tinies[0]), "A 1 -2 3")
    test_eq(pf_tiny_str(tinies[1]), "B 0.5 true")
    test_eq(pf_tiny_str(tinies[2]), "C")
    test_true(tinies[0] is PFTinyA)
    test_true(not (tinies[0] is PFTinyB))
    test_true(tinies[2] is PFTinyC)
    var tn:PFTiny = PFTinyA { -1, -1, -1 }  // Every field bit set, the index below them.
    test_true(tn is PFTinyA)
    test_eq(pf_tiny_str(tn), "A -1 -1 -1")
    switch tn:
        case PFTinyA:
            tn.y = 500
            tn.x += 2
            test_eq(tn.x, 1)
            test_eq(tn.y, 500)
            test_eq(tn.z, -1)
        default:
            test_true(false, "not a PFTinyA")
    test_true(tn is PFTinyA)
    test_eq("{tn}", "PFTinyA\{1, 500, -1\}")
    tn = PFTinyB { -2.5, false }
    test_true(tn is PFTinyB)
    test_eq("{tn}", "PFTinyB\{-2.5, false\}")
    test_true(tn == PFTinyB { -2.5, false })
    test_true(tn != PFTinyB { -2.5, true })
    test_true(tn != tinies[1])
    test_true(PFTinyC {} == PFTinyC {})
    test_true(PFTinyC {} != PFTinyA { 0, 0, 0 })
    let tdef = PFTinyDef {}
    test_eq(pf_tiny_str(tdef.t), "A 7 -8 9")
    test_eq(tdef.n, 1)
    let ptd, ptderr = parse_data(typeof PFTinyDef, "PFTinyDef \{\}")
    test_eq(ptderr, nil)
    test_true(equal(assert ptd, tdef))
    let ptn, ptnerr = parse_data(typeof [PFTiny], "{tinies}")
    test_eq(ptnerr, nil)
    test_true(equal(assert ptn, tinies))
    let tflex = flexbuffers_value_to_binary(tinies)
    let tval, tverr = flexbuffers_binary_to_value(typeof [PFTiny], tflex)
    test_eq(tverr, nil)
    test_true(equal(assert tval, tinies))
    test_eq(type_field_count(PFTinyDef {}), 2)
    test_eq(type_field_count(PFSlotCounts { PFSmall { 1, 2, 3, 4 }, m1, fl, wh, en, ns,
                                            PFTri { 1, 2, 3 }, PFTinyC {} }), 19)

    // Serialization: printed form, flexbuffers and lobster binary all go by field.
    let pd, pderr = parse_data(typeof [PFSmall], "[ PFSmall \{ -1, 2, -3, 4 \}, PFSmall \{ 5 \} ]")
    test_eq(pderr, nil)
    test_true(equal(assert pd, [ PFSmall { -1, 2, -3, 4 }, PFSmall { 5, 0, 0, 0 } ]))
    let pdo, pdoerr = parse_data(typeof PFObj, "{o}")
    test_eq(pdoerr, nil)
    test_true(equal(assert pdo, o))
    // (A string default is not supported by parse_data, see serialization.lobster.)
    let pdd, pdderr = parse_data(typeof PFDefaults, "PFDefaults \{\}")
    test_eq(pdderr, nil)
    test_true(equal(assert pdd, PFDefaults { s: "" }))
    let pdd2, pdderr2 = parse_data(typeof PFDefaults, "PFDefaults \{ 5, 6 \}")
    test_eq(pdderr2, nil)
    test_true(equal(assert pdd2, PFDefaults { a: 5, b: 6, s: "" }))
    let pdsh, pdsherr = parse_data(typeof [PFShape], "{shapes}")
    test_eq(pdsherr, nil)
    test_true(equal(assert pdsh, shapes))
    let flex = flexbuffers_value_to_binary(o)
    let fval, fverr = flexbuffers_binary_to_value(typeof PFObj, flex)
    test_eq(fverr, nil)
    test_true(equal(assert fval, o))
    let dflex = flexbuffers_value_to_binary(PFDefaults {})
    let djson = assert flexbuffers_binary_to_json(dflex, false, "")
    // Fields equal to their default are left out.
    test_eq(djson, "\{ p: \{ a: 1, b: 2, c: 3, d: 4 \}, s: \"x\" \}")
    let dval, dverr = flexbuffers_binary_to_value(typeof PFDefaults, dflex)
    test_eq(dverr, nil)
    test_true(equal(assert dval, PFDefaults {}))
    let sflex = flexbuffers_value_to_binary(shapes)
    let sval, sverr = flexbuffers_binary_to_value(typeof [PFShape], sflex)
    test_eq(sverr, nil)
    test_true(equal(assert sval, shapes))
    let ser = PFSer { PFSmall { 1, -2, 3, -4 }, o, 0.5, -3 }
    let bin = lobster_value_to_binary(ser)
    let bval, bverr = lobster_binary_to_value(typeof PFSer, bin)
    test_eq(bverr, nil)
    test_true(equal(assert bval, ser))

    // Compile errors.
    guard not vm_compiled_mode()
    expect_errors("""
        struct S:
            a:int<7>
        return 1
        """, [ "bit width of an int field must be a multiple of 8" ])
    expect_errors("""
        struct S:
            a:float<16>
        return 1
        """, [ "bit width of a float field must be 32 or 64" ])
    expect_errors("""
        struct S:
            a:string<8>
        return 1
        """, [ "only int, float and enum fields can be given a bit width" ])
    expect_errors("""
        struct S<T>:
            a:T<8>
        return 1
        """, [ "only int, float and enum fields can be given a bit width" ])
    expect_errors("""
        let x:int<8> = 1
        return x
        """, [ "end of line expected" ], 1)
    // No vector math, runtime indexing or numeric struct builtin arguments on them.
    expect_errors("""
        struct S:
            a:int<8>
            b:int<8>
        let s = S { 1, 2 } + S { 3, 4 }
        return 1
        """, [ "requires type `numeric/string/struct`" ])
    expect_errors("""
        struct S:
            a:int<8>
            b:int<8>
        let s = S { 1, 2 }
        return s[0]
        """, [ "requires type `vector/string/numeric struct`" ])
    expect_errors("""
        struct S:
            a:int<32>
            b:int<32>
        return manhattan(S { 1, 2 })
        """, [ "arguments match no overloads of manhattan" ])

packed_fields_tests()
