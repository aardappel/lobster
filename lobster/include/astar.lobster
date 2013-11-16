// A* search functionality

include "std.lobster"
include "vec.lobster"

struct astar_node: [ G, H, F, previous, state, delta, open, closed ]

function new_astar_node(state, h:float):
    [ 0.0, h, h, nil, state, nil, false, false ]:astar_node

function clear(n::astar_node):
    open = closed = false
    previous = nil
    
// the generic version searches any kind of graph in any kind of search space, use specialized versions below

function astar_generic(n, endcondition, generatenewstates, heuristic):
    openlist := [ n ]
    while(n & !endcondition(n)):
        openlist.removeobj(n)
        n.closed = true
        generatenewstates(n) delta, cost, nn:
            if(!nn.closed):
                G := n.G + cost
                if((!nn.open & openlist.push(nn)) | G < nn.G):
                    nn.open = true
                    nn.delta = delta
                    nn.previous = n
                    nn.H = heuristic(nn.state)
                    nn.G = G
                    nn.F = G + nn.H
        n = nil
        for(openlist) c:
            if(!n | c.F < n.F | (c.F == n.F & c.H < n.H)):
                n = c
    collectwhile(n):
        return_after(n):
            n = n.previous

// specialized to a graph in 2D or 3D space (assumes pre existing nodes), usage:
// - create a graph out of nodes inherited from astar_node above
// - costf must compute the cost of traversal between 2 nodes
// - neighbors generates adjacent nodes
// - returns a list of nodes from end to start inclusive, or empty list if no path
 
function astar_graph(startnode, endnode, costf, distancef, neighbors):
    astar_generic(startnode) n:
        n == endnode
    generatenewstates n, f:
        neighbors(n) nn:
            cost := costf(n, nn)
            if(cost > 0):
                f(nn.state - n.state, cost, nn)
    heuristic state:
        distancef(state - endnode.state)

function astar_space(startnode, endnode, costf, neighbors):
    astar_graph(startnode, endnode, costf, function(v): magnitude(v), neighbors)
        
// specialized to a 2D grid (specialized case of a graph)
            
function astar_2dgrid(isocta, gridsize, startnode, endnode, getnode, costf):
    directions := [ [ -1, 0 ]:xy, [ 1, 0 ]:xy, [ 0, -1 ]:xy, [ 0, 1 ]:xy ]
    distancef := function(v): abs(v.x) + abs(v.y)
    if(isocta):
        directions = directions.append([ [ -1, -1 ]:xy, [ 1, 1 ]:xy, [ 1, -1 ]:xy, [ -1, 1 ]:xy ])
        distancef = function(v):
            big := max(v.x, v.y)
            small := min(v.x, v.y)
            sqrt(2) * small + big - small
    astar_graph(startnode, endnode, costf, distancef) n, f:
        for(directions) delta:
            np := n.state + delta
            if(np.inrange2d(gridsize, vec_0)):
                f(getnode(np))

// specialized to do GOAP (nodes created on the fly)
        
value goapaction: [ name, precondition, effect ]
        
function astar_goap(goapactions, initstate, heuristic, endcondition):
    existingnodes := [ new_astar_node(initstate, heuristic(initstate)) ]
    astar_generic(existingnodes[0]):
        endcondition(_.state)
    generatenewstates n, f:
        goapactions.for() act:
            if(act.precondition(n.state)):
                nstate := n.state.copy
                act.effect(nstate)
                nn := (existingnodes.exists(): equal(_.state, nstate) & _) | new_astar_node(nstate, 0.0)
                f(act, 1, nn)
    heuristic: heuristic(_)
