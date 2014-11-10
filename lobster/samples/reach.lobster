// graphics demo showing very simple link based physics, based on a processing.js example

include "std.lobster"
include "vec.lobster"
include "color.lobster"

struct segment: [ pos, dir ]
segs := map(7): [ [ 0.5, 1 ]:xy, nil ]:segment

ballradius :== 0.1
ball := xy_h
ballvel := [ 0.005, -0.004 ]:xy

fatal(gl_window("reach", 600, 600))

while(gl_frame()):
    if(gl_wentdown("escape")): return
    gl_clear(color_black)

    gl_scale(min(gl_windowsize().x, gl_windowsize().y))

    gl_color(color_light_grey)
    gl_rect(xy_1)

    ball += ballvel

    if(ball.x > 1 - ballradius | ball.x < ballradius): ballvel *= [ -1,  1 ]
    if(ball.y > 1 - ballradius | ball.y < ballradius): ballvel *= [  1, -1 ]

    gl_translate(ball):
        gl_color(color_dark_grey)
        gl_circle(ballradius, 50)
        gl_color(color_light_grey)
        gl_circle(ballradius / 5, 20)

    target := nil
    seglength := 0.1

    for(segs) seg, i:
        in := i & target | ball
        seg.dir = normalize(in - seg.pos) * seglength
        target = in - seg.dir

    reducerev(segs) b, a:
        b.pos = a.pos + a.dir
        b

    for(segs) seg, i:
        gl_translate(seg.pos):
            segwidth := 0.01 + i * 0.005
            gl_color(color_white)
            gl_line(xy_0, seg.dir, segwidth)
            gl_translate(seg.dir)
            gl_color(color_grey)
            gl_circle(segwidth / 2 + 0.005, 20)

