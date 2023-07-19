// A minecraft clone in very few lines of code
// implements random world generation, chunks, rendering, and mining/building of blocks
// created in response to https://github.com/fogleman/Minecraft (which is 10x bigger in code)

import std
import vec
import color
import gl
import texture
import camera

fatal(gl_window("LobsterCraft(tm)", 1280, 800))

let lssize = int3 { 128, 128, 32 }
let csize = 16
let inventory = []::int // blocks we've mined
let camera = Camera { float(lssize) / 2.0, 45.0, 0.0 }

def inside(v): return all(v > 0) and all(v < lssize - 1)    // keep the outer blocks empty for simplicity

let cells = mapxyz(lssize) v:
    // Generate blocks using noise.
    let h = simplex(float(v) / float3 { 64.0, 64.0, 32.0 } + 11.0, 6, 1.0, 0.55)
    // more likely to be solid the lower it is, and only when not on the outside:
    let solid = h / 1.5 > div(v.z, lssize.z) - 0.5 and inside(v)
    // pick material with noise too:
    if solid: int((simplex(float(v) / float(lssize), 8, 3.0, 0.55) + 1.0) * 2.0) + 1
    else: 0

let colors = [ color_dark_red, color_olive, color_green, color_dark_grey, color_grey ]
let nbdirs = [ int3_x, -int3_x, int3_y, -int3_y, int3_z, -int3_z ]
let nbpolys = map([ "4576", "0231", "2673", "0154", "1375", "0462" ]) s:
    map(s) c: vec3_v(map(3) i: float(c & (1 << (2 - i)) != 0))
let tris = [ 0, 1, 2, 2, 3, 0 ]

let meshes = mapxyz(lssize / csize): nil
def generate_mesh(ci):
    let vpositions = []
    let vcolors = []
    let vnormals = []
    forxyz(int3_1 * csize) cv:  // For all cells in a chunk
        let v = ci * csize + cv
        let e = cells[v]
        guard e  // if this cell is solid
        for(nbdirs) nv, i:
            guard not cells[nv + v]  // and neighbor is empty
            for(tris) ti:
                vpositions.push(float(v) + nbpolys[i][ti])
                vcolors.push(colors[e - 1])
                vnormals.push(float(nv))
    meshes[ci] = gl_new_mesh("PCN", vpositions, vcolors, vnormals, [], [], [])
forxyz(lssize / csize) v: generate_mesh(v)

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_light_blue)
    gl_cursor(false)
    gl_perspective(70.0, 0.1, 1000.0)
    camera.FPS_update("w", "a", "s", "d", 10.0, 4.0, true)
    camera.FPS_view()
    gl_light(camera.position, float2 { 64.0, 0.25 })
    gl_blend(blend_none)
    gl_set_shader("phong")
    forxyz(lssize / csize) v:
        gl_render_mesh(assert meshes[v])
    var first_solid = int3_1 * -1 // find the first solid and last empty block we're looking at
    var last_empty = int3_1 * -1
    let camvec = camera.forward_vector()
    for(100) i:  // look at most 10 cubes ahead
        let pos = int(camera.position + camvec * (i / 10.0))
        if inside(pos) and first_solid.x < 0:
            if cells[pos]: first_solid = pos
            else: last_empty = pos
    // LMB places blocks and RMB removes blocks
    if gl_button("mouse1") == 1 and last_empty.x >= 0 and inventory.length:
        cells[last_empty] = inventory.pop()
    if gl_button("mouse3") == 1 and first_solid.x >= 0:
        inventory.push(cells[first_solid])
        cells[first_solid] = 0
    // Blindly regen chunk looked at each frame is the easiest way to deal with cross-chunk edits ;)
    generate_mesh(first_solid / csize)
    gl_set_shader("color")
    gl_blend(blend_alpha)
    let wh = 11.7  // let's add some water!
    gl_color(color { 0.5, 0.5, 1.0, 0.5 })
    gl_polygon([ float3 { 1.0, 1.0, wh }, float3 { 1.0, lssize.y - 1.0, wh },
                 float3 { lssize.x - 1.0, lssize.y - 1.0, wh }, float3 { lssize.x - 1.0, 1.0, wh } ])
    gl_ortho()
    gl_color(color_white)
    gl_translate float(gl_window_size()) / 2.0:
        gl_circle(5.0, 20)    // simplified crosshairs
