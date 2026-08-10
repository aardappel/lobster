import std


// Lexical structure: number/character/string literals, escapes, comments.

def lexical_tests():
    suite("lexical")

    // Integer literals.
    test_eq(123, 100 + 23)
    test_eq(0x10, 16)
    test_eq(0xABADCAFE, 2880293630)
    test_eq(0x7FFFFFFFFFFFFFFF, 9223372036854775807)
    // FIXME: hex literals that don't fit a signed 64-bit int silently lex to 0,
    // because parse_int ignores the overflow error from from_chars. All-bits-set
    // masks like this one should produce -1 (or at least a compile error).
    //test_eq(0xFFFFFFFFFFFFFFFF, -1)
    // Character constants are ints.
    test_eq('A', 65)
    test_eq('0', 48)
    test_eq(' ', 32)
    test_eq('\n', 10)
    test_eq('\t', 9)
    test_eq('\r', 13)
    test_eq('\\', 92)
    test_eq('\'', 39)
    test_eq('\x41', 65)
    // Booleans are just predefined enum values 1 and 0.
    test_eq(true, 1)
    test_eq(false, 0)

    // Float literals: leading/trailing digit forms and exponents
    // (exponents require a "." in the literal).
    test_eq(.5, 0.5)
    test_eq(1.5, 3.0 / 2.0)
    test_eq(1.5e3, 1500.0)
    test_eq(2.5e-2, 0.025)
    test_eq(1.5E2, 150.0)

    // String literal escapes.
    test_eq("\x41\x42", "AB")
    test_eq("\n".length, 1)
    test_eq("\n"[0], 10)
    test_eq("\"", "\x22")
    test_eq("\'"[0], 39)  // ' must always be written escaped in strings.
    test_eq("\\"[0], 92)
    // Unicode, as escape or verbatim.
    test_eq("⺆㋇䬈", "⺆㋇䬈")
    // Braces must be escaped in normal strings (2 ways).
    let ib = 42
    test_eq("\{ {ib} \}", "{{ 42 }}")
    // Triple-quoted strings: verbatim, no escapes, no interpolation.
    test_eq("""a"b""", "a\"b")
    test_eq("""a{ib}b""", "a\{ib\}b")
    test_eq("""line1
line2""", "line1\nline2")

    // Nested comments. /* outer /* inner */ still a comment */
    test_eq(1, /* mid-expression comment */ 1)  // line comment after.

    // A linefeed after a token that indicates an incomplete expression
    // continues the expression.
    let cont = 1 +
        2 +
        3
    test_eq(cont, 6)
    let contv = [
        1,
        2,
    ]
    test_eq(contv.length, 2)
    // A leading . on the next line continues the expression (builder style).
    let built = [ 1, 2 ]
        .map(): _ + 1
    test_eq(built.length, 2)
    test_eq(built[0], 2)

lexical_tests()
