import vec

// Unlike a smaller mat2x2, the 4x4 one is intended to be used with the gl. and matrix. functions,
// so stores the elements as a vector. Even though a [float] can support other matrix dimensions, all these
// built-in functions currently work with exactly 16 elements, hence the name.

struct mat4x4:
    v:[float]

    def operator*(o:mat4x4):
        return mat4x4 { matrix.multiply(v, o.v) }

    def set_uniform(name:string):
        return gl.set_uniform_matrix(name, v)

// Convenience functions for constructing one of these.

def mat4x4_rotate_x(angle:float2): return mat4x4 { matrix.rotate_x(angle) }
def mat4x4_rotate_y(angle:float2): return mat4x4 { matrix.rotate_y(angle) }
def mat4x4_rotate_z(angle:float2): return mat4x4 { matrix.rotate_z(angle) }
def mat4x4_translation(offset:float3): return mat4x4 { matrix.translation(offset) }

def mat4x4_mvp(): return mat4x4 { gl.model_view_projection() }
def mat4x4_mv(): return mat4x4 { gl.model_view() }
def mat4x4_p(): return mat4x4 { gl.projection() }

//print mat4x4_rotate_x(float2_1) * mat4x4_translation(float3_1)
