// A borrowed argument aliases a field of what another (borrowed) argument names, and the
// callee overwrites that field thru the alias: the borrow check must see thru the parameters.
// EXPECT-ERROR: cannot modify `o.f` while borrowed

class E:
    x:int

class O:
    f:E

def clear(o:O): o.f = E { 99 }

def use(e:E, o:O) -> int:
    clear(o)
    return e.x

let o = O { E { 1 } }
print use(o.f, o)
