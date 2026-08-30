// A dynamic dispatch on a free variable that is nil at one call and a subclass
// at another: the second call reuses the specialization typechecked for the
// promoted (non-nil) flow type, which the nil call then does not match.
//
// FIXME: this should compile and print "B" then nothing.
// EXPECT-ERROR: no overloads apply for `disp`

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

