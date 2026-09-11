import std
import vec

// Abstract structs: an inline stored, by-value discriminated union. Every struct with an
// abstract struct superclass (the "family") has the same slots, carries the type it
// dynamically is in a hidden slot, and takes part in switch, is and dynamic dispatch like a
// class does. The other hidden slots are padding, which is never visible: not in printing,
// serialization, or field counts.

abstract struct ASValue

struct ASInt : ASValue
    i:int

struct ASFloat : ASValue
    f:float

struct ASStr : ASValue
    s:string

struct ASPair : ASValue
    a:int
    b:int

// Mixed reference and scalar fields, in different orders, so the family layout has to line
// up references with references and scalars with scalars.
struct ASMix : ASValue
    s:string
    n:int

struct ASMix2 : ASValue
    n:int
    v:[int]
    t:string

// No fields of its own at all: the type slot is all there is to it.
struct ASNone : ASValue

// A nested plain struct, and a subclass of a concrete member (a mid-family type).
struct ASVec : ASValue
    p:float3
    tag:int

struct ASVecN : ASVec
    name:string

// An abstract struct in the middle of the family, with a field of its own.
abstract struct ASShape : ASValue
    id:int

struct ASCircle : ASShape
    r:float

struct ASSquare : ASShape
    side:float

def as_describe(v:ASValue) -> string:
    switch v:
        case ASInt: return "int {v.i}"
        case ASFloat: return "float {v.f}"
        case ASStr: return "str {v.s}"
        case ASPair: return "pair {v.a} {v.b}"
        case ASMix: return "mix {v.s} {v.n}"
        case ASMix2: return "mix2 {v.n} {v.v} {v.t}"
        case ASNone: return "none"
        case ASVec: return "vec {v.p} {v.tag}"  // Also covers ASVecN.
        case ASShape: return "shape {v.id}"     // Covers ASCircle and ASSquare.

// Dynamic dispatch, with an implementation for every concrete member, one of which serves a
// subclass.
def as_size(v::ASInt): return i
def as_size(v::ASFloat): return int(f)
def as_size(v::ASStr): return s.length
def as_size(v::ASPair): return a + b
def as_size(v::ASMix): return s.length + n
def as_size(v::ASMix2): return n + v.length + t.length
def as_size(v::ASNone): return 0
def as_size(v::ASVec): return tag
def as_size(v::ASShape): return id

// Dispatch from an abstract mid-family root, with super.
def as_area(v::ASShape) -> float: return 0.0
def as_area(v::ASCircle) -> float: return 3.0 * r * r + super as_area()
def as_area(v::ASSquare) -> float: return side * side

// A struct method declared inline, dispatched from the family root.
abstract struct ASNode:
    depth:int
    def describe() -> string: return "node {depth}"

struct ASLeaf : ASNode
    val:int
    def describe() -> string: return "leaf {depth} {val}"

struct ASBranch : ASNode
    left:int
    right:int
    def describe() -> string: return "branch {depth} {left} {right}"
    def as_sum(): return left + right

// Generic abstract struct, whose specializations each form a family of their own.
abstract struct ASMaybe<T>:

struct ASJust<T> : ASMaybe<T>
    v:T

struct ASNothing<T> : ASMaybe<T>

struct ASJustInt = ASJust<int>
struct ASNothingInt = ASNothing<int>
struct ASJustStr = ASJust<string>
struct ASNothingStr = ASNothing<string>

def as_or<T>(m:ASMaybe<T>, d:T) -> T:
    if m is ASJust<T>: return m.v
    return d

def as_show(m::ASJustInt) -> string: return "just {v}"
def as_show(m::ASNothingInt) -> string: return "nothing"

// Members whose fields are declared in different orders, which the layout is free to put in
// any slots, and a nested plain struct of mixed kinds, which stays one block.
struct ASSub:
    s:string
    n:int

abstract struct ASMixedFam

struct ASMixedA : ASMixedFam
    k:int
    sub:ASSub

struct ASMixedB : ASMixedFam
    sub:ASSub
    k:int
    f:float

struct ASMixedC : ASMixedFam
    t:string
    f:float
    k:int

// Family values inside classes, structs and vectors, with defaults.
class ASHolder:
    v:ASValue = ASInt { 7 }
    n = 1

