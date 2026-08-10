import std


// The std module: higher order functions and other helpers on top of the
// builtins.

def std_tests():
    suite("std")

    let v = [ 3, 1, 4, 1, 5 ]

    // map / map2 / zip.
    test_seq(v.map(): _ * 2, [ 6, 2, 8, 2, 10 ])
    test_seq(map(3) i: i * i, [ 0, 1, 4 ])
    test_seq(map2([ 1, 2 ], [ 10, 20 ]): _a + _b, [ 11, 22 ])
    test_seq(zip([ 1, 2 ], [ 3, 4 ]), [ [ 1, 3 ], [ 2, 4 ] ])

    // filter and friends.
    test_seq(v.filter(): _ > 2, [ 3, 4, 5 ])
    test_seq(v.filter_indices(): _ > 2, [ 0, 2, 4 ])
    test_seq(v.filter_map(): if _ > 2: _ * 10 else: 0, [ 30, 40, 50 ])
    let pt, pf = v.partition(): _ > 2
    test_seq(pt, [ 3, 4, 5 ])
    test_seq(pf, [ 1, 1 ])
    let pi, pj = v.partition_indices(): _ > 2
    test_seq(pi, [ 0, 2, 4 ])
    test_seq(pj, [ 1, 3 ])
    test_seq(split([ 1, 2, 3, 4 ]): _ % 2, [ [ 2, 4 ], [ 1, 3 ] ])

    // exists / find / find_or / find_or_push / find_best / reverse_find.
    test_true(v.exists(): _ == 4)
    test_true(not v.exists(): _ == 9)
    test_eq(v.find(): _ == 4, 2)
    test_eq(v.find(): _ == 9, -1)
    var fe, fi = v.find_or(-1): _ == 5
    test_eq(fe, 5)
    test_eq(fi, 4)
    fe, fi = v.find_or(-1): _ == 9
    test_eq(fe, -1)
    test_eq(fi, -1)
    let fp = [ 1, 2 ]
    var pe, pidx = fp.find_or_push(9): _ == 2
    test_eq(pe, 2)
    test_eq(pidx, 1)
    pe, pidx = fp.find_or_push(9): _ == 9
    test_eq(pe, 9)
    test_eq(pidx, 2)
    test_seq(fp, [ 1, 2, 9 ])
    test_eq(v.find_best<int>() x, i: x * 10, 4)  // Index of the largest value.
    test_eq(v.reverse_find(): _ == 1, 3)

    // fold / fold2 / reduce / reduce_reverse / sum / product.
    test_eq(v.fold(0): _a + _b, 14)
    test_eq(fold([ "a", "b" ], "x"): _a + _b, "xab")
    let f1, f2 = fold2([ 1, 2, 3 ], 0, 1) a1, a2, x: (a1 + x, a2 * x)
    test_eq(f1, 6)
    test_eq(f2, 6)
    test_eq(reduce([ 1, 2, 3 ]): _a - _b, -4)
    test_eq(reduce_reverse([ 1, 2, 3 ]): _a - _b, 2)  // 1 - (2 - 3)
    test_eq(sum(v), 14)
    test_eq(sum([ 0.5, 1.5 ]), 2.0)
    test_eq(product([ 2, 3, 4 ]), 24)
    test_eq(sum([]::int), 0)
    test_eq(product([]::int), 1)

    // connect: pairs of adjacent elements. Anonymous args bind in order of
    // appearance in the body, so use named args here.
    test_seq(connect([ 1, 2, 4 ]) a, b: b - a, [ 1, 2 ])
    test_seq(connect([ 7 ]) a, b: b - a, []::int)

    // reverse family.
    test_seq(reverse_list([ 1, 2, 3 ]), [ 3, 2, 1 ])
    test_seq(reverse_map([ 1, 2, 3 ]) x, i: x * 10, [ 30, 20, 10 ])
    test_eq(reverse_fold([ "a", "b", "c" ], ""): _a + _b, "cba")
    var rev = ""
    reverse([ "a", "b" ]) x, i: rev += x + i
    test_eq(rev, "b1a0")

    // flatten.
    test_seq(flatten([ [ 1, 2 ], [], [ 3 ] ]), [ 1, 2, 3 ])

    // Sorting, all three, and insert_ordered / binary search interplay.
    let unsorted = [ 3, 9, 5, 4, 1, 3 ]
    let s1 = unsorted.qsort(): _a < _b
    test_seq(s1, [ 1, 3, 3, 4, 5, 9 ])
    test_seq(unsorted, [ 3, 9, 5, 4, 1, 3 ])  // qsort does not mutate.
    let s2 = copy(unsorted)
    s2.qsort_in_place(): _a < _b
    test_seq(s2, s1)
    let s3 = copy(unsorted)
    s3.insertion_sort(): _a < _b
    test_seq(s3, s1)
    let so = [ 1, 3, 5 ]
    so.insert_ordered(4): _a < _b
    test_seq(so, [ 1, 3, 4, 5 ])
    so.insert_ordered(9): _a < _b
    test_seq(so, [ 1, 3, 4, 5, 9 ])
    // Sorting strings and by descending order.
    test_seq([ "b", "a", "c" ].qsort(): _a < _b, [ "a", "b", "c" ])
    test_seq([ 1, 3, 2 ].qsort(): _a > _b, [ 3, 2, 1 ])

    // shuffle: seeded, must remain a permutation.
    rnd_seed(7)
    let sh = copy(unsorted)
    sh.shuffle()
    test_eq(sh.length, unsorted.length)
    test_seq(sh.qsort(): _a < _b, s1)

    // weighted_pick: with a single nonzero weight it must pick that element.
    rnd_seed(1)
    let wpe, wpi = weighted_pick<int, int>([ 10, 20, 30 ]) x: if x == 20: 1 else: 0
    test_eq(wpe, 20)
    test_eq(wpi, 1)

    // rnd helpers.
    rnd_seed(3)
    let picked = rnd_pick([ 5, 6, 7 ])
    test_true(picked >= 5 and picked <= 7)
    let consumable = [ 1, 2, 3 ]
    let eaten = consumable.rnd_consume()
    test_eq(consumable.length, 2)
    test_true(eaten >= 1 and eaten <= 3)

    // FIXME: (docs) the language reference mentions "a similar function in
    // module std called collectwhile", but no such function exists in std (or
    // anywhere else).

    // collect / do / return_after.
    let collected = collect() emit:
        for(3) i: emit(i)
    test_seq(collected, [ 0, 1, 2 ])
    var doside = 0
    do():
        doside = 1
    test_eq(doside, 1)
    test_eq(return_after(42): doside = 2, 42)
    test_eq(doside, 2)

    // for_range and friends.
    var racc = 0
    for_range(2, 5) i: racc += i
    test_eq(racc, 9)
    racc = 0
    for_range_incl(2, 5) i: racc += i
    test_eq(racc, 14)
    racc = 0
    for_bias(3, 10) i: racc += i
    test_eq(racc, 33)
    racc = 0
    for_scale(3, 2) i: racc += i
    test_eq(racc, 6)

    // nest_if: conditionally wrap a block in another block.
    var trace = ""
    nest_if(true) f:
        trace += "["
        f()
        trace += "]"
    fn:
        trace += "x"
    test_eq(trace, "[x]")
    trace = ""
    nest_if(false) f:
        trace += "["
        f()
        trace += "]"
    fn:
        trace += "x"
    test_eq(trace, "x")

std_tests()
