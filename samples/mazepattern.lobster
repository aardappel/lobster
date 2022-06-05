/* random maze pattern inspired on this C64 basic program:

10 PRINT CHR$ (205.5 + RND (1)); : GOTO 10

Sadly, even lobster can't compete with that conciseness :)
http://10print.org/

*/

import vec
import color
import gl

fatal(gl_window("rndmaze", 1024, 768))

def endcap_line(pts, thickness):
    gl_line(pts[0], pts[1], thickness)
    for pts: gl_translate _: gl_circle(thickness / 2.0, 40)

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_black)
    rnd_seed(0)
    let scale = gl_window_size().y / 25
    gl_scale(scale)
    forxy(gl_window_size() / scale + 1) v:
        gl_translate float(v):
            if rnd(3):   // this is not in the original, and adds some blank spaces.. comment this line out if needed
                endcap_line(if rnd(2): [ xy_0, xy_1 ]
                                 else: [ xy_x, xy_y ], 0.25)
