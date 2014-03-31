// graphics demo showing a sierpinski triangle: http://en.wikipedia.org/wiki/Sierpinski_triangle

include "vec.lobster"
include "color.lobster"

directions := [ vec_0, vec_x, vec_y ]

function sierpinski(depth):
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
    gl_scale(gl_windowsize())
    sierpinski(7)
