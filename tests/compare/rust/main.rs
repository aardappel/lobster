// Rust implementation of the cross-language benchmarks, see ../SPEC.md.
//
//     rustc -C opt-level=3 main.rs -o rust.exe

use std::time::Instant;

// ------------------------------------------------------------------ shared

/// The 32-bit LCG from SPEC.md. Wrapping arithmetic is exactly the modulo it asks for.
struct Rnd {
    state: u32,
}

impl Rnd {
    fn new(seed: u32) -> Rnd {
        Rnd { state: seed }
    }
    fn next(&mut self) -> u32 {
        self.state = self.state.wrapping_mul(1664525).wrapping_add(1013904223);
        self.state
    }
    fn nextf(&mut self) -> f64 {
        self.next() as f64 * (1.0 / 4294967296.0)
    }
    fn range(&mut self, a: f64, b: f64) -> f64 {
        a + self.nextf() * (b - a)
    }
    fn below(&mut self, n: i64) -> i64 {
        self.next() as i64 % n
    }
}

#[derive(Clone, Copy)]
struct V3 {
    x: f64,
    y: f64,
    z: f64,
}

impl V3 {
    fn new(x: f64, y: f64, z: f64) -> V3 {
        V3 { x, y, z }
    }
    fn zero() -> V3 {
        V3 { x: 0.0, y: 0.0, z: 0.0 }
    }
}

impl std::ops::Add for V3 {
    type Output = V3;
    fn add(self, o: V3) -> V3 {
        V3::new(self.x + o.x, self.y + o.y, self.z + o.z)
    }
}
impl std::ops::Sub for V3 {
    type Output = V3;
    fn sub(self, o: V3) -> V3 {
        V3::new(self.x - o.x, self.y - o.y, self.z - o.z)
    }
}
impl std::ops::Mul<f64> for V3 {
    type Output = V3;
    fn mul(self, s: f64) -> V3 {
        V3::new(self.x * s, self.y * s, self.z * s)
    }
}

fn dot(a: V3, b: V3) -> f64 {
    a.x * b.x + a.y * b.y + a.z * b.z
}
fn cross(a: V3, b: V3) -> V3 {
    V3::new(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    )
}
fn normalize(v: V3) -> V3 {
    let l = (v.x * v.x + v.y * v.y + v.z * v.z).sqrt();
    if l == 0.0 {
        V3::zero()
    } else {
        V3::new(v.x / l, v.y / l, v.z / l)
    }
}

// ------------------------------------------------------------------ 1. nbody

#[derive(Clone, Copy)]
struct Body {
    p: V3,
    m: f64,
}

fn bench_nbody() -> f64 {
    const N: usize = 256;
    const STEPS: usize = 1000;
    const DT: f64 = 0.001;
    const EPS2: f64 = 0.5;
    let mut rnd = Rnd::new(12345);
    let mut bodies: Vec<Body> = Vec::with_capacity(N);
    let mut vel: Vec<V3> = Vec::with_capacity(N);
    for _ in 0..N {
        let p = V3::new(
            rnd.range(-40.0, 40.0),
            rnd.range(-40.0, 40.0),
            rnd.range(-40.0, 40.0),
        );
        let v = V3::new(rnd.range(-0.5, 0.5), rnd.range(-0.5, 0.5), rnd.range(-0.5, 0.5));
        bodies.push(Body { p, m: rnd.range(0.2, 1.2) });
        vel.push(v);
    }
    let mut acc: Vec<V3> = vec![V3::zero(); N];
    for _ in 0..STEPS {
        for i in 0..N {
            let pi = bodies[i].p;
            let mut a = V3::zero();
            for j in 0..N {
                let d = bodies[j].p - pi;
                let r2 = dot(d, d) + EPS2;
                // The j == i term has d == 0, so it contributes exactly zero.
                a = a + d * (bodies[j].m / (r2 * r2.sqrt()));
            }
            acc[i] = a;
        }
        for i in 0..N {
            let v = vel[i] + acc[i] * DT;
            vel[i] = v;
            bodies[i].p = bodies[i].p + v * DT;
        }
    }
    let mut e = 0.0;
    for i in 0..N {
        e += 0.5 * bodies[i].m * dot(vel[i], vel[i]);
    }
    for i in 0..N {
        for j in 0..N {
            if j > i {
                let d = bodies[j].p - bodies[i].p;
                e -= bodies[i].m * bodies[j].m / (dot(d, d) + EPS2).sqrt();
            }
        }
    }
    e
}

// ------------------------------------------------------------------ 2. raytrace

const RT_W: usize = 640;
const RT_H: usize = 480;
const NSPHERES: usize = 48;
const MAX_DEPTH: i32 = 3;
const EPS: f64 = 0.0001;
/// tan(27.5 degrees).
const HALF: f64 = 0.5205670505517462;
const AMBIENT: f64 = 0.12;

