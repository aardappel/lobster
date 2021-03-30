class A:
    def f(): return 1

var v:A? = nil

def g():
    return if v: v.f() else: 3

// causes f dispatch to by typechecked without B
assert g() == 3

class B : A
    def f(): return 2

v = B {}

// g is already typechecked, and not re-visited.
assert g() == 2
