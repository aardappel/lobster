// Formats Lobster source in the layout of docs/source/style_guide.md, like gofmt: no options.
// Only whitespace between tokens changes, and never which line a token is on (other than blank
// lines). The result is checked to read the same to the compiler's lexer and parser, as far as
// that can be told from the source alone (compilercheck.lobster asks the compiler itself);
// what can't be formatted safely is refused.

import std
import dictionary

// Why source can't be formatted, and where (1-based, in characters).
class format_error:
    message:string
    line:int
    column:int

// Why source can't be formatted, at an offset.
class refusal:
    message:string
    offset:int

enum token_kind:
    token_ident
    token_number
    token_string
    token_comment
    token_op

// What a token is, where its text alone doesn't say.
enum token_role:
    role_none
    role_unary  // - ~ ++ --
    role_postfix
    role_generic  // < > >> around type arguments
    role_lt       // a < without spaces where it could be the above
    role_index    // [ ]
    role_list
    role_type
    role_typecolon  // a:int
    role_inherit    // class a : b
    role_block      // a :: that ends a line: `case T::`
    role_defname    // def name(
    role_opname     // def operator+(
    role_namedot    // the . after a name: gl.f

class token:
    kind:token_kind
    text:string
    start:int
    end:int
    // Spaces and tabs before it, since the last token that isn't a comment: the compiler's
    // `whitespacebefore`, which some parses depend on.
    ws:int
    // Filled in by analyze: where it is among the tokens that aren't comments, how many brackets
    // it's in, its matching bracket, and of a < that starts type arguments, where they end.
    role:token_role = role_none
    i:int = -1
    depth:int = 0
    match:int = -1
    genericend:int = -1

private def startsat(s:string, at:int, prefix:string):
    return at + prefix.length <= s.length and
        compare_substring(s, at, prefix, 0, prefix.length) == 0

private def newlinein(s:string, start:int, end:int):
    for(end - start) k: if s[start + k] == '\n': return true
    return false

// Where the line `at` is on starts.
private def linestart(s:string, at:int):
    var i = at
    while i > 0 and s[i - 1] != '\n': i--
    return i

// The width of what follows the last line break, in characters.
private def lastlinewidth(s:string):
    var n = 0
    var i = s.length - 1
    while i >= 0 and s[i] != '\n':
        if s[i] < 0x80 or s[i] >= 0xC0: n++
        i--
    return n

// The line and column (1-based, in characters) of an offset.
def format_position(src:string, offset:int) -> int, int:
    var line, column = 1, 1
    for(min(offset, src.length)) i:
        if src[i] == '\n':
            line++
            column = 1
        elif src[i] < 0x80 or src[i] >= 0xC0:
            column++
    return line, column

// Longest first, such that the first match is the token.
private let operators = [
    "<<=", ">>=",
    "++", "--", "+=", "-=", "->", "*=", "/=", "%=", "<=", "<<", "==", "!=", ">=", ">>", "&=",
    "|=", "^=", "::", "..",
    "+", "-", "*", "/", "%", "<", ">", "=", "!", "&", "|", "^", "~", "?", ":", ",", ".", "(",
    ")", "[", "]", "{{", "}}", "@",
]

private def isdigit(c): return c >= '0' and c <= '9'
private def isalpha(c): return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z')
private def isidentstart(c): return isalpha(c) or c == '_' or c >= 0x80
private def isidentcont(c): return isidentstart(c) or isdigit(c)

