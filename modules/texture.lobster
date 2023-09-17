import color
import gl

// texture format constants

enum_flags texture_format:
    texture_format_none
    texture_format_clamp
    texture_format_nomipmap
    texture_format_nearest_mag
    texture_format_nearest_min
    texture_format_float
    texture_format_writeonly
    texture_format_readwrite
    texture_format_cubemap
    texture_format_multisample
    texture_format_single_channel
    texture_format_3d
    texture_format_buffer_has_mips
    texture_format_depth
    texture_format_compute
    texture_format_half

// blending constants

enum blend_mode:
    blend_none
    blend_alpha  // Default.
    blend_add
    blend_add_alpha
    blend_multiply
    blend_pre_multiplied_alpha

// FIXME: put this in a generic map type.
private let texture_names:[string] = []
private let textures:[resource<texture>] = []

def loadtexturecached(name, tf:texture_format = texture_format_none):
    let n, i = binary_search(texture_names, name)
    if n:
        return textures[i]
    let tex = gl.load_texture(name, tf)
    if tex:
        texture_names.insert(i, name)
        textures.insert(i, tex)
        return tex
    else:
        print "unable to load texture: {name}"
        return from program

// Pass nil for tex to have it created. Recreates tex if the wrong size. Renders f. Returns tex.
// Depth texture automatically created if depth is true and depthtex is nil.
def render_to_texture(tex, size, depth, depthtex, texture_format, f):
    if not tex or gl.texture_size(tex) != size:
        tex = gl.create_colored_texture(int3(size, 0), color_clear, texture_format)
    assert tex  // FIXME
    assert gl.switch_to_framebuffer(tex, depth, texture_format, nil, depthtex)
    f()
    assert gl.switch_to_framebuffer(nil)
    return tex

def create_depth_texture(tex, size):
    if not tex or gl.texture_size(tex) != size:
        tex = gl.create_colored_texture(int3(size, 0), color_black, texture_format_depth |
            texture_format_nomipmap | texture_format_clamp | texture_format_nearest_mag)
    assert tex  // FIXME
    return tex

// Same, but processes a source texture. Takes size from that texture.
def render_to_texture_from(tex, shader, srctex):
    let texture_format = texture_format_clamp | texture_format_nomipmap | texture_format_nearest_mag
    return render_to_texture(tex, gl.texture_size(srctex), false, nil, texture_format):
        gl.set_primitive_texture(0, srctex)
        gl.set_shader(shader)
        gl.rect(float(gl.texture_size(srctex)))

// Same but renders to the framebuffer.
def post_process(tex, size, shader, depth, texture_format, f):
    texture_format |= texture_format_clamp | texture_format_nomipmap
    tex = render_to_texture(tex, size, depth, nil, texture_format, f)
    gl.blend(0)
    gl.ortho(true, false)  // Needed to flip the texture.
    gl.set_shader(shader)
    gl.set_primitive_texture(0, tex)
    gl.rect(float(size))
    return tex
