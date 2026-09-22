// EXPECT-OK

// As dead_recursive_non_local_return.lobster, with the return from a function that isn't
// being called coming from `throw` (which returns from `exception_handler`), also when it is
// thrown by a function the recursive one calls, and when the recursion goes thru another
// function.

import std
import exception

def a(n:int) -> int:
    if n > 1000:
        throw "too deep"
    return if n <= 0: 0 else: a(n - 1)

def check(n:int):
    if n > 1000:
        throw "too deep"

def b(n:int) -> int:
    check(n)
    return if n <= 0: 0 else: c(n - 1)

def c(n:int) -> int:
    return if n <= 0: 0 else: b(n - 1)

def never_called(n:int):
    print a(n) + b(n)
