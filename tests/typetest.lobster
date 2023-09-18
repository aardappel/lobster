import testing
import std
import dictionary

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
        assert "{true}" == "true"
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
        abstract class C0:
            def ad(): return 10
        class C1 : C0
            a:int = 0
            def f(c:int): return 1 + a + c
            def g(c): return c + 1
            def r(c:int) -> int: return c
            def mr(): return 1, "a"
            def sup(): return 1
            def sni(): return 12
        class C2 : C1
            b:int = 0
            def f(c:int): return 2 + a + b + c
            def g(c): return c + 2
            def r(c:int) -> int: return if c: r(c - 1) else: (C1 {}).r(c)
            def mr(): return 2, "b"
            def sup(): return 1 + super sup()
            def ad(): return 11
        class C3<T> : C1
            b:T
            def f(c:int): return c + length(b)
        let x:C1 = C2 {}
        let x2:C1 = C3<string> { "hello" }
        // Must do dynamic dispatch.
        assert x.f(0) == 2
        // Static dispatch, there are no sub-classes.
        assert f(C2 {}, 0) == 2
        // Dynamic dispatch mixed with specialization (!)
        assert x.g("hi") == "hi2"
        assert x.g(3) == 5
        assert x2.f(10) == 15  // Generic "this"
        // Recursive dynamic dispatch.
        assert x.r(10) == 0
        // Call super methods:
        assert x.sup() == 2
        // Call method from above dispatch root:
        assert x.ad() == 11
        let x3:C0 = C1 {}
        // Dynamic dispatch with unused UDT impl missing:
        assert x3.sni() == 12
        // Multiple return:
        let mr1, mr2 = x.mr()
        assert mr1 == 2 and mr2 == "b"
        // Static dispatch on generic types!
        class D<T>:
            x:T
        def foo<T>(d:D<T>): return d.x
        def foo(i:int): return i
        def foo(s:string): return s
        def foo<T>(t:T): return t + 1
        assert foo(D<int> { 1 }) + foo(2) == 3
        assert foo(D<string> { "ha" }) + foo("llo") == "hallo"
        assert foo(1.0) == 2.0

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
        class A<T>:
            ts:[T]             // Type vars may occur nested.
            next:A<T>? = nil
            nexts:[A<T>] = []
            nexti:A<int>? = nil
            nexto:A<string>? = nil    // Refer to unnamed specialization.
            def f(t:T):        // Generic from surrounding class.
                ts.push(t)
            def g<U>(u:U):     // Own generic.
                ts.push(u)
            def h():
                let x:T = true // Use as internal type.
                return x
        def i<T>(a::A<T>, t:T):// Non-inline "method".
            ts.push(t)

        class Ai = A<int>      // Specialize.
        let a = Ai { [ 1 ] }
        a.nexts.push(a)
        a.nexts = []
        a.nexti = a
        a.nexti = nil
        a.f(2)
        a.g(3)
        a.i(4)
        assert a.h() == 1

        let b = A<string> { [ "hello" ] }  // Look ma, no named specialization.
        b.nexts.push(b)
        b.nexts = []
        b.nexto = b
        b.nexto = nil
        b.f("hi")
        b.g("yo")
        b.i("sup")

        /*
        class B : Ai
            def f(t:T):   // Explicit generic mixed with dynamic dispatch!
                ts.push(t + 1)
        let aaa:Ai = B { [ 1 ] }
        aaa.f(3)
        assert aaa.ts.top() == 4
        */
        /*
        class B : Ai
            def g<U>(u:U):   // Explicit generic mixed with dynamic dispatch!
                ts.push(u + 1)
        let aaa:Ai = B { [ 1 ] }
        aaa.g(3)
        assert aaa.ts.top() == 4
        */

        // Explicit use in functions.
        def j<T>(x:T, y:T): return y
        assert j<float>(1, 1) is float  // Explicit specializer.
        assert j(1.0, 1) is float       // Implicit, from 1st arg.
        def k<T>(t:T) -> T: return 1    // Explicit return type.
        assert k<float>(2) == 1.0
        assert k(2.0) == 1.0
        def q<T>() -> T, T:             // multiple return.
            return 1, 1
        let r, s = q<float>()
        assert r is float and s is float
        def t0(aa:Ai?) -> Ai:
            return A<int> { [ if aa: aa.ts[0] else: 2 ] }
        assert t0(a).ts[0] == 1

        // Use with various explicit type features.
        def l<T>(): return 1.0 is T     // Use with is.
        assert l<float>()
        def n<T>(): return nil::T       // As nil type.
        assert n<string>() == nil::string
        def o<T>(): return []::T        // As vector type.
        assert equal(o<string>(), []::string)
        def p<T>(): return typeof T     // Use with typeof.
        assert p<string>() == 3

        // Use without an named specialization type.
        def t1<V>(aa:A<V>) -> A<V>:
            return A<V> { [ aa.ts[0] ] }
        assert t1(a).ts[0] == 1
        def t2<V>(aa:A<V>?) -> A<V>:
            return A<V> { [ if aa: aa.ts[0] else: 2 ] }
        assert t2(a).ts[0] == 1

    do():
        // Test generics passed on to default values.
        class set1<T>:
            dict:dictionary<T, int> = dictionary<T, int>(5)
        class set2<T>:
            dict = dictionary<T, int>(15)
        let a1 = set1<string> {}
        let a2 = set2<string> {}
        a1.dict.set("a", 13)
        a2.dict.set("a", 13)
        assert a1.dict.get("a", 0) == a2.dict.get("a", 0)

    do():
        // Instantiate generics while choosing an overload.
        def make_set<T>(size:int): return 1
        def make_set<T>(xs:[T]): return 2
        assert make_set<int>([1,2,3]) == 2

    do():
        // Vector type params are typically invariant, to avoid illegal
        // mutation via aliases. They can be covariant with exceptions:
        class A:
            a:int = 1
        class B : A
            b:int = 2
        let as = [ A {} ]
        let bs = [ B {} ]
        // append args marked as const / non aliasing:
        append(as, bs)
        append(bs, as)
        // RHS is constructor so can't alias:
        let xs:[A] = [ B {} ]
        // Same in function call context
        def f(ys:[A]): ys
        f([ B {} ])
        // Or from a union:
        f(if as[0].a == 1: [ A {} ] else: [ B {} ])

    do():
        // Classes may have attributes.
        class A:
            attribute serializable = 1
            a = 2
        A {}
