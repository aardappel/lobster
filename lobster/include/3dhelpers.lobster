// helper functions typical for 3d games

// implement a standard FPS view with xy as the ground plane, and z up
// first call update, then view

include "vec.lobster"

camera_position := vec_0
camera_yaw := 0.0
camera_pitch := 0.0
camera_velocity := vec_0

function camera_FPS_update(upkey, leftkey, downkey, rightkey, movementspeed:float, mousesens:float, dopitch):
    camera_yaw  += gl_mousedelta(0).x / mousesens
    if(dopitch):
        camera_pitch += gl_mousedelta(0).y / mousesens
        camera_pitch = min(85, max(-85, camera_pitch))
    
    move   := gl_isdown(upkey) - gl_isdown(downkey)
    strafe := gl_isdown(rightkey) - gl_isdown(leftkey)
    friction := 20
    velspeed := min(1, friction * gl_deltatime())
    dir := vecfromyawpitch(camera_yaw, camera_pitch, move, strafe)
    camera_velocity = lerp(camera_velocity, dir, velspeed)
    camera_position += camera_velocity * gl_deltatime() * movementspeed

function camera_FPS_view():    
    gl_rotate_x(90) // 90: look forward rather than down
    gl_rotate_x(-camera_pitch)
    gl_rotate_z(-camera_yaw)
    gl_translate(-camera_position)

// an alternative way to set a 3D camera with an overriding direction, call this instead of camera_FPS_update, then still call camera_FPS_view as before:

function camera_direction(v):
    v = normalize(v)
    camera_yaw = atan2(v) + 90  // look along Y rather than X
    camera_pitch = arcsin(-v.z)

function camera_2d_worldspace(size, origin, scale):
    gl_translate(gl_windowsize() * origin)
    sc := gl_windowsize() / size
    gl_scale(min(sc.x, sc.y) * scale)

