// helper functions typical for 3d games

// implement a standard FPS view with xy as the ground plane, and Z up
// first call update, then view

import vec
import color
import gl
import texture

var camera_position = xyz_0
var camera_yaw = 0.0
var camera_pitch = 0.0
var camera_velocity = xyz_0

def camera_FPS_update(upkey, leftkey, downkey, rightkey, movementspeed:float, mousesens:float, dopitch):
    camera_yaw -= gl_mouse_delta(0).x / mousesens
    if dopitch:
        camera_pitch -= gl_mouse_delta(0).y / mousesens
        camera_pitch = min(85.0, max(-85.0, camera_pitch))

    let move   = (gl_button(upkey) >= 1) - (gl_button(downkey) >= 1)
    let strafe = (gl_button(rightkey) >= 1) - (gl_button(leftkey) >= 1)
    let friction = 20
    let velspeed = min(1.0, friction * gl_delta_time())
    let dir = vecfromyawpitch(camera_yaw, camera_pitch, move, strafe)
    camera_velocity = lerp(camera_velocity, dir, velspeed)
    camera_position += camera_velocity * gl_delta_time() * movementspeed

def camera_set_z_up_worldspace(f):
    // Default OpenGL view looks along -Z (Y up). We want a worldspace where -Z is down:
    gl_rotate_x(sincos(-90)): f()

def to_z_up(v):
    // Convert points that are in Y up space to Z up.
    return xyz { v.x, -v.z, v.y }

def camera_FPS_view():
    camera_set_z_up_worldspace(): nil
    gl_rotate_x(sincos(-camera_pitch - 90))
    gl_rotate_z(sincos(-camera_yaw))
    gl_translate(-camera_position)

def camera_forward_vector(): return vecfromyawpitch(camera_yaw, camera_pitch,      1.0, 0.0)
def camera_up_vector():      return vecfromyawpitch(camera_yaw, camera_pitch + 90, 1.0, 0.0)
def camera_left_vector():    return vecfromyawpitch(camera_yaw, camera_pitch,      0.0, 1.0)

// an alternative way to set a 3D camera with an overriding direction, call this instead of camera_FPS_update,
// then still call camera_FPS_view as before:

def camera_direction(v):
    v = normalize(v)
    camera_yaw = atan2(xy(v)) - 90  // look along Y rather than X
    camera_pitch = asin(v.z)

def camera_2d_worldspace(size:xy_f, origin:xy_f, scale:xy_f):
    let ws = float(gl_window_size())
    gl_translate(ws * origin)
    let sc = ws / size
    gl_scale(scale * min(sc))

private let delta_time_log = map(60): 1 / 30.0
def gl_deltatime_over_time():
    delta_time_log.push(gl_delta_time())
    delta_time_log.remove(0)
    return (delta_time_log.fold(0.0): _a + _e) / delta_time_log.length
