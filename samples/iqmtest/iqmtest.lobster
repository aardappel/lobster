// graphics demo showing Lobster's built-in loading of (animated) .iqm models

import vec
import color
import gl
import texture
import camera

fatal(gl.window("My Lobster OpenGL window!", 1280, 1024))

let iqmtest = check(gl.new_mesh_iqm("models/mrfixit.iqm"), "can\'t load model!")

let textures = map(gl.mesh_parts(iqmtest)) name, i:
    check(gl.load_texture("models/{name}.png"), "problem loading {name}")

for(textures) tex, i: gl.set_mesh_texture(iqmtest, i, 0, tex)

let camera = Camera { float3 { 0.0, 0.0, 8.0 }, -45.0, 0.0 }

while gl.frame():
    if gl.button("escape") == 1: return

    gl.clear(color_dark_grey)
    gl.cursor(false)
    gl.perspective(70.0, 0.1, 1000.0)

    var speed = 10.0
    if gl.button("left shift") >= 1: speed = 50.0
    camera.FPS_update("w", "a", "s", "d", speed, 4.0, true)
    if gl.button("space") >= 1: camera.direction(float3_1 * 10.0 - camera.position)
    camera.FPS_view()

    gl.light(camera.position, float2 { 128.0, 1.0 })
    //gl.light(float3(sincos(gl.time() * 20), 0) * 100 + float3_z * 100)

    gl.blend(blend_none)
    gl.set_shader("phong_anim")
    gl.animate_mesh(iqmtest, gl.time() * 60.0)
    for(10) i:
        gl.translate float2_1 * 10.0 + i * 5.0:
            gl.render_mesh(iqmtest)

    gl.set_shader("color")
    gl.debug_grid(int3 { 20, 20, 0 }, float3_1, 0.005)
