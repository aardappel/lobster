// A builtin that drops elements counts as a write to them, and the caller keeps the argument
// alive across it.
// EXPECT-OK

let v = [ "a" + "" ]

def f(s:string) -> string:
    v.pop()
    return s

assert f(v[0]) == "a"
assert v.length == 0
