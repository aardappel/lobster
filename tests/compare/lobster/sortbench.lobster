// Quicksort driven by a caller-supplied comparison, see ../SPEC.md section 4. Every language
// runs this same algorithm, so what differs is the cost of calling the comparator.

import std
import .rnd

private let N = 400000
private let ROUNDS = 6

private struct Rec:
    key:float
    id:int

// `lt` has no declared type, so each call site gets its own specialization and the comparison
// ends up inline in the partition loop.
// The vector is a parameter rather than a free variable of `rec`: a variable a nested function
// reads from an enclosing scope lives in the VM's global frame, and is re-loaded from there on
// every access, where a parameter is an ordinary local.
private def sort_by(xs:[Rec], lt):
    def rec(a:[Rec], lo:int, hi:int) -> void:
        guard hi - lo > 1
        let pivot = a[lo]
        var sp = lo + 1
        var ep = hi
        while sp < ep:
            let c = a[sp]
            if lt(c, pivot):
                a[sp - 1] = a[sp]
                sp++
            else:
                a[sp] = a[--ep]
                a[ep] = c
        a[--sp] = pivot
        rec(a, lo, sp)
        rec(a, ep, hi)
    rec(xs, 0, xs.length)

def bench_sort() -> float:
    let rnd = Rnd { 777 }
    let src = []::Rec
    src.vector_capacity(N)
    for(N) i:
        src.push(Rec { rnd.range(-1000.0, 1000.0), i })
    var last = src
    for(ROUNDS):
        let xs = copy(src)
        sort_by(xs) a, b: a.key < b.key or (a.key == b.key and a.id < b.id)
        last = xs
    var prev = last[0]
    for(last) r, i:
        if i > 0 and (r.key < prev.key or (r.key == prev.key and r.id < prev.id)):
            print "# sort: not sorted at {i}"
            return 0.0
        prev = r
    var sum = 0.0
    for(last) r, i:
        sum += float((i % 97 + 1) * r.id)
    return sum
