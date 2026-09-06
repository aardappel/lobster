/*
A synthetic benchmark whose mix of language features was measured from real Lobster code, so
that compiler work can be judged against what programs actually run rather than against what
one hand written test happens to do a lot of.

    bin/lobster tests/bench/synthetic2.lobster

It prints the seconds each section took, their total, and two checksums. Compare TOTAL between
builds, on the same machine, with the same backend and arguments. `verify` is a fixed value that
does not depend on how much work the timed part does: if it changes, the meaning of the code
changed and the timings are not comparable to earlier ones.

WHY
tests/speedtest.lobster is a collection of real algorithms, but their hand picked repeat counts
decide what it measures, and nothing keeps that in line with what Lobster code does elsewhere.
An optimization that helps whatever speedtest does most of looks better there than it is. This
file was built the other way around: measure real programs first, then write code with that mix.
Because it is synthetic it contains code shapes rather than algorithms, and neither its source
nor its repeat counts should be tuned while optimizing against it.

HOW THE PROFILE WAS MEASURED
Codegen was temporarily instrumented to emit a counter increment next to the code it generates
for every construct it recognizes, writing the totals out at exit, so that a run counted what it
executed rather than what it contained. That instrumentation is not in the tree: what to count
is the interesting question, and the next time this is worth doing the answer has probably moved,
so it is better rewritten from scratch than kept and trusted.

Each counter named which construct ran, on which types, and in which context, for example
`read.field.int3`, `math.float2_float.mul`, `native.push`, `call.static.args3.rets0`. Counting at
codegen means anything the optimizer folded away or inlined never appears, which is why the
helpers below come in two sizes: a function under 16 nodes is inlined into its caller and never
shows up as a call at all, so only the larger ones with more than one caller stay calls.

WHAT WAS MEASURED, AND HOW IT WAS MIXED
                                        programs    raw events    share of the mix
  projects/matvoxels                           1   889,274,329               50%
  projects/ (other prototypes)                81 8,844,300,432               30%
  tests/ and samples/ of this repo             47   267,416,911               20%

matvoxels is the largest Lobster codebase there is and a shipped game, so it decides half of the
mix. It ran its own instrumented build, loading savegames/SurvivalDemo.4668734651508286811 thru
--cpuprofile with --cpuprofile-frames=30 --cpuprofile-warmup=0, which covers loading the world
plus 30 real game updates. Everything else ran with --non-interactive-test 3 --background, one
pass of the initialization code and three frames of the frame code. Programs that failed to
start (missing custom natives, needing input) and anything under 1000 events are left out;
tests/speedtest.lobster is deliberately left out, since aiming the new benchmark at the old one
would defeat the point.

Within a group each program contributes the shape of its own counts, normalized to sum to one,
weighted by log10 of how many events it ran. A program that runs longer is more like the code we
actually spend time in, so it counts for more, but a single slow startup cannot drown out the
other eighty. The heaviest prototype (gradient_descent) still only carries 2.1% of its group.

WHAT THE MEASUREMENT SAID
The result was less about arithmetic than expected. Per hundred operations real Lobster runs
roughly 41 variable, field, global and element reads, 12 assignments, 11 constants, 9 arithmetic
operations, 6 builtin calls, 5 loop iterations, 5 conditionals, 3 calls of its own functions and
the 2 returns those come back thru. That is one call every twenty operations and one builtin every fifteen: the code moves
values around and calls things far more than it computes with them, and the sections below are
written at that density rather than as tight arithmetic loops.

The ranking, as order of magnitude buckets over the 1303 counters the corpus touched:

  target frequency   counters   share of all events   reached by this benchmark
  at least 1%              24                65.58%                      100.0%
  0.1% to 1%              90                26.66%                       77.8%
  0.01% to 0.1%          179                 6.85%                       36.4%
  0.001% to 0.01%        219                 0.82%                       13.1%
  below 0.001%           791                 0.10%                        2.8%

WHAT THIS BENCHMARK DOES
Nine sections, each a shape that showed up in the corpus, run interleaved and repeated by the
weights in `sections` below. Those weights are not hand picked: each section's profile was
measured on its own and the weights were fitted to bring the mixture closest to the target, with
a floor so every section keeps a real share.

  entities     objects in a list: field reads and writes, methods, nullable links, type tests
  query        a hand rolled best-of scan over the same objects
  grid         a flat int grid with int2 coordinates, neighbourhoods, masks and shifts
  heights      nested int vectors walked by index, plus int3 coordinate work
  numeric      float and float2/3/4 maths: dot, magnitude, normalize, lerp, sincos, conversions
  containers   building and consuming vectors: push, length, filter, sort, a stack
  actions      an event queue: void handlers, globals, follow-up work, short lived allocation
  logic        the plumbing: small calls, multiple returns, boolean operators, asserts
  text         string building, comparison and interpolation

An integer is threaded thru every path, folded into the checksums, and printed, so no section
can be optimized away, and `verify` asserts that the meaning of all of it is unchanged.

HOW CLOSE IT GETS (measured, benchmark against target, as percent of all events)
Exact agreement was not the goal and the code was not contorted to reach it. What matters is
that nothing important is missing or off by an order of magnitude.

  group          target   bench  ratio      group          target   bench  ratio
  read.local     25.186  28.717   1.14      read.index      1.986   1.529   0.77
  lval           11.726  13.024   1.11      bitop           1.177   1.442   1.23
  const          10.960  12.504   1.14      coerce          0.979   0.756   0.77
  read.field      9.246   7.552   0.82      alloc           0.504   0.424   0.84
  math            8.832  12.923   1.46      istype          0.415   0.272   0.66
  native          6.454   4.444   0.69      tostring        0.113   0.171   1.51
  read.global     5.462   2.626   0.48      string          0.103   0.095   0.92
  control         4.773   4.876   1.02      switch          0.036   0.014   0.37
  for             4.666   3.466   0.74
  call            2.862   1.669   0.58
  return          2.488   1.669   0.67
  refcount        2.031   1.829   0.90

  read.local.int         9.906 14.104 1.42   math.int.add         1.366  2.987   2.19
  read.local.class       6.705  7.865 1.17   for.iter.vector      1.360  0.831   0.61
  const.int              6.475  8.459 1.31   lval.writeref.local  1.329  1.235   0.93
  lval.write.local       6.144  6.404 1.04   math.int.mul         1.307  1.187   0.91
  const.float            3.323  3.044 0.92   return.local.rets0   1.178  0.531   0.45
  read.global.class      3.071  1.479 0.48   native.push          1.149  0.424   0.37
  if.then                2.862  2.941 1.03   read.local.int2      1.087  1.343   1.24
  read.field.int         2.702  2.647 0.98   read.global.vector   1.073  0.769   0.72
  read.field.class       2.402  0.884 0.37   return.local.rets1   1.071  0.920   0.86
  lval.writev.local      2.250  1.653 0.73   read.local.int3      0.829  0.208   0.25
  for.iter.int           2.001  1.855 0.93   read.local.float2    0.702  0.075   0.11
  refcount.inc           1.769  1.561 0.88   bitop.and            0.674  1.074   1.59
  read.local.vector      1.766  1.242 0.70   math.int.eq          0.642  0.606   0.94
  read.local.float       1.700  2.304 1.36   read.field.int3      0.638  0.162   0.25
  read.field.vector      1.581  1.568 0.99   const.function       0.636  0.187   0.29

Total variation distance to the target is 0.27, and 88.9% of the target's event mass falls in a
category this benchmark also reaches.

WHAT IS NOT REPRESENTED
- Graphics, audio, physics and file builtins. gl.* alone is 1.5% of the corpus, all of it in
  frame code, and a benchmark that opens a window measures the driver rather than the compiler.
  Their share is spread over the CPU builtins instead.
- The long tail: 791 counters below 0.001% each, 0.1% of events between them. Reproducing those
  would be contorting the code for nothing.
- The remaining known skews, kept rather than papered over: less arithmetic per statement would
  need an accumulator that the optimizer can still not remove; global reads are short because
  matvoxels' dynamic scope variables have no equivalent here; struct locals and pushes are the
  categories most worth adding to if this file is ever extended.

DOING THIS AGAIN
Instrument codegen with whatever categories matter then, run the corpus the same way, mix the
per program counts with the shares and log weighting above, and measure this file the same way to
compare. `-- <section> [work_scale]` runs one section on its own, which is how the per section
profiles behind the fitted weights were measured, and `work_scale` is the only number to turn to
change how long a run takes.

Captured 2026-09-06 against lobster 631239ee, with matvoxels built from the same instrumented
compiler.
*/

