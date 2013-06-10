include "vec.lobster"
include "color.lobster"

dirs := [ vec_xi, vec_yi ]
colorconstant := 1

function square(pos, size, depth):
    if((rnd(4) | depth < 2) & size.x > 8 & size.y > 8):
        //dir := rnd(2)
        //dir := size.y > size.x
        //split := rnd(size[dir] - 7) + 4
        notfound := true
        dir := 0
        split := 0
        while(notfound): // FIXME
            dir = 0
            split = rnd(size.x + size.y)
            if(split >= size.x):
                split -= size.x
                dir = 1
            if(split >= 3 & split <= size[dir] - 3):
                notfound = false
        for(2) i:
            width := if(i): size[dir] - split else: split
            square(pos + dirs[dir] * split * i, dirs[dir] * width + dirs[1 - dir] * size[1 - dir], depth + 1)
    else:
        gl_translate(pos):
            gl_color(color_black)
            gl_rect(size)
            //col := [ rndfloat(), rndfloat(), rndfloat(), 1 ] * 0.7 + 0.3
            //col[colorconstant] = 0.7
            col := [ rndfloat(), rndfloat(), rndfloat(), 1 ] * 0.15 + 0.75 
            range := max(0.1, min(0.7, 1 - sqrt(size.x * size.y) / 250))
            col[colorconstant] = rndfloat() * range + (1 - range) 
            gl_color(col)
            gl_rect(size - vec_1i * 2)

seed := 342342432
                
fatal(gl_window("bsp", 512, 512))

while(gl_frame()):    
    if(gl_wentdown("escape")): return
    if(gl_wentdown("space")): seed += 675656
    if(gl_wentdown("c")): colorconstant = (colorconstant + 1) % 3
    gl_clear(color_black)
    rndseed(seed)
    //seed++
    square(vec_0i, gl_windowsize(), 0)
    //for(4) x: for(3) y:
    //    square([ x, y ]:xy * 530 + [ 10, 2 ]:xy, [ 512, 512 ]:xy, 0)
