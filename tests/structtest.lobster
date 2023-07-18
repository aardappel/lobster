import testing
import vec

run_test("struct"):

    // Basic struct and nested struct usage.
    do():
        struct A:
            x:int
            y:int
        struct B:
            a:A  // Inline, sizeof(B) == 24
            b:int
        class C:
            a:A  // Also inline, inside heap allocated parent.
            b:int
        var a = int2 { 3, 4 } + int2 { 1, 1 } - int2 { 1, 1 }
        let b = B { A { 5, 6 }, a.x + a.y }
        assert b.a.x + b.a.y + b.b == 18
        var aa = a + a
        assert aa.x + aa.y == 14
        assert aa + 1 == int2 { 7, 9 }
        a += a
        assert a == int2 { 6, 8 }
        aa += 1
        assert aa == int2 { 7, 9 }
        let c = C { A { 10, 11 }, 12 }
        assert c.a.x + c.a.y + c.b == 33
        c.a += 1
        assert c.a == A { 11, 12 }
        c.a += A { 1, 2 }
        assert c.a == A { 12, 14 }
        c.a = A { 3, 4 }
        assert c.a == A { 3, 4 }
        assert int(float2 { 1.0, 2.0 }) == int2 { 1, 2 }
        assert min(int2 { 1, 2 }) == 1
        assert manhattan(int2 { 2, 3 }) == 5
        var nv = float2_0
        nv = normalize(float2 { 123.0, 456.0 })
        assert abs(magnitude(nv) - 1.0) < 0.001
        nv = normalize(float2_0)
        assert abs(magnitude(nv)) < 0.001  // Passthrough of zero vec
        let v = [ int2 { 1, 2 }, int2 { 3, 4 } ]
        assert v[1] == int2 { 3, 4 }
        v[1] = int2 { 5, 6 }
        assert v[1] == int2 { 5, 6 }
        assert v[1].y == 6
        v[1] += int2 { 1, 1 }
        assert v[1] == int2 { 6, 7 }
        v[1] += 1
        assert v[1] == int2 { 7, 8 }
        let tri = [ [ [ 123 ] ] ]
        assert tri[int3 { 0, 0, 0 }] == 123
        tri[int3 { 0, 0, 0 }] = 124
        assert tri[int3 { 0, 0, 0 }] == 124
        let triv = [ [ [ int2 { 10, 11 } ] ] ]
        assert triv[int3 { 0, 0, 0 }] == int2 { 10, 11 }
        triv[int3 { 0, 0, 0 }] += 4
        assert triv[int3 { 0, 0, 0 }] == int2 { 14, 15 }
        let trivv = [ [ [ B { A { 5, 6 }, 7 } ] ] ]
        assert trivv[int3 { 0, 0, 0 }].a == A { 5, 6 }
        assert trivv[int3 { 0, 0, 0 }].a.y == 6
        assert trivv[int2 { 0, 0 }][0].a.y == 6
        assert int3 { 5, 6, 7 }[1] == 6

    // Control structures.
    do():
        assert (if manhattan(int2 { 1, 1 }): int2 { 1, 2 } else: int2 { 3, 4 }).y == 2
        assert (if true: int2 { 1, 2 } else: int2 { 3, 4 }).y == 2  // Compile time.
        assert int2 { 1, 2 } == switch manhattan(int2 { 1, 1 }):
                                case 2: int2 { 1, 2 }
                                default: int2 { 3, 4 }
        for([ int2 { 1, 2 }, int2 { 0, 3 } ]) st: assert st.x + st.y == 3
        struct xy_s = vec2<string>
        for([ xy_s { "a", "bb" }, xy_s { "ab", "b" } ]) st: assert st.x + st.y == "abb"

    // Function calls.
    do():
        def f(a, b, c):
            return a + b.x + b.y + c
        def g<T>(a:T): return 1, vec2<T> { a, a }, 1
        assert f(1, int2 { 2, 3 }, 4) == 10
        let mr1, mr2, mr3 = g(7)
        assert mr2 == int2 { 7, 7 } and mr1 == 1 and mr3 == 1

    // As part of dynamic dispatch.
    do():
        class mm1:
            m:int
        class mm2 : mm1
        def mm(a:mm1, b:int2): return a.m + b.x + b.y
        def mm(a:mm2, b:int2): return a.m + b.x + b.y
        assert mm(mm1 { 1 }, int2 { 4, 5 }) == 10
        assert mm(mm2 { 0 }, int2 { 4, 5 }) == 9

    // Structs with reference counted members.
    // This very preliminary: needs more testing.
    do():
        struct R:
            a:string
            b:[int]
        struct RN:
            a:string
            r:R
        let r = R { "hello", [ 123 ] }
        def f(x): return R { x.a + x.a, append(x.b, x.b) }
        var fr = f(r)
        assert fr.a == "hellohello"
        let rs = [ R { "one", [ 1 ] }, R { "two", [ 2 ] } ]
        fr = f(rs[1])
        assert fr.a == "twotwo"
        let rn = RN { "A", R { "B", [ 64 ] } }
        def fun(x): return RN { x.a + x.a, R { x.r.a + x.r.a, append(x.r.b, x.r.b) } }
        var frn = fun(rn)
        assert frn.a == "AA" and frn.r.b[1] == 64
        let rns = [ RN { "AA", R { "one", [ 1 ] } }, RN { "BB", R { "two", [ 2 ] } } ]
        frn = fun(rns[1])
        assert frn.a == "BBBB" and frn.r.b[1] == 2

    // Mixed refc & scalar:
    do():
        struct RS:
            a:string
            b:int
        var rs = RS { "hi", 1 }
        def frs(x): return RS { x.a + x.a, x.b + x.b }
        rs = frs(rs)
        assert rs.a == "hihi" and rs.b == 2
        assert string(rs) == "RS\{\"hihi\", 2\}"

    // We used to only support math on structs of sizes 2..4
    do():
        struct V1:
            x:int
        assert V1 { 1 } + V1 { 1 } == V1 { 2 }
        assert min(V1 { 2 }, max(V1 { 1 }, V1 { -1 })) == V1 { 1 }
        assert manhattan(V1 { 2 }) == 2

        struct V5:
            x:float
            y:float
            z:float
            w:float
            v:float
        let v5test = V5 { 1.0, 2.0, 3.0, 4.0, 5.0 }
        let v5test2 = v5test + v5test
        assert v5test2 == v5test * 2.0
        assert magnitude(normalize(v5test2)) == 1.0
        assert min(v5test, max(v5test, v5test2)) == v5test
        assert dot(v5test, v5test2) == 110.0

    // Tests for struct operators with scalars
    do():
        // Communitive ops

        // Multiplication
        assert int2 { 2, 3 } * 5 == int2 { 10, 15 }
        assert 5 * int2 { 2, 3 } == int2 { 10, 15 }
        assert int3 { 2, 3, 4 } * 5 == int3 { 10, 15, 20 }
        assert 5 * int3 { 2, 3, 4 } == int3 {10, 15, 20 }

        // Test with float struct
        assert float2 { 2.1, 3.2 } * 5.0 == float2 { 10.5, 16.0 }
        assert 5.0 * float2 { 2.1, 3.2 } == float2 { 10.5, 16.0 }

         // Test with a large numeric struct
        struct A:
            a:int
            b:int
            c:int
            d:int
            e:int
        
        assert A { 2, 3, 4, 5, 6 } * 5 == A { 10, 15, 20, 25, 30 } 
        assert 5 * A { 2, 3, 4, 5, 6 } == A { 10, 15, 20, 25, 30 }   

        // Addition
        assert 5 + int2 { 2, 3 } == int2 { 7, 8 }
        assert int2 { 2, 3 } + 5 == int2 { 7, 8 }
        assert int3 { 2, 3, 4 } + 5 == int3 { 7, 8, 9 }
        assert 5 + int3 { 2, 3, 4 } == int3 { 7, 8, 9 }

        // Non-communtive ops

        // Subtraction
        assert 5 - int2 { 2, 3} == int2 { 3, 2}
        assert int2 { 2, 3} - 5 == int2 { -3, -2}
        assert 5 - int3 { 2, 3, 4} == int3 { 3, 2, 1 }
        assert int3 { 2, 3, 4} - 5 == int3 { -3, -2, -1 }

        // Division
        assert 10 / int2 { 5, 10 } == int2 { 2, 1 }
        assert int2 { 10, 20 } / 5 == int2 { 2, 4 }
        assert 10 / int3 { 5, 10, 1 } == int3 { 2, 1, 10 }
        assert int3 { 10, 20, 25 } / 5 == int3 { 2, 4, 5 }

        // Modulo
        assert 10 % int2 { 2, 3 } == int2 { 0, 1 }
        assert int2 { 10, 21 } % 4 == int2 { 2, 1 }
        assert 10 % int3 { 2, 3, 4 } == int3 { 0, 1, 2 }
        assert int3 { 10, 21, 24 } % 4 == int3 { 2, 1, 0 }

        // Compartive ops

        // Less than
        assert int2 { 2, 5 } < 5 == int2 { 1, 0 }
        assert 5 < int2 { 2, 5 } == int2 { 0, 0 } 
        assert int2 { 2, 6 } < 5 == int2 { 1, 0 }
        assert 5 < int2 { 2, 6 } == int2 { 0, 1 } 

        assert int3 { 2, 5, 6 } < 5 == int3 { 1, 0, 0 }
        assert 5 < int3 { 2, 5, 6 } == int3 { 0, 0, 1 } 

        // Less than or equal
        assert int2 { 2, 5 } <= 5 == int2 { 1, 1 }
        assert 5 <= int2 { 2, 5 } == int2 { 0, 1 } 
        assert int2 { 2, 6 } <= 5 == int2 { 1, 0 }
        assert 5 <= int2 { 2, 6 } == int2 { 0, 1 } 

        assert int3 { 2, 5, 6 } <= 5 == int3 { 1, 1, 0 }
        assert 5 <= int3 { 2, 5, 6 } == int3 { 0, 1, 1 } 

        // Greater than
        assert int2 { 2, 5 } > 5 == int2 { 0, 0 }
        assert 5 > int2 { 2, 5 } == int2 { 1, 0 } 
        assert int2 { 2, 6 } > 5 == int2 { 0, 1 }
        assert 5 > int2 { 2, 6 } == int2 { 1, 0 } 

        assert int3 { 2, 5, 6 } > 5 == int3 { 0, 0, 1 }
        assert 5 > int3 { 2, 5, 6 } == int3 { 1, 0, 0 } 

        // Greater than or equal
        assert int2 { 2, 5 } >= 5 == int2 { 0, 1 }
        assert 5 >= int2 { 2, 5 } == int2 { 1, 1 } 
        assert int2 { 2, 6 } >= 5 == int2 { 0, 1 }
        assert 5 >= int2 { 2, 6 } == int2 { 1, 0 } 

        assert int3 { 2, 5, 6 } >= 5 == int3 { 0, 1, 1 }
        assert 5 >= int3 { 2, 5, 6 } == int3 { 1, 1, 0 } 
