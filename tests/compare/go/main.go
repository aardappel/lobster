// Go implementation of the cross-language benchmarks, see ../SPEC.md.
//
//     go build -o go.exe main.go

package main

import (
	"fmt"
	"math"
	"os"
	"strconv"
	"time"
)

// ------------------------------------------------------------------ shared

// Rnd is the 32-bit LCG from SPEC.md. uint32 wraparound is exactly the modulo it asks for.
type Rnd struct {
	state uint32
}

func (r *Rnd) next() uint32 {
	r.state = r.state*1664525 + 1013904223
	return r.state
}

func (r *Rnd) nextf() float64 {
	return float64(r.next()) * (1.0 / 4294967296.0)
}

func (r *Rnd) rng(a, b float64) float64 {
	return a + r.nextf()*(b-a)
}

func (r *Rnd) below(n int64) int64 {
	return int64(r.next()) % n
}

type V3 struct {
	x, y, z float64
}

func add(a, b V3) V3    { return V3{a.x + b.x, a.y + b.y, a.z + b.z} }
func sub(a, b V3) V3    { return V3{a.x - b.x, a.y - b.y, a.z - b.z} }
func scale(a V3, s float64) V3 { return V3{a.x * s, a.y * s, a.z * s} }
func dot(a, b V3) float64 { return a.x*b.x + a.y*b.y + a.z*b.z }
func cross(a, b V3) V3 {
	return V3{a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}
}
func normalize(v V3) V3 {
	l := math.Sqrt(v.x*v.x + v.y*v.y + v.z*v.z)
	if l == 0.0 {
		return V3{0, 0, 0}
	}
	return V3{v.x / l, v.y / l, v.z / l}
}

// ------------------------------------------------------------------ 1. nbody

type Body struct {
	p V3
	m float64
}

func benchNbody() float64 {
	const N = 256
	const STEPS = 1000
	const DT = 0.001
	const EPS2 = 0.5
	rnd := &Rnd{12345}
	bodies := make([]Body, 0, N)
	vel := make([]V3, 0, N)
	for i := 0; i < N; i++ {
		p := V3{rnd.rng(-40, 40), rnd.rng(-40, 40), rnd.rng(-40, 40)}
		v := V3{rnd.rng(-0.5, 0.5), rnd.rng(-0.5, 0.5), rnd.rng(-0.5, 0.5)}
		bodies = append(bodies, Body{p, rnd.rng(0.2, 1.2)})
		vel = append(vel, v)
	}
	acc := make([]V3, N)
	for s := 0; s < STEPS; s++ {
		for i := 0; i < N; i++ {
			pi := bodies[i].p
			a := V3{0, 0, 0}
			for j := 0; j < N; j++ {
				d := sub(bodies[j].p, pi)
				r2 := dot(d, d) + EPS2
				// The j == i term has d == 0, so it contributes exactly zero.
				a = add(a, scale(d, bodies[j].m/(r2*math.Sqrt(r2))))
			}
			acc[i] = a
		}
		for i := 0; i < N; i++ {
			v := add(vel[i], scale(acc[i], DT))
			vel[i] = v
			bodies[i].p = add(bodies[i].p, scale(v, DT))
		}
	}
	e := 0.0
	for i := 0; i < N; i++ {
		e += 0.5 * bodies[i].m * dot(vel[i], vel[i])
	}
	for i := 0; i < N; i++ {
		for j := 0; j < N; j++ {
			if j > i {
				d := sub(bodies[j].p, bodies[i].p)
				e -= bodies[i].m * bodies[j].m / math.Sqrt(dot(d, d)+EPS2)
			}
		}
	}
	return e
}

// ------------------------------------------------------------------ 2. raytrace

const rtW = 640
const rtH = 480
const nSpheres = 48
const maxDepth = 3
const eps = 0.0001

// tan(27.5 degrees).
const half = 0.5205670505517462
const ambient = 0.12

type Sphere struct {
	c    V3
	r    float64
	col  V3
	refl float64
}

type Hit struct {
	t      float64
	n      V3
	albedo V3
	refl   float64
}

