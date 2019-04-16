// Showing off physics features in Lobster

import vec
import color
import texture
import util3d
import physics

fatal(gl_window("Physics demo : water", 1024, 768))

let worldsize = xy { 60.0, 40.0 }

ph_initialize(xy { 0.0, -10.0 })
ph_initialize_particles(0.15)
ph_set_shader(nil, "color_attr_particle")

let floor = ph_create_box(xy {   0.0, 1.0 }, xy { 20.0, 1.0 })
let wall1 = ph_create_box(xy { -20.0, 7.0 }, xy {  1.0, 7.0 })
let wall2 = ph_create_box(xy {  20.0, 7.0 }, xy {  1.0, 7.0 })

floor.ph_set_color(color_grey)
wall1.ph_set_color(color_grey)
wall2.ph_set_color(color_grey)

let boxes = map 5:
    let b = ph_create_box(xy { 0.0, 10.0 + _ * 3.0 }, xy { 1.0, 1.0 })
    b.ph_dynamic(true)
    b

ph_create_particle_circle(xy { 0.0, 5.0 }, 7.0, color_light_red, ph_colormixingparticle)

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)

    // create right-handed coordinate system, with (0, 0) at the bottom middle
    camera_2d_worldspace(worldsize, xy { 0.5, 1 }, xy { 1.0, -1.0 })

    if gl_button("mouse1") >= 1:
        ph_create_particle_circle(gl_local_mouse_pos(0), 0.5, color_light_blue, ph_colormixingparticle)

    ph_step(gl_delta_time(), 8, 3)

    gl_blend(blend_add_alpha)
    ph_render_particles(2.5)
    gl_blend(blend_none)
    ph_render()
