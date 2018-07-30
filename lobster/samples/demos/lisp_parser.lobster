include "std.lobster"

struct node:
    atom:string
    args:[node]

    def pretty():
        if args.length:
            "(" + atom + (args.fold(""): _a + " " + pretty(_e)) + ")"
        else:
            atom

def parse(s:string):
    token := ""
    atom := ""
    i := 0
    line := 1
    def error(err):
        return nil, "line " + line + ": " + err
    def lex_atom():
        i--
        start := i
        while s[i] > ' ' and not (exists("()\x7F"): _ == s[i]): i++
        if start == i:
            return error("unprintable character: " + s[i]) from parse
        atom = s.substring(start, i - start)
        token = "atom"
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
        a
    def parse_exp():
        switch token:
            case "(":
                lex_next()
                n := node { expect("atom"), [] }
                while token != ")":
                    n.args.push(parse_exp())
                lex_next()
                n
            case "atom":
                n := node { atom, [] }
                lex_next()
                n
            default:
                return error("cannot parse expression starting with: " + token) from parse
    root := parse_exp()
    expect("eof")
    return root, ""

test_code :== """
(+
  (* 123 456)  // Comment.
  789)
"""

ast, err := parse(test_code)
print if ast: ast.pretty() else: err
