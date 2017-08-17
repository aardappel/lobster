// graphics demo showing Lobster's built-in loading of (animated) .iqm models

include "vec.lobster"
include "color.lobster"
include "texture.lobster"
include "3dhelpers.lobster"

vrmode := vr_init()

fatal(gl_window(if vrmode: "VR Test" else: "No VR device", 1280, 640, false, vrmode, 16 /*samples*/))

// Make a quick sample scene out of some trees.

def model_tree(numb, branchl, narrowf, leafs):
    if numb:
        mg_translate(xyz_z * branchl):
            mg_tapered_cylinder(1, narrowf, branchl)
        mg_translate(xyz_z * branchl * 2):
            axis := sincos(rnd(360))
            branches := rnd(3) + 1
            for(branches) i:
                mg_rotate(xyz_z, 360 * i / branches):
                    mg_rotate(axis, 12 * branches):
                        mg_scale(narrowf):
                            mg_sphere(1)
                            model_tree(numb - 1, branchl, narrowf, leafs)
    else:
        mg_fill(2):
            mg_sphere(leafs)

model_tree(10, 1.5, 0.75, 12)
tree := mg_polygonize(50, [[ 0.7, 0.6, 0.5 ], [ 0.6, 1.0, 0.6 ]])

mg_sphere(1)
sphere := mg_polygonize(3, [ color_red ])

camera_position = xyz { 0.0, 0.0, 3.0 }
camera_yaw = 135
camera_pitch = 20

balls := []::[xyz_f]

controller_meshes := map(10): nil

def drawworld():
    gl_clear(color_grey)
    gl_blend(blend_none)

    gl_light(camera_position, [ 64.0, 1.0 ])
    //gl_light(xyz(sincos(gl_time() * 20), 0) * 100 + xyz_z * 100)

    gl_setshader("flat")
    for(10) i:
        gl_translate sincos(i * 72) * 3:
            gl_scale(0.3):
                gl_rendermesh(tree)

    for(balls) b:
        gl_translate b[0]:
            gl_scale(0.1):
                gl_rotate_z(sincos(gl_time() * 30)):
                    gl_rendermesh(sphere)

    gl_setshader("color")
    gl_debug_grid([ 20, 20, 0 ], xyz_1, 0.005)

while gl_frame():
    if gl_button("escape") == 1: return

    znear := 0.1
    zfar := 1000
    if vrmode:
        vr_start()
        for(2) e:
            vr_starteye(e, znear, zfar)
            camera_set_z_up_worldspace():
                drawworld()
            for(vr_nummotioncontrollers()) mc:
                if vr_motioncontrollerstracking(mc):
                    mcmesh := controller_meshes[mc]
                    if !mcmesh:
                        // FIXME: sadly have to do this on the fly because at init controllers and
                        // meshes are not available for some reason.
                        controller_meshes[mc] = mcmesh = vr_createmotioncontrollermesh(mc)
                    if mcmesh:
                        gl_color(color_white)
                        vr_motioncontroller(mc):
                            gl_setshader("phong_textured")
                            gl_rendermesh(mcmesh)
        vr_finish()
        for(2) mc:
            if vr_motioncontrollerbutton(mc, "trigger") == 1:
                pos := to_z_up(vr_motioncontrollervec(mc, 3))
                dir := to_z_up(vr_motioncontrollervec(mc, 2))
                balls.push([ pos, -dir ])
        // Render the two eye textures to the non-VR window for feedback:
        gl_clear(color_black)
        gl_blend(blend_none)
        gl_setshader("textured")

        for(2) e:
            vr_seteyetex(0, e)
            gl_translate([ gl_windowsize().x * e / 2.0, gl_windowsize().y ]):
                    gl_rect([ gl_windowsize().x / 2.0, -gl_windowsize().y ])
    else:
        // This is how we'd render normally, with no HMD connected:
        gl_cursor(0)
        camera_FPS_update("w", "a", "s", "d", 10, 4, true)
        gl_clear(color_dark_grey)
        gl_blend(blend_none)
        gl_perspective(70, znear, zfar)
        camera_FPS_view()
        drawworld()

    if balls.length > 100:
        balls.remove(0, balls.length - 100)
    for(balls) b:
        b[0] += b[1] * gl_deltatime() / 10
