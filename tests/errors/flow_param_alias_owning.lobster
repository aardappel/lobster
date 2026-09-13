// Like flow_param_alias, but the parameter owns a reference (it is assigned in the body)
// rather than borrowing what the caller passed: its fields are still those of the caller's
// object, so the write to one thru it drops the caller's promotion.
// EXPECT-ERROR: dereferencing nillable type

class Test:
    t:Test?

let x = Test { Test { nil } }

def clear(y:Test):
    y.t = nil
    y = Test { nil }

if x.t:
    clear(x)
    print x.t.t
