// standard functions that build on top of the builtin functions useful for almost all programs

def map(xs, fun):
    r := vector_reserve(xs.length)
    for(xs) x, i:
        r.push(fun(x, i))
    r

def filter(xs, fun):
    r := []
    for(xs) x, i:
        if(fun(x, i)):
            r.push(x)
    r

def filterindices(l, f):
    r := []
    for(l) x, i:
        if(f(x, i)):
            r.push(i)
    r

def exists(xs, fun):
    for(xs) x, i:
        if(fun(x, i)):
            return true
    false

def forever(fun): while(true): fun()

def fold(xs, init, fun):
    for(xs): init = fun(init, _)
    init

def reduce(xs, fun):
    assert(xs.length)
    init := xs[0]
    for(xs.length - 1) i: init = fun(init, xs[i + 1])
    init

def connect(xs, fun):
    map(max(0, xs.length - 1)) i: fun(xs[i], xs[i + 1])

def reducerev(xs, fun):
    assert(xs.length)
    init := xs[xs.length - 1]
    for(xs.length - 1) i: init = fun(xs[xs.length - i - 2], init)
    init

def find(xs, fun):
    for(xs) x, i: if(fun(x)): return i
    -1

def sum(xs):     fold(xs, 0): _x + _y
def product(xs): fold(xs, 1): _x * _y

def zip(xs, ys): map(xs.length): [ xs[_], ys[_] ]

def reverse(xs, fun): for(xs.length) i: fun(xs[xs.length - i - 1])
def reverselist(xs): map(xs.length) i: xs[xs.length - i - 1]

def flatten(xs): fold(xs, []): append(_a, _b)

def split(l, f):
    r := [ [], [] ]
    for(l) e: r[f(e)].push(e)
    r

def qsort(xs, lt):
    if(xs.length <= 1):
        xs
    else:
        pivot := xs[0]
        tail := xs.slice(1, -1)
        f1 := tail.filter():  lt(_, pivot)
        f2 := tail.filter(): !lt(_, pivot)
        append(append(qsort(f1, lt), [ pivot ]),
                      qsort(f2, lt))

def qsort_in_place(xs, lt):
    def rec(s, e):
        l := e - s
        if(l > 1):
            pivot := xs[s]
            sp := s + 1
            ep := e
            while(sp < ep):
                c := xs[sp]
                if(lt(c, pivot)):
                    xs[sp - 1] = xs[sp]
                    sp++
                else:
                    xs[sp] = xs[--ep]
                    xs[ep] = c
            xs[--sp] = pivot
            rec(s, sp)
            rec(ep, e)
    rec(0, xs.length)

def insertion_sort(xs, lt):
    for(xs) key, i: if(i):
        j := i
        while(j > 0 & lt(key, xs[j - 1])): xs[j--] = xs[j - 1]
        xs[j] = key

def nest_if(c, nest, with):
    if(c): nest(with)
    else: with()

def return_after(v, f):
    f()
    v

def forbias (num, bias,  fun): for(num): fun(_ + bias)
def forscale(num, scale, fun): for(num): fun(_ * scale)

def forrange    (a, b, fun): forbias(b - a,     a, fun)
def forrangeincl(a, b, fun): forbias(b - a + 1, a, fun)

// HOFs that work on other HOFs:

def collect(hof):
    list := []
    hof(): list.push(_)
    list

// coroutine utility functions:

// generally don't want to use this function, since you'd be better off directly using the HOF the coroutine is based on
def coroutine_for(co, f):
    while(co.active):
        f(co.returnvalue)
        co.resume
    co.returnvalue

// builtin for() made useful as a coroutine.
def cofor(n, f):
    for(n): f(_)
    n

// error checking

def fatal(msg):
    if(msg):
        print("fatal error: " + msg)
        return from program

def check(val, msg):
    if(!val): fatal(msg)
    val

