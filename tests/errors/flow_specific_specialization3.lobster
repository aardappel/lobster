// Same as flow_specific_specialization.lobster, with the variable declared by
// `member` inside a method rather than at the top level.
//
// FIXME: this should compile and print "B" then nothing.
// EXPECT-ERROR: no overloads apply for `disp`

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