// The tokens of src, comments included, or why it doesn't lex. Unlike the compiler's lexer this
// interprets nothing: a string, interpolations and all, is one token.
def lex(src:string) -> [token], refusal?:
    let tokens = []
    var ws = 0
    var i = 0
    def fail(message:string, at:int) -> void:
        return [], refusal { message, at } from lex
    def char(at): return if at < src.length: src[at] else: 0
    // Where the string or character constant starting at `start` ends.
    def stringend(start:int) -> int:
        if startsat(src, start, "\"\"\""):
            let end = find_string(src, "\"\"\"", start + 3)
            if end < 0: fail("end of file in multi-line string constant", start)
            return end + 3
        let quote = src[start]
        var j = start + 1
        while true:
            let c = char(j)
            if c == 0 or c == '\n' or c == '\r': fail("end of line in string constant", start)
            if c == '\\':
                j++
            elif c == quote:
                return j + 1
            elif quote == '\"' and c == '{':
                if char(j + 1) == '{':
                    j++
                else:
                    // An interpolation, up to its } (on another line if in brackets).
                    var depth = 0
                    var k = j + 1
                    while true:
                        let d = char(k)
                        if d == 0: fail("unterminated {{}} in string constant", j)
                        if d == '\"' or d == '\'':
                            k = stringend(k) - 1
                        elif d == '{':
                            depth++
                        elif d == '}':
                            if not depth:
                                j = k
                                break
                            depth--
                        k++
            elif quote == '\"' and c == '}' and char(j + 1) == '}':
                j++
            j++
        return 0
    // Where the number starting at `start` ends, as the compiler reads one.
    def numberend(start:int):
        var p = start
        def digits(): while isdigit(char(p)): p++
        if char(p) == '0' and (char(p + 1) == 'x' or char(p + 1) == 'X'):
            p += 2
            while isdigit(char(p)) or ((char(p) | 0x20) >= 'a' and (char(p) | 0x20) <= 'f'): p++
            return p
        var isfloat = char(p) == '.'
        if isfloat: p++
        digits()
        if not isfloat and char(p) == '.' and char(p + 1) != '.' and not isalpha(char(p + 1)):
            p++
            isfloat = true
            digits()
        if isfloat and (char(p) == 'e' or char(p) == 'E'):
            p++
            if char(p) == '+' or char(p) == '-': p++
            digits()
        return p
    def add(kind:token_kind, end:int):
        let comment = kind == token_comment
        tokens.push(token { kind, src.substring(i, end - i), i, end, if comment: 0 else: ws })
        if not comment: ws = 0
        i = end
    def addcomment(end:int): add(token_comment, if end < 0: src.length else: end)
    if startsat(src, 0, "#!"): addcomment(find_string(src, "\n"))
    while i < src.length:
        let c = src[i]
        switch c:
            case ' ', '\t', '\r', 12:
                ws++
                i++
            case '\n':
                i++
            case '\"', '\'':
                add(token_string, stringend(i))
            default:
                if startsat(src, i, "//"):
                    addcomment(find_string(src, "\n", i))
                elif startsat(src, i, "/*"):
                    let end = find_string(src, "*/", i + 2)
                    if end < 0: fail("end of file in multi-line comment", i)
                    add(token_comment, end + 2)
                elif isidentstart(c):
                    var end = i + 1
                    while end < src.length and isidentcont(src[end]): end++
                    add(token_ident, end)
                elif isdigit(c) or (c == '.' and isdigit(char(i + 1))):
                    add(token_number, numberend(i))
                else:
                    let op = find(operators): startsat(src, i, _)
                    if op < 0: fail("illegal character \'{unicode_to_string([ c ])}\'", i)
                    add(token_op, i + operators[op].length)
    return tokens, nil

// Whether two tokens written together would lex as something else.
def merges(a:token, b:token):
    let t, err = lex(a.text + b.text)
    return err != nil or t.length != 2 or t[0].text != a.text

// The formatted source, or why it can't be formatted safely.
def format_lobster(source:string) -> string, format_error?:
    let src = replace_string(source, "\r\n", "\n")
    let result, err = formatted(src)
    guard err:
        return result, nil
    let line, column = format_position(src, err.offset)
    return "", format_error { err.message, line, column }

private def formatted(src:string) -> string, refusal?:
    let tokens, err = lex(src)
    if err: return "", err
    let syntax = analyze(src, tokens)
    let blocks = structure(syntax)
    let result = layout(syntax, tokens)
    if result != src: check(syntax, tokens, blocks, result)
    return result, nil

private def refuse(message:string, at:int) -> void:
    return "", refusal { message, at } from formatted

private def bug(what:string, at:int):
    refuse("formatting would change {what} (this is a formatter bug)", at)

private def set(xs:[string]):
    let d = dictionary<string, int>(67)
    for(xs) x: d.set(x, 1)
    return d

