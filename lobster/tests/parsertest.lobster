// test version of samples/demos/simple_parser_eval.lobster

include testing
include std

run_test("parsereval"):

    class node {}

    class integer : node
        i:int

        def pretty(): return string(i)
        def eval(): return i
        def compile(): return string(i)

    class inst : node
        atom:string
        args:[node]

        def pretty():
            return "(" + (args.fold(atom): _a + " " + pretty(_e)) + ")"

        def eval():
            return switch atom:
                case "+":
                    args.fold(0): _a + _e.eval()
                case "*":
                    args.fold(1): _a * _e.eval()
                default:
                    assert false  // Verified by parser.

        def compile():
            return switch atom:
                case "+", "*":
                    (args.map(): compile(_)).reduce(): "(" + _a + atom + _e + ")"
                default:
                    assert false  // Verified by parser.

    def parse(s:string):
        token := ""
        atom := ""
        i := 0
        line := 1
        def error(err):
            // Note: return from recursive calls of parse_exp in one step!
            // "Look ma, no exception handling"
            return nil, "error: line " + line + ": " + err from parse
        def lex_atom():
            i--
            start := i
            while s[i] > ' ' and not (exists("()\x7F"): _ == s[i]): i++
            if start == i:
                error("unprintable character: " + s[i])
            atom = s.substring(start, i - start)
            ival, all := string_to_int(atom)
            token = if all: "int" else: "atom"
        def lex_next():
            while true:
                switch s[i++]:
                    case 0:
                        i--  // Repeat this token if needed.
                        token = "eof"
                        return
                    case '\n':
                        line++
                    case '\r', '\t', ' ':
                        nil // skip
                    case '(':
                        token = "("
                        return
                    case ')':
                        token = ")"
                        return
                    case '/':
                        if s[i] == '/':  // Skip comment.
                            while s[i] and s[i] != '\n': i++
                        else:
                            lex_atom()
                            return
                    default:
                        lex_atom()
                        return
        lex_next()
        def expect(tok):
            if tok != token:
                error("expected: " + tok + ", found: " + token)
            a := atom
            lex_next()
            return a
        def parse_exp() -> node:
            switch token:
                case "(":
                    lex_next()
                    name := expect("atom")
                    if not (exists([ "+", "*" ]): name == _):
                        error("unknown op: " + name)
                    n := inst { name, []::node }
                    while token != ")":
                        n.args.push(parse_exp())
                    lex_next()
                    return n
                case "int":
                    n := integer { string_to_int(atom) }
                    lex_next()
                    return n
                default:
                    error("cannot parse expression starting with: " + token)
        root := parse_exp()
        expect("eof")
        return root, ""

    test_code :== """
    (+
    (* 2 3 4)  // Comment.
    10)
    """

    ast, err := parse(test_code)
    assert ast
    assert ast.pretty() == "(+ (* 2 3 4) 10)"
    assert ast.eval() == 34
    code := compile(ast)
    assert code == "(((2*3)*4)+10)"

