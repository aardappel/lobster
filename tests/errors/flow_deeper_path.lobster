// Overwriting x.f replaces the object that the promoted x.f.g was a field of.
// EXPECT-ERROR: dereferencing nillable type

class G:
    z:int

class F:
    g:G?

class X:
    f:F

let x = X { F { G { 1 } } }
if x.f.g:
    x.f = F { nil }
    print x.f.g.z
