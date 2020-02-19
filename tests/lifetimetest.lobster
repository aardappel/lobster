import testing
import std

// Misc tests related to lifetime checking.

run_test("lifetime"):

    do():
        let v = [ "" ]
        var k = v[0]
        k = v[0]

    do():
        let v = [ 1 ]
        var k = v[0]
        k = v[0]


