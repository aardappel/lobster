import vec
import color
import gl
import imgui
import camera

// Inspired by https://tixy.land/

gl.window("Tixy3D", 1024, 768)
im.init(false, 0, 3.0)
assert im.add_font("data/fonts/Droid_Sans/DroidSans.ttf", 20.0)

let camera = Camera { float3_1 * 22.0, 45.0, 0.0 }

var gridsize = 11

var use_x = 0.0
var use_y = 0.0
var use_z = 0.0
var use_x_times_y_z = 0.0
var use_x_times_x = 0.3
var use_y_times_y = 0.3
var use_z_times_z = 0.3
var use_i = 0.0
var use_sqrt = 0.8
var use_rand = 0.0
var use_time = 1.0

while gl.frame():
    if gl.button("escape") == 1: return
    gl.window_title("{int(1.0 / gl.delta_time_rolling(64))}")
    gl.clear(color_black)
    let res = gl.window_size()
    rnd_seed(0)

    camera.position = float3_1 * (gridsize * 1.3)
    gl.perspective(65.0, 1.0, 100.0)
    camera.direction(float3 { -gridsize * 0.5, 0.0, -gridsize * 0.5 }  - camera.position)
    camera.FPS_view()

    gl.light(camera.position + float3 { 0.0, 5.0, 10.0 }, float2 { 64.0, 0.25 })
    gl.set_shader("phong_pnt")

    forxyz(int3_1 * gridsize) vp:
        let v = vp - gridsize / 2
        var in = 0.0
        in += use_x * v.x
        in += use_y * v.y
        in += use_z * v.z
        in += use_x_times_y_z * v.x * v.y * v.z
        in += use_x_times_x * v.x * v.x
        in += use_y_times_y * v.y * v.y
        in += use_z_times_z * v.z * v.z
        in += use_i * (v.x + v.y * gridsize + v.z * gridsize * gridsize)
        in = use_sqrt * sqrt(in) + (1.0 - use_sqrt) * in
        in += use_rand * (rnd_float() * 3.14159 * 2.0)
        in += use_time * gl.time()
        let o = sin(in * 90.0 / 3.14159)
        gl.color(color { 1.0, o > 0.0, o > 0.0, 1.0 })
        let size = abs(o)
        gl.translate float(vp) - size * 0.5:
            gl.scale size:
                gl.unit_cube()

    gl.ortho()

    im.frame():
        let imw = 300.0
        im.next_window_size(float2 { imw, res.y }, 0)
        im.next_window_pos(float2_x * (res.x - imw), float2_0, 0)
        im.window("Edit Formula", im.window_no_collapse):
            use_x = im.sliderfloat("x", use_x, 0.0, 4.0)
            use_y = im.sliderfloat("y", use_y, 0.0, 4.0)
            use_z = im.sliderfloat("z", use_z, 0.0, 4.0)
            use_x_times_y_z = im.sliderfloat("x * y * z", use_x_times_y_z, 0.0, 4.0)
            use_x_times_x = im.sliderfloat("x * x", use_x_times_x, 0.0, 4.0)
            use_y_times_y = im.sliderfloat("y * y", use_y_times_y, 0.0, 4.0)
            use_z_times_z = im.sliderfloat("z * z", use_z_times_z, 0.0, 4.0)
            use_i = im.sliderfloat("index", use_i, 0.0, 4.0)
            use_sqrt = im.sliderfloat("sqrt (on above)", use_sqrt, 0.0, 4.0)
            use_rand = im.sliderfloat("random", use_rand, 0.0, 4.0)
            use_time = im.sliderfloat("time", use_time, 0.0, 4.0)
            gridsize = im.sliderint("grid size", gridsize, 5, 25)