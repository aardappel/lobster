// EXPECT-OK

// A function that is never called is typechecked by itself (see TypeCheckDeadCode), where no
// `try` is active. A call in it of a function that throws still reuses the specialization live
// code has of it. Typechecking that anew instead would redo all it calls in a context no live
// call of it has: here, one in which `f` is still being typechecked when `h` calls it, which
// only a function with an explicit return type can be.

import std
import exception

def f(n:int):
    if n > 10: throw "big"
    if n > 0: h(n - 1)

def h(n:int) -> void:
    f(n)

var caught = ""
try():
    h(20)
fn e:
    caught = e
assert caught == "big"

def never_called(n:int):
    f(n)
