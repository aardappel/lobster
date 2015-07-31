// Showing off physics features in Lobster

include "vec.lobster"
include "color.lobster"
include "3dhelpers.lobster"

fatal(gl_window("Physics demo : boxes", 1024, 768))

worldsize :== xy { 60.0, 40.0 }

ph_initialize([ 0.0, -10.0 ])
ph_createbox([ 0.0, 1.0 ], [ 40.0, 1.0 ])  // floor

boxtime := 0.0

while(gl_frame() & !gl_wentdown("escape")):
    gl_clear(color_black)

    // create right-handed coordinate system, with (0, 0) at the bottom middle
    camera_2d_worldspace(worldsize, xy { 0.5, 1.0 }, xy { 1.0, -1.0 })

    ph_step(gl_deltatime())
    ph_render()

    // spawn tumbling boxes
    if(gl_time() > boxtime & gl_time() < 500):
        boxtime += 0.1  // spawn 10 boxes per second
        ph_dynamic(ph_createbox([ rnd(50) - 25.0, rnd(50) + 2.0 ], [ 0.5, 0.5 ]), true)
