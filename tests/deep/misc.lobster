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
    // add uses += on the existing value (or inserts).
    d1.add("a", 10)
    d1.add("c", 1)
    test_eq(d1.get("a", -1), 13)
    test_eq(d1.get("c", -1), 1)
    // get_or_set keeps an existing value.
    test_eq(d1.get_or_set("b", 99), 2)
    test_eq(d1.get_or_set("d", 99), 99)
    test_eq(d1.get("d", -1), 99)
    // get_keys/get_values are consistent with each other.
    let keys = d1.get_keys()
    let values = d1.get_values()
    test_eq(keys.length, 4)
    test_eq(values.length, 4)
    for(keys) k, i:
        test_eq(d1.get(k, -1), values[i])
    // forkv visits every pair.
    var kvsum = 0
    var kvkeylen = 0
    d1.forkv() k, val:
        kvsum += val
        kvkeylen += k.length
    test_eq(kvsum, 13 + 2 + 1 + 99)
    test_eq(kvkeylen, 4)
    // del removes just the given key, clear removes everything.
    d1.del("a")
    test_eq(d1.get("a", -1), -1)
    test_eq(d1.get_keys().length, 3)
    d1.clear()
    test_eq(d1.get_keys().length, 0)
    let d2 = dictionary<int2, string>(7)
    d2.set(int2 { 1, 2 }, "v")
    test_eq(d2.get(int2 { 1, 2 }, ""), "v")
    test_eq(d2.get(int2 { 2, 1 }, ""), "")
    let d3 = dictionary<string, string?>(7)
    d3.set("k", "w")
    test_eq(assert d3.get("k", nil), "w")  // assert: compare as non-nil string.
    test_true(d3.get("nope", nil) == nil)
    // With a reference value type, get without default returns nil.
    let d3r = dictionary<int, string>(7)
    d3r.set(1, "x")
    test_eq(assert d3r.get(1), "x")
    test_true(d3r.get(2) == nil)
    let d4 = dictionary<float, float>(7)
    d4.set(1.5, 2.5)
    test_eq(d4.get(1.5, 0.0), 2.5)
    // A dictionary with a single bucket forces all keys into one chain.
    let d1b = dictionary<int, int>(1)
    for(5) i: d1b.set(i, i * 10)
    for(5) i: test_eq(d1b.get(i, -1), i * 10)
    d1b.del(2)
    test_eq(d1b.get(2, -1), -1)
    test_eq(d1b.get_keys().length, 4)
    // get_or_create constructs a default value on a miss.
    let d5 = dictionary<string, MiscNS.nsclass>(7)
    test_eq(d5.get_or_create("k").x, 3)
    d5.get_or_create("k").x = 4
    test_eq(d5.get_or_create("k").x, 4)
    // Serialization to flexbuffers and back.
    let d1s = dictionary<string, int>(7)
    d1s.set("a", 1)
    d1s.set("b", 2)
    let dflex = d1s.serialize_to_flexbuffer()
    let d1r = dictionary<string, int>(7)
    test_eq(d1r.deserialize_from_flexbuffer(dflex), nil)
    test_eq(d1r.get("a", -1), 1)
    test_eq(d1r.get("b", -1), 2)

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
    // strftime-style formatting of the current time / an absolute time.
    test_eq(date_time_string_format("%Y", true).length, 4)
    test_eq(date_time_string_format("%Y", true), format_time("%Y", dt[0], false))
    test_eq(format_time("%Y-%m-%d %H:%M:%S", 0, false), "1970-01-01 00:00:00")
    test_eq(format_time("%Y", 86400 * 365, false), "1971")
    test_eq(format_time("%H:%M:%S", 3661, false), "01:01:01")
    test_eq(format_time("%j", 86400 * 31, false), "032")  // Day of year.
    test_true(date_time_build_info().length > 0)

    // Program info.
    // In compiled (Lobster -> C++) mode the program is a native executable
    // which may be run from a dir where the .lobster sources are not
    // reachable, so source related checks here and below are JIT-only.
    let compiled = vm_compiled_mode()
    if not compiled:
        test_true(program_name().find_string(".lobster") >= 0)
    test_eq(command_line_arguments().length, 0)
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
    // The dir this file is in relative to the main dir differs between an
    // unittest.lobster run and running deep/test.lobster directly.
    let deep_prefix = if read_file("deep/misc.lobster"): "deep/" else: ""
    let src = read_file(deep_prefix + "misc.lobster")
    if not compiled:
        // This suite's own source must be findable when running from source.
        test_true(exists_file("deep/misc.lobster") or exists_file("misc.lobster"))
        test_true(src and src.find_string("misc_tests") >= 0)
    test_true(read_file("no_such_file_hopefully_12345.xyz") == nil)
    // scan_folder returns parallel name/size vectors (rel=true searches
    // relative to the import dirs, like other file functions). Only scan if
    // src above established that the dir holding this file is reachable.
    if src:
        let fnames, fsizes = scan_folder(if deep_prefix.length: "deep" else: ".", true)
        test_true(fnames and fsizes, "scan_folder deep failed")
        if fnames and fsizes:
            test_eq(fnames.length, fsizes.length)
            let self_idx = fnames.find(): _ == "misc.lobster"
            test_true(self_idx >= 0, "scan_folder did not find misc.lobster")
            if self_idx >= 0:
                test_true(fsizes[self_idx] > 0)
    test_true(scan_folder("no_such_dir_hopefully_12345", true) == nil)
    // Writing, reading back and deleting a file (in the main file's dir).
    let tmpname = "deep_tmp_test_file.tmp"
    test_true(write_file(tmpname, "contents\n123"), "write_file failed")
    test_eq(read_file(tmpname), "contents\n123")
    test_true(exists_file(tmpname))
    let tmpname2 = "deep_tmp_test_file2.tmp"
    test_true(rename_file(tmpname, tmpname2), "rename_file failed")
    test_eq(read_file(tmpname2), "contents\n123")
    test_true(delete_file(tmpname2))
    test_eq(exists_file(tmpname), false)

    // string() is the identity on strings but converts everything else.
    test_eq(string(nil::string), "nil")
    test_eq(string(true), "true")
    test_eq(string(int3 { 1, 2, 3 }), "int3\{1, 2, 3\}")

    // parse_data works with namespaced type names.
    let nsv, nserr = parse_data(typeof MiscNS.nsclass, "MiscNS.nsclass \{\}")
    test_eq(nserr, nil)
    test_eq((assert nsv).x, 3)

misc_tests()
