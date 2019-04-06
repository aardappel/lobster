import std

let a, b = 1, 2
// let a, b = 1 -- no longer works (integer literal returns 1 values, 2 needed)

let c = [0, 1, 2, 3]
// let d = c + c -- no longer works (full crash)

let d = map(c): _ + _

let e = def(): 0
let f = def(): 1
let g = e() | f()

let h = 3
let i = switch h:
    case 1: "no"
    case 2, 4..6: "yes"
    default: "maybe"

let j = map(c) val: val * val
let k = filter c: _ > 0
let l = exists c: _ > 0

// now requires an explicit return statement
def fn_a(arg1:int, arg2:int) -> int: return arg1 + arg2
def fn_b(arg1, arg2): return arg1 + arg2

struct s_a { name:string }
let m = s_a { "Lobster" }
def fn_c(arg::s_a): return "Hello, " + name

struct s_b:
    name:string
    fav_color:string
    def stats():
        return "Name: " + name + ", Favorite Color: " + fav_color

let n = s_b { "Judah", "Green" }

// awesome
def fn_d(arg::s_a): return name
def fn_d(arg::s_b): return name

let o = def(): _
let p = o(10)

def fn_g(mul, fun): return fun() * mul
let q = fn_g(2): 3.1415927

let r = def(): _a * _b * _c
let s = r(2, int(q), 4)

def fn_h(): return 1, 2
let t, u = fn_h()

// Tests
assert(a == 1)
assert(b == 2)
assert(equal(c, [0, 1, 2, 3]))
assert(equal(d, [0, 2, 4, 6]))
assert(a | b & -1 == 3) // not exactly sure why this passes
assert(g == 1)
assert(i == "maybe")
assert(equal(j, [0, 1, 4, 9]))
assert(equal(k, [1, 2, 3]))
assert(l == 1)
assert(fn_a(1, 2) == 3)
assert(fn_b(1, 2) == 3)
assert(equal(fn_c(m), "Hello, Lobster"))
assert(equal(n.stats(), "Name: Judah, Favorite Color: Green"))
assert(equal(m.fn_d(), "Lobster"))
assert(equal(n.fn_d(), "Judah"))
assert(p == 10)
assert(q == 6.2831854)
assert(r(1,2,3) == 6)
assert(s == 48)
assert(t == 1)
assert(u == 2)

print("Tests passed!")
