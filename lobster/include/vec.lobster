// utility functions and constants for working with vectors

include "std.lobster"

// vector types: generally for float elements, but useful for other types as well

value xy:   [ x, y:x ]
value xyz:  xy [ z:x ]
value xyzw: xyz [ w:x ]

// Specialized type names:

value xy_f = xy(float)
value xy_i = xy(int)

value xyz_f = xyz(float)
value xyz_i = xyz(int)

value xyzw_f = xyzw(float)
value xyzw_i = xyzw(int)

// Convenient constants:

xy_0 :== [ 0.0, 0.0 ]:xy
xy_1 :== [ 1.0, 1.0 ]:xy
xy_h :== [ 0.5, 0.5 ]:xy
xy_x :== [ 1.0, 0.0 ]:xy
xy_y :== [ 0.0, 1.0 ]:xy

xyz_0 :== [ 0.0, 0.0, 0.0 ]:xyz
xyz_1 :== [ 1.0, 1.0, 1.0 ]:xyz
xyz_x :== [ 1.0, 0.0, 0.0 ]:xyz
xyz_y :== [ 0.0, 1.0, 0.0 ]:xyz
xyz_z :== [ 0.0, 0.0, 1.0 ]:xyz

xyzw_0 :== [ 0.0, 0.0, 0.0, 0.0 ]:xyzw
xyzw_1 :== [ 1.0, 1.0, 1.0, 1.0 ]:xyzw
xyzw_x :== [ 1.0, 0.0, 0.0, 0.0 ]:xyzw
xyzw_y :== [ 0.0, 1.0, 0.0, 0.0 ]:xyzw
xyzw_z :== [ 0.0, 0.0, 1.0, 0.0 ]:xyzw
xyzw_w :== [ 0.0, 0.0, 0.0, 1.0 ]:xyzw

// int versions

xy_0i :== [ 0, 0 ]:xy
xy_1i :== [ 1, 1 ]:xy
xy_xi :== [ 1, 0 ]:xy
xy_yi :== [ 0, 1 ]:xy

xyz_0i :== [ 0, 0, 0 ]:xyz
xyz_1i :== [ 1, 1, 1 ]:xyz
xyz_xi :== [ 1, 0, 0 ]:xyz
xyz_yi :== [ 0, 1, 0 ]:xyz
xyz_zi :== [ 0, 0, 1 ]:xyz

xyzw_0i :== [ 0, 0, 0, 0 ]:xyzw
xyzw_1i :== [ 1, 1, 1, 1 ]:xyzw
xyzw_xi :== [ 1, 0, 0, 0 ]:xyzw
xyzw_yi :== [ 0, 1, 0, 0 ]:xyzw
xyzw_zi :== [ 0, 0, 1, 0 ]:xyzw
xyzw_wi :== [ 0, 0, 0, 1 ]:xyzw

function allint(v): for(v): assert(_ is int)    // FIXME: should probably be a language feature

function forxy(v, fun): for(v.y) y: for(v.x) x: fun([ x, y ]:xy)
function mapxy(v, fun): map(v.y) y: map(v.x) x: fun([ x, y ]:xy)

function vecfromyawpitch(yaw, pitch, move, strafe):
    (sincos(yaw - 90/* start from Y rather than X axis */) *
        cos(pitch) - xyz_z * sin(pitch)) * move + sincos(yaw) * strafe

function inrange2d(test, num, bias): test.x.inrange(num.x, bias.x) &
                                     test.y.inrange(num.y, bias.y)

function rotate2D(v, angle):
    s := sin(-angle)
    c := cos(-angle)
    [ v.x * c + v.y * s, v.y * c - v.x * s ]:xy
