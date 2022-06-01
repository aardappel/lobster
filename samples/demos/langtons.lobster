// This is a demonstration of Langton’s Ant
// https://en.wikipedia.org/wiki/Langton%27s_ant
// Adapted by Kris Kowal <kris@cixar.com>
//
// With one lobster, this is a faithful representation of Langton’s Ant,
// implemented as a cellular automaton instead of a single-agent automaton.
// However, there are additional rules that dictate the behavior of collisions.
// Each lobster has a unique priority.
// Each lobster makes a bid to follow the course set out for them by the
// langton rule number beneath them.
// For each tile, the neighboring lobster with the highest priority that has
// bid to move onto that tile advances.
// All lobsters that fail their bid turn in place instead.

import gui
import texture

// scatter copies the values from the source array into their respective places
// in an array of all the neighborhoods around their position.
// The neighborhoods array must contain an array with the indices of each neighbor.
// So, for an array representing a 3x3 grid in row-major-order:
//   [ 0, 1, 2,
//     3, 4, 5,
//     6, 7, 8 ]
// the neighborhoods of indices in cardinal directions [north, west, south, east]
// would be:
//   [ [ 6, 2, 3, 1 ], [ 7, 0, 4, 2], ... ]
def scatter(dst, src, neighborhoods):
    for(neighborhoods) neighborhood, i:
        for(neighborhood) j, k:
            dst[i][k] = src[j]

// gather_bids visits each cell and determines the direction the lobster in
// that cell would like to move, based on its heading.
// headings are indexes into cardinal_directions, for the direction each
// lobster is currently heading.
def gather_bids(bids, neighborhoods, lobsters, headings):
    for(neighborhoods) neighbors, i:
        bids[i] = if lobsters[i]:
            let heading = headings[i]
            let neighbor = neighbors[heading]
            if not lobsters[neighbor]: neighbor
            else: -1
        else: -1

// The tick method then scatters bids and each lobster’s priority into
// bid_neighborhoods and weight_neighborhoods.

// gather_winners (through find_winner) examines the bids in each neighborhood
// and records the bidder with the highest weight to move onto each cell.
def gather_winners(area, winners, neighborhoods, bid_neighborhoods, weight_neighborhoods):
    for(area) i:
        winners[i] = find_winner(i, neighborhoods[i], bid_neighborhoods[i], weight_neighborhoods[i])

def find_winner(i, neighborhood, bid_neighborhood, weight_neighborhood):
    var best = -1
    var winner = -1
    for(length(neighborhood)) j:
        if bid_neighborhood[j] == i and (best < 0 or weight_neighborhood[j] > best):
            best = weight_neighborhood[j]
            winner = neighborhood[j]
    return winner

// The tick method then builds a transition matrix.
// The transition matrix contains the index of every cell in the source array,
// ordered according to where its data were stored in the previous generation.

// transit applies the results of a transition, effectively swapping every
// cell’s data with that data in its prior location, from the previous
// generation.
// The tick function uses transit to move all mobile data from one generation
// to the next.
def transit(len, dst, src, transitions):
    for(len) i: dst[i] = src[transitions[i]]

// move copies one array to another, given the length of both.
// This is necessary at the beginning of each tick, to overwrite columns of
// data from the previous generation.
def move(len, dst, src):
    for(len) i:
        dst[i] = src[i]

// move with a function additionally applies a relation on the data
// being moved.
// The tick function uses the transform to normalize headings.
// The -1 and 4 headings are otherwise valid for the purposes of animating
// transitions after the 0 and 3 headings.
def move(len, dst, src, fun):
    for(len) i:
        dst[i] = fun(src[i])

def index_of_point(v, size): return v.y * size + v.x

// center_unit_square projects a unit square coordinate system over
// the center of the window, regardless of its aspect ratio.
def center_unit_square():
    let size = float(gl_window_size())
    let scale = xy_1 * min(size)
    gl_translate((size - scale) / 2)
    gl_scale(scale)

