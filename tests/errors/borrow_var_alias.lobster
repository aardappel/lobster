// A callee overwrites, thru a global that was assigned the object (and owns a reference to
// it, since it is assigned again below), the field its borrowed argument was taken from:
// the caller keeps the argument alive rather than this being an error, so `b` stays the
// object it was passed instead of being freed by the write.
// EXPECT-OK

class Test:
    t:Test?
    n:int

var a:Test? = Test { Test { Test { Test { nil, 4 }, 3 }, 2 }, 1 }
var c:Test? = a

def f(b:Test) -> int:
    c.t = nil
    return b.n

if a.t and a.t.t:
    assert f(a.t.t) == 3
c = a
assert a.t == nil
