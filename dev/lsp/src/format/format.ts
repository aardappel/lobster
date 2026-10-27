// Formats Lobster source in the one layout of docs/source/style_guide.md, as gofmt does Go:
// there are no options. Only the whitespace between tokens changes, never the tokens or which
// line they're on (other than blank lines), and the result is checked to parse the same: what
// can't be formatted safely (code that doesn't lex, or indents inconsistently) is refused with
// a FormatError rather than guessed at.
//
// This only depends on ./lex, such that it can be built on its own (see main.ts).

import { FormatError, lex, Token } from "./lex";

export { FormatError } from "./lex";

const INDENT = 4;
// Of a line that continues the one before after an operator where a block follows that's
// indented once from that line, to tell them apart (which the compiler needs too).
const CONTINUATION = 8;

// Before a comment at the end of a line.
const COMMENT_GAP = "  ";

const KEYWORDS = new Set([
    "class", "def", "fn", "return", "is", "typeof", "struct", "import", "int", "float", "string",
    "any", "void", "from", "program", "private", "resource", "enum", "enum_flags", "var", "let",
    "pakfile", "if", "else", "elif", "while", "for", "switch", "case", "default", "out_of_range",
    "namespace", "break", "continue", "super", "operator", "attribute", "member", "member_frame",
    "static", "static_frame", "abstract", "union", "guard", "constructor", "assert", "nonlocal",
    "nil", "and", "or", "not",
]);
// Keywords written like a call, without a space before their "(": `for(xs) x:`.
const CALL_KEYWORDS = new Set(["for", "while", "fn"]);
// Keywords that start statements or are operators, unlike the rest (`int`, `resource`, `super`,
// ...), which the parser reads much like identifiers: whether a space follows them can matter.
const STATEMENT_KEYWORDS = new Set([
    "class", "def", "fn", "return", "is", "struct", "import", "from", "program", "private", "enum",
    "enum_flags", "var", "let", "pakfile", "if", "else", "elif", "while", "for", "switch", "case",
    "default", "namespace", "break", "continue", "operator", "attribute", "member",
    "member_frame", "static", "static_frame", "abstract", "guard", "assert", "nonlocal", "and",
    "or", "not",
]);
// Keywords that are values.
const VALUE_KEYWORDS = new Set(["nil", "super"]);
const DECLARATIONS = new Set(["let", "var", "member", "member_frame", "static", "static_frame"]);
// After which the lexer takes a line break to continue the line (it sets `cont`).
const CONTINUES = new Set([
    ",", "+", "-", "*", "/", "%", "<", ">", "=", "!", "&", "|", "^", "~", "+=", "-=", "*=", "/=",
    "%=", "&=", "|=", "^=", "<<=", ">>=", "->", "<=", ">=", "<<", ">>", "==", "!=", "::", "and",
    "or", "operator",
]);
const OPENERS = new Map([["(", ")"], ["[", "]"], ["{", "}"]]);
const CLOSERS = new Set([")", "]", "}"]);

// What a token is, where its text alone doesn't say.
type Role =
    | "unary" | "postfix"           // - ~ ++ --
    | "generic"                     // < > >> around type arguments
    | "lt"                          // a < written without spaces where it could be the above
    | "index" | "list" | "type"     // [ ]
    | "typecolon" | "inherit"       // a:int, class a : b
    | "block"                       // a :: that ends a line: `case T::`
    | "defname" | "opname";         // def name(, def operator+(

interface Analysis {
    code: Token[];
    role: Map<Token, Role>;
    match: Map<Token, Token>;
    index: Map<Token, number>;
    // How many brackets each code token is in.
    depth: number[];
    markType(i: number): number;
}

const isIdent = (t: Token) => t.kind == "ident" && !KEYWORDS.has(t.text);
// Whether a space before a following "(" (or "<") matters to the parser.
export const isName = (t: Token) => t.kind == "ident" && !STATEMENT_KEYWORDS.has(t.text);
// Whether this can start an argument to a call without (): `f x`, `f -1`.
const startsArgument = (t: Token) =>
    (t.kind != "op" && t.kind != "comment") || ["(", "[", "{", "-", "~", "++", "--"].includes(t.text);

// Whether a line break after `t` continues the line.
function continuing(role: Map<Token, Role>, t: Token): boolean {
    const r = role.get(t);
    return CONTINUES.has(t.text) && r != "generic" && r != "block";
}

