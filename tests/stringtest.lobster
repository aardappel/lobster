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
    assert "{{ {a} }}" ==
           "{{ " + a + " }}"
    assert "{{{1+2+a}}}" ==
           "{{" + (1+2+a) + "}}"
    assert "{a}{b}" ==
           string(a) + b
