include testing
include vec

run_test("misch"):
    struct A { x:int, y:int }
    struct B { a:A, b:int }  // Inline, sizeof(B) == 12 :)
    class C { a:A, b:int }  // Also inline, inside heap allocated, refcounted parent.

    def f(a, b, c):
        return a + b.x + b.y + c
    def g(a): return 1, xy { a, a }, 1

    class mm1 { m:int }
    class mm2 : mm1 {}
    def mm(a:mm1, b:xy_i): return a.m + b.x + b.y
    def mm(a:mm2, b:xy_i): return a.m + b.x + b.y

    a := xy { 3, 4 } + xy { 1, 1 } - xy { 1, 1 }
    b := B { A { 5, 6 }, a.x + a.y }
    assert b.a.x + b.a.y + b.b == 18
    aa := a + a
    assert aa.x + aa.y == 14
    assert aa + 1 == xy { 7, 9 }
    a += a
    assert a == xy { 6, 8 }
    aa += 1
    assert aa == xy { 7, 9 }
    c := C { A { 10, 11 }, 12 }
    assert c.a.x + c.a.y + c.b == 33
    c.a += 1
    assert c.a == A { 11, 12 }
    c.a += A { 1, 2 }
    assert c.a == A { 12, 14 }
    c.a = A { 3, 4 }
    assert c.a == A { 3, 4 }
    assert int(xy { 1.0, 2.0 }) == xy { 1, 2 }
    assert min(xy { 1, 2 }) == 1
    assert manhattan(xy { 2, 3 }) == 5
    nv := xy_0
    nv = normalize(xy { 123.0, 456.0 })
    assert abs(magnitude(nv) - 1.0) < 0.001
    v := [ xy { 1, 2 }, xy { 3, 4 } ]
    assert v[1] == xy { 3, 4 }
    v[1] = xy { 5, 6 }
    assert v[1] == xy { 5, 6 }
    assert v[1].y == 6
    v[1] += xy { 1, 1 }
    assert v[1] == xy { 6, 7 }
    v[1] += 1
    assert v[1] == xy { 7, 8 }
    assert f(1, xy { 2, 3 }, 4) == 10
    mr1, mr2, mr3 := g(7)
    assert mr2 == xy { 7, 7 }
    tri := [ [ [ 123 ] ] ]
    assert tri[xyz { 0, 0, 0 }] == 123
    tri[xyz { 0, 0, 0 }] = 124
    assert tri[xyz { 0, 0, 0 }] == 124
    triv := [ [ [ xy { 10, 11 } ] ] ]
    assert triv[xyz { 0, 0, 0 }] == xy { 10, 11 }
    triv[xyz { 0, 0, 0 }] += 4
    assert triv[xyz { 0, 0, 0 }] == xy { 14, 15 }
    trivv := [ [ [ B { A { 5, 6 }, 7 } ] ] ]
    assert trivv[xyz { 0, 0, 0 }].a == A { 5, 6 }
    assert trivv[xyz { 0, 0, 0 }].a.y == 6
    assert trivv[xy { 0, 0 }][0].a.y == 6
    assert xyz { 5, 6, 7 }[1] == 6
    assert mm(mm1 { 1 }, xy { 4, 5 }) == 10
    assert mm(mm2 { 0 }, xy { 4, 5 }) == 9
