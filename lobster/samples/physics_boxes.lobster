// Showing off physics features in Lobster

include "vec.lobster"
include "color.lobster"
include "3dhelpers.lobster"

fatal(gl_window("Physics demo : boxes", 1024, 768))

worldsize :== [ 60, 40 ] 

ph_initialize([ 0, -10 ])
ph_createbox([ 0, 1 ], [ 40, 1 ])  // floor

numboxes := 0

while(gl_frame() & !gl_wentdown("escape")):
    gl_clear(color_black)
    
    // create right-handed coordinate system, with (0, 0) at the bottom middle
    camera_2d_worldspace(worldsize, [ 0.5, 1 ], [ 1, -1 ])
    
    ph_step(gl_deltatime())
    ph_render()
    
    // spawn tumbling boxes
    if(numboxes++ < 5000):
        ph_dynamic(ph_createbox([ rnd(50) - 25, rnd(50) + 2 ], [ 0.5, 0.5 ]), true)
