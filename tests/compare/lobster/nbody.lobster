// Gravitational n-body with softening, see ../SPEC.md section 1.

import vec
import .rnd

private let N = 256
private let STEPS = 1000
private let DT = 0.001
private let EPS2 = 0.5

// Position and mass together, so the inner loop reads one contiguous element per body and
// never indexes: `for` over a vector needs no bounds check, indexing does.
private struct Body:
    p:float3
    m:float

def bench_nbody() -> float:
    let rnd = Rnd { 12345 }
    let bodies = []::Body
    let vel = []::float3
    for(N):
        let p = float3 { rnd.range(-40.0, 40.0), rnd.range(-40.0, 40.0), rnd.range(-40.0, 40.0) }
        let v = float3 { rnd.range(-0.5, 0.5), rnd.range(-0.5, 0.5), rnd.range(-0.5, 0.5) }
        bodies.push(Body { p, rnd.range(0.2, 1.2) })
        vel.push(v)
    let acc = map(N): float3_0
    for(STEPS):
        for(bodies) bi, i:
            let pi = bi.p
            var a = float3_0
            for(bodies) bj:
                let d = bj.p - pi
                let r2 = dot(d, d) + EPS2
                // The j == i term has d == 0, so it contributes exactly zero and needs no test.
                a += d * (bj.m / (r2 * sqrt(r2)))
            acc[i] = a
        for(bodies) bi, i:
            let v = vel[i] + acc[i] * DT
            vel[i] = v
            bodies[i] = Body { bi.p + v * DT, bi.m }
    var e = 0.0
    for(bodies) bi, i:
        let v = vel[i]
        e += 0.5 * bi.m * dot(v, v)
    for(bodies) bi, i:
        for(bodies) bj, j:
            if j > i:
                let d = bj.p - bi.p
                e -= bi.m * bj.m / sqrt(dot(d, d) + EPS2)
    return e
