import * as assert from "assert";
import * as fs from "fs";
import * as path from "path";
import { format, FormatError } from "./format";

const cases: [string, string, string][] = [
    ["spacing",
        "let x=[1,2 ,3]\nlet y = x[ 1 ]+ - x [0]*2\nf (y)\nf(y)\nlet z = T{1,2}\nlet e = []\nlet d = {}\n",
        "let x = [ 1, 2, 3 ]\nlet y = x[1] + -x [ 0 ] * 2\nf (y)\nf(y)\nlet z = T { 1, 2 }\nlet e = []\nlet d = {}\n"],
    ["types",
        "def f( a : int, b:[ int ] ,c::T )->int?:\n    return a\nvar v :float = 1.0\nlet m = dictionary<string,int>()\n",
        "def f(a:int, b:[int], c::T) -> int?:\n    return a\nvar v:float = 1.0\nlet m = dictionary<string, int>()\n"],
    ["blocks",
        "def f(a):\n\tif a:\n\t\treturn 1\n\telse:\n\t\treturn 2\n\n\n\nwhile x<3 :x++\n",
        "def f(a):\n    if a:\n        return 1\n    else:\n        return 2\n\nwhile x<3: x++\n"],
    ["continuation",
        "if a and\n   b:\n    c\nlet w =\n  if a:\n    1\n  else:\n    2\nlet s = a +\nb\n",
        "if a and\n        b:\n    c\nlet w =\n    if a:\n        1\n    else:\n        2\nlet s = a +\n    b\n"],
    ["brackets",
        "f(a,\nb)\nlet xs = [\n1,\n2\n]\n",
        "f(a,\n  b)\nlet xs = [\n    1,\n    2\n]\n"],
    ["comments",
        "x = 1 // one\nyy = 2   // two\n\n// own line\ndef f():\n\n    // in f\n    g()\n",
        "x = 1   // one\nyy = 2  // two\n\n// own line\ndef f():\n    // in f\n    g()\n"],
    ["classes",
        "class A : B\n  x : int = 1\n  y, z:float\n  def m(): x\nstruct V<T>:\n  a:T\n",
        "class A : B\n    x:int = 1\n    y, z:float\n    def m(): x\nstruct V<T>:\n    a:T\n"],
    ["unary",
        "a = - 1\nf -1\nb = a-1\nc = not  a\nd = ~a\ni ++\n",
        "a = -1\nf -1\nb = a - 1\nc = not a\nd = ~a\ni ++\n"],
    ["keyword types", "var t = []::resource<texture>\n", "var t = []::resource<texture>\n"],
    ["crlf", "a = 1\r\nb = 2\r\n", "a = 1\nb = 2\n"],
    ["empty", "", ""],
];
for (const [name, input, expected] of cases) {
    assert.strictEqual(format(input), expected, name);
    assert.strictEqual(format(expected), expected, name + ", again");
}

assert.throws(() => format("if a:\n    b\n  c\n"), FormatError, "inconsistent dedent");
assert.throws(() => format('a = "b\n'), FormatError, "unterminated string");
assert.throws(() => format("f(a\n"), FormatError, "unmatched bracket");

// Found by fuzz.ts: each formats, the same again.
const found = [
    // A block in a continuation line and one in the line it continues.
    "struct Ray:\n    let temp =\n            if rnd_float() < P:\n                radiance(Ray { x, tdir }, depth) * TP\n        else:\n",
    "class resources:\n    let goap_path =\n           astar_goap(goapactions) state:\n            state.hungry\n        fn s:\n",
    "def w() -> int:\n                if a and\n                        b:\n                        c:\n                    far_cell.push(u_home[i])\nx\n",
    // A continuation of a continuation.
    "def f():\n                if a and\n                  b and\n                        c:\n                    let diag = n.pdir + d\n",
    "let path =\n    f(a) b:\n        c\n\nd\n",
    "def bump(): y++::",
    "def text_pa <ss() -> int:\n    1\n",
    "f(x\n [0])\n",
    "def inside(v): return all(v\n< lssize - 1)  // c\n",
    "f(x\n-1)\n",
    "/lamp,\n// c\n\ndef f(f):\n    // a\n    g()\n",
];
for (const input of found) {
    const once = format(input);
    assert.strictEqual(format(once), once, input);
}

// Lobster's own code: all of it formats, the same again.
let files = 0;
const walk = (dir: string) => {
    for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
        const p = path.join(dir, e.name);
        if (e.isDirectory()) walk(p);
        else if (e.name.endsWith(".lobster")) {
            const once = format(fs.readFileSync(p, "utf8"));
            assert.strictEqual(format(once), once, p);
            files++;
        }
    }
};
for (const dir of ["modules", "samples", "tests"]) walk(path.join("..", "..", dir));
console.log(`format: ${cases.length} cases, ${found.length} found by fuzzing, ${files} files`);
