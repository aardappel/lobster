// graphics demo showing ability to create cube meshes.

include "vec.lobster"
include "color.lobster"
include "3dhelpers.lobster"

fatal(gl_window("cubegen test", 1280, 800))

dim :== xyz_1i * 128

camera_position = float(dim) / 2
camera_yaw = 135

w := cg_init(dim)

def modify():
    pos := xyz_rndi(dim)
    size := xyz_rndi(dim / 8)
    col := rnd(2) * rnd(256)
    cg_set(w, pos, size, col)

for 10000: modify()

random_cubes := cg_create_mesh(w)

while gl_frame():
    if gl_button("escape") == 1: return

    gl_windowtitle("cubegen test: " + gl_deltatime())

    gl_clear(color_dark_grey)

    gl_cursor(0)
    gl_perspective(60, 0.1, 1000)

    speed := 50
    camera_FPS_update("w", "a", "s", "d", speed, 4, true)
    camera_FPS_view()
    gl_light(camera_position, [ 128.0, 0.1 ])

    gl_blend 0:
        gl_setshader("flat")
        gl_rendermesh(random_cubes)

    if gl_button("space") == 1:
        for 1000: modify()
        random_cubes = cg_create_mesh(w)

