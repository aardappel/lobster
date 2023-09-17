// simple graphics demo showing euler spirals:
// http://blog.matthen.com/post/67889183164/draw-a-straight-line-and-then-continue-it-for-the

import vec
import color
import gl
import utilui

fatal(gl.window("euler spiral", 1024, 768))

let pts = [ float2_0, float2_x ]
var angle = 0.0

let szv = scrollzoomview {
    rate: 1.3,
    mins: 5,
    defaults: 10,
    maxs: 22,
    buttonname: "mouse1"
}

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    let scale, panoffset = szv.update(float2_1 * -50.0, float2_1 * 50.0, gl.window_size())
    gl.scale(scale)
    gl.translate(panoffset)
    if pts.length < 10000:
        angle += 2.0 + (rnd_float() - 0.5) / 20.0
        let a = atan2(pts.top - pts[pts.length - 2])
        pts.push(pts.top + sincos(a + angle))
    gl.line_mode 1:
        gl.polygon(pts)
