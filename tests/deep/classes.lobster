import std


// User defined types: classes, inheritance, dynamic dispatch, generics,
// constructor functions, operator overloading, member declarations.

// A small hierarchy for dispatch tests (top level so more files could use it).
abstract class DCShape:
    name = "shape"
    def describe(): return "abstract"

class DCCircle : DCShape
    r = 2
    def area2x(): return r * r * 6  // "pi" == 3 for test purposes.
    def describe(): return "circle"

class DCSquare : DCShape
    side = 3
    def area2x(): return side * side * 2
    def describe(): return "square"
    def sup(): return "sq" + super describe()

// Generic class, named and structural specializations, subclassing.
class DCBox<T>:
    contents:T
    def get(): return contents

class DCIntBox = DCBox<int>

class DCLabeledBox<T> : DCBox<T>
    label:string

// Constructor functions: once declared, {} construction is off-limits
// elsewhere, and all construction goes through them.
class DCRange:
    lo:int
    hi:int

constructor DCRange(single:int):
    return DCRange { single, single }

constructor DCRange(l:int, h:int):
    return DCRange { lo: l, hi: h }

// Operator overloading, on a class and on a struct.
class DCNum:
    n:int
    def operator+(o:DCNum): return DCNum { n + o.n }
    def operator-(): return DCNum { -n }
    def operator*(o:int): return DCNum { n * o }
    def operator==(o:DCNum): return n == o.n
    def operator!=(o:DCNum): return n != o.n
    def operator<(o:DCNum): return n < o.n
    def operator<<(i:int): return DCNum { n << i }
    def operator&(o:DCNum): return DCNum { n & o.n }
    def operator|(o:DCNum): return DCNum { n | o.n }
    def operator[](i:int): return n + i
    def operator[](s:string): return "{n}{s}"
    def operator+=(o:DCNum): n += o.n

struct DCVal:
    v:int
    def operator+(o:DCVal): return DCVal { v + o.v }
    def operator~(): return DCVal { ~v }

// Classes with a `member` declaration inside a method.
class DCCounter:
    base = 10
    def bump():
        member steps = 0
        steps++
        return base + steps
    def bumpf():
        // member_frame resets when a frame passes without this method being
        // called; with no frames advancing here it persists like member.
        member_frame fsteps = 100
        fsteps++
        return fsteps
    def bumpt():
        member typed:float = 0.5  // With an explicit type.
        typed += 1.0
        return typed

// Forward declaration: usable in types before the full definition exists.
class DCFwd
class DCFwdUser:
    link:DCFwd?
class DCFwd:
    v = 7

// Variable pre-declaration: declared, defined further down, usable in between
// only from code that runs after the definition.
class DCWorld
let dc_world:DCWorld
def dc_world_n(): return dc_world.n
class DCWorld:
    n = 3
let dc_world:DCWorld = DCWorld {}

// Private declarations are usable within this file.
private class DCPriv:
    private x = 1
    private def get(): return x
    def double(): return get() * 2

// A hierarchy with an abstract class in the middle, for exhaustive switches:
//
//   (TSA)
//    |  \
//   TSB (TSC)
//        |  \
//       TSD TSE
//            |
//           TSF
abstract class TSA:
    a = 0
class TSB : TSA
    b = 1
abstract class TSC : TSA
    c = 2
class TSD : TSC
    d = 3
class TSE : TSC
    e = 4
class TSF : TSE
    f = 5

// Generic constructor function.
class DCGBox<T>:
    val:T

constructor DCGBox<T>(x:T, y:T):
    return DCGBox<T> { x + y }

// Constructor functions on a class hierarchy.
class DCCtorBase:
    tag:int
class DCCtorSub : DCCtorBase
    extra = 0

constructor DCCtorBase(n:int):
    return DCCtorBase { n }

constructor DCCtorSub(n:int):
    return DCCtorSub { n * 2 }

// Dispatch corner cases: recursion, super, multi-return, generic subclasses.
abstract class DD0:
    def dd_ad(): return 10
class DD1 : DD0
    a:int = 0
    def dd_r(c:int) -> int: return c
    def dd_mr(): return 1, "a"
    def dd_sup(): return 1
    def dd_sni(): return 12
class DD2 : DD1
    b:int = 0
    def dd_r(c:int) -> int: return if c: dd_r(c - 1) else: (DD1 {}).dd_r(c)
    def dd_mr(): return 2, "b"
    def dd_sup(): return 1 + super dd_sup()
    def dd_ad(): return 11
class DD3<T> : DD1
    c:T
    def dd_f(x:int): return x + length(c)

