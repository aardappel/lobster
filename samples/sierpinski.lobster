// graphics demo showing a sierpinski triangle: http://en.wikipedia.org/wiki/Sierpinski_triangle

import vec
import color
import gl

fatal(gl.window("sierpinski", 512, 512))

let directions = [ float2_0, float2_x, float2_y ]
let poly = gl.new_poly(directions)

def sierpinski(depth) -> void:
    if depth:
        gl.scale 0.5:
            for(directions) d:
                gl.translate d:
                    sierpinski(depth - 1)
    else:
        gl.render_mesh(poly)

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    gl.scale(float(gl.window_size()))
    sierpinski(7)
