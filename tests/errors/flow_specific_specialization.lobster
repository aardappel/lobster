class A:
    def disp():
        print "A"

class B : A
    def disp():
        print "B"


var s:A? = nil
def foo():
    disp(s)
s = B {}
foo()
s = nil
foo()

