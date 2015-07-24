/* random maze pattern inspired on this C64 basic program:

10 PRINT CHR$ (205.5 + RND (1)); : GOTO 10

Sadly, even lobster can't compete with that conciseness :)
http://10print.org/

*/

include "vec.lobster"
include "color.lobster"

fatal(gl_window("rndmaze", 1024, 768))

def endcap_line(a, b, thickness):
    extension := (a - b) * thickness * sqrt(2) / 4
    gl_line(a + extension, b - extension, thickness)

while(gl_frame()):
    if(gl_wentdown("escape")): return
    gl_clear(color_black)
    rndseed(0)
    scale := gl_windowsize().y / 25
    gl_scale(scale)
    forxy(gl_windowsize() / scale) v:
        gl_translate(float(v)):
            if(rnd(3)):   // this is not in the original, and adds some blank spaces.. comment this line out if needed
                if(rnd(2)):
                    endcap_line(xy_0, xy_1, 0.25)
                else:
                    endcap_line(xy_x, xy_y, 0.25)