#[derive(Clone, Copy)]
struct Sphere {
    c: V3,
    r: f64,
    col: V3,
    refl: f64,
}

#[derive(Clone, Copy)]
struct Hit {
    t: f64,
    n: V3,
    albedo: V3,
    refl: f64,
}

fn hit_sphere(s: &Sphere, ro: V3, rd: V3) -> f64 {
    let oc = s.c - ro;
    let b = dot(oc, rd);
    let c = dot(oc, oc) - s.r * s.r;
    let disc = b * b - c;
    if !(disc > 0.0) {
        return -1.0;
    }
    let sd = disc.sqrt();
    let t = b - sd;
    if t > EPS {
        return t;
    }
    let t2 = b + sd;
    if t2 > EPS {
        t2
    } else {
        -1.0
    }
}

fn any_hit(spheres: &[Sphere], ro: V3, rd: V3) -> bool {
    for s in spheres {
        if hit_sphere(s, ro, rd) > 0.0 {
            return true;
        }
    }
    false
}

fn nearest(spheres: &[Sphere], ro: V3, rd: V3) -> Hit {
    let mut best = Hit { t: -1.0, n: V3::zero(), albedo: V3::zero(), refl: 0.0 };
    for s in spheres {
        let t = hit_sphere(s, ro, rd);
        if t > 0.0 && (best.t < 0.0 || t < best.t) {
            best = Hit { t, n: normalize(ro + rd * t - s.c), albedo: s.col, refl: s.refl };
        }
    }
    if rd.y < -1.0e-9 {
        let t = -ro.y / rd.y;
        if t > EPS && (best.t < 0.0 || t < best.t) {
            let p = ro + rd * t;
            let fx = p.x.floor() as i64;
            let fz = p.z.floor() as i64;
            let shade = if (fx + fz) % 2 == 0 { 0.9 } else { 0.35 };
            best = Hit {
                t,
                n: V3::new(0.0, 1.0, 0.0),
                albedo: V3::new(shade, shade, shade),
                refl: 0.25,
            };
        }
    }
    best
}

fn trace(spheres: &[Sphere], light: V3, ro: V3, rd: V3, depth: i32) -> V3 {
    let h = nearest(spheres, ro, rd);
    if !(h.t > 0.0) {
        let s = if rd.y > 0.0 { rd.y } else { 0.0 };
        return V3::new(0.35, 0.52, 0.85) * (0.4 + 0.6 * s);
    }
    let p = ro + rd * h.t;
    let off = p + h.n * EPS;
    let lit = if any_hit(spheres, off, light) { 0.0 } else { 1.0 };
    let mut nl = dot(h.n, light);
    if nl < 0.0 {
        nl = 0.0;
    }
    let shaded = h.albedo * (AMBIENT + nl * lit);
    if !(h.refl > 0.0 && depth > 0) {
        return shaded;
    }
    let refd = rd - h.n * (2.0 * dot(h.n, rd));
    shaded * (1.0 - h.refl) + trace(spheres, light, off, refd, depth - 1) * h.refl
}

fn bench_raytrace() -> f64 {
    let mut rnd = Rnd::new(98765);
    let mut spheres: Vec<Sphere> = Vec::with_capacity(NSPHERES);
    for i in 0..NSPHERES {
        let c = V3::new(rnd.range(-6.0, 6.0), rnd.range(0.4, 4.5), rnd.range(-6.0, 6.0));
        let r = rnd.range(0.35, 0.9);
        let col = V3::new(rnd.range(0.2, 1.0), rnd.range(0.2, 1.0), rnd.range(0.2, 1.0));
        spheres.push(Sphere { c, r, col, refl: if i % 3 == 0 { 0.55 } else { 0.0 } });
    }
    let light = normalize(V3::new(-0.6, 1.0, 0.45));
    let eye = V3::new(0.0, 2.6, 9.0);
    let fwd = normalize(V3::new(0.0, 1.2, 0.0) - eye);
    let right = normalize(cross(fwd, V3::new(0.0, 1.0, 0.0)));
    let cam_up = cross(right, fwd);
    let aspect = RT_W as f64 / RT_H as f64;
    let mut sum = 0.0;
    for py in 0..RT_H {
        let v = 1.0 - (py as f64 + 0.5) / RT_H as f64 * 2.0;
        for px in 0..RT_W {
            let u = (px as f64 + 0.5) / RT_W as f64 * 2.0 - 1.0;
            let rd = normalize(fwd + right * (u * aspect * HALF) + cam_up * (v * HALF));
            let col = trace(&spheres, light, eye, rd, MAX_DEPTH);
            sum += col.x + col.y + col.z;
        }
    }
    sum
}

