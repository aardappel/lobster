// graphics demo showing Lobster's builtin facilities of generating meshes procedurally from implicit functions
// this functionality is under construction

include "vec.lobster"
include "color.lobster"
include "3dhelpers.lobster"

print "generating models... this may take some time!"

fatal(gl_window("meshgen test (PLEASE WAIT...)", 1280, 800))

camera_position = xyz { 10.0, 10.0, 10.0 }
camera_yaw = 135

def mg_mirror_y(dist, body):
    for(2) i:
        mg_translate([ 0.0, i * dist * 2.0 - dist, 0.0 ], body)

//mg_set_colornoise(0.5, 1)
aspoints := false
mg_set_pointmode(aspoints)
if !aspoints:
    mg_set_polygonreduction(3, 0.98, 0.95)
    mg_set_vertrandomize(0)

density := 80

gun := nil
if true:
    mg_rotate(xyz_y, 90.0):
        mg_scalevec([ 1.0, 1.3, 1.0 ]):
            mg_supertoroid(3.0, [3.0, 3.0, 5.0])
    mg_translate([ 0.0, -1.3, 2.0 ]):
        mg_scalevec([ 0.5, 0.5, 3.5 ]):
            mg_rotate(xyz_x, 20.0):
                mg_superquadric([ 3.0, 3.0, 3.0 ])
    mg_translate([ 0.0, -8.0, 4.0 ]):
        mg_scalevec([ 0.5, 0.5, 2.5 ]):
            mg_rotate(xyz_x, 30.0):
                mg_superquadric([ 3.0, 3.0, 3.0 ])
    mg_translate([ 0.0, -5.5, -3.0 ]):
        mg_scalevec([ 1.5, 2.5, 6.5 ]):
            mg_rotate(xyz_x, -20.0):
                mg_superquadric([ 5.0, 5.0, 100.0 ])
    mg_translate([ 0.0, 2.0, 4.2 ]):
        mg_scalevec([ 2.0, 10.0, 3.0 ]):
            mg_superquadric([ 3.0, 100.0, 3.0 ])
    mg_translate([ 0.0, 14.0, 5.0 ]):
        mg_scalevec([ 1.0, 1.0, 3.0 ]):
            mg_rotate(xyz_x, 90.0):
                mg_cylinder()
    gun = mg_polygonize(density, [ [ 1.0, 1.0, 1.0 ], [ 1.0, 1.0, 1.0 ] ])

spaceship := nil
if true:
    mg_superquadric_non_uniform([ 1.0, 2.0, 2.0 ], [ 2.0, 2.0, 2.0 ], [ 2.0, 3.5, 0.3 ], [ 0.5, 3.5, 0.3 ])
    mg_superquadric_non_uniform([ 2.0, 1.5, 1.0 ], [ 2.0, 1.5, 2.0 ], [ 4.0, 1.0, 0.7 ], [ 0.7, 1.0, 0.4 ])
    mg_translate([ 1.0, 0.0, 0.4 ]):
        mg_superquadric_non_uniform([ 2.0, 2.0, 1.0 ], [ 2.0, 2.0, 2.0 ], [ 1.0, 0.5, 0.4 ], [ 0.5, 0.5, 0.5 ])
    mg_mirror_y(1.5):
        mg_translate(xyz_x):
            mg_scalevec([ 1.3, 0.2, 0.2 ]):
                mg_superquadric([ 100.0, 2.0, 2.0 ])
    mg_mirror_y(2):
        mg_translate(xyz_x):
            mg_scalevec([ 1.0, 0.15, 0.15 ]):
                mg_superquadric([ 100.0, 2.0, 2.0 ])
    mg_mirror_y(1):
        mg_translate(xyz_x * -0.6):
            mg_superquadric_non_uniform([ 1.0, 2.0, 2.0 ], [ 1.0, 2.0, 2.0 ], [ 1.5, 0.5, 0.5 ], [ 0.01, 0.5, 0.5 ])
            mg_fill(0):
                mg_scalevec([ 0.35, 0.35, 0.35 ]):
                    mg_rotate(xyz_y, 90.0):
                        mg_cylinder()
    spaceship = mg_polygonize(density * 2, [ [ 1.0, 1.0, 1.0 ], [ 0.5, 0.5, 1.0 ] ])

