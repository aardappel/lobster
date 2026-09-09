// A borrowed argument is an element of a vector, and the callee overwrites an element of it:
// elements are one location to the borrow check (unless both indices are constants that
// differ), and the caller keeps the argument's value alive.
// EXPECT-OK

let v = [ "a" + "", "b" + "" ]

def f(s:string, i:int) -> string:
    v[i] = "c" + ""
    return s

assert f(v[0], 0) == "a"
assert v[0] == "c"
