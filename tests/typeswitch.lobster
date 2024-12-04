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

    // Similarly, enums need to be checked exhaustively. A compile time error will result
    // if any enum value is not covered.

    let btests = [ true, false /*, bool(2)*/ ]

    let results5 = map(btests) t:
        switch t:
            case true: "t"
            case false: "f"
            // No default needed!
            // If bool(2) is a possible value, this switch will generate a runtime error.

    assert equal(results5, [ "t", "f" ])

    let btests2 = [ true, false, bool(2) ]

    let results6 = map(btests2) t:
        switch t:
            case true: "t"
            case false: "f"
            default:
                // Unlike class instances which can't be forged, it is possible to cast any
                // integer to an enum. This must generally be possible to load files that may
                // have old/incorrect enum values stored.
                // If this is possible, a default case should be used, since in the switch above
                // this one, a lack of a default case will result in a runtime error if an
                // illegal value is used.
                // Of course, a default case has the disadvantage that you will not get errors
                // about new enum values not having their own case.
                "?"

    assert equal(results6, [ "t", "f", "?" ])
