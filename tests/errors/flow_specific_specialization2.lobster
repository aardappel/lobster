// Same shape as flow_specific_specialization.lobster, but the promotion is on
// a free variable used inside a lambda passed to a builtin.
// EXPECT-OK

import std

var a:string? = nil

a = ""

print (qsort([ 1, 2, 3 ]) x, y: x < y + a.length)

