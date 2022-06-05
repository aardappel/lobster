// graphics demo showing very simple link based physics, based on a processing.js example

import std
import vec
import color
import gl

class segment:
    pos:xy_f
    dir = xy_0

let segs = map 7: segment { xy { 0.5, 1.0 } }

let ballradius = 0.1
var ball = xy_h
var ballvel = xy { 0.005, -0.004 }

fatal(gl_window("reach", 600, 600))

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_black)

    gl_scale(min(gl_window_size().x, gl_window_size().y))

    ball += ballvel

    if(ball.x > 1.0 - ballradius or ball.x < ballradius): ballvel *= xy { -1.0,  1.0 }
    if(ball.y > 1.0 - ballradius or ball.y < ballradius): ballvel *= xy {  1.0, -1.0 }

    gl_translate ball:
        gl_color(color_dark_grey)
        gl_circle(ballradius, 50)
        gl_color(color_light_grey)
        gl_circle(ballradius / 5.0, 20)

    var target = xy_0
    let seglength = 0.1

    for(segs) seg, i:
        let in = if i: target else: ball
        seg.dir = normalize(in - seg.pos) * seglength
        target = in - seg.dir

    reduce_reverse(segs) b, a:
        b.pos = a.pos + a.dir
        b

    for(segs) seg, i:
        gl_translate seg.pos:
            let segwidth = 0.01 + i * 0.005
            gl_color(color_white)
            gl_line(xy_0, seg.dir, segwidth)
            gl_translate(seg.dir)
            gl_color(color_grey)
            gl_circle(segwidth / 2.0 + 0.005, 20)

