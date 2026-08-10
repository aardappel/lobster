import std

import dictionary

// Misc builtins & features: namespaces, dictionary module, date/time,
// program info, and other odds and ends.

namespace MiscNS

// Namespaced declarations: referable as MiscNS.xxx from outside the
// namespace, or unqualified within.
class nsclass:
    x = 3
    def method(): return x + 1

def nsfun(): return (nsclass {}).method()

enum nsenum:
    ns_one = 1

namespace

def misc_tests():
    suite("misc")

    // Namespaces.
    test_eq(MiscNS.nsfun(), 4)
    test_eq((MiscNS.nsclass {}).x, 3)
    test_eq(MiscNS.ns_one, 1)

    // Dictionary module: scalar, string, struct and nilable values.
    let d1 = dictionary<string, int>(17)
    test_eq(d1.get("nope", -1), -1)
    d1.set("a", 1)
    d1.set("b", 2)
    d1.set("a", 3)  // Overwrite.
    test_eq(d1.get("a", -1), 3)
    test_eq(d1.get("b", -1), 2)
    let d2 = dictionary<int2, string>(7)
    d2.set(int2 { 1, 2 }, "v")
    test_eq(d2.get(int2 { 1, 2 }, ""), "v")
    test_eq(d2.get(int2 { 2, 1 }, ""), "")
    let d3 = dictionary<string, string?>(7)
    d3.set("k", "w")
    test_eq(assert d3.get("k", nil), "w")  // assert: compare as non-nil string.
    test_true(d3.get("nope", nil) == nil)
    let d4 = dictionary<float, float>(7)
    d4.set(1.5, 2.5)
    test_eq(d4.get(1.5, 0.0), 2.5)

    // date_time: vector of 9 ints, sane ranges (year is since 1900).
    let dt = date_time()
    test_eq(dt.length, 9)
    test_true(dt[0] > 1600000000)  // Unix epoch seconds, some time after 2020.
    test_true(dt[1] >= 120)        // Years since 1900.
    test_true(dt[2] >= 0 and dt[2] < 12)
    test_true(dt[3] >= 1 and dt[3] <= 31)
    test_true(dt[6] >= 0 and dt[6] < 24)
    // The UTC and local second counts must (essentially) agree.
    let dtu = date_time(true)
    test_true(abs(dtu[0] - dt[0]) <= 1)
    // Fixed-width string forms.
    test_eq(date_time_string().length, 24)
    test_eq(date_time_string(true).length, 24)
    // strftime-style formatting of an absolute time, in UTC.
    test_eq(format_time("%Y-%m-%d %H:%M:%S", 0, false), "1970-01-01 00:00:00")
    test_eq(format_time("%Y", 86400 * 365, false), "1971")
    test_true(date_time_build_info().length > 0)

    // Program info.
    test_true(program_name().find_string(".lobster") >= 0)
    test_eq(command_line_arguments().length, 0)
    test_eq(vm_compiled_mode(), false)  // This suite runs in JIT mode.
    // seconds_elapsed is monotonic.
    let t1 = seconds_elapsed()
    let t2 = seconds_elapsed()
    test_true(t1 >= 0.0 and t2 >= t1)
    // Thread info sanity (no actual workers started here).
    let hw, cores = thread_information()
    test_true(hw >= 1 and cores >= 1 and hw >= cores)
    test_eq(is_worker_thread(), false)
    test_eq(workers_alive(), false)

    // pass does nothing, usable as an empty body.
    if program_name() == "":
        pass()

    // reference_count: a fresh object owned by a single var reports 0 extra
    // references (compile-time refcounting borrows don't count); putting it
    // in a vector adds one.
    let rcv = [ 1, 2 ]
    test_eq(reference_count(rcv), 0)
    let holder = [ rcv ]
    test_eq(reference_count(rcv), 1)
    test_eq(holder.length, 1)

    // get_memory_usage returns a report string.
    test_true(get_memory_usage(3) is string)

    // File queries that don't touch the filesystem state.
    test_eq(exists_file("no_such_file_hopefully_12345.xyz"), false)
    // This suite's own source must be findable, when run from unittest.lobster.
    test_true(exists_file("deep/misc.lobster"))
    let src = read_file("deep/misc.lobster")
    test_true(src and src.find_string("misc_tests") >= 0)
    test_true(read_file("no_such_file_hopefully_12345.xyz") == nil)

    // string() is the identity on strings but converts everything else.
    test_eq(string(nil::string), "nil")
    test_eq(string(true), "true")
    test_eq(string(int3 { 1, 2, 3 }), "int3\{1, 2, 3\}")

misc_tests()
