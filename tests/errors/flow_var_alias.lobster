// A nil check promotes a field, and the field is set to nil thru a variable that was
// assigned the object (and owns a reference to it, since it is assigned again): the
// promotion must not survive the write.
// EXPECT-ERROR: dereferencing nillable type

class Test:
    t:Test?

let x = Test { Test { nil } }
var y = x
y = x
if x.t:
    y.t = nil
    print x.t.t