// Whether `t` ends a value, which a following ( or [ calls or indexes.
function value(role: Map<Token, Role>, t: Token): boolean {
    const r = role.get(t);
    return t.kind == "number" || t.kind == "string" || isIdent(t) || VALUE_KEYWORDS.has(t.text) ||
        CLOSERS.has(t.text) || (r == "generic" && t.text != "<") || r == "postfix";
}

function analyze(src: string, tokens: Token[]): Analysis {
    const code = tokens.filter(t => t.kind != "comment");
    const role = new Map<Token, Role>();
    const match = new Map<Token, Token>();
    const index = new Map<Token, number>();
    const depth: number[] = [];
    const open: Token[] = [];
    code.forEach((t, i) => {
        index.set(t, i);
        depth.push(open.length);
        if (OPENERS.has(t.text)) {
            open.push(t);
        } else if (CLOSERS.has(t.text)) {
            const o = open.pop();
            if (!o || OPENERS.get(o.text) != t.text) throw new FormatError(`unmatched '${t.text}'`, t.start);
            match.set(o, t);
            match.set(t, o);
        }
    });
    if (open.length) throw new FormatError(`unmatched '${open[open.length - 1].text}'`, open[open.length - 1].start);

    const newlineBefore = (i: number) => i > 0 && i < code.length && src.slice(code[i - 1].end, code[i].start).includes("\n");
    const isValue = (t: Token | undefined) => !!t && value(role, t);
    const continues = (t: Token) => continuing(role, t);
    // Whether code[i] starts a statement on a line of its own.
    const startsStatement = (i: number) => i == 0 || (newlineBefore(i) && depth[i] == 0 && !continues(code[i - 1]));

    // Type arguments: `name<` with no space, then types, then a matching > not followed by
    // something an expression would continue with. Returns the index of the closing token.
    const genericEnd = new Map<number, number>();
    const scanGeneric = (i: number): number => {
        let level = 0;
        for (let j = i + 1; j < code.length; j++) {
            const t = code[j];
            if (t.text == "<") {
                if (j > i + 1 && (code[j - 1].kind != "ident" || t.ws > 0 || newlineBefore(j))) return -1;
                level++;
            } else if (t.text == ">" || t.text == ">>") {
                level -= t.text.length;
                if (level < 0) return -1;
                if (level == 0) {
                    const next = code[j + 1];
                    if (next && !newlineBefore(j + 1) && (next.kind == "ident" || next.kind == "number" || next.kind == "string")) return -1;
                    return j;
                }
            } else if (!(t.kind == "ident" || t.kind == "number" || [",", "[", "]", "?"].includes(t.text))) {
                return -1;
            }
        }
        return -1;
    };
    for (let i = 0; i + 1 < code.length; i++) {
        if (!isName(code[i]) || code[i + 1].text != "<" || code[i + 1].ws > 0 || newlineBefore(i + 1)) continue;
        const end = scanGeneric(i);
        if (end < 0) {
            role.set(code[i + 1], "lt");
            continue;
        }
        for (let j = i + 1; j <= end; j++) {
            const t = code[j];
            if (t.text == "<" || t.text == ">" || t.text == ">>") role.set(t, "generic");
            if (t.text == "[" || t.text == "]") role.set(t, "type");
            if (t.text == "<") genericEnd.set(j, scanGeneric(j - 1));
        }
        i = end;
    }

    // Marks the type starting at code[i], returning where it ends.
    const markType = (i: number): number => {
        const t = code[i];
        if (!t) return i;
        if (t.text == "[") {
            const close = match.get(t)!;
            role.set(t, "type");
            role.set(close, "type");
            markType(i + 1);
            i = index.get(close)! + 1;
        } else if (t.kind == "ident") {
            i++;
            if (genericEnd.has(i)) i = genericEnd.get(i)! + 1;
        } else {
            return i;
        }
        while (code[i]?.text == "?") i++;
        return i;
    };

    for (let i = 0; i < code.length; i++) {
        const t = code[i];
        const prev = startsStatement(i) ? undefined : code[i - 1];
        if (t.text == "~") {
            role.set(t, "unary");
        } else if (t.text == "-" && (!isValue(prev) ||
                   // `f -1`: an argument to a call without ().
                   (isIdent(prev!) && t.ws > 0 && !newlineBefore(i) && code[i + 1]?.ws == 0 && !newlineBefore(i + 1)))) {
            role.set(t, "unary");
        } else if (t.text == "++" || t.text == "--") {
            role.set(t, isValue(prev) ? "postfix" : "unary");
        } else if (t.text == "[" && !role.has(t)) {
            role.set(t, isValue(prev) && !(isIdent(prev!) && t.ws > 0 && !newlineBefore(i)) ? "index" : "list");
            role.set(match.get(t)!, role.get(t)!);
        } else if (t.text == "::" && newlineBefore(i + 1) && depth[i] == 0) {
            role.set(t, "block");
        }
    }

    for (let i = 0; i < code.length; i++) {
        const t = code[i];
        // Parameters: `def name(`, `def name<T>(`, `def operator+(`, `fn(`.
        let params = -1;
        if (t.text == "def" && code[i + 1]) {
            let j = i + 1;
            if (code[j].text == "operator") {
                // The operator's name: `operator+`, `operator[]`, `operator()`.
                for (j++; code[j] && (code[j].text != "(" || code[j + 1]?.text == ")" && code[j + 2]?.text == "("); j++)
                    role.set(code[j], "opname");
            } else {
                const name = code[j];
                j++;
                if (genericEnd.has(j)) j = genericEnd.get(j)! + 1;
                if (code[j]?.text == "(") role.set(name, "defname");
            }
            if (code[j]?.text == "(") params = j;
        } else if (t.text == "fn" && code[i + 1]?.text == "(") {
            params = i + 1;
        }
        if (params >= 0) {
            const close = index.get(match.get(code[params])!)!;
            for (let j = params + 1; j < close; j++) {
                if (depth[j] != depth[params] + 1 || !isIdent(code[j])) continue;
                if (code[j + 1]?.text == ":") {
                    role.set(code[j + 1], "typecolon");
                    markType(j + 2);
                }
            }
        }
        // Declarations, up to their "=": `let a:int, b:float = ...`.
        if (DECLARATIONS.has(t.text)) {
            for (let j = i + 1; j < code.length && depth[j] == depth[i] && !newlineBefore(j); j++) {
                if (code[j].text == "=") break;
                if (isIdent(code[j]) && code[j + 1]?.text == ":") {
                    role.set(code[j + 1], "typecolon");
                    markType(j + 2);
                }
            }
        }
        if (t.text == "->" || t.text == "is" || (t.text == "::" && role.get(t) != "block")) markType(i + 1);
        // `class a : b`, unlike the : that starts the body.
        if ((t.text == "class" || t.text == "struct") && (startsStatement(i) || ["private", "abstract"].includes(code[i - 1].text))) {
            for (let j = i + 1; j < code.length && !newlineBefore(j); j++) {
                if (code[j].text == ":" && depth[j] == depth[i] && code[j + 1] && !newlineBefore(j + 1)) {
                    role.set(code[j], "inherit");
                    break;
                }
            }
        }
    }
    return { code, role, match, index, depth, markType };
}