private def has(d:dictionary<string, int>, s:string): return d.get(s, 0) != 0

private let indentwidth = 4
// Of a line continuing the one before when a block follows that's indented once from that line,
// which the two need telling apart by.
private let contwidth = 8
// Before a comment at the end of a line.
private let commentgap = "  "

private let keywords = set([
    "class", "def", "fn", "return", "is", "typeof", "struct", "import", "int", "float", "string",
    "any", "void", "from", "program", "private", "resource", "enum", "enum_flags", "var", "let",
    "pakfile", "if", "else", "elif", "while", "for", "switch", "case", "default", "out_of_range",
    "namespace", "break", "continue", "super", "operator", "attribute", "member", "member_frame",
    "static", "static_frame", "abstract", "union", "guard", "constructor", "assert", "nonlocal",
    "nil", "and", "or", "not",
])
// Keywords written like a call, without a space before their (: `for(xs) x:`.
private let callkeywords = set([ "for", "while", "fn" ])
// Keywords that start statements or are operators. The rest (`int`, `resource`, `super`, ...)
// the parser reads much like identifiers.
private let statementkeywords = set([
    "class", "def", "fn", "return", "is", "struct", "import", "from", "program", "private", "enum",
    "enum_flags", "var", "let", "pakfile", "if", "else", "elif", "while", "for", "switch", "case",
    "default", "namespace", "break", "continue", "operator", "attribute", "member",
    "member_frame", "static", "static_frame", "abstract", "guard", "assert", "nonlocal", "and",
    "or", "not",
])
private let valuekeywords = set([ "nil", "super" ])
private let declarations = set([ "let", "var", "member", "member_frame", "static", "static_frame" ])
// After which a line break continues the line (the lexer's `cont`).
private let continues = set([
    ",", "+", "-", "*", "/", "%", "<", ">", "=", "!", "&", "|", "^", "~", "+=", "-=", "*=", "/=",
    "%=", "&=", "|=", "^=", "<<=", ">>=", "->", "<=", ">=", "<<", ">>", "==", "!=", "::", "and",
    "or", "operator",
])

private def isopener(t:token):
    return t.kind == token_op and (t.text == "(" or t.text == "[" or t.text == "{{")
private def iscloser(t:token):
    return t.kind == token_op and (t.text == ")" or t.text == "]" or t.text == "}}")
private def closer(open:string):
    return switch open:
        case "(": ")"
        case "[": "]"
        default: "}}"

private def isident(t:token): return t.kind == token_ident and not keywords.has(t.text)
// Whether a space after it can matter to the parser: before a ( or <, or an argument.
def isname(t:token): return t.kind == token_ident and not statementkeywords.has(t.text)
// Whether this can start an argument to a call without (): `f x`, `f -1`.
private def startsargument(t:token):
    if t.kind != token_op and t.kind != token_comment: return true
    return t.text == "(" or t.text == "[" or t.text == "{{" or t.text == "-" or t.text == "~" or
        t.text == "++" or t.text == "--"

// Whether a line break after it continues the line.
private def continuing(t:token):
    return continues.has(t.text) and t.role != role_generic and t.role != role_block

private def isoperand(t:token):
    return t.kind == token_ident or t.kind == token_number or t.kind == token_string

// Whether it ends a value, which a ( or [ after it calls or indexes.
private def endsvalue(t:token):
    return t.kind == token_number or t.kind == token_string or isident(t) or
        valuekeywords.has(t.text) or iscloser(t) or (t.role == role_generic and t.text != "<") or
        t.role == role_postfix

