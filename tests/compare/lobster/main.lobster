// Driver for the cross-language benchmarks, see ../SPEC.md.
//
//     bin/lobster tests/compare/lobster/main.lobster -- [repeats] [name]
//
// Runs each benchmark `repeats` times and reports the best time, so that allocator warmup and
// scheduling noise stay out of the result. Timing is in-process and excludes startup and all
// output.

import std
import .nbody
import .raytrace
import .interp
import .sortbench
import .astar

def benchf() -> float

class Bench:
    name:string
    run:benchf

let benches = [
    Bench { "nbody",    fn(): bench_nbody() },
    Bench { "raytrace", fn(): bench_raytrace() },
    Bench { "interp",   fn(): bench_interp() },
    Bench { "sort",     fn(): bench_sort() },
    Bench { "astar",    fn(): bench_astar() },
]

// A JIT run needs `--` to separate the program's arguments from the compiler's, and a `--cpp`
// build passes everything straight through, so the separator is dropped here rather than in
// the harness.
let args = filter(command_line_arguments()) a: a != "--"
var repeats = 5
var only = ""
if args.length > 0:
    let n, ok = string_to_int(args[0])
    if ok and n > 0: repeats = n
if args.length > 1:
    only = args[1]

for(benches) b:
    if only == "" or only == b.name:
        let run = b.run
        var best = 1.0e30
        var checksum = 0.0
        for(repeats):
            let start = seconds_elapsed()
            checksum = run()
            best = min(best, seconds_elapsed() - start)
        print "bench\t{b.name}\t{best}\t{checksum}"
