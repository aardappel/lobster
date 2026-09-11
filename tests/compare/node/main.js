// JavaScript implementation of the cross-language benchmarks, see ../SPEC.md.
//
//     node main.js [repeats] [name]
//
// Numbers are doubles, so the LCG is written with an explicit modulo: its largest intermediate
// is under 2^53 and therefore exact. The integer grids use Float64Array rather than Int32Array,
// so that every language holds the same eight bytes per element.

'use strict';

// ------------------------------------------------------------------ shared

class Rnd {
  constructor(seed) { this.state = seed >>> 0; }
  next() {
    this.state = (this.state * 1664525 + 1013904223) % 4294967296;
    return this.state;
  }
  nextf() { return this.next() * (1.0 / 4294967296.0); }
  range(a, b) { return a + this.nextf() * (b - a); }
  below(n) { return this.next() % n; }
}

class V3 {
  constructor(x, y, z) { this.x = x; this.y = y; this.z = z; }
  add(o) { return new V3(this.x + o.x, this.y + o.y, this.z + o.z); }
  sub(o) { return new V3(this.x - o.x, this.y - o.y, this.z - o.z); }
  mul(s) { return new V3(this.x * s, this.y * s, this.z * s); }
}

const ZERO = new V3(0, 0, 0);
const UP = new V3(0, 1, 0);

