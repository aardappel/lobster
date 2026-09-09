// A builtin that drops elements counts as a write to them.
// EXPECT-ERROR: cannot modify `v.[]` while borrowed

let v = [ "a" + "" ]

def f(s:string):
    v.pop()
    print s

f(v[0])
