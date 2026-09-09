import std


// Inlining of functions with more than one return, see Call::Optimize: the value has to end
// up where the block leaves it, whatever was live inside the block has to go, and whatever
// the caller had live around the call has to survive.

def inlining_tests():
    suite("inlining")

    def clamp_step(v:int, lo:int, hi:int) -> int:
        if v < lo: return lo
        if v > hi: return hi
        return v + 1

    def classify(v:int) -> int:
        switch v % 7:
            case 0, 1: return 1
            case 2: return 2
            case 3, 4, 5: return 3
        return 4

    def first_over(xs:[int], lim:int) -> int:
        for(xs) x, i:
            if x > lim: return i
        return -1

    def first_over_owned(lim:int) -> int:
        // The loop owns its vector, so a return out of it has to release that.
        for(map(6): _ * 3) x, i:
            if x > lim: return i
        return -1

    def find_str(xs:[string], s:string) -> string:
        for(xs) x:
            if x == s: return x + "!"
        return "none"

    def lookup(xs:[string], s:string) -> string?, int:
        var i = 0
        while i < xs.length:
            if xs[i] == s: return xs[i], i
            i++
        return nil, -1

    def either(c:bool) -> int:
        if c:
            return 1
        else:
            return 2

    def nested_ifs(a:int, b:int) -> int:
        if a > 0:
            if b > 0: return 1
            return 2
        return 3

    def in_call(c:bool) -> int:
        // A return in the argument of a call, after other arguments were already evaluated.
        return max(10, if c: return 7 else: 3)

    def in_expr(c:bool) -> int:
        // A return inside an expression with a temp live below it.
        return 100 + (if c: return 5 else: 1)

    def strs(c:int) -> string:
        // Owned values live across the return.
        let a = "a" + string(c)
        if c > 1: return a + "big"
        let b = a + "small"
        if c > 0: return b
        return a + b

    def pair(c:int) -> int, string:
        if c == 0: return 0, "zero"
        if c == 1: return 1, "one"
        return c, "many"

    def vec(c:int) -> float2:
        if c == 0: return float2 { 1.0, 2.0 }
        return float2 { 3.0, 4.0 }

    def void_early(xs:[int], v:int):
        // No values: the jump only has to leave the loop.
        for(xs) x, i:
            if x == v:
                xs[i] = -1
                return
        xs.push(v)

    var acc = 0
    for(10) i:
        acc += clamp_step(i - 5, -2, 2)
    test_eq(acc, 3)

    var cl = 0
    for(21) i: cl += classify(i)
    test_eq(cl, 3 * (1 + 1 + 2 + 3 + 3 + 3 + 4))

    let xs = [1, 5, 2, 8, 3]
    test_eq(first_over(xs, 4), 1)
    test_eq(first_over(xs, 7), 3)
    test_eq(first_over(xs, 9), -1)
    test_eq(first_over_owned(7), 3)
    test_eq(first_over_owned(100), -1)

    let ss = ["ab", "cd", "ef"]
    test_eq(find_str(ss, "cd"), "cd!")
    test_eq(find_str(ss, "zz"), "none")
    let l1, i1 = lookup(ss, "ef")
    let l2, i2 = lookup(ss, "q")
    test_eq(l1, "ef")
    test_eq(i1, 2)
    test_true(not l2)
    test_eq(i2, -1)

    test_eq(either(true), 1)
    test_eq(either(false), 2)
    test_eq(nested_ifs(1, 1), 1)
    test_eq(nested_ifs(1, 0), 2)
    test_eq(nested_ifs(0, 1), 3)
    test_eq(in_call(true), 7)
    test_eq(in_call(false), 10)
    test_eq(in_expr(true), 5)
    test_eq(in_expr(false), 101)
    test_eq(strs(2), "a2big")
    test_eq(strs(1), "a1small")
    test_eq(strs(0), "a0a0small")

    var pacc = 0
    var pstr = ""
    for(4) i:
        let n, s = pair(i)
        pacc += n
        pstr += s
    test_eq(pacc, 6)
    test_eq(pstr, "zeroonemanymany")
    test_eq(vec(0), float2 { 1.0, 2.0 })
    test_eq(vec(1), float2 { 3.0, 4.0 })

    let ys = [1, 2, 3]
    void_early(ys, 2)
    void_early(ys, 9)
    test_seq(ys, [1, -1, 3, 9])

    // The same function inlined twice into one caller, with a value live around each call.
    let keep = "k" + string(acc)
    let r1 = first_over(xs, 1) + first_over(xs, 6)
    test_eq(r1, 1 + 3)
    test_eq(keep, "k3")

    // Inside a loop of the caller, with the caller's own iterator live below the block.
    var tot = 0
    for(ss) s, j:
        tot += first_over(xs, j * 3) + (if find_str(ss, s) == s + "!": 10 else: 0)
    test_eq(tot, 0 + 1 + 3 + 30)

    // One inlined multi-return function called from another.
    def outer_calls(v:int) -> int:
        if v < 0: return -1
        return clamp_step(classify(v), 1, 2) * 10
    test_eq(outer_calls(-1), -1)
    test_eq(outer_calls(0), 20)
    test_eq(outer_calls(2), 30)
    test_eq(outer_calls(6), 20)

    // A non-local return out of a callee that is reused by a second specialization of the
    // function it returns to: that specialization must not be inlined either, see
    // ReplayReturns.
    class reused_marker:
        v:int
    def reused_target(a):
        reused_thrower()
        return reused_marker { 1 }
    def reused_thrower(): return reused_marker { 0 } from reused_target
    test_eq(reused_target(1).v, 0)
    test_eq(reused_target("").v, 0)
    test_eq(reused_target(1.0).v, 0)

inlining_tests()
