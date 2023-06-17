import testing
import vec

run_test("knightstour"):

    let knight_moves = [ int2 { 1, 2 }, int2 { 1, -2 }, int2 { -1, 2 }, int2 { -1, -2 },
                         int2 { 2, 1 }, int2 { 2, -1 }, int2 { -2, 1 }, int2 { -2, -1 } ]

    def knights_tour(dim):
        let board = mapxy(dim): -1
        var cur = rnd_int2(dim)
        board[cur] = 0
        for(dim.x * dim.y - 1) step:
            var candidates = map(knight_moves) m: cur + m
            def empty(v): return in_range(v, dim) and board[v] < 0
            candidates = filter(candidates) c: empty(c)
            if not candidates.length: return nil
            let degrees = map(candidates) c: (filter(knight_moves) m: empty(c + m)).length
            cur = candidates[find_best<int>(degrees) d: -d]
            board[cur] = step + 1
        return board

    rnd_seed(0)
    assert knights_tour(int2 { 8, 8 })
