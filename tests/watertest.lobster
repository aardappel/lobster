// 2D water simulation code taken from a game prototype.

import testing
import std
import vec

run_test("water"):

    rnd_seed(0)

    class cell:
        h:float
        wh:float
        nw:float

    let lssize = 100
    let hmargin = 0.2

    var highest = int2_0
    var lowest  = int2_0
    var highh = -1000.0
    var lowh  =  1000.0

    let cells = map(lssize) y:
                map(lssize) x:
                    var h = rnd_float()  // Original had simplex noise, which is expensive.
                    h += 1.0
                    h = pow(h, 3.0)  // Simplest form of erosion: amplify points that are high.
                    h = h * 10.0
                    if h > highh:
                        highh = h
                        highest = int2 { x, y }
                    if h < lowh:
                        lowh = h
                        lowest = int2 { x, y }
                    cell { h, h, 0.0 }

    def loopworld(f):
        for(lssize) y:
            for(lssize) x:
                f(x, y, cells[y][x])

    def ifinworld(pos, f):
        if(pos.x >= 0 and pos.y >= 0 and pos.x < lssize and pos.y < lssize):
            f()

    let neighbours = [ int2_x, int2_y, -int2_x, -int2_y ]

    def forneighboursinworld(x, y, f):
        for(neighbours) n:
            n += int2 { x, y }
            ifinworld(n): f(cells[n.y][n.x])

    let starting_extra_water = 0.0 //10000
    let water_per_second = 4.0
    let give_water_divider = 40.0  // Higher means goes to neighbors more slowly.
    let minimum_water_accumulation = 0.2  // Before it overflows into any neighbors.

    cells[highest].wh += starting_extra_water

    def sim():
        cells[highest].wh += (1.0 / 60.0) * water_per_second
        loopworld() x, y, c:
            if abs(c.wh - c.h) > minimum_water_accumulation:
                var totald = 0.0
                let lower = []
                // Distibute water to neighbors that are lower. We give proportionally more to
                // neighbors we have a bigger height difference with.
                forneighboursinworld(x, y) nc:
                    let d = c.wh - nc.wh
                    if d > hmargin:
                        totald += d
                        lower.push(nc)
                let w = min((c.wh - c.h) / give_water_divider, totald / give_water_divider)
                for(lower) nc:
                    let give = (c.wh - nc.wh) / totald * w
                    nc.nw += give
                    c.nw -= give
        loopworld() x, y, c:
            if c.nw != 0.0:
                c.wh += c.nw
                c.nw = 0.0

    for 5: sim()
