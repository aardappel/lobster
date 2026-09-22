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
    // Positive zero has no bits set; negative zero's sign bit makes it true.
    expect_error("return assert 0.0", "assertion failed")
    expect_error("var x = 0.0\nx = 0.0\nreturn assert x", "assertion failed")
    let zeroret, zeroerr = compile_run_code(
        "assert -0.0\nvar x = -0.0\nx = -0.0\nassert x\nreturn 7", [])
    test_eq(zeroerr, nil)
    test_eq(zeroret, "7")
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
    // Inlining may discard unused argument bindings, but their assertions must run.
    expect_error("def ignore(x:int): return 7\nvar x = 0\nx = 0\nreturn ignore(assert x)",
                 "assertion failed")
    expect_error("def ignore(x:string): return 7\nvar s:string? = nil\ns = nil\n" +
                 "return ignore(assert s)", "assertion failed")
    expect_error("def ignore(x:int): return 7\nvar x = 0\nx = 0\nignore(1)\n" +
                 "return ignore(assert x)", "assertion failed")
    let assertret, asserterr = compile_run_code(
        "def ignore(x:int): return 7\nvar x = 1\nx = 1\nreturn ignore(assert x)", [])
    test_eq(asserterr, nil)
    test_eq(assertret, "7")
    // A failed assert reports its condition as written: operators with only the parentheses
    // their precedence needs, calls, fields, constructors and constants as in the source,
    // whatever the typechecker and optimizer (inlining, folding) later make of the tree.
    expect_error("let v = [ 1, 2 ]\nreturn assert v.length == 3 and not (v[0] < 2)",
                 "assertion failed: v.length == 3 and not v[0] < 2")
    expect_error("let n = 3\nreturn assert n - (n - 1) == 5 or -(n + 1) == 4",
                 "assertion failed: n - (n - 1) == 5 or -(n + 1) == 4")
    expect_error("let n = 3\nreturn assert n == 1 and (n == 2 or n == 3)",
                 "assertion failed: n == 1 and (n == 2 or n == 3)")
    expect_error("struct P:\n    x:int\n    y:int\nlet p = P \{ 1, 2 \}\n" +
                 "return assert P \{ 3, 4 \} == p and p.x + p.y == 5",
                 "assertion failed: P \{ 3, 4 \} == p and p.x + p.y == 5")
    expect_error("def sq(x:int): return x * x\nlet n = 3\nlet s = \"ab\"\n" +
                 "return assert sq(n) == 8 or s.substring(0, 1) == \"b\"",
                 "assertion failed: sq(n) == 8 or s.substring(0, 1) == \"b\"")
    expect_error("def approx(a:float, b:float): return abs(a - b) < 0.001\nlet f = 1.5\n" +
                 "return assert approx(f, 2.0)", "assertion failed: approx(f, 2.0)")
    expect_error("enum E:\n    A\n    B\nlet e = A\nvar ns:string? = nil\nns = nil\n" +
                 "return assert ns is string or e == B",
                 "assertion failed: ns is string or e == B")
    // Division and indexing checks are observable for unused arguments too.
    expect_error("def ignore(x:int): return 7\nvar z = 0\nz = 0\nreturn ignore(1 / z)",
                 "division by zero")
    expect_error("def ignore(x:int): return 7\nvar z = 0\nz = 0\nreturn ignore(1 % z)",
                 "division by zero")
    expect_error("def ignore(x:int): return 7\nlet v = [1]\nvar i = 3\ni = 3\n" +
                 "return ignore(v[i])", "out of range")
    expect_error("def ignore(x:int): return 7\nlet s = \"a\"\nvar i = 3\ni = 3\n" +
                 "return ignore(s[i])", "out of range")
    expect_error("return string_to_int(\"1\", 99)", "out of range")
    expect_error("return number_to_string(1, 99, 1)", "out of range")
    // Division by zero inside struct operations.
    expect_error("let z = 0\nreturn string(int2 \{ 1, 1 \} / int2 \{ z, 1 \})", "division by zero")
    // A switch without default on an enum value outside the enum's cases.
    expect_error("enum e:\n    e_a\nlet v = e(9)\nlet r = switch v:\n    case e_a: 1\nreturn r",
                 "out of range of possible values in switch")
    // An `out_of_range` case takes the place of that error and handles the value instead.
    let oret, oerr = compile_run_code(
        "enum e:\n    e_a\nlet v = e(9)\nreturn switch v:\n    case e_a: \"a\"\n" +
        "    out_of_range: \"o\{v\}\"\n", [])
    test_eq(oerr, nil)
    test_eq(oret, "o9")

    // Compile-time errors also come back as the error return value.
    expect_error("let x:string = 1\nreturn x", "error")
    expect_error("return undefined_variable_xyz", "unknown identifier")
    expect_error("let x = 1\nx = 2\nreturn x", "constant")
    expect_error("var x = 1\nx = \"s\"\nreturn x", "error")
    expect_error("return 1 +", "error")
    // A promotion only survives a branching statement if every path that can
    // reach the code after it establishes it.
    expect_error("class N:\n    v = 1\nvar a:N? = nil\n" +
                 "if rnd(2):\n    a = N \{\}\nreturn a.v", "dereferencing nillable")
    expect_error("class N:\n    v = 1\nvar a:N? = nil\n" +
                 "switch rnd(3):\n    case 1: a = N \{\}\n    default: pass()\nreturn a.v",
                 "dereferencing nillable")
    // A non-exhaustive switch is one such path, even if every case promotes.
    expect_error("class N:\n    v = 1\nvar a:N? = nil\n" +
                 "switch rnd(3):\n    case 1: a = N \{\}\n    case 2: a = N \{\}\nreturn a.v",
                 "dereferencing nillable")
    // Nor does one established inside a loop, which may run zero times.
    expect_error("class N:\n    v = 1\nvar a:N? = nil\n" +
                 "while rnd(2):\n    if not a: a = N \{\}\nreturn a.v",
                 "dereferencing nillable")
    // And an assignment in a loop body demotes for the code around it.
    expect_error("class N:\n    v = 1\nvar a:N? = N \{\}\n" +
                 "if a:\n    while rnd(2):\n        a = nil\n    return a.v\nreturn 0",
                 "dereferencing nillable")
    // A call to a specialization typechecked for an earlier call replays its writes
    // against this call's promotions.
    expect_error("class N:\n    v = 1\nvar a:N? = N \{\}\ndef f(): a = nil\n" +
                 "if a: f()\na = N \{\}\nif a:\n    f()\n    return a.v\nreturn 0",
                 "dereferencing nillable")
    // A write thru a variable that was assigned the object drops the promotion of the
    // object's field (see tests/errors/flow_var_alias.lobster), also when the variable may
    // hold something else by then, and when it got the object from a multiple assignment.
    expect_error("class N:\n    v:N?\nlet x = N \{ N \{ nil \} \}\nvar y = x\n" +
                 "y = N \{ nil \}\nif rnd(2): y = x\nif x.v:\n    y.v = nil\n" +
                 "    return x.v.v\nreturn nil", "dereferencing nillable")
    expect_error("class N:\n    v:N?\nlet x = N \{ N \{ nil \} \}\nvar y = N \{ nil \}\n" +
                 "var z = x\ny, z = z, y\nif x.v:\n    y.v = nil\n" +
                 "    return x.v.v\nreturn nil", "dereferencing nillable")
    // An error that the promotion would have avoided says which write dropped it, and how that
    // reached the promoted path.
    expect_error("class N:\n    r:N? = nil\n    a = 1\nvar y = N \{\}\nlet x = y\nx.r = x\n" +
                 "y = N \{\}\nreturn x.r.a",
                 "\n  `x.r` is no longer known to be non-nil after the assignment to `y` at " +
                 "string(7) (`x` may be an alias of `y`)")
    expect_error("class N:\n    v:N?\nlet x = N \{ N \{ nil \} \}\nvar y = x\n" +
                 "y = N \{ nil \}\nif rnd(2): y = x\nif x.v:\n    y.v = nil\n" +
                 "    return x.v.v\nreturn nil",
                 "`x.v` is no longer known to be non-nil after the assignment to `y.v` at " +
                 "string(8) (`y` may be an alias of `x`)")
    expect_error("class N:\n    r:N? = nil\n    a = 1\nvar g = N \{\}\ndef f():\n" +
                 "    if rnd(2): g = N \{\}\ng.r = N \{\}\nf()\nreturn g.r.a",
                 "`g.r` is no longer known to be non-nil after the assignment to `g` at " +
                 "string(6), in the call to `f` at string(8)")
    expect_error("class N:\n    v = 1\nvar a:N? = N \{\}\ndef f(): a = nil\n" +
                 "if a: f()\na = N \{\}\nif a:\n    f()\n    return a.v\nreturn 0",
                 "`a` is no longer known to be non-nil after the assignment to `a` at " +
                 "string(4), in the call to `f` at string(8)")
    expect_error("class N:\n    v = 1\nvar a:N? = N \{\}\nif a:\n    while rnd(2):\n" +
                 "        if rnd(2):\n            a = nil\n            break\n" +
                 "    return a.v\nreturn 0",
                 "`a` is no longer known to be non-nil after the assignment to `a` at string(7)")
    expect_error("class N:\n    r:N? = nil\ndef f(n:N): return n\nvar y = N \{\}\nlet x = y\n" +
                 "x.r = x\ny = N \{\}\nreturn f(x.r)",
                 "`f` (1st argument) requires type `N`, got `N?`\n  `x.r` is no longer known")
    expect_error("class A:\n    x = 1\nclass B : A\n    y = 2\nvar v:A = B \{\}\nif v is B:\n" +
                 "    v = A \{\}\n    return v.y\nreturn 0",
                 "`v` is no longer known to be `B` after the assignment to `v` at string(7)")
    let _, nerr = compile_run_code("class N:\n    v = 1\nvar a:N? = nil\n" +
                                   "if rnd(2):\n    a = N \{\}\nreturn a.v", [])
    test_true(nerr and nerr.find_string("no longer known") < 0)

    // Flow-sensitivity: dereferencing a possibly-nil value.
    expect_error("def f(s:string?) -> int: return s.length\nreturn f(nil)", "error")
    // Exhaustive switch on enums enforced at compile time.
    expect_error("enum e:\n    e_a\n    e_b\nlet v = e_a\nlet r = switch v:\n    case e_a: 1\nreturn r", "not tested in switch")
    // `out_of_range` handles values the enum doesn't declare, so unlike `default` it does not
    // make the switch exhaustive: a missing case is still an error.
    expect_error("enum e:\n    e_a\n    e_b\nlet v = e_a\nlet r = switch v:\n    case e_a: 1\n" +
                 "    out_of_range: 2\nreturn r", "not tested in switch")
    // `default` already runs for out of range values, so the two can't be combined.
    expect_error("enum e:\n    e_a\nlet v = e_a\nlet r = switch v:\n    case e_a: 1\n" +
                 "    default: 2\n    out_of_range: 3\nreturn r",
                 "cannot have both `default` and `out_of_range`")
    expect_error("enum e:\n    e_a\nlet v = e_a\nlet r = switch v:\n    case e_a: 1\n" +
                 "    out_of_range: 2\n    default: 3\nreturn r",
                 "cannot have both `default` and `out_of_range`")
    expect_error("enum e:\n    e_a\nlet v = e_a\nlet r = switch v:\n    case e_a: 1\n" +
                 "    out_of_range: 2\n    out_of_range: 3\nreturn r",
                 "cannot have more than one `out_of_range`")
    // Only an enum has values outside of the ones it declares.
    expect_error("let v = 1\nlet r = switch v:\n    case 1: 1\n    out_of_range: 2\nreturn r",
                 "`out_of_range` can only be used in a switch on an enum value")
    expect_error("let v = \"a\"\nlet r = switch v:\n    case \"a\": 1\n    out_of_range: 2\nreturn r",
                 "`out_of_range` can only be used in a switch on an enum value")
    expect_error("class a:\n    x:int = 1\nclass b : a\nlet v = b \{\}\nlet r = switch v:\n" +
                 "    case b: 1\n    out_of_range: 2\nreturn r",
                 "`out_of_range` can only be used in a switch on an enum value")
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
    expect_error("def f(a, b): return a\nreturn f(1)", "takes 1 argument")
    // The error lists where the versions that do exist were declared, and, for
    // a call inside a method, the receiver type an implicit first arg was
    // looked for with.
    expect_error("def f(a, b): return a\nreturn f(1)", "declared: f(a, b)")
    expect_error("class A:\n    x = 1\nclass B:\n    y = 2\n    def m(): return h()\n" +
                 "def h(a:A): return a.x\nreturn (B \{\}).m()",
                 "or 1 with `B` as implicit first argument")
    // A function that a call could never select over a builtin of that name.
    expect_error("def length(a): return 1\nreturn length([ 1 ])",
                 "would always select the built-in")
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
    // Nor can an expression that never completes define or assign anything: a call to a
    // function value whose only return is from the enclosing function, or a return itself.
    expect_error("def g():\n    let f = fn(): return 1\n    let r = f()\n    return 2\n" +
                 "return g()", "initializer of `r` never completes")
    expect_error("def g():\n    var r = 0\n    r = if true: return 1 else: return 2\n" +
                 "    return r\nreturn g()", "assigned expression never completes")
    // Lifetime analysis: assigning to a variable while an argument borrows it makes the
    // caller keep the value alive, so the argument keeps what it was passed.
    let bret, berr = compile_run_code(
        "var x = \"hello\"\ndef f(y):\n    x = \"world\"\n    return y\nreturn f(x)", [])
    test_eq(berr, nil)
    test_eq(bret, "hello")
    // The same for a later argument of the same call (see lifetime.lobster), but a write
    // while an operand of the same expression borrows the location is an error.
    expect_error("let v = [ \"a\" + \"\" ]\nreturn v[0] + v.pop()", "borrows it")

    // A `member` initializer is the field's default, so it runs wherever the
    // class is constructed and cannot read the object or the method's locals.
    expect_error("class C:\n    x = 1\n    def m():\n        member y = x\n        return y\n" +
                 "return C \{\}.m()",
                 "field `x` cannot be used in a `member` initializer")
    expect_error("class C:\n    def m():\n        let n = 5\n        member y = n\n" +
                 "        return y\nreturn C \{\}.m()",
                 "local variable `n` cannot be used in a `member` initializer")

    // A `member` variable is scoped to the method that declares it: not
    // visible on the object, nor from another method of the same class.
    expect_error("class C:\n    def m():\n        member x = 1\nlet c = C \{\}\nreturn c.x",
                 "can only be used there")
    expect_error("class C:\n    def m():\n        member x = 1\n        return x\n" +
                 "    def other():\n        return x\nreturn C \{\}.other()",
                 "declared by a `member` in `m`")
    // Once a class has a constructor function, plain {} construction elsewhere
    // is disallowed.
    expect_error("class C:\n    a:int\nconstructor C(n:int):\n    return C \{ n \}\n" +
                 "let c = C \{ 5 \}\nreturn c.a", "only be constructed thru its constructor")
    // Calling a non-function value.
    expect_error("let a = 1\nreturn a()", "doesn\'t have a function type")
    // struct types cannot be made nilable.
    expect_error("struct S:\n    s:string\nlet z = nil::S\nreturn 1", "nillable")
    // A struct with more fields than the one wanted is never shortened for you,
    // not for a function and not to pick an overload of a builtin.
    expect_error("import vec\ndef f(v:int2): return v.x\nreturn f(int3 \{ 1, 2, 3 \})",
                 "requires type `int2`, got `int3`")
    expect_error("import vec\nlet a = float4 \{ 1.0, 2.0, 3.0, 4.0 \}\n" +
                 "return string(cardinal_spline(a, a, a, a, 0.5, 0.5))",
                 "requires type `numeric struct (all float) of 2 to 3 slots`, got `float4`")
    // A builtin taking a numeric struct takes 1 to 4 slots of one scalar type, so a struct
    // of mixed types, or of more slots than that, is told which kinds of struct there are
    // overloads for rather than being reported against the one taking any reference.
    expect_error("struct S:\n    a:int\n    b:float\nreturn hash(S \{ 1, 2.0 \})",
                 "`S` does not have all int or all float fields")
    expect_error("import vec\nstruct S:\n    a:int4\n    b:int4\nreturn hash(S \{ int4_1, int4_1 \})",
                 "`S` has 8 slots, not 2: a struct field counts for as many slots as it has")
    // The slots a struct takes are what that counts, and a struct field is as many slots as
    // it has, so a pair of them is not a struct of two.
    expect_error("import vec\nstruct S:\n    a:float2\n    b:float2\nreturn string(atan2(S \{ float2_1, float2_1 \}))",
                 "requires type `float2`, got `S`")
    // An overloaded builtin with no numeric struct overload at all takes no struct there.
    expect_error("import vec\nreturn binary_search([ 1 ], int2 \{ 1, 2 \})",
                 "struct value cannot be used in `binary_search`")
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

    // An import of a file that was already imported elsewhere includes
    // nothing, and must also not end the namespace it sits in.
    expect_error("import std\nnamespace nsx\nimport std\n" +
                 "def foo(): return 1\nnamespace\nreturn foo()",
                 "unknown field/function reference")

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
