// simple graphics demo showing hypocycloids: http://en.wikipedia.org/wiki/Hypocycloid

include "vec.lobster"
include "color.lobster"

fatal(gl_window("hypocycloid", 1024, 768))

while(gl_frame()):
    if(gl_wentdown("escape")): return
    gl_clear(color_black)
    gl_translate(gl_windowsize() / 2.0)
    gl_scale(gl_windowsize().y / 4.0)
    scalechange := sin(gl_time() * 50) * 0.2
    pts := map(360 * 4 + 1) a:
        fold(5, xy_0) p, i:
            p + sincos(a / 4.0 * pow(3, i)).xy * pow(0.4 + scalechange, i)
    gl_linemode(1):
        gl_polygon(pts)
