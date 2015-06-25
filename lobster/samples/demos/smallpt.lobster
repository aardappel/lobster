/* translated from http://www.kevinbeason.com/smallpt/

made it incremental, i.e. it shows what it has rendered sofar after each sample for the whole scene, so the longer you
wait, the prettier the image will get!

It takes forever to run in an interpreted language, so just let it sit there for at least a few hrs if you want a pretty
picture, or turn the resolution way down.
It's not a great example to show of Lobster, this kind of code really belongs in C++, but maybe useful as a benchmark.

The variables w and h below change the size of the image... don't increase unless you feel like running it overnight :)

I timed it to be 2.5 times faster than the Python version, though most of that will be due to the builtin vector ops.

*/

include "vec.lobster"
include "color.lobster"

value Ray { o:xyz_f, d:xyz_f }

enum + DIFF, SPEC, REFR   // material types, used in radiance()

value Sphere {
    rad:float,                 // radius
    p:xyz_f, e:xyz_f, c:xyz_f, // position, emission, color
    refl:int                   // reflection type (DIFFuse, SPECular, REFRactive)
}

def intersect(sphere::Sphere, r:Ray):  // returns distance, 0 if nohit
    op := p-r.o   // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
    eps := 0.0001
    b := op.dot(r.d)
    det := b*b-op.dot(op)+rad*rad
    //print(op + " " + b + " " + det)
    if(det<0): return 0.0
    det = sqrt(det)
    t := b-det
    if(t>eps):
        t
    else:
        t=b+det
        if(t>eps): t else: 0.0

// made the radiusses of some spheres smaller compared to the original (and some other adjustments), as we use floats,
// not doubles. walls may look rounder :)
bigrad := 1000.0
lrad := 100.0

spheres := [ //Scene: radius, position, emission, color, material
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
    t := 1000000000000.0                            // distance to intersection
    id := -1                             // id of intersected object
    def intersectray(r:Ray):
        for(spheres) s, i:
            d := s.intersect(r)
            if(d != 0 & d<t):
                t = d
                id = i
        return id >= 0
    if(!intersectray(r)): return xyz_0 // if miss, return black
    obj := spheres[id]        // the hit object
    x := r.o+r.d*t
    n := normalize(x-obj.p)
    nl := if(n.dot(r.d)<0): n else: n*-1
    f := obj.c
    p := if(f.x>f.y & f.x>f.z): f.x else: if(f.y>f.z): f.y else: f.z // max refl
    if(++depth>5): if(rndfloat()<p): f = f*(1/p) else: return obj.e  //R.R.
    if(obj.refl == DIFF):                  // Ideal DIFFUSE reflection
        r1 := 360*rndfloat()
        r2 := rndfloat()
        r2s := sqrt(r2)
        w := nl
        u := normalize((if(abs(w.x)>.1): xyz_y else: xyz_x).cross(w))
        v := w.cross(u)
        d := normalize(u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1-r2))
        return obj.e + f * radiance(Ray { x, d },depth)
    else: if(obj.refl == SPEC):            // Ideal SPECULAR reflection
        return obj.e + f * radiance(Ray { x, r.d-n*2*n.dot(r.d) },depth)
    reflRay := Ray {x, r.d-n*2*n.dot(r.d)}     // Ideal dielectric REFRACTION
    into := n.dot(nl)>0                // Ray from outside going in?
    nc := 1.0
    nt := 1.5
    nnt := if(into): nc/nt else: nt/nc
    ddn := r.d.dot(nl)
    cos2t := 1-nnt*nnt*(1-ddn*ddn)
    if(cos2t<0):    // Total internal reflection
        return obj.e + f*radiance(reflRay,depth)
    tdir := normalize(r.d*nnt - n*((if(into): 1 else: -1)*(ddn*nnt+sqrt(cos2t))))
    a := nt-nc
    b := nt+nc
    R0 := a*a/(b*b)
    c := 1-(if(into): -ddn else: tdir.dot(n))
    Re := R0+(1-R0)*c*c*c*c*c
    Tr := 1-Re
    P := .25+.5*Re
    RP := Re/P
    TP := Tr/(1-P)
    temp :=
        if(depth>2):
            if(rndfloat()<P):
                radiance(reflRay,depth)*RP  // Russian roulette
            else:
                radiance(Ray { x, tdir },depth)*TP
        else:
            radiance(reflRay,depth)*Re+radiance(Ray { x, tdir },depth)*Tr
    obj.e + f*temp

w := 128
h := 96

cam := Ray { xyz { 50.0,50.0,290.0 }, normalize(xyz { 0.0,-0.042612,-1.0 }) } // cam pos, dir
cx := xyz_x * (w*.5135/h)
cy := normalize(cx.cross(cam.d))*.5135

set_max_stack_size(128)

c := map(h): map(w): xyz_0

samples := 0

def onesample():
    starttime := seconds_elapsed()
    for(h) y:                       // Loop over image rows
        for(w) x:                           // Loop cols
            r1 := 2*rndfloat()                  // removed the 2x2 subpixel sampling, its slow enough as it is :)
            dx := if(r1<1): sqrt(r1)-1 else: 1-sqrt(2-r1)
            r2 := 2*rndfloat()
            dy := if(r2<1): sqrt(r2)-1 else: 1-sqrt(2-r2)
            d := cx*( ( (1 + dx)/2 + x)/w - .5) +
                 cy*( ( (1 + dy)/2 + y)/h - .5) + cam.d
            d = normalize(d)
            r := radiance(Ray { cam.o+d*140, d },0)
            // Camera rays are pushed ^^^^^ forward to start in interior
            c[h-y-1][x] += r
    samples++
    c2 := map(c): map(_): map(_): pow(clamp(_ / samples, 0, 1),1/2.2)
    print("sample " + samples + " took " + (seconds_elapsed() - starttime) + " seconds")
    gl_windowtitle("smallpt: " + samples + " samples computed")
    gl_createtexture(c2)

fatal(gl_window("smallpt in lobster!", w, h))

while(gl_frame()):
    if(gl_wentdown("escape")): return
    gl_clear(color_black)
    tex := onesample()
    gl_setprimitivetexture(0, tex)
    gl_blend(0)
    gl_setshader("textured")
    gl_rect(gl_windowsize())
    gl_deletetexture(tex)




