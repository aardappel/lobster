
import vec

// Vectors (dynamic arrays): construction, indexing, and the vector builtins.

def vectors_tests():
    suite("vectors")

    // Construction and typed empty vectors.
    test_eq([ 1, 2, 3 ].length, 3)
    test_eq([].length, 0)
    test_eq([]::int.length, 0)
    test_seq([]::string, []::string)
    test_eq(length(42), 42)  // length on int is the identity.

    // Indexing, reading and writing, also nested and via int vectors.
    let vi = [ 10, 20, 30 ]
    test_eq(vi[0], 10)
    test_eq(vi[2], 30)
    vi[1] = 21
    test_eq(vi[1], 21)
    vi[1] += 1
    test_eq(vi[1], 22)
    let mat = [ [ 1, 2 ], [ 3, 4 ] ]
    test_eq(mat[1][0], 3)
    // Indexing with an int vector: last element indexes first (row-major).
    test_eq(mat[int2 { 0, 1 }], 3)
    test_eq(mat[int2 { 1, 0 }], 2)
    let cube = [ [ [ 123 ] ] ]
    test_eq(cube[int3 { 0, 0, 0 }], 123)
    cube[int3 { 0, 0, 0 }] = 124
    test_eq(cube[0][0][0], 124)

    // Iteration with element and index.
    var isum = 0
    for(vi) x, i:
        isum += x * (i + 1)
    test_eq(isum, 10 + 44 + 90)

    // push (returns the vector), pop, top, insert, remove.
    let pv = [ 1 ]
    pv.push(2).push(3)
    test_seq(pv, [ 1, 2, 3 ])
    test_eq(pv.top(), 3)
    test_eq(pv.pop(), 3)
    test_eq(pv.length, 2)
    pv.insert(0, 99)
    test_seq(pv, [ 99, 1, 2 ])
    pv.insert(3, 100)  // Insert at length appends.
    test_seq(pv, [ 99, 1, 2, 100 ])
    test_eq(pv.remove(0), 99)
    test_seq(pv, [ 1, 2, 100 ])
    pv.remove_range(1, 2)
    test_seq(pv, [ 1 ])
    // pop/top also work on vectors of structs.
    let sv = [ int2 { 1, 2 }, int2 { 3, 4 } ]
    test_eq(sv.top(), int2 { 3, 4 })
    test_eq(sv.pop(), int2 { 3, 4 })
    test_eq(sv.length, 1)
    // And on vectors of strings.
    let strv = [ "a" ]
    strv.push("b")
    test_eq(strv.pop(), "b")

    // remove_obj removes all elements equal to the argument.
    let ro = [ 1, 2, 1, 3, 1 ]
    remove_obj(ro, 1)
    test_seq(ro, [ 2, 3 ])
    // For reference elements, equality is by reference.
    let robj = [ 1 ]
    let rv = [ robj, [ 1 ], robj ]
    remove_obj(rv, robj)
    test_eq(rv.length, 1)

    // truncate.
    let tv = [ 1, 2, 3, 4 ]
    tv.truncate(2)
    test_seq(tv, [ 1, 2 ])
    tv.truncate(5)  // No-op beyond length.
    test_seq(tv, [ 1, 2 ])

    // append / append_into / vector_capacity.
    test_seq(append([ 1, 2 ], [ 3 ]), [ 1, 2, 3 ])
    test_seq(append([]::int, []::int), []::int)
    let ai = [ 1 ]
    test_seq(append_into(ai, [ 2, 3 ]), [ 1, 2, 3 ])
    test_seq(ai, [ 1, 2, 3 ])
    test_seq(vector_capacity([ 1 ], 100), [ 1 ])  // Capacity only, not length.

    // slice.
    test_seq([ 1, 2, 3, 4 ].slice(1, 2), [ 2, 3 ])
    test_seq([ 1, 2, 3, 4 ].slice(1, -1), [ 2, 3, 4 ])
    test_seq([ 1, 2, 3, 4 ].slice(4, -1), []::int)
    test_seq([ 1, 2, 3, 4 ].slice(0, 0), []::int)

    // any / all.
    test_eq(any([ 0, 0, 1 ]), true)
    test_eq(any([ 0, 0 ]), false)
    test_eq(any([]::int), false)
    test_eq(all([ 1, 2 ]), true)
    test_eq(all([ 1, 0 ]), false)
    test_eq(all([]::int), true)

    // equal: structural, recursing into nesting; == compares references.
    test_true(equal([ [ 1 ] ], [ [ 1 ] ]))
    test_true(not equal([ 1 ], [ 1, 2 ]))
    test_true(not equal([ 1 ], [ 2 ]))
    test_true(equal([ int2 { 1, 2 } ], [ int2 { 1, 2 } ]))
    test_true([ 1 ] != [ 1 ])

    // copy is shallow (nested references shared), deepcopy is not.
    let orig = [ [ 1 ], [ 2 ] ]
    let shallow = copy(orig)
    test_true(shallow != orig)
    shallow[0].push(9)
    test_seq(orig[0], [ 1, 9 ])
    let deep = deepcopy(orig, 10)
    deep[0].push(10)
    test_seq(orig[0], [ 1, 9 ])
    orig[0].pop()
    // deepcopy limited by depth: depth 1 behaves like copy.
    let dc1 = deepcopy(orig, 1)
    dc1[0].push(8)
    test_seq(orig[0], [ 1, 8 ])
    orig[0].pop()

    // min/max over int/float vectors.
    test_eq(min([ 3, 1, 2 ]), 1)
    test_eq(max([ 3, 1, 2 ]), 3)
    test_eq(min([ 3.5, 1.5 ]), 1.5)
    test_eq(max([ 3.5, 1.5 ]), 3.5)
    test_eq(min([ 3000000000000000000 ]), 3000000000000000000)
    test_eq(max([ -3000000000000000000 ]), -3000000000000000000)
    test_eq(max([ -5.0, -6.0 ]), -5.0)
    test_eq(min([ 1.0e100, 2.0e100 ]), 1.0e100)

    // binary_search: returns (number of matches, start index / insertion point).
    let bsv = [ 1, 3, 3, 5 ]
    var bn, bi = bsv.binary_search(3)
    test_eq(bn, 2)
    test_eq(bi, 1)
    bn, bi = bsv.binary_search(4)  // Not found: insertion point.
    test_eq(bn, 0)
    test_eq(bi, 3)
    bn, bi = bsv.binary_search(0)
    test_eq(bn, 0)
    test_eq(bi, 0)
    bn, bi = bsv.binary_search(9)
    test_eq(bn, 0)
    test_eq(bi, 4)
    bn, bi = [ 1.5, 2.5 ].binary_search(2.5)
    test_eq(bn, 1)
    test_eq(bi, 1)
    bn, bi = [ "a", "b", "b", "c" ].binary_search("b")
    test_eq(bn, 2)
    test_eq(bi, 1)
    // Object version compares by reference.
    let bobj = [ 5 ]
    bn, bi = binary_search_object([ bobj ], bobj)
    test_eq(bn, 1)
    test_eq(bi, 0)

vectors_tests()
