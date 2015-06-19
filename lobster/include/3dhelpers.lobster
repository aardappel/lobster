// helper functions typical for 3d games

// implement a standard FPS view with xy as the ground plane, and z up
// first call update, then view

include "vec.lobster"

camera_position := xyz_0
camera_yaw := 0.0
camera_pitch := 0.0
camera_velocity := xyz_0

def camera_FPS_update(upkey, leftkey, downkey, rightkey, movementspeed:float, mousesens:float, dopitch):
    camera_yaw  += gl_mousedelta(0).x / mousesens
    if(dopitch):
        camera_pitch += gl_mousedelta(0).y / mousesens
        camera_pitch = min(85.0, max(-85.0, camera_pitch))

    move   := gl_isdown(upkey) - gl_isdown(downkey)
    strafe := gl_isdown(rightkey) - gl_isdown(leftkey)
    friction :== 20
    velspeed := min(1.0, friction * gl_deltatime())
    dir := vecfromyawpitch(camera_yaw, camera_pitch, move, strafe)
    camera_velocity = lerp(camera_velocity, dir, velspeed)
    camera_position += camera_velocity * gl_deltatime() * movementspeed

def camera_FPS_view():
    gl_rotate_x(90) // 90: look forward rather than down
    gl_rotate_x(-camera_pitch)
    gl_rotate_z(-camera_yaw)
    gl_translate(-camera_position)

def camera_forward_vector(): vecfromyawpitch(camera_yaw, camera_pitch, 1.0, 0.0)
def camera_up_vector():      vecfromyawpitch(camera_yaw, camera_pitch + 90, 1.0, 0.0)
def camera_left_vector():    vecfromyawpitch(camera_yaw, camera_pitch, 0.0, 1.0)

// an alternative way to set a 3D camera with an overriding direction, call this instead of camera_FPS_update,
// then still call camera_FPS_view as before:

def camera_direction(v):
    v = normalize(v)
    camera_yaw = atan2(v) + 90  // look along Y rather than X
    camera_pitch = arcsin(-v.z)

def camera_2d_worldspace(size:xy_f, origin:xy_f, scale:xy_f):
    ws := float(gl_windowsize())
    gl_translate(ws * origin)
    sc := ws / size
    gl_scale(scale * min(sc.x, sc.y))