// The tokens that aren't comments, with what analyze found about them.
private class analysis:
    src:string
    code:[token]

    def at(i:int) -> token?: return if i >= 0 and i < code.length: code[i] else: nil

    def textat(i:int):
        let t = at(i)
        return if t: t.text else: ""

    def newlinebefore(i:int):
        return i > 0 and i < code.length and newlinein(src, code[i - 1].end, code[i].start)

    // Whether code[i] starts a statement on a line of its own.
    def startsstatement(i:int):
        return i == 0 or (newlinebefore(i) and code[i].depth == 0 and not continuing(code[i - 1]))

    // Of type arguments `name<` (no space), then types, then a > not followed by what an
    // expression goes on with: where they end, or -1.
    def scangeneric(i:int):
        var level = 0
        for(code.length - i - 1) n:
            let j = i + 1 + n
            let t = code[j]
            if t.text == "<":
                if j > i + 1 and (not isname(code[j - 1]) or t.ws or newlinebefore(j)): return -1
                level++
            elif t.text == ">" or t.text == ">>":
                level -= t.text.length
                if level < 0: return -1
                if not level:
                    let next = at(j + 1)
                    if next and not newlinebefore(j + 1) and isoperand(next): return -1
                    return j
            elif not (isname(t) or t.kind == token_number or t.text == "," or t.text == "[" or
                      t.text == "]" or t.text == "?"):
                return -1
        return -1

    // Marks the type starting at code[i], returning where it ends.
    def marktype(i:int) -> int:
        let t = at(i)
        guard t:
            return i
        var j = i
        if t.text == "[":
            let close = code[t.match]
            t.role = role_type
            close.role = role_type
            marktype(i + 1)
            j = close.i + 1
        elif t.kind == token_ident:
            j++
            let lt = at(j)
            if lt and lt.genericend >= 0: j = lt.genericend + 1
        else:
            return i
        while textat(j) == "?": j++
        return j

    def marktypecolon(j:int):
        code[j].role = role_typecolon
        marktype(j + 1)

private def analyze(src:string, tokens:[token]):
    let code = filter(tokens): _.kind != token_comment
    let syntax = analysis { src, code }
    let open = []
    for(code) t, i:
        t.i = i
        t.depth = open.length
        t.role = role_none
        t.match = -1
        t.genericend = -1
        if isopener(t):
            open.push(t)
        elif iscloser(t):
            if not open.length or closer(open.top.text) != t.text:
                refuse("unmatched \'{t.text}\'", t.start)
            let o = open.pop()
            o.match = i
            t.match = o.i
    if open.length: refuse("unmatched \'{open.top.text}\'", open.top.start)

    var i = 0
    while i + 1 < code.length:
        let lt = code[i + 1]
        if isname(code[i]) and lt.text == "<" and not lt.ws and not syntax.newlinebefore(i + 1):
            let end = syntax.scangeneric(i)
            if end < 0:
                lt.role = role_lt
            else:
                for(end - i) n:
                    let j = i + 1 + n
                    let t = code[j]
                    if t.text == "<" or t.text == ">" or t.text == ">>": t.role = role_generic
                    if t.text == "[" or t.text == "]": t.role = role_type
                    if t.text == "<": t.genericend = syntax.scangeneric(j - 1)
                i = end
        i++

    for(code) t, k:
        let prev = if syntax.startsstatement(k): nil else: syntax.at(k - 1)
        let prevvalue = prev and endsvalue(prev)
        let spaced = prev and isident(prev) and t.ws and not syntax.newlinebefore(k)
        if t.text == "~":
            t.role = role_unary
        elif t.text == "-" and (not prevvalue or
                                // `f -1`: an argument to a call without ().
                                (spaced and syntax.at(k + 1) and not code[k + 1].ws and
                                 not syntax.newlinebefore(k + 1))):
            t.role = role_unary
        elif t.text == "++" or t.text == "--":
            t.role = if prevvalue: role_postfix else: role_unary
        elif t.text == "[" and t.role == role_none:
            t.role = if prevvalue and not spaced: role_index else: role_list
            code[t.match].role = t.role
        elif t.text == "::" and syntax.newlinebefore(k + 1) and t.depth == 0:
            t.role = role_block
        elif t.text == "." and prev and isname(prev) and not syntax.newlinebefore(k):
            t.role = role_namedot

    for(code) t, k:
        // Parameters: `def name(`, `def name<T>(`, `def operator+(`, `fn(`.
        var params = -1
        if t.text == "def" and syntax.at(k + 1):
            var j = k + 1
            if code[j].text == "operator":
                // Its name: `operator+`, `operator[]`, `operator()`.
                j++
                def call(): return syntax.textat(j + 1) == ")" and syntax.textat(j + 2) == "("
                while syntax.at(j) and (code[j].text != "(" or call()):
                    code[j].role = role_opname
                    j++
            else:
                let name = code[j]
                j++
                let lt = syntax.at(j)
                if lt and lt.genericend >= 0: j = lt.genericend + 1
                if syntax.textat(j) == "(": name.role = role_defname
            if syntax.textat(j) == "(": params = j
        elif t.text == "fn" and syntax.textat(k + 1) == "(":
            params = k + 1
        if params >= 0:
            let p = code[params]
            for(p.match - params - 1) n:
                let j = params + 1 + n
                let c = code[j]
                if c.depth == p.depth + 1 and isident(c) and syntax.textat(j + 1) == ":":
                    syntax.marktypecolon(j + 1)
        // Declarations, up to their =: `let a:int, b:float = ...`.
        if declarations.has(t.text):
            var j = k + 1
            while j < code.length and code[j].depth == t.depth and not syntax.newlinebefore(j):
                if code[j].text == "=": break
                if isident(code[j]) and syntax.textat(j + 1) == ":": syntax.marktypecolon(j + 1)
                j++
        if t.text == "->":
            var j = syntax.marktype(k + 1)
            while syntax.textat(j) == "," and code[j].depth == t.depth: j = syntax.marktype(j + 1)
        elif t.text == "is" or (t.text == "::" and t.role != role_block):
            syntax.marktype(k + 1)
        // `class a : b`, unlike the : that starts the body.
        let pt = syntax.textat(k - 1)
        if (t.text == "class" or t.text == "struct") and
                (syntax.startsstatement(k) or pt == "private" or pt == "abstract"):
            var j = k + 1
            while j < code.length and not syntax.newlinebefore(j):
                if code[j].text == ":" and code[j].depth == t.depth and syntax.at(j + 1) and
                        not syntax.newlinebefore(j + 1):
                    code[j].role = role_inherit
                    break
                j++
    return syntax

