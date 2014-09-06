// utility functions and constants for working with vectors

include "std.lobster"

// vector types: generally for float elements, but useful for other types as well

value xy:   [ x, y ]
value xyz:  xy [ z ]
value xyzw: xyz [ w ]

vec_0 :== [ 0.0, 0.0, 0.0, 0.0 ]:xyzw
vec_1 :== [ 1.0, 1.0, 1.0, 1.0 ]:xyzw

vec_h :== [ 0.5, 0.5 ]:xy

vec_x :== [ 1.0, 0.0, 0.0, 0.0 ]:xyzw
vec_y :== [ 0.0, 1.0, 0.0, 0.0 ]:xyzw
vec_z :== [ 0.0, 0.0, 1.0, 0.0 ]:xyzw
vec_w :== [ 0.0, 0.0, 0.0, 1.0 ]:xyzw

vec_xy :== [ 1.0, 1.0, 0.0, 0.0 ]:xyzw

// int versions

vec_xi :== [ 1, 0, 0, 0 ]:xyzw
vec_yi :== [ 0, 1, 0, 0 ]:xyzw
vec_zi :== [ 0, 0, 1, 0 ]:xyzw
vec_wi :== [ 0, 0, 0, 1 ]:xyzw

vec_0i :== [ 0, 0, 0, 0 ]:xyzw
vec_1i :== [ 1, 1, 1, 1 ]:xyzw

function allint(v): for(v): assert(_ is int)    // FIXME: should probably be a language feature

function forxy(v, fun): for(v.y) y: for(v.x) x: fun([ x, y ]:xy)
function mapxy(v, fun): map(v.y) y: map(v.x) x: fun([ x, y ]:xy)

function vecfromyawpitch(yaw, pitch, move, strafe):
    (sincos(yaw - 90/* start from Y rather than X axis */) *
        cos(pitch) - vec_z * sin(pitch)) * move + sincos(yaw) * strafe

function inrange2d(test, num, bias): test.x.inrange(num.x, bias.x) &
                                     test.y.inrange(num.y, bias.y)

function rotate2D(v, angle):
    s := sin(-angle)
    c := cos(-angle)
    [ v.x * c + v.y * s, v.y * c - v.x * s ]:xy
