import vec
import color
import gl

let size = 32

gl.window("Animated Binary Number", size * 9, size * 24)
gl.set_font_name("data/fonts/US101/US101.ttf")

var n = 0

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    gl.color(color_white)
    gl.translate float2_1 * size
    gl.set_font_size(size * 2)
    gl.text("{n}")
    gl.translate float2_x * 6.0 * size:
        gl.text(number_to_string(n, 16 /* HEX */, 0))
    gl.set_font_size(size)
    gl.translate float2_y * 3.0 * size
    for(64) b:
        gl.translate float2_y * size * b:
            guard n & (1 << b)
            gl.color((color { b & 1, (b & 2) >> 1, (b & 4) >> 2, 1.0 } + color_white) * 0.5)
            gl.text("{b}")
            gl.translate float2_x * size
            gl.translate float2_1 * 0.5 * size:
                gl.circle(size * 0.5, 20)
            gl.translate float2_x * 1.5 * size
            gl.text("{1 << b}")
            gl.translate float2_x * 3.5 * size
            gl.text(number_to_string(1 << b, 16 /* HEX */, 0))
    n++

