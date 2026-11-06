// Tests for the formatter:
//
//     lobster tools/lobsterfmt/test.lobster -- [-seed N] [-rounds N] [-out DIR] path ...
//
// - goldens: inputs, and the output each rule of the layout gives them;
// - for each .lobster file under the paths: it formats, the result follows the layout's rules
//   (checked here apart from how the formatter applies them) and formats to itself; with its
//   whitespace scrambled (but not what it means) it formats the same; broken (lines and
//   characters deleted, duplicated, joined, split, spliced with another file) it's refused or
//   formats to something that formats to itself.
// Each result is checked twice: by format_lobster itself, then by the compiler
// (compiler_check). What the compiler catches that format_lobster let through is counted: it's
// what formatting without the compiler's check would get wrong.
// With -out (an absolute path), the input of each failure is written to DIR/failure_N.lobster.

import std
import .compilercheck
import .files

class failure:
    what:string
    input:string

let failures = []::failure

def failed(what:string, input:string): failures.push(failure { what, input })

// Each rule of the layout.
let goldens = [
    // Spaces around binary operators, after commas, none inside ( ) or in xs[i].
    [ "let x=a+b*c\nf( x ,y )\nlet y = xs[ 1 ]\n", "let x = a + b * c\nf(x, y)\nlet y = xs[1]\n" ],
    // Inside [ ] of a vector, { } of a constructor.
    [ "let v=[1,2]\nlet e=[]\nlet p=P{{1,2}}\nlet q=P{{}}\n",
      "let v = [ 1, 2 ]\nlet e = []\nlet p = P {{ 1, 2 }}\nlet q = P {{}}\n" ],
    // Types: a:int, [int], ->, ::, int?, generics.
    [ "def f( a : int, b:[ int ], c :: T )->int?:\n    a\n",
      "def f(a:int, b:[int], c::T) -> int?:\n    a\n" ],
    [ "let d = dictionary<string,int>()\nvar t = []::resource<texture>\n",
      "let d = dictionary<string, int>()\nvar t = []::resource<texture>\n" ],
    // Fields, and inheritance.
    [ "class A : B\n  x : int = 1\n  y, z:float\n",
      "class A : B\n    x:int = 1\n    y, z:float\n" ],
    // Blocks: 4 spaces, a space after the : of a block, none before.
    [ "if a :b\nelse :\n\tc\n", "if a: b\nelse:\n    c\n" ],
    // Unary operators, not, and keywords before (.
    [ "a = - 1\nb = not  a\nc = ~ a\nfor (xs) x: x\nassert(a)\n",
      "a = -1\nb = not a\nc = ~a\nfor(xs) x: x\nassert (a)\n" ],
    // Where the parser looks at whitespace, it stays: f(x) and f (x), f -1, a<b, gl .f.
    [ "f (x)\nf(x)\nf -1\nwhile a<b: a++\n", "f (x)\nf(x)\nf -1\nwhile a<b: a++\n" ],
    [ "a  .b\nc.  d\ne.f\n", "a .b\nc. d\ne.f\n" ],
    // Imports.
    [ "import  std\nimport .lib\nimport from \"x/\"\n",
      "import std\nimport .lib\nimport from \"x/\"\n" ],
    // Continuation lines: once more, or twice when a block follows.
    [ "let s = a +\nb\nif a and\n   b:\n    c\n",
      "let s = a +\n    b\nif a and\n        b:\n    c\n" ],
    // A comment between continuation lines, indented as they are.
    [ "if a and\n// c\n   b:\n    c\n", "if a and\n        // c\n        b:\n    c\n" ],
    // A block that starts in a continuation line.
    [ "let w =\n  if a:\n    1\n  else:\n    2\n",
      "let w =\n    if a:\n        1\n    else:\n        2\n" ],
    // Brackets: lined up after the bracket, or once more when it ends the line.
    [ "f(a,\nb)\nlet xs = [\n1,\n2\n]\n", "f(a,\n  b)\nlet xs = [\n    1,\n    2\n]\n" ],
    // Comments: two spaces before one at the end of a line, lined up on consecutive lines.
    [ "x = 1 // one\nyy = 2   // two\n", "x = 1   // one\nyy = 2  // two\n" ],
    // A comment on its own line goes with the code after it.
    [ "def f():\n        // c\n    g()\n", "def f():\n    // c\n    g()\n" ],
    // Blank lines: at most one, none at the start of a block or the file, none at the end.
    [ "\n\na\n\n\n\nb\nif c:\n\n    d\n\n\n", "a\n\nb\nif c:\n    d\n" ],
    // Strings, multi-line ones and interpolations are left alone.
    [ "s =  \"a  {{b+c}}  \"\nt = \"\"\"\n  x  \n\"\"\"\n",
      "s = \"a  {{b+c}}  \"\nt = \"\"\"\n  x  \n\"\"\"\n" ],
    // CRLF becomes LF; trailing whitespace goes.
    [ "a = 1   \r\nb = 2\r\n", "a = 1\nb = 2\n" ],
    [ "", "" ],
    [ "\n  \n", "" ],
    // Operators defined, and comments in a line.
    [ "def operator+= (o:T): a += o.a\ndef operator[](i:int): i\n",
      "def operator+=(o:T): a += o.a\ndef operator[](i:int): i\n" ],
    [ "a=b/* c */+d\n", "a = b /* c */ + d\n" ],
    // A function's name and its ( (where the space doesn't matter), vectors in type arguments.
    [ "def f (a): a\nlet d = dictionary<string,[ int ]>()\n",
      "def f(a): a\nlet d = dictionary<string, [int]>()\n" ],
    [ "def f() -> int, [ string ]: g()\n", "def f() -> int, [string]: g()\n" ],
    // A keyword isn't a type argument, and g on the line before (5) is called with it.
    [ "if a<b and c > (d): e\n", "if a<b and c > (d): e\n" ],
    [ "print(g\n(5))\nprint(g\n  (5))\n", "print(g\n(5))\nprint(g\n      (5))\n" ],
    [ "f(a \nor b)\nf(gl.\nf)\n", "f(a\n  or b)\nf(gl.\nf)\n" ],
    [ "f(g\n// c\n(5))\nf(g\n  // c\n(5))\n", "f(g\n// c\n(5))\nf(g\n  // c\n  (5))\n" ],
    // A line break in a string's interpolation, in brackets.
    [ "print \"{{f(1,\n2)}}\"\n", "print \"{{f(1,\n2)}}\"\n" ],
]