private def spacing(a:token, b:token):
    let ra, rb = a.role, b.role
    if b.text == ",": return ""
    if a.text == ",": return " "
    if rb == role_postfix or ra == role_unary or ra == role_opname or a.text == "operator":
        return ""
    if a.text == "import" and b.text == ".": return " "  // import .file
    if a.text == "." or a.text == ".." or a.text == "::" or b.text == "." or b.text == ".." or
            b.text == "::":
        return ""
    if a.text == "@" or b.text == "?": return ""
    if (ra == role_generic and a.text == "<") or rb == role_generic or ra == role_lt: return ""
    if b.text == ":": return if rb == role_inherit: " " else: ""
    if a.text == ":": return if ra == role_typecolon: "" else: " "
    if a.text == "(" or b.text == ")": return ""
    if a.text == "[": return if ra == role_list and b.text != "]": " " else: ""
    if b.text == "]": return if rb == role_list and a.text != "[": " " else: ""
    if a.text == "{{": return if b.text == "}}": "" else: " "
    if b.text == "}}": return " "
    if b.text == "(": return if endsvalue(a) or callkeywords.has(a.text): "" else: " "
    if b.text == "[": return if rb == role_index: "" else: " "
    return " "

// Whether the parser reads b differently with or without whitespace between a and it: `f(x)` and
// `f (x)`, `f<T>`, `gl.f` and `gl .f`.
private def wsdecides(a:token, b:token):
    if a.role == role_namedot or b.role == role_namedot: return true
    return isname(a) and a.role != role_defname and (b.text == "(" or b.text == "<")

// Whether the parser can read b differently without whitespace between a and it: also `f x` and
// `f -1` (if f is a function).
private def wsmatters(a:token, b:token):
    return wsdecides(a, b) or (isname(a) and a.role != role_defname and startsargument(b))

private def separator(a:token, b:token):
    if a.kind == token_comment or b.kind == token_comment: return " "
    // Keep what's there where the parser looks.
    if wsdecides(a, b) or (b.ws and wsmatters(a, b)): return if b.ws: " " else: ""
    let s = spacing(a, b)
    return if s == "" and merges(a, b): " " else: s

