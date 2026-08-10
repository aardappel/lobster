import std


// Error conditions: runtime and compile-time errors are tested via
// compile_run_code, which runs a program in its own sandboxed VM and
// returns its error as a value (JIT mode only).

def expect_error(code:string, expect:string):
    let ret, err = compile_run_code(code, [])
    if not err:
        num_failures++
        print "FAIL [{current_suite}]: no error from: {code} (returned: {ret})"
    elif err.find_string(expect) < 0:
        num_failures++
        print "FAIL [{current_suite}]: error for: {code}\n  did not contain \"{expect}\": {err}"
    num_checks++

def errors_tests():
    suite("errors")
    guard not vm_compiled_mode()

    // Sanity: a successful sandboxed run returns its result as a string.
    let ok, okerr = compile_run_code("return 1 + 2", [])
    test_eq(okerr, nil)
    test_eq(ok, "3")
    // Sandboxed programs can return values of any type as their string form.
    let vret, verr = compile_run_code("return [ 1, 2 ]", [])
    test_eq(verr, nil)
    test_eq(vret, "[1, 2]")
    // return .. from program aborts the whole (sandboxed) program.
    let pcode = "def f(c):\n    if c: return \"early\" from program\n    return \"no\"\nf(%)\nreturn \"late\""
    let eret, eerr = compile_run_code(pcode.replace_string("%", "1"), [])
    test_eq(eerr, nil)
    test_eq(eret, "early")
    let lret, lerr = compile_run_code(pcode.replace_string("%", "0"), [])
    test_eq(lerr, nil)
    test_eq(lret, "late")
    // Errors return the string "nil" (not the same as the value nil).
    let dret, derr = compile_run_code("crash()", [])
    test_true(dret == "nil")

    // Runtime errors. (A sandboxed program must end in a value-producing
    // statement, hence the returns.)
    expect_error("let x = 1\nreturn 1 / (x - x)", "division by zero")
    expect_error("let x = 1\nreturn 1 % (x - x)", "division by zero")
    expect_error("let v = [ 1 ]\nreturn v[2]", "out of range")
    expect_error("let v = [ 1 ]\nreturn v[0 - 1]", "out of range")
    // FIXME: (cosmetic) the string index error reports "index 5 out of range 2"
    // for a 1-char string, i.e. the range includes the 0-terminator.
    expect_error("let s = \"a\"\nreturn s[5]", "out of range")
    expect_error("let v = []::int\nreturn v.pop()", "empty vector")
    expect_error("let v = []::int\nreturn v.top()", "empty vector")
    expect_error("let v = [ 1 ]\nreturn v.remove(3)", "out of range")
    expect_error("let v = [ 1 ]\nv.insert(5, 1)\nreturn v.length", "out of range")
    expect_error("return [ 1 ].slice(0, 5)", "extends beyond")
    expect_error("return substring(\"ab\", 0, 5)", "extends beyond")
    expect_error("let a = 0\nreturn assert a == 1", "assert")
    expect_error("return string_to_int(\"1\", 99)", "out of range")
    expect_error("return number_to_string(1, 99, 1)", "out of range")

    // Compile-time errors also come back as the error return value.
    expect_error("let x:string = 1\nreturn x", "error")
    expect_error("return undefined_variable_xyz", "unknown identifier")
    expect_error("let x = 1\nx = 2\nreturn x", "constant")
    expect_error("var x = 1\nx = \"s\"\nreturn x", "error")
    expect_error("return 1 +", "error")
    // Flow-sensitivity: dereferencing a possibly-nil value.
    expect_error("def f(s:string?) -> int: return s.length\nreturn f(nil)", "error")
    // Exhaustive switch on enums enforced at compile time.
    expect_error("enum e:\n    e_a\n    e_b\nlet v = e_a\nlet r = switch v:\n    case e_a: 1\nreturn r", "not tested in switch")
    // A program whose last statement produces no value is also an error.
    expect_error("print(1)", "returns 0 values")

errors_tests()