func hitSphere(s *Sphere, ro, rd V3) float64 {
	oc := sub(s.c, ro)
	b := dot(oc, rd)
	c := dot(oc, oc) - s.r*s.r
	disc := b*b - c
	if !(disc > 0.0) {
		return -1.0
	}
	sd := math.Sqrt(disc)
	t := b - sd
	if t > eps {
		return t
	}
	t2 := b + sd
	if t2 > eps {
		return t2
	}
	return -1.0
}

func anyHit(spheres []Sphere, ro, rd V3) bool {
	for i := range spheres {
		if hitSphere(&spheres[i], ro, rd) > 0.0 {
			return true
		}
	}
	return false
}

func nearest(spheres []Sphere, ro, rd V3) Hit {
	best := Hit{-1.0, V3{}, V3{}, 0.0}
	for i := range spheres {
		t := hitSphere(&spheres[i], ro, rd)
		if t > 0.0 && (best.t < 0.0 || t < best.t) {
			best = Hit{t, normalize(sub(add(ro, scale(rd, t)), spheres[i].c)),
				spheres[i].col, spheres[i].refl}
		}
	}
	if rd.y < -1.0e-9 {
		t := -ro.y / rd.y
		if t > eps && (best.t < 0.0 || t < best.t) {
			p := add(ro, scale(rd, t))
			fx := int64(math.Floor(p.x))
			fz := int64(math.Floor(p.z))
			shade := 0.35
			if (fx+fz)%2 == 0 {
				shade = 0.9
			}
			best = Hit{t, V3{0, 1, 0}, V3{shade, shade, shade}, 0.25}
		}
	}
	return best
}

func trace(spheres []Sphere, light, ro, rd V3, depth int) V3 {
	h := nearest(spheres, ro, rd)
	if !(h.t > 0.0) {
		s := rd.y
		if s < 0.0 {
			s = 0.0
		}
		return scale(V3{0.35, 0.52, 0.85}, 0.4+0.6*s)
	}
	p := add(ro, scale(rd, h.t))
	off := add(p, scale(h.n, eps))
	lit := 1.0
	if anyHit(spheres, off, light) {
		lit = 0.0
	}
	nl := dot(h.n, light)
	if nl < 0.0 {
		nl = 0.0
	}
	shaded := scale(h.albedo, ambient+nl*lit)
	if !(h.refl > 0.0 && depth > 0) {
		return shaded
	}
	refd := sub(rd, scale(h.n, 2.0*dot(h.n, rd)))
	return add(scale(shaded, 1.0-h.refl), scale(trace(spheres, light, off, refd, depth-1), h.refl))
}

func benchRaytrace() float64 {
	rnd := &Rnd{98765}
	spheres := make([]Sphere, 0, nSpheres)
	for i := 0; i < nSpheres; i++ {
		c := V3{rnd.rng(-6, 6), rnd.rng(0.4, 4.5), rnd.rng(-6, 6)}
		r := rnd.rng(0.35, 0.9)
		col := V3{rnd.rng(0.2, 1.0), rnd.rng(0.2, 1.0), rnd.rng(0.2, 1.0)}
		refl := 0.0
		if i%3 == 0 {
			refl = 0.55
		}
		spheres = append(spheres, Sphere{c, r, col, refl})
	}
	light := normalize(V3{-0.6, 1.0, 0.45})
	eye := V3{0.0, 2.6, 9.0}
	fwd := normalize(sub(V3{0.0, 1.2, 0.0}, eye))
	right := normalize(cross(fwd, V3{0, 1, 0}))
	camUp := cross(right, fwd)
	aspect := float64(rtW) / float64(rtH)
	sum := 0.0
	for py := 0; py < rtH; py++ {
		v := 1.0 - (float64(py)+0.5)/float64(rtH)*2.0
		for px := 0; px < rtW; px++ {
			u := (float64(px)+0.5)/float64(rtW)*2.0 - 1.0
			rd := normalize(add(add(fwd, scale(right, u*aspect*half)), scale(camUp, v*half)))
			col := trace(spheres, light, eye, rd, maxDepth)
			sum += col.x + col.y + col.z
		}
	}
	return sum
}

// ------------------------------------------------------------------ 3. interp

