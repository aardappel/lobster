/* random maze pattern inspired on this C64 basic program:

10 PRINT CHR$ (205.5 + RND (1)); : GOTO 10

Sadly, even lobster can't compete with that conciseness :)
http://10print.org/

*/

include "vec.lobster"
include "color.lobster"

fatal(gl_window("rndmaze", 1024, 768))
    
while(gl_frame()):    
    if(gl_wentdown("escape")): return
    gl_clear(color_black)
    rndseed(0)
    scale := 40
    gl_scale(scale)
    forxy(gl_windowsize() / scale) v:
        gl_translate(v):
            if(rnd(4)):     // this is not in the original, and adds some blank spaces.. comment this line out if needed
                if(rnd(2)):
                    gl_line(vec_0, vec_xy, 0.2)
                else:
                    gl_line(vec_x, vec_y, 0.2)
