class A:
    def disp():
        print "A"

class B : A
    def disp():
        print "B"

class Test:
    def test():
        member s:A? = nil
        def foo():
            disp(s)
        s = B {}
        foo()
        s = nil
        foo()

(Test {}).test()
