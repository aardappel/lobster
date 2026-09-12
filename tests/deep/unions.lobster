import std

// The sugar for the discriminated union use of abstract classes (see classes.lobster) and
// abstract structs (abstract_structs.lobster): `union` declarations, and switch cases that
// name the fields of the value switched on, `case T::` and `case T(names)`.

// The examples of the language reference.
union class Foo:
    Bar(b:int)
    Baz(f:float)

// Members with several fields, none (with or without parens), defaults, and a private field.
union class UShape:
    UCircle(r:float)
    URect(w:float, h:float)
    UPoint
    UOrigin()
    UTagged(name = "tag", private n = 0)

// A subclass of a member, covered by the member's case.
class UBigCircle : UCircle
    label:string

// Members are classes, so get methods like any other: dispatch needs one per member.
def ut_area(s::UCircle) -> float: return 3.0 * r * r
def ut_area(s::URect) -> float: return w * h
def ut_area(s::UPoint) -> float: return 0.0
def ut_area(s::UOrigin) -> float: return 0.0
def ut_area(s::UTagged) -> float: return float(n)

// A hierarchy with inherited fields and an inline method, for what the case forms see.
abstract class UTA:
    a = 1
class UTB : UTA
    b = 2
abstract class UTC : UTA
    c = 3
class UTD : UTC
    d = 4
class UTE : UTC
    e = 5
    def ut_total(): return a + c + e

// A struct union is an abstract struct family: members are inline values, of any mix of
// field kinds, that carry their type in a hidden slot.
union struct UV:
    UVI(i:int)
    UVF(f:float, g:float)
    UVS(s:string, n:int)
    UVN
    UVP(p:float2, tag:int)

def uv_size(v::UVI): return i
def uv_size(v::UVF): return int(f + g)
def uv_size(v::UVS): return s.length + n
def uv_size(v::UVN): return 0
def uv_size(v::UVP): return tag

class UVHolder:
    v:UV

union class UOpt:
    USome(v:string?)
    UNone

union class UGeo:
    UAt(p:float2)
    UBox(lo:float2, hi:float2)

var ut_touched = 0

// Assignment thru :: and thru a pattern variable goes to the value's field.
def ut_grow(s:UShape):
    switch s:
        case UCircle:: r += 1.0
        case URect:: w += 1.0
        case UPoint: ut_touched++
        case UOrigin: ut_touched++
        case UTagged(_, n): n += 1

def ut_set(s:UShape, v:float):
    switch s:
        case UCircle(rad): rad = v
        case URect(w): w = v
        case UPoint: ut_touched++
        case UOrigin: ut_touched++
        case UTagged(_, n): n = int(v)

// A switch on a field of `this`, named thru its own ::.
class UHolder:
    s:UShape
    def ut_kind() -> string:
        switch s:
            case UCircle:: return "circle {r}"
            case URect(w, h): return "rect {w}x{h}"
            default: return "other"

