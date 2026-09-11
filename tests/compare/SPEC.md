# Cross-language benchmark specification

Five benchmarks, implemented once per language, in one program per language that runs all
five in-process and reports its own timings. Every implementation must produce the same
checksums; the harness enforces that.

The benchmarks were picked to sit in areas a game-oriented language is expected to be used
for, and to each stress a different part of a language implementation. They are also the
five categories that Lobster's compiler is expected to do well in, so the results are one
half of a fair picture; the categories it is expected to lose (hash maps, string building,
byte streams, multicore) are deliberately not here yet.

## Ground rules

- **In-process timing.** Each program times each benchmark itself with a monotonic clock,
  runs it `REPEATS` times, and reports the *minimum*. Process startup, JIT warmup outside
  the measured region, and I/O are therefore excluded. The harness additionally runs each
  program several times and takes the minimum of the minima.
- **Same algorithm, same order.** These are implementation comparisons, not algorithm
  comparisons. Loop order, operation order and data layout must match across languages, so
  that floating point results agree to the last few bits.
- **Idiomatic within that constraint.** Use each language's natural data structures and
  syntax. Do not hand-vectorise, do not use intrinsics, do not use unsafe/unchecked escape
  hatches unless a note says the language's idiomatic code would use them.
- **No fast-math.** Do not enable `-ffast-math`, `/fp:fast` or reassociation. Clang builds
  must pass `-ffp-contract=off`, because clang defaults to forming FMA in C++ and that
  changes results relative to MSVC.
- **No output in the timed region.**

## Output protocol

Each program writes to stdout, one line per benchmark, tab separated:

```
bench<TAB><name><TAB><seconds><TAB><checksum>
```

`<name>` is one of `nbody raytrace interp sort astar`. `<seconds>` is a decimal number.
`<checksum>` is printed with 17 significant digits (`%.17g` or equivalent), which round-trips
a double exactly; Lobster's default float formatting already does. The harness parses it as a
double and compares with a relative tolerance of 1e-9. Anything else the program prints must
start with `#`.

Command line: `<program> [repeats] [name]`. `repeats` defaults to 5. If `name` is given,
only that benchmark runs.

## Shared pseudo-random generator

A 32-bit linear congruential generator, chosen because every target language can evaluate it
exactly, including JavaScript: the largest intermediate is `2^32 * 1664525 + 1013904223`,
which is under `2^53`.

```
state : integer in [0, 2^32)

next()  = state := (state * 1664525 + 1013904223) mod 2^32 ; return state
nextf() = next() * (1.0 / 4294967296.0)          -> [0, 1)
range(a, b) = a + nextf() * (b - a)
below(n) = next() mod n
```

Every benchmark seeds it explicitly. No other source of randomness is allowed.

## 1. nbody

Gravitational n-body with softening, exercising value-type vector maths and a flat array of
structs with no indirection.

- `N = 256`, `STEPS = 1000`, `DT = 0.001`, `EPS2 = 0.5`, seed `12345`.
- Bodies are generated in order, each drawing seven values from the generator, in this
  sequence: `p.x, p.y, p.z` from `range(-40, 40)`, `v.x, v.y, v.z` from `range(-0.5, 0.5)`,
  then `mass` from `range(0.2, 1.2)`.
- Each step, for every body `i` in index order, accumulate acceleration over **all** bodies
  `j` in index order including `i` itself:
  ```
  d   = p[j] - p[i]
  r2  = dot(d, d) + EPS2
  a  += d * (m[j] / (r2 * sqrt(r2)))
  ```
  The self term contributes exactly zero, which is why no branch is needed. The full `N^2`
  form is used rather than the symmetric half so that no accumulator is written twice and
  the summation order is unambiguous.
- Accelerations for the whole system are computed before any of them is applied. Then, for
  every body in index order: `v += a * DT`, then `p += v * DT`.
- Checksum: total energy after the last step, kinetic plus potential, computed as
  `sum(0.5 * m[i] * dot(v[i], v[i]))` over `i` in order, minus
  `sum(m[i] * m[j] / sqrt(dot(p[j] - p[i], p[j] - p[i]) + EPS2))` over `i < j` in
  lexicographic order.

## 2. raytrace

Whitted-style ray tracer: one primary ray per pixel, a shadow ray per hit, and mirror
recursion. Exercises struct maths, recursion and a read-only array of larger structs.

