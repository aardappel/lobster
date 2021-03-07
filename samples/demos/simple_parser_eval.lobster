// Example mini language implementation for a lisp-y language:
//
// Parse a string into tokens (lex) and then into nodes.
// Then the nodes can be printed, directly evaluated, or compiled to Lobster.

import std

class node

class integer : node
    i:int

    def pretty() -> string: return "{i}"
    def eval() -> int: return i
    def compile() -> string: return "{i}"

class inst : node
    atom:string
    args:[node]

    def pretty() -> string:
        let elems = args.fold(atom): "{_a} {_e.pretty()}"
        return "({elems})"

    def eval() -> int:
        return switch atom:
            case "+":
                args.fold(0): _a + _e.eval()
            case "*":
                args.fold(1): _a * _e.eval()
            default:
                assert false  // Verified by parser.

    def compile() -> string:
        return switch atom:
            case "+", "*":
                (args.map(): _.compile()).reduce(): "({_a}{atom}{_e})"
            default:
                assert false  // Verified by parser.

def parse(s:string):
    var token = ""
    var atom = ""
    var i = 0
    var line = 1
    var ival = 0
    def error(err):
        // Note: return from recursive calls of parse_exp in one step!
        // "Look ma, no exception handling"
        return nil, "error: line {line}: {err}" from parse
    def lex_atom():
        i--
        let start = i
        while s[i] > ' ' and not (exists("()\x7F"): _ == s[i]): i++
        if start == i:
            error("unprintable character: {s[i]}")
        atom = s.substring(start, i - start)
        var all = false
        ival, all = string_to_int(atom)
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
            error("expected: {tok}, found: {token}")
        let a = atom
        lex_next()
        return a
    def parse_exp() -> node:
        switch token:
            case "(":
                lex_next()
                let name = expect("atom")
                if not (exists([ "+", "*" ]): name == _):
                    error("unknown op: {name}")
                let n = inst { name, []::node }
                while token != ")":
                    n.args.push(parse_exp())
                lex_next()
                return n
            case "int":
                let n = integer { ival }
                lex_next()
                return n
            default:
                error("cannot parse expression starting with: {token}")
    let root = parse_exp()
    expect("eof")
    return root, ""

let test_code = """
(+
  (* 2 3 4)  // Comment.
  10)
"""

let ast, err = parse(test_code)
if ast:
    print "ast pretty: {ast.pretty()}"
    print "ast eval: {ast.eval()}"
    let code = compile(ast)
    print "compiled to lobster: {code}"
    print "compiled result: {compile_run_code(code, [])}"
else:
    print err
