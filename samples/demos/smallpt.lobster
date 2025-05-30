/* translated from http://www.kevinbeason.com/smallpt/

made it incremental, i.e. it shows what it has rendered sofar after each sample for the whole scene, so the longer you
wait, the prettier the image will get!

It takes forever to run in an interpreted language, so just let it sit there for at least a few hrs if you want a pretty
picture, or turn the resolution way down.
It's not a great example to show of Lobster, this kind of code really belongs in C++, but maybe useful as a benchmark.

The variables w and h below change the size of the image... don't increase unless you feel like running it overnight :)

I timed it to be 2.5 times faster than the Python version, though most of that will be due to the builtin vector ops.

*/

import vec
import color
import gl
import texture

struct Ray:
    o:float3
    d:float3

enum Material:  // Used in radiance()
    DIFF
    SPEC
    REFR

struct Sphere:
    rad:float   // radius
    p:float3     // position
    e:float3     // emission
    c:float3     // color
    refl:Material

def intersect(sphere::Sphere, r:Ray):  // returns distance, 0 if nohit
    let op = p-r.o   // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
    let eps = 0.0001
    let b = op.dot(r.d)
    var det = b*b-op.dot(op)+rad*rad
    if det<0.0: return 0.0
    det = sqrt(det)
    var t = b-det
    return if t>eps:
        t
    else:
        t=b+det
        if t>eps: t else: 0.0

// made the radiusses of some spheres smaller compared to the original (and some other adjustments), as we use floats,
// not doubles. walls may look rounder :)
let bigrad = 1000.0
let lrad = 100.0

let spheres = [ //Scene: radius, position, emission, color, material
    Sphere { lrad,   float3 { 50.0,lrad+81.6-1.0,81.6  }, float3_1*12.0, float3_0,             DIFF }, //Lite
    Sphere { 16.5,   float3 { 73.0,16.5,78.0         },   float3_0,      float3_1*.999,        REFR }, //Glas
    Sphere { 16.5,   float3 { 27.0,16.5,47.0         },   float3_0,      float3_1*.999,        SPEC }, //Mirr
    Sphere { bigrad, float3 { 50.0,-bigrad+81.6,81.6 },   float3_0,      float3_1*.75,         DIFF }, //Top
    Sphere { bigrad, float3 { 50.0, bigrad, 81.6     },   float3_0,      float3_1*.75,         DIFF }, //Botm
    Sphere { bigrad, float3 { 50.0,40.8,-bigrad+170.0  }, float3_0,      float3_0,             DIFF }, //Frnt
    Sphere { bigrad, float3 { 50.0,40.8, bigrad      },   float3_0,      float3_1*.75,         DIFF }, //Back
    Sphere { bigrad, float3 { -bigrad+99.0,40.8,81.6   }, float3_0,      float3 {.25,.25,.75}, DIFF }, //Rght
    Sphere { bigrad, float3 {  bigrad+1.0, 40.8,81.6   }, float3_0,      float3 {.75,.25,.25}, DIFF }  //Left
]

def radiance(r:Ray, depth) -> float3:
    var t = 1000000000000.0                            // distance to intersection
    var id = -1                             // id of intersected object
    for(spheres) s, i:
        let d = s.intersect(r)
        if d != 0.0 and d<t:
            t = d
            id = i
    if id < 0: return float3_0 // if miss, return black
    let obj = spheres[id]        // the hit object
    let x = r.o+r.d*t
    let n = normalize(x-obj.p)
    let nl = if n.dot(r.d)<0.0: n else: n*-1
    var f = obj.c
    let p = if f.x>f.y and f.x>f.z: f.x elif f.y>f.z: f.y else: f.z // max refl
    if ++depth>5: if rnd_float()<p: f = f*(1.0/p) else: return obj.e  //R.R.
    if obj.refl == DIFF:                  // Ideal DIFFUSE reflection
        let r1 = 360.0*rnd_float()
        let r2 = rnd_float()
        let r2s = sqrt(r2)
        let w = nl
        let u = normalize((if(abs(w.x)>.1): float3_y else: float3_x).cross(w))
        let v = w.cross(u)
        let d = normalize(u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1.0-r2))
        return obj.e + f * radiance(Ray { x, d },depth)
    elif obj.refl == SPEC:            // Ideal SPECULAR reflection
        return obj.e + f * radiance(Ray { x, r.d-n*2.0*n.dot(r.d) },depth)
    let reflRay = Ray {x, r.d-n*2.0*n.dot(r.d)}     // Ideal dielectric REFRACTION
    let into = n.dot(nl)>0.0                // Ray from outside going in?
    let nc = 1.0
    let nt = 1.5
    let nnt = if into: nc/nt else: nt/nc
    let ddn = r.d.dot(nl)
    let cos2t = 1.0-nnt*nnt*(1.0-ddn*ddn)
    if cos2t<0.0:    // Total internal reflection
        return obj.e + f*radiance(reflRay,depth)
    let tdir = normalize(r.d*nnt - n*((if into: 1 else: -1)*(ddn*nnt+sqrt(cos2t))))
    let a = nt-nc
    let b = nt+nc
    let R0 = a*a/(b*b)
    let c = 1.0-(if into: -ddn else: tdir.dot(n))
    let Re = R0+(1.0-R0)*c*c*c*c*c
    let Tr = 1.0-Re
    let P = .25+.5*Re
    let RP = Re/P
    let TP = Tr/(1.0-P)
    let temp =
        if depth>2:
            if rnd_float()<P:
                radiance(reflRay,depth)*RP  // Russian roulette
            else:
                radiance(Ray { x, tdir },depth)*TP
        else:
            radiance(reflRay,depth)*Re+radiance(Ray { x, tdir },depth)*Tr
    return obj.e + f*temp

let w = 64
let h = 48  

let cam = Ray { float3 { 50.0,50.0,290.0 }, normalize(float3 { 0.0,-0.042612,-1.0 }) } // cam pos, dir
let cx = float3_x * (w*.5135/h)
let cy = normalize(cx.cross(cam.d))*.5135

let c = map(h): map(w): float3_0

var samples = 0

def onesample():
    //let starttime = seconds_elapsed()
    for(h) y:                       // Loop over image rows
        for(w) x:                           // Loop cols
            let r1 = 2.0*rnd_float()                  // removed the 2x2 subpixel sampling, its slow enough as it is :)
            let dx = if r1<1.0: sqrt(r1)-1.0 else: 1.0-sqrt(2.0-r1)
            let r2 = 2.0*rnd_float()
            let dy = if r2<1.0: sqrt(r2)-1.0 else: 1.0-sqrt(2.0-r2)
            var d = cx*( ( (1.0 + dx)/2.0 + x)/w - .5) +
                    cy*( ( (1.0 + dy)/2.0 + y)/h - .5) + cam.d
            d = normalize(d)
            let r = radiance(Ray { cam.o+d*140.0, d },0)
            // Camera rays are pushed ^^^^^ forward to start in interior
            c[h-y-1][x] += r
    samples++
    let c2 = map(c) row: map(row) col: vec4(pow(clamp(col / samples, float3_0, float3_1), 1.0 / 2.2), 1.0)
    //print("sample {samples} took {(seconds_elapsed() - starttime)} seconds")
    gl.window_title("smallpt: {samples} samples computed")
    return gl.create_texture(c2)

fatal(gl.window("smallpt in lobster!", w, h))

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    let tex = onesample()
    gl.set_primitive_texture(0, tex)
    gl.blend(blend_none)
    gl.set_shader("textured")
    gl.rect(float2 { w, h })