// A level of indentation, as the compiler's lexer keeps them (Lex::indentstack).
private class level:
    orig:int
    // Of the lines at this level, and the one blocks in it are indented from.
    indent:int
    children:int
    // A line continuing the one before (after an operator), indented more.
    cont:bool
    // The body of a class or struct, whose lines declare fields.
    fields:bool

private class bracket:
    indent:int
    hanging:bool
    align:int = -1

// What a line is to the lexer: one that starts a statement, continues the one before after an
// operator, or after a ., or is in brackets (where its indentation doesn't matter), or a comment.
private enum row_kind:
    row_comment
    row_block
    row_cont
    row_dot
    row_bracket

private class row:
    tokens:[token]
    orig:int
    blanks:int
    kind:row_kind = row_comment

private def firstcode(r:row) -> token?:
    for(r.tokens) t: if t.kind != token_comment: return t
    return nil

private def rows(syntax:analysis, tokens:[token]):
    let src = syntax.src
    let result = []
    var end = 0
    for(tokens) t:
        var newlines = 0
        for(t.start - end) k: if src[end + k] == '\n': newlines++
        if newlines or not result.length:
            result.push(row { [ t ], t.start - linestart(src, t.start), max(0, newlines - 1) })
        else:
            result.top.tokens.push(t)
        end = t.end
    for(result) r:
        let first = firstcode(r)
        if first:
            if first.depth: r.kind = row_bracket
            elif first.text == ".": r.kind = row_dot
            elif first.i > 0 and continuing(syntax.code[first.i - 1]): r.kind = row_cont
            else: r.kind = row_block
    return result

// Its text, without trailing whitespace on a // comment.
private def tokentext(t:token):
    guard startsat(t.text, 0, "//"):
        return t.text
    var n = t.text.length
    while n and (t.text[n - 1] == ' ' or t.text[n - 1] == '\t' or t.text[n - 1] == '\r' or
                 t.text[n - 1] == 12):
        n--
    return t.text.substring(0, n)

// Puts a line that starts a statement at `orig` on `levels`, as Lex::Next does. False if it's at
// no level.
private def place(levels:[level], orig:int, fields:bool):
    while true:
        let t = levels.top
        if orig > t.orig:
            let indent = t.children + indentwidth
            levels.push(level { orig, indent, indent, false, fields })
        elif orig < t.orig:
            var cont = false
            while orig < levels.top.orig: cont = levels.pop().cont
            if cont: continue
            return orig == levels.top.orig
        return true
    return true

private class trailingcomment:
    line:int
    code:string
    comment:string

