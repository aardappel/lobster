import std
import vec
import color
import gl

fatal(gl_window("Shooter Tutorial", 640, 480))

let worldsize = 20.0
var playerpos = float2_0
let playerspeed = 10

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)
    gl_color(color_white)

    gl_translate(float(gl_window_size()) / 2.0)
    let scale = gl_window_size().y / worldsize
    gl_scale(scale)

    let dir = float2 { (gl_button("d") >= 1) - (gl_button("a") >= 1),
                  (gl_button("s") >= 1) - (gl_button("w") >= 1) }
    let newpos = playerpos + normalize(dir) * gl_delta_time() * playerspeed
    if not any(abs(newpos) > float(gl_window_size()) / scale / 2):
        playerpos = newpos

    gl_translate playerpos:
        gl_circle(1, 6)
