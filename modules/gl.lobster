import std

namespace gl

enum_flags window_init:
    window_init_fullscreen = 1   // If not set, windowed.
    window_init_no_vsync         // If not set, vsync.
    window_init_linear_color     // If not set, SRGB.
    window_init_native           // If not set, borderless (if fullscreen).
    window_init_maximized        // If not set, normal window (if windowed).
    window_init_no_resizable     // If not set, window is resizable (if windowed).
    window_init_borderless       // If not set, window is bordered (if windowed).

// Used for built-in gl.save_mesh()
enum model_format:
    mf_ply
    mf_iqm

// These are versions of built-in transform and state functions that restore
// the state after executing body.

def color(col, body):
    let old = color(col)
    body()
    color(old)

def blend(mode, body):
    let old = blend(mode)
    body()
    blend(old)

def line_mode(on, body):
    let old = line_mode(on)
    body()
    line_mode(old)

def translate(vec, body):
    push_model_view()
    translate(vec)
    body()
    pop_model_view()

def rotate_x(vec, body):
    push_model_view()
    rotate_x(vec)
    body()
    pop_model_view()

def rotate_y(vec, body):
    push_model_view()
    rotate_y(vec)
    body()
    pop_model_view()

def rotate_z(vec, body):
    push_model_view()
    rotate_z(vec)
    body()
    pop_model_view()

def scale(factor, body):
    push_model_view()
    scale(factor)
    body()
    pop_model_view()

def scissor(topleft, size, body):
    let oldtl, oldsz = scissor(topleft, size)
    body()
    scissor(oldtl, oldsz)

// For backwards compatibility, don't use.
def update_bind_buffer_object(name, buf, ssbo, res = nil):
    res = update_buffer_object(buf, ssbo, -1, nil, false)
    assert bind_buffer_object(name, res)
    return res
