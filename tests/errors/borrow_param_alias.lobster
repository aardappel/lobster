// A borrowed argument aliases a field of what another (borrowed) argument names, and the
// callee overwrites that field thru the alias: the borrow check sees thru the parameters, and
// the caller keeps the value alive, so `e` stays the object it was passed.
// EXPECT-OK

class E:
    x:int

class O:
    f:E

def clear(o:O): o.f = E { 99 }

def use(e:E, o:O) -> int:
    clear(o)
    return e.x

let o = O { E { 1 } }
assert use(o.f, o) == 1
assert o.f.x == 99
