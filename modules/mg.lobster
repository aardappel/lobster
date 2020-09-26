// Utility functions to go along with the mg (mesh gen) builtins.

// These are versions of built-in transform and state functions that restore
// the state after executing body.

def mg_translate(vec, body):
    mg_push_transform()
    mg_translate(vec)
    body()
    mg_pop_transform()

def mg_scale(f, body):
    mg_push_transform()
    mg_scale(f)
    body()
    mg_pop_transform()

def mg_scale_vec(vec, body):
    mg_push_transform()
    mg_scale_vec(vec)
    body()
    mg_pop_transform()

def mg_rotate(axis, angle, body):
    mg_push_transform()
    mg_rotate(axis, angle)
    body()
    mg_pop_transform()

def mg_color(col, body):
    mg_push_transform()
    mg_color(col)
    body()
    mg_pop_transform()

def mg_smooth(smooth, body):
    mg_push_transform()
    mg_smooth(smooth)
    body()
    mg_pop_transform()