// Found by fuzzing: each of these once formatted wrongly or not the same twice.
let found = [
    // A block in a continuation line and one in the line it continues.
    "struct Ray:\n    let temp =\n            if rnd_float() < P:\n" +
    "                radiance(Ray {{ x, tdir }}, depth) * TP\n        else:\n",
    "class resources:\n    let goap_path =\n           astar_goap(goapactions) state:\n" +
    "            state.hungry\n        fn s:\n",
    "def w() -> int:\n                if a and\n                        b:\n" +
    "                        c:\n                    far_cell.push(u_home[i])\nx\n",
    // A continuation of a continuation.
    "def f():\n                if a and\n                  b and\n                        c:\n" +
    "                    let diag = n.pdir + d\n",
    "let path =\n    f(a) b:\n        c\n\nd\n",
    "def bump(): y++::",
    "def text_pa <ss() -> int:\n    1\n",
    "f(x\n [0])\n",
    "def inside(v): return all(v\n< lssize - 1)  // c\n",
    "f(x\n-1)\n",
    "/lamp,\n// c\n\ndef f(f):\n    // a\n    g()\n",
]

// Refused, where: whether lines end in CRLF or LF.
let refusals = [
    [ "if a:\n    b\n  c\n", "3:3" ],
    [ "a = 1\nb = \"c\n", "2:5" ],
    [ "let x = 1;\n", "1:10" ],
    [ "f(a\n", "1:2" ],
    [ "if a:\n\tb\n    c\n", "3:5" ],
]