import std
import vec

// ---------------------------------------------------------------------------------------------
// Shared state.
//
// Real Lobster code reaches for globals a lot: matvoxels threads its world, its settings and its
// asset stores thru functions as dynamic scope variables, which codegen sees as globals. Reading
// a global class or vector is 4% of everything the corpus runs, so the sections below read this
// state where they use it instead of caching it in a local first.
// ---------------------------------------------------------------------------------------------

let grid_size = int2 { 24, 24 }
let num_entities = 96
let num_cells = grid_size.x * grid_size.y

enum EntityKind:
    ek_walker
    ek_flyer
    ek_turret
    ek_prop

class Item:
    name:string
    amount:int
    value:float
    tint:float3

class Entity:
    id:int
    kind:EntityKind
    cell:int3
    pos:float3
    vel:float3
    home:Item
    held:Item
    health:float
    flags:int
    label:string
    items:[Item]
    trail:[int2]
    target:Entity? = nil

    def alive() -> bool: return health > 0.0
    def rating() -> int: return id + flags
    def tick(dt:float) -> float: return health - dt

class Walker : Entity
    stride = 1.0
    def rating(): return id + flags * 2
    def tick(dt:float): return health - dt * stride

class Flyer : Entity
    altitude = 4.0
    def rating(): return id + flags * 3
    def tick(dt:float): return health - dt * 0.5 + altitude * 0.01

