// Many labels a frame: consecutive gl.text calls draw as one batch per font, outlined ones too.

import std
import vec
import color
import gl

fatal(gl.window("text labels", 1200, 800))
check(gl.set_font_name("data/fonts/Droid_Sans/DroidSans.ttf"), "can\'t load font")

let colors = [ color_white, color_yellow, color_cyan, color_pink ]
var lastdraws = gl.draw_calls()

while gl.frame() and gl.button("escape") != 1:
    // The last batch of a frame draws when gl.frame ends it, so count from frame to frame.
    let draws = gl.draw_calls() - lastdraws
    lastdraws = gl.draw_calls()
    gl.clear(color_dark_grey)
    let cell = float(gl.window_size()) / float2 { 10.0, 23.0 }
    gl.set_font_size(int(cell.y * 0.5))
    for(200) i:
        let wobble = sincos(gl.time() * 90.0 + i * 20.0) * 4.0
        gl.translate(float(int2 { i % 10, i / 10 + 2 }) * cell + wobble):
            gl.color(colors[i % colors.length]):
                gl.text("unit {i}")
    gl.set_font_size(int(cell.y * 1.2), 3.0, color_black)
    gl.translate(cell * 0.2):
        gl.text("200 labels in {draws} draws a frame")
