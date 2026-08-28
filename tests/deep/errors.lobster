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
    // The second argument becomes the program's command_line_arguments.
    let argret, argerr = compile_run_code(
        "return concat_string(command_line_arguments(), \"-\")", [ "a", "b" ])
    test_eq(argerr, nil)
    test_eq(argret, "a-b")
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
    test_true(derr and derr.length)

    // Runtime errors. (A sandboxed program must end in a value-producing
    // statement, hence the returns.)
    expect_error("let x = 1\nreturn 1 / (x - x)", "division by zero")
    expect_error("let x = 1\nreturn 1 % (x - x)", "division by zero")
    expect_error("let v = [ 1 ]\nreturn v[2]", "out of range")
    expect_error("let v = [ 1 ]\nreturn v[0 - 1]", "out of range")
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
    // Division by zero inside struct operations.
    expect_error("let z = 0\nreturn string(int2 \{ 1, 1 \} / int2 \{ z, 1 \})", "division by zero")
    // A switch without default on an enum value outside the enum's cases.
    expect_error("enum e:\n    e_a\nlet v = e(9)\nlet r = switch v:\n    case e_a: 1\nreturn r",
                 "out of range of possible values in switch")

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

    // Literals that don't fit in 64 bits are lexer errors.
    expect_error("return 9223372036854775808", "int constant overflow")
    expect_error("return 0x10000000000000000", "hex constant overflow")
    // Exponents require a "." in the literal; 1e5 lexes as 1 then e5.
    expect_error("return 1e5", "found `e5`")
    // Strings are immutable: indexed writes don't compile.
    expect_error("let s = \"abc\"\ns[0] = 65\nreturn s", "lvalue")
    // Named functions are not function values.
    expect_error("def f(x): return x\nlet g = f\nreturn g(1)", "as value")
    // break must be enclosed in a loop.
    expect_error("break\nreturn 1", "must occur inside")
    // A struct cannot contain itself.
    expect_error("struct s:\n    n:s\nreturn 1", "self-referential")
    // Unknown fields, and wrong argument counts.
    expect_error("class c:\n    x = 1\nreturn (c \{\}).y", "does not have field")
    expect_error("def f(a, b): return a\nreturn f(1)", "takes 1 arguments")
    // Class switches must cover all concrete subclasses.
    expect_error("abstract class a:\n    x = 0\nclass b : a\nclass c : a\n" +
                 "def f(v:a): return switch v:\n    case b: 1\nreturn f(b \{\})",
                 "no case applies")
    // Multi-assignment requires enough values.
    expect_error("def f(): return 1\nlet a, b = f()\nreturn a + b", "returns 1 values")
    // Redefinition in the same scope.
    expect_error("let a = 1\nlet a = 2\nreturn a", "redefinition")
    // Abstract classes cannot be instantiated.
    expect_error("abstract class a:\n    x = 0\nreturn (a \{\}).x", "cannot instantiate")
    // Recursive functions must declare their return type.
    expect_error("def f(n): return if n: f(n - 1) else: 0\nreturn f(3)",
                 "explicit return type")
    // Ints don't implicitly convert to enums.
    expect_error("def f(b:bool): return b\nreturn f(2)", "error")
    // A void function call cannot be used as a value.
    expect_error("def f() -> void: return\nreturn \"\{f()\}\"", "requires 1")
    // Lifetime analysis: assigning to a variable while an argument borrows it.
    expect_error("var x = \"hello\"\ndef f(y):\n    x = \"world\"\n    return y\nreturn f(x)",
                 "while borrowed")

    // A `member` variable is scoped to its method, not visible on the object.
    expect_error("class C:\n    def m():\n        member x = 1\nlet c = C \{\}\nreturn c.x",
                 "not in scope")
    // Once a class has a constructor function, plain {} construction elsewhere
    // is disallowed.
    expect_error("class C:\n    a:int\nconstructor C(n:int):\n    return C \{ n \}\n" +
                 "let c = C \{ 5 \}\nreturn c.a", "only be constructed thru its constructor")
    // Calling a non-function value.
    expect_error("let a = 1\nreturn a()", "doesn\'t have a function type")
    // struct types cannot be made nilable.
    expect_error("struct S:\n    s:string\nlet z = nil::S\nreturn 1", "nillable")
    // continue outside a loop.
    expect_error("continue\nreturn 1", "must occur inside")
    // Assigning the wrong type to a typed field.
    expect_error("class C:\n    x:int\nlet c = C \{ 1 \}\nc.x = \"s\"\nreturn c.x", "error")
    // typeid comparison is type checked: comparing typeids of different static
    // types is a type error.
    expect_error("return typeof int == typeof float", "requires type")

    // Declaring a variable or function with the same name as a namespace (or
    // declaring a namespace over an existing declaration) is an error, since
    // "name.x" would lex as a namespaced identifier.
    //expect_error("namespace nsx\ndef f(): return 1\nnamespace\nlet nsx = 1\nreturn nsx",
    //             "clashes with namespace")
    //expect_error("namespace nsx\ndef f(): return 1\nnamespace\ndef nsx(): return 2\nreturn nsx()",
    //             "clashes with namespace")
    //expect_error("def nsx(): return 2\nnamespace nsx\ndef f(): return 1\nnamespace\nreturn 1",
    //             "clashes with existing declaration")

    // Calling a function value outside the scope of its free variables names
    // the variable and where it was defined.
    expect_error("def apply(f): return f()\n" +
                 "def maker():\n    let x = 1\n    return fn: x + 1\n" +
                 "return apply(maker())",
                 "free variable `x` not in scope: it is defined in `maker`")

    // Using the same value twice in a switch is an error, also for enums.
    expect_error("enum e:\n    e_a\n    e_b\nlet v = e_a\n" +
                 "let r = switch v:\n    case e_a: 1\n    case e_a: 2\n    case e_b: 3\nreturn r",
                 "value used twice")
    // A class whose field default constructs its own type would recurse forever.
    expect_error("class C:\n    s = \"s\"\n    c = C \{\}\nreturn (C \{\}).s",
                 "recursively constructs")
    // But nilable self-references and defaults constructing other classes are fine.
    let rret, rerr = compile_run_code(
        "class N:\n    value = 0\n    next:N? = nil\n" +
        "class W:\n    inner = N \{\}\n" +
        "let n = N \{ value: 1, next: N \{ value: 2 \} \}\n" +
        "return n.value + (assert n.next).value + (W \{\}).inner.value", [])
    test_eq(rerr, nil)
    test_eq(rret, "3")

errors_tests()
