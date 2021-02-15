import std

def quicksort<T>(xs:[T], lt) -> [T]:
    if xs.length <= 1:
        return xs
    else:
        let pivot = xs[0]
        let tail = xs.slice(1, -1)
        let f1 = filter tail:  lt(_, pivot)
        let f2 = filter tail: not lt(_, pivot)
        return append(append(quicksort(f1, lt), [ pivot ]),
                             quicksort(f2, lt))

let sorted = [ 3, 9, 5, 4, 1, 3, 9, 5, 4, 1 ].quicksort(): _a < _b
print sorted
