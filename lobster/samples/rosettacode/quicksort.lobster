include "std.lobster"

def quicksort(xs, lt):
    if(xs.length <= 1):
        xs
    else:
        pivot := xs[0]
        tail := xs.slice(1, -1)
        f1 := tail.filter():  lt(_, pivot)
        f2 := tail.filter(): !lt(_, pivot)
        append(append(quicksort(f1, lt), [ pivot ]),
                      quicksort(f2, lt))

print([ 3, 9, 5, 4, 1, 3, 9, 5, 4, 1 ].quicksort(): _a < _b)
