// graphics demo showing an animated "tree of pythagoras"

import vec
import color
import gl

def branch(n, max, scale, len) -> void:
    let poly = [ xy_x, -xy_x, xy { -scale, len }, xy { scale, len } ]
    gl_color(color { n / max, 0.8, n / max, 1.0 })
    gl_polygon(poly)
    gl_color(color { 0.3, 0.1, 0.0, 0.7 })
    for(2) i:
        gl_line(poly[i], poly[3 - i], 0.2)
    if n < max:
        gl_translate(xy { 0.0, len })
        gl_scale(scale)
        for(2) i:
            gl_rotate_z(sincos((i * 2.0 - 1.0) * 30.0 + sin(gl_time() * 50.0) * n + rnd(40) - 20.0)):
                branch(n + 1, max, scale, len + 0.5)

fatal(gl_window("Pythagoras tree in Lobster", 1024, 768))

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_grey)
    rnd_seed(0)
    gl_translate(float(gl_window_size()) / xy { 2.0, 1.0 })
    gl_rotate_z(xy { -1.0, 0.0 })
    gl_scale(gl_window_size().y / 17.0)
    branch(0, 11.0, 0.7, 4)

