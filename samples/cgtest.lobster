// graphics demo showing ability to create cube meshes.

import vec
import color
import gl
import camera

fatal(gl_window("cubegen test", 1280, 800))

let dim = int3_1 * 128

let camera = Camera { float(dim) / 2.0, 135.0, 0.0 }

let w = cg_init(dim)

def modify():
    let pos = rnd_int3(dim)
    let size = rnd_int3(dim / 8)
    let col = rnd(2) * rnd(256)
    cg_set(w, pos, size, col)

for 10000: modify()

var random_cubes = cg_create_mesh(w)

while gl_frame():
    if gl_button("escape") == 1: return

    gl_window_title("cubegen test: {gl_delta_time()}")

    gl_clear(color_dark_grey)

    gl_cursor(false)
    gl_perspective(60.0, 0.1, 1000.0)

    let speed = 50.0
    camera.FPS_update("w", "a", "s", "d", speed, 4.0, true)
    camera.FPS_view()
    gl_light(camera.position, float2 { 128.0, 0.1 })

    gl_blend 0:
        gl_set_shader("flat")
        gl_render_mesh(random_cubes)

    if gl_button("space") == 1:
        for 1000: modify()
        random_cubes = cg_create_mesh(w)

