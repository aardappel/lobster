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
    def operator<(o:DCNum): return n < o.n
    def operator[](i:int): return n + i
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
    // is: true only for the exact dynamic type.
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
    // And on structs.
    test_eq((DCVal { 1 } + DCVal { 2 }).v, 3)
    test_eq((~DCVal { 0 }).v, -1)

    // member: class member scoped to a method, persists per object.
    let cnt1 = DCCounter {}
    let cnt2 = DCCounter {}
    test_eq(cnt1.bump(), 11)
    test_eq(cnt1.bump(), 12)
    test_eq(cnt2.bump(), 11)  // Separate state per instance.

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

classes_tests()
