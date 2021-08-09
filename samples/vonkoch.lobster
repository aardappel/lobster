import color
import gl
import imgui
import vec

// Sequence of the angles for the Von Koch curve's segment. A line
// will be drawn along each angle by continuing from the previous point.
let seq = [ 0, -60, 60, 0 ]

def curve(depth, d) -> void:
    if depth <= 0:
        // Draw a single segment and move to its end.
        gl_line(xy { 0.0, 0.0 }, xy { d, 0.0 }, 0.002)
        gl_translate(xy { d, 0.0 })
    else:
        var curr_angle = 0.0
        for (seq) angle:
            // Move along the angle and iterate.
            gl_rotate_z(sincos(angle - curr_angle))
            curr_angle = angle
            curve(depth - 1, d / 3)

fatal(gl_window("Von Koch\'s curve", 640, 640))
im_init(false)
assert im_add_font("data/fonts/Droid_Sans/DroidSans.ttf", 20.0)

// Sequence of angles to draw the top-level triangle.
let triangle_seq = [ 0, 120, 240 ]

class Setup:
    depth: int

var setup = Setup { 5 }

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_white)
    gl_color(color_blue)
    gl_scale(gl_window_size().y * 0.75)
    gl_translate(xy { 0.1, 0.4 })

    var curr_angle = 0.0
    for (triangle_seq) angle:
        gl_rotate_z(sincos(angle - curr_angle))
        curr_angle = angle
        curve(setup.depth, 1.0)

    im_frame():
        im_window("Parameters", im_window_always_autoresize):
            setup = im_edit_anything(setup)