private def layout(syntax:analysis, tokens:[token]):
    let all = rows(syntax, tokens)
    let levels = [ level { 0, 0, 0, false, false } ]
    let brackets = []::bracket
    let out = []::string
    // The code and comment of each line of `out` that ends in one, to line them up.
    let trailing = []::trailingcomment
    var prevcode:token? = nil
    var fields = false      // Whether the last statement started a class or struct.
    var opensblock = false  // Whether the last line ended with the : of a block.
    var seencode = false
    // Comments on lines of their own between continuation lines, which go with the line after.
    let pending = []::int
    // Of each line, the indentation in the source of the next one that starts a statement, or -1.
    let nextblock = map(all): -1
    var upcoming = -1
    reverse(all) r, k:
        nextblock[k] = upcoming
        if r.kind == row_block: upcoming = r.orig

    def bracketindent(first:token?):
        let b = brackets.top
        if first and iscloser(first): return b.indent
        return if b.hanging or b.align < 0: b.indent + indentwidth else: b.align

    // Once from the line it continues, or twice if a block follows that is indented once
    // (`if a and\n b:`). A block that starts in it and continues at its level (`let x =\n if a:
    // ...\n else:`) is indented from it.
    def contindent(r:int, first:token):
        let line = all[r]
        let p = levels.top
        if line.orig < p.orig:
            refuse("line continuation can\'t indent less than the previous line", first.start)
        if line.orig == p.orig:
            if p.cont: return p.indent
            return p.indent + (if nextblock[r] > p.orig: contwidth else: indentwidth)
        // What follows, up to the line that dedents from it: lines at its level (a block in it
        // continuing), deeper ones (a block in it), and whether that line is deeper than `p` (a
        // block in `p`, unless that is a continuation whose blocks are indented from the line
        // before).
        let parent = if p.cont and p.children != p.indent: -1 else: p.orig
        var lands, nested, between = false, false, false
        for(all.length - r - 1) n:
            let next = all[r + 1 + n]
            if next.kind == row_block:
                if next.orig < line.orig:
                    between = parent >= 0 and next.orig > parent
                    break
                if next.orig == line.orig: lands = true
                else: nested = true
        let own = lands or between
        let indent = p.indent + (if between or (nested and not own): contwidth else: indentwidth)
        levels.push(level { line.orig, indent, if own: indent else: p.children, true, false })
        return indent

    def blockindent(r:int, first:token, fc:token?):
        let line = all[r]
        if not seencode and line.orig: refuse("the first line of code is indented", first.start)
        if not place(levels, line.orig, fields): refuse("inconsistent dedent", first.start)
        if fc:
            let i = fc.i
            let next = syntax.textat(i + 1)
            fields = fc.text == "class" or fc.text == "struct" or
                ((fc.text == "private" or fc.text == "abstract") and
                 (next == "class" or next == "struct"))
            if levels.top.fields:
                // Fields: `a:int`, `a, b:float = 0.0`.
                let code = syntax.code
                var j = i
                while j < code.length and isident(code[j]) and syntax.textat(j + 1) == ",": j += 2
                if j < code.length and isident(code[j]) and syntax.textat(j + 1) == ":" and
                        code[j + 1].role == role_none:
                    syntax.marktypecolon(j + 1)
        return levels.top.indent

    // Of a comment on a line of its own: it goes with the code around it, at the deepest level (of
    // the one it's in, or the block the next line starts) not deeper than it was.
    def commentindent(r:int):
        let orig = all[r].orig
        var best, bestorig = 0, -1
        def consider(l:level):
            if l.orig <= orig and l.orig >= bestorig:
                bestorig = l.orig
                best = l.indent
        for(levels) l: consider(l)
        let after = copy(levels)
        if nextblock[r] >= 0 and place(after, nextblock[r], false): consider(after.top)
        return best

    // Whether the line is in brackets after a name, where the next code has no whitespace before
    // it, which comment lines between would add to: `g\n(5)` calls g, `g\n (5)` (or
    // `g \n(5)`) calls it with (5).
    def unindented():
        let p = prevcode
        guard p and p.i + 1 < syntax.code.length:
            return false
        return not syntax.code[p.i + 1].ws and (isname(p) or p.role == role_namedot)

    def indentof(r:int):
        let line = all[r]
        let first = line.tokens[0]
        let fc = firstcode(line)
        if first.kind == token_comment and fc and not startsat(first.text, 0, "#!"):
            refuse("code after a comment at the start of a line", fc.start)
        return switch line.kind:
            case row_bracket:
                if unindented(): 0 else: bracketindent(fc)
            case row_dot:
                levels.top.indent + indentwidth
            case row_cont:
                contindent(r, first)
            case row_block:
                blockindent(r, first, fc)
            default:
                let pc = prevcode
                if brackets.length: (if unindented(): 0 else: bracketindent(nil))
                elif pc and continuing(pc): levels.top.indent + indentwidth
                else: commentindent(r)

    // The line all[r], indented by `indent`, and the comment it ends in if any.
    def write(r:int, indent:int) -> string, trailingcomment?:
        let rowtokens = all[r].tokens
        var line = repeat_string(" ", indent)
        var prev:token? = nil
        var comment:trailingcomment? = nil
        var align:bracket? = nil
        for(rowtokens) t, i:
            let sep = if prev: separator(prev, t) else: ""
            if t.kind == token_comment and startsat(t.text, 0, "//") and prev:
                comment = trailingcomment { out.length, line, tokentext(t) }
            line += if comment and t.kind == token_comment: commentgap else: sep
            if t.kind != token_comment and align:
                align.align = lastlinewidth(line)
                align = nil
            line += tokentext(t)
            if t.kind != token_comment:
                if isopener(t):
                    let hanging = not exists(rowtokens) u, k: k > i and u.kind != token_comment
                    let b = bracket { indent, hanging }
                    brackets.push(b)
                    if not hanging: align = b
                elif iscloser(t):
                    brackets.pop()
                prevcode = t
            prev = t
        return line, comment

    for(all) row, r:
        let pc = prevcode
        let indent = indentof(r)
        let line, comment = write(r, indent)
        let fc = firstcode(row)
        if out.length and not opensblock and row.blanks: out.push("")
        if not fc and not brackets.length and pc and continuing(pc):
            pending.push(out.length)
        elif fc:
            for(pending) at:
                var n = 0
                while n < out[at].length and out[at][n] == ' ': n++
                out[at] = repeat_string(" ", indent) + out[at].substring(n, out[at].length - n)
            pending.truncate(0)
        if comment and find_string(comment.code, "\n") < 0:
            comment.line = out.length
            trailing.push(comment)
        out.push(line)
        let last = prevcode
        if fc:
            seencode = true
            opensblock = last and not brackets.length and
                ((last.text == ":" and last.role == role_none) or last.role == role_block)

    // Comments at the end of consecutive lines line up, as gofmt does.
    var i = 0
    while i < trailing.length:
        var j = i
        while j + 1 < trailing.length and trailing[j + 1].line == trailing[j].line + 1: j++
        let group = trailing.slice(i, j - i + 1)
        let width = fold(group, 0): max(_a, lastlinewidth(_b.code))
        for(group) c:
            out[c.line] = c.code + repeat_string(" ", width - lastlinewidth(c.code)) + commentgap +
                c.comment
        i = j + 1
    return if out.length: concat_string(out, "\n") + "\n" else: ""

