/* random maze pattern inspired on this C64 basic program:

10 PRINT CHR$ (205.5 + RND (1)); : GOTO 10

Sadly, even lobster can't compete with that conciseness :)
http://10print.org/

*/

import vec
import color
import gl

fatal(gl.window("rndmaze", 1024, 768))

def endcap_line(pts, thickness):
    gl.line(pts[0], pts[1], thickness)
    for pts: gl.translate _: gl.circle(thickness / 2.0, 40)

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    rnd_seed(0)
    let scale = gl.window_size().y / 25
    gl.scale(scale)
    forxy(gl.window_size() / scale + 1) v:
        gl.translate v:
            if rnd(3):   // this is not in the original, and adds some blank spaces.. comment this line out if needed
                endcap_line(if rnd(2): [ float2_0, float2_1 ]
                                 else: [ float2_x, float2_y ], 0.25)
