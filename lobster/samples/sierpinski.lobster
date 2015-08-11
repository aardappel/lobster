// graphics demo showing a sierpinski triangle: http://en.wikipedia.org/wiki/Sierpinski_triangle

include "vec.lobster"
include "color.lobster"

directions := [ xy_0, xy_x, xy_y ]

def sierpinski(depth):
    if(depth):
        gl_scale(0.5):
            for(directions) d:
                gl_translate(d):
                    sierpinski(depth - 1)
    else:
        gl_polygon(directions)

fatal(gl_window("sierpinski", 512, 512))

while(gl_frame()):
    if(gl_wentdown("escape")): return
    gl_clear(color_black)
    gl_scale(float(gl_windowsize()))
    sierpinski(7)
