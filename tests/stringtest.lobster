import testing

run_test("string"):
    let a = 42
    let b = [ 3.14 ]
    // This is not only a test, it also shows exactly how string interpolation
    // gets compiled (i.e. turned into + and string ops by the parser).
    assert "a = {a}!" ==
           "a = " + a + "!"
    assert "a = {a} and b = {b}!" ==
           "a = " + a + " and b = " + b + "!"
    assert "{1+2+a}" ==
           string(1+2+a)
    assert "{a}{b}" ==
           string(a) + b
    assert "{{ {a} }}" ==
           "{{ " + a + " }}"
    assert "\{ {a} \}" ==
           "{{ " + a + " }}"
    assert "{{{1+2+a}}}" ==
           "{{" + (1+2+a) + "}}"
    assert "{ "{ "{a}" }" }" ==
           string(a)

run_test("string constant compare"):
    // A comparison against a string constant is written out as a length test plus a compare of
    // the bytes, see CodeGen::GenStringConstCompare, so it has to agree with the helper on
    // length, on content, and on the bytes that have no character literal of their own.
    // `s` is a re-assigned var on purpose: that keeps it out of constant propagation, so the
    // constant stays on one side only, which is what selects that path.
    var s = "walk"
    assert s == "walk" and not (s != "walk")
    assert s != "wal" and s != "walkk" and s != "wxlk"
    s = "a\'b\\c"
    assert s == "a\'b\\c" and s != "a\'bXc"
    s = "héllo"
    assert s == "héllo" and s != "hello"
    s = "a\nb"
    assert s == "a\nb" and s != "a b"
    s = ""
    assert s == "" and s != "x"
    // Longer than the cap on writing it out, so this one stays a call.
    s = "12345678901234567890"
    assert s == "12345678901234567890" and s != "12345678901234567891"
    // A nilable string keeps the helper that gives nil an answer of its own.
    var n = if s == "": "walk" else: nil
    assert n != "walk"
    n = "walk"
    assert n == "walk"