class Turret : Entity
    range_sq = 64.0
    def rating(): return id - flags
    def tick(dt:float): return health - dt * 0.25

class World:
    entities:[Entity]
    player:Entity
    cells:[int]
    heights:[[int]]
    points:[float3]
    corners:[int3]
    stock:[Item]
    frame:int
    origin:int3
    wind:float3

let null_item = Item { "none", 0, 0.0, float3_0 }
let null_entity = Entity { 0, ek_prop, int3_0, float3_0, float3_0, null_item, null_item, 0.0, 0,
                           "", [], [] }

// Rebuilt at the start of every trial, then read as a global everywhere below.
var world = World { [], null_entity, [], [], [], [], [], 0, int3_0, float3_0 }
var frame_no = 0
var tally = 0

// A function value kept in a variable and one passed as a typed argument, which is how the
// corpus mostly uses them: stored and called back rather than written out at the callsite.
def scoref(a:int, b:int) -> int

var scorer:scoref = fn(a:int, b:int) -> int: a + b

def combine(xs:[int], f:scoref) -> int:
    var a = 0
    for(xs) v: a = f(a, v) & 0xFFFFF
    return a

class Rule:
    name:string
    limit:int
    apply:scoref

def rule_table(round:int) -> [Rule]:
    return [ Rule { "add",   16 + round, fn(a:int, b:int) -> int: a + b },
             Rule { "scale",  8 + round, fn(a:int, b:int) -> int: a * 3 + b },
             Rule { "mask",  32 + round, fn(a:int, b:int) -> int: (a + b) & 63 } ]

// ---------------------------------------------------------------------------------------------
// Setup, which builds the object graph a trial works on. Programs in the corpus spend real time
// constructing state, so this is measured along with the rest rather than hoisted out of it.
// ---------------------------------------------------------------------------------------------

def make_items(n:int, seed:int) -> [Item]:
    return map(n) i: Item { "it", (i * 7 + seed) % 23, i * 0.5, float3 { 0.5, i * 0.1, 0.25 } }

def make_entity(i:int, seed:int) -> Entity:
    let cell = int3 { i % grid_size.x, (i / grid_size.x) % grid_size.y, i % 5 }
    let pos = float3 { cell.x * 0.5, cell.y * 0.5, i % 7 }
    let vel = float3 { 0.25, -0.125, 0.0625 }
    let home = Item { "home", i % 13, i * 0.25, float3_1 }
    let held = Item { "held", i % 5, 1.5, float3 { 0.25, 0.5, 0.75 } }
    let hp = float((i + seed) % 64 + 8)
    let fl = (i * 3 + seed) % 16
    let items = make_items(2 + i % 3, seed + i)
    let trail = map(3) j: int2 { (i + j) % 8, j }
    switch i % 4:
        case 0: return Walker { i, ek_walker, cell, pos, vel, home, held, hp, fl, "walk", items,
                                trail }
        case 1: return Flyer  { i, ek_flyer,  cell, pos, vel, home, held, hp, fl, "fly",  items,
                                trail }
        case 2: return Turret { i, ek_turret, cell, pos, vel, home, held, hp, fl, "turr", items,
                                trail }
    return Entity { i, ek_prop, cell, pos, vel, home, held, hp, fl, "prop", items, trail }

