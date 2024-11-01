import vec

let foo = 1
def f():
    static bar = foo
    bar++
    print bar
def g():
    static bar = foo
    bar++
    print bar
f()
f()
g()

gl.window("dfs", 1000, 1000)

class D:
    i = 1

class C:
    def f():
        member_frame a = 1
        member_frame b = float2_1
        member_frame s = "s"
        def nested():
            member_frame rf = D {}
            return rf
        let r = nested()
        print "c: {a++} {b} {s} {r}"
        b += 1.0
        s += "s"
        r.i++

def freestanding(c::C):
    member_frame a2 = 1
    print "a2: {a2++}"

def aa():
    static_frame a = 1
    static_frame b = float2_1
    static_frame s = "s"
    static_frame r = D {}
    print "a: {a++} {b} {s} {r}"
    b += 1.0
    s += "s"
    r.i++

while gl.frame():
    static c = C {}
    static count = 0
    print "count: {count}"
    if count % 4:
        aa()
        c.f()
        c.freestanding()
    if count++ == 10:
        return
