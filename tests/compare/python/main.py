"""Python implementation of the cross-language benchmarks, see ../SPEC.md.

    python main.py [repeats] [name]

Vectors are unpacked into local floats rather than wrapped in a class. A class with an
operator method per component would be several times slower again, and is not what someone
who cared about the speed of pure Python would write; anyone who cared more than that would
reach for numpy, which would no longer be a comparison of language implementations.
"""

import sys
import time
from math import floor, sqrt

# ------------------------------------------------------------------ shared


class Rnd:
    """The 32-bit LCG from SPEC.md."""

    __slots__ = ("state",)

    def __init__(self, seed):
        self.state = seed

    def next(self):
        self.state = (self.state * 1664525 + 1013904223) & 0xFFFFFFFF
        return self.state

    def nextf(self):
        return self.next() * (1.0 / 4294967296.0)

    def range(self, a, b):
        return a + self.nextf() * (b - a)

    def below(self, n):
        return self.next() % n


# ------------------------------------------------------------------ 1. nbody


def bench_nbody():
    N = 256
    STEPS = 1000
    DT = 0.001
    EPS2 = 0.5
    rnd = Rnd(12345)
    px = [0.0] * N
    py = [0.0] * N
    pz = [0.0] * N
    vx = [0.0] * N
    vy = [0.0] * N
    vz = [0.0] * N
    mass = [0.0] * N
    for i in range(N):
        px[i] = rnd.range(-40.0, 40.0)
        py[i] = rnd.range(-40.0, 40.0)
        pz[i] = rnd.range(-40.0, 40.0)
        vx[i] = rnd.range(-0.5, 0.5)
        vy[i] = rnd.range(-0.5, 0.5)
        vz[i] = rnd.range(-0.5, 0.5)
        mass[i] = rnd.range(0.2, 1.2)
    ax = [0.0] * N
    ay = [0.0] * N
    az = [0.0] * N
    rng = range(N)
    for _ in range(STEPS):
        for i in rng:
            pix = px[i]
            piy = py[i]
            piz = pz[i]
            sx = 0.0
            sy = 0.0
            sz = 0.0
            for j in rng:
                dx = px[j] - pix
                dy = py[j] - piy
                dz = pz[j] - piz
                r2 = dx * dx + dy * dy + dz * dz + EPS2
                # The j == i term has d == 0, so it contributes exactly zero.
                f = mass[j] / (r2 * sqrt(r2))
                sx += dx * f
                sy += dy * f
                sz += dz * f
            ax[i] = sx
            ay[i] = sy
            az[i] = sz
        for i in rng:
            nvx = vx[i] + ax[i] * DT
            nvy = vy[i] + ay[i] * DT
            nvz = vz[i] + az[i] * DT
            vx[i] = nvx
            vy[i] = nvy
            vz[i] = nvz
            px[i] += nvx * DT
            py[i] += nvy * DT
            pz[i] += nvz * DT
    e = 0.0
    for i in rng:
        e += 0.5 * mass[i] * (vx[i] * vx[i] + vy[i] * vy[i] + vz[i] * vz[i])
    for i in rng:
        for j in rng:
            if j > i:
                dx = px[j] - px[i]
                dy = py[j] - py[i]
                dz = pz[j] - pz[i]
                e -= mass[i] * mass[j] / sqrt(dx * dx + dy * dy + dz * dz + EPS2)
    return e


# ------------------------------------------------------------------ 2. raytrace

RT_W = 640
RT_H = 480
NSPHERES = 48
MAX_DEPTH = 3
EPS = 0.0001
# tan(27.5 degrees).
HALF = 0.5205670505517462
AMBIENT = 0.12


def normalize(x, y, z):
    l = sqrt(x * x + y * y + z * z)
    if l == 0.0:
        return (0.0, 0.0, 0.0)
    return (x / l, y / l, z / l)


