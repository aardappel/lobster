import vec

// A 2x2 matrix representated as a 4 length vector of x,y,z,w so that
// |x y|
// |z w|

struct mat2x2 : xyzw_f

    def operator*(o:mat2x2):
        return mat2x2 { dot(this.xy, o.xz), dot(this.xy, o.yw),
                        dot(this.zw, o.xz), dot(this.zw, o.yw) }
    
    def operator*(o:xy_f):
        return xy_f { dot(o, this.xy),
                      dot(o, this.zw) }

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
let mat2x2_identity = mat2x2 { 1.0, 0.0, 0.0, 1.0 }


// Unlike a smaller matrix like the one above, the 4x4 one is intended to be used with the gl_ and matrix_ functions,
// so stores the elements as a vector. Even though a [float] can support other matrix dimensions, all these
// built-in functions currently work with exactly 16 elements, hence the name.

struct mat4x4:
    v:[float]

    def operator*(o:mat4x4):
        return mat4x4 { matrix_multiply(v, o.v) }

    def set_uniform(name:string):
        return gl_set_uniform_matrix(name, v)

// Convenience functions for constructing one of these.

def mat4x4_rotate_x(angle:xy_f): return mat4x4 { matrix_rotate_x(angle) }
def mat4x4_rotate_y(angle:xy_f): return mat4x4 { matrix_rotate_y(angle) }
def mat4x4_rotate_z(angle:xy_f): return mat4x4 { matrix_rotate_z(angle) }
def mat4x4_translation(offset:xyz_f): return mat4x4 { matrix_translation(offset) }

def mat4x4_mvp(): return mat4x4 { gl_model_view_projection() }
def mat4x4_mv(): return mat4x4 { gl_model_view() }
def mat4x4_p(): return mat4x4 { gl_projection() }

//print mat4x4_rotate_x(xy_1) * mat4x4_translation(xyz_1)
