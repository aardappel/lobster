// A nil check promotes a field, and a callee sets that field to nil thru a parameter that
// aliases the object: the promotion must not survive the call.
// EXPECT-ERROR: dereferencing nillable type

class E:
    x:int

class O:
    f:E?

def clear(o:O): o.f = nil

def use(o:O) -> int:
    if o.f:
        clear(o)
        return o.f.x
    return -1

let o = O { E { 1 } }
print use(o)
