import vec
import color
import gl

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

// Sequence of angles to draw the top-level triangle.
let triangle_seq = [ 0, 120, 240 ]

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_white)
    gl_color(color { 0.0 , 0.0, 1.0, 1.0 })
    gl_scale(gl_window_size().y * 0.75)
    gl_translate(xy { 0.1, 0.4 })

    var curr_angle = 0.0
    for (triangle_seq) angle:
        gl_rotate_z(sincos(angle - curr_angle))
        curr_angle = angle
        curve(5, 1.0)
