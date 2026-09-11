// C# implementation of the cross-language benchmarks, see ../SPEC.md.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;

// ------------------------------------------------------------------ shared

/// The 32-bit LCG from SPEC.md. uint wraparound is exactly the modulo it asks for.
sealed class Rnd {
    uint state;
    public Rnd(uint seed) { state = seed; }
    public uint Next() { state = state * 1664525u + 1013904223u; return state; }
    public double Nextf() => Next() * (1.0 / 4294967296.0);
    public double Range(double a, double b) => a + Nextf() * (b - a);
    public long Below(long n) => (long)Next() % n;
}

readonly struct V3 {
    public readonly double x, y, z;
    public V3(double x, double y, double z) { this.x = x; this.y = y; this.z = z; }
    public static V3 operator +(V3 a, V3 b) => new V3(a.x + b.x, a.y + b.y, a.z + b.z);
    public static V3 operator -(V3 a, V3 b) => new V3(a.x - b.x, a.y - b.y, a.z - b.z);
    public static V3 operator *(V3 a, double s) => new V3(a.x * s, a.y * s, a.z * s);
}

static class Vec {
    public static double Dot(V3 a, V3 b) => a.x * b.x + a.y * b.y + a.z * b.z;
    public static V3 Cross(V3 a, V3 b) => new V3(
        a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    public static V3 Normalize(V3 v) {
        double l = Math.Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        return l == 0.0 ? new V3(0, 0, 0) : new V3(v.x / l, v.y / l, v.z / l);
    }
}

// ------------------------------------------------------------------ 1. nbody

static class NBody {
    struct Body { public V3 p; public double m; }

    public static double Run() {
        const int N = 256, STEPS = 1000;
        const double DT = 0.001, EPS2 = 0.5;
        var rnd = new Rnd(12345);
        var bodies = new Body[N];
        var vel = new V3[N];
        for (int i = 0; i < N; i++) {
            var p = new V3(rnd.Range(-40, 40), rnd.Range(-40, 40), rnd.Range(-40, 40));
            var v = new V3(rnd.Range(-0.5, 0.5), rnd.Range(-0.5, 0.5), rnd.Range(-0.5, 0.5));
            bodies[i] = new Body { p = p, m = rnd.Range(0.2, 1.2) };
            vel[i] = v;
        }
        var acc = new V3[N];
        for (int s = 0; s < STEPS; s++) {
            for (int i = 0; i < N; i++) {
                var pi = bodies[i].p;
                var a = new V3(0, 0, 0);
                for (int j = 0; j < N; j++) {
                    var d = bodies[j].p - pi;
                    double r2 = Vec.Dot(d, d) + EPS2;
                    // The j == i term has d == 0, so it contributes exactly zero.
                    a = a + d * (bodies[j].m / (r2 * Math.Sqrt(r2)));
                }
                acc[i] = a;
            }
            for (int i = 0; i < N; i++) {
                var v = vel[i] + acc[i] * DT;
                vel[i] = v;
                bodies[i].p = bodies[i].p + v * DT;
            }
        }
        double e = 0.0;
        for (int i = 0; i < N; i++) e += 0.5 * bodies[i].m * Vec.Dot(vel[i], vel[i]);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (j > i) {
                    var d = bodies[j].p - bodies[i].p;
                    e -= bodies[i].m * bodies[j].m / Math.Sqrt(Vec.Dot(d, d) + EPS2);
                }
            }
        }
        return e;
    }
}

// ------------------------------------------------------------------ 2. raytrace

static class Raytrace {
    const int W = 640, H = 480, NSPHERES = 48, MAX_DEPTH = 3;
    const double EPS = 0.0001;
    // tan(27.5 degrees).
    const double HALF = 0.5205670505517462;
    const double AMBIENT = 0.12;

    struct Sphere { public V3 c; public double r; public V3 col; public double refl; }
    struct Hit { public double t; public V3 n; public V3 albedo; public double refl; }

    static double HitSphere(in Sphere s, V3 ro, V3 rd) {
        var oc = s.c - ro;
        double b = Vec.Dot(oc, rd);
        double c = Vec.Dot(oc, oc) - s.r * s.r;
        double disc = b * b - c;
        if (!(disc > 0.0)) return -1.0;
        double sd = Math.Sqrt(disc);
        double t = b - sd;
        if (t > EPS) return t;
        double t2 = b + sd;
        return t2 > EPS ? t2 : -1.0;
    }

