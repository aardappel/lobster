// The 32-bit LCG every language in tests/compare shares, see ../SPEC.md. The largest
// intermediate stays under 2^53 so that JavaScript can evaluate it exactly too.

class Rnd:
    state:int

    def next() -> int:
        state = (state * 1664525 + 1013904223) & 0xFFFFFFFF
        return state

    def nextf() -> float:
        return next() * (1.0 / 4294967296.0)

    def range(a:float, b:float) -> float:
        return a + nextf() * (b - a)

    def below(n:int) -> int:
        return next() % n
