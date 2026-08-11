import std


// Lexical structure: number/character/string literals, escapes, comments.

def lexical_tests():
    suite("lexical")

    // Integer literals.
    test_eq(123, 100 + 23)
    test_eq(0, 0x0)
    test_eq(0x10, 16)
    test_eq(0xabadcafe, 2880293630)  // Hex digits may be lower case.
    test_eq(0xABADCAFE, 2880293630)
    test_eq(0x7FFFFFFFFFFFFFFF, 9223372036854775807)
    test_eq(0xFFFFFFFFFFFFFFFF, -1)
    test_eq('A', 65)
    test_eq('0', 48)
    test_eq(' ', 32)
    test_eq('\n', 10)
    test_eq('\t', 9)
    test_eq('\r', 13)
    test_eq('\\', 92)
    test_eq('\'', 39)
    test_eq('\x41', 65)
    test_eq('\x4a', 74)  // Lower case hex in escapes too.
    // Multi-character constants: big endian packing, up to 8 chars.
    test_eq('AB', ('A' << 8) + 'B')
    test_eq('ABC', ('A' << 16) + ('B' << 8) + 'C')
    test_eq('ABCDEFGH', 0x4142434445464748)
    // A verbatim multi-byte unicode char is its UTF-8 bytes, packed big endian.
    test_eq('⺆', 0xE2BA86)
    // Booleans are just predefined enum values 1 and 0.
    test_eq(true, 1)
    test_eq(false, 0)
    // Identifiers may start with keyword prefixes, contain digits and _.
    let iffy_2 = 1
    let format = 2
    let classy = 3
    let _lead = 4
    test_eq(iffy_2 + format + classy + _lead, 10)

    // Float literals: leading/trailing digit forms and exponents
    // (exponents require a "." in the literal).
    test_eq(.5, 0.5)
    test_eq(1., 1.0)
    test_eq(1.5, 3.0 / 2.0)
    test_eq(1.5e3, 1500.0)
    test_eq(2.5e-2, 0.025)
    test_eq(1.5E2, 150.0)
    test_eq(2.5E-1, 0.25)
    test_eq(2.5e+2, 250.0)
    // "1.e2" is NOT a float literal: a "." followed by an alpha character is
    // parsed as a field/method dereference on the int 1 instead.
    // "1..2" is also not a float: the ".." token makes it a range (in switch).
    let one_in_range = switch 1:
        case 1..2: "in"
        default: "out"
    test_eq(one_in_range, "in")

    // String literal escapes.
    test_eq("\x41\x42", "AB")
    // \u takes exactly 4 hex digits and encodes as UTF-8.
    test_eq("\u2E86", "⺆")
    test_eq("\u0041\u0042", "AB")
    test_eq("\u00E9".length, 2)  // é encodes as 2 UTF-8 bytes.
    // Strings may hold any bytes, including zeros.
    test_eq("a\x00b".length, 3)
    test_eq("a\x00b"[1], 0)
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
    // When starting with a linefeed, the indentation of the first line is
    // stripped from every line.
    test_eq("""
        indented
        lines""", "indented\nlines")

    // Nested comments. /* outer /* inner */ still a comment */
    test_eq(1, /* mid-expression comment */ 1)  // line comment after.

    // A linefeed after a token that indicates an incomplete expression
    // continues the expression.
    let cont = 1 +
        2 +
        3
    test_eq(cont, 6)
    let contl = 1 == 1 and
        2 == 2
    test_eq(contl, true)
    let contv = [
        1,
        2,
    ]
    test_eq(contv.length, 2)
    // Also inside call argument lists.
    test_eq(max(1,
                2), 2)
    // A leading . on the next line continues the expression (builder style).
    let built = [ 1, 2 ]
        .map(): _ + 1
    test_eq(built.length, 2)
    test_eq(built[0], 2)

lexical_tests()
