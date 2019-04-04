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
import texture

struct Ray { o:xyz_f, d:xyz_f }

enum + DIFF, SPEC, REFR   // material types, used in radiance()

struct Sphere {
    rad:float,                 // radius
    p:xyz_f, e:xyz_f, c:xyz_f, // position, emission, color
    refl:int                   // reflection type (DIFFuse, SPECular, REFRactive)
}

def intersect(sphere::Sphere, r:Ray):  // returns distance, 0 if nohit
    let op = p-r.o   // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
    let eps = 0.0001
    let b = op.dot(r.d)
    var det = b*b-op.dot(op)+rad*rad
    //print(op + " " + b + " " + det)
    if det<0: return 0.0
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
    Sphere { lrad,   xyz { 50.0,lrad+81.6-1,81.6  }, xyz_1*12, xyz_0,             DIFF }, //Lite
    Sphere { 16.5,   xyz { 73.0,16.5,78.0         }, xyz_0,    xyz_1*.999,        REFR }, //Glas
    Sphere { 16.5,   xyz { 27.0,16.5,47.0         }, xyz_0,    xyz_1*.999,        SPEC }, //Mirr
    Sphere { bigrad, xyz { 50.0,-bigrad+81.6,81.6 }, xyz_0,    xyz_1*.75,         DIFF }, //Top
    Sphere { bigrad, xyz { 50.0, bigrad, 81.6     }, xyz_0,    xyz_1*.75,         DIFF }, //Botm
    Sphere { bigrad, xyz { 50.0,40.8,-bigrad+170  }, xyz_0,    xyz_0,             DIFF }, //Frnt
    Sphere { bigrad, xyz { 50.0,40.8, bigrad      }, xyz_0,    xyz_1*.75,         DIFF }, //Back
    Sphere { bigrad, xyz { -bigrad+99,40.8,81.6   }, xyz_0,    xyz {.25,.25,.75}, DIFF }, //Rght
    Sphere { bigrad, xyz {  bigrad+1, 40.8,81.6   }, xyz_0,    xyz {.75,.25,.25}, DIFF }  //Left
]

def radiance(r:Ray, depth):
    var t = 1000000000000.0                            // distance to intersection
    var id = -1                             // id of intersected object
    for(spheres) s, i:
        let d = s.intersect(r)
        if d != 0 and d<t:
            t = d
            id = i
    if id < 0: return xyz_0 // if miss, return black
    let obj = spheres[id]        // the hit object
    let x = r.o+r.d*t
    let n = normalize(x-obj.p)
    let nl = if n.dot(r.d)<0: n else: n*-1
    var f = obj.c
    let p = if f.x>f.y and f.x>f.z: f.x else: if f.y>f.z: f.y else: f.z // max refl
    if ++depth>5: if rnd_float()<p: f = f*(1/p) else: return obj.e  //R.R.
    if obj.refl == DIFF:                  // Ideal DIFFUSE reflection
        let r1 = 360*rnd_float()
        let r2 = rnd_float()
        let r2s = sqrt(r2)
        let w = nl
        let u = normalize((if(abs(w.x)>.1): xyz_y else: xyz_x).cross(w))
        let v = w.cross(u)
        let d = normalize(u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1-r2))
        return obj.e + f * radiance(Ray { x, d },depth)
    else: if obj.refl == SPEC:            // Ideal SPECULAR reflection
        return obj.e + f * radiance(Ray { x, r.d-n*2*n.dot(r.d) },depth)
    let reflRay = Ray {x, r.d-n*2*n.dot(r.d)}     // Ideal dielectric REFRACTION
    let into = n.dot(nl)>0                // Ray from outside going in?
    let nc = 1.0
    let nt = 1.5
    let nnt = if into: nc/nt else: nt/nc
    let ddn = r.d.dot(nl)
    let cos2t = 1-nnt*nnt*(1-ddn*ddn)
    if cos2t<0:    // Total internal reflection
        return obj.e + f*radiance(reflRay,depth)
    let tdir = normalize(r.d*nnt - n*((if into: 1 else: -1)*(ddn*nnt+sqrt(cos2t))))
    let a = nt-nc
    let b = nt+nc
    let R0 = a*a/(b*b)
    let c = 1-(if into: -ddn else: tdir.dot(n))
    let Re = R0+(1-R0)*c*c*c*c*c
    let Tr = 1-Re
    let P = .25+.5*Re
    let RP = Re/P
    let TP = Tr/(1-P)
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

let cam = Ray { xyz { 50.0,50.0,290.0 }, normalize(xyz { 0.0,-0.042612,-1.0 }) } // cam pos, dir
let cx = xyz_x * (w*.5135/h)
let cy = normalize(cx.cross(cam.d))*.5135

set_max_stack_size(128)

let c = map(h): map(w): xyz_0

var samples = 0

def onesample():
    let starttime = seconds_elapsed()
    for(h) y:                       // Loop over image rows
        for(w) x:                           // Loop cols
            let r1 = 2*rnd_float()                  // removed the 2x2 subpixel sampling, its slow enough as it is :)
            let dx = if r1<1: sqrt(r1)-1 else: 1-sqrt(2-r1)
            let r2 = 2*rnd_float()
            let dy = if r2<1: sqrt(r2)-1 else: 1-sqrt(2-r2)
            var d = cx*( ( (1 + dx)/2 + x)/w - .5) +
                    cy*( ( (1 + dy)/2 + y)/h - .5) + cam.d
            d = normalize(d)
            let r = radiance(Ray { cam.o+d*140, d },0)
            // Camera rays are pushed ^^^^^ forward to start in interior
            c[h-y-1][x] += r
    samples++
    let c2 = map(c) row: map(row) col: xyzw(pow(clamp(col / samples, xyz_0, xyz_1), 1 / 2.2), 1.0)
    //print("sample " + samples + " took " + (seconds_elapsed() - starttime) + " seconds")
    gl_window_title("smallpt: " + samples + " samples computed")
    return gl_create_texture(c2)

fatal(gl_window("smallpt in lobster!", w, h))

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_black)
    let tex = onesample()
    gl_set_primitive_texture(0, tex)
    gl_blend(blend_none)
    gl_set_shader("textured")
    gl_rect(xy_f { w, h })




