// graphics demo showing a sierpinski triangle: http://en.wikipedia.org/wiki/Sierpinski_triangle

import vec
import color
import gl

let directions = [ xy_0, xy_x, xy_y ]

def sierpinski(depth) -> void:
    if depth:
        gl_scale 0.5:
            for(directions) d:
                gl_translate d:
                    sierpinski(depth - 1)
    else:
        gl_polygon(directions)

fatal(gl_window("sierpinski", 512, 512))

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_black)
    gl_scale(float(gl_window_size()))
    sierpinski(7)