const ipDepth = 14
const ipRounds = 1500
const nVars = 8

// An interface with one method per node type, which is how a Go program would spell this tree.
type Expr interface {
	eval(env []float64) float64
}

type Const struct{ v float64 }
type Var struct{ i int }
type Neg struct{ a Expr }
type Add struct{ a, b Expr }
type Sub struct{ a, b Expr }
type Mul struct{ a, b Expr }

func (e *Const) eval(env []float64) float64 { return e.v }
func (e *Var) eval(env []float64) float64   { return env[e.i] }
func (e *Neg) eval(env []float64) float64   { return -e.a.eval(env) }
func (e *Add) eval(env []float64) float64   { return e.a.eval(env) + e.b.eval(env) }
func (e *Sub) eval(env []float64) float64   { return e.a.eval(env) - e.b.eval(env) }
func (e *Mul) eval(env []float64) float64   { return e.a.eval(env) * e.b.eval(env) }

func build(rnd *Rnd, d int) Expr {
	if d == 0 {
		switch rnd.below(3) {
		case 0:
			return &Const{rnd.rng(-1.0, 1.0)}
		case 1:
			return &Var{int(rnd.below(nVars))}
		default:
			return &Neg{build(rnd, 0)}
		}
	}
	// Multiplication only near the leaves, so the value of the tree stays finite.
	lim := int64(2)
	if d <= 3 {
		lim = 3
	}
	k := rnd.below(lim)
	a := build(rnd, d-1)
	b := build(rnd, d-1)
	switch k {
	case 0:
		return &Add{a, b}
	case 1:
		return &Sub{a, b}
	default:
		return &Mul{a, b}
	}
}

func benchInterp() float64 {
	rnd := &Rnd{4242}
	tree := build(rnd, ipDepth)
	env := make([]float64, nVars)
	total := 0.0
	for r := 0; r < ipRounds; r++ {
		for k := 0; k < nVars; k++ {
			env[k] = float64((r*7+k*13)%101)/50.0 - 1.0
		}
		total += tree.eval(env)
	}
	return total
}

// ------------------------------------------------------------------ 4. sort

const sortN = 400000
const sortRounds = 6

type Rec struct {
	key float64
	id  int64
}

// The comparison arrives as a function value, which is what Go code taking one looks like.
func sortRec(a []Rec, lo, hi int64, less func(x, y Rec) bool) {
	if hi-lo <= 1 {
		return
	}
	pivot := a[lo]
	sp := lo + 1
	ep := hi
	for sp < ep {
		c := a[sp]
		if less(c, pivot) {
			a[sp-1] = a[sp]
			sp++
		} else {
			ep--
			a[sp] = a[ep]
			a[ep] = c
		}
	}
	sp--
	a[sp] = pivot
	sortRec(a, lo, sp, less)
	sortRec(a, ep, hi, less)
}

func benchSort() float64 {
	rnd := &Rnd{777}
	src := make([]Rec, 0, sortN)
	for i := int64(0); i < sortN; i++ {
		src = append(src, Rec{rnd.rng(-1000, 1000), i})
	}
	less := func(a, b Rec) bool { return a.key < b.key || (a.key == b.key && a.id < b.id) }
	var last []Rec
	for r := 0; r < sortRounds; r++ {
		xs := make([]Rec, len(src))
		copy(xs, src)
		sortRec(xs, 0, int64(len(xs)), less)
		last = xs
	}
	for i := 1; i < len(last); i++ {
		if less(last[i], last[i-1]) {
			fmt.Printf("# sort: not sorted at %d\n", i)
			return 0.0
		}
	}
	sum := 0.0
	for i := 0; i < len(last); i++ {
		sum += float64((int64(i)%97 + 1) * last[i].id)
	}
	return sum
}

// ------------------------------------------------------------------ 5. astar

const asW = 512
const asH = 512
const asRounds = 20
const wallP = 0.20
const inf = 0x3FFFFFFF

type HeapEnt struct {
	f, n int64
}

func before(a, b HeapEnt) bool {
	return a.f < b.f || (a.f == b.f && a.n < b.n)
}

