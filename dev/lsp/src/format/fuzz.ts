// Chaos tests for the formatter, over real code:
//
//     node out/format/fuzz.js [-seed N] [-rounds N] [-out DIR] path ...
//
// For each .lobster file (paths as for lobsterfmt), and each round:
// - scrambles the whitespace of its formatted text without changing what it means (spacing,
//   indentation width, tabs, blank lines, trailing spaces, CRLF), which must format back to
//   exactly that text;
// - breaks it (deletes, duplicates, joins and splits lines and tokens, shifts indentation),
//   which must either be refused or format to something that formats to itself.
// With -out, writes each broken file it formatted and the result to DIR as N.lobster and
// N.fmt.lobster (with the file's path in N.txt), for checking with the compiler that the two
// mean the same.

import * as fs from "fs";
import * as path from "path";
import { format, FormatError, isName } from "./format";
import { lex, Token } from "./lex";

let seed = 1, seed0 = 1;
// mulberry32
function random(): number {
    seed = (seed + 0x6d2b79f5) | 0;
    let t = Math.imul(seed ^ (seed >>> 15), 1 | seed);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
}
const pick = <T>(xs: T[]): T => xs[Math.floor(random() * xs.length)];
const int = (n: number) => Math.floor(random() * n);

function files(p: string): string[] {
    if (!fs.statSync(p).isDirectory()) return [p];
    return fs.readdirSync(p, { withFileTypes: true })
        .filter(e => e.isDirectory() ? !e.name.startsWith(".") : e.name.endsWith(".lobster"))
        .flatMap(e => files(path.join(p, e.name)));
}

// The same code with other whitespace, which formats the same.
function scramble(src: string): string {
    const tokens = lex(src);
    const factor = pick([1, 2, 3]);
    const tabs = random() < 0.2;
    const crlf = random() < 0.2;
    let out = "\n".repeat(int(3));
    let depth = 0;
    let prev: Token | undefined, prevCode: Token | undefined, beforePrev: Token | undefined;
    for (const t of tokens) {
        if (prev) {
            const between = src.slice(prev.end, t.start);
            const newlines = between.split("\n").length - 1;
            if (newlines) {
                if (random() < 0.3) out += " ".repeat(1 + int(3));
                out += "\n".repeat(newlines > 1 ? newlines + int(3) : 1);
                const orig = between.length - between.lastIndexOf("\n") - 1;
                // In brackets indentation doesn't matter, but whether there is any does.
                const indent = depth && orig ? 1 + int(12) : orig * factor;
                out += tabs && !depth ? "\t".repeat(indent) : " ".repeat(indent);
            } else if ((prev.kind != "comment" && isName(prev)) ||
                       (prev.text == "-" && beforePrev && isName(beforePrev))) {
                // Where the parser looks at the whitespace: `f(x)` and `f (x)`, `f -1`.
                out += between;
            } else {
                const n = int(4);
                out += n || mergesWith(prev, t) ? " ".repeat(Math.max(n, 1)) : "";
            }
        }
        out += t.text;
        if (t.kind != "comment") {
            if ("([{".includes(t.text)) depth++;
            if (")]}".includes(t.text)) depth--;
            beforePrev = prevCode;
            prevCode = t;
        }
        prev = t;
    }
    out += "\n".repeat(1 + int(3));
    return crlf ? out.replace(/\n/g, "\r\n") : out;
}

function mergesWith(a: Token, b: Token): boolean {
    if (a.kind == "comment" && a.text.startsWith("//")) return true;
    try {
        const t = lex(a.text + b.text);
        return t.length != 2 || t[0].text != a.text;
    } catch {
        return true;
    }
}

const SNIPPETS = ["(", ")", "[", "]", "{", "}", ":", ",", "-", "<", ">", "=", ".", "::", "if", "x",
    "1", '"s"', "//", "/*", "*/", '"""', "\t", "  ", "\n", "\n    ", "def f():", "fn x:", "->"];

// The code, broken somewhere.
function mutate(src: string): string {
    const lines = src.split("\n");
    const l = int(lines.length);
    switch (int(9)) {
        case 0: lines.splice(l, 1); break;
        case 1: lines.splice(l, 0, lines[l]); break;
        case 2: lines[l] = " ".repeat(1 + int(4)) + lines[l]; break;
        case 3: lines[l] = lines[l].replace(/^ {1,4}/, ""); break;
        case 4: if (l + 1 < lines.length) lines.splice(l, 2, lines[l] + " " + lines[l + 1].trimStart()); break;
        case 5: {
            const at = lines[l].lastIndexOf(" ", int(lines[l].length));
            if (at > 0) lines.splice(l, 1, lines[l].slice(0, at), " ".repeat(int(9)) + lines[l].slice(at + 1));
            break;
        }
        default: {
            const at = int(src.length);
            return random() < 0.5
                ? src.slice(0, at) + src.slice(at + 1 + int(3))
                : src.slice(0, at) + pick(SNIPPETS) + src.slice(at);
        }
    }
    return lines.join("\n");
}

function main(args: string[]): number {
    let rounds = 20, out: string | undefined;
    const paths: string[] = [];
    for (let i = 0; i < args.length; i++) {
        if (args[i] == "-seed") seed = seed0 = +args[++i];
        else if (args[i] == "-rounds") rounds = +args[++i];
        else if (args[i] == "-out") out = args[++i];
        else paths.push(args[i]);
    }
    let failures = 0, scrambled = 0, broken = 0, refused = 0, written = 0;
    const fail = (file: string, what: string, input: string) => {
        failures++;
        const name = `fuzz_failure_${seed0}_${failures}.lobster`;
        fs.writeFileSync(name, input);
        process.stdout.write(`${file}: ${what} (input in ${name})\n`);
    };
    for (const file of paths.flatMap(files)) {
        let canonical: string;
        try {
            canonical = format(fs.readFileSync(file, "utf8"));
        } catch (e) {
            if (!(e instanceof FormatError)) throw e;
            continue;
        }
        for (let r = 0; r < rounds; r++) {
            const s = scramble(canonical);
            scrambled++;
            try {
                if (format(s) != canonical) fail(file, "scrambled whitespace formats differently", s);
            } catch (e) {
                fail(file, `scrambled whitespace: ${e instanceof Error ? e.message : e}`, s);
            }
            let m = canonical;
            for (let k = 1 + int(3); k > 0; k--) m = mutate(m);
            broken++;
            let result: string;
            try {
                result = format(m);
            } catch (e) {
                if (e instanceof FormatError && !e.message.includes("formatter bug")) {
                    refused++;
                    continue;
                }
                fail(file, `broken code: ${e instanceof Error ? e.message : e}`, m);
                continue;
            }
            try {
                if (format(result) != result) fail(file, "formatting broken code twice changes it", m);
            } catch (e) {
                fail(file, `formatting broken code twice: ${e instanceof Error ? e.message : e}`, m);
            }
            if (out && result != m) {
                written++;
                fs.writeFileSync(path.join(out, `${written}.lobster`), m);
                fs.writeFileSync(path.join(out, `${written}.fmt.lobster`), result);
                fs.writeFileSync(path.join(out, `${written}.txt`), file);
            }
        }
    }
    process.stdout.write(`${scrambled} scrambled, ${broken} broken (${refused} refused), ${failures} failures\n`);
    return failures ? 1 : 0;
}

process.exitCode = main(process.argv.slice(2));
