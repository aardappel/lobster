// simple graphics demo showing euler spirals: http://blog.matthen.com/post/67889183164/draw-a-straight-line-and-then-continue-it-for-the

include "vec.lobster"
include "color.lobster"
include "uihelpers.lobster"

fatal(gl_window("euler spiral", 1024, 768))

pts := [ xy_0, xy_x ]
angle := 0.0

while(gl_frame()):
    if(gl_wentdown("escape")): return
    gl_clear(color_black)
    origin := gl_windowsize() / 2.0
    gl_translate(origin)
    scale := ui_mousewheelorpinchzoom(1.3, 5, 10, 25, origin)
    gl_scale(scale)
    gl_translate(-ui_inertiascroll("mouse1", scale, xy_1 * -1000, xy_1 * 1000, gl_windowsize()))
    if(pts.length < 10000):
        angle += 2.0 + (rndfloat() - 0.5) / 20.0
        a := atan2(pts.top - pts[pts.length - 2])
        pts.push(pts.top + sincos(a + angle).xy)
    gl_linemode(1):
        gl_polygon(pts)