def setup(seed:int):
    let entities = map(num_entities) i: make_entity(i, seed)
    for(entities) e, i:
        e.target = if i % 3: entities[(i * 7 + 1) % num_entities] else: nil
    world = World {
        entities,
        entities[0],
        map(num_cells) i: (i * 13 + seed) % 251,
        map(grid_size.y) y: map(grid_size.x) x: (x * x + y * seed) % 97,
        map(64) i: float3 { i % 8, i / 8, i % 3 },
        map(64) i: int3 { i % 4, (i / 4) % 4, i / 16 },
        make_items(12, seed),
        seed,
        int3 { 2, 3, 4 },
        float3 { 0.125, 0.0, -0.25 }
    }
    frame_no = seed
    tally = seed
    if seed % 2:
        scorer = fn(a:int, b:int) -> int: a + b * 2
    else:
        scorer = fn(a:int, b:int) -> int: a * 2 + b

// ---------------------------------------------------------------------------------------------
// 1. Entities: the shape most of matvoxels is. A pass over a list of heap objects that reads and
// writes their fields, follows nullable links, calls methods and void helpers on them, and
// dispatches on their type. Field reads together are 9% of everything the corpus runs, and only
// 9% of it is arithmetic, so these bodies move values around far more than they compute with
// them.
// ---------------------------------------------------------------------------------------------

def wrap(v:int, m:int) -> int:
    return if v >= m: v % m else: v

// The helpers the sections share. The short ones get folded into their callers, as short
// accessors in real code do; the ones from cell_score down are past the size the optimizer
// inlines at and have more than one caller, so they stay real calls.
def bucket_of(v:int, n:int) -> int:
    let b = wrap(v, n)
    let hi = n - 1
    if b < 0: return 0
    if b > hi: return hi
    return b

def cell_score(c:int3, o:int3) -> int:
    let d = c - o
    var s = abs(d.x) + abs(d.y)
    if d.z > 0: s += abs(d.z)
    s = min(64, max(0, s))
    return s + bucket_of(s, 8)

// An int position turned into a colour, which is the shape of every palette lookup: an int to
// float conversion per component and a few masks.
def cell_tint(c:int3) -> float3:
    let m = int3 { c.x & 7, c.y & 7, c.z & 3 }
    let f = float3 { m.x, m.y, m.z } * 0.125
    if f.x > 0.5: return float3 { 1.0, f.y, f.z }
    return f

def blend_color(a:float3, b:float3, t:float) -> float3:
    let u = 1.0 - t
    let r = a * u + b * t
    if r.x > 1.0: return float3 { 1.0, r.y, r.z }
    if r.z < 0.0: return float3 { r.x, r.y, 0.0 }
    return r

def update_item(it:Item, round:int, tint:float3):
    it.amount = wrap(it.amount + round, 29)
    it.value = it.value * 0.5 + 0.125
    it.tint = blend_color(it.tint, tint, 0.25)
    if it.value > 8.0: it.value = 1.0
    if it.amount > 24: it.name = "big"
    for(3) k:
        it.value += max(0.0, tint.x - k * 0.25)

def push_trail(e:Entity, p:int2):
    e.trail.push(p)
    let n = e.trail.length
    if n > 6:
        e.trail.remove(0)
        e.flags = e.flags | 4
    if n < 2: e.trail.push(p)
    for(min(3, e.trail.length)) k:
        e.flags = e.flags | (e.trail[k].x & 1)

def mix2(a:int, b:int) -> int:
    return a + b * 3

def is_hot(e:Entity) -> bool:
    return (e.flags & 1) != 0

def near(a:float3, b:float3, r:float) -> bool:
    let d = b - a
    return dot(d, d) < r

def step_cell(c:int3) -> int3:
    return int3 { c.x, c.y, wrap(c.z + 1, 5) }

def item_of(e:Entity, i:int) -> Item:
    return e.items[wrap(i, e.items.length)]

def score_of(it:Item) -> int:
    return it.amount + it.name.length + int(abs(it.value))

def clamp01(f:float) -> float:
    return min(1.0, max(0.0, f))

// The distance test that decides almost everything in a game frame.
def in_reach(a:Entity, b:Entity, r:float) -> bool:
    let d = b.pos - a.pos
    if abs(d.z) > r: return false
    let m = magnitude(d)
    return m < r and b.alive()

// Void helpers taking a couple of arguments, which is the most common call shape in the corpus.
def damage(e:Entity, amount:float, from_id:int):
    e.health = max(0.0, e.health - amount)
    e.flags = e.flags | (from_id & 3)

def move(e:Entity, by:float3):
    e.pos += by
    let c = step_cell(e.cell)
    e.cell = c
    push_trail(e, int2 { c.x, c.y })

def restock(e:Entity, n:int):
    let h = e.home
    h.amount = wrap(h.amount + n, 19)
    h.value = clamp01(h.value * 0.5)
    if e.items.length < 6:
        e.items.push(Item { e.label, n, h.value, h.tint })

