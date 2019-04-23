// a minecraft clone in very few lines of code
// implements random world generation, rendering, and mining/building of blocks
// created in response to https://github.com/fogleman/Minecraft (which is 10x bigger in code)
// NOTE: this code is seriously inefficient, it was written for small size only.
// Don't use this as a base to write a minecraft clone.

import std
import vec
import color
import texture
import util3d

let lssize = 32 // uses lssize^3 blocks... make this bigger, and you'll anger the gods of framerate
let inventory = []::int // blocks we've mined
camera_position = xyz_1 * lssize / 2
camera_yaw = 45

def inside(v): return all(v > 0) and all(v < lssize - 1)    // keep the outer blocks empty for simplicity

let cells = map(lssize) z:
            map(lssize) y:
                map(lssize) x:
                    let v = xyz { x, y, z }
                    let h = simplex(float(v) / lssize + 11, 6, 1, 0.55)   // generate blocks using noise
                    // more likely to be solid the lower it is, and only when not on the outside:
                    var solid = h / 1.5 > div(z, lssize) - 0.5 and inside(v)
                    // pick material with noise too:
                    if solid: int((simplex(float(v) / lssize, 8, 3, 0.55) + 1) * 2) + 1
                    else: 0

let colors = [ color_dark_red, color_olive, color_green, color_dark_grey, color_grey ]
let nbdirs = [ xyz_xi, -xyz_xi, xyz_yi, -xyz_yi, xyz_zi, -xyz_zi ]

fatal(gl_window("LobsterCraft(tm)", 800, 600))

let nbpolys = map([ "4576", "0231", "2673", "0154", "1375", "0462" ]) s:
    gl_new_poly(map(s) c: xyz_v(map(3) i: float(c & (1 << (2 - i)) != 0)))

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_light_blue)
    gl_cursor(false)
    gl_perspective(70, 0.1, 1000)
    camera_FPS_update("w", "a", "s", "d", 10, 4, true)
    camera_FPS_view()
    gl_light(camera_position, xy { 64.0, 0.25 })
    gl_blend(blend_none)
    gl_set_shader("phong")
    for(cells) zl, z: for(zl) yl, y: for(yl) e, x:  // for all cells in the world
        if e:  // if this cell is solid
            let v = xyz { x, y, z }
            for(nbdirs) nv, i:
                if(!cells[nv + v]):  // and neighbor is empty
                    // we do this here rather than outside the loop (there's less visible faces than total solid cubes)
                    gl_color(colors[e - 1])
                    gl_translate float(v):
                        gl_render_mesh(nbpolys[i])  // draw face
    var first_solid = xyz_1i * -1 // find the first solid and last empty block we're looking at
    var last_empty = xyz_1i * -1
    let camvec = camera_forward_vector()
    for(100) i:  // look at most 10 cubes ahead
        let pos = int(camera_position + camvec * (i / 10.0))
        if inside(pos) and first_solid.x < 0:
            if cells[pos]: first_solid = pos
            else: last_empty = pos
    // LMB places blocks and RMB removes blocks
    if gl_button("mouse1") == 1 and last_empty.x >= 0 and inventory.length:
        cells[last_empty] = inventory.pop()
    if gl_button("mouse3") == 1 and first_solid.x >= 0:
        inventory.push(cells[first_solid])
        cells[first_solid] = 0
    gl_set_shader("color")
    gl_blend(blend_alpha)
    let wh = 11.7  // let's add some water!
    gl_color(color { 0.5, 0.5, 1.0, 0.5 })
    gl_polygon([ xyz { 1.0, 1.0, wh }, xyz { 1.0, lssize - 1.0, wh },
                 xyz { lssize - 1.0, lssize - 1.0, wh }, xyz {lssize - 1.0, 1.0, wh } ])
    gl_ortho()
    gl_color(color_white)
    gl_translate float(gl_window_size()) / 2:
        gl_circle(5, 20)    // simplified crosshairs
