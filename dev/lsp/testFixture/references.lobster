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
