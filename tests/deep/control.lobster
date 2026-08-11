import std


// Control structures: if/elif/else, for, while, break, continue, guard,
// switch, assert, and conditional declarations.

def control_tests():
    suite("control")

    // if as expression, one-liner and multi-line, elif chains.
    test_eq(if 1: "y" else: "n", "y")
    test_eq(if 0: "y" else: "n", "n")
    let classify = fn a:
        if a < 0: "neg"
        elif a == 0: "zero"
        elif a < 10: "small"
        else: "big"
    test_eq(classify(-5), "neg")
    test_eq(classify(0), "zero")
    test_eq(classify(5), "small")
    test_eq(classify(50), "big")
    // if without else produces no value but is legal as statement.
    if classify(1) == "small":
        pass()
    // Any false-y value selects the else branch.
    test_eq(if nil: 1 else: 2, 2)
    test_eq(if 0.0: 1 else: 2, 2)
    // Branches of different numeric types unify to float.
    let rc = 1 < 2
    let iu = 1
    test_true((if rc: iu else: 1.5) is float)
    test_eq(if rc: iu else: 1.5, 1.0)

    // for over int, string, vector; 0, 1 or 2 block args.
    var acc = 0
    for(5) i:
        acc += i
    test_eq(acc, 10)
    // Over an int, element and index are the same.
    for(3) x, i:
        test_eq(x, i)
    acc = 0
    for(3):
        acc += 10
    test_eq(acc, 30)
    acc = 0
    for(-3):
        acc++
    test_eq(acc, 0)  // Negative count: no iterations.
    acc = 0
    for([ 5, 6 ]) x, i:
        acc += x + i
    test_eq(acc, 12)
    acc = 0
    for("ab") c:
        acc += c
    test_eq(acc, 'a' + 'b')
    for([]::int):
        acc += 100  // Empty vector: not reached.
    test_eq(acc, 'a' + 'b')
    while false:
        acc += 100  // Never runs, not even once.
    test_eq(acc, 'a' + 'b')

    // while, incl. side effects in condition.
    var wc = 0
    while wc < 5: wc++
    test_eq(wc, 5)
    var wsum = 0
    var wn = 3
    while wn-- > 0:
        wsum += 10
    test_eq(wsum, 30)

    // break / continue in for and while.
    var bc = 0
    for(10) i:
        if i == 5: break
        bc++
    test_eq(bc, 5)
    bc = 0
    for(10) i:
        if i % 2: continue
        bc++
    test_eq(bc, 5)
    bc = 0
    while true:
        bc++
        if bc == 3: break
    test_eq(bc, 3)
    bc = 0
    var bi = 0
    while bi < 10:
        bi++
        if bi % 2: continue
        bc++
    test_eq(bc, 5)
    // Nested: break only exits the innermost loop.
    var nested = 0
    for(3):
        for(10) j:
            if j == 2: break
            nested++
    test_eq(nested, 6)
    // break inside a switch breaks the enclosing loop, not the switch.
    var swb = 0
    for(5) i:
        switch i:
            case 2:
                break
            default:
                swb++
    test_eq(swb, 2)

    // guard: with and without an else-body, and producing values from blocks.
    let gs = map(4) i:
        guard i >= 2:
            guard i >= 1:
                10
            20
        guard i >= 3:
            30
        40
    test_seq(gs, [ 10, 20, 30, 40 ])
    def guarded(x):
        guard x > 0
        acc += x
    acc = 0
    guarded(-1)
    test_eq(acc, 0)
    guarded(5)
    test_eq(acc, 5)

    // switch on ints: multi-values, inclusive ranges, default, as expression.
    let st = map(10) i: switch i:
        case 1: "no"
        case 2, 3:
            let x = i
            "yes{x}"
        case 4..6, 8: "maybe"
        default: "what?"
    test_seq(st, [ "what?", "no", "yes2", "yes3", "maybe", "maybe", "maybe", "what?", "maybe", "what?" ])
    // switch on floats, with inclusive float ranges.
    def swf(f:float): return switch f:
        case 10.0..20.0: true
        default: false
    test_true(not swf(3.14))
    test_true(swf(15.0))
    test_true(swf(10.0))
    test_true(swf(20.0))  // Inclusive at both ends.
    test_true(not swf(20.01))
    // switch on strings, cases may be arbitrary expressions.
    let animals = [ "cat", "dog" ]
    def swstr(s:string): return switch s:
        case "cat": 1
        case animals[1]: 2
        default: 3
    test_eq(swstr("cat"), 1)
    test_eq(swstr("dog"), 2)
    test_eq(swstr("emu"), 3)
    // A default that is just a value.
    let swd = switch 5:
        case 1: 10
        default: 0
    test_eq(swd, 0)
    // Character constants and negative values in cases and ranges.
    def swc(c:int): return switch c:
        case 'a'..'z': "lower"
        case 'A'..'Z': "upper"
        case -1: "negative"
        default: "other"
    test_eq(swc('q'), "lower")
    test_eq(swc('Q'), "upper")
    test_eq(swc('a'), "lower")
    test_eq(swc('z'), "lower")
    test_eq(swc(-1), "negative")
    test_eq(swc('0'), "other")
    // switch may be a statement (result unused), only running side effects.
    var sweff = 0
    switch 2 * 2:
        case 4: sweff = 1
        default: sweff = 2
    test_eq(sweff, 1)

    // assert passes through its (truthy) value and promotes nilables.
    var forty = 40
    forty += 2
    let av = assert forty
    test_eq(av, 42)
    def maybe_s() -> string?: return "s"
    let ms = assert maybe_s()
    test_eq(ms.length, 1)  // Type is promoted to non-nil string.

    // Conditional declarations in if / guard / assert, incl. chains with and.
    def maybe(x) -> string?: return if x: "v" else: nil
    if let cv = maybe(1):
        test_eq(cv, "v")
    else:
        test_true(false, "if let took else branch")
    if let cv = maybe(0):
        test_true(false, "if let on nil took then branch: {cv}")
    guard let gv = maybe(1):
        test_true(false, "guard let failed")
    test_eq(gv, "v")
    assert let av2 = maybe(1)
    test_eq(av2, "v")
    if let c1 = maybe(1) and let c2 = maybe(1):
        test_eq(c1 + c2, "vv")
    else:
        test_true(false, "chained if let failed")
    if let c1 = maybe(1) and let c2 = maybe(0):
        test_true(false, "chained if let with nil succeeded: {c1} {c2}")

control_tests()
