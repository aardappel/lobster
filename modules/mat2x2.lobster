import vec

// A 2x2 matrix representated as a 4 length vector of x,y,z,w so that
// |x y|
// |z w|

struct mat2x2 : float4

    def operator*(o:mat2x2):
        return mat2x2 { dot(o.xz, this),    dot(o.yw, this),
                        dot(o.xz, this.zw), dot(o.yw, this.zw) }

    def operator*(o:float2):
        return float2 { dot(o, this),
                        dot(o, this.zw) }

    def determinant():
        return x*w - y*z

    def adjugate():
        return mat2x2 { w, -y, -z, x }

    def inverse():
        return adjugate() / determinant()

    def trace():
        return x*w


// The identity matrix:
// |1 0|
// |0 1|
let mat2x2_identity = mat2x2 { 1.0, 0.0, 0.0, 1.0 }
