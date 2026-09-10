import std

// Parser error recovery (see Parser::Error in parser.h): with max_errors > 1,
// compile_run_code collects every parse error rather than stopping at the
// first, one per line of the error string (each followed by its "in:"/"at:"
// context lines). These check that the errors reported for a program are
// exactly the ones it has, in source order, with no consequential ones, and
// that the parser survives whatever an error leaves behind.

def error_lines(err:string?) -> [string]:
    let lines = []::string
    guard err: return lines
    for(tokenize(err, "\n", "")) l:
        if l.find_string(": error: ") >= 0: lines.push(l)
    return lines

def expect_errors(code:string, expect:[string], max_errors:int):
    let _, err = compile_run_code(code, [], max_errors)
    let lines = error_lines(err)
    num_checks++
    if lines.length != expect.length:
        fail("expected {expect.length} errors, got {lines.length}:\n{err}\n  for:\n{code}")
        return
    for(expect) e, i:
        if lines[i].find_string(e) < 0:
            fail("error {i + 1} did not contain \"{e}\": {lines[i]}\n  for:\n{code}")

def expect_errors(code:string, expect:[string]):
    expect_errors(code, expect, 100)

def recovery_tests():
    suite("recovery")
    guard not vm_compiled_mode()

    // A program without errors still runs with recovery on.
    let ok, okerr = compile_run_code("return 1 + 2", [], 100)
    test_eq(okerr, nil)
    test_eq(ok, "3")

    // The default is to stop at the first error, and max_errors caps the count.
    expect_errors("print(a)\nprint(b)\nprint(c)\n", [ "unknown identifier `a`" ], 1)
    expect_errors("print(a)\nprint(b)\nprint(c)\n",
                  [ "unknown identifier `a`", "unknown identifier `b`" ], 2)

    // Independent statements each get their error.
    expect_errors("print(a)\nprint(b)\n",
                  [ "unknown identifier `a`", "unknown identifier `b`" ])

    // Also inside nested blocks, which stay intact.
    expect_errors("""
        def f(x):
            if x:
                print(a)
            else:
                print(b)
            return 1
        print(c)
        """, [ "unknown identifier `a`", "unknown identifier `b`", "unknown identifier `c`" ])

    // What is left of a statement after an error is skipped without further errors.
    expect_errors("let x = 1 2\nlet y = z\n",
                  [ "end of line expected, found `2`", "unknown identifier `z`" ])
    // Each unknown identifier is an error of its own.
    expect_errors("let x = a + b + c\nprint(d)\n",
                  [ "unknown identifier `a`", "unknown identifier `b`",
                    "unknown identifier `c`", "unknown identifier `d`" ])

    // An unexpected indented block is skipped whole.
    expect_errors("""
        let x = 1
            let y = 2
        let z = q
        """, [ "end of line expected, found `indentation`", "unknown identifier `q`" ])

    // A trailing operator makes the lexer continue the line, so the next line is
    // consumed as part of the broken statement, but recovery resumes after it.
    expect_errors("let x = 1 +\nlet y = 2\nprint(z)\n",
                  [ "illegal start of expression: `let`", "unknown identifier `z`" ])

    // A missing `:` before a function body: the body is still parsed as such.
    expect_errors("""
        def f(x)
            return x
        print(f(1))
        """, [ "`:` expected, found `indentation`" ])
    expect_errors("def f(x) return x\nprint(f(1))\n", [ "`:` expected, found `return`" ])
    expect_errors("let f = fn(x) x + 1\nprint(f(1))\n", [ "`:` expected, found `x`" ])
    expect_errors("""
        let x = 1
        if x
            print(x)
        print(y)
        """, [ "`:` expected, found `indentation`", "unknown identifier `y`" ])

    // A broken argument list skips to its closing bracket, and the body is parsed.
    expect_errors("def f(a b): return a\nprint(f(1))\n", [ "`)` expected, found `b`" ])
    expect_errors("print(1 2)\nprint(c)\n",
                  [ "`,` expected, found `2`", "unknown identifier `c`" ])
    expect_errors("let v = [ 1 2 ]\nprint(v)\nprint(c)\n",
                  [ "`,` expected, found `2`", "unknown identifier `c`" ])
    expect_errors("let x = (1 2) + 3\nprint(x)\nprint(c)\n",
                  [ "`)` expected, found `2`", "unknown identifier `c`" ])

    // A bracket that is never closed suppresses linefeeds up to the end of the file, so
    // nothing after it can be parsed.
    expect_errors("let x = (1 + 2\nlet y = 3\n",
                  [ "`)` expected, found `let`", "unmatched \'(\' at end of file" ])

    // Declarations with errors in them still get declared.
    expect_errors("""
        class A:
            x:int
            y:Foo
            z:Bar
        def f(a:A): return a.x
        print(f(A { 1, 2, 3 }))
        """, [ "unknown type: Foo", "unknown type: Bar" ])
    expect_errors("""
        class A:
            x:int
            def f() return x
            def g(): return x
        print(A { 1 }.g())
        """, [ "`:` expected, found `return`" ])
    expect_errors("""
        class A<T, >:
            x:T
        print(A<int> { 1 }.x)
        """, [ "`identifier` expected, found `>`" ])
    expect_errors("""
        class A<T>:
            x:T
        def f(a:A): return 1
        print(f(A<int> { 1 }))
        """, [ "use of type `A` requires specializers" ])
    expect_errors("""
        class A<T>:
            x:T
        let a:A<int, float> = A<int> { 1 }
        print(a.x)
        """, [ "too many type specializers" ])
    expect_errors("""
        class A<T, U>:
            x:T
            y:U
        let a:A<int> = A<int, int> { 1, 2 }
        print(a.x)
        """, [ "too few type specializers" ])
    expect_errors("""
        enum E:
            a
            b = "x"
            c
        print(c)
        """, [ "enum value expression must evaluate to an integer constant" ])
    expect_errors("enum E:\nlet x = 1\nprint(x)\n",
                  [ "`indentation` expected, found `linefeed`" ])
    // A clashing declaration is kept apart from what it clashes with.
    expect_errors("""
        enum bool:
            false
            true
        print(true)
        """, [ "double declaration of enum: bool", "double declaration of enum value: false",
               "double declaration of enum value: true" ])
    expect_errors("""
        class A:
            x:int
        class A:
            y:int
        print(A { 1 }.x)
        """, [ "type previously declared as specialization: A" ])
    expect_errors("""
        def f(x):
            switch x:
                case 1 2: return 1
                default: return 2
        print(f(1))
        """, [ "`,` expected, found `2`" ])

    // Errors found by the symbol table.
    expect_errors("let x = 1\nlet x = 2\nprint(x)\n", [ "identifier redefinition: x" ])
    expect_errors("""
        def f(g): return g(1)
        def h(g): return g(2)
        print(f(): _ + h(): _ + 1)
        """, [ "identifier shadowing: _" ])
    expect_errors("""
        class C<T>:
            x:T
            def f(c:int
        print(1)
        """, [ "`)` expected", "unmatched \'(\' at end of file" ])
    expect_errors("member x = 1\nprint(1)\n", [ "member declaration outside of class scope" ])
    expect_errors("def f(): return 1 from g\nprint(f())\n",
                  [ "`g` is not a known function" ])

    // Errors found by the lexer.
    expect_errors("let x = 1)\nprint(z)\n", [ "unmatched \')\'", "unknown identifier `z`" ])
    expect_errors("let x = [ 1, 2)\nprint(z)\n",
                  [ "mismatched \')\', expected \']\'", "unknown identifier `z`" ])
    expect_errors("let x = 1;\nprint(z)\n",
                  [ "\';\' isn\'t used as a statement terminator", "unknown identifier `z`" ])
    expect_errors("let x = \"abc\nprint(z)\n",
                  [ "end of line found in string constant", "unknown identifier `z`" ])
    expect_errors("let x = 1 # 2\nprint(z)\n", [ "illegal token: \'#\'", "unknown identifier `z`" ])
    expect_errors("let x = !1\nprint(z)\n", [ "use \"not\" instead of !", "unknown identifier `z`" ])
    expect_errors("let x = 1 && 2\nprint(z)\n",
                  [ "use \"and\" instead of &&", "unknown identifier `z`" ])
    expect_errors("let x = 1 /* oops\n", [ "end of file in multi-line comment" ])
    expect_errors("def f(x:", [ "unmatched \'(\' at end of file" ])
    expect_errors("import nonexistent_module\nprint(z)\n",
                  [ "can\'t open file: nonexistent_module.lobster", "unknown identifier `z`" ])
    expect_errors("""
        if 1:
            print(1)
          print(2)
        print(z)
        """, [ "inconsistent dedent", "unknown identifier `z`" ])

recovery_tests()