func hpush(heap []HeapEnt, e HeapEnt) []HeapEnt {
	heap = append(heap, e)
	i := int64(len(heap)) - 1
	for i > 0 {
		p := (i - 1) >> 1
		pe := heap[p]
		if !before(e, pe) {
			break
		}
		heap[i] = pe
		i = p
	}
	heap[i] = e
	return heap
}

func hpop(heap []HeapEnt) (HeapEnt, []HeapEnt) {
	top := heap[0]
	last := heap[len(heap)-1]
	heap = heap[:len(heap)-1]
	n := int64(len(heap))
	if n > 0 {
		i := int64(0)
		for {
			l := (i << 1) + 1
			if l >= n {
				break
			}
			c := l
			r := l + 1
			if r < n && before(heap[r], heap[l]) {
				c = r
			}
			ce := heap[c]
			if !before(ce, last) {
				break
			}
			heap[i] = ce
			i = c
		}
		heap[i] = last
	}
	return top, heap
}

func solve(seed uint32) (int64, int64) {
	rnd := &Rnd{seed}
	cells := int64(asW * asH)
	grid := make([]int64, 0, cells)
	for i := int64(0); i < cells; i++ {
		if rnd.nextf() < wallP {
			grid = append(grid, 1)
		} else {
			grid = append(grid, 0)
		}
	}
	goal := cells - 1
	grid[0] = 0
	grid[1] = 0
	grid[asW] = 0
	grid[goal] = 0
	grid[goal-1] = 0
	grid[goal-asW] = 0
	gscore := make([]int64, cells)
	for i := range gscore {
		gscore[i] = inf
	}
	closed := make([]int64, cells)
	var heap []HeapEnt
	dx := [4]int64{1, 0, -1, 0}
	dy := [4]int64{0, 1, 0, -1}
	gx := int64(asW - 1)
	gy := int64(asH - 1)
	gscore[0] = 0
	heap = hpush(heap, HeapEnt{gx + gy, 0})
	expanded := int64(0)
	cost := int64(-1)
	for len(heap) > 0 {
		var e HeapEnt
		e, heap = hpop(heap)
		n := e.n
		if closed[n] == 0 {
			closed[n] = 1
			expanded++
			if n == goal {
				cost = gscore[n]
				break
			}
			y := n / asW
			x := n - y*asW
			g := gscore[n] + 1
			for k := 0; k < 4; k++ {
				nx := x + dx[k]
				ny := y + dy[k]
				if nx >= 0 && ny >= 0 && nx < asW && ny < asH {
					nb := n + dy[k]*asW + dx[k]
					if grid[nb] == 0 && closed[nb] == 0 && g < gscore[nb] {
						gscore[nb] = g
						heap = hpush(heap, HeapEnt{g + (gx - nx) + (gy - ny), nb})
					}
				}
			}
		}
	}
	return cost, expanded
}

func benchAstar() float64 {
	sum := int64(0)
	for r := 0; r < asRounds; r++ {
		cost, expanded := solve(uint32(1000 + r))
		sum += cost*1000003 + expanded
	}
	return float64(sum)
}

// ------------------------------------------------------------------ driver

func main() {
	benches := []struct {
		name string
		fn   func() float64
	}{
		{"nbody", benchNbody},
		{"raytrace", benchRaytrace},
		{"interp", benchInterp},
		{"sort", benchSort},
		{"astar", benchAstar},
	}
	args := []string{}
	for _, a := range os.Args[1:] {
		if a != "--" {
			args = append(args, a)
		}
	}
	repeats := 5
	if len(args) > 0 {
		if n, err := strconv.Atoi(args[0]); err == nil && n > 0 {
			repeats = n
		}
	}
	only := ""
	if len(args) > 1 {
		only = args[1]
	}
	for _, b := range benches {
		if only != "" && only != b.name {
			continue
		}
		best := math.MaxFloat64
		checksum := 0.0
		for r := 0; r < repeats; r++ {
			t0 := time.Now()
			checksum = b.fn()
			dt := time.Since(t0).Seconds()
			if dt < best {
				best = dt
			}
		}
		fmt.Printf("bench\t%s\t%.7f\t%s\n", b.name, best,
			strconv.FormatFloat(checksum, 'g', 17, 64))
	}
}