def model_tree(numb, branchl, narrowf, leafs):
    if numb:
        mg_scalevec(xyz_1 * [ 1, 1, branchl ]):
            mg_translate(xyz_z):
                mg_tapered_cylinder(1, narrowf)
        mg_translate(xyz_z * branchl * 2):
            axis := sincos(rnd(360))
            branches := rnd(3) + 1
            for(branches) i:
                mg_rotate(xyz_z, 360 * i / branches):
                    mg_rotate(axis, 12 * branches):
                        mg_scalevec(xyz_1 * narrowf):
                            mg_sphere()
                            model_tree(numb - 1, branchl, narrowf, leafs)
    else:
        mg_fill(2):
            mg_scalevec(xyz_1 * leafs):
                mg_sphere()

trees := []

if true:
    model_tree(10, 1.5, 0.75, 12)
    trees.push(mg_polygonize(density, [[ 0.7, 0.6, 0.5 ], [ 0.6, 1.0, 0.6 ]]))

    model_tree(10, 1.2, 0.8, 1.2)
    trees.push(mg_polygonize(density, [[ 0.5, 0.4, 0.3 ], [ 0.8, 1.0, 0.5 ]]))

    model_tree(9, 2.0, 0.7, 15)
    trees.push(mg_polygonize(density, [[ 0.6, 0.5, 0.4 ], [ 0.5, 0.7, 0.5 ]]))

landscape := nil
// Currently expensive way to do a landscape.
if true:
    mg_landscape(0.5, 1.0)
    landscape = mg_polygonize(density, [ [ 0.7, 0.6, 0.5 ], [ 0.5, 1.0, 0.5 ] ])

if gun: print "vertcount: gun " + gl_meshsize(gun)
if spaceship: print "vertcount: ship " + gl_meshsize(spaceship)
for trees: print("vertcount: tree1 " + gl_meshsize(_))

print seconds_elapsed()

while gl_frame():
    if gl_button("escape") == 1: return

    gl_windowtitle("meshgen test: " + gl_deltatime())

    gl_clear(color_dark_grey)

    fovscale := 1.3

    if true:
        gl_cursor(0)
        gl_perspective(60, 0.1, 1000)

        speed := 10
        if gl_button("left shift") >= 1: speed = 50
        camera_FPS_update("w", "a", "s", "d", speed, 4, true)
        camera_FPS_view()
        gl_light(camera_position, [ 128.0, 0.1 ])
    else:
        gl_ortho3d(xyz_0, xyz_f { gl_windowsize().x, gl_windowsize().y, 2000 } / 100)
        gl_rotate_x(sincos(45))
        gl_rotate_z(sincos(45))
        gl_light(xyz(sincos(gl_time() * 10), 0) * 100 + xyz_z * 300, [ 128.0, 0.1 ])

    gl_blend 0:
        gl_setshader(aspoints and "phong_particle" or "flat")

        if spaceship:
            gl_translate(xy_1 * 10):
                gl_pointscale(1 * fovscale)
                gl_rendermesh(spaceship)

        if gun:
            gl_translate(xy_1 * 20):
                gl_scale(0.2):
                    gl_pointscale(0.2 * fovscale)
                    gl_rendermesh(gun)

        if landscape:
            gl_translate(xy_y * 20):
                gl_scale(5):
                    gl_pointscale(5 * fovscale)
                    gl_rendermesh(landscape)

        for(10) j:
            gl_translate(xyz_y * j * -5):
                for(trees) t, i:
                    gl_translate(xyz_x * i * 10):
                        gl_scale(0.5):
                            gl_pointscale(0.5 * fovscale)
                            gl_rendermesh(t)

    gl_setshader("color")
    gl_debug_grid([ 20, 20, 0 ], xyz_1, 0.005)