- Image `640 x 480`, `MAX_DEPTH = 3`, seed `98765`.
- `48` spheres, generated in order, each drawing seven values: centre `x` from
  `range(-6, 6)`, `y` from `range(0.4, 4.5)`, `z` from `range(-6, 6)`, radius from
  `range(0.35, 0.9)`, colour `r, g, b` each from `range(0.2, 1.0)`. Reflectivity is
  `0.55` when `i mod 3 == 0`, otherwise `0.0`.
- Ground plane `y = 0`, with a checkerboard albedo: `0.9` when
  `(floor(x) + floor(z))` is even, `0.35` when odd, applied to all three channels.
  Reflectivity `0.25`.
- Camera: `eye = (0, 2.6, 9)`, `target = (0, 1.2, 0)`, world up `(0, 1, 0)`,
  `fwd = normalize(target - eye)`, `right = normalize(cross(fwd, up))`,
  `cam_up = cross(right, fwd)`, half-height `tan(27.5 degrees)`, aspect `W / H`.
  For pixel `(px, py)`, with `u = (px + 0.5) / W * 2 - 1` and
  `v = 1 - (py + 0.5) / H * 2`, the ray direction is
  `normalize(fwd + right * (u * aspect * half) + cam_up * (v * half))`.
- Light: a single directional light from `normalize((-0.6, 1.0, 0.45))`, intensity `1.0`,
  plus ambient `0.12`. A hit point is offset along its normal by `1e-4` before the shadow
  ray. Shadow rays test spheres only, not the plane.
- Shading is Lambert: `albedo * (ambient + max(0, dot(n, l)) * (shadowed ? 0 : 1))`.
  If reflectivity `k > 0` and depth remains, the result is
  `shaded * (1 - k) + trace(reflected, depth - 1) * k`.
- Misses return the background `(0.35, 0.52, 0.85)` scaled by `0.4 + 0.6 * max(0, dir.y)`.
- Sphere intersection uses the geometric form with `t` accepted when `t > 1e-4`; the nearest
  hit across spheres wins, tested in index order, then the plane is tested and replaces the
  hit if it is nearer.
- Checksum: the sum of `r + g + b` over all pixels, in row-major order.

## 3. interp

An AST interpreter over a tree built once and evaluated many times. Each language uses its
own idiomatic dispatch: a `switch` over class cases in Lobster, `match` over an enum in
Rust, virtual calls in C++, Java and C#, class dispatch in Python and JS.

- `DEPTH = 14`, `ROUNDS = 1500`, `NVARS = 8`, seed `4242`.
- The tree is built recursively, root at depth `DEPTH`, in pre-order: the node's own draws
  happen before its children are built, left child before right.
  - At depth `0`: draw `k = below(3)`. `k == 0` gives `Const(range(-1, 1))`,
    `k == 1` gives `Var(below(NVARS))`, `k == 2` gives `Neg` wrapping a freshly built
    depth-0 node.
  - Above depth `0`: draw `k = below(3)` when the depth is `3` or less, otherwise
    `k = below(2)`. `0` is `Add`, `1` is `Sub`, `2` is `Mul`. Multiplication is confined to
    the bottom three levels so the tree value cannot overflow to infinity, which would make
    the checksum meaningless.
- Evaluation: for round `r` in `0 .. ROUNDS-1`, set `env[k]` to
  `((r * 7 + k * 13) mod 101) / 50.0 - 1.0` for each `k`, then evaluate the tree and add the
  result to a running total.
- Checksum: that running total.

## 4. sort

The same quicksort in every language, driven by a caller-supplied comparison function. This
isolates the cost of calling a user comparator, which is the thing that differs between
implementations; it deliberately does not use each language's library sort.

- `N = 400000`, `ROUNDS = 6`, seed `777`.
- Records are `{ key: float, id: int }`. Record `i` gets `key = range(-1000, 1000)` then
  `id = i`, in that order.
- Comparison: `a.key < b.key`, and when the keys are equal, `a.id < b.id`.
- Each round copies the source array and sorts the copy, so every round does the same work.
- The algorithm, which must be reproduced exactly:
  ```
  sort(lo, hi):                    # hi exclusive
      if hi - lo <= 1: return
      pivot = xs[lo]
      sp = lo + 1
      ep = hi
      while sp < ep:
          c = xs[sp]
          if less(c, pivot):
              xs[sp - 1] = xs[sp]
              sp += 1
          else:
              ep -= 1
              xs[sp] = xs[ep]
              xs[ep] = c
      sp -= 1
      xs[sp] = pivot
      sort(lo, sp)
      sort(ep, hi)
  ```
- Checksum: after the last round, `sum over i of ((i mod 97) + 1) * id[i]`, as a float. The
  implementation must also verify the array is sorted and fail loudly if not.