def hit_sphere(sc, sr, rox, roy, roz, rdx, rdy, rdz):
    ocx = sc[0] - rox
    ocy = sc[1] - roy
    ocz = sc[2] - roz
    b = ocx * rdx + ocy * rdy + ocz * rdz
    c = ocx * ocx + ocy * ocy + ocz * ocz - sr * sr
    disc = b * b - c
    if not disc > 0.0:
        return -1.0
    sd = sqrt(disc)
    t = b - sd
    if t > EPS:
        return t
    t2 = b + sd
    return t2 if t2 > EPS else -1.0


def any_hit(spheres, rox, roy, roz, rdx, rdy, rdz):
    for s in spheres:
        if hit_sphere(s[0], s[1], rox, roy, roz, rdx, rdy, rdz) > 0.0:
            return True
    return False


def trace(spheres, light, rox, roy, roz, rdx, rdy, rdz, depth):
    best_t = -1.0
    best_n = None
    best_albedo = None
    best_refl = 0.0
    for s in spheres:
        t = hit_sphere(s[0], s[1], rox, roy, roz, rdx, rdy, rdz)
        if t > 0.0 and (best_t < 0.0 or t < best_t):
            sc = s[0]
            best_t = t
            best_n = normalize(
                rox + rdx * t - sc[0], roy + rdy * t - sc[1], roz + rdz * t - sc[2]
            )
            best_albedo = s[2]
            best_refl = s[3]
    if rdy < -1.0e-9:
        t = -roy / rdy
        if t > EPS and (best_t < 0.0 or t < best_t):
            hx = rox + rdx * t
            hz = roz + rdz * t
            shade = 0.9 if (int(floor(hx)) + int(floor(hz))) % 2 == 0 else 0.35
            best_t = t
            best_n = (0.0, 1.0, 0.0)
            best_albedo = (shade, shade, shade)
            best_refl = 0.25
    if not best_t > 0.0:
        sk = rdy if rdy > 0.0 else 0.0
        f = 0.4 + 0.6 * sk
        return (0.35 * f, 0.52 * f, 0.85 * f)
    hx = rox + rdx * best_t
    hy = roy + rdy * best_t
    hz = roz + rdz * best_t
    nx, ny, nz = best_n
    ox = hx + nx * EPS
    oy = hy + ny * EPS
    oz = hz + nz * EPS
    lit = 0.0 if any_hit(spheres, ox, oy, oz, light[0], light[1], light[2]) else 1.0
    nl = nx * light[0] + ny * light[1] + nz * light[2]
    if nl < 0.0:
        nl = 0.0
    k = AMBIENT + nl * lit
    sr = best_albedo[0] * k
    sg = best_albedo[1] * k
    sb = best_albedo[2] * k
    if not (best_refl > 0.0 and depth > 0):
        return (sr, sg, sb)
    d2 = 2.0 * (nx * rdx + ny * rdy + nz * rdz)
    rr, rg, rb = trace(
        spheres, light, ox, oy, oz, rdx - nx * d2, rdy - ny * d2, rdz - nz * d2, depth - 1
    )
    inv = 1.0 - best_refl
    return (sr * inv + rr * best_refl, sg * inv + rg * best_refl, sb * inv + rb * best_refl)


def bench_raytrace():
    rnd = Rnd(98765)
    spheres = []
    for i in range(NSPHERES):
        c = (rnd.range(-6.0, 6.0), rnd.range(0.4, 4.5), rnd.range(-6.0, 6.0))
        r = rnd.range(0.35, 0.9)
        col = (rnd.range(0.2, 1.0), rnd.range(0.2, 1.0), rnd.range(0.2, 1.0))
        spheres.append((c, r, col, 0.55 if i % 3 == 0 else 0.0))
    light = normalize(-0.6, 1.0, 0.45)
    eye = (0.0, 2.6, 9.0)
    fwd = normalize(0.0 - eye[0], 1.2 - eye[1], 0.0 - eye[2])
    # cross(fwd, up) with up = (0, 1, 0).
    right = normalize(
        fwd[1] * 0.0 - fwd[2] * 1.0, fwd[2] * 0.0 - fwd[0] * 0.0, fwd[0] * 1.0 - fwd[1] * 0.0
    )
    cam_up = (
        right[1] * fwd[2] - right[2] * fwd[1],
        right[2] * fwd[0] - right[0] * fwd[2],
        right[0] * fwd[1] - right[1] * fwd[0],
    )
    aspect = RT_W / RT_H
    total = 0.0
    for pyi in range(RT_H):
        v = 1.0 - (pyi + 0.5) / RT_H * 2.0
        cv = v * HALF
        for pxi in range(RT_W):
            u = (pxi + 0.5) / RT_W * 2.0 - 1.0
            cu = u * aspect * HALF
            rd = normalize(
                fwd[0] + right[0] * cu + cam_up[0] * cv,
                fwd[1] + right[1] * cu + cam_up[1] * cv,
                fwd[2] + right[2] * cu + cam_up[2] * cv,
            )
            cr, cg, cb = trace(
                spheres, light, eye[0], eye[1], eye[2], rd[0], rd[1], rd[2], MAX_DEPTH
            )
            total += cr + cg + cb
    return total


