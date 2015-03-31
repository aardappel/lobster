// graphics demo showing Lobster's built-in loading of (animated) .iqm models

include "vec.lobster"
include "color.lobster"
include "3dhelpers.lobster"

fatal(gl_window("My Lobster OpenGL window!", 1280, 1024))

iqmtest := check(gl_newmesh_iqm("models/mrfixit.iqm"), "can\'t load model!")

for(gl_meshparts(iqmtest)) name, i:
    tex := check(gl_loadtexture("models/" + name + ".png"), "problem loading " + name)
    gl_setmeshtexture(iqmtest, i, 0, tex)

camera_position = xyz { 0.0, 0.0, 8.0 }
camera_yaw = 135
camera_pitch = 20

while(gl_frame()):
    if(gl_wentdown("escape")): return

    gl_clear(color_dark_grey)
    gl_cursor(0)
    gl_perspective(70, 0.1, 1000)

    speed := 10
    if(gl_isdown("left shift")): speed = 50
    camera_FPS_update("w", "a", "s", "d", speed, 4, true)
    if(gl_isdown("space")): camera_direction(xyz_1 * 10 - camera_position)
    camera_FPS_view()

    gl_light(camera_position)
    //gl_light(sincos(gl_time() * 20) * 100 + xyz_z * 100)

    gl_blend(0)
    gl_setshader("phong_anim")
    gl_animatemesh(iqmtest, gl_time() * 60)
    gl_translate(xy_1 * 10):
        gl_rendermesh(iqmtest)

    gl_setshader("color")
    gl_debug_grid([20, 20, 0], xyz_1, 0.005)