struct ASOuter:
    v:ASValue
    x:int

// Serializable members, for the lobster binary format.
abstract struct ASSer

struct ASSerA : ASSer
    attribute serializable
    x:int
    s:string

struct ASSerB : ASSer
    attribute serializable
    f:float
    v:[int]

class ASSerHolder:
    attribute serializable
    a:ASSer
    b:ASSer
    vs:[ASSer]

def as_pick(i:int) -> ASValue:
    // Different members from the branches of a switch unify to their family root.
    return switch i:
        case 0: ASInt { 10 }
        case 1: ASStr { "ten" }
        default: ASNone {}

def as_ident(v:ASValue) -> ASValue: return v
def as_twice(f) -> [ASValue]: return [ f(), f() ]

def abstract_structs_tests():
    suite("abstract_structs")

    // Construction, and each member printing as itself: the family's hidden slots don't show.
    let vi = ASInt { 1 }
    let vf = ASFloat { 2.5 }
    let vs = ASStr { "hello" }
    let vp = ASPair { 3, 4 }
    let vm = ASMix { "m", 5 }
    let vm2 = ASMix2 { 6, [ 7, 8 ], "t" }
    let vn = ASNone {}
    let vv = ASVec { float3 { 1.0, 2.0, 3.0 }, 9 }
    let vvn = ASVecN { float3_1, 10, "named" }
    let vc = ASCircle { 11, 1.5 }
    let vsq = ASSquare { id: 12, side: 2.0 }
    test_eq("{vi}", "ASInt\{1\}")
    test_eq("{vf}", "ASFloat\{2.5\}")
    test_eq("{vs}", "ASStr\{\"hello\"\}")
    test_eq("{vp}", "ASPair\{3, 4\}")
    test_eq("{vm}", "ASMix\{\"m\", 5\}")
    test_eq("{vm2}", "ASMix2\{6, [7, 8], \"t\"\}")
    test_eq("{vn}", "ASNone\{\}")
    test_eq("{vv}", "ASVec\{float3\{1.0, 2.0, 3.0\}, 9\}")
    test_eq("{vvn}", "ASVecN\{float3\{1.0, 1.0, 1.0\}, 10, \"named\"\}")
    test_eq("{vc}", "ASCircle\{11, 1.5\}")
    test_eq("{vsq}", "ASSquare\{12, 2.0\}")
    // The same thru a variable of the abstract type: it prints as what it holds.
    var av:ASValue = vs
    test_eq("{av}", "ASStr\{\"hello\"\}")
    av = vm2
    test_eq("{av}", "ASMix2\{6, [7, 8], \"t\"\}")
    av = vn
    test_eq("{av}", "ASNone\{\}")
    // Field access on the concrete types, and writes.
    test_eq(vi.i, 1)
    test_eq(vp.a + vp.b, 7)
    test_eq(vm2.v[1] + vm2.n, 14)
    test_eq(vvn.p.z, 1.0)
    test_eq(vvn.name, "named")
    var vw = ASMix { "a", 1 }
    vw.s += "b"
    vw.n++
    test_eq("{vw}", "ASMix\{\"ab\", 2\}")
    vw = ASMix { "c", 3 }
    test_eq(vw.s, "c")

    // A vector of the family type holds any member, in place.
    let vals = [ vi, vf, vs, vp, vm, vm2, vn, vv, vvn, vc, vsq ]::ASValue
    test_eq(vals.length, 11)
    test_eq("{vals[2]}", "ASStr\{\"hello\"\}")
    test_eq("{vals}", "[ASInt\{1\}, ASFloat\{2.5\}, ASStr\{\"hello\"\}, ASPair\{3, 4\}, " +
                      "ASMix\{\"m\", 5\}, ASMix2\{6, [7, 8], \"t\"\}, ASNone\{\}, " +
                      "ASVec\{float3\{1.0, 2.0, 3.0\}, 9\}, " +
                      "ASVecN\{float3\{1.0, 1.0, 1.0\}, 10, \"named\"\}, " +
                      "ASCircle\{11, 1.5\}, ASSquare\{12, 2.0\}]")
    // Unifying different members: a vector without an explicit type, if/else, switch.
    let vals2 = [ ASInt { 1 }, ASStr { "s" } ]
    test_eq("{vals2}", "[ASInt\{1\}, ASStr\{\"s\"\}]")
    let cond = vals.length > 5
    let picked = if cond: ASInt { 1 } else: ASFloat { 2.0 }
    test_eq("{picked}", "ASInt\{1\}")
    test_eq("{as_pick(0)}", "ASInt\{10\}")
    test_eq("{as_pick(1)}", "ASStr\{\"ten\"\}")
    test_eq("{as_pick(2)}", "ASNone\{\}")
    let twice = as_twice(): ASPair { 1, 2 }
    test_eq("{twice}", "[ASPair\{1, 2\}, ASPair\{1, 2\}]")
    // Passing a member where the family type is expected, and back thru a function.
    test_eq("{as_ident(vp)}", "ASPair\{3, 4\}")
    let idx = as_ident(ASStr { "x" })
    test_eq("{idx}", "ASStr\{\"x\"\}")

    // switch on the family type: exhaustive over its concrete members, with superclass
    // cases covering their subtypes, and the value promoted to the case's type inside.
    let descs = map(vals) v: as_describe(v)
    test_seq(descs, [ "int 1", "float 2.5", "str hello", "pair 3 4", "mix m 5",
                      "mix2 6 [7, 8] t", "none", "vec float3\{1.0, 2.0, 3.0\} 9",
                      "vec float3\{1.0, 1.0, 1.0\} 10", "shape 11", "shape 12" ])
    // Cases on a mid-family abstract type, and default.
    let kinds = map(vals) v:
        switch v:
            case ASShape: "shape"
            case ASVec: "vec"
            default: "other"
    test_seq(kinds, [ "other", "other", "other", "other", "other", "other", "other", "vec",
                      "vec", "shape", "shape" ])
    // Most specific case wins.
    let vecs = map(vals) v:
        switch v:
            case ASVecN: v.name
            case ASVec: "vec {v.tag}"
            default: ""
    test_seq(vecs, [ "", "", "", "", "", "", "", "vec 9", "named", "", "" ])
    // switch on a mid-family type only needs its own subtypes.
    def shape_name(s:ASShape) -> string:
        switch s:
            case ASCircle: return "circle {s.r}"
            case ASSquare: return "square {s.side}"
    test_eq(shape_name(vc), "circle 1.5")
    test_eq(shape_name(vsq), "square 2.0")
    // A switch that returns from every case is terminal (see as_describe), and values of
    // different members from the cases unify (see as_pick).

    // is: decided at runtime against the type slot, matching the tested type's subtree.
    test_seq(map(vals) v: v is ASInt, [ true, false, false, false, false, false, false, false,
                                        false, false, false ])
    test_seq(map(vals) v: v is ASVec, [ false, false, false, false, false, false, false, true,
                                        true, false, false ])
    test_seq(map(vals) v: v is ASVecN, [ false, false, false, false, false, false, false, false,
                                         true, false, false ])
    test_seq(map(vals) v: v is ASShape, [ false, false, false, false, false, false, false, false,
                                          false, true, true ])
    for(vals) v: test_true(v is ASValue)  // Compile time.
    // On the concrete type both sides are known statically.
    test_true(vi is ASInt)
    test_true(vi is ASValue)
    test_true(not (vi is ASFloat))
    test_true(vvn is ASVec)
    test_true(not (vv is ASVecN))
    // A mid-family static type tested against its subtypes.
    let vcs:ASVec = vvn
    test_true(vcs is ASVecN)
    let vcs2:ASVec = vv
    test_true(not (vcs2 is ASVecN))
    // is promotes in an if, like a class, including fields of the promoted type.
    for(vals) v:
        if v is ASPair:
            test_eq(v.a * v.b, 12)
        elif v is ASMix2:
            test_eq(v.t, "t")
        elif v is ASVec and v.tag > 9:
            test_eq(v.p.x, 1.0)
    // Promotion of a variable, and of a field path.
    var pv:ASValue = ASStr { "prom" }
    if pv is ASStr:
        test_eq(pv.s, "prom")
        pv = ASInt { 3 }  // Demotes it.
    test_eq("{pv}", "ASInt\{3\}")
    let ho = ASHolder { v: ASMix { "held", 2 }, n: 0 }
    if ho.v is ASMix:
        test_eq(ho.v.s + "{ho.v.n}", "held2")
    switch ho.v:
        case ASMix: ho.v.n = 5
        default: test_true(false)
    test_eq("{ho.v}", "ASMix\{\"held\", 5\}")

    // Dynamic dispatch on the family type, incl. a subclass served by its superclass's
    // implementation, and an abstract mid-family type.
    test_seq(map(vals) v: as_size(v), [ 1, 2, 5, 7, 6, 9, 0, 9, 10, 11, 12 ])
    test_eq(as_size(vi), 1)  // Static.
    test_eq(as_size(av), 0)
    let shapes = [ vc, vsq ]::ASShape
    test_seq(map(shapes) s: as_area(s), [ 6.75, 4.0 ])
    test_eq(as_area(vsq), 4.0)
    // Inline methods, dispatched from the root and called on the concrete types.
    let nodes = [ ASLeaf { 1, 100 }, ASBranch { 2, 3, 4 }, ASLeaf { 5, 6 } ]::ASNode
    test_seq(map(nodes) n: n.describe(), [ "leaf 1 100", "branch 2 3 4", "leaf 5 6" ])
    test_eq(ASBranch { 0, 1, 2 }.as_sum(), 3)
    test_eq(nodes[1].depth, 2)  // A root field, at the same slot in every member.
    let sums = map(nodes) n:
        switch n:
            case ASBranch: n.as_sum()
            case ASLeaf: n.val
    test_seq(sums, [ 100, 7, 6 ])

    // Generic families: each specialization has its own layout, with the type variable
    // resolved per member.
    let mi:ASMaybe<int> = ASJustInt { 5 }
    let mn:ASMaybe<int> = ASNothingInt {}
    let ms:ASMaybe<string> = ASJustStr { "js" }
    test_eq(as_or(mi, 0), 5)
    test_eq(as_or(mn, -1), -1)
    test_eq(as_or(ms, ""), "js")
    test_eq(as_or(ASNothingStr {}, "def"), "def")
    test_eq("{mi}", "ASJustInt\{5\}")
    test_eq("{mn}", "ASNothingInt\{\}")
    test_eq("{ms}", "ASJustStr\{\"js\"\}")
    test_true(mi is ASJustInt)
    test_true(not (mi is ASNothingInt))
    test_seq(map([ mi, mn ]) m: as_show(m), [ "just 5", "nothing" ])
    let msw = switch mi:
        case ASJustInt: mi.v * 2
        case ASNothingInt: 0
    test_eq(msw, 10)

    // Whatever slots the layout gives the fields, values come back as constructed, thru
    // the concrete types, the family type, and all serialized forms.
    let mfa = ASMixedA { 1, ASSub { "a", 2 } }
    let mfb = ASMixedB { ASSub { "b", 3 }, 4, 5.5 }
    let mfc = ASMixedC { "c", 6.5, 7 }
    test_eq("{mfa}", "ASMixedA\{1, ASSub\{\"a\", 2\}\}")
    test_eq("{mfb}", "ASMixedB\{ASSub\{\"b\", 3\}, 4, 5.5\}")
    test_eq("{mfc}", "ASMixedC\{\"c\", 6.5, 7\}")
    let mfs = [ mfa, mfb, mfc ]::ASMixedFam
    test_eq("{mfs}", "[ASMixedA\{1, ASSub\{\"a\", 2\}\}, ASMixedB\{ASSub\{\"b\", 3\}, 4, 5.5\}, " +
                     "ASMixedC\{\"c\", 6.5, 7\}]")
    let mfsum = map(mfs) m:
        switch m:
            case ASMixedA: m.k + m.sub.n
            case ASMixedB: m.sub.n + m.k + int(m.f)
            case ASMixedC: int(m.f) + m.k + m.t.length
    test_seq(mfsum, [ 3, 12, 14 ])
    var mfw:ASMixedFam = mfb
    if mfw is ASMixedB:
        mfw.sub.s += "x"
        mfw.k++
        mfw.f *= 2.0
    test_eq("{mfw}", "ASMixedB\{ASSub\{\"bx\", 3\}, 5, 11.0\}")
    let pmf, pmferr = parse_data(typeof [ASMixedFam], "{mfs}")
    test_eq(pmferr, nil)
    test_eq("{pmf}", "{mfs}")
    let fmf, fmferr = flexbuffers_binary_to_value(typeof [ASMixedFam],
                                                  flexbuffers_value_to_binary(mfs))
    test_eq(fmferr, nil)
    test_eq("{fmf}", "{mfs}")

    // Family values as fields: defaults, writes of another member, nested in a struct and
    // in a class, and the whole struct copied.
    let h = ASHolder {}
    test_eq("{h}", "ASHolder\{ASInt\{7\}, 1\}")
    h.v = ASStr { "now" }
    test_eq("{h}", "ASHolder\{ASStr\{\"now\"\}, 1\}")
    h.v = ASNone {}
    test_eq("{h.v}", "ASNone\{\}")
    var o = ASOuter { ASPair { 1, 2 }, 3 }
    test_eq("{o}", "ASOuter\{ASPair\{1, 2\}, 3\}")
    let o2 = o
    o.v = ASMix2 { 4, [ 5 ], "six" }
    test_eq("{o}", "ASOuter\{ASMix2\{4, [5], \"six\"\}, 3\}")
    test_eq("{o2}", "ASOuter\{ASPair\{1, 2\}, 3\}")
    let os = [ o, o2 ]
    test_eq("{os[0].v}", "ASMix2\{4, [5], \"six\"\}")
    let os1 = os[1]
    switch os1.v:
        case ASPair: test_eq(os1.v.b, 2)
        default: test_true(false)
    // Vectors: push, pop, copy, iteration, and element writes.
    let vec = []::ASValue
    vec.push(ASInt { 1 })
    vec.push(ASStr { "two" })
    vec.push(ASMix { "three", 3 })
    test_eq("{vec}", "[ASInt\{1\}, ASStr\{\"two\"\}, ASMix\{\"three\", 3\}]")
    vec[0] = ASFloat { 1.5 }
    test_eq("{vec[0]}", "ASFloat\{1.5\}")
    let vcopy = copy(vec)
    vcopy[1] = ASNone {}
    test_eq("{vec[1]}", "ASStr\{\"two\"\}")
    test_eq("{vcopy[1]}", "ASNone\{\}")
    let popped = vec.pop()
    test_eq("{popped}", "ASMix\{\"three\", 3\}")
    test_eq(vec.length, 2)
    var total = 0
    for(vals) v: total += as_size(v)
    test_eq(total, 72)
    // Equality is by value, over all slots, so different members are never equal, and
    // the same member with the same fields is.
    test_true(ASInt { 1 } == ASInt { 1 })
    test_true(ASInt { 1 } != ASInt { 2 })
    test_true(ASPair { 1, 2 } != ASPair { 2, 1 })
    let ev1:ASValue = ASInt { 1 }
    let ev2:ASValue = ASFloat { 1.0 }
    test_true(ev1 != ev2)
    test_true(ev1 == vi)
    test_true(equal([ ASInt { 1 }, ASNone {} ]::ASValue, [ ASInt { 1 }, ASNone {} ]::ASValue))
    test_true(not equal([ ASInt { 1 } ]::ASValue, [ ASNone {} ]::ASValue))

    // References held by members survive copies and get released: a vector member's
    // vector is shared by value copies of it.
    let shared = [ 1 ]
    var mv:ASValue = ASMix2 { 0, shared, "" }
    let mv2 = mv
    shared.push(2)
    switch mv2:
        case ASMix2: test_seq(mv2.v, [ 1, 2 ])
        default: test_true(false)
    mv = ASInt { 0 }
    test_seq(shared, [ 1, 2 ])

    // Serialization: the printed form parses back, as the members, thru the abstract type.
    let pvals, perr = parse_data(typeof [ASValue], "{vals}")
    test_eq(perr, nil)
    test_eq("{pvals}", "{vals}")
    let pone, ponerr = parse_data(typeof [ASOuter], "[ ASOuter \{ ASStr \{ \"p\" \}, 4 \} ]")
    test_eq(ponerr, nil)
    test_eq("{pone}", "[ASOuter\{ASStr\{\"p\"\}, 4\}]")
    // Missing fields get their defaults, an abstract type can't be constructed.
    let pdef, pdeferr = parse_data(typeof ASHolder, "ASHolder \{\}")
    test_eq(pdeferr, nil)
    test_eq("{pdef}", "ASHolder\{ASInt\{7\}, 1\}")
    let pabs, pabserr = parse_data(typeof [ASOuter], "[ ASOuter \{ ASValue \{\}, 4 \} ]")
    test_true(pabs == nil)
    test_true(pabserr and pabserr.find_string("abstract") >= 0)
    // flexbuffers: the member is named by _type, like a subclass of an object.
    let flex = flexbuffers_value_to_binary(os)
    let fjson = assert flexbuffers_binary_to_json(flex, false, "")
    test_eq(fjson, "[ \{ v: \{ _type: \"ASMix2\", n: 4, t: \"six\", v: [ 5 ] \}, x: 3 \}, " +
                   "\{ v: \{ _type: \"ASPair\", a: 1, b: 2 \}, x: 3 \} ]")
    let fval, fverr = flexbuffers_binary_to_value(typeof [ASOuter], flex)
    test_eq(fverr, nil)
    test_eq("{fval}", "{os}")
    let fh, fherr = flexbuffers_binary_to_value(typeof ASHolder,
        assert flexbuffers_json_to_binary("\{ v: \{ _type: \"ASStr\", s: \"fs\" \} \}"))
    test_eq(fherr, nil)
    test_eq("{fh}", "ASHolder\{ASStr\{\"fs\"\}, 1\}")
    let fmiss, fmisserr = flexbuffers_binary_to_value(typeof ASHolder,
        assert flexbuffers_json_to_binary("\{ v: \{ s: \"fs\" \} \}"))
    test_true(fmiss == nil)
    test_true(fmisserr and fmisserr.find_string("abstract") >= 0)
    // lobster binary: members marked serializable go by their id.
    let sh = ASSerHolder { ASSerA { 1, "a" }, ASSerB { 2.5, [ 3 ] },
                           [ ASSerB { 0.5, [] }, ASSerA { 4, "d" } ] }
    let lb = lobster_value_to_binary(sh)
    let lval, lerr = lobster_binary_to_value(typeof ASSerHolder, lb)
    test_eq(lerr, nil)
    test_eq("{lval}", "{sh}")
    test_true(equal(lval, sh))

    // Compile errors (see typecheck_recovery.lobster for the mechanism).
    guard not vm_compiled_mode()
    // An abstract struct can't be constructed.
    expect_errors("""
        abstract struct V
        struct A : V
            x:int
        let v = V {}
        return 1
        """, [ "cannot instantiate abstract class/struct `V`" ])
    // A switch must cover every concrete member, and only takes types.
    expect_errors("""
        abstract struct V
        struct A : V
            x:int
        struct B : V
            y:int
        let v:V = A { 1 }
        let r = switch v:
            case A: v.x
        return r
        """, [ "no case applies to `B`" ])
    expect_errors("""
        abstract struct V
        struct A : V
            x:int
        let v:V = A { 1 }
        switch v:
            case 1: print(v)
            default: print(0)
        return 1
        """, [ "non-type value in switch on type" ])
    // A struct outside a family has no dynamic type to switch on or dispatch with.
    expect_errors("""
        struct P:
            x:int
        struct Q : P
            y:int
        let p = P { 1 }
        switch p:
            case P: print(p)
        return 1
        """, [ "switch value must be int / float / string / class / abstract struct" ])
    // A member can't hold a value of its own family, since no layout fits that.
    expect_errors("""
        abstract struct V
        struct A : V
            x:int
        struct P : V
            l:V
        let p = P { A { 1 } }
        return 1
        """, [ "struct `P` cannot contain `V` (self-referential)" ])
    // Struct and class don't mix in a family.
    expect_errors("""
        abstract struct V
        class C : V
            x:int
        return 1
        """, [ "class/struct must match parent" ])
    // Only members of a family can be serializable structs.
    expect_errors("""
        struct P:
            attribute serializable
            x:int
        return 1
        """, [ "serializable attribute only for non-generic non-abstract classes and structs" ])

abstract_structs_tests()