// What the compiler's lexer makes of the indentation (following Lex::Next): where blocks start
// and end. Refuses what it would report an error for.
private def structure(syntax:analysis):
    let src = syntax.src
    let events = []::string
    let stack = [ 0 ]
    let conts = [ false ]
    var previndent = ""
    for(syntax.code) t, i:
        guard i and not t.depth and t.text != "." and syntax.newlinebefore(i)
        let start = linestart(src, t.start)
        let indent = t.start - start
        // A line indented with other spaces and tabs than the one before it.
        let chars = src.substring(start, indent)
        for(min(chars.length, previndent.length)) k:
            if chars[k] != previndent[k]:
                refuse("adjacent lines do not start with the same sequence of spaces and/or tabs",
                       t.start)
        previndent = chars
        if continuing(syntax.code[i - 1]):
            if indent < stack.top:
                refuse("line continuation can\'t indent less than the previous line", t.start)
            if indent > stack.top:
                stack.push(indent)
                conts.push(true)
            continue
        while true:
            if indent > stack.top:
                stack.push(indent)
                conts.push(false)
                events.push("{i} indent")
            elif indent < stack.top:
                var iscont, dedent = false, false
                while stack.top > indent:
                    stack.pop()
                    iscont = conts.pop()
                    if not iscont:
                        events.push("{i} dedent")
                        dedent = true
                // Then Lex::Next goes again, which after a dedent adds a line break before it,
                // which the parser takes the same as none.
                if iscont:
                    if dedent and indent == stack.top: break
                    continue
                if indent != stack.top: refuse("inconsistent dedent", t.start)
            else:
                events.push("{i} line")
            break
    return concat_string(events, ",")

// Refuses `result` unless it has the tokens of `syntax`, the same blocks (`blocks`), and the same
// whitespace where the parser looks at it.
private def check(syntax:analysis, tokens:[token], blocks:string, result:string):
    let after, err = lex(result)
    if err or after.length != tokens.length: bug("the tokens", 0)
    for(tokens) t, i: if tokentext(t) != tokentext(after[i]): bug("the tokens", t.start)
    let aftersyntax = analyze(result, after)
    for(syntax.code) t, i:
        // A line break outside brackets is a token of its own.
        guard i and (t.depth or not syntax.newlinebefore(i))
        let a = syntax.code[i - 1]
        let ws = aftersyntax.code[i].ws
        if (if wsdecides(a, t): (t.ws > 0) != (ws > 0) else: t.ws and not ws and wsmatters(a, t)):
            bug("whitespace the parser uses", t.start)
    if structure(aftersyntax) != blocks: bug("the indentation", 0)
