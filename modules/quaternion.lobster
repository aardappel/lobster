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

    def operator*(f:float):
        return quat { x * f, y * f, z * f, w * f }

    def operator-():
        return quat { -x, -y, -z, w }

    def transform(p:xyz_f):
        let v = xyz(this)
        return p + cross(v, cross(v, p) + p * w) * 2.0

def quat_from_angle_axis(angle:float, axis:xyz_f):
    angle *= 0.5
    let v = axis * sin(angle)
    return quat { v.x, v.y, v.z, cos(angle) }

def quat_from_vec(v:xyz_f):
    return quat { v.x, v.y, v.z, -sqrt(max(1.0 - dot(v, v), 0.0)) }

def quat_from_2vecs(u:xyz_f, v:xyz_f):
    let norm_u_norm_v = sqrt(dot(u, u) * dot(v, v))
    var real_part = norm_u_norm_v + dot(u, v)
    let w =
        if real_part < 1.0e-6 * norm_u_norm_v:
            // If u and v are exactly opposite, rotate 180 degrees
            // around an arbitrary orthogonal axis. Axis normalisation
            // can happen later, when we normalise the quaternion.
            real_part = 0.0
            if abs(u.x) > abs(u.z):
                xyz_f { -u.y,  u.x, 0.0 }
            else:
                xyz_f {  0.0, -u.z, u.y }
        else:
            // Otherwise, build quaternion the standard way.
            cross(u, v)
    return normalize(quat{ w.x, w.y, w.z, real_part })

def quat_from_euler(yaw, pitch):
    let cy = cos(yaw * 0.5)
    let sy = sin(yaw * 0.5)
    let cp = cos(pitch * 0.5)
    let sp = sin(pitch * 0.5)
    return quat { -sp * sy, sp * cy, cp * sy, cp * cy }

def quat_from_yaw(yaw):
    let cy = cos(yaw * 0.5)
    let sy = sin(yaw * 0.5)
    return quat { 0.0, 0.0, sy, cy }

def quat_from_pitch(pitch):
    let cp = cos(pitch * 0.5)
    let sp = sin(pitch * 0.5)
    return quat { 0.0, sp, 0.0, cp }

let quat_unit = quat { 0.0, 0.0, 0.0, 1.0 }
