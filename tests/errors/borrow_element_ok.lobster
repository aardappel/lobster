// Element accesses that must keep compiling: writing elements inside a loop over the vector,
// reading an element to write another, dropping elements while the loop element is owned,
// and writing a field of one element while a field of an element with a different constant
// index is borrowed.
// EXPECT-OK

import std

class C:
    n:int
    s:string

let cs = map(5) i: C { i, "c" + i }
for(cs) c, i:
    cs[i] = C { c.n * 2, c.s }
    if i == 2: cs[0] = cs[1]
assert cs[3].n == 6

var strs = [ "a", "b", "c" ]
strs[0] = strs[2]
strs[1] = strs[1]
let t = strs[0]
strs[0] = "z"
assert t == "c"
for(strs) s, i:
    if s == "b": strs.remove(i)
assert strs.length == 2

def rename(c:C, s:string):
    c.s = s
    return c.n
assert rename(cs[1], cs[2].s) == 2

def sum_first(v:[C], c:C) -> int:
    v.push(C { 9, "x" })
    return v[0].n + c.n
assert sum_first(cs, cs[1]) == 4