function dot(a, b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

function cross(a, b) {
  return new V3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

function normalize(v) {
  const l = Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  return l === 0.0 ? ZERO : new V3(v.x / l, v.y / l, v.z / l);
}

// ------------------------------------------------------------------ 1. nbody

function benchNbody() {
  const N = 256, STEPS = 1000, DT = 0.001, EPS2 = 0.5;
  const rnd = new Rnd(12345);
  const pos = new Array(N);
  const mass = new Float64Array(N);
  const vel = new Array(N);
  for (let i = 0; i < N; i++) {
    const p = new V3(rnd.range(-40, 40), rnd.range(-40, 40), rnd.range(-40, 40));
    const v = new V3(rnd.range(-0.5, 0.5), rnd.range(-0.5, 0.5), rnd.range(-0.5, 0.5));
    pos[i] = p;
    mass[i] = rnd.range(0.2, 1.2);
    vel[i] = v;
  }
  const acc = new Array(N).fill(ZERO);
  for (let s = 0; s < STEPS; s++) {
    for (let i = 0; i < N; i++) {
      const pi = pos[i];
      let a = ZERO;
      for (let j = 0; j < N; j++) {
        const d = pos[j].sub(pi);
        const r2 = dot(d, d) + EPS2;
        // The j === i term has d === 0, so it contributes exactly zero.
        a = a.add(d.mul(mass[j] / (r2 * Math.sqrt(r2))));
      }
      acc[i] = a;
    }
    for (let i = 0; i < N; i++) {
      const v = vel[i].add(acc[i].mul(DT));
      vel[i] = v;
      pos[i] = pos[i].add(v.mul(DT));
    }
  }
  let e = 0.0;
  for (let i = 0; i < N; i++) e += 0.5 * mass[i] * dot(vel[i], vel[i]);
  for (let i = 0; i < N; i++) {
    for (let j = 0; j < N; j++) {
      if (j > i) {
        const d = pos[j].sub(pos[i]);
        e -= mass[i] * mass[j] / Math.sqrt(dot(d, d) + EPS2);
      }
    }
  }
  return e;
}

// ------------------------------------------------------------------ 2. raytrace

const RT_W = 640, RT_H = 480, NSPHERES = 48, MAX_DEPTH = 3;
const EPS = 0.0001;
// tan(27.5 degrees).
const HALF = 0.5205670505517462;
const AMBIENT = 0.12;

class Sphere {
  constructor(c, r, col, refl) { this.c = c; this.r = r; this.col = col; this.refl = refl; }
}

class Hit {
  constructor(t, n, albedo, refl) {
    this.t = t; this.n = n; this.albedo = albedo; this.refl = refl;
  }
}

function hitSphere(s, ro, rd) {
  const oc = s.c.sub(ro);
  const b = dot(oc, rd);
  const c = dot(oc, oc) - s.r * s.r;
  const disc = b * b - c;
  if (!(disc > 0.0)) return -1.0;
  const sd = Math.sqrt(disc);
  const t = b - sd;
  if (t > EPS) return t;
  const t2 = b + sd;
  return t2 > EPS ? t2 : -1.0;
}

function anyHit(spheres, ro, rd) {
  for (let i = 0; i < spheres.length; i++) {
    if (hitSphere(spheres[i], ro, rd) > 0.0) return true;
  }
  return false;
}

function nearest(spheres, ro, rd) {
  let best = new Hit(-1.0, ZERO, ZERO, 0.0);
  for (let i = 0; i < spheres.length; i++) {
    const s = spheres[i];
    const t = hitSphere(s, ro, rd);
    if (t > 0.0 && (best.t < 0.0 || t < best.t)) {
      best = new Hit(t, normalize(ro.add(rd.mul(t)).sub(s.c)), s.col, s.refl);
    }
  }
  if (rd.y < -1.0e-9) {
    const t = -ro.y / rd.y;
    if (t > EPS && (best.t < 0.0 || t < best.t)) {
      const p = ro.add(rd.mul(t));
      const fx = Math.floor(p.x);
      const fz = Math.floor(p.z);
      const shade = (fx + fz) % 2 === 0 ? 0.9 : 0.35;
      best = new Hit(t, UP, new V3(shade, shade, shade), 0.25);
    }
  }
  return best;
}

function trace(spheres, light, ro, rd, depth) {
  const h = nearest(spheres, ro, rd);
  if (!(h.t > 0.0)) {
    const sk = rd.y > 0.0 ? rd.y : 0.0;
    return new V3(0.35, 0.52, 0.85).mul(0.4 + 0.6 * sk);
  }
  const p = ro.add(rd.mul(h.t));
  const off = p.add(h.n.mul(EPS));
  const lit = anyHit(spheres, off, light) ? 0.0 : 1.0;
  let nl = dot(h.n, light);
  if (nl < 0.0) nl = 0.0;
  const shaded = h.albedo.mul(AMBIENT + nl * lit);
  if (!(h.refl > 0.0 && depth > 0)) return shaded;
  const refd = rd.sub(h.n.mul(2.0 * dot(h.n, rd)));
  return shaded.mul(1.0 - h.refl).add(trace(spheres, light, off, refd, depth - 1).mul(h.refl));
}

function benchRaytrace() {
  const rnd = new Rnd(98765);
  const spheres = [];
  for (let i = 0; i < NSPHERES; i++) {
    const c = new V3(rnd.range(-6, 6), rnd.range(0.4, 4.5), rnd.range(-6, 6));
    const r = rnd.range(0.35, 0.9);
    const col = new V3(rnd.range(0.2, 1.0), rnd.range(0.2, 1.0), rnd.range(0.2, 1.0));
    spheres.push(new Sphere(c, r, col, i % 3 === 0 ? 0.55 : 0.0));
  }
  const light = normalize(new V3(-0.6, 1.0, 0.45));
  const eye = new V3(0.0, 2.6, 9.0);
  const fwd = normalize(new V3(0.0, 1.2, 0.0).sub(eye));
  const right = normalize(cross(fwd, UP));
  const camUp = cross(right, fwd);
  const aspect = RT_W / RT_H;
  let sum = 0.0;
  for (let py = 0; py < RT_H; py++) {
    const v = 1.0 - (py + 0.5) / RT_H * 2.0;
    for (let px = 0; px < RT_W; px++) {
      const u = (px + 0.5) / RT_W * 2.0 - 1.0;
      const rd = normalize(fwd.add(right.mul(u * aspect * HALF)).add(camUp.mul(v * HALF)));
      const col = trace(spheres, light, eye, rd, MAX_DEPTH);
      sum += col.x + col.y + col.z;
    }
  }
  return sum;
}

// ------------------------------------------------------------------ 3. interp

const IP_DEPTH = 14, IP_ROUNDS = 1500, NVARS = 8;

class ConstE {
  constructor(v) { this.v = v; }
  eval(env) { return this.v; }
}
class VarE {
  constructor(i) { this.i = i; }
  eval(env) { return env[this.i]; }
}
class NegE {
  constructor(a) { this.a = a; }
  eval(env) { return -this.a.eval(env); }
}
class AddE {
  constructor(a, b) { this.a = a; this.b = b; }
  eval(env) { return this.a.eval(env) + this.b.eval(env); }
}
class SubE {
  constructor(a, b) { this.a = a; this.b = b; }
  eval(env) { return this.a.eval(env) - this.b.eval(env); }
}
class MulE {
  constructor(a, b) { this.a = a; this.b = b; }
  eval(env) { return this.a.eval(env) * this.b.eval(env); }
}

function build(rnd, d) {
  if (d === 0) {
    const k = rnd.below(3);
    if (k === 0) return new ConstE(rnd.range(-1.0, 1.0));
    if (k === 1) return new VarE(rnd.below(NVARS));
    return new NegE(build(rnd, 0));
  }
  // Multiplication only near the leaves, so the value of the tree stays finite.
  const k = rnd.below(d <= 3 ? 3 : 2);
  const a = build(rnd, d - 1);
  const b = build(rnd, d - 1);
  if (k === 0) return new AddE(a, b);
  if (k === 1) return new SubE(a, b);
  return new MulE(a, b);
}

function benchInterp() {
  const rnd = new Rnd(4242);
  const tree = build(rnd, IP_DEPTH);
  const env = new Float64Array(NVARS);
  let total = 0.0;
  for (let r = 0; r < IP_ROUNDS; r++) {
    for (let k = 0; k < NVARS; k++) env[k] = ((r * 7 + k * 13) % 101) / 50.0 - 1.0;
    total += tree.eval(env);
  }
  return total;
}

// ------------------------------------------------------------------ 4. sort

const SORT_N = 400000, SORT_ROUNDS = 6;

class Rec {
  constructor(key, id) { this.key = key; this.id = id; }
}

// The comparison arrives as a function value, which is what JavaScript code taking one looks
// like.
function sortRec(a, lo, hi, less) {
  if (hi - lo <= 1) return;
  const pivot = a[lo];
  let sp = lo + 1;
  let ep = hi;
  while (sp < ep) {
    const c = a[sp];
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
  sortRec(a, lo, sp, less);
  sortRec(a, ep, hi, less);
}

function benchSort() {
  const rnd = new Rnd(777);
  const src = new Array(SORT_N);
  for (let i = 0; i < SORT_N; i++) src[i] = new Rec(rnd.range(-1000, 1000), i);
  const less = (a, b) => a.key < b.key || (a.key === b.key && a.id < b.id);
  let last = src;
  for (let r = 0; r < SORT_ROUNDS; r++) {
    const xs = src.slice();
    sortRec(xs, 0, xs.length, less);
    last = xs;
  }
  for (let i = 1; i < last.length; i++) {
    if (less(last[i], last[i - 1])) {
      console.log('# sort: not sorted at ' + i);
      return 0.0;
    }
  }
  let sum = 0.0;
  for (let i = 0; i < last.length; i++) sum += (i % 97 + 1) * last[i].id;
  return sum;
}

// ------------------------------------------------------------------ 5. astar

const AS_W = 512, AS_H = 512, AS_ROUNDS = 20, WALL_P = 0.20, INF = 0x3FFFFFFF;

// The heap is two parallel arrays of numbers, grown by doubling.
class Heap {
  constructor() {
    this.f = new Float64Array(1024);
    this.n = new Float64Array(1024);
    this.size = 0;
    this.popNode = 0;
  }
  before(af, an, bf, bn) { return af < bf || (af === bf && an < bn); }
  push(ef, en) {
    if (this.size === this.f.length) {
      const nf = new Float64Array(this.size * 2);
      const nn = new Float64Array(this.size * 2);
      nf.set(this.f);
      nn.set(this.n);
      this.f = nf;
      this.n = nn;
    }
    let i = this.size++;
    while (i > 0) {
      const p = (i - 1) >> 1;
      if (!this.before(ef, en, this.f[p], this.n[p])) break;
      this.f[i] = this.f[p];
      this.n[i] = this.n[p];
      i = p;
    }
    this.f[i] = ef;
    this.n[i] = en;
  }
  pop() {
    this.popNode = this.n[0];
    this.size--;
    const lastF = this.f[this.size], lastN = this.n[this.size];
    if (this.size > 0) {
      let i = 0;
      for (;;) {
        const l = (i << 1) + 1;
        if (l >= this.size) break;
        let c = l;
        const r = l + 1;
        if (r < this.size && this.before(this.f[r], this.n[r], this.f[l], this.n[l])) c = r;
        if (!this.before(this.f[c], this.n[c], lastF, lastN)) break;
        this.f[i] = this.f[c];
        this.n[i] = this.n[c];
        i = c;
      }
      this.f[i] = lastF;
      this.n[i] = lastN;
    }
  }
}

function solve(seed) {
  const rnd = new Rnd(seed);
  const cells = AS_W * AS_H;
  const grid = new Float64Array(cells);
  for (let i = 0; i < cells; i++) grid[i] = rnd.nextf() < WALL_P ? 1 : 0;
  const goal = cells - 1;
  grid[0] = 0; grid[1] = 0; grid[AS_W] = 0;
  grid[goal] = 0; grid[goal - 1] = 0; grid[goal - AS_W] = 0;
  const gscore = new Float64Array(cells).fill(INF);
  const closed = new Float64Array(cells);
  const heap = new Heap();
  const dx = [1, 0, -1, 0];
  const dy = [0, 1, 0, -1];
  const gx = AS_W - 1, gy = AS_H - 1;
  gscore[0] = 0;
  heap.push(gx + gy, 0);
  let expanded = 0;
  let cost = -1;
  while (heap.size > 0) {
    heap.pop();
    const n = heap.popNode;
    if (closed[n] === 0) {
      closed[n] = 1;
      expanded++;
      if (n === goal) { cost = gscore[n]; break; }
      const y = Math.floor(n / AS_W);
      const x = n - y * AS_W;
      const g = gscore[n] + 1;
      for (let k = 0; k < 4; k++) {
        const nx = x + dx[k];
        const ny = y + dy[k];
        if (nx >= 0 && ny >= 0 && nx < AS_W && ny < AS_H) {
          const nb = n + dy[k] * AS_W + dx[k];
          if (grid[nb] === 0 && closed[nb] === 0 && g < gscore[nb]) {
            gscore[nb] = g;
            heap.push(g + (gx - nx) + (gy - ny), nb);
          }
        }
      }
    }
  }
  return [cost, expanded];
}

function benchAstar() {
  let sum = 0;
  for (let r = 0; r < AS_ROUNDS; r++) {
    const [cost, expanded] = solve(1000 + r);
    sum += cost * 1000003 + expanded;
  }
  return sum;
}

// ------------------------------------------------------------------ driver

const BENCHES = [
  ['nbody', benchNbody],
  ['raytrace', benchRaytrace],
  ['interp', benchInterp],
  ['sort', benchSort],
  ['astar', benchAstar],
];

function main() {
  const args = process.argv.slice(2).filter((a) => a !== '--');
  let repeats = 5;
  if (args.length > 0) {
    const n = parseInt(args[0], 10);
    if (Number.isFinite(n) && n > 0) repeats = n;
  }
  const only = args.length > 1 ? args[1] : '';
  let out = '';
  for (const [name, fn] of BENCHES) {
    if (only && only !== name) continue;
    let best = Infinity;
    let checksum = 0.0;
    for (let r = 0; r < repeats; r++) {
      const t0 = process.hrtime.bigint();
      checksum = fn();
      const dt = Number(process.hrtime.bigint() - t0) / 1e9;
      if (dt < best) best = dt;
    }
    out += `bench\t${name}\t${best.toFixed(7)}\t${checksum}\n`;
  }
  process.stdout.write(out);
}

main();
