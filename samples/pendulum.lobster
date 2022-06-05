// based on: http://www.physicsandbox.com/projects/double-pendulum.html

import std
import vec
import color
import gl
import texture

let g = 9.8
let mass = 1
let max_steps = 2000

class link:
    len = 180.0
    dtheta = 0.0
    theta = 181.0

    def ddl(): return dtheta * dtheta * len

let links = map(2): link {}

let pts = []::xy_f

fatal(gl_window("double pendulum", 800, 800))
while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_black)
    let time = gl_delta_time() * 4.0

    let sc = sincos(links[0].theta - links[1].theta)
    for(links) l, i:
        let o = links[1 - i]
        let d2theta =  ((i + 1) * g * (sin(o.theta) * sc.x - (2.0 - i) * sin(l.theta)) +
                     (i * 2.0 - 1.0) * ((i + 1) * ddl(o) + ddl(l) * sc.x) * sc.y) /
                    (l.len * (2.0 - sc.x * sc.x))
        l.dtheta += d2theta * time
        l.theta += degrees(l.dtheta * time)

    gl_translate(float(gl_window_size()) / 2.0)

    gl_blend(blend_add):
        for(pts) pos, i:
            gl_color(color { 0.15, 0.075, 0.0, 1.0 } * i / max_steps)
            gl_translate(pos):
                gl_circle(10.0, 10)

    gl_color(color_white)
    var pos = xy_0
    for(links) l:
        let p = yx(sincos(l.theta)) * l.len
        gl_line(xy_0, p, 2.0)
        gl_translate(p)
        gl_circle(4.0, 10)
        pos += p
        pts.push(pos)
        if pts.length > max_steps: pts.remove(0)
