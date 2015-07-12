// utility functions and constants for working with vectors

include "std.lobster"

// vector types: generally for float elements, but useful for other types as well

value xy { x, y:x }
value xyz : xy { z:x }
value xyzw : xyz { w:x }

// Specialized type names:

value xy_f = xy(float)
value xy_i = xy(int)

value xyz_f = xyz(float)
value xyz_i = xyz(int)

value xyzw_f = xyzw(float)
value xyzw_i = xyzw(int)

// Convenient constants:

xy_0 :== xy { 0.0, 0.0 }
xy_1 :== xy { 1.0, 1.0 }
xy_h :== xy { 0.5, 0.5 }
xy_x :== xy { 1.0, 0.0 }
xy_y :== xy { 0.0, 1.0 }

xyz_0 :== xyz { 0.0, 0.0, 0.0 }
xyz_1 :== xyz { 1.0, 1.0, 1.0 }
xyz_x :== xyz { 1.0, 0.0, 0.0 }
xyz_y :== xyz { 0.0, 1.0, 0.0 }
xyz_z :== xyz { 0.0, 0.0, 1.0 }

xyzw_0 :== xyzw { 0.0, 0.0, 0.0, 0.0 }
xyzw_1 :== xyzw { 1.0, 1.0, 1.0, 1.0 }
xyzw_x :== xyzw { 1.0, 0.0, 0.0, 0.0 }
xyzw_y :== xyzw { 0.0, 1.0, 0.0, 0.0 }
xyzw_z :== xyzw { 0.0, 0.0, 1.0, 0.0 }
xyzw_w :== xyzw { 0.0, 0.0, 0.0, 1.0 }

// int versions

xy_0i :== xy { 0, 0 }
xy_1i :== xy { 1, 1 }
xy_xi :== xy { 1, 0 }
xy_yi :== xy { 0, 1 }

xyz_0i :== xyz { 0, 0, 0 }
xyz_1i :== xyz { 1, 1, 1 }
xyz_xi :== xyz { 1, 0, 0 }
xyz_yi :== xyz { 0, 1, 0 }
xyz_zi :== xyz { 0, 0, 1 }

xyzw_0i :== xyzw { 0, 0, 0, 0 }
xyzw_1i :== xyzw { 1, 1, 1, 1 }
xyzw_xi :== xyzw { 1, 0, 0, 0 }
xyzw_yi :== xyzw { 0, 1, 0, 0 }
xyzw_zi :== xyzw { 0, 0, 1, 0 }
xyzw_wi :== xyzw { 0, 0, 0, 1 }

cardinal_directions := [ xy { 0, -1 }, xy { 1, 0 }, xy { 0, 1 }, xy { -1, 0 } ]
diagonal_directions := [ xy { -1, -1 }, xy { 1, 1 }, xy { 1, -1 }, xy { -1, 1 } ]

positive_directions := [ xy { 0, 0 }, xy { 1, 0 }, xy { 1, 1 }, xy { 0, 1 } ]

// shorten vectors, e.g. xyz -> xy
// FIXME: subtyping should take care of this
def xy(v): xy { v.x, v.y }
def xyz(v): xyz { v.x, v.y, v.z }

def forxy(v, fun): for(v.y) y: for(v.x) x: fun(xy { x, y })
def mapxy(v, fun): map(v.y) y: map(v.x) x: fun(xy { x, y })

def vecfromyawpitch(yaw, pitch, move, strafe):
    (sincos(yaw - 90/* start from Y rather than X axis */) *
        cos(pitch) - xyz_z * sin(pitch)) * move + sincos(yaw) * strafe

def rotate2D(v, angle):
    s := sin(-angle)
    c := cos(-angle)
    xy { v.x * c + v.y * s,
         v.y * c - v.x * s }
