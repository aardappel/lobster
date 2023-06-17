// graphics demo showing Lobster's built-in loading of (animated) .iqm models

import vec
import color
import gl
import texture
import camera

fatal(gl_window("My Lobster OpenGL window!", 1280, 1024))

let iqmtest = check(gl_new_mesh_iqm("models/mrfixit.iqm"), "can\'t load model!")

let textures = map(gl_mesh_parts(iqmtest)) name, i:
    check(gl_load_texture("models/{name}.png"), "problem loading {name}")

for(textures) tex, i: gl_set_mesh_texture(iqmtest, i, 0, tex)

let camera = Camera { float3 { 0.0, 0.0, 8.0 }, -45.0, 0.0 }

while gl_frame():
    if gl_button("escape") == 1: return

    gl_clear(color_dark_grey)
    gl_cursor(false)
    gl_perspective(70.0, 0.1, 1000.0)

    var speed = 10.0
    if gl_button("left shift") >= 1: speed = 50.0
    camera.FPS_update("w", "a", "s", "d", speed, 4.0, true)
    if gl_button("space") >= 1: camera.direction(float3_1 * 10.0 - camera.position)
    camera.FPS_view()

    gl_light(camera.position, float2 { 128.0, 1.0 })
    //gl_light(float3(sincos(gl_time() * 20), 0) * 100 + float3_z * 100)

    gl_blend(blend_none)
    gl_set_shader("phong_anim")
    gl_animate_mesh(iqmtest, gl_time() * 60.0)
    for(10) i:
        gl_translate float2_1 * 10.0 + i * 5.0:
            gl_render_mesh(iqmtest)

    gl_set_shader("color")
    gl_debug_grid(int3 { 20, 20, 0 }, float3_1, 0.005)
