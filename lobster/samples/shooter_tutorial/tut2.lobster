include "std.lobster"
include "vec.lobster"
include "color.lobster"

fatal(gl_window("Shooter Tutorial", 640, 480))

worldsize := 20.0

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)
    gl_color(color_white)
    gl_translate(float(gl_windowsize()) / 2.0)
    gl_scale(gl_windowsize().y / worldsize)
    gl_circle(1, 6)
