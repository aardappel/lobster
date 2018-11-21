// simple graphics demo showing hypocycloids: http://en.wikipedia.org/wiki/Hypocycloid

include vec
include color

fatal(gl_window("hypocycloid", 1024, 768))

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_black)
    gl_translate(float(gl_window_size()) / 2.0)
    gl_scale(gl_window_size().y / 4.0)
    scalechange := sin(gl_time() * 50) * 0.2
    pts := map(360 * 4 + 1) a:
        fold(5, xy_0) p, i:
            p + sincos(a / 4.0 * pow(3, i)) * pow(0.4 + scalechange, i)
    gl_line_mode 1:
        gl_polygon(pts)
