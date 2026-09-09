// A method borrows `this` from a field of an object that a global names, and a method called
// from it overwrites that field thru the global.
// EXPECT-ERROR: cannot modify `b.a` while borrowed

class A
class B
let b:B

class A:
    x = "a"
    def ov():
        b.a = A {}
    def sec():
        ov()
        x = "b"

class B:
    a = A {}
    def inb():
        a.sec()

let b:B = B {}
b.inb()
