// implement a standard FPS view with xy as the ground plane, and Z up
// first call update, then view

import vec

class Camera:
    position:float3
    yaw:float
    pitch:float
    velocity = float3_0

    def FPS_update(upkey, leftkey, downkey, rightkey, movementspeed:float, mousesens:float, dopitch):
        yaw -= gl_mouse_delta(0).x / mousesens
        if dopitch:
            pitch -= gl_mouse_delta(0).y / mousesens
            pitch = min(85.0, max(-85.0, pitch))

        let move   = (gl_button(upkey) >= 1) - (gl_button(downkey) >= 1)
        let strafe = (gl_button(rightkey) >= 1) - (gl_button(leftkey) >= 1)
        let friction = 20.0
        let velspeed = min(1.0, friction * gl_delta_time())
        let dir = vecfromyawpitch(yaw, pitch, move, strafe)
        velocity = lerp(velocity, dir, velspeed)
        position += velocity * gl_delta_time() * movementspeed

    def FPS_view():
        set_z_up_worldspace()
        gl_rotate_x(sincos(-pitch))
        gl_rotate_z(sincos(-yaw))
        gl_translate(-position)

    def forward_vector(): return vecfromyawpitch(yaw, pitch,        1.0, 0.0)
    def up_vector():      return vecfromyawpitch(yaw, pitch + 90.0, 1.0, 0.0)
    def left_vector():    return vecfromyawpitch(yaw, pitch,        0.0, 1.0)

    // an alternative way to set a 3D camera with an overriding direction, call this instead of FPS_update,
    // then still call FPS_view as before:
    def direction(v):
        v = normalize(v)
        yaw = atan2(v.xy) - 90.0  // look along Y rather than X
        pitch = asin(v.z)

// Default OpenGL view looks along -Z (Y up). This creates a worldspace where Z is up:
def set_z_up_worldspace():
    gl_rotate_x(sincos(-90.0))

def set_z_up_worldspace(f):
    gl_push_model_view()
    set_z_up_worldspace()
    f()
    gl_pop_model_view()

// Convert points that are in Y up space to Z up.
def to_z_up(v:float3):
    return float3 { v.x, -v.z, v.y }

def set_2d_worldspace(size:float2, origin:float2, scale:float2):
    let ws = float(gl_framebuffer_size())
    gl_translate(ws * origin)
    let sc = ws / size
    gl_scale(scale * min(sc))
