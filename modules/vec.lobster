// utility functions and constants for working with vectors

import std

// Convenient constants:

let xy_0 = xy_f { 0.0, 0.0 }
let xy_1 = xy_f { 1.0, 1.0 }
let xy_h = xy_f { 0.5, 0.5 }
let xy_x = xy_f { 1.0, 0.0 }
let xy_y = xy_f { 0.0, 1.0 }

let xyz_0 = xyz_f { 0.0, 0.0, 0.0 }
let xyz_1 = xyz_f { 1.0, 1.0, 1.0 }
let xyz_x = xyz_f { 1.0, 0.0, 0.0 }
let xyz_y = xyz_f { 0.0, 1.0, 0.0 }
let xyz_z = xyz_f { 0.0, 0.0, 1.0 }

let xyzw_0 = xyzw_f { 0.0, 0.0, 0.0, 0.0 }
let xyzw_1 = xyzw_f { 1.0, 1.0, 1.0, 1.0 }
let xyzw_x = xyzw_f { 1.0, 0.0, 0.0, 0.0 }
let xyzw_y = xyzw_f { 0.0, 1.0, 0.0, 0.0 }
let xyzw_z = xyzw_f { 0.0, 0.0, 1.0, 0.0 }
let xyzw_w = xyzw_f { 0.0, 0.0, 0.0, 1.0 }

// int versions

let xy_0i = xy_i { 0, 0 }
let xy_1i = xy_i { 1, 1 }
let xy_xi = xy_i { 1, 0 }
let xy_yi = xy_i { 0, 1 }

let xyz_0i = xyz_i { 0, 0, 0 }
let xyz_1i = xyz_i { 1, 1, 1 }
let xyz_xi = xyz_i { 1, 0, 0 }
let xyz_yi = xyz_i { 0, 1, 0 }
let xyz_zi = xyz_i { 0, 0, 1 }

let xyzw_0i = xyzw_i { 0, 0, 0, 0 }
let xyzw_1i = xyzw_i { 1, 1, 1, 1 }
let xyzw_xi = xyzw_i { 1, 0, 0, 0 }
let xyzw_yi = xyzw_i { 0, 1, 0, 0 }
let xyzw_zi = xyzw_i { 0, 0, 1, 0 }
let xyzw_wi = xyzw_i { 0, 0, 0, 1 }

let cardinal_directions = [ xy_i { 0, -1 }, xy_i { 1, 0 }, xy_i { 0, 1 }, xy_i { -1, 0 } ]
let diagonal_directions = [ xy_i { -1, -1 }, xy_i { 1, 1 }, xy_i { 1, -1 }, xy_i { -1, 1 } ]

let positive_directions = [ xy_i { 0, 0 }, xy_i { 1, 0 }, xy_i { 1, 1 }, xy_i { 0, 1 } ]

let octant_directions = [
    xyz_i {  1,  1,  1 },
    xyz_i {  1,  1, -1 },
    xyz_i {  1, -1,  1 },
    xyz_i {  1, -1, -1 },
    xyz_i { -1,  1,  1 },
    xyz_i { -1,  1, -1 },
    xyz_i { -1, -1,  1 },
    xyz_i { -1, -1, -1 },
]

// shorten vectors, e.g. xyz -> xy
// FIXME: subtyping should take care of this
def xy(v): return xy { v.x, v.y }
def xz(v): return xy { v.x, v.z }
def yw(v): return xy { v.y, v.w }
def zw(v): return xy { v.z, v.w }

def xyz(v): return xyz { v.x, v.y, v.z }

// lengthen vectors
def xyz<T>(v:xy<T>, z:T):   return xyz<T>  { v.x, v.y, z }
def xyzw<T>(v:xyz<T>, w:T): return xyzw<T> { v.x, v.y, v.z, w }

// create from vector types
def xy_v<T>(v:[T]):   return xy<T>   { v[0], v[1] }
def xyz_v<T>(v:[T]):  return xyz<T>  { v[0], v[1], v[2] }
def xyzw_v<T>(v:[T]): return xyzw<T> { v[0], v[1], v[2], v[3] }
// or to vector
def v_xy<T>(v:xy<T>):     return [ v.x, v.y ]
def v_xyz<T>(v:xyz<T>):   return [ v.x, v.y, v.z ]
def v_xyzw<T>(v:xyzw<T>): return [ v.x, v.y, v.z, v.w ]

// flip vectors
def yx<T>(v:xy<T>):     return xy<T> { v.y, v.x }
def zyx<T>(v:xyz<T>):   return xyz<T> { v.z, v.y, v.x }
def wzyx<T>(v:xyzw<T>): return xyzw<T> { v.w, v.z, v.y, v.x }

def xy_rnd():   return xy_f   { rnd_float(), rnd_float() }
def xyz_rnd():  return xyz_f  { rnd_float(), rnd_float(), rnd_float() }
def xyzw_rnd(): return xyzw_f { rnd_float(), rnd_float(), rnd_float(), rnd_float() }

def xy_rnd_norm():  return normalize(xy_rnd() - 0.5)
def xyz_rnd_norm(): return normalize(xyz_rnd() - 0.5)

def xy_rndi(n:int):    return xy_i  { rnd(n), rnd(n) }
def xy_rndi(n:xy_i):   return xy_i  { rnd(n.x), rnd(n.y) }
def xyz_rndi(n:int):   return xyz_i { rnd(n), rnd(n), rnd(n) }
def xyz_rndi(n:xyz_i): return xyz_i { rnd(n.x), rnd(n.y), rnd(n.z) }

def forxy(v:xy_i, fun): for(v.y) y: for(v.x) x: fun(xy_i { x, y })
def foryx(v:xy_i, fun): for(v.x) x: for(v.y) y: fun(xy_i { x, y })
def mapxy(v:xy_i, fun): return map(v.y) y: map(v.x) x: fun(xy_i { x, y })

def forxyz(v:xyz_i, fun): for(v.z) z: for(v.y) y: for(v.x) x: fun(xyz_i { x, y, z })
def mapxyz(v:xyz_i, fun): return map(v.z) z: map(v.y) y: map(v.x) x: fun(xyz_i { x, y, z })

def vecfromyawpitch(yaw:float, pitch:float, move:float, strafe:float):
    return (xyz(sincos(yaw + 90.0/* start from Y rather than X axis */), 0.0) *
        cos(pitch) + xyz_z * sin(pitch)) * move + xyz(sincos(yaw), 0.0) * strafe

def rotate2D(v:xy_f, angle:float):
    let s = sin(-angle)
    let c = cos(-angle)
    return xy_f { v.x * c + v.y * s,
                  v.y * c - v.x * s }