# ------------------------------------------------------------------ 3. interp

IP_DEPTH = 14
IP_ROUNDS = 1500
NVARS = 8


class Expr:
    __slots__ = ()


class ConstE(Expr):
    __slots__ = ("v",)

    def __init__(self, v):
        self.v = v

    def eval(self, env):
        return self.v


class VarE(Expr):
    __slots__ = ("i",)

    def __init__(self, i):
        self.i = i

    def eval(self, env):
        return env[self.i]


class NegE(Expr):
    __slots__ = ("a",)

    def __init__(self, a):
        self.a = a

    def eval(self, env):
        return -self.a.eval(env)


class AddE(Expr):
    __slots__ = ("a", "b")

    def __init__(self, a, b):
        self.a = a
        self.b = b

    def eval(self, env):
        return self.a.eval(env) + self.b.eval(env)


class SubE(Expr):
    __slots__ = ("a", "b")

    def __init__(self, a, b):
        self.a = a
        self.b = b

    def eval(self, env):
        return self.a.eval(env) - self.b.eval(env)


class MulE(Expr):
    __slots__ = ("a", "b")

    def __init__(self, a, b):
        self.a = a
        self.b = b

    def eval(self, env):
        return self.a.eval(env) * self.b.eval(env)


def build(rnd, d):
    if d == 0:
        k = rnd.below(3)
        if k == 0:
            return ConstE(rnd.range(-1.0, 1.0))
        if k == 1:
            return VarE(rnd.below(NVARS))
        return NegE(build(rnd, 0))
    # Multiplication only near the leaves, so the value of the tree stays finite.
    k = rnd.below(3 if d <= 3 else 2)
    a = build(rnd, d - 1)
    b = build(rnd, d - 1)
    if k == 0:
        return AddE(a, b)
    if k == 1:
        return SubE(a, b)
    return MulE(a, b)


def bench_interp():
    rnd = Rnd(4242)
    tree = build(rnd, IP_DEPTH)
    env = [0.0] * NVARS
    total = 0.0
    for r in range(IP_ROUNDS):
        for k in range(NVARS):
            env[k] = ((r * 7 + k * 13) % 101) / 50.0 - 1.0
        total += tree.eval(env)
    return total


# ------------------------------------------------------------------ 4. sort

SORT_N = 400000
SORT_ROUNDS = 6


def sort_rec(a, lo, hi, less):
    """The comparison arrives as a callable, which is what Python code taking one looks like."""
    if hi - lo <= 1:
        return
    pivot = a[lo]
    sp = lo + 1
    ep = hi
    while sp < ep:
        c = a[sp]
        if less(c, pivot):
            a[sp - 1] = a[sp]
            sp += 1
        else:
            ep -= 1
            a[sp] = a[ep]
            a[ep] = c
    sp -= 1
    a[sp] = pivot
    sort_rec(a, lo, sp, less)
    sort_rec(a, ep, hi, less)


def bench_sort():
    rnd = Rnd(777)
    src = []
    for i in range(SORT_N):
        src.append((rnd.range(-1000.0, 1000.0), i))

    def less(a, b):
        return a[0] < b[0] or (a[0] == b[0] and a[1] < b[1])

    last = src
    for _ in range(SORT_ROUNDS):
        xs = list(src)
        sort_rec(xs, 0, len(xs), less)
        last = xs
    for i in range(1, len(last)):
        if less(last[i], last[i - 1]):
            print("# sort: not sorted at %d" % i)
            return 0.0
    total = 0.0
    for i in range(len(last)):
        total += (i % 97 + 1) * last[i][1]
    return total


