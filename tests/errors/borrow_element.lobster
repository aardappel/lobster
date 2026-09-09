// A borrowed argument is an element of a vector, and the callee overwrites an element of it:
// elements are one location to the borrow check, unless both indices are constants that
// differ.
// EXPECT-ERROR: cannot modify `v.[]` while borrowed

let v = [ "a" + "", "b" + "" ]

def f(s:string, i:int):
    v[i] = "c" + ""
    print s

f(v[0], 0)
