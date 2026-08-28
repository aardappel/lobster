import std

// The parser warns about expression statements that have no effect
// ("expression statement has no effect"), which catches errors like == where =
// was intended, or a multi-line expression where the connecting operator was
// forgotten. Warnings can't be observed from within a program, so these tests
// cover the patterns that must NOT be affected by it, plus that such a
// statement is a warning, not an error.

def noeffect_tests():
    suite("noeffect")

    // A pure expression as the last statement of a block is its value, not dead code.
    let a = if 1 > 0: 1 + 1 else: 0
    test_eq(a, 2)

    // and/or with a side effecting operand used as a statement is a valid idiom.
    var calls = 0
    def bump():
        calls++
        return true
    1 > 0 and bump()
    0 > 1 or bump()
    test_eq(calls, 2)

    // Calls conservatively count as side effects, even when their body is pure.
    def pure(x): return x + 1
    pure(1)

    // Control flow and assert made of pure parts are exempt.
    var n = 0
    for(3): if _ > 0: n++
    test_eq(n, 2)

    // A statement with no effect compiles and runs fine (warning, not error).
    if not vm_compiled_mode():
        let ret, err = compile_run_code("let x = 1\nx == 2\nreturn x", [])
        test_eq(err, nil)
        test_eq(ret, "1")

noeffect_tests()