def unions_tests():
    suite("unions")

    // A union member is a class with the union as its abstract superclass.
    let foos:[Foo] = [ Bar { 1 }, Baz { 2.5 } ]
    test_true(foos[0] is Bar)
    test_true(foos[1] is Baz)
    test_true(foos[0] is Foo)
    test_true(not (foos[0] is Baz))
    test_eq("{foos}", "[Bar\{1\}, Baz\{2.5\}]")
    // A switch on it is exhaustive over the members.
    let foovals = map(foos) f:
        switch f:
            case Bar: float(f.b)
            case Baz: f.f
    test_seq(foovals, [ 1.0, 2.5 ])

    // Members: several fields, none, with or without parens, defaults (also set by tag),
    // and a private field. A subclass of a member is one of it.
    let shapes:[UShape] = [ UCircle { 1.0 }, URect { 2.0, 3.0 }, UPoint {}, UOrigin {},
                            UTagged {}, UTagged { n: 5 }, UBigCircle { 2.0, "big" } ]
    test_seq(map(shapes): ut_area(_), [ 3.0, 6.0, 0.0, 0.0, 0.0, 5.0, 12.0 ])
    let tagged = UTagged { name: "t" }
    test_eq(tagged.name, "t")
    test_eq(tagged.n, 0)
    test_eq("{shapes}", "[UCircle\{1.0\}, URect\{2.0, 3.0\}, UPoint\{\}, UOrigin\{\}, " +
                        "UTagged\{\"tag\", 0\}, UTagged\{\"tag\", 5\}, UBigCircle\{2.0, \"big\"\}]")
    test_true(shapes[6] is UCircle)
    test_true(shapes[6] is UShape)
    let kinds = map(shapes) s:
        switch s:
            case UCircle: "circle"
            case URect: "rect"
            case UPoint: "point"
            case UOrigin: "origin"
            case UTagged: "tagged"
    test_seq(kinds, [ "circle", "rect", "point", "origin", "tagged", "tagged", "circle" ])

    // A union declared inside a function.
    union class ULocal:
        ULA(x:int)
        ULB(y:int, z:int)
    let locals:[ULocal] = [ ULA { 1 }, ULB { 2, 3 } ]
    let localsum = map(locals) l:
        switch l:
            case ULA(x): x
            case ULB(y, z): y + z
    test_seq(localsum, [ 1, 5 ])

    // case T:: puts the fields of the value in scope by name, as `this::` does in a method.
    let areas = map(shapes) s:
        switch s:
            case UCircle:: 3.0 * r * r
            case URect::
                let ar = w * h
                ar
            case UPoint:: 0.0
            case UOrigin:: 0.0
            case UTagged:: float(n)
    test_seq(areas, [ 3.0, 6.0, 0.0, 0.0, 0.0, 5.0, 12.0 ])
    // Inherited fields too, and methods, which get the value as receiver.
    let uts:[UTA] = [ UTB {}, UTD {}, UTE {} ]
    let utsums = map(uts) t:
        switch t:
            case UTB:: a + b
            case UTD:: a + c + d
            case UTE:: ut_total()
    test_seq(utsums, [ 3, 8, 9 ])
    // Naming the value explicitly still works, of course.
    let utas = map(uts) t:
        switch t:
            case UTB:: t.a + b
            case UTC:: t.a * c
    test_seq(utas, [ 3, 3, 3 ])
    // Assignment goes to the value's field.
    let mutc = UCircle { 1.0 }
    ut_grow(mutc)
    test_eq(mutc.r, 2.0)
    let mutr = URect { 1.0, 1.0 }
    ut_grow(mutr)
    test_eq(mutr.w, 2.0)
    let mutt = UTagged {}
    ut_grow(mutt)
    test_eq(mutt.n, 1)
    ut_grow(UPoint {})
    test_eq(ut_touched, 1)
    // A switch on a field (a path from a variable) works the same, from a method (where the
    // field is one of `this`) or outside.
    let holder = UHolder { URect { 2.0, 5.0 } }
    test_eq(holder.ut_kind(), "rect 2.0x5.0")
    holder.s = UCircle { 1.5 }
    test_eq(holder.ut_kind(), "circle 1.5")
    let hr = switch holder.s:
        case UCircle:: r * 2.0
        case URect(w, h): w + h
        default: -1.0
    test_eq(hr, 3.0)
    // Nested: an inner switch on another value has the fields of both in scope.
    let foo0 = foos[0]
    let nested = switch holder.s:
        case UCircle::
            switch foo0:
                case Bar:: r + float(b)
                case Baz:: r + f
        default: 0.0
    test_eq(nested, 2.5)
    // The same object in scope twice (an argument, and a case on it) is not ambiguous.
    def ut_desc(t::UTC) -> string:
        switch t:
            case UTD:: return "d {a} {c} {d}"
            case UTE:: return "e {a} {c} {e}"
    test_eq(ut_desc(UTD {}), "d 1 3 4")
    test_eq(ut_desc(UTE {}), "e 1 3 5")
    // A lambda in the body sees the fields too.
    let lam = switch holder.s:
        case UCircle:: fold(map(3) i: r * float(i), 0.0) acc, x: acc + x
        default: 0.0
    test_eq(lam, 4.5)
    // On a struct union (an abstract struct family), whose members print as what they are.
    let uvs = [ UVI { 3 }, UVF { 1.5, 2.5 }, UVS { "abc", 2 }, UVN {},
                UVP { float2 { 1.0, 2.0 }, 7 } ]::UV
    test_eq("{uvs}", "[UVI\{3\}, UVF\{1.5, 2.5\}, UVS\{\"abc\", 2\}, UVN\{\}, " +
                     "UVP\{float2\{1.0, 2.0\}, 7\}]")
    test_true(uvs[0] is UVI)
    test_true(uvs[2] is UV)
    test_true(not (uvs[3] is UVI))
    test_seq(map(uvs): uv_size(_), [ 3, 4, 5, 0, 7 ])
    let uvdesc = map(uvs) v:
        switch v:
            case UVI:: "i {i}"
            case UVF(x, y): "f {x + y}"
            case UVS:: "s {s} {n}"
            case UVN: "n"
            case UVP(p, tag): "p {p.x} {tag}"
    test_seq(uvdesc, [ "i 3", "f 4.0", "s abc 2", "n", "p 1.0 7" ])
    // Assignment thru :: or a pattern variable modifies the struct variable in place, also
    // when that is a field.
    var uvm:UV = UVF { 1.0, 2.0 }
    switch uvm:
        case UVF:: f += 10.0
        case UVI(i): i += 1
        case UVS:: n += 1
        case UVN: ut_touched++
        case UVP:: tag += 1
    test_eq("{uvm}", "UVF\{11.0, 2.0\}")
    let uvh = UVHolder { UVI { 5 } }
    switch uvh.v:
        case UVI(i): i += 1
        case UVF:: f += 10.0
        case UVS:: n += 1
        case UVN: ut_touched++
        case UVP:: tag += 1
    test_eq("{uvh.v}", "UVI\{6\}")

    // case T(names) names the fields of T in order: as many as wanted, `_` for one not
    // needed, by any name, including the field's own.
    let pv = map(shapes) s:
        switch s:
            case UCircle(rad): rad
            case URect(w, h): w * h
            case UPoint(): 0.0
            case UOrigin: 0.0
            case UTagged(_, n): float(n)
    test_seq(pv, [ 1.0, 6.0, 0.0, 0.0, 0.0, 5.0, 2.0 ])
    // Inherited fields come first, as in the constructor.
    let pvi = map(uts) t:
        switch t:
            case UTB(x): x
            case UTD(x, y, z): x + y + z
            case UTE(_, _, z): z
    test_seq(pvi, [ 1, 8, 5 ])
    // A pattern variable is the field: assignment goes to it, and flow typing narrows it.
    let pc = UCircle { 1.0 }
    ut_set(pc, 9.0)
    test_eq(pc.r, 9.0)
    let pr = URect { 1.0, 1.0 }
    ut_set(pr, 9.0)
    test_eq(pr.w, 9.0)
    test_eq(pr.h, 1.0)
    let pt = UTagged {}
    ut_set(pt, 9.0)
    test_eq(pt.n, 9)
    let opts:[UOpt] = [ USome { "abc" }, USome { nil }, UNone {} ]
    let optlens = map(opts) o:
        switch o:
            case USome(v): if v: v.length else: -1
            case UNone: -2
    test_seq(optlens, [ 3, -1, -2 ])
    // Of a struct type.
    let geos:[UGeo] = [ UAt { float2 { 1.0, 2.0 } },
                        UBox { float2 { 0.0, 0.0 }, float2 { 1.0, 1.0 } } ]
    let geosum = map(geos) g:
        switch g:
            case UAt(p): p.x + p.y
            case UBox(lo, hi): hi.x - lo.x + hi.y - lo.y
    test_seq(geosum, [ 3.0, 2.0 ])
    // Names are per case, so siblings and nested switches may reuse them, and a lambda in
    // the body sees them.
    let reuse = map(shapes) s:
        switch s:
            case UCircle(x): x
            case URect(x, y): x * y
            case UTagged(x, y): float(x.length + y)
            default: 0.0
    test_seq(reuse, [ 1.0, 6.0, 0.0, 0.0, 3.0, 8.0, 2.0 ])
    let inlam = switch holder.s:
        case UCircle(rad): fold(map(3) i: rad * float(i), 0.0) acc, x: acc + x
        default: 0.0
    test_eq(inlam, 4.5)
    let nestp = switch holder.s:
        case UCircle(rad):
            switch foo0:
                case Bar(v): rad + float(v)
                case Baz(v): rad + v
        default: 0.0
    test_eq(nestp, 2.5)
    // Next to fields in scope thru a :: argument.
    def ut_pat(t::UTC) -> int:
        switch t:
            case UTD(_, _, dd): return a + c + dd
            case UTE(_, _, ee): return t.ut_total() + ee
    test_eq(ut_pat(UTD {}), 8)
    test_eq(ut_pat(UTE {}), 14)

    // Compile errors (see typecheck_recovery.lobster for the mechanism).
    guard not vm_compiled_mode()
    // The union itself is abstract, and a switch on it is exhaustive over the members.
    expect_errors("""
        union class U:
            A(x:int)
        let u = U {}
        return 1
        """, [ "cannot instantiate abstract class/struct `U`" ])
    expect_errors("""
        union class U:
            A(x:int)
            B(y:int)
        let u:U = A { 1 }
        return switch u:
            case A: u.x
        """, [ "no case applies to `B`" ])
    // A field needs a type or a default, and members are types like any other.
    expect_errors("""
        union class U:
            A(x)
        return 1
        """, [ "must specify either type or default value" ])
    expect_errors("""
        union class U:
            A(x:int)
            A(y:int)
        return 1
        """, [ "type previously declared as specialization: A" ])
    expect_errors("""
        class A:
            x:int
        union class U:
            A(y:int)
        return 1
        """, [ "type previously declared as specialization: A" ])
    // `union` needs `class` or `struct`, is not generic, and its members are an indented list.
    expect_errors("""
        union U:
            A(x:int)
        return 1
        """, [ "`class` or `struct` expected after `union`, found `U`" ], 1)
    expect_errors("""
        union class U<T>:
            A(x:T)
        return 1
        """, [ "`:` expected, found `<`" ], 1)
    expect_errors("union class U:\nreturn 1\n", [ "`indentation` expected, found `linefeed`" ], 1)
    // An unclosed field list continues the line, like any bracket does.
    expect_errors("""
        union class U:
            A(x:int
            B(y:int)
        return 1
        """, [ "`,` expected, found `B`", "unmatched \'(\' at end of file" ])
    // A struct union is an abstract struct family, with its restrictions.
    expect_errors("""
        union struct V:
            A(x:int)
            P(l:V)
        let p = P { A { 1 } }
        return 1
        """, [ "struct `P` cannot contain `V` (self-referential)" ])
    expect_errors("""
        union struct V:
            A(x:int)
        class C : V
            y:int
        return 1
        """, [ "class/struct must match parent" ])
    // In a namespace, the union and its members all get its prefix.
    let nsret, nserr = compile_run_code("""
        namespace ns
        union class U:
            A(x:int)
            B(y:int)
        def get(u:U) -> int:
            switch u:
                case A: return u.x
                case B: return u.y
        return get(A { 3 }) + get(B { 4 })
        """, [])
    test_eq(nserr, nil)
    test_eq(nsret, "7")

    // :: and pattern variables need a value whose fields can be named: a variable or field
    // path, not a call or an element.
    expect_errors("""
        union class U:
            A(x:int)
        def f() -> U: return A { 1 }
        return switch f():
            case A:: x
        """, [ "requires switching on a variable or field" ], 1)
    expect_errors("""
        union class U:
            A(x:int)
        let v = [ A { 1 } ]
        return switch v[0]:
            case A(y): y
        """, [ "requires switching on a variable or field" ], 1)
    // Only a type case can have them.
    expect_errors("return switch 1:\n    case 1:: 2\n    default: 3\n",
                  [ "`,` expected, found `::`" ], 1)
    // At most as many names as fields.
    expect_errors("""
        union class U:
            A(x:int)
        let u:U = A { 1 }
        return switch u:
            case A(p, q): p + q
        """, [ "type `A` has only 1 field(s), none for pattern variable `q`" ], 1)
    // Pattern variables are declared like variables: no duplicates, no shadowing, and
    // nothing may shadow them. No `_` prefix, since that would be an implicit argument.
    expect_errors("""
        union class U:
            A(x:int, y:int)
        let u:U = A { 1, 2 }
        return switch u:
            case A(p, p): p
        """, [ "identifier redefinition: p" ], 1)
    expect_errors("""
        union class U:
            A(x:int, y:int)
        let u:U = A { 1, 2 }
        let p = 3
        return switch u:
            case A(p, q): p + q
        """, [ "identifier shadowing: p" ], 1)
    expect_errors("""
        union class U:
            A(x:int, y:int)
        let u:U = A { 1, 2 }
        return switch u:
            case A(p, q):
                let p = 3
                p + q
        """, [ "identifier shadowing: p" ], 1)
    expect_errors("""
        union class U:
            A(x:int, y:int)
        class C:
            z:int
            def m(u:U): return switch u:
                case A(z, q): z + q
        let c = C { 1 }
        return c.m(A { 1, 2 })
        """, [ "cannot define variable with same name as field in this scope: z" ], 1)
    expect_errors("""
        union class U:
            A(x:int)
        let u:U = A { 1 }
        return switch u:
            case A(_p): 1
        """, [ "pattern variable `_p` cannot start with `_`" ], 1)
    // Two different objects with the same field in scope make its name ambiguous, and a
    // type in scope thru :: can't be brought in again for a different object.
    expect_errors("""
        union class U:
            A(x:int)
            B(x:int)
        def f(a::A, u:U) -> int:
            switch u:
                case A:: return x
                case B:: return x + 1
        return f(A { 1 }, B { 2 })
        """, [ "type used twice in the same scope with ::", "access to ambiguous field: x" ])

unions_tests()