def entity_pass() -> int:
    var acc = 0
    let dt = 0.0625
    for(world.entities) e, i:
        e.health = e.tick(dt)
        let items = e.items
        let cell = e.cell
        let ground = float2 { e.pos.x, e.pos.y }
        e.pos = float3 { ground.x, ground.y, world.origin.z * 0.5 }
        // Both ways of getting at a field appear on purpose: bound to a local once, and
        // reached thru the chain each time, which is the split the corpus has.
        let home = e.home
        let held = e.held
        held.value = clamp01(home.value + world.wind.x)
        if ground.x > ground.y and home.amount > held.amount:
            held.amount = home.amount
        e.held.tint = blend_color(e.held.tint, e.home.tint, 0.5)
        acc += cell_score(cell, world.origin) + cell.x + cell.y
        if is_hot(e):
            move(e, e.vel + world.wind)
            damage(e, 0.5, i)
        elif e.health > 24.0:
            move(e, e.vel * -0.5)
            e.flags = wrap(mix2(e.flags, world.frame), 16)
        else:
            e.health += 1.5
            restock(e, i)
        let t = e.target
        if t and in_reach(e, t, 10.0):
            if near(e.pos, t.pos, 96.0):
                damage(t, 0.25, e.id)
                t.home.amount = e.home.amount
            elif is_hot(t):
                t.held.value = clamp01(e.held.value)
                t.held.name = e.held.name
        if e is Turret:
            e.home.amount = wrap(e.rating(), 32)
        elif e is Flyer:
            e.held.amount = wrap(e.id, 16)
        elif e is Walker:
            e.held.name = e.label
        if e.held.name == e.home.name:
            e.held.name = "held"
        if e.label == "walk" or e.label == "fly":
            acc += e.label.length
        for(items) it:
            update_item(it, world.frame, e.home.tint)
        for(e.trail.length) k:
            let tr = e.trail[k]
            e.held.amount = mix2(tr.x, tr.y)
        for(4) k:
            acc += min(k, e.items.length) + abs(cell.z - k) + frame_no
        if e is Walker and e.target is Walker: acc += 1
        if items.length > 4: items.pop()
        if items.length < 3: items.push(Item { e.label, i, 1.0, cell_tint(cell) })
        e.health = min(96.0, e.health + i * 0.001)
        if e.target is Turret: acc += 2
        acc += score_of(item_of(e, i)) + e.rating() + max(cell.z, items.length)
    return acc

def entity_query() -> int:
    var acc = 0
    for(world.entities) e:
        guard e.alive()
        guard e.flags & 2
        // A hand rolled best-of loop over a list of objects, which is what game code does once
        // the score it ranks by needs more than one field.
        var best_score = -1
        var best:Entity? = nil
        for(world.entities) o:
            let sc = wrap(o.rating() + score_of(o.home) + score_of(o.held), 97)
            if sc > best_score:
                best_score = sc
                best = o
        if best and best.id != e.id:
            acc += best.id + e.label.length
        if e.items.length > 2:
            acc += score_of(item_of(e, world.frame))
        if e is Walker or e is Flyer:
            acc += min(64, max(0, e.flags))
        if best is Turret: acc += 1
        elif best is Walker: acc += 2
        if world.player is Walker: acc += world.player.flags
        if e.target is Flyer: acc += 1
    let p = world.player
    let stock = world.stock
    for(stock) it:
        if p.home.name == it.name: acc += 1
        if p.held.value > it.value: acc += 2
    acc += p.id + p.home.amount + p.held.amount + p.items.length + stock.length
    return acc

// ---------------------------------------------------------------------------------------------
// 2. Grid: integer and inline-struct code over flat and nested vectors, the shape voxel and tile
// work has. int and int2/int3 arithmetic, bit masking, index reads and in-place index writes,
// and counted rather than iterator loops.
// ---------------------------------------------------------------------------------------------

let neighbour_offsets = [ int2 { 1, 0 }, int2 { -1, 0 }, int2 { 0, 1 }, int2 { 0, -1 } ]

def grid_index(p:int2) -> int:
    return p.x + p.y * grid_size.x

def grid_at(p:int2) -> int:
    if not in_range(p, grid_size): return 0
    return world.cells[grid_index(p)]

def grid_set(p:int2, v:int):
    if not in_range(p, grid_size): return
    world.cells[grid_index(p)] = v

def neighbour_sum(p:int2) -> int:
    var n = 0
    for(neighbour_offsets.length) i:
        n += grid_at(p + neighbour_offsets[i])
    for(2) i:
        n += max(0, grid_at(p + neighbour_offsets[i] * 2))
    return n

