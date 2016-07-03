include "vec.lobster"
include "color.lobster"

dirs := [ xy_xi, xy_yi ]
colorconstant := 1

def square(pos, size, depth):
    if ((rnd(4) or depth < 2) and size.x > 8 and size.y > 8):
        notfound := true
        dir := 0
        split := 0
        while notfound: // FIXME
            dir = 0
            split = rnd(size.x + size.y)
            if split >= size.x:
                split -= size.x
                dir = 1
            if split >= 3 and split <= size[dir] - 3:
                notfound = false
        for(2) i:
            width := if i: size[dir] - split else: split
            square(pos + dirs[dir] * split * i, dirs[dir] * width + dirs[1 - dir] * size[1 - dir], depth + 1)
    else:
        gl_translate float(pos):
            gl_color(color_black)
            gl_rect(float(size))
            col := [ rndfloat(), rndfloat(), rndfloat(), 1 ] * 0.15 + 0.75
            range := max(0.1, min(0.7, 1 - sqrt(size.x * size.y) / 250))
            col[colorconstant] = rndfloat() * range + (1 - range)
            gl_color(col)
            gl_rect(float(size - xy_1i * 2))

seed := 342342432

fatal(gl_window("bsp", 512, 512))

while gl_frame():
    if gl_button("escape") == 1: return
    if gl_button("space") == 1: seed += 675656
    if gl_button("c") == 1: colorconstant = (colorconstant + 1) % 3
    gl_clear(color_black)
    rndseed(seed)
    square(xy_0i, gl_windowsize(), 0)
