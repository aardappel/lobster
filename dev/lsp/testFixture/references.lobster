class counter:
    count:int
    def bump():
        count++

enum color:
    red
    green

let c = counter { 0 }
c.bump()
print c.count
print green

class shape:
    area:int

class square : shape
    side:int

class circle : shape
    radius:int

let sq = square { 4, 2 }
let ci = circle { 3, 1 }
print sq.area + sq.side
print ci.area + ci.radius

def apply(v, f): return f(v)

def ends_in_block(v):
    let total = 1
    apply(v) x:
        let more = x + total
        more + 1

def head_on_block(v): apply(v) y:
    let z = y + 1
    z + y

ends_in_block(1)
head_on_block(1)

def default_value(v, g = fn(w): w + 1): return g(v)

default_value(1)

class big : square
    extra:int

class later

class sooner : later
    own:int

class later:
    top:int

let bg = big { 5, 6, 7 }
let so = sooner { 8, 9 }
print bg.area + bg.side + bg.extra
print so.top + so.own
