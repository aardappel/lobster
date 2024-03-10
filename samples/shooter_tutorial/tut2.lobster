import std
import vec
import color
import gl

fatal(gl.window("Shooter Tutorial", 640, 480))

let worldsize = 20.0

while gl.frame() and gl.button("escape") != 1:
    gl.clear(color_black)
    gl.color(color_white)
    gl.translate(float(gl.window_size()) / 2.0)
    gl.scale(gl.window_size().y / worldsize)
    gl.circle(1.0, 6)
