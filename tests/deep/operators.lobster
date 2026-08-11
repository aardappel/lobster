

// Built-in operators on scalars: arithmetic, bitwise, comparison, logical,
// precedence, increment/decrement, assignment forms.

def operators_tests():
    suite("operators")

    // Integer arithmetic.
    test_eq(2 + 3, 5)
    test_eq(2 - 3, -1)
    test_eq(2 * 3, 6)
    test_eq(7 / 2, 3)
    test_eq(7 % 2, 1)
    // Division / modulo truncate towards zero (C semantics).
    test_eq(-7 / 2, -3)
    test_eq(7 / -2, -3)
    test_eq(-7 % 3, -1)
    test_eq(7 % -3, 1)
    test_eq(-(-5), 5)

    // Float arithmetic, and int->float coercion in mixed expressions
    // (via variables: literals in float context give a warning instead).
    let i1, i7 = 1, 7
    test_eq(2.5 + 0.5, 3.0)
    test_eq(7.0 / 2.0, 3.5)
    test_eq(i1 + 0.5, 1.5)
    test_eq(i7 / 2.0, 3.5)
    test_true(1.5 is float)
    test_true((i1 + 1.5) is float)
    test_approx(4.0 % 1.5, 1.0)
    test_approx(-4.0 % 1.5, -1.0)  // fmod semantics: sign of the dividend.

    // Precedence: * / % over + -, over << >>, over & | ^, over comparisons,
    // over == !=, over and/or.
    test_eq(2 + 3 * 4, 14)
    test_eq(10 - 4 - 3, 3)          // Left associative.
    test_eq(24 / 4 / 2, 3)
    test_eq(1 + 2 << 1, 6)          // Shifts bind looser than +.
    test_eq(1 << 2 & 3, 0)          // Shifts bind tighter than &.
    test_eq(1 | 2 == 3, true)       // Bitwise binds tighter than ==.
    test_eq(2 < 3 == 1 > 0, true)   // Comparisons bind tighter than ==.
    test_true(1 == 1 and 2 == 2)    // and/or loosest.
    test_eq(-2 * 3, -6)
    test_eq(2 * -3, -6)
    // & | ^ are a single precedence level, left associative.
    let i2 = 2
    test_eq(i2 | 1 & 1, 1)          // (2|1)&1, not 2|(1&1).
    test_eq(i2 ^ 2 | 2, 2)          // (2^2)|2.
    // and/or are also one shared level, left associative.
    let i0 = 0
    test_eq(i2 or i0 and i0, 0)     // (2 or 0) and 0, not 2 or (0 and 0).
    test_eq(i0 and i0 or i2, 2)     // (0 and 0) or 2.

    // Integer arithmetic wraps around (two's complement).
    var big = 0x7FFFFFFFFFFFFFFF
    test_eq(big + 1, 0x8000000000000000)
    test_true(big + 1 < 0)
    big = 0x8000000000000000
    test_eq(big - 1, 0x7FFFFFFFFFFFFFFF)
    test_eq(big * 2, 0)
    test_eq(-big, big)       // Negating int min wraps back to itself.
    test_eq(abs(big), big)

    // Bitwise.
    test_eq(1 & 3, 1)
    test_eq(1 | 4, 5)
    test_eq(5 ^ 3, 6)
    test_eq(~0, -1)
    test_eq(~1, -2)
    test_eq(1 << 4, 16)
    test_eq(256 >> 4, 16)
    test_eq(-8 >> 1, -4)   // Right shift is arithmetic.
    test_eq(-1 << 1, -2)
    // Shift counts are masked to 6 bits (like Java/C#/JS/Wasm); the var
    // forces the runtime path so it and the constant-folded path must agree.
    var sh = 64
    test_eq(1 << 64, 1)
    test_eq(1 << sh, 1)
    sh = -1
    test_eq(1 << -1, 1 << 63)
    test_eq(1 << sh, 1 << 63)
    sh = 100
    test_eq(256 >> 100, 256 >> 36)
    test_eq(256 >> sh, 256 >> 36)

    // Comparisons on ints, floats and mixed.
    test_true(1 < 2 and 2 > 1 and 1 <= 1 and 1 >= 1 and 1 == 1 and 1 != 2)
    test_true(1.5 < 2.5 and 2.5 >= 2.5 and 1.5 != 2.5)
    test_true(i1 < 1.5 and i1 * 2 > 1.5 and i1 == 1.0)

    // Logical operators return one of their operands.
    test_eq(2 and 3, 3)
    test_eq(0 and 3, 0)
    test_eq(2 or 3, 2)
    test_eq(0 or 3, 3)
    test_eq("" or "a", "")    // "" is a true value, unlike e.g. Python!
    test_eq(nil or "a", "a")
    test_eq(0.0 or 2.5, 2.5)
    // int and float do NOT unify in and/or (no coercion): as with any
    // operands of unrelated types, the result is a bool.
    test_eq(i0 or 2.5, true)
    test_eq(i2 and 2.5, true)
    test_eq(0.0 and i2, false)
    // With operands of unrelated types the result is a bool instead.
    let stra = "a"
    test_eq(i2 and stra, true)
    test_eq(i0 and stra, false)
    test_eq(stra or i2, true)
    // Comparisons produce bool.
    test_eq(1 < 2, true)
    test_eq(1 > 2, false)
    // not turns false values into 1, everything else into 0.
    test_eq(not 0, 1)
    test_eq(not 42, 0)
    test_eq(not nil, 1)
    test_eq(not 0.0, 1)
    test_eq(not "", 0)
    test_eq(not not 42, 1)
    // not binds looser than ==, so this is not(1 == 2).
    test_eq(not 1 == 2, 1)
    // is binds tighter than and/or; this is (1 is int) and (1.5 is float).
    test_true(1 is int and 1.5 is float)
    // not applied to an is-test.
    test_eq(not 1 is float, 1)
    // Short circuiting: the rhs must not be evaluated.
    var sideeffect = 0
    def bump():
        sideeffect++
        return true
    test_true(0 and bump() or 1 or bump())
    test_eq(sideeffect, 0)

    // Increment / decrement, pre (new value) and post (old value).
    var incr = 0
    test_eq(incr++, 0)
    test_eq(incr, 1)
    test_eq(++incr, 2)
    test_eq(incr--, 2)
    test_eq(--incr, 0)
    test_eq(incr++ + ++incr + incr-- + --incr, 4)
    var fincr = 1.0
    test_eq(++fincr, 2.0)
    // Also on vector elements and fields.
    let iv = [ 10 ]
    test_eq(iv[0]++, 10)
    test_eq(++iv[0], 12)

    // Assignment operators, equivalent to their expanded forms.
    var acc = 10
    acc += 5
    test_eq(acc, 15)
    acc -= 3
    test_eq(acc, 12)
    acc *= 2
    test_eq(acc, 24)
    acc /= 5
    test_eq(acc, 4)
    acc %= 3
    test_eq(acc, 1)
    // Bitwise assignment forms; assignments are expressions yielding the lvalue.
    var bits = 1
    test_eq(bits |= 2, 3)
    test_eq(bits &= ~2, 1)
    test_eq(bits ^= 3, 2)
    test_eq(bits <<= 2, 8)
    test_eq(bits >>= 1, 4)
    var fdiv = 7.0
    fdiv /= 2.0
    test_eq(fdiv, 3.5)
    // Plain assignment is also an expression yielding the lvalue,
    // on variables as well as vector elements.
    var pa = 1
    test_eq(pa = 5, 5)
    let pav = [ 1 ]
    test_eq(pav[0] = 3, 3)
    test_eq(pav[0] += 3, 6)
    // Assignment operators on an indexed lvalue evaluate the index once.
    var idxcount = 0
    def idx0():
        idxcount++
        return 0
    pav[idx0()] += 10
    test_eq(idxcount, 1)
    test_eq(pav[0], 16)

    // Multiple assignment / definition, and swap via multiple assignment.
    var ma, mb = 1, 2
    test_eq(ma, 1)
    test_eq(mb, 2)
    ma, mb = mb, ma
    test_eq(ma, 2)
    test_eq(mb, 1)
    let mc, md = 3, 4
    test_eq(mc + md, 7)
    // Multiple assignment with class fields as targets (e.g. from builtins
    // with multiple return values, like the write_ family).
    class WPos:
        buf = ""
        pos = 0
        def write_next():
            buf, pos = buf.write_int8_le(pos, 7)
    let wpos = WPos {}
    wpos.write_next()
    test_eq(wpos.pos, 1)
    test_eq(wpos.buf.read_int8_le(0), 7)
    // Chained plain assignment: = is right associative via the grammar.
    var ch1 = 1
    var ch2 = 2
    ch1 = ch2 = 5
    test_eq(ch1, 5)
    test_eq(ch2, 5)

    // String concatenation with +, converting either side.
    test_eq("ab" + "cd", "abcd")
    test_eq("a" + 1, "a1")
    test_eq(1 + "a", "1a")
    test_eq("a" + 1.5, "a1.5")
    test_eq("a" + [ 1, 2 ], "a[1, 2]")
    test_eq("a" + true, "atrue")  // Enums convert by name.
    test_eq("" + 0, "0")
    var scat = "x"
    scat += "y"
    test_eq(scat, "xy")

    // Equality on references: same object only. equal() for structure.
    let ra = [ 1 ]
    let rb = [ 1 ]
    test_true(ra == ra)
    test_true(ra != rb)
    test_true(equal(ra, rb))
    // Strings compare by value.
    test_true("abc" == "abc")
    test_true("abc" != "abd")
    test_true("abc" < "abd")
    test_true("ab" < "abc")
    test_true("" < "a")
    test_true("b" > "a" and "b" >= "b" and "a" <= "a")

operators_tests()
