import std

// Parser robustness under error recovery (see Parser::Error in parser.h): compiles
// mutations of the given programs with recovery on, to check that the parser survives
// (does not crash on, or loop forever on) whatever an error leaves behind. Every prefix of
// each file, cut at a line boundary and also in the middle of a line, models a program
// being typed into an editor, and single-line deletions model most other typos.
//
//   bin/lobster tests/parser_fuzz.lobster [files...]
//
// With no files given, runs on the test programs. Only reports; a crash or a hang is the
// failure this looks for, and the mutant that caused it is left in parser_fuzz_last.lobster.
// The mutants all end in a line that can't parse, such that none of them (the prefixes that
// happen to be complete programs) ever runs.

var num_mutants = 0
var num_no_errors = 0

def check(code:string):
    num_mutants++
    let mutant = code + "\nunknown_identifier_ending_the_mutant\n"
    write_file("parser_fuzz_last.lobster", mutant)
    let ret, err = compile_run_code(mutant, [], 100)
    if not err: num_no_errors++

def fuzz_file(fname:string):
    let src = read_file(fname)
    guard src:
        print "could not read {fname}"
        return
    let lines = tokenize(src, "\n", "")
    // Cap the amount of work per file: prefixes are quadratic in its length.
    let step = max(1, lines.length / 150)
    var i = 0
    while i < lines.length:
        let prefix = concat_string(slice(lines, 0, i + 1), "\n")
        check(prefix)
        // Cut in the middle of the last line as well.
        let last = lines[i]
        if last.length > 1:
            check(concat_string(slice(lines, 0, i), "\n") + "\n" +
                  substring(last, 0, last.length / 2))
        // And leave the line out entirely.
        check(concat_string(slice(lines, 0, i), "\n") + "\n" +
              concat_string(slice(lines, i + 1, lines.length - i - 1), "\n"))
        i += step
    print "{fname}: {lines.length} lines, {num_mutants} mutants so far"

var files = command_line_arguments()
if not files.length:
    files = [ "tests/parsertest.lobster", "tests/misctest.lobster", "tests/typetest.lobster",
              "tests/structtest.lobster", "tests/deep/control.lobster",
              "tests/deep/classes.lobster", "tests/deep/generics.lobster",
              "tests/deep/functions.lobster", "tests/deep/lexical.lobster" ]
for(files) f: fuzz_file(f)
print "{num_mutants} mutants, {num_no_errors} without errors (expected 0)"
