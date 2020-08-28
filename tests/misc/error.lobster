def rec(n) -> int:
    if n:
        rec(n - 1)
    else:
        pop([])

rec(3)