## 5. astar

A* over a random 4-connected grid with a binary heap. Integer only, so the checksum is exact
in every language regardless of floating point.

- Grid `512 x 512`, `ROUNDS = 20`, wall probability `0.20`, round `r` uses seed `1000 + r`.
- Generation: cells in row-major order, cell is a wall when `nextf() < 0.20`. Then six cells
  are forced open: the start `(0,0)` and its two orthogonal neighbours, and the goal
  `(W-1,H-1)` and its two orthogonal neighbours. Without that, a corner whose only two
  neighbours are both walls is trapped often enough to turn a round into no work at all. With
  it, all twenty rounds find a path, at a cost of `1022`, expanding around 128000 nodes each.
- Cost per step `1`, heuristic the Manhattan distance to the goal. Since neither coordinate
  can exceed the goal's, that distance is `(W-1-x) + (H-1-y)` and needs no absolute value.
- Neighbours come from the direction table `[(1,0), (0,1), (-1,0), (0,-1)]`, iterated in that
  order, with the candidate rejected when it falls outside the grid.
- Open list is a binary min-heap of `(f, node)` ordered by `f`, ties broken by the smaller
  node index, so the expansion order is fully determined. Sift-up on push, sift-down on pop,
  standard array heap with children at `2i+1` and `2i+2`.
- Loop: pop the smallest; skip it if already closed; mark closed and count it; stop if it is
  the goal; otherwise for each open, not-closed neighbour with `g + 1 < gscore[neighbour]`,
  record the new `gscore` and push. `gscore` starts at the sentinel `0x3FFFFFFF`.
- The grid, the `gscore` and `closed` arrays and the heap are allocated fresh each round, so
  allocation is part of the measured work.
- Checksum: `sum over rounds of (path_cost * 1000003 + nodes_expanded)`, where `path_cost` is
  `-1` if the goal was unreachable. Accumulated as an integer, reported as a float.

## Configurations

The harness builds and runs every configuration it can find a toolchain for, and skips the
rest with a note in the results.

| language | configurations |
| --- | --- |
| Lobster | libtcc JIT (the default), `--mir 2`, `--cpp` + MSVC `/O2`, `--cpp` + clang-cl `/O2` |
| C++ | MSVC `/O2`, MSVC `/O2 /GL` (LTCG), clang-cl `/O2`, clang-cl `/O2 -flto` |
| Rust | `opt-level=3`, `opt-level=3` + `lto=fat` + `codegen-units=1` |
| Java | default HotSpot C2 |
| Node | default V8 |
| Python | CPython 3.13 |
| C# | .NET SDK required, not installed at time of writing |

## Implementation notes

Where a language cannot express what the others do, the choice made here is recorded so the
results are read correctly.

- **Java and JavaScript have no value types.** `V3` is an immutable object in both, allocated
  per operation, and the vector maths depends on the runtime's escape analysis to scalar-replace
  it. That is the shape a program in those languages would have. For reference, hand-scalarising
  the nbody inner loop in Java brings it from 0.23 s to 0.15 s, so escape analysis recovers most
  but not all of the gap; the object version is what the table reports.
- **Python** unpacks vectors into local floats rather than wrapping them in a class. A class with
  an operator method per component is several times slower again and is not what anyone who cared
  about the speed of pure Python would write. Anyone who cared more would reach for numpy, which
  stops being a comparison of language implementations.
- **Lua** does the same, and runs unchanged on Lua 5.5 and LuaJIT 2.1, so it avoids the integer
  division and bitwise operators LuaJIT does not have.
- **The comparator in `sort`** is monomorphised in C++ (template) and Rust (generic), and is a
  function value everywhere else: a delegate in C#, a functional interface in Java, a closure in
  Go, JavaScript, Python and Lua, and an untyped parameter in Lobster, which specializes and
  inlines it. That difference is the point of the benchmark.
- **The AST interpreter** uses each language's idiomatic dispatch: a `switch` over class cases in
  Lobster, `match` over an enum in Rust, virtual calls in C++, Java and C#, an interface in Go,
  class dispatch in Python and JavaScript, and a tag field in Lua.
- **JavaScript** holds the integer grids in `Float64Array` rather than `Int32Array`, so that every
  language stores the same eight bytes per element.
- **Lobster's `--cpp` clang configuration** compiles only the generated code with clang-cl and
  links it against the MSVC-built Lobster runtime. Rebuilding the runtime under clang takes
  minutes and risks compile errors in the vendored engine, and what this comparison is about is
  the code the Lobster backend emits.