    static bool AnyHit(Sphere[] spheres, V3 ro, V3 rd) {
        for (int i = 0; i < spheres.Length; i++) {
            if (HitSphere(in spheres[i], ro, rd) > 0.0) return true;
        }
        return false;
    }

    static Hit Nearest(Sphere[] spheres, V3 ro, V3 rd) {
        var best = new Hit { t = -1.0, n = new V3(0, 0, 0), albedo = new V3(0, 0, 0), refl = 0.0 };
        for (int i = 0; i < spheres.Length; i++) {
            double t = HitSphere(in spheres[i], ro, rd);
            if (t > 0.0 && (best.t < 0.0 || t < best.t)) {
                best = new Hit {
                    t = t,
                    n = Vec.Normalize(ro + rd * t - spheres[i].c),
                    albedo = spheres[i].col,
                    refl = spheres[i].refl
                };
            }
        }
        if (rd.y < -1.0e-9) {
            double t = -ro.y / rd.y;
            if (t > EPS && (best.t < 0.0 || t < best.t)) {
                var p = ro + rd * t;
                long fx = (long)Math.Floor(p.x);
                long fz = (long)Math.Floor(p.z);
                double shade = (fx + fz) % 2 == 0 ? 0.9 : 0.35;
                best = new Hit {
                    t = t, n = new V3(0, 1, 0), albedo = new V3(shade, shade, shade), refl = 0.25
                };
            }
        }
        return best;
    }

    static V3 Trace(Sphere[] spheres, V3 light, V3 ro, V3 rd, int depth) {
        var h = Nearest(spheres, ro, rd);
        if (!(h.t > 0.0)) {
            double sk = rd.y > 0.0 ? rd.y : 0.0;
            return new V3(0.35, 0.52, 0.85) * (0.4 + 0.6 * sk);
        }
        var p = ro + rd * h.t;
        var off = p + h.n * EPS;
        double lit = AnyHit(spheres, off, light) ? 0.0 : 1.0;
        double nl = Vec.Dot(h.n, light);
        if (nl < 0.0) nl = 0.0;
        var shaded = h.albedo * (AMBIENT + nl * lit);
        if (!(h.refl > 0.0 && depth > 0)) return shaded;
        var refd = rd - h.n * (2.0 * Vec.Dot(h.n, rd));
        return shaded * (1.0 - h.refl) + Trace(spheres, light, off, refd, depth - 1) * h.refl;
    }

    public static double Run() {
        var rnd = new Rnd(98765);
        var spheres = new Sphere[NSPHERES];
        for (int i = 0; i < NSPHERES; i++) {
            var c = new V3(rnd.Range(-6, 6), rnd.Range(0.4, 4.5), rnd.Range(-6, 6));
            double r = rnd.Range(0.35, 0.9);
            var col = new V3(rnd.Range(0.2, 1.0), rnd.Range(0.2, 1.0), rnd.Range(0.2, 1.0));
            spheres[i] = new Sphere { c = c, r = r, col = col, refl = i % 3 == 0 ? 0.55 : 0.0 };
        }
        var light = Vec.Normalize(new V3(-0.6, 1.0, 0.45));
        var eye = new V3(0.0, 2.6, 9.0);
        var fwd = Vec.Normalize(new V3(0.0, 1.2, 0.0) - eye);
        var right = Vec.Normalize(Vec.Cross(fwd, new V3(0, 1, 0)));
        var camUp = Vec.Cross(right, fwd);
        double aspect = (double)W / (double)H;
        double sum = 0.0;
        for (int py = 0; py < H; py++) {
            double v = 1.0 - ((double)py + 0.5) / (double)H * 2.0;
            for (int px = 0; px < W; px++) {
                double u = ((double)px + 0.5) / (double)W * 2.0 - 1.0;
                var rd = Vec.Normalize(fwd + right * (u * aspect * HALF) + camUp * (v * HALF));
                var col = Trace(spheres, light, eye, rd, MAX_DEPTH);
                sum += col.x + col.y + col.z;
            }
        }
        return sum;
    }
}

// ------------------------------------------------------------------ 3. interp

