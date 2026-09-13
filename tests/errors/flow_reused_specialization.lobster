// A function that sets a variable to nil is called a second time, from a context where the
// variable is promoted again: the specialization is reused rather than typechecked again, so
// its write is replayed against this context, and the promotion must not survive the call.
// EXPECT-ERROR: dereferencing nillable type

class T:
    s:int

var x:T? = T { 1 }

def f(): x = nil

if x:
    f()
x = T { 2 }
if x:
    f()
    print x.s
