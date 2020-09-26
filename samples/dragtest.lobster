// demonstrates how Lobster's event state can help program common operations without having to store state yourself

import vec
import color
import gl

fatal(gl_window("event animation and mouse drag test", 1024, 1024))

let lines = []

while gl_frame():

    if gl_button("escape") == 1: return

    gl_clear(color_black)
    gl_color(color_white)

    // allow user to drag a line, and store it when complete
    // note how we can track a drag operation conveniently thanks to gl_last_pos
    let start = float(gl_last_pos("mouse1", true))
    let cur = float(gl_mouse_pos(0))
    if magnitude(start - cur) > 1:  // a click is not a drag
        let mouse1 = gl_button("mouse1")
        if mouse1 >= 1:      // show feedback
            gl_line(start, cur, 1)
        if mouse1 == 0:      // store line
            lines.push([start, float(gl_last_pos("mouse1", false))])
            // note how we use the mouse up position, which may be != cur

    // draw previous lines
    gl_color(color_light_grey)
    for(lines) l:
        gl_line(l[0], l[1], 1)

    // just for demonstration purposes, gl_last_time makes it easy to animate clicks etc.
    // we show little animated circles on each mouse down or up
    for(2) down:
        gl_color(if down: color_green else: color_blue)
        let clicktime = gl_time() - gl_last_time("mouse1", down)
        if  clicktime < 1:
            gl_translate float(gl_last_pos("mouse1", down)):
                gl_line_mode 1:
                    gl_circle(clicktime * 20, 10)
