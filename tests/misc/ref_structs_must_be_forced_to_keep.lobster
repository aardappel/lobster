struct Ref:
    a = []::float3
    b = []::int

class Holder:
    r = Ref {}

let h = Holder {}
let v = [ Ref {} ]

def f(r:Ref):
    r.a = []
    r.b = []

def g(hh:Holder):
    hh = Holder {}

g(h)
f(h.r)
f(v[0])
