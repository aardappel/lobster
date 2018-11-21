// graphics demo showing Lobster's built-in loading of (animated) .iqm models

include vec
include color
include texture
include util3d

fatal(gl_window("My Lobster OpenGL window!", 1280, 1024))

iqmtest := check(gl_newmesh_iqm("models/mrfixit.iqm"), "can\'t load model!")

textures := map(gl_meshparts(iqmtest)) name, i:
    check(gl_loadtexture("models/" + name + ".png"), "problem loading " + name)

for(textures) tex, i: gl_setmeshtexture(iqmtest, i, 0, tex)

camera_position = xyz { 0.0, 0.0, 8.0 }
camera_yaw = -45
camera_pitch = 20

while gl_frame():
    if gl_button("escape") == 1: return

    gl_clear(color_dark_grey)
    gl_cursor(0)
    gl_perspective(70, 0.1, 1000)

    speed := 10
    if gl_button("left shift") >= 1: speed = 50
    camera_FPS_update("w", "a", "s", "d", speed, 4, true)
    if gl_button("space") >= 1: camera_direction(xyz_1 * 10 - camera_position)
    camera_FPS_view()

    gl_light(camera_position, xy { 128.0, 1.0 })
    //gl_light(xyz(sincos(gl_time() * 20), 0) * 100 + xyz_z * 100)

    gl_blend(blend_none)
    gl_setshader("phong_anim")
    gl_animatemesh(iqmtest, gl_time() * 60)
    for(10) i:
        gl_translate xy_1 * 10 + i * 5:
            gl_rendermesh(iqmtest)

    gl_setshader("color")
    gl_debug_grid(xyz { 20, 20, 0 }, xyz_1, 0.005)
