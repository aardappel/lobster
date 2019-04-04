import vec

class pathmap:
    steps:int
    dir:xy_i
    pos:xy_i

// This creates a 2d flood fill map where each cell has a direction that points to a the
// neighboring cell that eventually brings it to the target location. Useful for games where lots
// of units all need to path to the same location (e.g. the player).
// Pass it the size of the world, and the target.
// traversablef receives a position and returns if that location is traversable.
def floodfillpath(sz:xy_i, target:xy_i, traversablef):
    let m = mapxy(sz) v: pathmap { -1, xy_0i, v }
    let end = m[target]
    end.steps = 0
    let pathq = [ end ]
    def flood(dir, parent):
        let pos = parent.pos + dir
        if pos.in_range(sz):
            let c = m[pos]
            if traversablef(pos) and c.steps < 0:
                c.steps = parent.steps + 1
                c.dir = -dir
                var i = pathq.length
                while i > 0 and pathq[i - 1].steps < c.steps: i--
                pathq.insert(i, c)
    while pathq.length:
        let c = pathq.pop
        for(2) x: for(2) y:
            flood(xy { 1 - y - x, y - x }, c)
    return m

