// An element borrowed for an argument while a later argument drops elements: nothing can
// keep it alive, since the call it is for is not known yet.
// EXPECT-ERROR: cannot modify `v[..]` while `v[0]` borrows it at

let v = [ "a" + "" ]

def f(a:string, b:string): print a + b

f(v[0], v.pop())
