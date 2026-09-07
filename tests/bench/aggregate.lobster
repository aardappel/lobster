// The hook that lets one program weigh several benchmarks against each other, see
// tests/bench/all.lobster. A benchmark run on its own leaves `aggregated` false and drives
// itself the way it always has. An aggregator sets it before importing the benchmark, which
// then only registers the work it can do, in whatever unit that benchmark counts in, and the
// aggregator decides how much of it to run and times it.

def workf(units:int) -> int

class bench_work:
    name:string
    run:workf

let bench_works = []
var aggregated = false

// Called by the aggregator before it imports the benchmarks.
def bench_aggregate():
    aggregated = true

def bench_register(name:string, f:workf):
    bench_works.push(bench_work { name, f })
