include "std.lobster"
include "vec.lobster"
include "color.lobster"

fatal(gl_window("Shooter Tutorial", 640, 480))

worldsize := 20.0
playerpos := xy_0
playerspeed := 10

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)
    gl_color(color_white)

    gl_translate(float(gl_windowsize()) / 2.0)
    scale := gl_windowsize().y / worldsize
    gl_scale(scale)

    dir := xy_f { (gl_button("d") >= 1) - (gl_button("a") >= 1),
                  (gl_button("s") >= 1) - (gl_button("w") >= 1) }
    newpos := playerpos + normalize(dir) * gl_deltatime() * playerspeed
    if !any(abs(newpos) > float(gl_windowsize()) / scale / 2):
        playerpos = newpos

    gl_translate playerpos:
        gl_circle(1, 6)
