// based on: http://www.physicsandbox.com/projects/double-pendulum.html

include "std.lobster"
include "vec.lobster"
include "color.lobster"
include "texture.lobster"

g :== 9.8
mass := 1
max_steps := 2000

struct link { len:float = 180, dtheta:float = 0, theta:float = 181 }

def ddl(l::link): dtheta * dtheta * len

links := map(2): link {}

pts := []::xy_f

fatal(gl_window("double pendulum", 800, 800))
while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_black)
    time := gl_deltatime() * 4

    sc := sincos(links[0].theta - links[1].theta)
    for(links) l, i:
        o := links[1 - i]
        d2theta :=  ((i + 1) * g * (sin(o.theta) * sc.x - (2 - i) * sin(l.theta)) +
                     (i * 2 - 1) * ((i + 1) * ddl(o) + ddl(l) * sc.x) * sc.y) /
                    (l.len * (2 - sc.x * sc.x))
        l.dtheta += d2theta * time
        l.theta += degrees(l.dtheta * time)

    gl_translate(float(gl_windowsize()) / 2)

    gl_blend(blend_add):
        for(pts) pos, i:
            gl_color(color { 0.15, 0.075, 0.0, 1.0 } * i / max_steps)
            gl_translate(pos):
                gl_circle(10, 10)

    gl_color(color_white)
    pos := xy_0
    for(links) l:
        p := yx(sincos(l.theta)) * l.len
        gl_line(xy_0, p, 2)
        gl_translate(p)
        gl_circle(4, 10)
        pos += p
        pts.push(pos)
        if pts.length > max_steps: pts.remove(0)