def isbug(err:format_error): return find_string(err.message, "formatter bug") >= 0

def escape(s:string): return replace_string(replace_string(s, "\n", "\\n"), "\t", "\\t")

def goldentests():
    for(goldens) g, i:
        let result, err = format_checked(g[0], "golden.lobster")
        if err: failed("golden {i}: {err.message}", g[0])
        elif result != g[1]:
            failed("golden {i}: got {escape(result)} expected {escape(g[1])}", g[0])
        else:
            let again, err2 = format_checked(g[1], "golden.lobster")
            if err2 or again != g[1]:
                failed("golden {i}: its output doesn\'t format to itself", g[1])
    for(found) src, i:
        let once, err = format_checked(src, "found.lobster")
        if err and isbug(err): failed("found {i}: {err.message}", src)
        elif not err:
            let twice, err2 = format_checked(once, "found.lobster")
            if err2 or twice != once: failed("found {i}: formatting it twice changes it", src)
    for(refusals) r:
        for([ r[0], replace_string(r[0], "\n", "\r\n") ]) src:
            let _, err = format_lobster(src)
            if not err: failed("should have been refused", src)
            elif "{err.line}:{err.column}" != r[1]:
                failed("refused at {err.line}:{err.column}, not {r[1]}: {err.message}", src)

// The layout's rules, from the formatted text.

let spacedoperators = [ "=", "==", "!=", "<=", ">=", "+=", "-=", "*=", "/=", "%=", "->", "and",
                        "or" ]
let statementwords = [ "import", "return", "if", "elif", "while", "let", "var", "def", "class",
                       "struct", "enum", "switch", "case", "not", "is" ]

def checklayout(file:string, out:string):
    def bad(why:string, at:int):
        let line = max(1, tokenize(out.substring(0, at), "\n", "", true).length)
        failed("{file}:{line}: {why}", out)
    if out.length and out[out.length - 1] != '\n': bad("doesn\'t end in a line break", out.length)
    for([ [ "\r", "has a CR" ], [ "\n\n\n", "two blank lines in a row" ] ]) c:
        let at = find_string(out, c[0])
        if at >= 0: bad(c[1], at)
    if out.length and out[0] == '\n': bad("starts with a blank line", 0)
    let tokens, err = lex(out)
    if err: bad("doesn\'t lex: {err.message}", err.offset)
    var depth = 0
    for(tokens) t, k:
        let isop = exists(spacedoperators): _ == t.text
        if k:
            let p = tokens[k - 1]
            let gap = out.substring(p.end, t.start - p.end)
            let lf = find_string_reverse(gap, "\n")
            let after = exists(spacedoperators): _ == p.text
            let word = exists(statementwords): _ == p.text
            if lf >= 0:
                if find_string(gap, "\n") > 0: bad("trailing whitespace", p.end)
                if find_string(gap, "\t", lf) >= 0: bad("a tab in indentation", t.start)
            elif t.kind == token_comment and t.text[0] == '/' and t.text[1] == '/':
                if gap.length < 2: bad("less than two spaces before a comment", t.start)
            elif p.kind != token_comment and t.kind != token_comment and p.text != "operator" and
                    not (k >= 2 and tokens[k - 2].text == "operator" and t.text == "("):
                if gap.length > 1: bad("more than one space between {p.text} and {t.text}", t.start)
                if t.text == "," and gap.length: bad("a space before a comma", t.start)
                if p.text == "," and gap != " ": bad("no space after a comma", t.start)
                if isop and gap != " ": bad("no space before {t.text}", t.start)
                if after and gap != " ": bad("no space after {p.text}", t.start)
                if word and gap != " " and not (exists([ ":", "(", ")", ",", "]" ]): _ == t.text):
                    bad("no space after {p.text}", t.start)
                if p.text == "(" and gap.length: bad("a space after (", t.start)
                if t.text == ")" and gap.length: bad("a space before )", t.start)
            // Lines outside brackets are indented in steps of 4.
            if lf >= 0 and not depth and t.kind != token_comment and (gap.length - lf - 1) % 4:
                bad("indented {gap.length - lf - 1}, not a multiple of 4", t.start)
        if t.kind != token_comment:
            if t.text == "(" or t.text == "[" or t.text == "{{": depth++
            if t.text == ")" or t.text == "]" or t.text == "}}": depth--