// Whether two tokens written together would lex as something else.
function merges(a: Token, b: Token): boolean {
    try {
        const t = lex(a.text + b.text);
        return t.length != 2 || t[0].text != a.text;
    } catch {
        return true;
    }
}

function spacing(an: Analysis, a: Token, b: Token): string {
    const ra = an.role.get(a), rb = an.role.get(b);
    if (b.text == ",") return "";
    if (a.text == ",") return " ";
    if (rb == "postfix" || ra == "unary" || ra == "opname" || a.text == "operator") return "";
    if ([".", "..", "::"].includes(a.text) || [".", "..", "::"].includes(b.text)) return "";
    if (a.text == "@" || b.text == "?") return "";
    if ((ra == "generic" && a.text == "<") || rb == "generic" || ra == "lt") return "";
    if (b.text == ":") return rb == "inherit" ? " " : "";
    if (a.text == ":") return ra == "typecolon" ? "" : " ";
    if (a.text == "(" || b.text == ")") return "";
    if (a.text == "[") return ra == "list" && b.text != "]" ? " " : "";
    if (b.text == "]") return rb == "list" && a.text != "[" ? " " : "";
    if (a.text == "{") return b.text == "}" ? "" : " ";
    if (b.text == "}") return " ";
    if (b.text == "(") return value(an.role, a) || CALL_KEYWORDS.has(a.text) ? "" : " ";
    if (b.text == "[") return rb == "index" ? "" : " ";
    return " ";
}

