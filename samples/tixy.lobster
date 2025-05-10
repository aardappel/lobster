import vec
import color
import gl
import imgui

// Inspired by https://tixy.land/

gl.window("Tixy", 1024, 768)
im.init(false, 0, 3.0)
assert im.add_font("data/fonts/Droid_Sans/DroidSans.ttf", 20.0)

let gridsize = 33

var use_x = 0.0
var use_y = 0.0
var use_x_times_y = 0.0
var use_x_times_x = 0.2
var use_y_times_y = 0.2
var use_i = 0.0
var use_sqrt = 0.0
var use_rand = 0.0
var use_time = 1.0

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    let res = gl.window_size()
    let scale = float(res.y) / (gridsize + 1)
    rnd_seed(0)
    gl.scale scale:
        gl.translate float2_1
        forxy(int2_1 * gridsize) vp:
            let v = vp - gridsize / 2
            var in = 0.0
            in += use_x * v.x
            in += use_y * v.y
            in += use_x_times_y * v.x * v.y
            in += use_x_times_x * v.x * v.x
            in += use_y_times_y * v.y * v.y
            in += use_i * (v.x + v.y * gridsize)
            in = use_sqrt * sqrt(in) + (1.0 - use_sqrt) * in
            in += use_rand * (rnd_float() * 3.14159 * 2.0)
            in += use_time * gl.time()
            let o = sin(in * 90.0 / 3.14159)
            gl.color(color { 1.0, o > 0.0, o > 0.0, 1.0 })
            gl.translate float(vp):
                gl.circle(abs(o) * 0.5, 20)
    im.frame():
        im.next_window_size(float2 { res.x - res.y, res.y }, 0)
        im.next_window_pos(float2_x * (gridsize + 1) * scale, float2_0, 0)
        im.window("Edit Formula", im.window_no_collapse):
            use_x = im.sliderfloat("x", use_x, 0.0, 4.0)
            use_y = im.sliderfloat("y", use_y, 0.0, 4.0)
            use_x_times_y = im.sliderfloat("x * y", use_x_times_y, 0.0, 4.0)
            use_x_times_x = im.sliderfloat("x * x", use_x_times_x, 0.0, 4.0)
            use_y_times_y = im.sliderfloat("y * y", use_y_times_y, 0.0, 4.0)
            use_i = im.sliderfloat("index", use_i, 0.0, 4.0)
            use_sqrt = im.sliderfloat("sqrt (on above)", use_sqrt, 0.0, 4.0)
            use_rand = im.sliderfloat("random", use_rand, 0.0, 4.0)
            use_time = im.sliderfloat("time", use_time, 0.0, 4.0)
