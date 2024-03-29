// A* search functionality

import vec

class astar_node<N,S,D>:
    private G = 0.0
    private H = 0.0
    private F = 0.0
    previous:N = nil
    state:S
    delta:D
    open = false
    closed = false

    def astar_clear():
        open = closed = false
        previous = nil

// the generic version searches any kind of graph in any kind of search space,
// use specialized versions below

def astar_generic(startnode, endcondition, generatenewstates, heuristic):
    let openlist = [ startnode ]
    var n = startnode or nil
    while n and not endcondition(n):
        openlist.remove_obj(n)
        n.closed = true
        generatenewstates(n) delta, cost, nn:
            guard not nn.closed
            let G = n.G + cost
            if (not nn.open and openlist.push(nn)) or G < nn.G:
                nn.open = true
                nn.delta = delta
                nn.previous = n
                nn.H = heuristic(nn.state)
                nn.G = G
                nn.F = G + nn.H
        n = nil
        for(openlist) c:
            if not n or c.F < n.F or (c.F == n.F and c.H < n.H):
                n = c
    let path = []
    while n:
        path.push(n)
        n = n.previous
    return path

// specialized to a graph in 2D or 3D space (assumes pre existing nodes), usage:
// - create a graph out of nodes inherited from astar_node above
// - costf must compute the cost of traversal between 2 nodes, 0 for impassable
// - neighbors generates adjacent nodes
// - returns a list of nodes from end to start inclusive, or empty list if no path

def astar_graph(startnode, endnode, costf, distancef, neighbors):
    return astar_generic(startnode) n:
        n == endnode
    fn n, f:
        neighbors(n) nn:
            let cost = costf(n, nn)
            if cost > 0.0:
                f(nn.state - n.state, cost, nn)
    fn state:
        distancef(state - endnode.state)

// specialized to a 2D grid (specialized case of a graph)

def astar_2dgrid(isocta, gridsize, startnode, endnode, getnode, costf):
    var directions = cardinal_directions
    def astar_distance(distancef):
        return astar_graph(startnode, endnode, costf, distancef) n, f:
            for(directions) delta:
                let np = n.state + delta
                if np.in_range(gridsize):
                    f(getnode(np))
    return if isocta:
        directions = append(directions, diagonal_directions)
        astar_distance() v:
            let x = abs(v.x)
            let y = abs(v.y)
            let big = max(x, y)
            let small = min(x, y)
            sqrt(2.0) * small + big - small
    else:
        astar_distance(): manhattan(_)

// specialized to do GOAP (nodes created on the fly)

abstract class goapaction:
    def precondition(state):
        return false
    def effect(state):
        pass()

class goap_node<T> : astar_node<goap_node<T>?, T, goapaction?>

def astar_goap<T>(goapactions:[goapaction], initstate:T, heuristic, endcondition):
    let H = heuristic(initstate)
    let existingnodes = [ goap_node<T> { H: H, F: H, state: initstate, delta: nil } ]
    return astar_generic(existingnodes[0]):
        endcondition(_.state)
    fn n, f:
        for(goapactions) act:
            guard act.precondition(n.state)
            let nstate = n.state.copy
            act.effect(nstate)
            var i = existingnodes.find(): equal(_.state, nstate)
            if i < 0:
                i = existingnodes.length
                existingnodes.push(goap_node<T> { nstate, nil })
            f(act, 1, existingnodes[i])
    fn: heuristic(_)
