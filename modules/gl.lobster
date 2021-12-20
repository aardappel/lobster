import std

enum_flags gl_window_init:
    gl_window_init_fullscreen = 1
    gl_window_init_no_vsync
    gl_window_init_linear_color

// These are versions of built-in transform and state functions that restore
// the state after executing body.

def gl_color(col, body):
    let old = gl_color(col)
    body()
    gl_color(old)

def gl_blend(mode, body):
    let old = gl_blend(mode)
    body()
    gl_blend(old)

def gl_line_mode(on, body):
    let old = gl_line_mode(on)
    body()
    gl_line_mode(old)

def gl_translate(vec, body):
    gl_push_model_view()
    gl_translate(vec)
    body()
    gl_pop_model_view()

def gl_rotate_x(vec, body):
    gl_push_model_view()
    gl_rotate_x(vec)
    body()
    gl_pop_model_view()

def gl_rotate_y(vec, body):
    gl_push_model_view()
    gl_rotate_y(vec)
    body()
    gl_pop_model_view()

def gl_rotate_z(vec, body):
    gl_push_model_view()
    gl_rotate_z(vec)
    body()
    gl_pop_model_view()

def gl_scale(factor, body):
    gl_push_model_view()
    gl_scale(factor)
    body()
    gl_pop_model_view()

def gl_scissor(topleft, size, body):
    let oldtl, oldsz = gl_scissor(topleft, size)
    body()
    gl_scissor(oldtl, oldsz)

private let delta_time_log = map(60): 1 / 30.0
def gl_deltatime_over_time():
    delta_time_log.push(gl_delta_time())
    delta_time_log.remove(0)
    return (delta_time_log.fold(0.0): _a + _e) / delta_time_log.length
