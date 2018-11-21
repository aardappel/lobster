include vec

// For each position, indicates the direction you should move to when on this cell, out of 8 possible
// cardinal and diagonal directions.
struct nav_map { grid:[[xy_i]], start:xy_i }

// blocked_cost should be a multiple of the base cost 1, or 0 to disallow movement over blocked areas completely.
// is_blocked should say for any world position whether it is blocked or not.
// allow_diag says if you want to automatically create diagonal steps where possible.
// TODO: more efficient to make a seperate blocked Q for efficiency
def create_nav_map(start:xy_i, worlddim:xy_i, allow_diag:int, blocked_cost:int, is_blocked):
    struct nav_cell { pdir:xy_i, pos:xy_i, psteps:int = -1, cost:int = 0 }
    assert blocked_cost or !is_blocked(start)
    pathq := []::nav_cell
    grid := mapxy(worlddim) v: nav_cell { xy_0i, v }
    startcell := grid[start]
    startcell.psteps = 0
    startcell.pos = start
    pathq.push(startcell)
    while pathq.length:
        c := pathq.pop()
        for(cardinal_directions) d:
            dpos := c.pos - d
            if inrange(dpos, worlddim):
                cost := 1
                if !is_blocked(dpos) or (cost = blocked_cost):
                    n := grid[dpos]
                    if n.psteps >= 0:
                        if allow_diag and cost == 1 and n.cost == 1 and c.cost == 1 and
                           n.psteps == c.psteps + 1 and
                           n.pdir.x * n.pdir.y == 0 and n.pdir.x * d.x == 0 and n.pdir.y * d.y == 0:
                            diag := n.pdir + d
                            if !is_blocked(n.pos + diag):
                                n.pdir = diag
                    else:
                        n.psteps = c.psteps + cost
                        n.pdir = d  // All these vecs are shared, so low final memory cost.
                        n.pos = dpos
                        n.cost = cost
                        i := pathq.length
                        while i > 0 and pathq[i - 1].psteps < n.psteps: i--
                        pathq.insert(i, n)
    nav_map { mapxy(worlddim) v: grid[v].pdir, start }

