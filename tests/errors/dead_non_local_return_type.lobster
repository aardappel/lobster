// EXPECT-OK

// A return from a function none of whose calls is active, which only functions that are never
// called can contain (see dead_recursive_non_local_return.lobster), has no specialization of
// it to return its value to. So it doesn't change what `h` and `exception_handler` return where
// they do get called later on.

import std
import exception

def h(body):
    body()
    return 0

def thrower(): return 1 from h

def never_called(x:int):
    thrower()
    let r = h(): thrower()
    print r + x

def never_called_either(x:int):
    if x: throw "outside"
    try():
        if x > 1: throw "inside"
    fn v:
        print v
