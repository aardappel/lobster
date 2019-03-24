// Showing off physics features in Lobster

include vec
include color
include util3d

fatal(gl_window("Physics demo : boxes", 1024, 768))

let worldsize = xy { 60.0, 40.0 }

ph_initialize(xy { 0.0, -10.0 })

let floor = ph_create_box(xy { 0.0, 1.0 }, xy { 40.0, 1.0 })
var boxes = []

var boxtime = 0.0

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)

    // create right-handed coordinate system, with (0, 0) at the bottom middle
    camera_2d_worldspace(worldsize, xy { 0.5, 1.0 }, xy { 1.0, -1.0 })

    ph_step(gl_delta_time(), 8, 3)
    ph_render()

    // spawn tumbling boxes
    if gl_time() > boxtime and gl_time() < 500:
        boxtime += 0.1  // spawn 10 boxes per second
        let box = ph_create_box(xy { rnd(50) - 25.0, rnd(50) + 2.0 }, xy { 0.5, 0.5 })
        ph_dynamic(box, true)
        boxes.push(box)

    if gl_button("space") == 1:
        boxes = []  // Kill all boxes.
