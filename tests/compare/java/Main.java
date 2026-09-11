// Java implementation of the cross-language benchmarks, see ../SPEC.md.
//
// Java has no value types, so V3 is an immutable object and the vector maths depends on
// HotSpot's escape analysis to scalar-replace it. That is the shape a Java program would
// have, and what it costs is one of the things worth measuring.

import java.util.ArrayList;

public class Main {

    // -------------------------------------------------------------- shared

    /** The 32-bit LCG from SPEC.md. A Java int wraps at 32 bits, which is the modulo it asks
     *  for; the value is widened to a long where it has to be read as unsigned. */
    static final class Rnd {
        int state;
        Rnd(int seed) { state = seed; }
        long next() { state = state * 1664525 + 1013904223; return state & 0xFFFFFFFFL; }
        double nextf() { return next() * (1.0 / 4294967296.0); }
        double range(double a, double b) { return a + nextf() * (b - a); }
        long below(long n) { return next() % n; }
    }

    static final class V3 {
        final double x, y, z;
        V3(double x, double y, double z) { this.x = x; this.y = y; this.z = z; }
        V3 add(V3 o) { return new V3(x + o.x, y + o.y, z + o.z); }
        V3 sub(V3 o) { return new V3(x - o.x, y - o.y, z - o.z); }
        V3 mul(double s) { return new V3(x * s, y * s, z * s); }
    }

    static final V3 ZERO = new V3(0, 0, 0);
    static final V3 UP = new V3(0, 1, 0);

