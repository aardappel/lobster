

// Strings: indexing, iteration, interpolation, and the string builtins.

def strings_tests():
    suite("strings")

    // Strings are byte vectors: length/indexing operate on bytes.
    test_eq("hello".length, 5)
    test_eq("".length, 0)
    test_eq("abc"[0], 'a')
    test_eq("abc"[2], 'c')
    test_eq("⺆".length, 3)  // UTF-8 encoded.
    var bytesum = 0
    for("abc") c, i:
        bytesum += c + i
    test_eq(bytesum, 'a' + 'b' + 1 + 'c' + 2)

    // Interpolation, and its equivalence to concatenation.
    let ia = 42
    let fv = [ 1.5 ]
    test_eq("a = {ia}!", "a = " + ia + "!")
    test_eq("{ia}{fv}", string(ia) + fv)
    test_eq("{1 + 2 + ia}", string(1 + 2 + ia))
    test_eq("{ "{ "{ia}" }" }", "42")  // Nested interpolation.
    test_eq("x{ if ia > 0: "pos" else: "neg" }x", "xposx")

    // string() of misc values.
    test_eq(string(123), "123")
    test_eq(string(-1.5), "-1.5")
    test_eq(string("abc"), "abc")  // Identity on strings.
    test_eq(string([ 1, 2 ]), "[1, 2]")
    test_eq(string(1.0), "1.0")

    // substring.
    test_eq(substring("abcdef", 1, 3), "bcd")
    test_eq(substring("abcdef", 0, -1), "abcdef")
    test_eq(substring("abcdef", 4, -1), "ef")     // Negative size: rest.
    test_eq(substring("abcdef", 6, -1), "")
    test_eq(substring("abcdef", 2, 0), "")

    // find_string / find_string_reverse.
    test_eq(find_string("abcdabcd", "bc"), 1)
    test_eq(find_string("abcdabcd", "bc", 4), 5)
    test_eq(find_string("abcdabcd", "BC"), -1)
    test_eq(find_string("abc", "abcd"), -1)
    test_eq(find_string("abc", ""), 0)
    test_eq(find_string("abc", "a", 5), -1)  // Offset past the end.
    test_eq(find_string_reverse("abcdabcd", "bc"), 5)
    test_eq(find_string_reverse("abcdabcd", "bc", 4), 1)
    test_eq(find_string_reverse("abc", "x"), -1)

    // replace_string, with and without count.
    test_eq(replace_string("abcdabcd", "bc", "X"), "aXdaXd")
    test_eq(replace_string("abcdabcd", "bc", "X", 1), "aXdabcd")
    test_eq(replace_string("aaa", "a", "aa"), "aaaaaa")
    test_eq(replace_string("abc", "", "X"), "abc")  // Empty pattern: unchanged.
    test_eq(replace_string("abc", "abc", ""), "")

    // split_string / split_string_reverse.
    var sl, sr = split_string("foo->bar->baz", "->")
    test_eq(sl, "foo")
    test_eq(sr, "bar->baz")
    sl, sr = split_string("foobar", "->")
    test_eq(sl, "foobar")
    test_eq(sr, "")
    sl, sr = split_string_reverse("foo->bar->baz", "->")
    test_eq(sl, "foo->bar")
    test_eq(sr, "baz")
    sl, sr = split_string_reverse("foobar", "->")
    test_eq(sl, "")
    test_eq(sr, "foobar")

    // tokenize: delimiter separated, whitespace stripped, optionally dividing.
    test_seq(tokenize(" A | B C |", "|", " ", true), [ "A", "B C", "" ])
    test_seq(tokenize("; A ; B C;; ", ";", " "), [ "", "A", "B C", "" ])
    test_seq(tokenize(", A , B C,, ", ",", " ", true), [ "", "A", "B C", "", "" ])
    test_seq(tokenize("", ",", " "), []::string)

    // case conversion, ASCII only, unicode untouched.
    test_eq(lowercase("Hello Wörld 123"), "hello wörld 123")
    test_eq(uppercase("Hello wörld 123"), "HELLO WöRLD 123")

    // escape_string / concat_string / repeat_string.
    test_eq(escape_string("a*b&c", "*&", "<", ">"), "a<*>b<&>c")
    test_eq(escape_string("abc", "x", "<", ">"), "abc")
    test_eq(concat_string([], ","), "")
    test_eq(concat_string([ "a" ], ","), "a")
    test_eq(concat_string([ "a", "b", "c" ], ", "), "a, b, c")
    test_eq(concat_string([ "a", "b" ], ""), "ab")
    test_eq(repeat_string("ab", 3), "ababab")
    test_eq(repeat_string("ab", 0), "")
    test_eq(repeat_string("ab", -1), "")
    test_eq(repeat_string("", 1000), "")

    // string_to_int: optional base, second value is "all chars parsed".
    var sn, sok = string_to_int("123")
    test_eq(sn, 123)
    test_eq(sok, true)
    sn, sok = string_to_int("-42")
    test_eq(sn, -42)
    test_eq(sok, true)
    sn, sok = string_to_int("12ab")
    test_eq(sn, 12)
    test_eq(sok, false)
    sn, sok = string_to_int("ab")
    test_eq(sn, 0)
    test_eq(sok, false)
    sn, sok = string_to_int("")
    test_eq(sn, 0)
    test_eq(sok, true)  // Vacuously all parsed.
    sn, sok = string_to_int("FF", 16)
    test_eq(sn, 255)
    test_eq(sok, true)
    sn, sok = string_to_int("101", 2)
    test_eq(sn, 5)
    test_eq(sok, true)
    sn, sok = string_to_int("zz", 36)
    test_eq(sn, 35 * 36 + 35)
    test_eq(sok, true)

    // string_to_float.
    var fnum, fok = string_to_float("2.5")
    test_eq(fnum, 2.5)
    test_eq(fok, true)
    fnum, fok = string_to_float("2.5e2")
    test_eq(fnum, 250.0)
    test_eq(fok, true)
    fnum, fok = string_to_float("-.5")
    test_eq(fnum, -0.5)
    test_eq(fok, true)
    fnum, fok = string_to_float("3.x")
    test_eq(fnum, 3.0)
    test_eq(fok, false)

    // number_to_string: unsigned interpretation, base and minimum width.
    test_eq(number_to_string(255, 16, 2), "FF")
    test_eq(number_to_string(255, 16, 4), "00FF")
    test_eq(number_to_string(5, 2, 1), "101")
    test_eq(number_to_string(0, 10, 3), "000")
    test_eq(number_to_string(-1, 16, 1), "FFFFFFFFFFFFFFFF")

    // unicode conversions, round trip and error case.
    let unicodes = [ 0x30E6, 0x2E86, 'A', 0x10348 ]
    test_seq(string_to_unicode(unicode_to_string(unicodes)), unicodes)
    test_eq(unicode_to_string([ 'A', 'B' ]), "AB")
    let uv, uok = string_to_unicode("A\xFF")  // Invalid UTF-8 after 'A'.
    test_eq(uok, false)
    test_seq(uv, [ 'A' ])

strings_tests()
