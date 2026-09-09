// Paths deeper than the borrow check's in-line storage (see small_vector), written thru
// `this` in methods that get reused from several contexts, and element removal and
// insertion on a vector reached thru such a path.
// EXPECT-OK

import std

class Keyframe:
    time:float
    name:string

class Track:
    timeline:[Keyframe]

    def move_keyframe(index:int, dst_time:float) -> int:
        let kf = timeline[index]
        timeline.remove(index)
        var new_index = 0
        while new_index < timeline.length and timeline[new_index].time < dst_time:
            new_index++
        kf.time = dst_time
        timeline.insert(new_index, kf)
        return new_index

class Bone:
    positions:Track
    rotations:Track

class Body:
    bones:[Bone]
    root:Bone

class Scene:
    body:Body
    label:string

def make_track(n:int) -> Track:
    return Track { map(n) i: Keyframe { float(i), "k" + i } }

def make_scene(name:string) -> Scene:
    let b = Bone { make_track(4), make_track(3) }
    return Scene { Body { [ b, Bone { make_track(2), make_track(2) } ], b }, name }

// Writes thru paths of 4 to 6 derefs, thru a borrowed parameter.
def retime(s:Scene, bone:int, dst:float) -> int:
    s.body.root.positions.timeline[0].name = "moved" + ""
    let bt = s.body.bones[bone]
    let first = bt.positions.timeline[0]
    let where = bt.positions.move_keyframe(1, dst)
    s.body.bones[bone].rotations.timeline[0].name = first.name
    return where

let sa = make_scene("a")
let sb = make_scene("b")
assert retime(sa, 0, 2.5) == 2
assert retime(sb, 1, 0.5) == 1
assert retime(sa, 1, 5.0) == 1
assert sa.body.root.positions.timeline[0].name == "moved"
assert sa.body.bones[0].positions.timeline[2].time == 2.5
assert sb.body.bones[1].rotations.timeline[0].name == "k0"

// A loop element borrowed from a deep path, while the loop drops elements of it.
var count = 0
for(sa.body.root.rotations.timeline) kf, i:
    count += kf.name.length
    if i == 0: sa.body.root.rotations.timeline.remove(i)
assert count == 7
assert sa.body.root.rotations.timeline.length == 2