def classes_tests():
    suite("classes")

    // Field access, defaults, tagged constructors, field writes.
    class Point:
        x = 1
        y = 2
        z:int
    let pt = Point { 3 }
    test_eq(pt.x, 1)
    test_eq(pt.y, 2)
    test_eq(pt.z, 3)
    let pt2 = Point { z: 4, y: 9 }  // Tags, any order, defaults overridable.
    test_eq(pt2.y, 9)
    test_eq(pt2.z, 4)
    pt.x = 10
    pt.x++
    test_eq(pt.x, 11)

    // Reference semantics and equality.
    let pref = pt
    pref.y = 20
    test_eq(pt.y, 20)
    test_true(pt == pref)
    test_true(pt != pt2)
    test_true(equal(Point { 5 }, Point { 5 }))
    // copy() gives an independent object (shallow).
    let pcopy = copy(pt)
    pcopy.y = 30
    test_eq(pt.y, 20)

    // Inheritance: subclass constructor takes superclass fields first.
    class Sub : Point
        w = 0
    let sub = Sub { 7 }
    test_eq(sub.z, 7)
    test_eq(sub.w, 0)
    test_eq(sub.x, 1)

    // Methods, incl. defined outside the class with :: syntax.
    def scaled(p::Point, f:int): return (x + y + z) * f
    test_eq(pt2.scaled(2), (1 + 9 + 4) * 2)

    // Dynamic dispatch through a superclass-typed value.
    let shapes = [ DCCircle {}, DCSquare {} ]::DCShape
    test_eq(shapes[0].describe(), "circle")
    test_eq(shapes[1].describe(), "square")
    let areas = map(shapes) s:
        switch s:  // switch on class types, exhaustive over subclasses.
            case DCCircle: s.area2x()
            case DCSquare: s.area2x()
    test_seq(areas, [ 24, 18 ])
    // Superclass method via super.
    test_eq((DCSquare {}).sup(), "sqabstract")
    // Fields from the abstract base.
    test_eq(shapes[0].name, "shape")
    // is: true only for the exact dynamic type, not for a sibling type.
    test_true(shapes[0] is DCCircle)
    test_true(not (shapes[0] is DCSquare))
    // switch cases may use a superclass to cover several subclasses.
    let kinds = map(shapes) s:
        switch s:
            case DCShape: "some shape"
    test_seq(kinds, [ "some shape", "some shape" ])

    // Dynamic dispatch mixed with specialization on further args.
    class SpecA:
        a = 1
        def add(c): return c + a
    class SpecB : SpecA
        def add(c): return c + c + a
    let spec:SpecA = SpecB {}
    test_eq(spec.add(3), 7)
    test_eq(spec.add("x"), "xx1")

    // More dispatch corner cases (classes at top level, methods would
    // otherwise shadow same-named methods of other classes in this scope).
    // The generic subclass must be instantiated before methods dispatch over
    // its superclass, so its specialization takes part in the vtables.
    let dd3 = DD3<string> { c: "hello" }
    test_eq(dd3.dd_f(10), 15)   // Method on a generic subclass, generic "this".
    let dd:DD1 = DD2 {}
    test_eq(dd.dd_r(10), 0)     // Recursive dynamic dispatch.
    test_eq(dd.dd_sup(), 2)     // super call one level up.
    test_eq(dd.dd_ad(), 11)     // Override of a method from above the dispatch root.
    let ddm1, ddm2 = dd.dd_mr() // Dispatch with multiple return values.
    test_eq(ddm1, 2)
    test_eq(ddm2, "b")
    let dd0:DD0 = DD1 {}
    test_eq(dd0.dd_sni(), 12)   // Dispatch root whose sibling has no override.
    let dd3i:DD1 = dd3
    test_true(dd3i is DD3<string>)

    // Exhaustive switches over the TS hierarchy (abstract in the middle).
    let ts:[TSA] = [ TSB {}, TSD {}, TSE {}, TSF {} ]
    // One case per concrete class, no default needed.
    let ts1 = map(ts) t:
        switch t:
            case TSB: t.b
            case TSD: t.d
            case TSE: t.e
            case TSF: t.f
    test_seq(ts1, [ 1, 3, 4, 5 ])
    // A superclass case covers its subclasses.
    let ts2 = map(ts) t:
        switch t:
            case TSB: t.b
            case TSD: t.d
            case TSE: t.e  // Also covers TSF.
    test_seq(ts2, [ 1, 3, 4, 4 ])
    // An abstract base case may cover everything below it.
    let ts3 = map(ts) t:
        switch t:
            case TSB: t.b
            case TSC: t.c
    test_seq(ts3, [ 1, 2, 2, 2 ])
    // default may replace any number of cases; the most specific case wins.
    let ts4 = map(ts) t:
        switch t:
            default: t.a
            case TSF: t.f
    test_seq(ts4, [ 0, 0, 0, 5 ])
    // The same exhaustiveness applies to dynamic dispatch: TSF omitted, so it
    // falls back to the TSE implementation.
    def ts_name(t::TSB): return "b"
    def ts_name(t::TSD): return "d"
    def ts_name(t::TSE): return "e"
    test_seq(map(ts): ts_name(_), [ "b", "d", "e", "e" ])

    // Generic classes.
    let bi = DCIntBox { 42 }
    test_eq(bi.get(), 42)
    let bs = DCBox<string> { "s" }
    test_eq(bs.get(), "s")
    let bg = DCBox<float> { 1.5 }  // Unnamed specialization, constructed directly.
    test_eq(bg.get(), 1.5)
    let bpick = DCBox { 43 }  // Generic type picks a matching named specialization.
    test_eq(bpick.get(), 43)
    let lb = DCLabeledBox<int> { 1, "one" }
    test_eq(lb.get() + lb.label.length, 4)

    // Constructor functions.
    let r1 = DCRange(5)
    test_eq(r1.lo, 5)
    test_eq(r1.hi, 5)
    let r2 = DCRange(1, 9)
    test_eq(r2.hi - r2.lo, 8)
    // Generic constructor function, specializing per type.
    test_eq(DCGBox(1, 2).val, 3)
    test_eq(DCGBox("a", "b").val, "ab")
    test_eq(DCGBox<float>(1.0, 2.0).val, 3.0)

    // Operator overloading on classes.
    let n3 = DCNum { 3 }
    let n4 = DCNum { 4 }
    test_true(n3 + n4 == DCNum { 7 })
    test_true(-n3 == DCNum { -3 })
    test_true(n3 * 5 == DCNum { 15 })
    test_true(n3 < n4)
    test_eq(n3[2], 5)
    var nacc = DCNum { 1 }
    nacc += n4  // Mutates in place via the overload.
    test_true(nacc == DCNum { 5 })
    // Bitwise, shift, inequality and non-int index overloads.
    test_true(n3 != n4)
    test_eq((n3 << 2).n, 12)
    test_eq((n3 & n4).n, 0)
    test_eq((n3 | n4).n, 7)
    test_eq(n3["s"], "3s")
    // And on structs.
    test_eq((DCVal { 1 } + DCVal { 2 }).v, 3)
    test_eq((~DCVal { 0 }).v, -1)

    // member: class member scoped to a method, persists per object.
    let cnt1 = DCCounter {}
    let cnt2 = DCCounter {}
    test_eq(cnt1.bump(), 11)
    test_eq(cnt1.bump(), 12)
    test_eq(cnt2.bump(), 11)  // Separate state per instance.
    test_eq(cnt1.bumpf(), 101)
    test_eq(cnt1.bumpf(), 102)  // member_frame persists within one frame.
    test_eq(cnt1.bumpt(), 1.5)
    test_eq(cnt1.bumpt(), 2.5)

    // Reference-typed field defaults are evaluated per instance, not shared.
    class VHolder:
        v = [ 1 ]
    let vh1 = VHolder {}
    let vh2 = VHolder {}
    test_true(vh1.v != vh2.v)
    vh1.v.push(2)
    test_seq(vh2.v, [ 1 ])

    // A class need not have any fields.
    class NoFields:
        def just_a_method(): return 99
    test_eq((NoFields {}).just_a_method(), 99)

    // super with call arguments.
    class SupArg:
        def scale2(x:int): return x * 2
    class SupArgSub : SupArg
        def scale2(x:int): return super scale2(x + 1)
    test_eq((SupArgSub {}).scale2(3), 8)

    // Vector type parameters are invariant by default, but covariance is
    // allowed where no aliasing can occur: fresh constructors and unions.
    class CoA:
        a = 1
    class CoB : CoA
        b = 2
    let coas = [ CoA {} ]
    let cobs = [ CoB {} ]
    test_eq(append(coas, cobs).length, 2)
    let comixed:[CoA] = [ CoB {} ]
    test_eq(comixed.length, 1)
    def co_take(ys:[CoA]): return ys.length
    test_eq(co_take([ CoB {} ]), 1)
    test_eq(co_take(if coas[0].a == 1: [ CoA {} ] else: [ CoB {} ]), 1)

    // Constructor functions may have per-subclass overloads.
    test_eq(DCCtorBase(1).tag, 1)
    test_eq(DCCtorSub(2).tag, 4)
    test_true(DCCtorSub(2) is DCCtorSub)

    // Auto constructors: {} without type in typed contexts.
    class Linked:
        next:Linked?
        tag = 0
    let links:[Linked] = [
        { nil },
        { { nil, tag: 2 } },
    ]
    test_eq((assert links[1].next).tag, 2)
    // Fields with defaults are not positional constructor args, only tags.
    def takes_linked(l:Linked): return l.tag
    test_eq(takes_linked({ nil, tag: 9 }), 9)

    // String conversion of objects.
    test_eq("{DCVal { 6 }}", "DCVal\{6\}")

    // Forward-declared class and pre-declared variable in action.
    let fwd = DCFwdUser { DCFwd {} }
    test_eq((assert fwd.link).v, 7)
    test_eq(dc_world_n(), 3)

    // Private class with private members, used within this file.
    test_eq((DCPriv {}).double(), 2)

classes_tests()
