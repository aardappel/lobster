// a minecraft clone in very few lines of code
// implements random world generation, rendering, and mining/building of blocks
// created in response to https://github.com/fogleman/Minecraft (which is 10x bigger in code)
// NOTE: this code is seriously inefficient, it was written for small size only.
// Don't use this as a base to write a minecraft clone.

include "std.lobster"
include "vec.lobster"
include "color.lobster"
include "3dhelpers.lobster"

lssize := 32 // uses lssize^3 blocks... make this bigger, and you'll anger the gods of framerate
inventory := []:int // blocks we've mined
camera_position = xyz_1 * lssize / 2
camera_yaw = 45

def inside(v): all(v > 0) & all(v < lssize - 1)    // keep the outer blocks empty for simplicity

cells := map(lssize) z:
            map(lssize) y:
                map(lssize) x:
                    v := xyz { x, y, z }
                    h := simplex(v / float(lssize) + 11, 6, 1, 0.55)   // generate blocks using noise
                    // more likely to be solid the lower it is, and only when not on the outside:
                    solid := h / 1.5 > div(z, lssize) - 0.5 & inside(v)
                    // pick material with noise too:
                    if(solid): solid = int((simplex(v / float(lssize), 8, 3, 0.55) + 1) * 2) + 1
                    solid

colors := [ color_dark_red, color_olive, color_green, color_dark_grey, color_grey ]
nbdirs := [ xyz_xi, -xyz_xi, xyz_yi, -xyz_yi, xyz_zi, -xyz_zi ]
nbpolys := map([ "4576", "0231", "2673", "0154", "1375", "0462" ]) s: map(s) c: map(3) i: and(c, shl(1, 2 - i)) != 0

fatal(gl_window("LobsterCraft(tm)", 800, 600))

while(gl_frame()):
    if(gl_wentdown("escape")): return
    gl_clear(color_light_blue)
    gl_cursor(0)
    gl_perspective(70, 0.1, 1000)
    camera_FPS_update("w", "a", "s", "d", 10, 4, true)
    camera_FPS_view()
    gl_light(camera_position)
    gl_blend(0)
    gl_setshader("phong")
    for(cells) zl, z: for(zl) yl, y: for(yl) e, x:  // for all cells in the world
        if(e):  // if this cell is solid
            v := xyz { x, y, z }
            for(nbdirs) nv, i:
                if(!cells[nv + v]):  // and neighbor is empty
                    // we do this here rather than outside the loop (there's less visible faces than total solid cubes)
                    gl_color(colors[e - 1])
                    gl_translate(v):
                        gl_polygon(nbpolys[i])  // draw face
    first_solid := nil  // find the first solid and last empty block we're looking at
    last_empty := nil
    camvec := vecfromyawpitch(camera_yaw, camera_pitch, 1, 0)
    for(100) i:  // look at most 10 cubes ahead
        pos := int(camera_position + camvec * (i / 10.0))
        if(inside(pos) & !first_solid):
            if(cells[pos]): first_solid = pos
            else: last_empty = pos
    // LMB places blocks & RMB removes blocks
    if(gl_wentdown("mouse1") & last_empty & inventory.length): cells[last_empty] = inventory.pop()
    if(gl_wentdown("mouse3") & first_solid): inventory.push(cells[first_solid]); cells[first_solid] = 0
    gl_setshader("color")
    gl_blend(1)
    wh := 11.7  // let's add some water!
    gl_color([ 0.5, 0.5, 1.0, 0.5 ])
    gl_polygon([ [1, 1, wh], [1, lssize - 1, wh], [lssize - 1, lssize - 1, wh], [lssize - 1, 1, wh] ])
    gl_ortho()
    gl_color(color_white)
    gl_translate(gl_windowsize() / 2):
        gl_circle(5, 20)    // simplified crosshairs
