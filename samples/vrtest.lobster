// graphics demo showing Lobster's built-in loading of (animated) .iqm models

import vec
import color
import gl
import texture
import camera
import gl
import mg

let vrmode = vr_init()

fatal(gl_window(if vrmode: "VR Test" else: "No VR device", 1280, 640,
                if vrmode: gl_window_init_no_vsync else: 0, 16 /*samples*/))

// Make a quick sample scene out of some trees.

def model_tree(numb, branchl, narrowf, leafs, leafc) -> void:
    if numb:
        mg_translate(xyz_z * branchl):
            mg_tapered_cylinder(1.0, narrowf, branchl)
            mg_translate(xyz_z * branchl):
                let axis = sincos(rnd(360))
                let branches = rnd(3) + 1
                for(branches) i:
                    mg_rotate(xyz_z, 360 * i / branches):
                        mg_rotate(xyz(axis, 0), 12 * branches):
                            mg_scale(narrowf):
                                mg_sphere(1.0)
                                model_tree(numb - 1, branchl, narrowf, leafs, leafc)
    else:
        mg_color(leafc):
            mg_sphere(leafs)

mg_smooth(0.5)
mg_color(color { 0.7, 0.6, 0.5, 1.0 })
model_tree(10, 1.5, 0.75, 12, color { 0.6, 1.0, 0.6, 1.0 })
let tree = mg_polygonize(50)

mg_smooth(0.0)
mg_color(color_red)
mg_sphere(1.0)
let sphere = mg_polygonize(3)

let camera = Camera { xyz { 0.0, 0.0, 3.0 }, 135.0, 20.0 }

let balls = []::[xyz_f]

let controller_meshes = map(10): nil

def drawworld():
    gl_clear(color_grey)
    gl_blend(blend_none)

    gl_light(camera.position, xy { 64.0, 1.0 })
    //gl_light(xyz(sincos(gl_time() * 20), 0) * 100 + xyz_z * 100)

    gl_set_shader("flat")
    for(10) i:
        gl_translate sincos(i * 72.0) * 3.0:
            gl_scale(0.3):
                gl_render_mesh(tree)

    for(balls) b:
        gl_translate b[0]:
            gl_scale(0.1):
                gl_rotate_z(sincos(gl_time() * 30.0)):
                    gl_render_mesh(sphere)

    gl_set_shader("color")
    gl_debug_grid(xyz { 20, 20, 0 }, xyz_1, 0.005)

while gl_frame():
    if gl_button("escape") == 1: return

    let znear = 0.1
    let zfar = 1000
    if vrmode:
        vr_start()
        for(2) e:
            vr_start_eye(e, znear, zfar)
            set_z_up_worldspace():
                drawworld()
            for(vr_num_motion_controllers()) mc:
                if vr_motioncontrollerstracking(mc):
                    var mcmesh = controller_meshes[mc]
                    if not mcmesh:
                        // FIXME: sadly have to do this on the fly because at init controllers and
                        // meshes are not available for some reason.
                        controller_meshes[mc] = mcmesh = vr_create_motion_controller_mesh(mc)
                    if mcmesh:
                        gl_color(color_white)
                        gl_push_model_view()
                        vr_motion_controller(mc)
                        gl_set_shader("phong_textured")
                        gl_render_mesh(mcmesh)
                        gl_pop_model_view()
        vr_finish()
        for(2) mc:
            if vr_motion_controller_button(mc, "trigger") == 1:
                let pos = to_z_up(vr_motion_controller_vec(mc, 3))
                let dir = to_z_up(vr_motion_controller_vec(mc, 2))
                balls.push([ pos, -dir ])
        // Render the two eye textures to the non-VR window for feedback:
        gl_clear(color_black)
        gl_blend(blend_none)
        gl_set_shader("textured")

        for(2) e:
            vr_set_eye_texture(0, e)
            gl_translate(xy { gl_window_size().x * e / 2.0, gl_window_size().y }):
                    gl_rect(xy { gl_window_size().x / 2.0, -gl_window_size().y })
    else:
        // This is how we'd render normally, with no HMD connected:
        gl_cursor(false)
        camera.FPS_update("w", "a", "s", "d", 10.0, 4.0, true)
        gl_clear(color_dark_grey)
        gl_blend(blend_none)
        gl_perspective(70.0, znear, zfar)
        camera.FPS_view()
        drawworld()

    if balls.length > 100:
        balls.remove(0, balls.length - 100)
    for(balls) b:
        b[0] += b[1] * gl_delta_time() / 10.0