// ------------------------------------------------------------------ 3. interp

const IP_DEPTH: i32 = 14;
const IP_ROUNDS: i32 = 1500;
const NVARS: i64 = 8;

/// An enum with boxed children, which is how a Rust program would spell this tree.
enum Expr {
    Const(f64),
    Var(usize),
    Neg(Box<Expr>),
    Add(Box<Expr>, Box<Expr>),
    Sub(Box<Expr>, Box<Expr>),
    Mul(Box<Expr>, Box<Expr>),
}

fn build(rnd: &mut Rnd, d: i32) -> Box<Expr> {
    if d == 0 {
        return match rnd.below(3) {
            0 => Box::new(Expr::Const(rnd.range(-1.0, 1.0))),
            1 => Box::new(Expr::Var(rnd.below(NVARS) as usize)),
            _ => Box::new(Expr::Neg(build(rnd, 0))),
        };
    }
    // Multiplication only near the leaves, so the value of the tree stays finite.
    let k = rnd.below(if d <= 3 { 3 } else { 2 });
    let a = build(rnd, d - 1);
    let b = build(rnd, d - 1);
    match k {
        0 => Box::new(Expr::Add(a, b)),
        1 => Box::new(Expr::Sub(a, b)),
        _ => Box::new(Expr::Mul(a, b)),
    }
}

fn eval(e: &Expr, env: &[f64]) -> f64 {
    match e {
        Expr::Const(v) => *v,
        Expr::Var(i) => env[*i],
        Expr::Neg(a) => -eval(a, env),
        Expr::Add(a, b) => eval(a, env) + eval(b, env),
        Expr::Sub(a, b) => eval(a, env) - eval(b, env),
        Expr::Mul(a, b) => eval(a, env) * eval(b, env),
    }
}

fn bench_interp() -> f64 {
    let mut rnd = Rnd::new(4242);
    let tree = build(&mut rnd, IP_DEPTH);
    let mut env = [0.0f64; NVARS as usize];
    let mut total = 0.0;
    for r in 0..IP_ROUNDS {
        for k in 0..NVARS as i32 {
            env[k as usize] = ((r * 7 + k * 13) % 101) as f64 / 50.0 - 1.0;
        }
        total += eval(&tree, &env);
    }
    total
}

// ------------------------------------------------------------------ 4. sort

const SORT_N: i64 = 400000;
const SORT_ROUNDS: i32 = 6;

#[derive(Clone, Copy)]
struct Rec {
    key: f64,
    id: i64,
}

/// Generic over the comparison, which is how Rust code that takes one is normally written, so
/// it is monomorphised into the partition loop.
fn sort_rec<F: Fn(&Rec, &Rec) -> bool + Copy>(a: &mut [Rec], lo: i64, hi: i64, less: F) {
    if hi - lo <= 1 {
        return;
    }
    let pivot = a[lo as usize];
    let mut sp = lo + 1;
    let mut ep = hi;
    while sp < ep {
        let c = a[sp as usize];
        if less(&c, &pivot) {
            a[(sp - 1) as usize] = a[sp as usize];
            sp += 1;
        } else {
            ep -= 1;
            a[sp as usize] = a[ep as usize];
            a[ep as usize] = c;
        }
    }
    sp -= 1;
    a[sp as usize] = pivot;
    sort_rec(a, lo, sp, less);
    sort_rec(a, ep, hi, less);
}

fn bench_sort() -> f64 {
    let mut rnd = Rnd::new(777);
    let mut src: Vec<Rec> = Vec::with_capacity(SORT_N as usize);
    for i in 0..SORT_N {
        src.push(Rec { key: rnd.range(-1000.0, 1000.0), id: i });
    }
    let less = |a: &Rec, b: &Rec| a.key < b.key || (a.key == b.key && a.id < b.id);
    let mut last: Vec<Rec> = Vec::new();
    for _ in 0..SORT_ROUNDS {
        let mut xs = src.clone();
        let n = xs.len() as i64;
        sort_rec(&mut xs, 0, n, less);
        last = xs;
    }
    for i in 1..last.len() {
        if less(&last[i], &last[i - 1]) {
            println!("# sort: not sorted at {}", i);
            return 0.0;
        }
    }
    let mut sum = 0.0;
    for i in 0..last.len() {
        sum += ((i as i64 % 97 + 1) * last[i].id) as f64;
    }
    sum
}

// ------------------------------------------------------------------ 5. astar

const AS_W: i64 = 512;
const AS_H: i64 = 512;
const AS_ROUNDS: i32 = 20;
const WALL_P: f64 = 0.20;
const INF: i64 = 0x3FFFFFFF;

#[derive(Clone, Copy)]
struct HeapEnt {
    f: i64,
    n: i64,
}

