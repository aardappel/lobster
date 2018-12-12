include vec

knight_moves :== [ xy { 1, 2 }, xy { 1, -2 }, xy { -1, 2 }, xy { -1, -2 },
                   xy { 2, 1 }, xy { 2, -1 }, xy { -2, 1 }, xy { -2, -1 } ]

def knights_tour(dim):
    board := mapxy(dim): -1
    cur := xy_rndi(dim)
    board[cur] = 0
    for(dim.x * dim.y - 1) step:
        candidates := map(knight_moves) m: cur + m
        def empty(v): return in_range(v, dim) and board[v] < 0
        candidates = filter(candidates) c: empty(c)
        if not candidates.length: return nil
        degrees := map(candidates) c: (filter(knight_moves) m: empty(c + m)).length
        cur = candidates[find_best(degrees) d: -d]
        board[cur] = step + 1
    return board

while true:
    board := knights_tour(xy { 8, 8 })
    if board:
        for(board) row: print row
        return
