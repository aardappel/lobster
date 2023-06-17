import std
import testing

run_test("typeswitch"):

    abstract class A:
        a = 0

    class B : A
        b = 1

    abstract class C : A
        c = 2

    class D : C
        d = 3

    class E : C
        e = 4

    class F : E
        f = 5

    // 6 classes, but 2 are abstract, so this switch will demand that exactly 4 cases
    // must always be covered!

    /*

    (A)
     |\
     B (C)
        |\
        D E
          |
          F

    */

    let tests = [ B {}, D {}, E {}, F {} ]

    // Exactly 1 case per class.
    let results1 = map(tests) t:
        switch t:
            case B: t.b
            case D: t.d
            case E: t.e
            case F: t.f
            // No default needed / allowed!

    assert equal(results1, [ 1, 3, 4, 5 ])

    // Subclasses can be done by superclasses.
    let results2 = map(tests) t:
        switch t:
            case B: t.b
            case D: t.d
            case E: t.e
            // No default needed / allowed!

    assert equal(results2, [ 1, 3, 4, 4 ])

    // Abstract base class may implement for all subclasses.
    let results3 = map(tests) t:
        switch t:
            case B: t.b
            case C: t.c
            // No default needed / allowed!

    assert equal(results3, [ 1, 2, 2, 2 ])

    // Probably bad practice, but defaults are still allowed.
    let results4 = map(tests) t:
        switch t:
            default: t.a  // Can't access anything else!
            case F: t.f

    assert equal(results4, [ 0, 0, 0, 5 ])
