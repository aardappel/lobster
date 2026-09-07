// Runs all three benchmarks in one program, weighted the way the corpus they were measured
// against is weighted: synthetic1 about half, synthetic2 about a third, speedtest about a fifth.
//
//     bin/lobster tests/bench/all.lobster
//
// It prints the seconds each of the three took, their share of the total, and the total, which
// is the number to compare between builds on the same machine with the same backend. Each of
// them still runs on its own, unchanged, and this only decides how much of each runs, see
// tests/bench/aggregate.lobster.
//
// WEIGHTS
// The amounts below are in the unit each benchmark counts in: rounds for synthetic1, the work
// scale for synthetic2, and whole passes over the collected tests for speedtest. They were
// picked so that the three take about 50/30/20 percent of the time, measured on the libtcc JIT
// on 2026-09-07: 0.73, 0.45 and 0.26 seconds. That split is a property of a build, not of the
// source: a backend or an optimization that helps one of the three more than the others moves
// it, which is worth knowing but is not a reason to retune. Retune only when the intended
// weighting is meant to change, and say so when comparing across a retune.

import from "../"
import bench.aggregate

bench_aggregate()

import bench.synthetic1
import bench.synthetic2
import speedtest

def units_for(name:string) -> int:
    switch name:
        case "synthetic1": return 3000
        case "synthetic2": return 10
        case "speedtest": return 2
        default: return 1

let meta_runs = 3
let best = map(bench_works.length): 999999.0
var checksum = 0
for(meta_runs) m:
    for(bench_works) w, i:
        let run = w.run
        let start = seconds_elapsed()
        let r = run(units_for(w.name))
        let taken = seconds_elapsed() - start
        best[i] = min(best[i], taken)
        if m == 0: checksum = (checksum * 31 + r) & 0xFFFFFFFFFFFF

var total = 0.0
for(best) t: total += t
for(bench_works) w, i:
    print "{w.name}: {best[i]} s, {best[i] / total * 100.0}%"
print "TOTAL: {total}"
print "checksum: {checksum}"