// Scrambling: other whitespace, the same meaning.

// The namespaces the compiler joined a name with in a log of compiler_parse: gl.f is one name,
// gl .f three tokens.
def namespaces(log:string):
    let r = []::string
    for(tokenize(log, "\n", "")) l:
        let dot = find_string_reverse(l, ".")
        if find_string(l, "identifier ") == 0 and dot > 0:
            let ns = l.substring(11, dot - 11)
            if not (exists(r): _ == ns): r.push(ns)
    return r

def scramble(src:string, joined:[string]):
    let tokens = lex(src)
    let factor = 1 + rnd(3)
    var out = repeat_string("\n", rnd(3))
    var depth = 0
    var prev:token? = nil
    var prevcode:token? = nil
    var beforeprev:token? = nil
    for(tokens) t:
        let p = prev
        if p:
            let between = src.substring(p.end, t.start - p.end)
            let newlines = fold(between, 0): _a + (_b == '\n')
            if newlines:
                if not rnd(3): out += repeat_string(" ", 1 + rnd(3))
                out += repeat_string("\n", if newlines > 1: newlines + rnd(3) else: 1)
                let orig = between.length - find_string_reverse(between, "\n") - 1
                // In brackets indentation doesn't matter, but whether there is any does.
                out += repeat_string(" ", if depth and orig: 1 + rnd(12) else: orig * factor)
            elif (p.text == "." and beforeprev and (exists(joined): _ == beforeprev.text)) or
                    (t.text == "." and (exists(joined): _ == p.text)):
                out += between
            elif (p.kind != token_comment and isname(p)) or p.text == "." or t.text == "." or
                    (p.text == "-" and beforeprev and isname(beforeprev)):
                // Where the parser looks at whether there's whitespace: `f(x)` and `f (x)`,
                // `f -1` and `f - 1` (which the formatter tells apart by the space after -).
                out += if between.length: repeat_string(" ", 1 + rnd(3)) else: ""
            else:
                let n = rnd(4)
                out += if n or merges(p, t): repeat_string(" ", max(n, 1)) else: ""
        out += t.text
        if t.kind != token_comment:
            if t.text == "(" or t.text == "[" or t.text == "{{": depth++
            if t.text == ")" or t.text == "]" or t.text == "}}": depth--
            beforeprev = prevcode
            prevcode = t
        prev = t
    out += repeat_string("\n", 1 + rnd(3))
    return if rnd(5): out else: replace_string(out, "\n", "\r\n")

// Breaking: the code damaged somewhere.

def linesof(s:string):
    let lines = tokenize(s, "\n", "", true)
    if not lines.length: lines.push("")
    return lines

let snippets = [ "(", ")", "[", "]", "{{", "}}", ":", ",", "-", "<", ">", "=", ".", "::", "if", "x",
                 "1", "\"s\"", "//", "/*", "*/", "\"\"\"", "\t", "  ", "\n", "\n    ", "def f():",
                 "fn x:", "->" ]

def mutate(src:string, other:string):
    let lines = linesof(src)
    let l = rnd(lines.length)
    switch rnd(10):
        case 0:
            lines.remove(l)
        case 1:
            lines.insert(l, lines[l])
        case 2:
            lines[l] = repeat_string(" ", 1 + rnd(4)) + lines[l]
        case 3:
            var n = 0
            while n < 4 and n < lines[l].length and lines[l][n] == ' ': n++
            lines[l] = lines[l].substring(n, lines[l].length - n)
        case 4:
            if l + 1 < lines.length:
                lines[l] = lines[l] + " " + lines[l + 1]
                lines.remove(l + 1)
        case 5:
            // Spliced: the start of this, the rest of another file.
            let others = linesof(other)
            let o = rnd(others.length)
            return concat_string(lines.slice(0, l), "\n") + "\n" +
                concat_string(others.slice(o, others.length - o), "\n")
        default:
            let at = rnd(src.length + 1)
            if rnd(2):
                return src.substring(0, at) + snippets[rnd(snippets.length)] +
                    src.substring(at, src.length - at)
            let n = min(1 + rnd(3), src.length - at)
            return src.substring(0, at) + src.substring(at + n, src.length - at - n)
    return concat_string(lines, "\n")

