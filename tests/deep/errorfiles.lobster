import std

// Runs the standalone programs in tests/errors/. Those are repros that are too
// big to inline as a string in errors.lobster, and until now they were only
// ever run by hand, so nobody noticed when a compiler change made one of them
// start (or stop) working.
//
// Every file in that directory must state what it expects, in a comment on any
// line:
//
//   // EXPECT-OK                    compiles and runs without an error
//   // EXPECT-ERROR: <substring>    fails, with an error containing <substring>
//
// A file that expects an error because of a compiler bug (rather than because
// the program is wrong) should say so in a FIXME comment next to the marker: if
// the bug gets fixed, this suite fails and the marker gets updated to EXPECT-OK.
//
// Files named `x._lobster` are not run: those are manual demos rather than
// tests (e.g. one that deliberately crashes to show off a stack trace).

def trim_ws(s:string) -> string:
    var start = 0
    var stop = s.length
    while start < stop and s[start] <= ' ': start++
    while stop > start and s[stop - 1] <= ' ': stop--
    return substring(s, start, stop - start)

// The text following `marker` up to the end of that line, or nil if absent.
def marker_value(src:string, marker:string) -> string?:
    let pos = src.find_string(marker)
    guard pos >= 0: return nil
    let rest = substring(src, pos + marker.length, -1)
    let line, _ = split_string(rest, "\n")
    return trim_ws(line)

def run_error_file(dir:string, fname:string):
    num_checks++
    let src = read_file(dir + "/" + fname)
    guard src:
        fail("could not read {fname}")
        return
    // These programs end in a statement rather than a value, which
    // compile_run_code requires, so give them one.
    let ret, err = compile_run_code(src + "\nreturn \"ok\"\n", [])
    let expect_error = marker_value(src, "// EXPECT-ERROR:")
    if expect_error:
        if not err:
            fail("{fname}: expected an error containing \"{expect_error}\", " +
                 "but it ran fine (returned: {ret})")
        elif err.find_string(expect_error) < 0:
            fail("{fname}: error did not contain \"{expect_error}\": {err}")
    elif marker_value(src, "// EXPECT-OK"):
        if err: fail("{fname}: expected no error, got: {err}")
    else:
        fail("{fname}: no EXPECT-OK or EXPECT-ERROR: marker")

def errorfile_tests():
    suite("errorfiles")
    // compile_run_code needs a compiler, which a --cpp build doesn't have.
    guard not vm_compiled_mode()
    // The dir this file is in relative to the main dir differs between an
    // unittest.lobster run and running deep/test.lobster directly.
    let dir = if read_file("deep/errorfiles.lobster"): "errors" else: "../errors"
    let fnames = scan_folder(dir, true)
    guard fnames:
        // Not reachable from a .pak file, where there is no source to scan.
        return
    for(fnames) fname:
        // "._lobster" also ends in ".lobster", so exclude it first.
        if fname.find_string("._lobster") < 0 and
           fname.find_string_reverse(".lobster") == fname.length - 8:
            run_error_file(dir, fname)

errorfile_tests()