function separator(an: Analysis, a: Token, b: Token): string {
    if (a.kind == "comment" || b.kind == "comment") return " ";
    let s = spacing(an, a, b);
    if (isName(a) && an.role.get(a) != "defname") {
        // `f(x)` is a call with the argument list (x), `f (x)` one with the argument (x),
        // `f<T>` has type arguments, and `f x` is a call (but f isn't): keep what's there.
        if (b.text == "(" || b.text == "<") s = b.ws > 0 ? " " : "";
        else if (b.ws > 0 && startsArgument(b)) s = " ";
    }
    if (s == "" && merges(a, b)) s = " ";
    return s;
}

// A level of indentation, as the compiler's lexer keeps them (Lex::indentstack).
interface Level {
    orig: number;
    // Of the lines at this level, and that blocks in it are indented from.
    indent: number;
    children: number;
    // A line that continues the one before (after an operator) but is indented more.
    cont: boolean;
    // Of a class or struct, whose lines declare fields.
    fields: boolean;
}

interface Bracket {
    indent: number;
    hanging: boolean;
    align?: number;
}

// What a line is to the lexer: one that starts a statement, continues the line before after an
// operator, after a "." or inside brackets (whose indentation doesn't matter), or a comment.
type RowKind = "block" | "cont" | "dot" | "bracket" | "comment";

interface Row {
    tokens: Token[];
    orig: number;
    blanks: number;
    kind: RowKind;
}

function rows(src: string, tokens: Token[], an: Analysis): Row[] {
    const result: Row[] = [];
    let end = 0;
    for (const t of tokens) {
        const between = src.slice(end, t.start);
        const newlines = between.split("\n").length - 1;
        if (newlines || !result.length) {
            const lineStart = src.lastIndexOf("\n", t.start - 1) + 1;
            result.push({ tokens: [t], orig: t.start - lineStart, blanks: Math.max(0, newlines - 1), kind: "comment" });
        } else {
            result[result.length - 1].tokens.push(t);
        }
        end = t.end;
    }
    for (const row of result) {
        const first = row.tokens.find(t => t.kind != "comment");
        if (!first) continue;
        const i = an.index.get(first)!;
        row.kind = an.depth[i] > 0 ? "bracket"
            : first.text == "." ? "dot"
            : i > 0 && continuing(an.role, an.code[i - 1]) ? "cont"
            : "block";
    }
    return result;
}

const textOf = (t: Token) => t.text.startsWith("//") ? t.text.trimEnd() : t.text;

// Puts a line that starts a statement at `orig` on `levels`, as Lex::Next does. False if it's
// at no level.
function place(levels: Level[], orig: number, fields: boolean): boolean {
    const top = () => levels[levels.length - 1];
    for (;;) {
        const t = top();
        if (orig > t.orig) {
            const indent = t.children + INDENT;
            levels.push({ orig, indent, children: indent, cont: false, fields });
        } else if (orig < t.orig) {
            let cont = false;
            while (orig < top().orig) cont = levels.pop()!.cont;
            if (cont) continue;
            return orig == top().orig;
        }
        return true;
    }
}

