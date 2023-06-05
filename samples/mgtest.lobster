// graphics demo showing Lobster's builtin facilities of generating meshes procedurally from implicit functions
// this functionality is under construction

import vec
import color
import gl
import camera
import gl
import mg

print "generating models... this may take some time!"

fatal(gl_window("meshgen test (PLEASE WAIT...)", 1280, 800, gl_window_init_linear_color))

let camera = Camera { xyz_1 * 10.0, 135.0, 0.0 }

def mg_mirror_y(dist, body):
    for(2) i:
        mg_translate(xyz_f { 0.0, i * dist * 2.0 - dist, 0.0 }, body)

//mg_set_color_noise(0.5, 1)
let aspoints = false
mg_set_point_mode(aspoints)
if not aspoints:
    //mg_set_polygon_reduction(3, 0.98, 0.95)
    mg_set_vertex_randomize(0.0)

let density = 80

var rounded_cube = nil
if true:
    mg_smooth(1.0)
    mg_color(color_white)
    mg_rotate(xyz_1, 45):
        mg_superquadric(xyz_1 * 10.0, xyz_1)
    rounded_cube = mg_polygonize(density)

var gun = nil
if true:
    mg_smooth(0.5)
    mg_color(color_white)
    mg_rotate(xyz_y, 90.0):
        mg_scale_vec(xyz_f { 1.0, 1.3, 1.0 }):
            mg_supertoroid(3.0, xyz_f { 3.0, 3.0, 5.0 })
    mg_translate(xyz_f { 0.0, -1.3, 2.0 }):
        mg_rotate(xyz_x, 20.0):
            mg_superquadric(xyz_f { 3.0, 3.0, 3.0 }, xyz_f { 0.5, 0.5, 3.5 })
    mg_translate(xyz_f { 0.0, -8.0, 4.0 }):
        mg_rotate(xyz_x, 30.0):
            mg_superquadric(xyz_f { 3.0, 3.0, 3.0 }, xyz_f { 0.5, 0.5, 2.5 })
    mg_translate(xyz_f { 0.0, -5.5, -3.0 }):
        mg_rotate(xyz_x, -20.0):
            mg_superquadric(xyz_f { 5.0, 5.0, 100.0 }, xyz_f { 1.5, 2.5, 6.5 })
    mg_translate(xyz_f { 0.0, 2.0, 4.2 }):
        mg_superquadric(xyz_f { 3.0, 100.0, 3.0 }, xyz_f { 2.0, 10.0, 3.0 })
    mg_translate(xyz_f { 0.0, 14.0, 5.0 }):
        mg_rotate(xyz_x, 90.0):
            mg_cylinder(1.0, 3.0)
    gun = mg_polygonize(density)

var spaceship = nil
if true:
    mg_smooth(0.5)
    mg_color(color_white)
    mg_superquadric_non_uniform(xyz_f { 1.0, 2.0, 2.0 }, xyz_f { 2.0, 2.0, 2.0 },
                                xyz_f { 2.0, 3.5, 0.3 }, xyz_f { 0.5, 3.5, 0.3 })
    mg_superquadric_non_uniform(xyz_f { 2.0, 1.5, 1.0 }, xyz_f { 2.0, 1.5, 2.0 },
                                xyz_f { 4.0, 1.0, 0.7 }, xyz_f { 0.7, 1.0, 0.4 })
    mg_translate(xyz_f { 1.0, 0.0, 0.4 }):
        mg_superquadric_non_uniform(xyz_f { 2.0, 2.0, 1.0 }, xyz_f { 2.0, 2.0, 2.0 },
                                    xyz_f { 1.0, 0.5, 0.4 }, xyz_f { 0.5, 0.5, 0.5 })
    mg_mirror_y(1.5):
        mg_translate(xyz_x):
            mg_superquadric(xyz_f { 100.0, 2.0, 2.0 }, xyz_f { 1.3, 0.2, 0.2 })
    mg_mirror_y(2):
        mg_translate(xyz_x):
            mg_superquadric(xyz_f { 100.0, 2.0, 2.0 }, xyz_f { 1.0, 0.15, 0.15 })
    mg_mirror_y(1):
        mg_translate(xyz_x * -0.6):
            mg_superquadric_non_uniform(xyz_f { 1.0, 2.0, 2.0 }, xyz_f { 1.0, 2.0, 2.0 },
                                        xyz_f { 1.5, 0.5, 0.5 }, xyz_f { 0.01, 0.5, 0.5 })
            mg_color(color { 1.0, 1.0, 1.0, 0.0 /* carve */ }):
                mg_rotate(xyz_y, 90.0):
                    mg_smooth(0.0):
                        mg_cylinder(0.35, 0.25)
    spaceship = mg_polygonize(density * 2)

