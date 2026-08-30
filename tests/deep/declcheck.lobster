import std


// Declaration-order behaviors. This file pins down which declaration orders
// are legal and what they mean, so that declcheck (the declaration resolving
// pass between parser and typechecker) cannot silently change them.
// The first section contains order patterns compiled as part of this file;
// the second section runs whole programs via compile_run_code, since these
// are order-sensitive at the top level of a program.


// A function can call a function declared later in the file.
def dk_calls_forward(): return dk_declared_later() + 1
def dk_declared_later(): return 41

// A function body can construct/use a pre-declared class defined later in
// the file, as long as the call happens after the definition. (Without at
// least a pre-declaration, `DKLater {` does not even parse as a constructor.)
class DKLater
def dk_make_later_class(): return DKLater { 42 }.x
class DKLater:
    x:int

// Field types inferred from initializers.
let dk_typed_global:int = 4
enum dk_color:
    dk_red
    dk_green
class DKInferred:
    a = 1              // int
    b = 2.5            // float
    c = "s"            // string
    d = dk_green       // enum
    e = -3             // unary minus
    f = DKLater { 1 }  // constructor of an earlier-declared class
    g = [ 1, 2 ]       // vector of int
    h = 1.0 + 1.0      // constant math
    i = 6 / 2 + 1      // more constant math, stays int
    j = 2 * 2.5        // int coerces to float
    k = "a" + "b"      // string concat
    l = 1 < 2          // comparison, bool
    m = 1 | 6          // bit op, int
    n = "pre{7}post"   // string interpolation
    o = [ "x" ]        // vector of string, from elements
    p = dk_typed_global + 1
    q = nil::DKLater   // typed nil

// Field default referencing a global declared before the class.
let dk_global_default = 7
class DKUsesGlobal:
    g = dk_global_default

// A field default that calls a function, so its type can only come from
// typechecking the initializer expression.
def dk_default_fn(): return 11
class DKCallsFn:
    h = dk_default_fn()

// Pre-declared class used as a (nilable) field type before its definition.
class DKPre
class DKHolder:
    p:DKPre? = nil
class DKPre:
    v = 5

// Mutually recursive classes thru a pre-declaration.
class DKMutA
class DKMutB:
    a:DKMutA? = nil
class DKMutA:
    b:DKMutB? = nil

// A class may refer to itself in a nilable field.
class DKList:
    val = 0
    next:DKList? = nil

// Pre-declared global: type given at pre-declaration, value later.
let dk_predecl_g:int
def dk_read_predecl(): return dk_predecl_g
let dk_predecl_g:int = 21

// Hierarchy fully declared before any dispatch happens.
abstract class DKShape:
    def dk_area(): return 0
class DKCircle : DKShape
    r = 2
    def dk_area(): return r * r * 3
class DKSquare : DKShape
    s = 3
    def dk_area(): return s * s

// Generic class with a field type derived from a default using its
// type variable.
class DKGen<T>:
    data = []::T

// Generic class, named specialization, generic subclass.
class DKBox<T>:
    val:T
class DKIntBox = DKBox<int>
class DKSubBox<T> : DKBox<T>
    tag = "t"

// Function type declared before use.
def dk_ftype(x:int) -> int
def dk_apply(f:dk_ftype, v:int): return f(v)

// Operator overload on a struct declared just before it.
struct DKVec2:
    x:int
    y:int
def operator+(a:DKVec2, b:DKVec2): return DKVec2 { a.x + b.x, a.y + b.y }

// A member declaration adds a field from inside a method.
class DKMember:
    dummy = 0
    def dk_bump():
        member count = 0
        count++
        return count

// A static inside a function.
def dk_counter():
    static n = 10
    n++
    return n

def declcheck_tests():
    suite("declcheck")
    test_eq(dk_calls_forward(), 42)
    test_eq(dk_make_later_class(), 42)
    let ifd = DKInferred {}
    test_eq(ifd.a, 1)
    test_eq(ifd.b, 2.5)
    test_eq(ifd.c, "s")
    test_true(ifd.d == dk_green)
    test_eq(ifd.e, -3)
    test_eq(ifd.f.x, 1)
    test_seq(ifd.g, [ 1, 2 ])
    test_eq(ifd.h, 2.0)
    test_eq(ifd.i, 4)
    test_eq(ifd.j, 5.0)
    test_eq(ifd.k, "ab")
    test_true(ifd.l)
    test_eq(ifd.m, 7)
    test_eq(ifd.n, "pre7post")
    test_seq(ifd.o, [ "x" ])
    test_eq(ifd.p, 5)
    test_true(not ifd.q)
    let dg = DKGen<int> {}
    dg.data.push(3)
    test_eq(dg.data[0], 3)
    test_eq(DKUsesGlobal {}.g, 7)
    test_eq(DKCallsFn {}.h, 11)
    let hold = DKHolder {}
    test_true(not hold.p)
    hold.p = DKPre {}
    test_eq(hold.p.v, 5)
    let mb = DKMutB {}
    mb.a = DKMutA {}
    test_true(mb.a)
    let lst = DKList { val: 1, next: DKList { val: 2 } }
    let nxt = lst.next
    if nxt: test_eq(nxt.val, 2)
    test_eq(dk_read_predecl(), 21)
    let shapes:[DKShape] = [ DKCircle {}, DKSquare {} ]
    test_eq(shapes[0].dk_area(), 12)
    test_eq(shapes[1].dk_area(), 9)
    test_eq(DKIntBox { 3 }.val, 3)
    test_eq(DKBox<float> { 1.5 }.val, 1.5)
    test_eq(DKSubBox<int> { 4 }.tag, "t")
    test_eq(DKSubBox<int> { 4 }.val, 4)
    test_eq(dk_apply(fn(x): x * 2, 10), 20)
    test_eq(DKVec2 { 1, 2 } + DKVec2 { 3, 4 }, DKVec2 { 4, 6 })
    let m = DKMember {}
    test_eq(m.dk_bump(), 1)
    test_eq(m.dk_bump(), 2)
    test_eq(dk_counter(), 11)
    test_eq(dk_counter(), 12)

