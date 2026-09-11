// Whitted-style ray tracer, see ../SPEC.md section 2.

import vec
import .rnd

private let W = 640
private let H = 480
private let NSPHERES = 48
private let MAX_DEPTH = 3
private let EPS = 0.0001
// tan(27.5 degrees), spelled out because Lobster's trig takes degrees and the other
// implementations' take radians.
private let HALF = 0.5205670505517462

private struct Sphere:
    c:float3
    r:float
    col:float3
    refl:float

// What a ray found, with a negative t meaning it hit nothing.
private struct Hit:
    t:float
    n:float3
    albedo:float3
    refl:float

private let BACKGROUND = float3 { 0.35, 0.52, 0.85 }
private let LIGHT = normalize(float3 { -0.6, 1.0, 0.45 })
private let AMBIENT = 0.12

private def build_scene() -> [Sphere]:
    let rnd = Rnd { 98765 }
    let spheres = []::Sphere
    for(NSPHERES) i:
        let c = float3 { rnd.range(-6.0, 6.0), rnd.range(0.4, 4.5), rnd.range(-6.0, 6.0) }
        let r = rnd.range(0.35, 0.9)
        let col = float3 { rnd.range(0.2, 1.0), rnd.range(0.2, 1.0), rnd.range(0.2, 1.0) }
        spheres.push(Sphere { c, r, col, if i % 3 == 0: 0.55 else: 0.0 })
    return spheres

// Distance along the ray to this sphere, or a negative value where it misses.
private def hit_sphere(s:Sphere, ro:float3, rd:float3) -> float:
    let oc = s.c - ro
    let b = dot(oc, rd)
    let c = dot(oc, oc) - s.r * s.r
    let disc = b * b - c
    guard disc > 0.0: return -1.0
    let sd = sqrt(disc)
    let t = b - sd
    if t > EPS: return t
    let t2 = b + sd
    return if t2 > EPS: t2 else: -1.0

private def any_hit(spheres:[Sphere], ro:float3, rd:float3) -> bool:
    for(spheres) s:
        if hit_sphere(s, ro, rd) > 0.0: return true
    return false

private def nearest(spheres:[Sphere], ro:float3, rd:float3) -> Hit:
    var best = Hit { -1.0, float3_0, float3_0, 0.0 }
    for(spheres) s:
        let t = hit_sphere(s, ro, rd)
        if t > 0.0 and (best.t < 0.0 or t < best.t):
            best = Hit { t, normalize(ro + rd * t - s.c), s.col, s.refl }
    // The ground plane, which only faces up and so is only hit by downward rays.
    if rd.y < -1.0e-9:
        let t = -ro.y / rd.y
        if t > EPS and (best.t < 0.0 or t < best.t):
            let p = ro + rd * t
            let shade = if (floor(p.x) + floor(p.z)) % 2 == 0: 0.9 else: 0.35
            best = Hit { t, float3_y, float3 { shade, shade, shade }, 0.25 }
    return best

private def trace(spheres:[Sphere], ro:float3, rd:float3, depth:int) -> float3:
    let h = nearest(spheres, ro, rd)
    guard h.t > 0.0:
        return BACKGROUND * (0.4 + 0.6 * max(0.0, rd.y))
    let p = ro + rd * h.t
    let off = p + h.n * EPS
    let lit = if any_hit(spheres, off, LIGHT): 0.0 else: 1.0
    let shaded = h.albedo * (AMBIENT + max(0.0, dot(h.n, LIGHT)) * lit)
    guard h.refl > 0.0 and depth > 0: return shaded
    let refd = rd - h.n * (2.0 * dot(h.n, rd))
    return shaded * (1.0 - h.refl) + trace(spheres, off, refd, depth - 1) * h.refl

def bench_raytrace() -> float:
    let spheres = build_scene()
    let eye = float3 { 0.0, 2.6, 9.0 }
    let fwd = normalize(float3 { 0.0, 1.2, 0.0 } - eye)
    let right = normalize(cross(fwd, float3_y))
    let cam_up = cross(right, fwd)
    let aspect = float(W) / float(H)
    var sum = 0.0
    for(H) py:
        let v = 1.0 - (float(py) + 0.5) / float(H) * 2.0
        for(W) px:
            let u = (float(px) + 0.5) / float(W) * 2.0 - 1.0
            let rd = normalize(fwd + right * (u * aspect * HALF) + cam_up * (v * HALF))
            let col = trace(spheres, eye, rd, MAX_DEPTH)
            sum += col.x + col.y + col.z
    return sum