def grid_pass() -> int:
    var acc = 0
    for(grid_size.y) y:
        for(grid_size.x) x:
            let p = int2 { x, y }
            let here = grid_at(p)
            let v = (here + neighbour_sum(p) + world.frame) & 255
            grid_set(p, v)
            if v > 128:
                grid_set(p + neighbour_offsets[0], v >> 3)
            if v == here:
                grid_set(p + neighbour_offsets[2], v | 1)
            if v & 3:
                acc += world.cells[grid_index(p)] & 15
            acc += v >> 4
    return acc

def height_pass() -> int:
    var acc = 0
    for(world.heights.length) y:
        let row = world.heights[y]
        for(row.length) x:
            let d = wrap(mix2(row[x], x * y) + world.frame, 97)
            row[x] = d
            if d > 48: acc += d
            acc += row[x] & 7
        for(4) k:
            acc += max(row[k], k) | k
        acc += row.length + min(row[0], row[1])
    for(world.corners) c:
        let q = c * 3 - world.origin
        acc += cell_score(q, world.origin)
        if q == world.origin: acc += 3
        if q.x > q.z: acc += q.x
        acc += bucket_of(q.z, 8)
    return acc

// ---------------------------------------------------------------------------------------------
// 3. Numeric: float scalars and float2/3/4 vectors, which is what camera, physics and
// procedural generation code is, including the int to float conversions those mix in.
// ---------------------------------------------------------------------------------------------

def falloff(d:float, r:float) -> float:
    if d >= r: return 0.0
    var t = 1.0 - d / r
    for(3) k:
        t = t * 0.75 + min(0.25, abs(t) * 0.1 + k * 0.01)
    return t * t

def track(p:float3, to:float3, rate:float) -> float3:
    return p + (to - p) * rate

def numeric_pass() -> int:
    var acc = 0
    var sum2 = float2_0
    var sum3 = float3_0
    let base = world.frame * 0.03125
    for(world.points) p, i:
        let q = p * 0.5 + world.wind
        let r = track(q, world.wind, 0.25)
        sum3 += r * 0.75
        let m = magnitude(r)
        let uv = float2 { q.x, q.y } * (0.5 + base)
        let duv = uv - sum2 * 0.01
        sum2 += uv
        if duv.x > duv.y: acc += int(duv.x)
        let sc = sincos(wrap(i, 12) * 30.0)
        if m > 2.0:
            world.points[i] = blend_color(q, float3 { uv.x, uv.y, sqrt(m) }, 0.25)
        if sc.x > 0.0 and m < 40.0:
            world.points[i] = track(r, float3 { sc.x, sc.y, uv.x }, 0.5)
        if not (i % 5):
            acc += int(clamp01(uv.y) * 8.0)
        let tint = cell_tint(int3 { i, i / 2, i / 4 })
        if tint.x > uv.x:
            sum2 += float2 { tint.y, tint.z }
        acc += int(falloff(m, 8.0) * 16.0) + int(lerp(0.0, 4.0, uv.y))
    let n3 = normalize(sum3 + float3_1)
    acc += int(magnitude(sum2) + n3.x * 4.0)
    acc += int(min(64.0, max(-64.0, sum3.y)))
    return acc

// ---------------------------------------------------------------------------------------------
// 4. Containers: building and consuming vectors, which every prototype does. push is the single
// most called builtin in the corpus, followed by length.
// ---------------------------------------------------------------------------------------------

