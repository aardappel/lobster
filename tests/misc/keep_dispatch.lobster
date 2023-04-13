// This tests what happens if you don't force all args of a dynamic dispatch to LT_KEEP.

class A:
    def test(foo:string):
        pass()

    def loop():
        for(3):
            let name = "x"
            test(name)

class B : A
    def test(foo:string):
        // Overwrite forces foo to be LT_KEEP, but its superclass version was fine with borrow.
        foo = "a"


B{}.loop()
