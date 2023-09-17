// Utility functions to go along with the mg (mesh gen) builtins.

// These are versions of built-in transform and state functions that restore
// the state after executing body.

namespace mg

def translate(vec, body):
    push_transform()
    translate(vec)
    body()
    pop_transform()

def scale(f, body):
    push_transform()
    scale(f)
    body()
    pop_transform()

def scale_vec(vec, body):
    push_transform()
    scale_vec(vec)
    body()
    pop_transform()

def rotate(axis, angle, body):
    push_transform()
    rotate(axis, angle)
    body()
    pop_transform()

def color(col, body):
    push_transform()
    color(col)
    body()
    pop_transform()

def smooth(sm, body):
    push_transform()
    smooth(sm)
    body()
    pop_transform()
