import testing
import std

// Misc tests for builtin functions.

run_test("builtins"):
    do():
        class Nest:
            a:[Nest]
            b:string
            c:float2
            d:Nest?
            e:[string]
            f:[[int]]
        let nested = Nest { [], "a", float2 { 1.0, 2.0 }, nil, [ "b", "c" ], [ [ 1, 2 ], [ 3, 4 ] ] }
        let sub = deepcopy(nested, 10)
        nested.d = sub
        nested.a.push(sub)
        nested.a.push(sub)
        assert equal(deepcopy(nested, 10), nested)


