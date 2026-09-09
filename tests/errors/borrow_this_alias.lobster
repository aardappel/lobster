// A method borrows `this` from a field of an object that a global names, and a method called
// from it overwrites that field thru the global: the caller keeps the old object alive.
// EXPECT-OK

class A
class B
let b:B

class A:
    x = "a"
    def ov():
        b.a = A {}
    def sec() -> string:
        ov()
        x = "b"
        return x

class B:
    a = A {}
    def inb() -> string:
        return a.sec()

let b:B = B {}
assert b.inb() == "b"
assert b.a.x == "a"
