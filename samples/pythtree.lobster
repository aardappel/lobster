// graphics demo showing an animated "tree of pythagoras"

import vec
import color
import gl

def branch(n, max, scale, len) -> void:
    let poly = [ float2_x, -float2_x, float2 { -scale, len }, float2 { scale, len } ]
    gl.color(color { n / max, 0.8, n / max, 1.0 })
    gl.polygon(poly)
    gl.color(color { 0.3, 0.1, 0.0, 0.7 })
    for(2) i:
        gl.line(poly[i], poly[3 - i], 0.2)
    guard n < max
    gl.translate(float2 { 0.0, len })
    gl.scale(scale)
    for(2) i:
        gl.rotate_z(sincos((i * 2.0 - 1.0) * 30.0 + sin(gl.time() * 50.0) * n + rnd(40) - 20.0)):
            branch(n + 1, max, scale, len + 0.5)

fatal(gl.window("Pythagoras tree in Lobster", 1024, 768))

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_grey)
    rnd_seed(0)
    gl.translate(float(gl.window_size()) / float2 { 2.0, 1.0 })
    gl.rotate_z(float2 { -1.0, 0.0 })
    gl.scale(gl.window_size().y / 17.0)
    branch(0, 11.0, 0.7, 4)

