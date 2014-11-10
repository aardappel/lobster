// graphics demo showing Lobster's builtin facilities of generating meshes procedurally from implicit functions
// this functionality is under construction

include "vec.lobster"
include "color.lobster"
include "3dhelpers.lobster"

print("generating models... this may take some time!")

fatal(gl_window("meshgen test (PLEASE WAIT...)", 1280, 1024))

camera_position = [ -10, -10, 10 ]:xyz
camera_yaw = 135

function mg_scale(f, body): mg_scalevec(xyz_1 * f, body)

function mg_mirror_y(dist, body):
    for(2) i:
        mg_translate([ 0, i * dist * 2 - dist, 0 ], body)

mg_set_vertrandomize(0)
mg_set_colornoise(0.5, 1)
mg_set_polygonreduction(100, 0.98, 0.95)

gun := nil
if(true):
    mg_rotate(xyz_y, 90):
        mg_scalevec([ 1, 1.3, 1 ]):
            mg_supertoroid(3, [3, 3, 5])
    mg_translate([ 0, -1.3, 2 ]):
        mg_scalevec([ 0.5, 0.5, 3.5 ]):
            mg_rotate(xyz_x, 20):
                mg_superquadric([ 3, 3, 3 ])
    mg_translate([ 0, -8, 4 ]):
        mg_scalevec([ 0.5, 0.5, 2.5 ]):
            mg_rotate(xyz_x, 30):
                mg_superquadric([ 3, 3, 3 ])
    mg_translate([ 0, -5.5, -3 ]):
        mg_scalevec([ 1.5, 2.5, 6.5 ]):
            mg_rotate(xyz_x, -20):
                mg_superquadric([ 5, 5, 100 ])
    mg_translate([ 0, 2, 4.2 ]):
        mg_scalevec([ 2, 10, 3 ]):
            mg_superquadric([ 3, 100, 3 ])
    mg_translate([ 0, 14, 5 ]):
        mg_scalevec([ 1, 1, 3 ]):
            mg_rotate(xyz_x, 90):
                mg_cylinder()
    gun = mg_polygonize(256, [[1.0, 1.0, 1.0]])

spaceship := nil
if(true):
    mg_superquadric_non_uniform([ 1, 2, 2], [ 2, 2, 2], [ 2, 3.5, 0.3 ], [ 0.5, 3.5, 0.3 ])
    mg_superquadric_non_uniform([ 2, 1.5, 1], [ 2, 1.5, 2], [ 4, 1, 0.7 ], [ 0.7, 1, 0.4 ])
    mg_translate([ 1, 0, 0.4 ]):
        mg_superquadric_non_uniform([ 2, 2, 1 ], [ 2, 2, 2 ], [ 1, 0.5, 0.4 ], [ 0.5, 0.5, 0.5 ])
    mg_mirror_y(1.5):
        mg_translate(xyz_x):
            mg_scalevec([ 1.3, 0.2, 0.2 ]):
                mg_superquadric([ 100, 2, 2 ])
    mg_mirror_y(2):
        mg_translate(xyz_x):
            mg_scalevec([ 1, 0.15, 0.15 ]):
                mg_superquadric([ 100, 2, 2 ])
    mg_mirror_y(1):
        mg_translate(xyz_x * -0.6):
            mg_superquadric_non_uniform([ 1, 2, 2 ], [ 1, 2, 2 ], [ 1.5, 0.5, 0.5 ], [ 0.01, 0.5, 0.5 ])
            mg_fill(0):
                mg_scalevec([ 0.35, 0.35, 0.35 ]):
                    mg_rotate(xyz_y, 90):
                        mg_cylinder()
    spaceship = mg_polygonize(256, [[1.0, 1.0, 1.0]])

function model_tree(numb, branchl, narrowf, leafs):
    if(numb):
        mg_scale([ 1, 1, branchl ]):
            mg_translate(xyz_z):
                mg_tapered_cylinder(1, narrowf)
        mg_translate(xyz_z * branchl * 2):
            axis := sincos(rnd(360))
            branches := rnd(3) + 1
            for(branches) i:
                mg_rotate(xyz_z, 360 * i / branches):
                    mg_rotate(axis, 12 * branches):
                        mg_scale(narrowf):
                            mg_sphere()
                            model_tree(numb - 1, branchl, narrowf, leafs)
    else:
        mg_fill(2):
            mg_scale(leafs):
                mg_sphere()

density := 100

trees := [ nil, nil, nil ]

if(true):
    model_tree(10, 1.5, 0.75, 12)
    trees[0] = mg_polygonize(density, [[0.7, 0.6, 0.5], [0.6, 1.0, 0.6]])

    model_tree(10, 1.2, 0.8, 1.2)
    trees[1] = mg_polygonize(density, [[0.5, 0.4, 0.3], [0.8, 1.0, 0.5]])

    model_tree(9, 2.0, 0.7, 15)
    trees[2] = mg_polygonize(density, [[0.6, 0.5, 0.4], [0.5, 0.7, 0.5]])

print("vertcounts: gun " + gl_meshsize(gun) +
               " / ship " + gl_meshsize(spaceship) +
               " / tree1 " + gl_meshsize(trees[0]) +
               " / tree2 " + gl_meshsize(trees[1]) +
               " / tree3 " + gl_meshsize(trees[2]))

print(seconds_elapsed())

gl_windowtitle("meshgen test")

while(gl_frame()):
    if(gl_wentdown("escape")): return

    gl_clear(color_dark_grey)
    gl_cursor(0)
    gl_perspective(70, 0.1, 1000)

    speed := 10
    if(gl_isdown("left shift")): speed = 50
    camera_FPS_update("w", "a", "s", "d", speed, 4, true)
    camera_FPS_view()

    gl_light(camera_position)
    //gl_light(sincos(gl_time() * 20) * 100 + xyz_z * 100)

    rndseed(0)
    gl_blend(0):
        gl_setshader("phong")

        if(spaceship):
            gl_translate(xy_1 * 10):
                gl_rendermesh(spaceship)

        if(gun):
            gl_translate(xy_1 * 20):
                gl_scale(0.1):
                    gl_rendermesh(gun)

        for(trees) t, i:
            if(t):
                gl_translate(xyz_x * i * 10):
                    gl_scale(0.5):
                        gl_rendermesh(t)

        gl_setshader("color")

    gl_debug_grid([20, 20, 0], xyz_1, 0.005)

