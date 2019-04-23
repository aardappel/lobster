// texture format constants

enum_flags texture_format:
    texture_format_none = 0
    texture_format_clamp = 1
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
private let textures:[resource] = []

def loadtexturecached(name, tf:texture_format):
    let n, i = binary_search(texture_names, name)
    if n:
        return textures[i]
    else:
        let tex = gl_load_texture(name, tf)
        if tex:
            texture_names.insert(i, name)
            textures.insert(i, tex)
            return tex
        else:
            print "unable to load texture: " + name
            return from program

def loadtexturecached(name): return loadtexturecached(name, texture_format_none)  // FIXME: default args.
