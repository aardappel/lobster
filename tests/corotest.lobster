import testing
import std

run_test("coro"):

    def mycoro(f):
        for_range(3, 6): f(_)
        return 1337

    def myfor(n, f):
        for n: f(_)
        return 0

    let co1 = coroutine mycoro()   // out of nested HOFs
    let co2 = coroutine myfor(4)   // out of a simple function

    def interleave(a, b, f):   // test using 2 at once
        while a.active or b.active:
            if a.active:
                f(a.return_value)
                a.resume
            if b.active:
                f(b.return_value)
                b.resume

    //interleave(co1, co2): print(_)

    assert equal(collect() f: interleave(co1, co2, f), [ 3, 0, 4, 1, 5, 2, 3 ])
    assert !co1.active
    assert !co2.active
    assert co1.return_value == 1337

    def pass_thru(co, f):
        return coroutine_for co: f(_)

    let co3 = coroutine pass_thru(coroutine myfor(10))    // test consuming 1 coro with another and then passing that on
    var sum = 0
    while co3.active:       // we currently can't use coroutine_for here, will be a runtime error (nesting)
        sum += co3.return_value
        co3.resume
    assert sum == 45

    // Access variables inside coroutines from the outside:
    def loctest(f):
        let a = 1
        for(10) i:
            let b = 2
            f(nil)
        return nil

    let co4 = coroutine loctest()
    assert co4.active
    assert co4->a + co4->i + co4->b == 3

    // Even functions that don't yield are cool:
    def conoyield(f): return 1
    let co5 = coroutine conoyield()
    assert !co5.active
    assert co5.return_value == 1

    def refcoro(borrow, f):
        let ss = [ "A", "B", "C" ]
        for(ss): f(_)
        return "D"
    let borrow = "E"
    assert (coroutine_for(coroutine refcoro(borrow)): assert(_ + "_" is string)) == "D"
    let rc = coroutine refcoro(borrow)
    assert rc->ss[1] == "B"
    rc->ss[1] = "F"
    rc.resume()
    assert rc->ss[1] == "F"