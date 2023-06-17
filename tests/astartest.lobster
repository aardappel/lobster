import testing
import astar

run_test("astar"):

    let initworld = [
        ".................................",
        "................S................",
        "........#..............#.........",
        "........#..............#/////////",
        "........#..............#.........",
        "........#..............#.........",
        "........################.........",
        ".................................",
        "...............///...............",
        ".............../E/...............",
        "...............///...............",
        "................................."
    ]

    let expected_result = [
        "......OXXXXXXXXXXXXXXXXXXXO......",
        ".....OXPPPPPPPPPPXXXXXXXXXXO.....",
        ".....OXP#XXXXXXXXXXXXXX#XXXO.....",
        ".....OXP#XXXXXXXXXXXXXX#XOO//////",
        ".....OXP#XXXXXXXXXXXXXX#XO.......",
        ".....OXP#XXXXXXXXXXXXXX#XO.......",
        ".....OXP################XO.......",
        ".....OXPPPPPPPPPPXXXXXXXXO.......",
        ".....OXXXXXXXXXOPOXXXXXXXO.......",
        ".....OXXXXXXXXXOPOXXXXXXXO.......",
        "......OXXXXXXXXO//OOOOOOO........",
        ".......OOOOOOOO.................."
    ]

    class pathingcell : astar_node<pathingcell?, int2, int2>
        path:int
        c:int

    let worldsize = int2 { initworld[0].length, initworld.length }

    var startpos = int2_0
    var endpos = int2_0

    let world = worldsize.y.map() y: worldsize.x.map() x:
        let c = initworld[y][x]
        if c == 'S': startpos = int2 { x, y }
        if c == 'E': endpos   = int2 { x, y }
        pathingcell { int2 { x, y }, int2_0, false, c }

    let path =
        astar_2dgrid(false, worldsize, world[startpos], world[endpos], fn: world[_]) (n, nn::pathingcell):
            (c == '#' and -1) or (c == '/' and 5) or 1

    assert path.length == 27

    for(path) n:
        n.path = true

    let astar_result = map(world) row: fold(row, "") accum, n:
        accum + ((n.path   and "P") or
                 (n.closed and "X") or
                 (n.open   and "O") or
                 unicode_to_string([ n.c ]))

    assert equal(astar_result, expected_result)
