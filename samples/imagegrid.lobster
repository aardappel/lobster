// This example illustrates how to translate and scale a unit square to fill
// the center of the window, then fills that unit square with lobsters in five
// ranks and files, using the forxy 2d iterator defined in modules/vec.lobster.

import gui
import texture

fatal(gl_window("Lobsters Rank and Vile", 512, 512, 1))

let tex = gl_load_texture("data/textures/lobster.jpg")
assert tex

let astride = 5

def center_unit_square():
    let size = float(gl_window_size())
    let major = min(size.x, size.y)
    let scale = xy_1 * major
    gl_translate((size - scale) / 2)
    gl_scale(scale)

while gl_frame():
    gl_clear(color_black)

    center_unit_square()

    gl_set_shader("textured")
    gl_set_primitive_texture(0, tex)

    forxy(xy_1i * astride) v:
        gl_translate float(v) / astride:
            gl_rect(xy_1 / astride, 0)
