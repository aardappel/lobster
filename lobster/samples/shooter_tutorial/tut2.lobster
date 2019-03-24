include std
include vec
include color

fatal(gl_window("Shooter Tutorial", 640, 480))

let worldsize = 20.0

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)
    gl_color(color_white)
    gl_translate(float(gl_window_size()) / 2.0)
    gl_scale(gl_window_size().y / worldsize)
    gl_circle(1, 6)
