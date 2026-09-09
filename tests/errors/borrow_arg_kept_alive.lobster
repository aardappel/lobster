// A callee overwrites the vector slot its borrowed argument was taken from (directly, thru a
// lambda's parameter, and thru dynamic dispatch), and keeps using the argument: the caller
// keeps the value alive rather than this being an error.
// EXPECT-OK

import std

class Obj:
    idx:int
    name:string

class World:
    objects:[Obj] = []

    def add(o:Obj):
        o.idx = objects.length
        objects.push(o)

    def delete(o:Obj) -> string:
        objects[o.idx] = Obj { -1, "empty" + "" }
        return o.name

    def foreach_idx(idxs:[int], f):
        for(idxs) i: f(objects[i])

class Killer:
    def kill(w:World, o:Obj) -> string:
        w.objects[o.idx] = Obj { -1, "killed" + "" }
        return o.name

class LoudKiller : Killer
    def kill(w:World, o:Obj) -> string:
        w.objects[o.idx] = Obj { -1, "KILLED" + "" }
        return o.name + "!"

def make_world() -> World:
    let w = World {}
    for(4) i: w.add(Obj { 0, "o" + i })
    return w

let w = make_world()
var names = ""
w.foreach_idx([ 0, 2 ]) o:
    names += w.delete(o)
assert names == "o0o2"
assert w.objects[0].name == "empty" and w.objects[1].name == "o1"
assert w.delete(w.objects[1]) == "o1"
assert w.objects[1].name == "empty"

// The same specialization reused from another context.
def delete_last(ww:World) -> string:
    return ww.delete(ww.objects[ww.objects.length - 1])
assert delete_last(w) == "o3"
assert delete_last(make_world()) == "o3"

let killers = [ Killer {}, LoudKiller {} ]
let w2 = make_world()
var killed = ""
for(killers) k, i:
    killed += k.kill(w2, w2.objects[i])
assert killed == "o0o1!"
assert w2.objects[1].name == "KILLED"
