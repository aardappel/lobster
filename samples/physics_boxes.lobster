// Showing off physics features in Lobster

import vec
import color
import gl
import camera

fatal(gl.window("Physics demo : boxes", 1024, 768))

let worldsize = float2 { 60.0, 40.0 }

ph.initialize(float2 { 0.0, -10.0 })

let floor = ph.create_box(float2 { 0.0, 1.0 }, float2 { 40.0, 1.0 })
var boxes = []

var boxtime = 0.0

while gl.frame() and gl.button("escape") != 1:
    gl.clear(color_black)

    // create right-handed coordinate system, with (0, 0) at the bottom middle
    set_2d_worldspace(worldsize, float2 { 0.5, 1.0 }, float2 { 1.0, -1.0 })

    ph.step(gl.delta_time(), 8, 3)
    ph.render()

    // spawn tumbling boxes
    if gl.time() > boxtime and gl.time() < 500.0:
        boxtime += 0.1  // spawn 10 boxes per second
        let box = ph.create_box(float2 { rnd(50) - 25.0, rnd(50) + 2.0 }, float2 { 0.5, 0.5 })
        ph.dynamic(box, true)
        boxes.push(box)

    if gl.button("space") == 1:
        boxes = []  // Kill all boxes.
