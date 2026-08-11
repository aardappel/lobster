import std


// Functions: definitions, overloads, default arguments, generics, multiple
// return values, non-local returns, function values, free variables.

// Function type declaration, usable as an explicit (non-inlined) arg type.
def int_to_int_fun(i:int) -> int

// Explicit free variables: usable outside lexical scope of the vars.
def freevar_add(c, d) [dfv_a, dfv_b]:
    dfv_a = dfv_a + c
    dfv_b = dfv_b + d

def functions_tests():
    suite("functions")

    // Basics: named functions return void without an explicit return.
    def add1(a, b): return a + b
    def add2(a, b):
        return a + b
    test_eq(add1(1, 2) + add2(3, 4), 10)
    // Generic functions specialize per argument types.
    test_eq(add2(1.5, 1), 2.5)
    test_eq(add2("a", 1), "a1")
    // Typed args, explicit return type coercion.
    def addf(a:int, b:int) -> float: return a + b
    test_eq(addf(1, 2), 3.0)
    test_true(addf(1, 2) is float)
    // void function.
    var vside = 0
    def voidf() -> void:
        vside++
    voidf()
    test_eq(vside, 1)

    // Call syntaxes: normal, method-style dot, dot without parens.
    def double(x): return x * 2
    test_eq(double(21), 42)
    test_eq(21.double(), 42)
    test_eq(21.double, 42)
    def between(x, lo, hi): return x >= lo and x <= hi
    test_true(5.between(1, 10))

    // Overloading on argument type (static) and on argument count.
    def ovl(x:int): return "int"
    def ovl(x:float): return "float"
    def ovl(x:string): return "string"
    def ovl(x, y): return "two"
    test_eq(ovl(1), "int")
    test_eq(ovl(1.5), "float")
    test_eq(ovl(""), "string")
    test_eq(ovl(1, 2), "two")
    // Overloading on the element type of a vector.
    def ovv(v:[int]): return "ints"
    def ovv(v:[string]): return "strings"
    test_eq(ovv([ 1 ]), "ints")
    let ovsv = [ "s" ]
    test_eq(ovv(ovsv), "strings")
    // Overloading on struct types.
    def ovs(v:int2): return "i2"
    def ovs(v:float2): return "f2"
    def ovs(v:int3): return "i3"
    test_eq(ovs(int2 { 1, 2 }), "i2")
    test_eq(ovs(float2 { 1.0, 2.0 }), "f2")
    test_eq(ovs(int3 { 1, 2, 3 }), "i3")
    // Overloading on enum (bool) vs int: an enum value picks the enum
    // overload, a plain int the int one.
    def ove(x:bool): return "bool"
    def ove(x:int): return "int"
    test_eq(ove(true), "bool")
    test_eq(ove(2), "int")
    // A more specific subclass overload wins over the superclass one, when
    // resolved statically.
    class OvA:
        a = 0
    class OvB : OvA
    def ovc(x:OvA): return "base"
    def ovc(x:OvB): return "sub"
    test_eq(ovc(OvB {}), "sub")
    test_eq(ovc(OvA {}), "base")

    // Default arguments, which may reference variables in scope.
    let defbase = 10
    def dfa(a, b = 5, c = defbase + 5): return a + b + c
    test_eq(dfa(1), 21)
    test_eq(dfa(1, 2), 18)
    test_eq(dfa(1, 2, 3), 6)
    // Overloads must share the same defaults; a different argument count is a
    // completely separate function.
    def dfb(a:int, b, c = defbase + 1): return a + b + c
    def dfb(a:float, b, c = defbase + 1): return int(a) - b - c
    def dfb(a): return a
    test_eq(dfb(1) + dfb(1, 2) + dfb(1, 2, 3), 1 + 14 + 6)
    test_eq(dfb(2.0, 1), -10)

    // Explicit generics.
    def gadd<T>(x:T, y:T): return x + y
    test_eq(gadd(1, 2), 3)
    test_eq(gadd(1.5, 2.5), 4.0)
    test_eq(gadd(int2_1, int2_1), int2 { 2, 2 })  // Struct type argument.
    let gi = 1
    test_true(gadd(1.0, gi) is float)  // Second arg coerced to T=float.
    test_true(gadd<float>(gi, gi) is float)  // Explicit specializer coerces both.
    def gret<T>(t:T) -> T: return t
    test_eq(gret("x"), "x")
    // With multiple type variables, a partial explicit list is allowed:
    // the rest is inferred. The float specializer coerces the first arg.
    def gpair<T, U>(x:T, y:U): return "{x}|{y}"
    let gpi = 1
    test_eq(gpair<float>(gpi, "s"), "1.0|s")
    test_eq(gpair(gpi, "s"), "1|s")

    // Multiple return values.
    def mr2(): return 1, 2
    let m1, m2 = mr2()
    test_eq(m1, 1)
    test_eq(m2, 2)
    let monly = mr2()  // Extra values may be dropped.
    test_eq(monly, 1)
    def mrpass(): return mr2()  // Pass-through needs the return.
    let p1, p2 = mrpass()
    test_eq(p1 + p2, 3)
    def mrtyped() -> float, string: return 1.0, "a"
    let mf, ms = mrtyped()
    test_eq(mf, 1.0)
    test_eq(ms, "a")
    // Multiple returns from an if expression need parentheses.
    def mcond(c) -> int, int:
        return if c: mr2() else: (3, 4)
    let c1, c2 = mcond(false)
    test_eq(c1, 3)
    test_eq(c2, 4)

    // return exits the enclosing *named* function, through HOF blocks.
    def findit(xs, x):
        for(xs) e:
            if e == x:
                return true
        return false
    test_true(findit([ 1, 2, 3 ], 2))
    test_true(not findit([ 1, 2, 3 ], 9))

    // Non-local return: return from a named outer function.
    def nlr_outer():
        nlr_inner()
        return 2
    def nlr_inner():
        return 1 from nlr_outer
    test_eq(nlr_outer(), 1)
    // return from a do() block.
    var reached = 0
    do():
        for(2) i:
            if i: return from do
            reached++
    test_eq(reached, 1)
    // A function mixing a plain return with a trailing non-local return must
    // compile and take the plain path here (the from-return is unreached).
    def mid_g():
        return mid_h() + 1
    def mid_h():
        if reached == 1:
            return 4
        return 3 from mid_g
    test_eq(mid_g(), 5)
    // And the non-local path when the condition flips.
    def mid_g2():
        return mid_h2() + 1
    def mid_h2():
        if reached == 99:
            return 4
        return 3 from mid_g2
    test_eq(mid_g2(), 3)
    // Multiple different paths through the same unwind point, with different
    // return types and value counts.
    def unwind2(mode):
        def unwind1():
            def return_past() -> float:
                switch mode:
                    case 0: return "a" from unwind1
                    case 1: return int2 { 4, 5 } from unwind2
                    default: return 3.1  // Local return.
            return string(return_past())
        return int2 { length(unwind1()), 0 }
    test_eq(unwind2(0), int2 { 1, 0 })
    test_eq(unwind2(1), int2 { 4, 5 })
    test_eq(unwind2(2), int2 { 3, 0 })
    // Non-local returns work as branches of if/switch value expressions.
    def outer_vals():
        def bail():
            return "bailed" from outer_vals
        let a = if reached == 1: 1 else: bail()
        let b = switch a:
            case 1: 2
            default: bail()
        return "ok{a + b}"
    test_eq(outer_vals(), "ok3")

    // Function values: full fn syntax, terse syntax, anonymous args.
    let f1 = fn(a:int, b:int): a + b
    test_eq(f1(1, 2), 3)
    let f2 = fn a, b: a * b
    test_eq(f2(3, 4), 12)
    let apply4 = fn f: f(4)
    test_eq(apply4(fn x: x * x), 16)
    // fn with an explicit return type coerces its result.
    let fcoerce = fn(x:int) -> float: x
    test_eq(fcoerce(3), 3.0)
    test_true(fcoerce(3) is float)
    // Function values can be stored in fields and vectors, but must be
    // copied into a variable to be called.
    class FHolder:
        f:int_to_int_fun
    let fheld = FHolder { fn i: i + 1 }
    let fout = fheld.f
    test_eq(fout(1), 2)
    // In a vector, distinct lambdas have no common supertype by themselves:
    // an explicit function type is needed.
    let fvec = [ fn i: i * 2, fn i: i * 3 ]::int_to_int_fun
    let fel = fvec[1]
    test_eq(fel(2), 6)
    // Blocks after calls, with all syntax variants.
    def twice(f): return f() + f()
    test_eq(twice(): 3, 6)
    def repeatn(n, f):
        for(n) i: f(i)
    var rsum = 0
    repeatn(3) i:
        rsum += i
    test_eq(rsum, 3)
    rsum = 0
    repeatn 3: rsum += _
    test_eq(rsum, 3)
    // Multiple anonymous args become args in order of appearance.
    def combine2(a, b, f): return f(a, b)
    test_eq(combine2(10, 4): _x - _y, 6)
    // Multiple function values: subsequent ones need the fn keyword.
    def bracket(f, g):
        return f() + g()
    let br = bracket():
        "a"
    fn:
        "b"
    test_eq(br, "ab")

    // Function value called via an explicitly declared function type is not
    // inlined but still works the same.
    def calls_ft(f:int_to_int_fun): return 1 + f(2)
    test_eq(calls_ft(fn i: i * 10), 21)
    // call_function_value invokes a function value from native code.
    var cfv_state = 1
    let cfv = fn() -> void:
        cfv_state = 2
    call_function_value(cfv)
    test_eq(cfv_state, 2)
    // Function values with 0 args, and passing them on to other functions.
    let zerof = fn: 42
    test_eq(zerof(), 42)
    def call_it(f): return f()
    test_eq(call_it(zerof), 42)

    // Free variables from enclosing scope.
    var counter = 0
    def bump_counter():
        counter++
    bump_counter()
    bump_counter()
    test_eq(counter, 2)
    // A var declared with nonlocal is preferred when binding explicit free
    // variables, here over the class member of the same name that would
    // otherwise be found in the ::-scope of the calling method.
    nonlocal var nl_pick = 1
    def nl_add() [nl_pick]:
        nl_pick += 10
    class NLHost:
        nl_pick = 100
        def trigger():
            nl_add()
    let nlh = NLHost {}
    nlh.trigger()
    test_eq(nlh.nl_pick, 100)  // The member was not the one picked...
    test_eq(nl_pick, 11)       // ...the nonlocal var was.
    // A HOF-passed function value that differs only in its free variable
    // still gets specialized per context.
    def call_thru(i): return i()
    def with_free(f):
        return call_thru(): f()
    test_eq(1, with_free(): 1)
    test_eq("a", with_free(): "a")
    // Explicit free variables declared with [names].
    var dfv_a = 0
    var dfv_b = 10
    freevar_add(1, 2)
    test_eq(dfv_a, 1)
    test_eq(dfv_b, 12)

    // Recursion (explicit return type needed) and mutual recursion.
    def fact(n:int) -> int: return if n <= 1: 1 else: n * fact(n - 1)
    test_eq(fact(6), 720)
    test_eq(6.fact(), 720)  // Recursive call via dot syntax.
    // Recursion via `or`: the bool/int union collapses to int.
    def factor(n:int) -> int: return 1 > n or factor(n - 1) * n
    test_eq(factor(5), 120)
    def is_even(n:int) -> int:
        return if n == 0: true else: is_odd(n - 1)
    def is_odd(n:int) -> int:
        return if n == 0: false else: is_even(n - 1)
    test_true(is_even(10))
    test_true(is_odd(7))

    // Early return without a value from a void function.
    var early = 0
    def maybe_bump(c) -> void:
        if c: return
        early++
    maybe_bump(1)
    maybe_bump(0)
    test_eq(early, 1)

    // static: function-scoped globals persist across calls.
    def next_id():
        static id = 100
        return ++id
    test_eq(next_id(), 101)
    test_eq(next_id(), 102)
    // With an explicit type.
    def next_idf():
        static idf:float = 0.5
        return ++idf
    test_eq(next_idf(), 1.5)
    test_eq(next_idf(), 2.5)
    // static_frame: like static, but resets when a frame passes without the
    // function being called. Without gl.frame() advancing frames, it simply
    // persists like static.
    def next_idsf():
        static_frame idsf = 10
        return ++idsf
    test_eq(next_idsf(), 11)
    test_eq(next_idsf(), 12)

    // Functions defined at local scope.
    def outerfn():
        def innerfn(x): return x + 1
        return innerfn(41)
    test_eq(outerfn(), 42)

    // Functions may carry attributes.
    def attributed():
        attribute marker
        attribute valued = 123
        return 1
    test_eq(attributed(), 1)

functions_tests()
