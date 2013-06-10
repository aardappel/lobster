/* translated from http://www.kevinbeason.com/smallpt/

made it incremental, i.e. it shows what it has rendered sofar after each sample for the whole scene, so the longer you wait, the prettier the image will get!

It takes forever to run in an interpreted language, so just let it sit there for at least a few hrs if you want a pretty picture, or turn the resolution way down.
It's not a great example to show of Lobster, this kind of code really belongs in C++, but maybe useful as a benchmark.

The variables w and h below change the size of the image... don't increase unless you feel like running it overnight :)

I timed it to be 2.5 times faster than the Python version, though most of that will be due to the builtin vector ops.

*/

include "vec.lobster"
include "color.lobster"

value Ray: [ o, d ]

DIFF := 0   // material types, used in radiance()
SPEC := 1
REFR := 2

value Sphere: [
    rad,       // radius
    p, e, c,   // position, emission, color
    refl       // reflection type (DIFFuse, SPECular, REFRactive)
]

function intersect(sphere::Sphere, r:Ray):  // returns distance, 0 if nohit
    op := p-r.o   // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
    eps := 0.0001
    b := op.dot(r.d)
    det := b*b-op.dot(op)+rad*rad
    //print(op + " " + b + " " + det)
    if(det<0): return 0
    det = sqrt(det)
    t := b-det
    if(t>eps):
        t
    else:
        t=b+det
        if(t>eps): t else: 0

// made the radiusses of some spheres smaller compared to the original (and some other adjustments), as we use floats, not doubles. walls may look rounder :)
bigrad := 1000.0
lrad := 100.0

spheres := [ //Scene: radius, position, emission, color, material
    [lrad,   [50.0,lrad+81.6-1,81.6  ]:xyz, vec_1*12, vec_0,             DIFF]:Sphere, //Lite
    [16.5,   [73.0,16.5,78.0         ]:xyz, vec_0,    vec_1*.999,        REFR]:Sphere, //Glas
    [16.5,   [27.0,16.5,47.0         ]:xyz, vec_0,    vec_1*.999,        SPEC]:Sphere, //Mirr
    [bigrad, [50.0,-bigrad+81.6,81.6 ]:xyz, vec_0,    vec_1*.75,         DIFF]:Sphere, //Top
    [bigrad, [50.0, bigrad, 81.6     ]:xyz, vec_0,    vec_1*.75,         DIFF]:Sphere, //Botm
    [bigrad, [50.0,40.8,-bigrad+170  ]:xyz, vec_0,    vec_0,             DIFF]:Sphere, //Frnt
    [bigrad, [50.0,40.8, bigrad      ]:xyz, vec_0,    vec_1*.75,         DIFF]:Sphere, //Back
    [bigrad, [-bigrad+99,40.8,81.6   ]:xyz, vec_0,    [.25,.25,.75]:xyz, DIFF]:Sphere, //Rght
    [bigrad, [ bigrad+1, 40.8,81.6   ]:xyz, vec_0,    [.75,.25,.25]:xyz, DIFF]:Sphere  //Left
]

function radiance(r:Ray, depth):
    t := 1000000000000.0                            // distance to intersection
    id := -1                             // id of intersected object
    function intersectray(r:Ray):
        for(spheres) s, i:
            d := s.intersect(r)
            if(d != 0 & d<t):
                t = d
                id = i
        return id >= 0
    if(!intersectray(r)): return vec_0 // if miss, return black
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
        u := normalize((if(abs(w.x)>.1): vec_y else: vec_x).cross(w))
        v := w.cross(u)
        d := normalize(u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1-r2))
        return obj.e + f * radiance([ x, d ]:Ray,depth)
    else: if(obj.refl == SPEC):            // Ideal SPECULAR reflection
        return obj.e + f * radiance([ x, r.d-n*2*n.dot(r.d) ]:Ray,depth)
    reflRay := [x, r.d-n*2*n.dot(r.d)]:Ray     // Ideal dielectric REFRACTION
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
                radiance([ x, tdir ]:Ray,depth)*TP
        else:
            radiance(reflRay,depth)*Re+radiance([ x, tdir ]:Ray,depth)*Tr
    obj.e + f*temp
    
w := 128
h := 96

cam := [ [50,50,290 ]:xyz, normalize([0,-0.042612,-1]:xyz) ]:Ray // cam pos, dir
cx := vec_x * (w*.5135/h)
cy := normalize(cx.cross(cam.d))*.5135

set_max_stack_size(128)

c := map(w): map(h): vec_0

samples := 0


function onesample():
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
            r := radiance([ cam.o+d*140, d ]:Ray,0)
            // Camera rays are pushed ^^^^^ forward to start in interior
            c[x][h-y-1] += r
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
    gl_scale(gl_windowsize())
    gl_blend(0)
    gl_setshader("textured")
    gl_rect(vec_1)
    gl_deletetexture(tex)

                