# ------------------------------------------------------------------ 5. astar

AS_W = 512
AS_H = 512
AS_ROUNDS = 20
WALL_P = 0.20
INF = 0x3FFFFFFF


def before(af, an, bf, bn):
    return af < bf or (af == bf and an < bn)


def solve(seed):
    rnd = Rnd(seed)
    cells = AS_W * AS_H
    grid = [0] * cells
    for i in range(cells):
        if rnd.nextf() < WALL_P:
            grid[i] = 1
    goal = cells - 1
    grid[0] = 0
    grid[1] = 0
    grid[AS_W] = 0
    grid[goal] = 0
    grid[goal - 1] = 0
    grid[goal - AS_W] = 0
    gscore = [INF] * cells
    closed = [0] * cells
    # The heap holds f and node in two parallel lists.
    hf = []
    hn = []
    dx = (1, 0, -1, 0)
    dy = (0, 1, 0, -1)
    gx = AS_W - 1
    gy = AS_H - 1
    gscore[0] = 0

    def push(ef, en):
        hf.append(ef)
        hn.append(en)
        i = len(hf) - 1
        while i > 0:
            p = (i - 1) >> 1
            if not before(ef, en, hf[p], hn[p]):
                break
            hf[i] = hf[p]
            hn[i] = hn[p]
            i = p
        hf[i] = ef
        hn[i] = en

    def pop():
        top_n = hn[0]
        last_f = hf.pop()
        last_n = hn.pop()
        size = len(hf)
        if size > 0:
            i = 0
            while True:
                l = (i << 1) + 1
                if l >= size:
                    break
                c = l
                r = l + 1
                if r < size and before(hf[r], hn[r], hf[l], hn[l]):
                    c = r
                if not before(hf[c], hn[c], last_f, last_n):
                    break
                hf[i] = hf[c]
                hn[i] = hn[c]
                i = c
            hf[i] = last_f
            hn[i] = last_n
        return top_n

    push(gx + gy, 0)
    expanded = 0
    cost = -1
    while hf:
        n = pop()
        if closed[n] == 0:
            closed[n] = 1
            expanded += 1
            if n == goal:
                cost = gscore[n]
                break
            y = n // AS_W
            x = n - y * AS_W
            g = gscore[n] + 1
            for k in range(4):
                nx = x + dx[k]
                ny = y + dy[k]
                if 0 <= nx < AS_W and 0 <= ny < AS_H:
                    nb = n + dy[k] * AS_W + dx[k]
                    if grid[nb] == 0 and closed[nb] == 0 and g < gscore[nb]:
                        gscore[nb] = g
                        push(g + (gx - nx) + (gy - ny), nb)
    return cost, expanded


def bench_astar():
    total = 0
    for r in range(AS_ROUNDS):
        cost, expanded = solve(1000 + r)
        total += cost * 1000003 + expanded
    return float(total)


# ------------------------------------------------------------------ driver

BENCHES = [
    ("nbody", bench_nbody),
    ("raytrace", bench_raytrace),
    ("interp", bench_interp),
    ("sort", bench_sort),
    ("astar", bench_astar),
]


def main():
    sys.setrecursionlimit(100000)
    args = [a for a in sys.argv[1:] if a != "--"]
    repeats = 5
    if args:
        try:
            n = int(args[0])
            if n > 0:
                repeats = n
        except ValueError:
            pass
    only = args[1] if len(args) > 1 else ""
    out = []
    for name, fn in BENCHES:
        if only and only != name:
            continue
        best = float("inf")
        checksum = 0.0
        for _ in range(repeats):
            t0 = time.perf_counter()
            checksum = fn()
            dt = time.perf_counter() - t0
            if dt < best:
                best = dt
        out.append("bench\t%s\t%.7f\t%r" % (name, best, checksum))
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
