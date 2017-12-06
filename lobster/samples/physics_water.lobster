// Showing off physics features in Lobster

include "vec.lobster"
include "color.lobster"
include "texture.lobster"
include "3dhelpers.lobster"
include "physics.lobster"

fatal(gl_window("Physics demo : water", 1024, 768))

worldsize :== xy { 60.0, 40.0 }

ph_initialize(xy { 0.0, -10.0 })
ph_initializeparticles(0.15)
ph_setshader(nil, "color_attr_particle")

floor := ph_createbox(xy {   0.0, 1.0 }, xy { 20.0, 1.0 })
wall1 := ph_createbox(xy { -20.0, 7.0 }, xy {  1.0, 7.0 })
wall2 := ph_createbox(xy {  20.0, 7.0 }, xy {  1.0, 7.0 })

floor.ph_setcolor(color_grey)
wall1.ph_setcolor(color_grey)
wall2.ph_setcolor(color_grey)

boxes := map 5:
    b := ph_createbox(xy { 0.0, 10.0 + _ * 3.0 }, xy { 1.0, 1.0 })
    b.ph_dynamic(true)
    b

ph_createparticlecircle(xy { 0.0, 5.0 }, 7.0, color_light_red, ph_colormixingparticle)

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)

    // create right-handed coordinate system, with (0, 0) at the bottom middle
    camera_2d_worldspace(worldsize, xy { 0.5, 1 }, xy { 1.0, -1.0 })

    if gl_button("mouse1") >= 1:
        ph_createparticlecircle(gl_localmousepos(0), 0.5, color_light_blue, ph_colormixingparticle)

    ph_step(gl_deltatime(), 8, 3)

    gl_blend(blend_add_alpha)
    ph_renderparticles(2.5)
    gl_blend(blend_none)
    ph_render()