declcheck_tests()


// Whole programs whose meaning depends on top-level statement order,
// run sandboxed. Only in JIT mode, like deep/errors.lobster.

def expect_dk_ok(code:string, expect:string):
    let ret, err = compile_run_code(code, [])
    num_checks++
    if err:
        num_failures++
        print "FAIL [{current_suite}]: unexpected error: {err}\n  for: {code}"
    elif ret != expect:
        num_failures++
        print "FAIL [{current_suite}]: got {ret}, wanted {expect}\n  for: {code}"

def declcheck_order_tests():
    suite("declcheck_order")
    guard not vm_compiled_mode()

    // Dispatch picks up a subclass declared before the dispatching call.
    expect_dk_ok("def f(s) -> int: return s.area()\n" +
                 "class A:\n    def area(): return 1\n" +
                 "class B : A\n    def area(): return 2\n" +
                 "let v:A = B \{\}\nreturn f(v)", "2")

    // A class declared after a dispatch on its superclass has been
    // typechecked participates in that dispatch, since the full class
    // hierarchy is collected before typechecking starts.
    expect_dk_ok("class A:\n    def m(): return 1\n" +
                 "class B : A\n    def m(): return 2\n" +
                 "var v:A = B \{\}\ndef d() -> int: return v.m()\n" +
                 "assert d() == 2\n" +
                 "class C : A\n    def m(): return 3\n" +
                 "v = C \{\}\nreturn d()", "3")

    // A switch on class type picks up subclasses declared before the switch
    // is typechecked, even when the value only later becomes that subclass.
    expect_dk_ok("class A:\n    x = 1\n" +
                 "class B : A\n    y = 2\n" +
                 "var v:A = A \{\}\n" +
                 "def d() -> int:\n    switch v:\n        case B: return 20\n        case A: return 10\n" +
                 "assert d() == 10\n" +
                 "v = B \{\}\nreturn d()", "20")

    // A subclass declared after a switch on its superclass has been
    // typechecked falls to its superclass case.
    expect_dk_ok("class A:\n    x = 1\n" +
                 "var v:A = A \{\}\n" +
                 "def d() -> int:\n    switch v:\n        case A: return 10\n        default: return 0\n" +
                 "assert d() == 10\n" +
                 "class B : A\n    y = 2\n" +
                 "v = B \{\}\nreturn d()", "10")

    // Constructing a pre-declared class inside a field default: the default
    // values of the pre-declared class apply once it is fully declared,
    // regardless of the order in which the classes get typechecked.
    expect_dk_ok("class PreDecl\n" +
                 "class Foo:\n    p = PreDecl \{\}\n" +
                 "class PreDecl:\n    x = 1\n" +
                 "let f = Foo \{\}\nreturn f.p.x", "1")

    // Inheriting from a pre-declared class: the subclass gets the superclass
    // fields once the superclass is fully declared.
    expect_dk_ok("class Base\n" +
                 "class Sub : Base\n    y = 2\n" +
                 "class Base:\n    x = 1\n" +
                 "let s = Sub \{\}\nreturn s.x + s.y", "3")

    // A pre-declared class whose definition statement sits in a position the
    // typechecker visits after the constructing call can still be constructed.
    expect_dk_ok("class C\ndef f() -> int:\n    return C \{ 5 \}.x\n" +
                 "class C:\n    x:int\n" +
                 "return f()", "5")

    // Calling a function that reads a global before the global's initializing
    // statement has run is an error.
    expect_error("foo()\nvar x = 1\ndef foo():\n    print x\nreturn x",
                 "before being initialized")

    // A plain identifier that exists nowhere is an error at parse time.
    expect_error("return unknown_identifier_xyz", "unknown identifier")

    // A dispatch typechecked (and then reused) before a subclass is declared
    // still sees the subclass (scenario from tests/errors/dispatchorder._lobster).
    expect_dk_ok("class A:\n    def f(): return 1\n" +
                 "var v:A? = nil\n" +
                 "def setv(x:A?): v = x\n" +
                 "def g() -> int:\n    return if v: v.f() else: 3\n" +
                 "assert g() == 3\n" +
                 "class B : A\n    def f(): return 2\n" +
                 "setv(B \{\})\n" +
                 "return g()", "2")

    // An unknown function errors even in code the typechecker never reaches.
    expect_error("if false:\n    completely_unknown_function(1, 2)\nreturn \"ok\"", "unknown")

    // An inheritance cycle formed thru a pre-declaration is an error.
    expect_error("class A\nclass B : A\nclass A : B\nreturn 1", "inheritance cycle")

    // A struct that directly contains itself is an error.
    expect_error("struct S:\n    x:int\n    s:S\nreturn 1", "self-referential")

    // A field whose default recursively constructs its own type is an error.
    expect_error("class R:\n    r = R \{\}\nlet a = R \{\}\nreturn 1",
                 "recursively constructs")

    // Also when the recursion goes thru the defaults of another type.
    expect_error("class B2\n" +
                 "class A2:\n    b = B2 \{\}\n" +
                 "class B2:\n    a = A2 \{\}\n" +
                 "let x = A2 \{\}\nreturn 1", "recursively constructs")

    // Pre-declared struct never defined is an error.
    expect_error("class Undefined\nreturn 1", "never defined")

    // A local function shadows an outer one of the same name for lexically
    // nested code only.
    expect_dk_ok("def s() -> int: return 1\n" +
                 "def outer() -> int:\n    def s() -> int: return 2\n    return s()\n" +
                 "assert s() == 1\n" +
                 "return outer()", "2")

    // Local functions are visible in the whole block they are declared in,
    // including in functions declared before them.
    expect_dk_ok("def h() -> int:\n" +
                 "    def p() -> int: return q()\n" +
                 "    def q() -> int: return 3\n" +
                 "    return p()\n" +
                 "return h()", "3")

    // A local function can be called from outside its lexical scope while
    // its enclosing function is active ("functions as environments").
    // NOTE: pending decision on whether to keep supporting this; matvoxels
    // currently relies on it (game.lobster helpers used from game_item).
    expect_dk_ok("def outer() -> int:\n" +
                 "    def inner() -> int: return 7\n" +
                 "    return helper()\n" +
                 "def helper() -> int: return inner()\n" +
                 "return outer()", "7")

    // But not when the enclosing function is not active.
    expect_error("def outer() -> int:\n" +
                 "    let v = 1\n" +
                 "    def inner() -> int: return v\n" +
                 "    return inner()\n" +
                 "assert outer() == 1\n" +
                 "return inner()", "unknown")

    // Methods only dispatch together when declared in the same scope, so a
    // nested method on a class related to one with methods elsewhere (which
    // would silently split the dispatch) is an error.
    expect_error("abstract class Act:\n    def go() -> int: return 0\n" +
                 "def runner(a:Act) -> int: return a.go()\n" +
                 "def test() -> int:\n" +
                 "    class MyAct : Act\n        def go() -> int: return 9\n" +
                 "    return runner(MyAct \{\})\n" +
                 "return test()", "same scope")

    // A local helper whose first arg happens to be a class with same-name
    // methods on unrelated types elsewhere is its own function as usual.
    expect_dk_ok("class Unrelated:\n    x = 1\n" +
                 "def go(u:Unrelated) -> int: return 4\n" +
                 "def test() -> int:\n" +
                 "    class Local:\n        y = 2\n" +
                 "    def go(l:Local) -> int: return l.y\n" +
                 "    return go(Local \{\})\n" +
                 "return test() + go(Unrelated \{\})", "6")

    // Overload selection is on receiver type before arity: the variant with
    // the closest matching first arg wins, even when it needs default args
    // filled in and a worse match of the exact arity exists.
    expect_dk_ok("class Base:\n    v = 1\n" +
                 "class Der : Base\n    w = 2\n" +
                 "def q(d:Der, c = 5) -> int: return 1\n" +
                 "def q(b:Base) -> int: return 2\n" +
                 "assert q(Base \{\}) == 2\n" +
                 "return q(Der \{\})", "1")

    // Methods on a class can be called by an earlier-declared function
    // (methods resolve against the whole program, not parse position).
    expect_dk_ok("def call_it(x) -> int: return x.method_declared_later()\n" +
                 "class M:\n    def method_declared_later(): return 6\n" +
                 "return call_it(M \{\})", "6")

    // A field access whose field name is only declared on a later class.
    expect_dk_ok("def get_field(x) -> int: return x.late_field\n" +
                 "class L:\n    late_field = 8\n" +
                 "return get_field(L \{\})", "8")

declcheck_order_tests()
