import std

// FIXME why is the `0` literal typed
def sumf(xs): return fold(xs, 0.0): _x + _y

def timeit(budget, limit, body):
    var times = []::float
    while times.length() < limit and times.sumf() < budget:
        let n = if times.length() == 0:
            1
        else:
            let total = times.sumf()
            let avg = total / times.length()
            let rem = budget - total
            min(limit, max(1, int(rem / 2 / avg)))
        let more = map(n):
            let t0 = seconds_elapsed()
            body()
            let t1 = seconds_elapsed()
            t1 - t0
        times = append(times, more)
    return times from timeit

for([10, 100, 200, 400, 1000]) N:
    var xs = map(N): _
    let times = timeit(1.0, 100000):
        var ys = copy(xs)
        ys.randomize()
        ys.qsort_in_place(): _a < _b
        assert equal(xs, ys)
    let total = times.sumf()
    let avg = total / times.length()
    print "N:" + N + "\tops:" + times.length() + "\ts/op:" + avg
