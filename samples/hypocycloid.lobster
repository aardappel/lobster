// simple graphics demo showing hypocycloids: http://en.wikipedia.org/wiki/Hypocycloid

import vec
import color
import gl

fatal(gl.window("hypocycloid", 1024, 768))

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    gl.translate(float(gl.window_size()) / 2.0)
    gl.scale(gl.window_size().y / 4.0)
    let scalechange = sin(gl.time() * 50.0) * 0.2
    let pts = map(360 * 4 + 1) a:
        fold(5, float2_0) p, i:
            p + sincos(a / 4.0 * pow(3.0, i)) * pow(0.4 + scalechange, i)
    gl.line_mode 1:
        gl.polygon(pts)
