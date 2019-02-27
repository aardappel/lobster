include vec
include color

dirs := [ xy_xi, xy_yi ]

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
            range := max(0.1, min(0.7, 1 - sqrt(size.x * size.y) / 250))
            col := color { rnd_float(), rnd_float() * range + (1 - range), rnd_float(), 1 } * 0.15 + 0.75
            gl_color(col)
            gl_rect(float(size - xy_1i * 2))

seed := 342342432

fatal(gl_window("bsp", 512, 512))

while gl_frame():
    if gl_button("escape") == 1: return
    if gl_button("space") == 1: seed += 675656
    gl_clear(color_black)
    rnd_seed(seed)
    square(xy_0i, gl_window_size(), 0)
