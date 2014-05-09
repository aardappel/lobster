// Showing off physics features in Lobster

include "vec.lobster"
include "color.lobster"
include "3dhelpers.lobster"
include "physics.lobster"

fatal(gl_window("Physics demo : water", 1024, 768))

worldsize :== [ 60, 40 ] 

ph_initialize([ 0, -10 ])
ph_initializeparticles(0.15)
ph_setshader(0, "color_attr_particle")

ph_setcolor(ph_createbox([   0, 1 ], [ 20, 1 ]), color_grey) // floor
ph_setcolor(ph_createbox([ -20, 7 ], [  1, 7 ]), color_grey) // wall
ph_setcolor(ph_createbox([  20, 7 ], [  1, 7 ]), color_grey) // wall

for(5): ph_dynamic(ph_createbox([ 0, 10 + _ * 2 ], [ 1, 1 ]), true)

ph_createparticlecircle([ 0, 5 ], 7, color_light_red, ph_colormixingparticle)

while(gl_frame() & !gl_wentdown("escape")):
    gl_clear(color_black)

    // create right-handed coordinate system, with (0, 0) at the bottom middle
    camera_2d_worldspace(worldsize, [ 0.5, 1 ], [ 1, -1 ])

    if(gl_isdown("mouse1")):
        ph_createparticlecircle(gl_localmousepos(0), 0.5, color_light_blue, ph_colormixingparticle)

    ph_step(gl_deltatime())

    gl_blend(3)
    ph_renderparticles(2.5)
    gl_blend(0)
    ph_render()
