import std
import vec
import color
import gl

fatal(gl.window("Shooter Tutorial", 640, 480))

let worldsize = 20.0
var playerpos = float2_0
let playerspeed = 10

while gl.frame() and gl.button("escape") != 1:
    gl.clear(color_black)
    gl.color(color_white)

    gl.translate(float(gl.window_size()) / 2.0)
    let scale = gl.window_size().y / worldsize
    gl.scale(scale)

    let dir = float2 { (gl.button("d") >= 1) - (gl.button("a") >= 1),
                  (gl.button("s") >= 1) - (gl.button("w") >= 1) }
    let newpos = playerpos + normalize(dir) * gl.delta_time() * playerspeed
    if not any(abs(newpos) > float(gl.window_size()) / scale / 2):
        playerpos = newpos

    gl.translate playerpos:
        gl.circle(1, 6)
