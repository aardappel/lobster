/* translated from http://www.kevinbeason.com/smallpt/ 
and ported to glsl from smallpt.lobster
Path tracer + accumulator 
*/

import vec
import color
import gl
import texture

//TODO: unified structs
struct Ray:
    o:float3
    d:float3


let rgba32f = texture_format_float | texture_format_clamp | texture_format_nomipmap

let albedo_tex_format = rgba32f
var last_window_size = int2{800, 600}
fatal(gl.window("compute path tracer", last_window_size.x, last_window_size.y))
gl.require_version(4, 3)

let pathtracer = read_file("custom_shader_pt.glsl")
if pathtracer:
    fatal(gl.load_materials(pathtracer, true))
else:
    fatal("Could not find custom_shader_pt.glsl")

let mats =
"""
SHADER screen
    VERTEX
        INPUTS apos:4 atc:2
        UNIFORMS mvp
        gl_Position = mvp * apos;
        itc = atc;
    PIXEL
        INPUTS itc:2
        UNIFORMS tex0
        frag_color = texture(tex0, itc);
"""

fatal(gl.load_materials(mats, true))
var albedo_tex = gl.create_blank_texture(int3(last_window_size, 0), albedo_tex_format)
var window_size_changed_last_frame = false
var frame_count = 0
var frame_accum = 0 // for averaging

let cam = Ray { float3 { 50.0,50.0,290.0 }, normalize(float3 { 0.0,-0.042612,-1.0 }) } // cam pos, dir
// pack cam into 2x3 matrix
let cam_vector = [cam.o.x, cam.o.y, cam.o.z, cam.d.x, cam.d.y, cam.d.z]

var cx = float3_x * (last_window_size.x * .5135 / last_window_size.y)
var cy = normalize(cx.cross(cam.d)) * .5135
var lens_matrix = [cx.x, cx.y, cx.z, cy.x, cy.y, cy.z]

while(gl.frame()):
    if gl.button("escape") == 1: return
    window_size_changed_last_frame = last_window_size != gl.window_size()
    last_window_size = gl.window_size()
    ++frame_count
    ++frame_accum
    if window_size_changed_last_frame:
        albedo_tex = gl.create_blank_texture(int3(last_window_size, 0), albedo_tex_format)
        frame_accum = 0
        cx = float3_x * (last_window_size.x * .5135 / last_window_size.y)
        cy = normalize(cx.cross(cam.d)) * .5135
        lens_matrix = [cx.x, cx.y, cx.z, cy.x, cy.y, cy.z]
    gl.set_shader("pathtracer")
    gl.set_uniform("frame_count", frame_count)
    gl.set_uniform("frame_accum", frame_accum)
    gl.set_uniform_matrix("m_ray", cam_vector, true)
    gl.set_uniform_matrix("lens", lens_matrix, true)
    gl.set_uniform("window_size", last_window_size)
    gl.set_image_texture(0, albedo_tex, 0, texture_format_writeonly)
    let groups = (last_window_size + 7) / 8
    gl.dispatch_compute(int3(groups, 1))
    gl.blend(blend_none)
    gl.set_shader("screen")
    gl.set_primitive_texture(0, albedo_tex)
    gl.rect(float(gl.window_size()))