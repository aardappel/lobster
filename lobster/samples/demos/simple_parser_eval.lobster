// Example mini language implementation for a lisp-y language:
//
// Parse a string into tokens (lex) and then into nodes.
// Then the nodes can be printed, directly evaluated, or compiled to Lobster.

include std

struct node {}

struct integer : node
    i:int

    def pretty(): "" + i
    def eval(): i
    def compile(): "" + i

struct inst : node
    atom:string
    args:[node]

    def pretty():
        (args.fold("(" + atom): _a + " " + pretty(_e)) + ")"

    def eval():
        switch atom:
            case "+": args.fold(0): _a + _e.eval()
            case "*": args.fold(1): _a * _e.eval()
            default: assert 0  // Verified by parser.

    def compile():
        switch atom:
            case "+", "*": (args.map(): compile(_)).reduce(): "(" + _a + atom + _e + ")"
            default: assert 0; ""  // Verified by parser.

def parse(s:string):
    token := ""
    atom := ""
    i := 0
    line := 1
    def error(err):
        return nil, "error: line " + line + ": " + err
    def lex_atom():
        i--
        start := i
        while s[i] > ' ' and not (exists("()\x7F"): _ == s[i]): i++
        if start == i:
            return error("unprintable character: " + s[i]) from parse
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
            return error("expected: " + tok + ", found: " + token) from parse
        a := atom
        lex_next()
        return a
    def parse_exp() -> node:
        switch token:
            case "(":
                lex_next()
                name := expect("atom")
                if not (exists([ "+", "*" ]): name == _):
                    return error("unknown op: " + name) from parse
                n := inst { name, []::node }
                while token != ")":
                    n.args.push(parse_exp())
                lex_next()
                n
            case "int":
                n := integer { string_to_int(atom) }
                lex_next()
                n
            default:
                return error("cannot parse expression starting with: " + token) from parse
    root := parse_exp()
    expect("eof")
    return root, ""

test_code :== """
(+
  (* 2 3 4)  // Comment.
  10)
"""

ast, err := parse(test_code)
if ast:
    print "ast pretty: " + ast.pretty()
    print "ast eval: " + (do(): "" + ast.eval())
    code := compile(ast)
    print "compiled to lobster: " + code
    print "compiled result: " + compile_run_code(code, [])
else:
    print err
