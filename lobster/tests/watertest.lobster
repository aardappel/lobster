// 2D water simulation code taken from a game prototype.

include testing
include std
include vec

run_test("water"):

    rnd_seed(0)

    struct cell { h:float, wh:float, nw:float }

    lssize := 100
    hmargin := 0.2

    highest := xy_0i
    lowest  := xy_0i
    highh := -1000.0
    lowh  :=  1000.0

    cells := map(lssize) y:
                map(lssize) x:
                    h := rnd_float()  // Original had simplex noise, which is expensive.
                    h += 1
                    h = pow(h, 3)  // Simplest form of erosion: amplify points that are high.
                    h = h * 10
                    if h > highh:
                        highh = h
                        highest = xy { x, y }
                    if h < lowh:
                        lowh = h
                        lowest = xy { x, y }
                    cell { h, h, 0.0 }

    def loopworld(f):
        for(lssize) y:
            for(lssize) x:
                f(x, y, cells[y][x])

    def ifinworld(pos, f):
        if(pos.x >= 0 and pos.y >= 0 and pos.x < lssize and pos.y < lssize):
            f()

    neighbours := [ xy_xi, xy_yi, -xy_xi, -xy_yi ]

    def forneighboursinworld(x, y, f):
        for(neighbours) n, i:
            n += xy { x, y }
            ifinworld(n): f(cells[n.y][n.x])

    starting_extra_water :== 0 //10000
    water_per_second :== 4
    give_water_divider :== 40  // Higher means goes to neighbors more slowly.
    minimum_water_accumulation :== 0.2  // Before it overflows into any neighbors.

    cells[highest].wh += starting_extra_water

    def sim():
        cells[highest].wh += gl_delta_time() * water_per_second
        loopworld() x, y, c:
            if abs(c.wh - c.h) > minimum_water_accumulation:
                totald := 0.0
                lower := []
                // Distibute water to neighbors that are lower. We give proportionally more to
                // neighbors we have a bigger height difference with.
                forneighboursinworld(x, y) nc:
                    d := c.wh - nc.wh
                    if d > hmargin:
                        totald += d
                        lower.push(nc)
                w := min((c.wh - c.h) / give_water_divider, totald / give_water_divider)
                for(lower) nc:
                    give := (c.wh - nc.wh) / totald * w
                    nc.nw += give
                    c.nw -= give
        loopworld() x, y, c:
            if c.nw != 0:
                c.wh += c.nw
                c.nw = 0

    for 5: sim()
