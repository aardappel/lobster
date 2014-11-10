include "std.lobster"
include "vec.lobster"
include "color.lobster"

fatal(gl_window("Shooter Tutorial", 640, 480))

worldsize := 20.0
playerpos := xy_0
playerspeed := 10

while(gl_frame() & !gl_wentdown("escape")):
    gl_clear(color_black)
    gl_color(color_white)

    gl_translate(gl_windowsize() / 2.0)
    scale := gl_windowsize().y / worldsize
    gl_scale(scale)

    dir := [ gl_isdown("d") - gl_isdown("a"),
             gl_isdown("s") - gl_isdown("w") ]
    newpos := playerpos + normalize(dir) * gl_deltatime() * playerspeed
    if(!any(abs(newpos) > gl_windowsize() / scale / 2)):
        playerpos = newpos

    gl_translate(playerpos):
        gl_circle(1, 6)
