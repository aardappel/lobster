include "vec.lobster"

struct pathmap:
    steps:int
    dir:xy_i
    pos:xy_i

// This creates a 2d flood fill map where each cell has a direction that points to a the
// neighboring cell that eventually brings it to the target location. Useful for games where lots
// of units all need to path to the same location (e.g. the player).
// Pass it the size of the world, and the target.
// traversablef receives a position and returns if that location is traversable.
def floodfillpath(sz:xy_i, target:xy_i, traversablef):
    m := mapxy(sz) v: pathmap { -1, xy_0i, v }
    end := m[target]
    end.steps = 0
    pathq := [ end ]
    def flood(dir, parent):
        pos := parent.pos + dir
        if pos.inrange(sz):
            c := m[pos]
            if traversablef(pos) and c.steps < 0:
                c.steps = parent.steps + 1
                c.dir = -dir
                i := pathq.length
                while i > 0 and pathq[i - 1].steps < c.steps: i--
                pathq.insert(i, c)
    while pathq.length:
        c := pathq.pop
        for(2) x: for(2) y:
            flood(xy { 1 - y - x, y - x }, c)
    m