function layout(src: string, tokens: Token[]): string {
    const an = analyze(src, tokens);
    const all = rows(src, tokens, an);
    const levels: Level[] = [{ orig: 0, indent: 0, children: 0, cont: false, fields: false }];
    const top = () => levels[levels.length - 1];
    const brackets: Bracket[] = [];
    const out: string[] = [];
    // The code and trailing comment of each line of `out` that has both, to align the comments.
    const trailing = new Map<number, [string, string]>();
    let prevCode: Token | undefined;
    let fields = false;  // Whether the last line at block level started a class or struct.
    let opensBlock = false;  // Whether the last line ended with the : of a block.
    let seenCode = false;
    // The indentation of the next line that starts a statement, or -1.
    const nextBlock = (r: number) => all.slice(r + 1).find(row => row.kind == "block")?.orig ?? -1;
    // What follows a continuation line at `orig`, deeper than the level `parent` it continues,
    // up to the line that dedents from it: lines at its level (a block in it continuing),
    // deeper ones (a block in it), and whether that line is deeper than `parent` (a block in
    // `parent`, unless that is a continuation whose blocks are indented from the line before).
    const following = (r: number, p: Level, orig: number) => {
        const parent = p.cont && p.children != p.indent ? Infinity : p.orig;
        let lands = false, nested = false, between = false;
        for (const row of all.slice(r + 1)) {
            if (row.kind != "block") continue;
            if (row.orig < orig) {
                between = row.orig > parent;
                break;
            }
            if (row.orig == orig) lands = true;
            else nested = true;
        }
        return { lands, nested, between };
    };
    const bracketIndent = (first: Token | undefined) => {
        const b = brackets[brackets.length - 1];
        return first && CLOSERS.has(first.text) ? b.indent
            : b.hanging || b.align === undefined ? b.indent + INDENT : b.align;
    };
    for (const [r, row] of all.entries()) {
        const first = row.tokens[0];
        const firstCode = row.tokens.find(t => t.kind != "comment");
        if (first.kind == "comment" && firstCode && !first.text.startsWith("#!"))
            throw new FormatError("code after a comment at the start of a line", firstCode.start);
        let indent: number;
        if (row.kind == "bracket") {
            indent = bracketIndent(firstCode);
        } else if (row.kind == "dot") {
            indent = top().indent + INDENT;
        } else if (row.kind == "cont") {
            // Indented once from the line it continues, or twice if a block follows that is
            // indented once (`if a and\n b:`). A block that starts in the continuation and
            // continues at its level (`let x =\n if a:\n ...\n else:`) is indented from it.
            const p = top();
            if (row.orig < p.orig)
                throw new FormatError("line continuation can't indent less than the previous line", first.start);
            if (row.orig > p.orig) {
                const { lands, nested, between } = following(r, p, row.orig);
                const own = lands || between;
                indent = p.indent + (between || (nested && !own) ? CONTINUATION : INDENT);
                levels.push({ orig: row.orig, indent, children: own ? indent : p.children, cont: true, fields: false });
            } else {
                indent = p.cont ? p.indent : p.indent + (nextBlock(r) > p.orig ? CONTINUATION : INDENT);
            }
        } else if (row.kind == "block") {
            if (!seenCode && row.orig) throw new FormatError("the first line of code is indented", first.start);
            if (!place(levels, row.orig, fields)) throw new FormatError("inconsistent dedent", first.start);
            indent = top().indent;
            const i = an.index.get(firstCode!)!;
            fields = ["class", "struct"].includes(firstCode!.text) ||
                (["private", "abstract"].includes(firstCode!.text) && ["class", "struct"].includes(an.code[i + 1]?.text));
            if (top().fields) {
                // Field declarations: `a:int`, `a, b:float = 0.0`.
                let j = i;
                while (isIdent(an.code[j]) && an.code[j + 1]?.text == ",") j += 2;
                if (isIdent(an.code[j]) && an.code[j + 1]?.text == ":" && an.role.get(an.code[j + 1]) === undefined) {
                    an.role.set(an.code[j + 1], "typecolon");
                    an.markType(j + 2);
                }
            }
        } else if (brackets.length) {
            indent = bracketIndent(undefined);
        } else if (prevCode && continuing(an.role, prevCode)) {
            indent = top().indent + INDENT;
        } else {
            // A comment on its own line goes with the code around it: at the deepest level
            // (of the one it's in, or the block the next line starts) not deeper than it was.
            const next = all.slice(r + 1).find(row => row.kind == "block");
            const candidates = levels.map(l => [l.orig, l.indent]);
            const after = [...levels];
            if (next && place(after, next.orig, false)) {
                const l = after[after.length - 1];
                candidates.push([l.orig, l.indent]);
                candidates.sort((a, b) => a[0] - b[0]);
            }
            indent = (candidates.filter(([orig]) => orig <= row.orig).pop() ?? [0, 0])[1];
        }

        let line = " ".repeat(indent);
        let prev: Token | undefined;
        let comment: [string, string] | undefined;
        let pendingAlign: Bracket | undefined;
        for (const [i, t] of row.tokens.entries()) {
            const sep = prev ? separator(an, prev, t) : "";
            if (t.kind == "comment" && t.text.startsWith("//") && prev) comment = [line, textOf(t)];
            line += comment && t.kind == "comment" ? COMMENT_GAP : sep;
            if (t.kind != "comment" && pendingAlign) {
                pendingAlign.align = line.length - (line.lastIndexOf("\n") + 1);
                pendingAlign = undefined;
            }
            line += textOf(t);
            if (t.kind != "comment") {
                if (OPENERS.has(t.text)) {
                    const hanging = !row.tokens.slice(i + 1).some(t => t.kind != "comment");
                    brackets.push({ indent, hanging });
                    if (!hanging) pendingAlign = brackets[brackets.length - 1];
                } else if (CLOSERS.has(t.text)) {
                    brackets.pop();
                }
                prevCode = t;
            }
            prev = t;
        }

        if (out.length && !opensBlock) for (let b = 0; b < Math.min(row.blanks, 1); b++) out.push("");
        if (comment && !comment[0].includes("\n")) trailing.set(out.length, comment);
        out.push(line);
        if (firstCode) {
            seenCode = true;
            opensBlock = !brackets.length && (prevCode!.text == ":" && an.role.get(prevCode!) === undefined ||
                an.role.get(prevCode!) == "block");
        }
    }

    // Trailing comments on consecutive lines line up, as gofmt does.
    for (let i = 0; i < out.length; i++) {
        if (!trailing.has(i)) continue;
        let j = i;
        while (trailing.has(j + 1)) j++;
        const width = Math.max(...Array.from({ length: j - i + 1 }, (_, k) => trailing.get(i + k)![0].length));
        for (let k = i; k <= j; k++) {
            const [code, comment] = trailing.get(k)!;
            out[k] = code.padEnd(width) + COMMENT_GAP + comment;
        }
        i = j;
    }
    return out.length ? out.join("\n") + "\n" : "";
}

