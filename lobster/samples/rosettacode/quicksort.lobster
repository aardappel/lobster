include "std.lobster"

def quicksort(xs, lt):
    if xs.length <= 1:
        xs
    else:
        pivot := xs[0]
        tail := xs.slice(1, -1)
        f1 := filter tail:  lt(_, pivot)
        f2 := filter tail: !lt(_, pivot)
        append(append(quicksort(f1, lt), [ pivot ]),
                      quicksort(f2, lt))

sorted := [ 3, 9, 5, 4, 1, 3, 9, 5, 4, 1 ].quicksort(): _a < _b
print sorted
