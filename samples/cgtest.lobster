// graphics demo showing ability to create cube meshes.

import vec
import color
import gl
import camera

fatal(gl.window("cubegen test", 1280, 800))

let dim = int3_1 * 128

let camera = Camera { float(dim) / 2.0, 135.0, 0.0 }

let w = cg.init(dim)

def modify():
    let pos = rnd_int3(dim)
    let size = rnd_int3(dim / 8)
    let col = rnd(2) * rnd(256)
    cg.set(w, pos, size, col)

for 10000: modify()

var random_cubes = cg.create_mesh(w)

while gl.frame():
    if gl.button("escape") == 1: return

    gl.window_title("cubegen test: {gl.delta_time()}")

    gl.clear(color_dark_grey)

    gl.cursor(false)
    gl.perspective(60.0, 0.1, 1000.0)

    let speed = 50.0
    camera.FPS_update("w", "a", "s", "d", speed, 4.0, true)
    camera.FPS_view()
    gl.light(camera.position, float2 { 128.0, 0.1 })

    gl.blend 0:
        gl.set_shader("flat")
        gl.render_mesh(random_cubes)

    if gl.button("space") == 1:
        for 1000: modify()
        random_cubes = cg.create_mesh(w)

