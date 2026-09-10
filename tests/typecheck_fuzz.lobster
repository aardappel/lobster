import std

// Declchecker/typechecker robustness under error recovery (see TypeChecker::Error in
// typecheck.h): compiles mutations of the given programs with recovery on, to check that
// typechecking survives (does not crash on, or loop forever on) whatever an error leaves
// behind. Unlike parser_fuzz.lobster, the mutations are meant to parse but not typecheck:
// an identifier replaced by another one from the same file, an int literal replaced by a
// string, a type name replaced by another, a line deleted.
//
//   bin/lobster tests/typecheck_fuzz.lobster [files...]
//
// With no files given, runs on the test programs. Only reports; a crash or a hang is the
// failure this looks for, and the mutant that caused it is left in typecheck_fuzz_last.lobster.
// The mutants all end in a line that can't typecheck, such that none of them (the ones that
// happen to be valid programs otherwise) ever runs. Errors that mean the recovery itself left
// the typechecker's bookkeeping inconsistent are counted separately, and expected not to occur.

var num_mutants = 0
var num_no_errors = 0
var num_parse_errors = 0
var num_inconsistent = 0

def check(code:string):
    num_mutants++
    let mutant = code + "\nlet typecheck_fuzz_end:int = \"end of mutant\"\n"
    write_file("typecheck_fuzz_last.lobster", mutant)
    let ret, err = compile_run_code(mutant, [], 100)
    if not err:
        num_no_errors++
    else:
        // The parser stops compilation, so these mutants don't exercise the typechecker.
        if err.find_string("expected, found") >= 0 or err.find_string("unknown identifier") >= 0:
            num_parse_errors++
        if err.find_string("still has") >= 0 or err.find_string("without being borrowed") >= 0 or
           err.find_string("INTERNAL") >= 0 or err.find_string("internal:") >= 0:
            num_inconsistent++
            print "inconsistent: {err}"

def is_ident_start(c:int): return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_'
def is_ident_cont(c:int): return is_ident_start(c) or (c >= '0' and c <= '9')

// The identifiers (as start, length pairs) in a line, skipping string constants and comments.
def identifiers(line:string) -> [int]:
    let r = []::int
    var i = 0
    while i < line.length:
        let c = line[i]
        if c == '\"':
            i++
            while i < line.length and line[i] != '\"': i++
        elif c == '/' and i + 1 < line.length and line[i + 1] == '/':
            return r
        elif is_ident_start(c):
            let start = i
            while i < line.length and is_ident_cont(line[i]): i++
            r.push(start)
            r.push(i - start)
            continue
        i++
    return r

def replace_at(line:string, start:int, len:int, with:string) -> string:
    return substring(line, 0, start) + with + substring(line, start + len, -1)

def fuzz_file(fname:string):
    let src = read_file(fname)
    guard src:
        print "could not read {fname}"
        return
    let lines = tokenize(src, "\n", "")
    // All identifiers of the file, to swap in for others.
    let all_idents = []::string
    for(lines) l:
        let ids = identifiers(l)
        for(ids.length / 2) i:
            let id = substring(l, ids[i * 2], ids[i * 2 + 1])
            if all_idents.find(fn(x): x == id) < 0: all_idents.push(id)
    let mutate = fn(li:int, replacement:string):
        check(concat_string(slice(lines, 0, li), "\n") + "\n" + replacement + "\n" +
              concat_string(slice(lines, li + 1, lines.length - li - 1), "\n"))
    // Cap the amount of work per file: each mutant typechecks the whole program.
    let step = max(1, lines.length / 100)
    var i = 0
    while i < lines.length:
        let line = lines[i]
        let ids = identifiers(line)
        if ids.length:
            // Replace one identifier by another one from the file, picked deterministically.
            let which = (i * 7) % (ids.length / 2)
            let start = ids[which * 2]
            let len = ids[which * 2 + 1]
            let other = all_idents[(i * 13 + which) % all_idents.length]
            mutate(i, replace_at(line, start, len, other))
            // And by a type name, which turns type annotations into other types.
            mutate(i, replace_at(line, start, len, if (i & 1): "string" else: "float"))
        // Replace the first int literal by a string.
        var j = 0
        while j < line.length and not (line[j] >= '0' and line[j] <= '9' and
                                      (j == 0 or not is_ident_cont(line[j - 1]))): j++
        if j < line.length:
            var k = j
            while k < line.length and line[k] >= '0' and line[k] <= '9': k++
            mutate(i, replace_at(line, j, k - j, "\"s\""))
        // And leave the line out entirely.
        mutate(i, "")
        i += step
    print "{fname}: {lines.length} lines, {num_mutants} mutants so far"

var files = command_line_arguments()
if not files.length:
    files = [ "tests/misctest.lobster", "tests/typetest.lobster", "tests/structtest.lobster",
              "tests/lifetimetest.lobster", "tests/deep/control.lobster",
              "tests/deep/classes.lobster", "tests/deep/generics.lobster",
              "tests/deep/functions.lobster", "tests/deep/overloads.lobster",
              "tests/deep/nilable.lobster", "tests/deep/lifetime.lobster",
              "tests/deep/structs.lobster", "tests/deep/enums.lobster" ]
for(files) f: fuzz_file(f)
print "{num_mutants} mutants, {num_no_errors} without errors (expected 0), " +
      "{num_parse_errors} with parse errors, {num_inconsistent} inconsistent (expected 0)"
