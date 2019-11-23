import testing
import std

// Misc tests related to type checking.

run_test("types"):

    do():
        def h(i): return i()
        def g(f):
            // Here, the def passed to h appears the same each time, but it has a freevar f that is different.
            return h(): f()
        assert 1 == (g(): 1)
        assert "a" == (g(): "a")

    do():
        def compile_time_if(x):
            return if x is int or x is float:
                // The division is a type error if x is a string.
                // But it compiles anyway since this if is compile-time constant,
                // and the type checker ignores this branch.
                // Would be an error in a C++ template, and most other statically typed languages,
                // because they don't cull code without the optimizer.
                1 / x
            else:
                // Also note the return type is just the else branch in the string case,
                // it doesn't need to make a type union between the branches anymore.
                x
        assert compile_time_if(1) is int
        assert compile_time_if("") is string

    assert typeof int == 0  // Always the first one added to the typetable.

    do():
        class testa:
            a:int
        class testb : testa
            b:int

        // return from should generalize both specializations
        def return_from(a):
            return_from2()
            return testb { 0, 0 }
        def return_from2(): return testa { 0 } from return_from
        assert return_from(1) is testa
        assert return_from("") is testa

    do():
        // strongly typed enums.
        var b = true
        b = bool(1)
        assert b == true
        assert string(true) == "true"
        assert (assert parse_data(typeof [bool], "[true]"))[0] == true
        switch true:  // All values need to have a case.
            case false: assert false
            case true: assert true

    do():
        // Dynamic dispatch (& Specialization)
        struct S1:
            a:int = 0
            def f(): return 1
        struct S2 : S1
            b:int = 0
            def f(): return 2
        // Always static dispatch even though there's a sub-class, because there's no way to pass
        // an S2 to an S1 recipient (structs are fixed size and carry no typeinfo).
        assert f(S1 {}) == 1
        assert f(S2 {}) == 2
        class C1:
            a:int = 0
            def f(c:int): return 1 + a + c
            def g(c): return c + 1
            def r(c:int): return c
        class C2 : C1
            b:int = 0
            def f(c:int): return 2 + a + b + c
            def g(c): return c + 2
            def r(c:int): return if c: r(this, c - 1) else: r(C1 {}, c)
        let x:C1 = C2 {}
        // Must do dynamic dispatch.
        assert x.f(0) == 2
        // Static dispatch, there are no sub-classes.
        assert f(C2 {}, 0) == 2
        // Dynamic dispatch mixed with specialization (!)
        assert x.g("hi") == "hi2"
        assert x.g(3) == 5
        // Recursive dynamic dispatch.
        assert x.r(10) == 0

    do():
        // Flow sensitive checking.
        let a:string? = ""
        assert a == ""  // Here a is already promoted to non-nil.
        let b:string? = nil
        if b == nil:  // Should work similarly to just "b" or "not b".
            nil
        else:
            assert b == ""
        def f(x): assert x == ""
        f(a)  // Promoted type should be passed.

    do():
        // Use of type variables:
        class Ai             // Pre-declared specialization.
        class A<T>:
            ts:[T]           // Type vars may occur nested.
            next:A? = nil    // Automatically "upgraded" to A<T>
            nexts:[A] = []   // Same.
            nexti:Ai? = nil  // Refer to specialization before fully defined!
            def f(t /*:T*/): // TODO!
                ts.push(t)
        class Ai = A<int>    // Specialize.
        let a = Ai { [ 1 ] }
        a.nexts.push(a)
        a.nexts = []
        a.nexti = a
        a.nexti = nil
        a.f(2)
