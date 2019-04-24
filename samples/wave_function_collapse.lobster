// Example of using Wave Function Collapse to generate gameworlds based on tiles.
// Using ascii chars here for simplicity.

// Using """ string literals so we don't have to escape \ :(

let tilemap = [
    """   /--\    """,
    """   |  |    """,
    """   |  |    """,
    """/--J  L--\ """,
    """|        | """,
    """|        | """,
    """L--\  /--J """,
    """   |  |    """,
    """   |  |    """,
    """   L--J    """,
    """           """,
]

let benchmark = false

if benchmark:
    var no_conflicts = 0
    for(1000) i:
        let outmap, conflicts = wave_function_collapse(tilemap, xy { 100, 100 })
        print i + ": " + conflicts
        if not conflicts: no_conflicts++
    print no_conflicts
    print seconds_elapsed()

else:
    // Just print a single no-conflict example.
    for(100) i:
        let outmap, conflicts = wave_function_collapse(tilemap, xy { 100, 50 })
        if not conflicts:
            print "iteration: " + i
            for(outmap) s: print s
            return from program

/*

prints:

           |      |  /-\ /-------J  |    |  L-J                                      /--J
           |   /--J  | | |          |    |                                           |
           |   |     | L-J          |    |                                    /-\    |
-----------J /-J     |              |    |     /------------\                 | |    |             /
             |       L-\       /----J    |     |            |                 | |    |             |
             |         |       |      /--J     |            |                 | L----J             |
             |         |       L-\    |        |            |                 |                    |
----\        |         L-\       |    | /-\  /-J            |                 |                    L
    |        |           |       |    | | |  |              L-------------\   |      /---\
    |        |           | /-----J    L-J L--J                            |   L-\    |   |
    |        |           | |                                              |     |    |   |
    |        |           | |                              /-------\       |     |    |   |
    |        |           | |                              |       |       |     L----J   |
    |        | /---------J L-----\                        |   /---J       |              |
  /-J        | |                 |                  /-----J   |         /-J         /----J
  |          | |                 |         /----\   |         |         |           |
  |          | |      /-\   /----J         |    |   |         |         |           |
  |          | |      | |   |              |    |   |         |         |    /------J
  |          | |      | | /-J  /--\  /-----J    |   |         |         |    |
  |          L-J      | | |    |  |  |          |   |         |      /--J    |
  L-----\             | | |    |  L--J          |   L---\     |      |       |    /--\      /----\
        |   /---\     | | |    |                |       |     L------J       |    |  |      |    |
        |   |   |     | | |    |                |       |                    |    |  |      |    |
--\     |   |   L-----J | |    |                |       |                    |    |  |      |    L--
  |     |   |           | L--\ |       /--------J       |                    |    |  |   /--J
--J  /--J   |      /-\  |    | |       |                |                 /--J    |  |   |    /-----
     |      L---\  | |  |    | |       |                |                 |       |  L---J    |
     |          |  | |  |    | |       |                |      /----\     |       |           |
     |          L--J |  |    | |       |       /-\      |      |    |     |       |  /----\   L----\
     |               |  |    L-J       |       | |      |      |    |     |       |  |    |        |
     L------------\  |  |              L-------J |      |      |    |     |       |  |    |        |
                  |  |  |                        |      |      |    |     |       |  |    L--\     |
    /-------------J  |  |          /---\         |      L------J    |     |       |  |       |     |
    |              /-J  |          |   |     /---J                  |     |       |  |       |     |
    |              |    L------\   |   L---\ |                      |     |       |  L--\ /--J     |
    |         /-\  L-\         |   |       | |                      |     L-----\ |     | |        |
    |         | |    |         |   |       | |         /----\       | /-\       | L--\  | |        |
--\ |         | |    |         L---J       | L--\      |    |       | | |       |    |  | |        L
  | |         | |    |                     |    |      |  /-J       | L-J       |    |  | |
  L-J         L-J    L---------\           |    |      |  |         |        /--J    |  | |
                               |           L-\  |      |  |         |        |       |  | L---\
----\                /---\     |             |  L------J  |         |        |       |  |     |  /--
    |                |   |     |             |            |         |        L--\    |  |     |  |
    |                L---J     |         /---J            |         |           |    L--J     |  |
    |                          | /--\    |     /--\       |         |           |             |  |
    L------\             /-----J |  |    |     |  |       |         |        /--J             |  |
           |      /--\   |       |  |    |     L--J       |         |        |                |  |
           |      |  |   |       |  |    |  /-\           L-------\ |        L----------\     |  |
           |      |  L---J       |  |    |  | |                   | |                   |     L--J
           |      |              |  |    |  | |                   L-J                   |

*/
