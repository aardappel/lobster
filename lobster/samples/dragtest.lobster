// demonstrates how Lobster's event state can help program common operations without having to store state yourself

include "vec.lobster"
include "color.lobster"

fatal(gl_window("event animation and mouse drag test", 1024, 1024))

lines := []

while gl_frame():

    if gl_wentdown("escape"): return

    gl_clear(color_black)
    gl_color(color_white)

    // allow user to drag a line, and store it when complete
    // note how we can track a drag operation conveniently thanks to gl_lastpos
    start := float(gl_lastpos("mouse1", true))
    cur := float(gl_mousepos(0))
    if magnitude(start - cur) > 1:  // a click is not a drag
        if gl_isdown("mouse1"):      // show feedback
            gl_line(start, cur, 1)
        if gl_wentup("mouse1"):      // store line
            lines.push([start, float(gl_lastpos("mouse1", false))])
            // note how we use the mouse up position, which may be != cur

    // draw previous lines
    gl_color(color_light_grey)
    for(lines) l:
        gl_line(l[0], l[1], 1)

    // just for demonstration purposes, gl_lasttime makes it easy to animate clicks etc.
    // we show little animated circles on each mouse down or up
    for(2) down:
        gl_color(down and color_green or color_blue)
        clicktime := gl_time() - gl_lasttime("mouse1", down)
        if  clicktime < 1:
            gl_translate float(gl_lastpos("mouse1", down)):
                gl_linemode 1:
                    gl_circle(clicktime * 20, 10)
