import std
import exception


// The exception module: try/throw built on non-local returns.

class DExcErr:
    code:int

// Dispatch hierarchy for the throw-through-dispatch test.
abstract class DExcBase:
    def maybe_throw() -> void: return
class DExcQuiet : DExcBase
class DExcLoud : DExcBase
    def maybe_throw() -> void: throw "sub threw"
let throwers = [ DExcQuiet {}, DExcLoud {} ]::DExcBase

def exceptions_tests():
    suite("exceptions")

    // Basic catch, and no-throw path.
    var caught = ""
    try():
        throw "boom"
    fn v:
        caught = v
    test_eq(caught, "boom")
    // No-throw path: the catch is not executed. (A throw must be reachable in
    // the body somewhere or the catch argument type can't be inferred.)
    var ran = 0
    try():
        ran = 1
        if ran == 99: throw "never"
    fn v:
        ran = v.length
    test_eq(ran, 1)

    // Any non-scalar value can be thrown, e.g. objects or vectors.
    var code = 0
    try():
        throw DExcErr { 42 }
    fn v:
        if v is DExcErr:
            code = v.code
    test_eq(code, 42)
    var vlen = 0
    try():
        throw [ 1, 2, 3 ]
    fn v:
        vlen = v.length
    test_eq(vlen, 3)

    // Convenience try without catch arg swallows the exception.
    var after = 0
    try():
        throw "ignored"
    after = 1
    test_eq(after, 1)

    // Nested try: inner throw caught by inner catch; rethrow reaches outer.
    var order = ""
    try():
        try():
            order += "a"
            throw "x"
        fn v:
            order += "b"
            rethrow v
        order += "never"
    fn v:
        order += "c{v}"
    test_eq(order, "abcx")

    // The catch block may take a second arg: the stack trace, which is only
    // non-empty for throw_trace.
    var traced = ""
    try():
        throw_trace("boom")
    fn v, trace:
        traced = v + "|{trace.length}"
    test_eq(traced.find_string("boom|"), 0)
    try():
        throw "plain"
    fn v, trace:
        traced = v + trace
    test_eq(traced, "plain")  // Plain throw: empty trace.

    // throw_if only throws for true (non-nil) values.
    var t1 = 0
    try():
        throw_if(nil::string)
        t1 = 1
        throw_if(if t1 == 1: "err" else: nil)
        t1 = 2
    fn v:
        t1 += 10
    test_eq(t1, 11)

    // protect: the finally block runs on both paths. (Needs an enclosing try
    // since it may rethrow.)
    var cleanups = 0
    try():
        protect():
            throw "p"
        fn:
            cleanups++
    try():
        protect():
            if cleanups == 99: throw "never"
        fn:
            cleanups++
    test_eq(cleanups, 2)

    // Exceptions from recursive calls unwind multiple frames.
    def recurse(n) -> void:
        if n == 0:
            throw "deep"
        recurse(n - 1)
    var got = ""
    try():
        recurse(5)
    fn v:
        got = v
    test_eq(got, "deep")

    // A throw from inside a HOF block unwinds through the HOF.
    got = ""
    try():
        [ 1, 2, 3 ].map() x:
            if x == 2: throw "inmap"
            x
    fn v:
        got = v
    test_eq(got, "inmap")

    // A throw from inside a dynamically dispatched method unwinds through the
    // dispatch.
    got = ""
    try():
        for(throwers) t:
            t.maybe_throw()
    fn v:
        got = v
    test_eq(got, "sub threw")

    // An inner try that swallows leaves the outer catch untouched. (The
    // unreached throw gives the outer catch argument its type.)
    var outer_ran = 0
    try():
        try():
            throw "swallowed"
        outer_ran = 1
        if outer_ran == 99: throw "never"
    fn v:
        outer_ran = v.length
    test_eq(outer_ran, 1)

    // A throw from inside an if value expression unwinds correctly.
    got = ""
    try():
        let x = if date_time_build_info().length != 0: throw "inif" else: 2
        got += "unreached{x}"
    fn v:
        got = v
    test_eq(got, "inif")

exceptions_tests()