// world captures all allocations necessary to run the langton’s lobsters
// cellular automaton.
// The automaton uses columns of data organized by their respective point in
// the world in row-major-order, but is flexible enough to use other orders
// like z-order or hilbert-order with slight adaptation if these were to
// provide better cache locality, and if that were to matter.
//
// The columns on the leading edge are inputs from the prior generation.
// The columns on the following edge are copied from the leading edge at the
// beginning of each tick.
// Otherwise, information flows strictly from input to output.
class world:
    rules:[int]
    size:int
    area:int

    // leading edge
    lobsters_next:[int]
    weights_next:[int]
    headings_next:[int]
    langtons_next:[int]
    // following edge
    lobsters:[int]
    weights:[int]
    headings:[int]
    langtons:[int]

    // intermediate
    points:[xy_i]
    neighborhoods:[[int]]
    bids:[int]
    bid_neighborhoods:[[int]]
    weight_neighborhoods:[[int]]
    winners:[int]
    winner_neighborhoods:[[int]]
    transitions:[int]
    directions:[xy_i]
    rotations:[int]
    headings_rotated:[int]

    def tick():
        move(area, lobsters, lobsters_next)
        move(area, weights, weights_next)
        move(area, langtons, langtons_next)
        move(area, headings, headings_next) n: (4 + n) % 4

        gather_bids(bids, neighborhoods, lobsters, headings)
        scatter(bid_neighborhoods, bids, neighborhoods)
        scatter(weight_neighborhoods, weights, neighborhoods)
        gather_winners(area, winners, neighborhoods, bid_neighborhoods, weight_neighborhoods)
        scatter(winner_neighborhoods, winners, neighborhoods)
        gather_transitions()

        transit(area, lobsters_next, lobsters, transitions)
        transit(area, weights_next, weights, transitions)
        transit(area, headings_next, headings_rotated, transitions)

    def gather_transitions():
        for(area) i:
            let transition, direction, rotation, langton = find_transition(i, neighborhoods[i], winners[i], winner_neighborhoods[i], langtons[i])
            transitions[i] = transition
            directions[i] = direction
            rotations[i] = rotation
            headings_rotated[i] = headings[i] + rotation
            langtons_next[i] = langton

    def find_transition(i, neighbors, winner, winner_neighbors, langton):
        // A neighbor moves here.
        if winner >= 0:
            return winner, xy_0i, 0, langton
        let found = find(winner_neighbors) j: i == j
        // We move to a neighbor.
        if found >= 0:
            return neighbors[found], cardinal_directions[found], rules[langton], (length(rules) + langton + 1) % length(rules)
        // Rotate in place if cannot proceed.
        return i, xy_0i, 1, langton

    def for_lobsters(func):
        for(area) i: if lobsters[i]:
            let v1 = float(points[i])
            let v2 = float(points[i] + directions[i])
            func(i, v1, v2)
            // render lobsters again if they transit the edge of the world
            if v2.x < 0:
                let d = xy_x * size
                func(i, v1+d, v2+d)
            if v2.y < 0:
                let d = xy_y * size
                func(i, v1+d, v2+d)

def new_world(size, count, rules):
    let grid = xy_1i * size
    let points = flatten(mapxy(grid) v: v)
    let area = size * size
    let headings = map(area): rnd(4)

    // // just one lobster:
    // let lobsters_next = map(area): 0
    // lobsters_next[length(area)/2] = 1
    // // random lobsters
    // let lobsters_next = map(area): if rnd(area) <= count: 1 else: 0
    // count lobsters randomly distributed
    count = min(area, count)
    let lobsters_next = map(area): 0
    move(length(count), lobsters_next, map(count): 1)
    shuffle(lobsters_next)

    return world{
        rules: rules,
        size: size,
        area: area,
        points: points,
        neighborhoods: map(points) v: map(cardinal_directions) d: index_of_point((v + d + grid) % grid, size),
        weights_next: shuffle(map(area) n: n),
        weights: map(area): 0,
        lobsters_next: lobsters_next,
        lobsters: map(area): 0,
        bids: map(points): 0,
        bid_neighborhoods: map(points): map(cardinal_directions): 0,
        weight_neighborhoods: map(points): map(cardinal_directions): 0,
        winners: map(points): 0,
        winner_neighborhoods: map(points): map(cardinal_directions): 0,
        transitions: map(points): 0,
        directions: map(points): rnd_pick(cardinal_directions),
        headings: headings,
        headings_next: map(area): headings[_],
        rotations: map(area): 0,
        headings_rotated: map(area): 0,
        langtons: map(area): 0,
        langtons_next: map(area): 0
    }

do():

    let hz = 3
    let size = 9
    let count = 5
    let rules = [1, -1, 0]
    let w = new_world(size, count, rules)
    let first_rule_color = color_dark_grey
    let last_rule_color = color_grey

    w.tick()
    var start = gl_time()
    def next():
        let now = gl_time()
        let tween = (now - start) * hz
        if tween < 1.0:
            return tween
        start = now
        w.tick()
        return 0.0

    fatal(gl_window("Langosta de Langton", 512, 512, 0))

    let tex = gl_load_texture("data/textures/lobster.jpg")
    assert tex

    while gl_frame() and not gl_button("escape"):
        let tween = next()

        gl_clear(first_rule_color)
        center_unit_square()

        // draw background color according to langton rule number.
        gl_set_shader("color")
        for(w.area) i:
            let v = float(w.points[i])
            gl_translate v / w.size:
                // Interpolate the rule number colors between the first and
                // last rule colors.
                // Then interpolate the rule number colors between the previous
                // and next generation.
                let r1 = float(w.langtons[i]) / float(length(w.rules))
                let r2 = float(w.langtons_next[i]) / float(length(w.rules))
                let c1 = lerp(first_rule_color, last_rule_color, r1)
                let c2 = lerp(first_rule_color, last_rule_color, r2)
                let c = lerp(c1, c2, tween)
                gl_color(c)
                gl_scale(xy_1 / w.size)
                gl_rect(xy_1, 0)

        // draw foreground according to presence and heading of lobsters,
        // tweened along their translation and rotation vectors.
        gl_set_shader("textured")
        gl_set_primitive_texture(0, tex)

        // Each lobster, including duplicate lobsters when a lobster exits off
        // an edge and returns through another.
        w.for_lobsters() i, v1, v2:
            // Interpolate position
            let v = lerp(v1, v2, tween)

            // Interpolate heading
            let a1 = float(w.headings[i]) * 90.0
            let a2 = float(w.headings_rotated[i]) * 90.0
            let a = lerp(a1, a2, tween)

            gl_translate v / w.size:
                gl_scale(xy_1 / w.size)
                gl_translate(xy_1 / 2)
                gl_rotate_z(rotate2D(xy_x, a))
                gl_scale(0.75)
                gl_translate(xy_1 / -2)
                gl_rect(xy_1, 0)