def container_pass() -> int:
    var acc = 0
    let ints = []
    let names = []
    let spots = []
    let cells = []
    for(96) i:
        ints.push(wrap(i * 17 + world.frame, 211))
        spots.push(int2 { i % 8, i / 8 })
        if i % 3 == 0: names.push(if i % 2: "odd" else: "even")
        if i % 2 == 0: cells.push(int3 { i % 4, i % 3, i % 5 })
        if i % 5 == 0: world.stock.push(Item { "made", i, i * 0.5, cell_tint(cells[0]) })
    while world.stock.length > 12: world.stock.pop()
    acc += ints.length + names.length + spots.length + cells.length
    let evens = []
    for(ints) v:
        if v & 1: evens.push(v)
        acc += wrap(v, 101)
    acc += evens.length
    for(names) n:
        if n == "odd": acc += n.length
    for(spots) sp:
        if sp.x > sp.y: acc += mix2(sp.x, sp.y)
    for(cells) c:
        if c.x == c.y: acc += mix2(c.z, c.y)
    for(world.stock) it:
        acc += score_of(it)
    let picked = filter(ints) v: v > 90
    acc += picked.length
    acc += combine(ints, scorer)
    for(rule_table(world.frame)) r:
        acc += combine(picked, r.apply) + r.limit + r.name.length
    qsort_in_place(ints): _a < _b
    for(8) k:
        acc += ints[k] + picked.length
        acc += min(ints[k], spots[k].x)
    acc += ints[0] + ints[ints.length - 1]
    let stack = []
    for(ints) v:
        stack.push(v)
        if stack.length > 8:
            acc += stack.top()
            stack.pop()
    while stack.length:
        acc += stack.pop() & 31
    let made = []
    let tints = []
    for(cells) c:
        tints.push(cell_tint(c))
    for(tints) t, i:
        if t.x > 0.5: acc += i
    acc += tints.length
    for(world.stock) it, i:
        update_item(it, world.frame + i, float3_1)
        made.push(Item { it.name, it.amount, it.value, it.tint })
        acc += score_of(it)
    for(made) it:
        update_item(it, world.frame, it.tint)
        acc += it.amount
    acc += made.length
    return acc

// ---------------------------------------------------------------------------------------------
// 5. Actions: the pending-event queue every game grows, which in matvoxels is a good part of
// what a frame does. Handlers take a few arguments and return nothing, read the world thru
// globals rather than being handed it, and queue follow-up work as they go. This is where most
// of the corpus's void calls, global reads and short lived allocations live.
// ---------------------------------------------------------------------------------------------

enum ActionKind:
    act_move
    act_hit
    act_pick
    act_note

class Action:
    kind:ActionKind
    who:int
    arg:int
    at:int2
    tag:string

var queue:[Action] = []
var pending:[Action] = []

def enqueue(kind:ActionKind, who:int, arg:int, at:int2):
    pending.push(Action { kind, who, arg, at, if arg & 1: "odd" else: "even" })

// Turrets do not act for themselves, so their events land on the player instead, which is the
// kind of redirect a game's event handling is full of.
def actor(who:int) -> Entity:
    let es = world.entities
    let e = es[wrap(who, es.length)]
    if e is Turret: return world.player
    if e is Flyer and not e.alive(): return world.player
    return e

def do_move(who:int, at:int2):
    let e = actor(who)
    e.cell = int3 { at.x, at.y, e.cell.z }
    push_trail(e, at)
    enqueue(act_note, who, cell_score(e.cell, world.origin), at)

def do_hit(who:int, arg:int):
    let e = actor(who)
    damage(e, 0.5, arg)
    if e.alive():
        enqueue(act_pick, who + 1, arg, int2 { arg & 7, arg & 3 })

def do_pick(who:int, arg:int):
    let e = actor(who)
    restock(e, arg)
    update_item(e.held, arg, e.home.tint)
    tally += e.home.amount

def do_note(who:int, arg:int):
    let rules = rule_table(who)
    let r = rules[wrap(arg, rules.length)]
    let f = r.apply
    tally = (tally * 3 + f(who, arg)) & 0xFFFFF

def action_pass() -> int:
    var acc = tally & 7
    queue = []
    pending = []
    let log:[int2] = []
    for(48) i:
        enqueue(ActionKind(i % 4), i, i * 3 + world.frame, int2 { i % 8, (i / 8) % 6 })
    for(8) k:
        acc += min(k, pending.length) + world.entities.length
    for(3) wave:
        for(log) at:
            if at.x > at.y: acc += 1
        while log.length: log.pop()
        queue = pending
        pending = []
        for(queue) a:
            if a.kind == act_move: do_move(a.who, a.at)
            elif a.kind == act_hit: do_hit(a.who, a.arg)
            elif a.kind == act_pick: do_pick(a.who, a.arg)
            else: do_note(a.who, a.arg)
            if a.tag == "odd": acc += a.arg & 7
            log.push(a.at)
            acc += mix2(a.who, a.at.y) + frame_no
        for(log) at:
            acc += at.x
        acc += (queue.length + wave + tally) & 15
    acc += tally & 0xFFFF
    return acc

// ---------------------------------------------------------------------------------------------
// 6. Logic: the plumbing between the above. Calls of a few arguments returning nothing, one
// value or several, conditionals, boolean operators, asserts, and the occasional switch. Kept
// deliberately shallow, as the corpus's calls are.
// ---------------------------------------------------------------------------------------------

def blend(a:int, b:int) -> int:
    return (a * 3 + b) & 0xFFFF

