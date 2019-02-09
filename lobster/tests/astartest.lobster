include testing
include astar

run_test("astar"):

    initworld := [
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

    expected_result := [
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

    struct pathingcell : astar_node<pathingcell?, xy_i, xy_i?>:
        path:int
        c:int

    worldsize := xy { initworld[0].length, initworld.length }

    startpos := xy_0i
    endpos := xy_0i

    world := worldsize.y.map() y: worldsize.x.map() x:
        c := initworld[y][x]
        if c == 'S': startpos = xy { x, y }
        if c == 'E': endpos   = xy { x, y }
        pathingcell { xy { x, y }, false, c }

    path :=
        astar_2dgrid(false, worldsize, world[startpos], world[endpos], def(): world[_]) (n, nn::pathingcell):
            (c == '#' and -1) or (c == '/' and 5) or 1

    assert path.length == 27

    for(path) n:
        n.path = true

    astar_result := map(world) row: fold(row, "") accum, n:
        accum + ((n.path   and "P") or
                 (n.closed and "X") or
                 (n.open   and "O") or
                 unicode_to_string([ n.c ]))

    assert equal(astar_result, expected_result)
