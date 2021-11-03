import vec

// Operator overloading example. Some quat operations are the same as for
// vectors and can still use Lobster's built-in operators, whereas others
// have special implementations below.

struct quat : xyzw_f

    def operator*(o:quat):
        return quat { w * o.x + x * o.w + y * o.z - z * o.y,
                      w * o.y - x * o.z + y * o.w + z * o.x,
                      w * o.z + x * o.y - y * o.x + z * o.w,
                      w * o.w - x * o.x - y * o.y - z * o.z }

    def operator-():
        return quat { -x, -y, -z, w }

    def transform(p:xyz_f):
        let v = xyz(this)
        return p + cross(v, cross(v, p) + p * w) * 2.0

def quat_from_angle_axis(angle:float, axis:xyz_f):
    angle *= 0.5
    let v = sin(angle) * axis
    return quat { v.x, v.y, v.z, cos(angle) }

def quat_from_vec(v:xyz_f):
    return quat { v.x, v.y, v.z, -sqrt(max(1.0 - manhattan(v), 0.0)) }