def clamp_step(v:int, lo:int, hi:int) -> int:
    if v < lo: return lo
    if v > hi: return hi
    return v + 1

def split_value(v:int) -> int, float:
    return v / 2, v * 0.25

def weigh(a:int, b:int, c:float, d:float) -> float:
    return a * c + b * d

def note(slot:int, v:int):
    tally = (tally * 3 + slot + v) & 0xFFFFF

def classify(v:int) -> int:
    switch v % 7:
        case 0, 1: return 1
        case 2: return 2
        case 3, 4, 5: return 3
    return 4

def logic_pass() -> int:
    var acc = world.frame
    for(160) i:
        acc = blend(acc, i)
        acc = clamp_step(acc, -1000000, 1000000)
        let h, f = split_value(acc)
        note(i & 7, h)
        if int(f) % 3 == 0:
            acc += classify(i)
        elif not (i % 5):
            acc -= 1
        else:
            acc += 2
        if acc == i: acc += 7
        acc += int(weigh(i, h, 0.5, 0.25)) & 31
        assert acc > -100000000
    var i = 0
    while i < 48:
        note(i, acc)
        i += 2
    acc += tally + frame_no
    return acc

// ---------------------------------------------------------------------------------------------
// 7. Text: string building and comparison, a small but real slice of the corpus, coming mostly
// from naming, lookups and debug output.
// ---------------------------------------------------------------------------------------------

def text_pass() -> int:
    var acc = 0
    for(32) i:
        let it = world.stock[wrap(i, world.stock.length)]
        let n = it.name
        acc += n.length
        if n == "it": acc += 2
        elif n > "h": acc += 1
        let e = world.entities[wrap(i, world.entities.length)]
        if e.label == n: acc += 1
        if e.held.name != n: acc += 2
        let s = "{e.label}{i}/{world.frame % 10}"
        acc += s.length
        if s == "walk0/0": acc += 5
        if i % 4 == 0:
            acc += (n + "-" + string(it.amount)).length
    return acc

// ---------------------------------------------------------------------------------------------
// Driver: interleave the sections and take the minimum over the meta runs, the same way
// speedtest.lobster does, so that scheduling noise does not decide which build looks faster.
// ---------------------------------------------------------------------------------------------

def sectionf() -> int

class Section:
    name:string
    f:sectionf
    runs:int
    best:float = 999999.0

let sections = [
    Section { "entities",   fn(): entity_pass(),     30 },
    Section { "query",      fn(): entity_query(),     2 },
    Section { "grid",       fn(): grid_pass(),        8 },
    Section { "heights",    fn(): height_pass(),     15 },
    Section { "numeric",    fn(): numeric_pass(),    45 },
    Section { "containers", fn(): container_pass(),  12 },
    Section { "actions",    fn(): action_pass(),    300 },
    Section { "logic",      fn(): logic_pass(),      30 },
    Section { "text",       fn(): text_pass(),      275 },
]

// How often each section's `runs` weight is repeated, which is the one number to turn when the
// whole benchmark should take longer or shorter. Everything else is a shape, not an amount.
let work_scale = 24
let meta_runs = 5
let rounds = 30

// Optional program arguments: `-- <section> [work_scale]` runs only that section, which is how
// the per section counter profiles behind the header above were measured.
let args = command_line_arguments()
let only = if args.length > 0: args[0] else: ""
let scale = if args.length > 1: string_to_int(args[1]) else: work_scale

// A fixed amount of work whose result does not depend on how long the timed part runs, so the
// assert below keeps its meaning when work_scale is turned up or down. Every value the sections
// compute feeds into it, which is also what keeps the optimizer from throwing their bodies away.
def verify() -> int:
    var v = 0
    for(4) r:
        setup(r + 1)
        for(sections) s:
            let run = s.f
            world.frame = r
            v = (v * 31 + run()) & 0xFFFFFFFFFFFF
    return v

let expected = verify()
print "verify: {expected}"
assert expected == 248898468858485

var checksum = 0
for(meta_runs) m:
    for(sections) s:
        if only.length and s.name != only: continue
        setup(1)
        let run = s.f
        let start = seconds_elapsed()
        var local = 0
        for(rounds) r:
            world.frame = r
            for(s.runs * scale): local += run()
        let taken = seconds_elapsed() - start
        s.best = min(s.best, taken)
        if m == 0: checksum = (checksum * 31 + local) & 0xFFFFFFFFFFFF

var total = 0.0
for(sections) s:
    if only.length and s.name != only: continue
    total += s.best
    print "{s.name}: {s.best}"
print "TOTAL: {total}"
print "checksum: {checksum}"
