import std

// Declchecker and typechecker error recovery (see TypeChecker::Error in
// typecheck.h): with max_errors > 1, compile_run_code collects every error
// these passes find rather than stopping at the first. These check that the
// errors reported for a program are exactly the ones it has, with no
// consequential ones (the error type an erroneous expression gets is accepted
// by every check), and that typechecking survives whatever an error leaves
// behind. The declchecker runs before the typechecker, so its errors come
// first regardless of source position.

def typecheck_recovery_tests():
    suite("typecheck_recovery")
    guard not vm_compiled_mode()

    // A program without errors still runs with recovery on.
    let ok, okerr = compile_run_code("def f(x): return x + 1\nreturn f(2)", [], 100)
    test_eq(okerr, nil)
    test_eq(ok, "3")

    // The default is to stop at the first error, and max_errors caps the count.
    let three = "let a:string = 1\nlet b:int = \"s\"\nlet c:float = \"f\"\nreturn 1"
    expect_errors(three, [ "requires type `string`, got `int`" ], 1)
    expect_errors(three, [ "requires type `string`, got `int`",
                           "requires type `int`, got `string`" ], 2)
    // Independent statements each get their error.
    expect_errors(three, [ "requires type `string`, got `int`", "requires type `int`, got `string`",
                           "requires type `float`, got `string`" ])

    // Also inside function bodies, nested blocks, and after them.
    expect_errors("""
        def f(x:int) -> int:
            if x:
                let a:string = x
                print(a)
            else:
                let b:[int] = x
                print(b)
            return x
        let c:float = f(1) - "s"
        return 1
        """, [ "requires type `string`, got `int`", "requires type `[int]`, got `int`",
               "`-` (right argument) requires type `numeric/struct`, got `string`" ])
    expect_errors("""
        def f(x:int) -> int:
            if x:
                let a:string = x
                print(a)
            else:
                let b:[int] = x
                print(b)
            return x
        let c:float = "s"
        return f(1)
        """, [ "requires type `float`, got `string`", "requires type `string`, got `int`",
               "requires type `[int]`, got `int`" ])

    // An error in a function typechecked for several specializations is reported once.
    expect_errors("""
        def f(x): return x - "a"
        print(f(1))
        print(f(2.0))
        let y:string = 3
        return 1
        """, [ "`-` (right argument) requires type `numeric/struct`, got `string`",
               "requires type `string`, got `int`" ])

    // Nothing that uses a value an error was reported for reports errors of its own.
    expect_errors("""
        class A:
            x = 1
        class B:
            y = 2
        let a = A {}
        let v = a.y
        print(v.x)
        print(v + 1)
        print(v[0])
        print(v())
        for(v) i: print(i)
        if v: print(1)
        let w = [ v, 1 ]
        let u = v.y.x + 1
        print(u.x)
        def f(s:string): return s.length
        print(f(v))
        let z:float = "s"
        return 1
        """, [ "type `A` does not have field `y`", "requires type `float`, got `string`" ])
    expect_errors("""
        def f(a:int, b:string): return a
        let r = f("s", 1)
        let z:float = "s"
        return r + 1
        """, [ "`f` (1st argument) requires type `int`, got `string`",
               "`f` (2nd argument) requires type `string`, got `int`",
               "requires type `float`, got `string`" ])
    // A type variable bound to an erroneous type stays silent, also where it flows to.
    expect_errors("""
        let v = []
        v.push(nonexistent(1))
        let w = v.pop()
        print(w.x + 1)
        let z:float = "s"
        return 1
        """, [ "unknown field/function reference `nonexistent`",
               "requires type `float`, got `string`" ])
    // A function called with an erroneous argument is still typechecked, and reports the
    // errors that are its own, but not those about the argument.
    expect_errors("""
        class A:
            x = 1
        def g(v):
            let a:string = 1
            return v.x + v
        print(g(nonexistent(1)))
        return 1
        """, [ "unknown field/function reference `nonexistent`",
               "requires type `string`, got `int`" ])

    // Calls nothing applies to.
    expect_errors("""
        def f(x:int): return x
        def f(x:string): return x
        print(f([ 1 ]) + 1)
        print(f(2, 3))
        let z:float = "s"
        return 1
        """, [ "no overloads apply for `f` with 1st argument type `[int]`",
               "no version of function `f` takes 2 arguments",
               "requires type `float`, got `string`" ])
    expect_errors("""
        def f(x, y:int): return x
        def f(x, y:string): return x
        print(f(1, [ 1 ]) + 1)
        let z:float = "s"
        return 1
        """, [ "no overloads apply for `f` with 2nd argument type `[int]`",
               "requires type `float`, got `string`" ])
    expect_errors("""
        def f(x:int, y): return x
        def f(x:int, z): return x
        print(f(1, 2) + 1)
        let z:float = "s"
        return 1
        """, [ "multiple overloads for `f` match", "requires type `float`, got `string`" ])
    expect_errors("""
        print(substring("a", 1, 2, 3) + 1)
        print(substring("a") + 1)
        print(length(1.5) + 1)
        let z:float = "s"
        return 1
        """, [ "too many arguments for substring", "wrong number of many arguments for substring",
               "arguments match no overloads of length", "requires type `float`, got `string`" ])
    expect_errors("""
        let a = 1
        print(a(1) + 1)
        let f = fn(x, y): x + y
        print(f(1) + 1)
        let z:float = "s"
        return 1
        """, [ "dynamic function call value doesn\'t have a function type `int`",
               "function value called with too few arguments",
               "requires type `float`, got `string`" ])
    // The arguments of such a call still get typechecked, and an error about the call that
    // doesn't depend on their types is still reported when one of them is erroneous.
    expect_errors("""
        def f(x:int): return x
        def f(x:string): return x
        print(f([ 1 ], f(nil)))
        return 1
        """, [ "no overloads apply for `f` with 1st argument type `nil`",
               "no version of function `f` takes 2 arguments" ])
    expect_errors("""
        let a = 1
        let z:float = "s"
        print(a(z, 1.0 - "s"))
        return 1
        """, [ "requires type `float`, got `string`",
               "dynamic function call value doesn\'t have a function type `int`",
               "`-` (right argument) requires type `numeric/struct`, got `string`" ])
    expect_errors("""
        def f(x:int) -> void
        print(f(1))
        let z:float = "s"
        return 1
        """, [ "can\'t call function type `f`", "requires type `float`, got `string`" ])

    // Field access.
    expect_errors("""
        class A:
            x = 1
        let a:A? = nil
        print(a.x + 1)
        let n = 1
        print(n.x + 1)
        let z:float = "s"
        return 1
        """, [ "dereferencing nillable type: `A?`", "type `int` does not have field `x`",
               "requires type `float`, got `string`" ])
    expect_errors("""
        class A:
            x = 1
            def m():
                member y = 2
                return y
            def n(): return y
        print(A {}.y + 1)
        let z:float = "s"
        return A {}.n()
        """, [ "declared by a `member` in `m`", "requires type `float`, got `string`",
               "declared by a `member` in `m`" ])

    // Constructors.
    expect_errors("""
        class A:
            x:int
            y:string
        print(A { 1 }.x)
        print(A { 1, "a", 3 }.x)
        print(A { y: "a", z: 1 }.x)
        print(A { x: 1, x: 2 }.x)
        print(A { "s", 1 }.x)
        let b = { 1, 2 }
        print(b.x)
        let z:float = "s"
        return 1
        """, [ "field `y` not initialized", "too many initializers for `A`",
               "type `A` does not have field `z`", "field `x` initialized twice",
               "`object_constructor` (1st argument) requires type `int`, got `string`",
               "`object_constructor` (2nd argument) requires type `string`, got `int`",
               "class/struct type for auto constructor unknown in this context",
               "requires type `float`, got `string`" ])
    expect_errors("""
        class A<T>:
            x:T
        class Ai = A<int>
        print(A { "s" }.x)
        print(A { 1, 2 }.x)
        let z:float = "s"
        return 1
        """, [ "generic constructor matches no named explicit specialization of `A`",
               "too many initializers for `A`", "requires type `float`, got `string`" ])
    // The initializers of a constructor that can't be resolved still get typechecked.
    expect_errors("""
        class A:
            x:int
        print(A { x: 1, y: 1.0 - "s" }.x)
        return 1
        """, [ "type `A` does not have field `y`",
               "`-` (right argument) requires type `numeric/struct`, got `string`" ])
    expect_errors("""
        class C:
            s = "s"
            c = C {}
        print(C {}.s)
        let z:float = "s"
        return 1
        """, [ "recursively constructs", "requires type `float`, got `string`" ])

    // Vectors and indexing.
    expect_errors("""
        let v = [ 1, "a" ]
        print(v[0])
        let w:[int] = [ "a" ]
        let n = 1
        print(n[0])
        let u = [ 1 ]
        print(u["a"])
        print(u[int2 { 0, 0 }])
        let z:float = "s"
        return 1
        """, [ "have no common supertype", "requires type `int`, got `string`",
               "requires type `vector/string/numeric struct`, got `int`",
               "requires type `int/struct of int`, got `string`",
               "requires type `nested vector`, got `int`",
               "requires type `float`, got `string`" ])

    // Control flow.
    expect_errors("""
        class A:
            x = 1
        for(A {}) i: print(i + 1)
        switch [ 1 ]:
            case 1: print(1)
        enum e:
            e_a
            e_b
            e_c
        let r = switch e_a:
            case e_a: 1
        let s = switch 1:
            case 1: 1
        while 1:
            let q:float = "s"
            print(q)
        break
        continue
        let z:float = "s"
        return r + s
        """, [ "`for` can only iterate over int / string / vector, not `A`",
               "switch value must be int / float / string / class",
               "enum value `e_b` not tested in switch", "enum value `e_c` not tested in switch",
               "non-exhaustive switch that returns a value must have a default case",
               "requires type `float`, got `string`",
               "`break` must occur inside", "`continue` must occur inside",
               "requires type `float`, got `string`" ])
    expect_errors("""
        abstract class a:
            x = 0
        class b : a
        class c : a
        def f(v:a): return switch v:
            case b: 1
            case 1: 2
        print(f(b {}))
        let z:float = "s"
        return 1
        """, [ "non-type value in switch on type", "no case applies to `c`",
               "requires type `float`, got `string`" ])

    // Return values.
    expect_errors("""
        def f(): return 1
        let a, b = f()
        print(a + b)
        let x = print(1)
        print(x + 1)
        def g() -> int: return 1
        let c, d = g()
        print(c + d)
        def h(n) -> int:
            if n: return 1
        def k() -> int:
            return
        def m() -> int:
            print(1)
        return h(1) + k() + m()
        """, [ "`integer literal` returns 1 values, 2 needed",
               "`native call` returns 0 values, 1 needed",
               "returning 1 values, caller requires 2",
               "control reaches end of non-void function",
               "`default value` returns 0 values, 1 needed",
               "`return` requires type `int`, got `void`" ])
    expect_errors("""
        def f(n): return if n: f(n - 1) else: 0
        print(f(3))
        let z:float = "s"
        return 1
        """, [ "recursive function `f` must have explicit return type",
               "requires type `float`, got `string`" ])
    // A function whose only returns are non-local never returns normally; its callers see
    // the type it declared regardless.
    expect_errors("""
        def outer() -> int:
            def inner() -> int:
                return 1 from outer
            return inner() + 1
        let z:float = "s"
        return outer()
        """, [ "requires type `float`, got `string`", "missing return statement" ])
    // A return from the program keeps one value, so extra ones are not an error.
    let mret, merr = compile_run_code("def f():\n    return 1, 2 from program\nf()\nreturn 3",
                                      [], 100)
    test_eq(merr, nil)
    test_eq(mret, "1")

    // Generics.
    expect_errors("""
        def f<T>(x:int) -> T: return T {}
        print(f(1))
        print(f<int, int>(1))
        let z:float = "s"
        return 1
        """, [ "cannot implicitly bind type variable `T` in call to `f`",
               "too many specializers given", "requires type `float`, got `string`" ])

    // Free variables and struct constants.
    expect_errors("""
        def apply(f): return f()
        def maker():
            let x = 1
            return fn: x + 1
        print(apply(maker()) + 1)
        let s = int2 { 1, 2 }
        s.x = 3
        let z:float = "s"
        return 1
        """, [ "free variable `x` not in scope", "is a constant struct",
               "requires type `float`, got `string`" ])

    // Operators.
    expect_errors("""
        print("a" - 1)
        print(1 < "a")
        print(typeof int == typeof float)
        print(-"a")
        print([ 1 ] * 2)
        let z:float = "s"
        return 1
        """, [ "`-` (left argument) requires type `numeric/struct`, got `string`",
               "`<` doesn\'t work on `int` and `string`",
               "requires type", "requires type `numeric / numeric struct`, got `string`",
               "`*` (left argument) requires type `numeric/struct`, got `[int]`",
               "requires type `float`, got `string`" ])

    // Errors the declchecker finds: all of them, and the typechecker still runs after.
    expect_errors("""
        def f() -> int: return unknown_a(1)
        def g() -> int:
            if false: unknown_b()
            return 1
        let z:float = "s"
        return f() + g()
        """, [ "unknown field/function reference `unknown_a`",
               "unknown field/function reference `unknown_b`",
               "requires type `float`, got `string`" ])
    expect_errors("""
        class B
        class A : B
            y = 1
            x = 2
        class B:
            x = 3
        let z:float = "s"
        return A {}.x
        """, [ "field `x` of `A` is also a field of superclass `B`",
               "requires type `float`, got `string`" ])
    expect_errors("""
        abstract class Act:
            def go() -> int: return 0
        def runner(a:Act) -> int: return a.go()
        def test() -> int:
            class MyAct : Act
                def go() -> int: return 9
            return runner(MyAct {})
        let z:float = "s"
        return test()
        """, [ "same scope", "requires type `float`, got `string`" ])

    // Unreached functions are still checked for their errors after errors elsewhere.
    expect_errors("""
        class H:
            hf = 1
        def dead(x:int) -> int:
            return x.hf
        let z:float = "s"
        return 1
        """, [ "requires type `float`, got `string`", "type `int` does not have field `hf`" ])
    // In an unreached function, code that needs an active caller (a call to a local function
    // of another function here) is not an error, and ends the check of that function, so
    // what comes before it is checked, and what comes after is not.
    expect_errors("""
        def outer() -> int:
            def inner() -> int: return 3
            return inner()
        def dead(x:int) -> int:
            let z:string = x
            let y = inner()
            let w:string = x
            return y + x
        return outer()
        """, [ "requires type `string`, got `int`" ])

    // Errors that were fatal: the borrow checker's, the string assignment codegen used to
    // catch, and a duplicate serializable id.
    expect_errors("""
        let s = "abc"
        s[0] = 65
        let z:float = "s"
        return s
        """, [ "cannot use string element as lvalue", "requires type `float`, got `string`" ])
    expect_errors("""
        class A:
            attribute serializable = 1
            x = 1
        class B:
            attribute serializable = 1
            y = 2
        let z:float = "s"
        return A {}.x + B {}.y
        """, [ "index that is already in use: 1", "requires type `float`, got `string`" ])

typecheck_recovery_tests()
