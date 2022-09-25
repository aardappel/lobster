import vec

// A 2x2 matrix representated as a 4 length vector of x,y,z,w so that
// |x y|
// |z w|

struct mat2x2 : xyzw_f

    def operator*(o:mat2x2):
        // This could be expressed with 4 dot product if `this.zw` existed (and 
        // others)
        return mat2x2 { x*o.x + y*o.z, x*o.y + y*o.w,
                        z*o.x + w*o.z, z*o.y + w*o.w }  
    
    def operator*(o:xy_f):
        // This would be nicer if `this.zw` existed.
        return xy_f { dot(o, this.xy),
                      dot(o, xy_f{z,w})}


    def operator*(f:float):
        // Note the order is flipped so that it is called with xyzw_f * operator
        // instead of mat2x2's
        return f * this

    def determinant():
        return x*w - y*z

    def adjugate():
        return mat2x2{ w, -y, -z, x }

    def inverse():
        return adjugate() / determinant()

    def trace():
        return x*w

        
// The identity matrix:
// |1 0|
// |0 1|
let mat2x2_identity = mat2x2{1.0, 0.0, 0.0, 1.0}