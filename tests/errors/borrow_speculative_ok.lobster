// Variables that borrow what they are initialized with (see SpecIdent::speculative) and get
// turned into owners by a later write to what they borrow from. Every one of these must keep
// the value the variable had.
// EXPECT-OK

import std

class Node:
    name:string
    next:Node?

// A let from an element, then the element is overwritten.
let names = [ "a" + "", "b" + "" ]
let first = names[0]
names[0] = "z" + ""
assert first == "a"

// A let from a field, then the field is overwritten.
let n = Node { "n" + "", nil }
let nm = n.name
n.name = "m" + ""
assert nm == "n"

// The old := cur pattern.
var cur = Node { "c0" + "", nil }
for(3) i:
    let old = cur
    cur = Node { "c" + (i + 1), old }
    assert old.name == "c" + i
assert cur.name == "c3"

// A loop element while the loop drops elements.
let v = map(6) i: "s" + i
var seen = ""
for(v) s, i:
    seen += s
    if i % 2 == 0: v.remove(i)
assert seen == "s0s2s3s5"
assert v.length == 4

// A loop element passed to a builtin that removes it.
let objs = map(4) i: Node { "o" + i, nil }
for(objs) o:
    if o.name == "o1": objs.remove_obj(o)
    assert o.name.length == 2
assert objs.length == 3

// A let from an element, and a nested function that overwrites the element.
let w = [ "w0" + "", "w1" + "" ]
def clobber(): w[0] = "x" + ""
let w0 = w[0]
clobber()
assert w0 == "w0"

// A let from a field of an argument, while a callee overwrites that field thru the object.
def rename(nn:Node, s:string): nn.name = s
def peek(nn:Node) -> string:
    let old = nn.name
    rename(nn, "renamed" + "")
    return old
assert peek(n) == "m"
assert n.name == "renamed"

// A let of a whole vector, which is then replaced in its variable.
var list = [ 1, 2, 3 ]
let alias = list
list = [ 4 ]
assert alias.length == 3

// Values borrowed in a loop and stored in another structure outlive the loop.
let kept = []
for(names) s: kept.push(s)
names[1] = "q" + ""
assert kept[1] == "b"
