// An `and` or an `or` that a constant makes always take the false side has a constant value,
// but its type is that of the other side. When that is a reference, the value is that type's
// nil, not the false scalar the fold came out of, or the constant lands in a different stack
// slot than the code that reads it looks in.

let never = false
let always = true

assert (never and "yes" or "no") == "no"
assert (always and "yes" or "no") == "yes"

class C:
    x:int

let c = C { 1 }
assert not (never and c)
assert (never and c or c).x == 1

let v = [ 1, 2 ]
assert (never and v or v).length == 2

assert (never and 1.5) == 0.0
assert (never and 1) == 0

// The same, where nothing is constant, which is the path the fold has to agree with.
var runtime_false = false
if seconds_elapsed() > 1000000.0: runtime_false = true
assert (runtime_false and "yes" or "no") == "no"
assert not (runtime_false and c)

print "ok"
