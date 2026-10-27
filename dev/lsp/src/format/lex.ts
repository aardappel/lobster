// A lexer for formatting: unlike the compiler's, it keeps comments and where each token is,
// and doesn't interpret anything (a string, with any interpolations, is one token).

export type Kind = "ident" | "number" | "string" | "comment" | "op";

export interface Token {
    kind: Kind;
    text: string;
    start: number;
    end: number;
    // Spaces and tabs since the end of the previous token that isn't a comment, which is what
    // the compiler's `whitespacebefore` counts (and decides some parses on).
    ws: number;
}

export class FormatError extends Error {
    constructor(message: string, public offset: number) {
        super(message);
    }
}

// Longest first, such that the first match is the token.
const OPS = [
    "<<=", ">>=",
    "++", "--", "+=", "-=", "->", "*=", "/=", "%=", "<=", "<<", "==", "!=", ">=", ">>", "&=",
    "|=", "^=", "::", "..",
    "+", "-", "*", "/", "%", "<", ">", "=", "!", "&", "|", "^", "~", "?", ":", ",", ".", "(",
    ")", "[", "]", "{", "}", "@",
];

const isDigit = (c: string) => c >= "0" && c <= "9";
const isXDigit = (c: string) => isDigit(c) || (c >= "a" && c <= "f") || (c >= "A" && c <= "F");
const isAlpha = (c: string) => (c >= "a" && c <= "z") || (c >= "A" && c <= "Z");
const isIdentStart = (c: string) => isAlpha(c) || c == "_" || c >= "\x80";
const isIdentCont = (c: string) => isIdentStart(c) || isDigit(c);

// Where the string or character constant starting at `i` ends.
function stringEnd(src: string, i: number): number {
    if (src.startsWith('"""', i)) {
        const end = src.indexOf('"""', i + 3);
        if (end < 0) throw new FormatError("end of file in multi-line string constant", i);
        return end + 3;
    }
    const quote = src[i];
    for (let j = i + 1; ; j++) {
        const c = src[j];
        if (c === undefined || c == "\n" || c == "\r")
            throw new FormatError("end of line in string constant", i);
        if (c == "\\") j++;
        else if (c == quote) return j + 1;
        else if (quote == '"' && c == "{") {
            if (src[j + 1] == "{") {
                j++;
                continue;
            }
            // An interpolated expression, up to the } that closes it.
            for (let depth = 0, k = j + 1; ; k++) {
                const d = src[k];
                if (d === undefined || d == "\n") throw new FormatError("unterminated {} in string constant", j);
                if (d == '"' || d == "'") {
                    k = stringEnd(src, k) - 1;
                } else if (d == "{") {
                    depth++;
                } else if (d == "}") {
                    if (depth-- == 0) {
                        j = k;
                        break;
                    }
                }
            }
        } else if (quote == '"' && c == "}" && src[j + 1] == "}") {
            j++;
        }
    }
}

// Where the number starting at `i` ends, as the compiler reads one.
function numberEnd(src: string, i: number): number {
    let p = i;
    if (src[p] == "0" && (src[p + 1] == "x" || src[p + 1] == "X")) {
        p += 2;
        while (isXDigit(src[p] ?? "")) p++;
        return p;
    }
    let isfloat = src[p] == ".";
    if (isfloat) p++;
    while (isDigit(src[p] ?? "")) p++;
    if (!isfloat && src[p] == "." && src[p + 1] != "." && !isAlpha(src[p + 1] ?? "")) {
        p++;
        isfloat = true;
        while (isDigit(src[p] ?? "")) p++;
    }
    if (isfloat && (src[p] == "e" || src[p] == "E")) {
        p++;
        if (src[p] == "+" || src[p] == "-") p++;
        while (isDigit(src[p] ?? "")) p++;
    }
    return p;
}

export function lex(src: string): Token[] {
    const tokens: Token[] = [];
    let ws = 0;
    let i = 0;
    const push = (kind: Kind, end: number) => {
        tokens.push({ kind, text: src.slice(i, end), start: i, end, ws: kind == "comment" ? 0 : ws });
        if (kind != "comment") ws = 0;
        i = end;
    };
    if (src.startsWith("#!")) {
        const eol = src.indexOf("\n");
        push("comment", eol < 0 ? src.length : eol);
    }
    while (i < src.length) {
        const c = src[i];
        if (c == " " || c == "\t" || c == "\r" || c == "\f") {
            ws++;
            i++;
        } else if (c == "\n") {
            i++;
        } else if (src.startsWith("//", i)) {
            const eol = src.indexOf("\n", i);
            push("comment", eol < 0 ? src.length : eol);
        } else if (src.startsWith("/*", i)) {
            const end = src.indexOf("*/", i + 2);
            if (end < 0) throw new FormatError("end of file in multi-line comment", i);
            push("comment", end + 2);
        } else if (c == '"' || c == "'") {
            push("string", stringEnd(src, i));
        } else if (isIdentStart(c)) {
            let end = i + 1;
            while (end < src.length && isIdentCont(src[end])) end++;
            push("ident", end);
        } else if (isDigit(c) || (c == "." && isDigit(src[i + 1] ?? ""))) {
            push("number", numberEnd(src, i));
        } else {
            const op = OPS.find(op => src.startsWith(op, i));
            if (!op) throw new FormatError(`illegal character ${JSON.stringify(c)}`, i);
            push("op", i + op.length);
        }
    }
    return tokens;
}