    static double dot(V3 a, V3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

    static V3 cross(V3 a, V3 b) {
        return new V3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }

    static V3 normalize(V3 v) {
        double l = Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        return l == 0.0 ? ZERO : new V3(v.x / l, v.y / l, v.z / l);
    }

    // -------------------------------------------------------------- 1. nbody

    static double benchNbody() {
        final int N = 256, STEPS = 1000;
        final double DT = 0.001, EPS2 = 0.5;
        Rnd rnd = new Rnd(12345);
        V3[] pos = new V3[N];
        double[] mass = new double[N];
        V3[] vel = new V3[N];
        for (int i = 0; i < N; i++) {
            V3 p = new V3(rnd.range(-40, 40), rnd.range(-40, 40), rnd.range(-40, 40));
            V3 v = new V3(rnd.range(-0.5, 0.5), rnd.range(-0.5, 0.5), rnd.range(-0.5, 0.5));
            pos[i] = p;
            mass[i] = rnd.range(0.2, 1.2);
            vel[i] = v;
        }
        V3[] acc = new V3[N];
        for (int s = 0; s < STEPS; s++) {
            for (int i = 0; i < N; i++) {
                V3 pi = pos[i];
                V3 a = ZERO;
                for (int j = 0; j < N; j++) {
                    V3 d = pos[j].sub(pi);
                    double r2 = dot(d, d) + EPS2;
                    // The j == i term has d == 0, so it contributes exactly zero.
                    a = a.add(d.mul(mass[j] / (r2 * Math.sqrt(r2))));
                }
                acc[i] = a;
            }
            for (int i = 0; i < N; i++) {
                V3 v = vel[i].add(acc[i].mul(DT));
                vel[i] = v;
                pos[i] = pos[i].add(v.mul(DT));
            }
        }
        double e = 0.0;
        for (int i = 0; i < N; i++) e += 0.5 * mass[i] * dot(vel[i], vel[i]);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (j > i) {
                    V3 d = pos[j].sub(pos[i]);
                    e -= mass[i] * mass[j] / Math.sqrt(dot(d, d) + EPS2);
                }
            }
        }
        return e;
    }

    // -------------------------------------------------------------- 2. raytrace

    static final int RT_W = 640, RT_H = 480, NSPHERES = 48, MAX_DEPTH = 3;
    static final double EPS = 0.0001;
    /** tan(27.5 degrees). */
    static final double HALF = 0.5205670505517462;
    static final double AMBIENT = 0.12;

    static final class Sphere {
        final V3 c; final double r; final V3 col; final double refl;
        Sphere(V3 c, double r, V3 col, double refl) {
            this.c = c; this.r = r; this.col = col; this.refl = refl;
        }
    }

    static final class Hit {
        final double t; final V3 n; final V3 albedo; final double refl;
        Hit(double t, V3 n, V3 albedo, double refl) {
            this.t = t; this.n = n; this.albedo = albedo; this.refl = refl;
        }
    }

    static double hitSphere(Sphere s, V3 ro, V3 rd) {
        V3 oc = s.c.sub(ro);
        double b = dot(oc, rd);
        double c = dot(oc, oc) - s.r * s.r;
        double disc = b * b - c;
        if (!(disc > 0.0)) return -1.0;
        double sd = Math.sqrt(disc);
        double t = b - sd;
        if (t > EPS) return t;
        double t2 = b + sd;
        return t2 > EPS ? t2 : -1.0;
    }

    static boolean anyHit(Sphere[] spheres, V3 ro, V3 rd) {
        for (Sphere s : spheres) {
            if (hitSphere(s, ro, rd) > 0.0) return true;
        }
        return false;
    }

    static Hit nearest(Sphere[] spheres, V3 ro, V3 rd) {
        Hit best = new Hit(-1.0, ZERO, ZERO, 0.0);
        for (Sphere s : spheres) {
            double t = hitSphere(s, ro, rd);
            if (t > 0.0 && (best.t < 0.0 || t < best.t)) {
                best = new Hit(t, normalize(ro.add(rd.mul(t)).sub(s.c)), s.col, s.refl);
            }
        }
        if (rd.y < -1.0e-9) {
            double t = -ro.y / rd.y;
            if (t > EPS && (best.t < 0.0 || t < best.t)) {
                V3 p = ro.add(rd.mul(t));
                long fx = (long) Math.floor(p.x);
                long fz = (long) Math.floor(p.z);
                double shade = (fx + fz) % 2 == 0 ? 0.9 : 0.35;
                best = new Hit(t, UP, new V3(shade, shade, shade), 0.25);
            }
        }
        return best;
    }

    static V3 trace(Sphere[] spheres, V3 light, V3 ro, V3 rd, int depth) {
        Hit h = nearest(spheres, ro, rd);
        if (!(h.t > 0.0)) {
            double sk = rd.y > 0.0 ? rd.y : 0.0;
            return new V3(0.35, 0.52, 0.85).mul(0.4 + 0.6 * sk);
        }
        V3 p = ro.add(rd.mul(h.t));
        V3 off = p.add(h.n.mul(EPS));
        double lit = anyHit(spheres, off, light) ? 0.0 : 1.0;
        double nl = dot(h.n, light);
        if (nl < 0.0) nl = 0.0;
        V3 shaded = h.albedo.mul(AMBIENT + nl * lit);
        if (!(h.refl > 0.0 && depth > 0)) return shaded;
        V3 refd = rd.sub(h.n.mul(2.0 * dot(h.n, rd)));
        return shaded.mul(1.0 - h.refl)
                .add(trace(spheres, light, off, refd, depth - 1).mul(h.refl));
    }

    static double benchRaytrace() {
        Rnd rnd = new Rnd(98765);
        Sphere[] spheres = new Sphere[NSPHERES];
        for (int i = 0; i < NSPHERES; i++) {
            V3 c = new V3(rnd.range(-6, 6), rnd.range(0.4, 4.5), rnd.range(-6, 6));
            double r = rnd.range(0.35, 0.9);
            V3 col = new V3(rnd.range(0.2, 1.0), rnd.range(0.2, 1.0), rnd.range(0.2, 1.0));
            spheres[i] = new Sphere(c, r, col, i % 3 == 0 ? 0.55 : 0.0);
        }
        V3 light = normalize(new V3(-0.6, 1.0, 0.45));
        V3 eye = new V3(0.0, 2.6, 9.0);
        V3 fwd = normalize(new V3(0.0, 1.2, 0.0).sub(eye));
        V3 right = normalize(cross(fwd, UP));
        V3 camUp = cross(right, fwd);
        double aspect = (double) RT_W / (double) RT_H;
        double sum = 0.0;
        for (int py = 0; py < RT_H; py++) {
            double v = 1.0 - ((double) py + 0.5) / (double) RT_H * 2.0;
            for (int px = 0; px < RT_W; px++) {
                double u = ((double) px + 0.5) / (double) RT_W * 2.0 - 1.0;
                V3 rd = normalize(fwd.add(right.mul(u * aspect * HALF)).add(camUp.mul(v * HALF)));
                V3 col = trace(spheres, light, eye, rd, MAX_DEPTH);
                sum += col.x + col.y + col.z;
            }
        }
        return sum;
    }

    // -------------------------------------------------------------- 3. interp

    static final int IP_DEPTH = 14, IP_ROUNDS = 1500, NVARS = 8;

    abstract static class Expr {
        abstract double eval(double[] env);
    }
    static final class ConstE extends Expr {
        final double v;
        ConstE(double v) { this.v = v; }
        double eval(double[] env) { return v; }
    }
    static final class VarE extends Expr {
        final int i;
        VarE(int i) { this.i = i; }
        double eval(double[] env) { return env[i]; }
    }
    static final class NegE extends Expr {
        final Expr a;
        NegE(Expr a) { this.a = a; }
        double eval(double[] env) { return -a.eval(env); }
    }
    static final class AddE extends Expr {
        final Expr a, b;
        AddE(Expr a, Expr b) { this.a = a; this.b = b; }
        double eval(double[] env) { return a.eval(env) + b.eval(env); }
    }
    static final class SubE extends Expr {
        final Expr a, b;
        SubE(Expr a, Expr b) { this.a = a; this.b = b; }
        double eval(double[] env) { return a.eval(env) - b.eval(env); }
    }
    static final class MulE extends Expr {
        final Expr a, b;
        MulE(Expr a, Expr b) { this.a = a; this.b = b; }
        double eval(double[] env) { return a.eval(env) * b.eval(env); }
    }

    static Expr build(Rnd rnd, int d) {
        if (d == 0) {
            long k = rnd.below(3);
            if (k == 0) return new ConstE(rnd.range(-1.0, 1.0));
            if (k == 1) return new VarE((int) rnd.below(NVARS));
            return new NegE(build(rnd, 0));
        }
        // Multiplication only near the leaves, so the value of the tree stays finite.
        long k = rnd.below(d <= 3 ? 3 : 2);
        Expr a = build(rnd, d - 1);
        Expr b = build(rnd, d - 1);
        if (k == 0) return new AddE(a, b);
        if (k == 1) return new SubE(a, b);
        return new MulE(a, b);
    }

    static double benchInterp() {
        Rnd rnd = new Rnd(4242);
        Expr tree = build(rnd, IP_DEPTH);
        double[] env = new double[NVARS];
        double total = 0.0;
        for (int r = 0; r < IP_ROUNDS; r++) {
            for (int k = 0; k < NVARS; k++) env[k] = ((r * 7 + k * 13) % 101) / 50.0 - 1.0;
            total += tree.eval(env);
        }
        return total;
    }

    // -------------------------------------------------------------- 4. sort

    static final int SORT_N = 400000, SORT_ROUNDS = 6;

    static final class Rec {
        final double key; final long id;
        Rec(double key, long id) { this.key = key; this.id = id; }
    }

    interface Less {
        boolean lt(Rec a, Rec b);
    }

    /** The comparison arrives as a functional interface, which is what Java code taking one
     *  looks like; whether it devirtualises is up to the JIT. */
    static void sortRec(Rec[] a, int lo, int hi, Less less) {
        if (hi - lo <= 1) return;
        Rec pivot = a[lo];
        int sp = lo + 1;
        int ep = hi;
        while (sp < ep) {
            Rec c = a[sp];
            if (less.lt(c, pivot)) {
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
        sortRec(a, lo, sp, less);
        sortRec(a, ep, hi, less);
    }

    static double benchSort() {
        Rnd rnd = new Rnd(777);
        Rec[] src = new Rec[SORT_N];
        for (int i = 0; i < SORT_N; i++) src[i] = new Rec(rnd.range(-1000, 1000), i);
        Less less = (a, b) -> a.key < b.key || (a.key == b.key && a.id < b.id);
        Rec[] last = src;
        for (int r = 0; r < SORT_ROUNDS; r++) {
            Rec[] xs = src.clone();
            sortRec(xs, 0, xs.length, less);
            last = xs;
        }
        for (int i = 1; i < last.length; i++) {
            if (less.lt(last[i], last[i - 1])) {
                System.out.println("# sort: not sorted at " + i);
                return 0.0;
            }
        }
        double sum = 0.0;
        for (int i = 0; i < last.length; i++) sum += (long) (i % 97 + 1) * last[i].id;
        return sum;
    }

    // -------------------------------------------------------------- 5. astar

    static final long AS_W = 512, AS_H = 512;
    static final int AS_ROUNDS = 20;
    static final double WALL_P = 0.20;
    static final long INF = 0x3FFFFFFF;

    /** The heap is two parallel arrays rather than an array of pairs, because Java cannot put
     *  a pair in an array without boxing it. */
    static final class Heap {
        long[] f = new long[1024];
        long[] n = new long[1024];
        int size = 0;

        boolean before(long af, long an, long bf, long bn) {
            return af < bf || (af == bf && an < bn);
        }

        void push(long ef, long en) {
            if (size == f.length) {
                long[] nf = new long[size * 2];
                long[] nn = new long[size * 2];
                System.arraycopy(f, 0, nf, 0, size);
                System.arraycopy(n, 0, nn, 0, size);
                f = nf;
                n = nn;
            }
            int i = size++;
            while (i > 0) {
                int p = (i - 1) >> 1;
                if (!before(ef, en, f[p], n[p])) break;
                f[i] = f[p];
                n[i] = n[p];
                i = p;
            }
            f[i] = ef;
            n[i] = en;
        }

        long popNode;

        long popF() {
            long topF = f[0];
            popNode = n[0];
            size--;
            long lastF = f[size], lastN = n[size];
            if (size > 0) {
                int i = 0;
                for (;;) {
                    int l = (i << 1) + 1;
                    if (l >= size) break;
                    int c = l;
                    int r = l + 1;
                    if (r < size && before(f[r], n[r], f[l], n[l])) c = r;
                    if (!before(f[c], n[c], lastF, lastN)) break;
                    f[i] = f[c];
                    n[i] = n[c];
                    i = c;
                }
                f[i] = lastF;
                n[i] = lastN;
            }
            return topF;
        }
    }

    static long asCost, asExpanded;

    static void solve(int seed) {
        Rnd rnd = new Rnd(seed);
        int cells = (int) (AS_W * AS_H);
        long[] grid = new long[cells];
        for (int i = 0; i < cells; i++) grid[i] = rnd.nextf() < WALL_P ? 1 : 0;
        int goal = cells - 1;
        grid[0] = 0; grid[1] = 0; grid[(int) AS_W] = 0;
        grid[goal] = 0; grid[goal - 1] = 0; grid[goal - (int) AS_W] = 0;
        long[] gscore = new long[cells];
        java.util.Arrays.fill(gscore, INF);
        long[] closed = new long[cells];
        Heap heap = new Heap();
        long[] dx = { 1, 0, -1, 0 };
        long[] dy = { 0, 1, 0, -1 };
        long gx = AS_W - 1, gy = AS_H - 1;
        gscore[0] = 0;
        heap.push(gx + gy, 0);
        long expanded = 0;
        long cost = -1;
        while (heap.size > 0) {
            heap.popF();
            long n = heap.popNode;
            if (closed[(int) n] == 0) {
                closed[(int) n] = 1;
                expanded++;
                if (n == goal) { cost = gscore[(int) n]; break; }
                long y = n / AS_W;
                long x = n - y * AS_W;
                long g = gscore[(int) n] + 1;
                for (int k = 0; k < 4; k++) {
                    long nx = x + dx[k];
                    long ny = y + dy[k];
                    if (nx >= 0 && ny >= 0 && nx < AS_W && ny < AS_H) {
                        long nb = n + dy[k] * AS_W + dx[k];
                        if (grid[(int) nb] == 0 && closed[(int) nb] == 0 && g < gscore[(int) nb]) {
                            gscore[(int) nb] = g;
                            heap.push(g + (gx - nx) + (gy - ny), nb);
                        }
                    }
                }
            }
        }
        asCost = cost;
        asExpanded = expanded;
    }

    static double benchAstar() {
        long sum = 0;
        for (int r = 0; r < AS_ROUNDS; r++) {
            solve(1000 + r);
            sum += asCost * 1000003 + asExpanded;
        }
        return (double) sum;
    }

    // -------------------------------------------------------------- driver

    interface Bench {
        double run();
    }

    public static void main(String[] argv) {
        String[] names = { "nbody", "raytrace", "interp", "sort", "astar" };
        Bench[] fns = {
            Main::benchNbody, Main::benchRaytrace, Main::benchInterp,
            Main::benchSort, Main::benchAstar,
        };
        ArrayList<String> args = new ArrayList<>();
        for (String a : argv) if (!a.equals("--")) args.add(a);
        int repeats = 5;
        if (args.size() > 0) {
            try {
                int n = Integer.parseInt(args.get(0));
                if (n > 0) repeats = n;
            } catch (NumberFormatException e) {
                // Leave the default.
            }
        }
        String only = args.size() > 1 ? args.get(1) : "";
        StringBuilder out = new StringBuilder();
        for (int b = 0; b < names.length; b++) {
            if (!only.isEmpty() && !only.equals(names[b])) continue;
            double best = Double.MAX_VALUE;
            double checksum = 0.0;
            for (int r = 0; r < repeats; r++) {
                long t0 = System.nanoTime();
                checksum = fns[b].run();
                double dt = (System.nanoTime() - t0) / 1.0e9;
                if (dt < best) best = dt;
            }
            out.append("bench\t").append(names[b]).append('\t')
               .append(String.format(java.util.Locale.ROOT, "%.7f", best)).append('\t')
               .append(Double.toString(checksum)).append('\n');
        }
        System.out.print(out);
    }
}
