import vec

// For each position, indicates the direction you should move to when on this cell, out of 8 possible
// cardinal and diagonal directions.
class nav_map:
    grid:[[int2]]
    start:int2

// blocked_cost should be a multiple of the base cost 1, or 0 to disallow movement over blocked areas completely.
// is_blocked should say for any world position whether it is blocked or not.
// allow_diag says if you want to automatically create diagonal steps where possible.
// TODO: more efficient to make a seperate blocked Q for efficiency
def create_nav_map(start:int2, worlddim:int2, allow_diag:int, blocked_cost:int, is_blocked):
    class nav_cell:
        pdir:int2
        pos:int2
        psteps = -1
        cost = 0
    assert blocked_cost or not is_blocked(start)
    let pathq = []::nav_cell
    let grid = mapxy(worlddim) v: nav_cell { int2_0, v }
    let startcell = grid[start]
    startcell.psteps = 0
    startcell.pos = start
    pathq.push(startcell)
    while pathq.length:
        let c = pathq.pop()
        for(cardinal_directions) d:
            let dpos = c.pos - d
            guard in_range(dpos, worlddim)
            var cost = 1
            guard not is_blocked(dpos) or (cost = blocked_cost)
            let n = grid[dpos]
            if n.psteps >= 0:
                if allow_diag and cost == 1 and n.cost == 1 and c.cost == 1 and
                   n.psteps == c.psteps + 1 and
                   n.pdir.x * n.pdir.y == 0 and n.pdir.x * d.x == 0 and n.pdir.y * d.y == 0:
                    let diag = n.pdir + d
                    if not is_blocked(n.pos + diag):
                        n.pdir = diag
            else:
                n.psteps = c.psteps + cost
                n.pdir = d  // All these vecs are shared, so low final memory cost.
                n.pos = dpos
                n.cost = cost
                var i = pathq.length
                while i > 0 and pathq[i - 1].psteps < n.psteps: i--
                pathq.insert(i, n)
    return nav_map { mapxy(worlddim) v: grid[v].pdir, start }