// What the compiler's lexer makes of the indentation: where blocks start and end, following
// Lex::Next.
function structure(src: string, an: Analysis): string[] {
    const events: string[] = [];
    const stack: [number, boolean][] = [[0, false]];
    let depth = 0;
    an.code.forEach((t, i) => {
        const prev = an.code[i - 1];
        if (prev && depth == 0 && t.text != "." && src.slice(prev.end, t.start).includes("\n")) {
            const indent = t.start - (src.lastIndexOf("\n", t.start - 1) + 1);
            const top = () => stack[stack.length - 1][0];
            if (continuing(an.role, prev)) {
                if (indent < top()) throw new FormatError("line continuation can't indent less than the previous line", t.start);
                if (indent > top()) stack.push([indent, true]);
            } else {
                for (;;) {
                    if (indent > top()) {
                        stack.push([indent, false]);
                        events.push(`${i} indent`);
                    } else if (indent < top()) {
                        let iscont = false, dedent = false;
                        while (top() > indent) {
                            iscont = stack.pop()![1];
                            if (!iscont) events.push(`${i} dedent`);
                            dedent ||= !iscont;
                        }
                        // Then Lex::Next goes again, which after a dedent adds a line break
                        // before it, which the parser takes the same as none.
                        if (iscont) {
                            if (dedent && indent == top()) break;
                            continue;
                        }
                        if (indent != top()) throw new FormatError("inconsistent dedent", t.start);
                    } else {
                        events.push(`${i} line`);
                    }
                    break;
                }
            }
        }
        if (OPENERS.has(t.text)) depth++;
        if (CLOSERS.has(t.text)) depth--;
    });
    return events;
}

// Checks that `after` parses as `before` does: the same tokens, the same blocks, and the same
// whitespace where the parser looks at it.
function verify(before: string, after: string) {
    const fail = (what: string) => {
        throw new FormatError(`formatting would change ${what} (this is a formatter bug)`, 0);
    };
    const bt = lex(before), at = lex(after);
    if (bt.length != at.length || bt.some((t, i) => textOf(t) != textOf(at[i]))) fail("the tokens");
    const ba = analyze(before, bt), aa = analyze(after, at);
    ba.code.forEach((t, i) => {
        const prev = ba.code[i - 1];
        if (!prev || !isName(prev) || ba.role.get(prev) == "defname" || before.slice(prev.end, t.start).includes("\n")) return;
        const ws = aa.code[i].ws;
        if ((t.text == "(" || t.text == "<") ? (t.ws > 0) != (ws > 0) : t.ws > 0 && startsArgument(t) && !ws) fail("whitespace the parser uses");
    });
    if (structure(before, ba).join() != structure(after, aa).join()) fail("the indentation");
}

export function format(src: string): string {
    src = src.replace(/\r\n/g, "\n");
    const tokens = lex(src);
    // Refuses what the compiler would read differently, or not at all.
    structure(src, analyze(src, tokens));
    const result = layout(src, tokens);
    verify(src, result);
    return result;
}

// The 0-based line and column of an offset, for reporting a FormatError.
export function position(src: string, offset: number): { line: number; character: number } {
    const before = src.slice(0, offset).replace(/\r\n/g, "\n");
    const line = before.split("\n").length - 1;
    return { line, character: before.length - (before.lastIndexOf("\n") + 1) };
}