// The corpus.

var seed = 1
var rounds = 10
var outdir = ""
let paths = []::string
let args = command_line_arguments()
var a = 0
while a < args.length:
    let value = if a + 1 < args.length: args[a + 1] else: ""
    if args[a] == "-seed" and value.length:
        seed = string_to_int(value)
        a++
    elif args[a] == "-out" and value.length:
        outdir = value
        a++
    elif args[a] == "-rounds" and value.length:
        rounds = string_to_int(value)
        a++
    else:
        paths.push(args[a])
    a++
rnd_seed(seed)

// What formatting without the compiler's check let through: in code without errors (each a
// formatter bug, as the compiler's check reports it), and in code with errors (which formatting
// would change, or recover from differently).
var unsafeclean, unsafebroken = 0, 0

// src formatted by format_lobster, and then the compiler's check of that.
def formatboth(src:string, file:string) -> string, format_error?:
    let result, err = format_lobster(src)
    if err: return result, err
    let e = compiler_check(file, replace_string(src, "\r\n", "\n"), result)
    guard e:
        return result, nil
    if isbug(e): unsafeclean++
    else: unsafebroken++
    return "", e

let start = seconds_elapsed()
goldentests()
let files = []::string
for(paths) p: append_into(files, lobster_files(p))
var formatted, witherrors, scrambled, meaningchanged, broken, refused = 0, 0, 0, 0, 0, 0
let sources = map(files) f: replace_string(read_file(f) or "", "\r\n", "\n")
for(files) file, fi:
    let src = sources[fi]
    let result, err = formatboth(src, file)
    if err:
        // Only for an error the compiler reports, which formatting would change.
        let _, errors = compiler_parse(file, src)
        let compilersays = exists(errors) e: e == err.message
        if compilersays: witherrors++
        else: failed("{file}: refused: {err.message}", src)
        continue
    formatted++
    checklayout(file, result)
    let again, err2 = formatboth(result, file)
    if err2 or again != result: failed("{file}: formatting it again changes it", result)
    let parsed = compiler_parse(file, result)
    let joined = namespaces(parsed)
    for(rounds):
        let s = scramble(result, joined)
        if compiler_parse(file, s) != parsed:
            meaningchanged++
        else:
            scrambled++
            let r1, e1 = formatboth(s, file)
            if e1: failed("{file}: scrambled: {e1.message}", s)
            elif r1 != result: failed("{file}: scrambled, formats differently", s)
        var m = result
        for(1 + rnd(3)): m = mutate(m, sources[rnd(sources.length)])
        broken++
        let r2, e2 = formatboth(m, file)
        if e2:
            if isbug(e2): failed("{file}: broken: {e2.message}", m)
            else: refused++
        else:
            let r3, e3 = formatboth(r2, file)
            if e3 or r3 != r2: failed("{file}: broken, formatting it twice changes it", m)

for(failures) f, i:
    print f.what
    if outdir.length: write_file("{outdir}/failure_{i}.lobster", f.input, false, true)
print "{goldens.length} golden, {formatted} files formatted ({witherrors} refused for an error " +
    "formatting would change), {scrambled} scrambled ({meaningchanged} more changed what they " +
    "mean), {broken} broken ({refused} refused): {failures.length} failures, in " +
    "{seconds_elapsed() - start} s"
print "without the compiler\'s check: {unsafeclean} changes to how code without errors parses, " +
    "{unsafebroken} to code with errors"
set_exit_code(if failures.length: 1 else: 0)