fn before(a: HeapEnt, b: HeapEnt) -> bool {
    a.f < b.f || (a.f == b.f && a.n < b.n)
}

fn hpush(heap: &mut Vec<HeapEnt>, e: HeapEnt) {
    heap.push(e);
    let mut i = heap.len() as i64 - 1;
    while i > 0 {
        let p = (i - 1) >> 1;
        let pe = heap[p as usize];
        if !before(e, pe) {
            break;
        }
        heap[i as usize] = pe;
        i = p;
    }
    heap[i as usize] = e;
}

fn hpop(heap: &mut Vec<HeapEnt>) -> HeapEnt {
    let top = heap[0];
    let last = heap.pop().unwrap();
    let n = heap.len() as i64;
    if n > 0 {
        let mut i = 0i64;
        loop {
            let l = (i << 1) + 1;
            if l >= n {
                break;
            }
            let mut c = l;
            let r = l + 1;
            if r < n && before(heap[r as usize], heap[l as usize]) {
                c = r;
            }
            let ce = heap[c as usize];
            if !before(ce, last) {
                break;
            }
            heap[i as usize] = ce;
            i = c;
        }
        heap[i as usize] = last;
    }
    top
}

fn solve(seed: u32) -> (i64, i64) {
    let mut rnd = Rnd::new(seed);
    let cells = (AS_W * AS_H) as usize;
    let mut grid: Vec<i64> = Vec::with_capacity(cells);
    for _ in 0..cells {
        grid.push(if rnd.nextf() < WALL_P { 1 } else { 0 });
    }
    let goal = AS_W * AS_H - 1;
    grid[0] = 0;
    grid[1] = 0;
    grid[AS_W as usize] = 0;
    grid[goal as usize] = 0;
    grid[(goal - 1) as usize] = 0;
    grid[(goal - AS_W) as usize] = 0;
    let mut gscore: Vec<i64> = vec![INF; cells];
    let mut closed: Vec<i64> = vec![0; cells];
    let mut heap: Vec<HeapEnt> = Vec::new();
    let dirs: [(i64, i64); 4] = [(1, 0), (0, 1), (-1, 0), (0, -1)];
    let gx = AS_W - 1;
    let gy = AS_H - 1;
    gscore[0] = 0;
    hpush(&mut heap, HeapEnt { f: gx + gy, n: 0 });
    let mut expanded = 0i64;
    let mut cost = -1i64;
    while !heap.is_empty() {
        let e = hpop(&mut heap);
        let n = e.n;
        if closed[n as usize] == 0 {
            closed[n as usize] = 1;
            expanded += 1;
            if n == goal {
                cost = gscore[n as usize];
                break;
            }
            let y = n / AS_W;
            let x = n - y * AS_W;
            let g = gscore[n as usize] + 1;
            for (ddx, ddy) in dirs.iter() {
                let nx = x + ddx;
                let ny = y + ddy;
                if nx >= 0 && ny >= 0 && nx < AS_W && ny < AS_H {
                    let nb = n + ddy * AS_W + ddx;
                    if grid[nb as usize] == 0
                        && closed[nb as usize] == 0
                        && g < gscore[nb as usize]
                    {
                        gscore[nb as usize] = g;
                        hpush(&mut heap, HeapEnt { f: g + (gx - nx) + (gy - ny), n: nb });
                    }
                }
            }
        }
    }
    (cost, expanded)
}

fn bench_astar() -> f64 {
    let mut sum = 0i64;
    for r in 0..AS_ROUNDS {
        let (cost, expanded) = solve((1000 + r) as u32);
        sum += cost * 1000003 + expanded;
    }
    sum as f64
}

// ------------------------------------------------------------------ driver

fn main() {
    let benches: [(&str, fn() -> f64); 5] = [
        ("nbody", bench_nbody),
        ("raytrace", bench_raytrace),
        ("interp", bench_interp),
        ("sort", bench_sort),
        ("astar", bench_astar),
    ];
    let args: Vec<String> = std::env::args().skip(1).filter(|a| a != "--").collect();
    let mut repeats = 5;
    if let Some(a) = args.get(0) {
        if let Ok(n) = a.parse::<i32>() {
            if n > 0 {
                repeats = n;
            }
        }
    }
    let only = args.get(1).cloned().unwrap_or_default();
    for (name, f) in benches.iter() {
        if !only.is_empty() && only != *name {
            continue;
        }
        let mut best = f64::MAX;
        let mut checksum = 0.0;
        for _ in 0..repeats {
            let t0 = Instant::now();
            checksum = f();
            let dt = t0.elapsed().as_secs_f64();
            if dt < best {
                best = dt;
            }
        }
        println!("bench\t{}\t{:.7}\t{}", name, best, checksum);
    }
}