def model_tree(numb, branchl, narrowf, leafs, leafc) -> void:
    if numb:
        mg_translate(xyz_z * branchl):
            mg_tapered_cylinder(1.0, narrowf, branchl)
            mg_translate(xyz_z * branchl):
                let axis = xyz(sincos(rnd(360)), 0)
                let branches = rnd(3) + 1
                for(branches) i:
                    mg_rotate(xyz_z, 360 * i / branches):
                        mg_rotate(axis, 12 * branches):
                            mg_scale(narrowf):
                                mg_sphere(1.0)
                                model_tree(numb - 1, branchl, narrowf, leafs, leafc)
    else:
        mg_color(leafc):
            mg_sphere(leafs)

let trees = []

if true:
    mg_smooth(0.5)
    mg_color(color { 0.6, 0.5, 0.4, 1.0 })
    model_tree(10, 1.5, 0.75, 12, color { 0.4, 1.0, 0.4, 1.0 })
    trees.push(mg_polygonize(density))

    mg_smooth(0.5)
    mg_color(color { 0.4, 0.3, 0.2, 1.0 })
    model_tree(10, 1.2, 0.8, 1.2, color { 0.6, 1.0, 0.3, 1.0 })
    trees.push(mg_polygonize(density))

    mg_smooth(0.5)
    mg_color(color { 0.5, 0.4, 0.3, 1.0 })
    model_tree(9, 2.0, 0.7, 15, color { 0.4, 0.6, 0.4, 1.0 })
    trees.push(mg_polygonize(density))

var landscape = nil
// Currently expensive way to do a landscape.
if true:
    mg_smooth(0.1)
    mg_color(color { 0.6, 0.5, 0.4, 1.0 })
    mg_landscape(0.5, 1.0)
    landscape = mg_polygonize(density)

if gun: print "vertcount: gun {gl_mesh_size(gun)}"
if spaceship: print "vertcount: ship {gl_mesh_size(spaceship)}"
for trees: print("vertcount: tree1 {gl_mesh_size(_)}")
if rounded_cube: print "vertcount: rounded_cube {gl_mesh_size(rounded_cube)}"

print seconds_elapsed()

while gl_frame():
    if gl_button("escape") == 1: return

    gl_window_title("meshgen test: {gl_delta_time()}")

    gl_clear(color_darkest_grey)

    let fovscale = 1.5

    if true:
        gl_cursor(false)
        gl_perspective(60.0, 0.1, 1000.0)

        var speed = 10
        if gl_button("left shift") >= 1: speed = 50
        camera.FPS_update("w", "a", "s", "d", speed, 4.0, true)
        camera.FPS_view()
        gl_light(camera.position, xy_f { 128.0, 0.1 })
    else:
        gl_ortho3d(xyz_0, xyz_f { gl_window_size().x, gl_window_size().y, 2000 } / 100)
        gl_rotate_x(sincos(45))
        gl_rotate_z(sincos(45))
        gl_light(xyz(sincos(gl_time() * 10), 0) * 100 + xyz_z * 300, xy_f { 128.0, 0.1 })

    gl_blend 0:
        gl_set_shader(aspoints and "phong_particle" or "flat")

        if spaceship:
            gl_translate(xy_1 * 10.0):
                gl_point_scale(1.0 * fovscale)
                gl_render_mesh(spaceship)

        if gun:
            gl_translate(xy_1 * 20.0):
                gl_scale(0.2):
                    gl_point_scale(0.2 * fovscale)
                    gl_render_mesh(gun)

        if landscape:
            gl_translate(xy_y * 30.0):
                gl_scale(10):
                    gl_point_scale(10.0 * fovscale)
                    gl_render_mesh(landscape)

        if rounded_cube:
            gl_translate(xy_y * 10.0):
                gl_scale(2):
                    gl_point_scale(3.0 * fovscale)
                    gl_render_mesh(rounded_cube)

        for(10) j:
            gl_translate(xyz_y * j * -5):
                for(trees) t, i:
                    gl_translate(xyz_x * i * 10.0):
                        gl_scale(0.5):
                            gl_point_scale(0.5 * fovscale)
                            gl_render_mesh(t)

    gl_set_shader("color")
    gl_debug_grid(xyz_i { 20, 20, 0 }, xyz_1, 0.005)

