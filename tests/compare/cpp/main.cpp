// C++ implementation of the cross-language benchmarks, see ../SPEC.md.
//
// Build with the harness, or by hand:
//     cl /O2 /std:c++20 /EHsc main.cpp
//     clang++ -O3 -std=c++20 -ffp-contract=off main.cpp

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

// ------------------------------------------------------------------ shared

// The 32-bit LCG from SPEC.md. Unsigned wraparound is exactly the modulo it asks for.
struct Rnd {
    uint32_t state;
    explicit Rnd(uint32_t s) : state(s) {}
    uint32_t next() {
        state = state * 1664525u + 1013904223u;
        return state;
    }
    double nextf() { return next() * (1.0 / 4294967296.0); }
    double range(double a, double b) { return a + nextf() * (b - a); }
    int64_t below(int64_t n) { return (int64_t)next() % n; }
};

struct V3 {
    double x, y, z;
};

static inline V3 operator+(V3 a, V3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline V3 operator-(V3 a, V3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline V3 operator*(V3 a, double s) { return { a.x * s, a.y * s, a.z * s }; }
static inline double dot(V3 a, V3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline V3 cross(V3 a, V3 b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
static inline V3 normalize(V3 v) {
    double l = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return l == 0.0 ? V3 { 0.0, 0.0, 0.0 } : V3 { v.x / l, v.y / l, v.z / l };
}

// ------------------------------------------------------------------ 1. nbody

static double bench_nbody() {
    const int N = 256;
    const int STEPS = 1000;
    const double DT = 0.001;
    const double EPS2 = 0.5;
    struct Body {
        V3 p;
        double m;
    };
    Rnd rnd(12345);
    std::vector<Body> bodies;
    std::vector<V3> vel;
    bodies.reserve(N);
    vel.reserve(N);
    for (int i = 0; i < N; i++) {
        V3 p { rnd.range(-40.0, 40.0), rnd.range(-40.0, 40.0), rnd.range(-40.0, 40.0) };
        V3 v { rnd.range(-0.5, 0.5), rnd.range(-0.5, 0.5), rnd.range(-0.5, 0.5) };
        bodies.push_back({ p, rnd.range(0.2, 1.2) });
        vel.push_back(v);
    }
    std::vector<V3> acc(N, V3 { 0.0, 0.0, 0.0 });
    for (int s = 0; s < STEPS; s++) {
        for (int i = 0; i < N; i++) {
            V3 pi = bodies[i].p;
            V3 a { 0.0, 0.0, 0.0 };
            for (int j = 0; j < N; j++) {
                V3 d = bodies[j].p - pi;
                double r2 = dot(d, d) + EPS2;
                // The j == i term has d == 0, so it contributes exactly zero.
                a = a + d * (bodies[j].m / (r2 * std::sqrt(r2)));
            }
            acc[i] = a;
        }
        for (int i = 0; i < N; i++) {
            V3 v = vel[i] + acc[i] * DT;
            vel[i] = v;
            bodies[i].p = bodies[i].p + v * DT;
        }
    }
    double e = 0.0;
    for (int i = 0; i < N; i++) e += 0.5 * bodies[i].m * dot(vel[i], vel[i]);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (j > i) {
                V3 d = bodies[j].p - bodies[i].p;
                e -= bodies[i].m * bodies[j].m / std::sqrt(dot(d, d) + EPS2);
            }
        }
    }
    return e;
}

// ------------------------------------------------------------------ 2. raytrace

namespace rt {

const int W = 640;
const int H = 480;
const int NSPHERES = 48;
const int MAX_DEPTH = 3;
const double EPS = 0.0001;
// tan(27.5 degrees).
const double HALF = 0.5205670505517462;
const double AMBIENT = 0.12;

struct Sphere {
    V3 c;
    double r;
    V3 col;
    double refl;
};

struct Hit {
    double t;
    V3 n;
    V3 albedo;
    double refl;
};

static const V3 BACKGROUND { 0.35, 0.52, 0.85 };
static const V3 UP { 0.0, 1.0, 0.0 };

static inline double hit_sphere(const Sphere &s, V3 ro, V3 rd) {
    V3 oc = s.c - ro;
    double b = dot(oc, rd);
    double c = dot(oc, oc) - s.r * s.r;
    double disc = b * b - c;
    if (!(disc > 0.0)) return -1.0;
    double sd = std::sqrt(disc);
    double t = b - sd;
    if (t > EPS) return t;
    double t2 = b + sd;
    return t2 > EPS ? t2 : -1.0;
}

static bool any_hit(const std::vector<Sphere> &spheres, V3 ro, V3 rd) {
    for (const auto &s : spheres) {
        if (hit_sphere(s, ro, rd) > 0.0) return true;
    }
    return false;
}

static Hit nearest(const std::vector<Sphere> &spheres, V3 ro, V3 rd) {
    Hit best { -1.0, { 0, 0, 0 }, { 0, 0, 0 }, 0.0 };
    for (const auto &s : spheres) {
        double t = hit_sphere(s, ro, rd);
        if (t > 0.0 && (best.t < 0.0 || t < best.t)) {
            best = Hit { t, normalize(ro + rd * t - s.c), s.col, s.refl };
        }
    }
    if (rd.y < -1.0e-9) {
        double t = -ro.y / rd.y;
        if (t > EPS && (best.t < 0.0 || t < best.t)) {
            V3 p = ro + rd * t;
            int64_t fx = (int64_t)std::floor(p.x);
            int64_t fz = (int64_t)std::floor(p.z);
            double shade = (fx + fz) % 2 == 0 ? 0.9 : 0.35;
            best = Hit { t, UP, { shade, shade, shade }, 0.25 };
        }
    }
    return best;
}

static V3 trace(const std::vector<Sphere> &spheres, V3 light, V3 ro, V3 rd, int depth) {
    Hit h = nearest(spheres, ro, rd);
    if (!(h.t > 0.0)) {
        double s = rd.y > 0.0 ? rd.y : 0.0;
        return BACKGROUND * (0.4 + 0.6 * s);
    }
    V3 p = ro + rd * h.t;
    V3 off = p + h.n * EPS;
    double lit = any_hit(spheres, off, light) ? 0.0 : 1.0;
    double nl = dot(h.n, light);
    if (nl < 0.0) nl = 0.0;
    V3 shaded = h.albedo * (AMBIENT + nl * lit);
    if (!(h.refl > 0.0 && depth > 0)) return shaded;
    V3 refd = rd - h.n * (2.0 * dot(h.n, rd));
    return shaded * (1.0 - h.refl) + trace(spheres, light, off, refd, depth - 1) * h.refl;
}

}  // namespace rt

static double bench_raytrace() {
    using namespace rt;
    Rnd rnd(98765);
    std::vector<Sphere> spheres;
    spheres.reserve(NSPHERES);
    for (int i = 0; i < NSPHERES; i++) {
        V3 c { rnd.range(-6.0, 6.0), rnd.range(0.4, 4.5), rnd.range(-6.0, 6.0) };
        double r = rnd.range(0.35, 0.9);
        V3 col { rnd.range(0.2, 1.0), rnd.range(0.2, 1.0), rnd.range(0.2, 1.0) };
        spheres.push_back({ c, r, col, i % 3 == 0 ? 0.55 : 0.0 });
    }
    V3 light = normalize(V3 { -0.6, 1.0, 0.45 });
    V3 eye { 0.0, 2.6, 9.0 };
    V3 fwd = normalize(V3 { 0.0, 1.2, 0.0 } - eye);
    V3 right = normalize(cross(fwd, UP));
    V3 cam_up = cross(right, fwd);
    double aspect = (double)W / (double)H;
    double sum = 0.0;
    for (int py = 0; py < H; py++) {
        double v = 1.0 - ((double)py + 0.5) / (double)H * 2.0;
        for (int px = 0; px < W; px++) {
            double u = ((double)px + 0.5) / (double)W * 2.0 - 1.0;
            V3 rd = normalize(fwd + right * (u * aspect * HALF) + cam_up * (v * HALF));
            V3 col = trace(spheres, light, eye, rd, MAX_DEPTH);
            sum += col.x + col.y + col.z;
        }
    }
    return sum;
}

// ------------------------------------------------------------------ 3. interp

namespace ip {

const int DEPTH = 14;
const int ROUNDS = 1500;
const int NVARS = 8;

struct Expr {
    virtual ~Expr() = default;
    virtual double eval(const double *env) const = 0;
};
using P = std::unique_ptr<Expr>;

struct Const : Expr {
    double v;
    explicit Const(double v) : v(v) {}
    double eval(const double *) const override { return v; }
};
struct Var : Expr {
    int64_t i;
    explicit Var(int64_t i) : i(i) {}
    double eval(const double *env) const override { return env[i]; }
};
struct Neg : Expr {
    P a;
    explicit Neg(P a) : a(std::move(a)) {}
    double eval(const double *env) const override { return -a->eval(env); }
};
struct Add : Expr {
    P a, b;
    Add(P a, P b) : a(std::move(a)), b(std::move(b)) {}
    double eval(const double *env) const override { return a->eval(env) + b->eval(env); }
};
struct Sub : Expr {
    P a, b;
    Sub(P a, P b) : a(std::move(a)), b(std::move(b)) {}
    double eval(const double *env) const override { return a->eval(env) - b->eval(env); }
};
struct Mul : Expr {
    P a, b;
    Mul(P a, P b) : a(std::move(a)), b(std::move(b)) {}
    double eval(const double *env) const override { return a->eval(env) * b->eval(env); }
};

static P build(Rnd &rnd, int d) {
    if (d == 0) {
        switch (rnd.below(3)) {
            case 0: return P(new Const(rnd.range(-1.0, 1.0)));
            case 1: return P(new Var(rnd.below(NVARS)));
            default: return P(new Neg(build(rnd, 0)));
        }
    }
    // Multiplication only near the leaves, so the value of the tree stays finite.
    int64_t k = rnd.below(d <= 3 ? 3 : 2);
    P a = build(rnd, d - 1);
    P b = build(rnd, d - 1);
    switch (k) {
        case 0: return P(new Add(std::move(a), std::move(b)));
        case 1: return P(new Sub(std::move(a), std::move(b)));
        default: return P(new Mul(std::move(a), std::move(b)));
    }
}

}  // namespace ip

static double bench_interp() {
    using namespace ip;
    Rnd rnd(4242);
    P tree = build(rnd, DEPTH);
    double env[NVARS] = {};
    double total = 0.0;
    for (int r = 0; r < ROUNDS; r++) {
        for (int k = 0; k < NVARS; k++) {
            env[k] = (double)((r * 7 + k * 13) % 101) / 50.0 - 1.0;
        }
        total += tree->eval(env);
    }
    return total;
}

// ------------------------------------------------------------------ 4. sort

namespace srt {

const int64_t N = 400000;
const int ROUNDS = 6;

struct Rec {
    double key;
    int64_t id;
};

// The comparison arrives as a template parameter, which is how C++ code that takes one is
// normally written, and means it ends up inline in the partition loop.
template <class Less>
static void sort_rec(Rec *a, int64_t lo, int64_t hi, Less less) {
    if (hi - lo <= 1) return;
    Rec pivot = a[lo];
    int64_t sp = lo + 1;
    int64_t ep = hi;
    while (sp < ep) {
        Rec c = a[sp];
        if (less(c, pivot)) {
            a[sp - 1] = a[sp];
            sp++;
        } else {
            ep--;
            a[sp] = a[ep];
            a[ep] = c;
        }
    }
    sp--;
    a[sp] = pivot;
    sort_rec(a, lo, sp, less);
    sort_rec(a, ep, hi, less);
}

}  // namespace srt

static double bench_sort() {
    using namespace srt;
    Rnd rnd(777);
    std::vector<Rec> src;
    src.reserve(N);
    for (int64_t i = 0; i < N; i++) src.push_back({ rnd.range(-1000.0, 1000.0), i });
    auto less = [](const Rec &a, const Rec &b) {
        return a.key < b.key || (a.key == b.key && a.id < b.id);
    };
    std::vector<Rec> last;
    for (int r = 0; r < ROUNDS; r++) {
        std::vector<Rec> xs = src;
        sort_rec(xs.data(), 0, (int64_t)xs.size(), less);
        last = std::move(xs);
    }
    for (size_t i = 1; i < last.size(); i++) {
        if (less(last[i], last[i - 1])) {
            printf("# sort: not sorted at %zu\n", i);
            return 0.0;
        }
    }
    double sum = 0.0;
    for (size_t i = 0; i < last.size(); i++) {
        sum += (double)(((int64_t)i % 97 + 1) * last[i].id);
    }
    return sum;
}

// ------------------------------------------------------------------ 5. astar

namespace as {

const int64_t W = 512;
const int64_t H = 512;
const int ROUNDS = 20;
const double WALL_P = 0.20;
const int64_t INF = 0x3FFFFFFF;

struct HeapEnt {
    int64_t f, n;
};

static inline bool before(HeapEnt a, HeapEnt b) {
    return a.f < b.f || (a.f == b.f && a.n < b.n);
}

static void hpush(std::vector<HeapEnt> &heap, HeapEnt e) {
    heap.push_back(e);
    int64_t i = (int64_t)heap.size() - 1;
    while (i > 0) {
        int64_t p = (i - 1) >> 1;
        HeapEnt pe = heap[p];
        if (!before(e, pe)) break;
        heap[i] = pe;
        i = p;
    }
    heap[i] = e;
}

static HeapEnt hpop(std::vector<HeapEnt> &heap) {
    HeapEnt top = heap[0];
    HeapEnt last = heap.back();
    heap.pop_back();
    int64_t n = (int64_t)heap.size();
    if (n > 0) {
        int64_t i = 0;
        for (;;) {
            int64_t l = (i << 1) + 1;
            if (l >= n) break;
            int64_t c = l;
            int64_t r = l + 1;
            if (r < n && before(heap[r], heap[l])) c = r;
            HeapEnt ce = heap[c];
            if (!before(ce, last)) break;
            heap[i] = ce;
            i = c;
        }
        heap[i] = last;
    }
    return top;
}

struct Result {
    int64_t cost, expanded;
};

static Result solve(uint32_t seed) {
    Rnd rnd(seed);
    std::vector<int64_t> grid;
    grid.reserve(W * H);
    for (int64_t i = 0; i < W * H; i++) grid.push_back(rnd.nextf() < WALL_P ? 1 : 0);
    int64_t goal = W * H - 1;
    grid[0] = 0;
    grid[1] = 0;
    grid[W] = 0;
    grid[goal] = 0;
    grid[goal - 1] = 0;
    grid[goal - W] = 0;
    std::vector<int64_t> gscore(W * H, INF);
    std::vector<int64_t> closed(W * H, 0);
    std::vector<HeapEnt> heap;
    const int64_t dx[4] = { 1, 0, -1, 0 };
    const int64_t dy[4] = { 0, 1, 0, -1 };
    int64_t gx = W - 1;
    int64_t gy = H - 1;
    gscore[0] = 0;
    hpush(heap, { gx + gy, 0 });
    int64_t expanded = 0;
    int64_t cost = -1;
    while (!heap.empty()) {
        HeapEnt e = hpop(heap);
        int64_t n = e.n;
        if (closed[n] == 0) {
            closed[n] = 1;
            expanded++;
            if (n == goal) {
                cost = gscore[n];
                break;
            }
            int64_t y = n / W;
            int64_t x = n - y * W;
            int64_t g = gscore[n] + 1;
            for (int k = 0; k < 4; k++) {
                int64_t nx = x + dx[k];
                int64_t ny = y + dy[k];
                if (nx >= 0 && ny >= 0 && nx < W && ny < H) {
                    int64_t nb = n + dy[k] * W + dx[k];
                    if (grid[nb] == 0 && closed[nb] == 0 && g < gscore[nb]) {
                        gscore[nb] = g;
                        hpush(heap, { g + (gx - nx) + (gy - ny), nb });
                    }
                }
            }
        }
    }
    return { cost, expanded };
}

}  // namespace as

static double bench_astar() {
    int64_t sum = 0;
    for (int r = 0; r < as::ROUNDS; r++) {
        as::Result res = as::solve((uint32_t)(1000 + r));
        sum += res.cost * 1000003 + res.expanded;
    }
    return (double)sum;
}

// ------------------------------------------------------------------ driver

struct Bench {
    const char *name;
    double (*fn)();
};

static const Bench BENCHES[] = {
    { "nbody", bench_nbody },   { "raytrace", bench_raytrace }, { "interp", bench_interp },
    { "sort", bench_sort },     { "astar", bench_astar },
};

int main(int argc, char **argv) {
    int repeats = 5;
    std::string only;
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") != 0) args.push_back(argv[i]);
    }
    if (args.size() > 0) {
        int n = atoi(args[0].c_str());
        if (n > 0) repeats = n;
    }
    if (args.size() > 1) only = args[1];
    for (const auto &b : BENCHES) {
        if (!only.empty() && only != b.name) continue;
        double best = 1.0e30;
        double checksum = 0.0;
        for (int r = 0; r < repeats; r++) {
            auto t0 = std::chrono::steady_clock::now();
            checksum = b.fn();
            std::chrono::duration<double> dt = std::chrono::steady_clock::now() - t0;
            if (dt.count() < best) best = dt.count();
        }
        printf("bench\t%s\t%.7f\t%.17g\n", b.name, best, checksum);
        fflush(stdout);
    }
    return 0;
}
