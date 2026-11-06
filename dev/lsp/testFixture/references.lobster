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
