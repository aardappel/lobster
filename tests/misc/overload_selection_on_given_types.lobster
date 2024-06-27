class A:
    a = 1

class B:
    b = 2

def id(t, p:A):
    return p.a

def id(t, r:B):
    return r.b

def foo<T>(t:float):
    return id(t, T {})

assert foo<A>(0.0) + foo<B>(0.0) == 3