abstract class Expr {
    public abstract double Eval(double[] env);
}
sealed class ConstE : Expr {
    readonly double v;
    public ConstE(double v) { this.v = v; }
    public override double Eval(double[] env) => v;
}
sealed class VarE : Expr {
    readonly int i;
    public VarE(int i) { this.i = i; }
    public override double Eval(double[] env) => env[i];
}
sealed class NegE : Expr {
    readonly Expr a;
    public NegE(Expr a) { this.a = a; }
    public override double Eval(double[] env) => -a.Eval(env);
}
sealed class AddE : Expr {
    readonly Expr a, b;
    public AddE(Expr a, Expr b) { this.a = a; this.b = b; }
    public override double Eval(double[] env) => a.Eval(env) + b.Eval(env);
}
sealed class SubE : Expr {
    readonly Expr a, b;
    public SubE(Expr a, Expr b) { this.a = a; this.b = b; }
    public override double Eval(double[] env) => a.Eval(env) - b.Eval(env);
}
sealed class MulE : Expr {
    readonly Expr a, b;
    public MulE(Expr a, Expr b) { this.a = a; this.b = b; }
    public override double Eval(double[] env) => a.Eval(env) * b.Eval(env);
}

static class Interp {
    const int DEPTH = 14, ROUNDS = 1500, NVARS = 8;

    static Expr Build(Rnd rnd, int d) {
        if (d == 0) {
            switch (rnd.Below(3)) {
                case 0: return new ConstE(rnd.Range(-1.0, 1.0));
                case 1: return new VarE((int)rnd.Below(NVARS));
                default: return new NegE(Build(rnd, 0));
            }
        }
        // Multiplication only near the leaves, so the value of the tree stays finite.
        long k = rnd.Below(d <= 3 ? 3 : 2);
        var a = Build(rnd, d - 1);
        var b = Build(rnd, d - 1);
        switch (k) {
            case 0: return new AddE(a, b);
            case 1: return new SubE(a, b);
            default: return new MulE(a, b);
        }
    }

    public static double Run() {
        var rnd = new Rnd(4242);
        var tree = Build(rnd, DEPTH);
        var env = new double[NVARS];
        double total = 0.0;
        for (int r = 0; r < ROUNDS; r++) {
            for (int k = 0; k < NVARS; k++) env[k] = ((r * 7 + k * 13) % 101) / 50.0 - 1.0;
            total += tree.Eval(env);
        }
        return total;
    }
}

// ------------------------------------------------------------------ 4. sort

static class SortBench {
    const int N = 400000, ROUNDS = 6;

    public struct Rec { public double key; public long id; }

    // The comparison arrives as a delegate, which is what C# code taking one looks like.
    static void SortRec(Rec[] a, long lo, long hi, Func<Rec, Rec, bool> less) {
        if (hi - lo <= 1) return;
        var pivot = a[lo];
        long sp = lo + 1;
        long ep = hi;
        while (sp < ep) {
            var c = a[sp];
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
        SortRec(a, lo, sp, less);
        SortRec(a, ep, hi, less);
    }

    public static double Run() {
        var rnd = new Rnd(777);
        var src = new Rec[N];
        for (long i = 0; i < N; i++) src[i] = new Rec { key = rnd.Range(-1000, 1000), id = i };
        Func<Rec, Rec, bool> less = (a, b) => a.key < b.key || (a.key == b.key && a.id < b.id);
        Rec[] last = src;
        for (int r = 0; r < ROUNDS; r++) {
            var xs = (Rec[])src.Clone();
            SortRec(xs, 0, xs.LongLength, less);
            last = xs;
        }
        for (int i = 1; i < last.Length; i++) {
            if (less(last[i], last[i - 1])) {
                Console.WriteLine("# sort: not sorted at " + i);
                return 0.0;
            }
        }
        double sum = 0.0;
        for (long i = 0; i < last.LongLength; i++) sum += (i % 97 + 1) * last[i].id;
        return sum;
    }
}

// ------------------------------------------------------------------ 5. astar

static class AStar {
    const long W = 512, H = 512;
    const int ROUNDS = 20;
    const double WALL_P = 0.20;
    const long INF = 0x3FFFFFFF;

    struct HeapEnt { public long f, n; }

    static bool Before(HeapEnt a, HeapEnt b) => a.f < b.f || (a.f == b.f && a.n < b.n);

