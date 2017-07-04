// A* search functionality

include "std.lobster"
include "vec.lobster"

struct astar_node {
    G:float = 0.0,
    H:float = 0.0,
    F:float = 0.0,
    previous = nil,
    state,
    delta = nil,
    open:int = false,
    closed:int = false
}

def astar_clear(n::astar_node):
    open = closed = false
    previous = nil

// the generic version searches any kind of graph in any kind of search space,
// use specialized versions below

def astar_generic(startnode, endcondition, generatenewstates, heuristic):
    openlist := [ startnode ]
    n := startnode or nil
    while n and !endcondition(n):
        openlist.removeobj(n)
        n.closed = true
        generatenewstates(n) delta, cost, nn:
            if !nn.closed:
                G := n.G + cost
                if ((!nn.open and openlist.push(nn)) or G < nn.G):
                    nn.open = true
                    nn.delta = delta
                    nn.previous = n
                    nn.H = heuristic(nn.state)
                    nn.G = G
                    nn.F = G + nn.H
        n = nil
        for(openlist) c:
            if !n or c.F < n.F or (c.F == n.F and c.H < n.H):
                n = c
    path := []
    while n:
        path.push(n)
        n = n.previous
    path

// specialized to a graph in 2D or 3D space (assumes pre existing nodes), usage:
// - create a graph out of nodes inherited from astar_node above
// - costf must compute the cost of traversal between 2 nodes, 0 for impassable
// - neighbors generates adjacent nodes
// - returns a list of nodes from end to start inclusive, or empty list if no path

def astar_graph(startnode, endnode, costf, distancef, neighbors):
    astar_generic(startnode) n:
        n == endnode
    generatenewstates n, f:
        neighbors(n) nn:
            cost := costf(n, nn)
            if cost > 0:
                f(nn.state - n.state, cost, nn)
    heuristic state:
        distancef(state - endnode.state)

// specialized to a 2D grid (specialized case of a graph)

def astar_2dgrid(isocta, gridsize, startnode, endnode, getnode, costf):
    directions := cardinal_directions
    def astar_distance(distancef):
        astar_graph(startnode, endnode, costf, distancef) n, f:
            for(directions) delta:
                np := n.state + delta
                if np.inrange(gridsize):
                    f(getnode(np))
    if isocta:
        directions = append(directions, diagonal_directions)
        astar_distance() v:
            x := abs(v.x)
            y := abs(v.y)
            big := max(x, y)
            small := min(x, y)
            sqrt(2) * small + big - small
    else:
        astar_distance() v:
            abs(v.x) + abs(v.y)

// specialized to do GOAP (nodes created on the fly)

def goapf(state:[int]) :== int

value goapaction {
    name:string,
    precondition:goapf,
    effect:goapf
}

struct goap_node = astar_node(goap_node?, [int], goapaction?)

def astar_goap(goapactions:[goapaction], initstate:[int], heuristic, endcondition):
    H := heuristic(initstate)
    existingnodes := [ goap_node { H: H, F: H, state: initstate } ]
    astar_generic(existingnodes[0]):
        endcondition(_.state)
    generatenewstates n, f:
        for(goapactions) act:
            pre := act.precondition
            if pre(n.state):
                nstate := n.state.copy
                eff := act.effect
                eff(nstate)
                i := existingnodes.find(): equal(_.state, nstate)
                nn := i >= 0 and existingnodes[i] or goap_node { nstate }
                f(act, 1, nn)
    heuristic: heuristic(_)
