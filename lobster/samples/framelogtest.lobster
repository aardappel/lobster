
include "vec.lobster"
include "color.lobster"

fatal(gl_window("logtest", 1024, 1024))

while gl_frame():

    if gl_button("escape") == 1: return

    gl_clear(color_black)
    gl_color(color_white)

    def a():
        v ?= 0
        v++

    def b():
        i := a()
        if a() % 4:
            v, w ?= 0, 1
            v++
            print v + " | " + w
        else:
            nil
            print "--- " + i

    b()
