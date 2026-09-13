// An element borrowed for an argument while a later argument of the same call drops
// elements: the earlier argument takes a reference of its own, so it keeps what it was
// passed rather than this being an error.
// EXPECT-OK

let v = [ "a" + "", "b" + "" ]

def f(a:string, b:string) -> string: return a + b

assert f(v[0], v.pop()) == "ab"
assert v.length == 1
