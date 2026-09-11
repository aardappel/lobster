// A* over a random 4-connected grid, see ../SPEC.md section 5. Integer only, so the checksum
// is exact in every language.

import std
import .rnd

private let W = 512
private let H = 512
private let ROUNDS = 20
private let WALL_P = 0.20
private let INF = 0x3FFFFFFF

// The open list holds the f score and the node, and orders on the node index when two f scores
// are equal, so the expansion order is fully determined.
private struct HeapEnt:
    f:int
    n:int

private def before(a:HeapEnt, b:HeapEnt) -> bool:
    return a.f < b.f or (a.f == b.f and a.n < b.n)

private def hpush(heap:[HeapEnt], e:HeapEnt):
    heap.push(e)
    var i = heap.length - 1
    while i > 0:
        let p = (i - 1) >> 1
        let pe = heap[p]
        if not before(e, pe): break
        heap[i] = pe
        i = p
    heap[i] = e

private def hpop(heap:[HeapEnt]) -> HeapEnt:
    let top = heap[0]
    let last = heap.pop()
    let n = heap.length
    if n > 0:
        var i = 0
        while true:
            let l = (i << 1) + 1
            if l >= n: break
            var c = l
            let r = l + 1
            if r < n and before(heap[r], heap[l]): c = r
            let ce = heap[c]
            if not before(ce, last): break
            heap[i] = ce
            i = c
        heap[i] = last
    return top

// The cost of the best path and the number of nodes expanded, with a cost of -1 where the goal
// could not be reached. Everything the search touches is a local: a variable a nested function
// reads from an enclosing scope lives in the VM's global frame instead, and is re-loaded from
// there on every access.
private def solve(seed:int) -> int, int:
    let rnd = Rnd { seed }
    let grid = []::int
    grid.vector_capacity(W * H)
    for(W * H):
        grid.push(if rnd.nextf() < WALL_P: 1 else: 0)
    let goal = W * H - 1
    // The two corners and their orthogonal neighbours are forced open: a corner whose only two
    // neighbours are both walls is trapped, which turns a whole round into no work at all.
    grid[0] = 0
    grid[1] = 0
    grid[W] = 0
    grid[goal] = 0
    grid[goal - 1] = 0
    grid[goal - W] = 0
    let gscore = map(W * H): INF
    let closed = map(W * H): 0
    let heap = []::HeapEnt
    let dirs = [ int2 { 1, 0 }, int2 { 0, 1 }, int2 { -1, 0 }, int2 { 0, -1 } ]
    let gx = W - 1
    let gy = H - 1
    gscore[0] = 0
    hpush(heap, HeapEnt { gx + gy, 0 })
    var expanded = 0
    var cost = -1
    while heap.length > 0:
        let e = hpop(heap)
        let n = e.n
        if closed[n] == 0:
            closed[n] = 1
            expanded++
            if n == goal:
                cost = gscore[n]
                break
            let y = n / W
            let x = n - y * W
            let g = gscore[n] + 1
            for(dirs) d:
                let nx = x + d.x
                let ny = y + d.y
                if nx >= 0 and ny >= 0 and nx < W and ny < H:
                    let nb = n + d.y * W + d.x
                    if grid[nb] == 0 and closed[nb] == 0 and g < gscore[nb]:
                        gscore[nb] = g
                        // Neither coordinate can exceed the goal's, so this needs no abs().
                        hpush(heap, HeapEnt { g + (gx - nx) + (gy - ny), nb })
    return cost, expanded

def bench_astar() -> float:
    var sum = 0
    for(ROUNDS) r:
        let cost, expanded = solve(1000 + r)
        sum += cost * 1000003 + expanded
    return float(sum)