    static void HPush(List<HeapEnt> heap, HeapEnt e) {
        heap.Add(e);
        long i = heap.Count - 1;
        while (i > 0) {
            long p = (i - 1) >> 1;
            var pe = heap[(int)p];
            if (!Before(e, pe)) break;
            heap[(int)i] = pe;
            i = p;
        }
        heap[(int)i] = e;
    }

    static HeapEnt HPop(List<HeapEnt> heap) {
        var top = heap[0];
        var last = heap[heap.Count - 1];
        heap.RemoveAt(heap.Count - 1);
        long n = heap.Count;
        if (n > 0) {
            long i = 0;
            for (;;) {
                long l = (i << 1) + 1;
                if (l >= n) break;
                long c = l;
                long r = l + 1;
                if (r < n && Before(heap[(int)r], heap[(int)l])) c = r;
                var ce = heap[(int)c];
                if (!Before(ce, last)) break;
                heap[(int)i] = ce;
                i = c;
            }
            heap[(int)i] = last;
        }
        return top;
    }

    static void Solve(uint seed, out long cost, out long expanded) {
        var rnd = new Rnd(seed);
        long cells = W * H;
        var grid = new long[cells];
        for (long i = 0; i < cells; i++) grid[i] = rnd.Nextf() < WALL_P ? 1 : 0;
        long goal = cells - 1;
        grid[0] = 0; grid[1] = 0; grid[W] = 0;
        grid[goal] = 0; grid[goal - 1] = 0; grid[goal - W] = 0;
        var gscore = new long[cells];
        for (long i = 0; i < cells; i++) gscore[i] = INF;
        var closed = new long[cells];
        var heap = new List<HeapEnt>();
        var dx = new long[] { 1, 0, -1, 0 };
        var dy = new long[] { 0, 1, 0, -1 };
        long gx = W - 1, gy = H - 1;
        gscore[0] = 0;
        HPush(heap, new HeapEnt { f = gx + gy, n = 0 });
        expanded = 0;
        cost = -1;
        while (heap.Count > 0) {
            var e = HPop(heap);
            long n = e.n;
            if (closed[n] == 0) {
                closed[n] = 1;
                expanded++;
                if (n == goal) { cost = gscore[n]; break; }
                long y = n / W;
                long x = n - y * W;
                long g = gscore[n] + 1;
                for (int k = 0; k < 4; k++) {
                    long nx = x + dx[k];
                    long ny = y + dy[k];
                    if (nx >= 0 && ny >= 0 && nx < W && ny < H) {
                        long nb = n + dy[k] * W + dx[k];
                        if (grid[nb] == 0 && closed[nb] == 0 && g < gscore[nb]) {
                            gscore[nb] = g;
                            HPush(heap, new HeapEnt { f = g + (gx - nx) + (gy - ny), n = nb });
                        }
                    }
                }
            }
        }
    }

    public static double Run() {
        long sum = 0;
        for (int r = 0; r < ROUNDS; r++) {
            Solve((uint)(1000 + r), out long cost, out long expanded);
            sum += cost * 1000003 + expanded;
        }
        return (double)sum;
    }
}

// ------------------------------------------------------------------ driver

static class Program {
    static int Main(string[] argv) {
        var benches = new (string, Func<double>)[] {
            ("nbody", NBody.Run),
            ("raytrace", Raytrace.Run),
            ("interp", Interp.Run),
            ("sort", SortBench.Run),
            ("astar", AStar.Run),
        };
        var args = new List<string>();
        foreach (var a in argv) if (a != "--") args.Add(a);
        int repeats = 5;
        if (args.Count > 0 && int.TryParse(args[0], out int n) && n > 0) repeats = n;
        string only = args.Count > 1 ? args[1] : "";
        foreach (var (name, fn) in benches) {
            if (only.Length > 0 && only != name) continue;
            double best = double.MaxValue;
            double checksum = 0.0;
            for (int r = 0; r < repeats; r++) {
                var sw = Stopwatch.StartNew();
                checksum = fn();
                double dt = sw.Elapsed.TotalSeconds;
                if (dt < best) best = dt;
            }
            Console.WriteLine("bench\t" + name + "\t" +
                best.ToString("F7", CultureInfo.InvariantCulture) + "\t" +
                checksum.ToString("R", CultureInfo.InvariantCulture));
        }
        return 0;
    }
}
