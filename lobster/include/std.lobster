// standard functions that build on top of the builtin functions useful for almost all programs

function forever(fun): while(true): fun()

function fold(xs, init, fun):
    for(xs): init = fun(init, _)
    init

function reduce(xs, fun):
    assert(xs.length)
    init := xs[0]
    for(xs.length - 1) i: init = fun(init, xs[i + 1])
    init

function connect(xs, fun):
    map(max(0, xs.length - 1)) i: fun(xs[i], xs[i + 1])

function reducerev(xs, fun):
    assert(xs.length)
    init := xs[xs.length - 1]
    for(xs.length - 1) i: init = fun(xs[xs.length - i - 2], init)
    init

function filterindices(l, f):   // FIXME: inefficient, maybe better as a builtin
    filter(map(l) x, i: i): f(l[_])

function find(xs, fun):
    for(xs) x, i: if(fun(x)): return i
    return -1

function sum(xs):     fold(xs, 0): _x + _y
function product(xs): fold(xs, 1): _x * _y

function zip(xs, ys): map(xs.length): [ xs[_], ys[_] ]

function reverse(xs, fun): for(xs.length) i: fun(xs[xs.length - i - 1])
function reverselist(xs): map(xs.length) i: xs[xs.length - i - 1]

function split(l, f):
    r := [ [], [] ]
    for(l) e: r[f(e)].push(e)
    r

function qsort(xs, lt):
    if(xs.length <= 1):
        xs
    else:
        pivot := xs[0]
        tail := xs.slice(1, -1)
        append(append(qsort(tail.filter():  lt(_, pivot), lt), [ pivot ]),
                      qsort(tail.filter(): !lt(_, pivot), lt))

function qsort_in_place(xs, lt):
    function rec(s, e):
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

function insertion_sort(xs, lt):
    for(xs) key, i: if(i):
        j := i
        while(j > 0 & lt(key, xs[j - 1])): xs[j--] = xs[j - 1]
        xs[j] = key
    nil

function nest_if(c, nest, with):
    if(c): nest(with)
    else: with()

function return_after(v, f):
    f()
    v

function forbias (num, bias,  fun): for(num): fun(_ + bias)
function forscale(num, scale, fun): for(num): fun(_ * scale)

function forrange    (a, b, fun): forbias(b - a,     a, fun)
function forrangeincl(a, b, fun): forbias(b - a + 1, a, fun)

function inrange(test, num, bias): test >= bias & test < bias + num

// HOFs that work on other HOFs:

function collect(hof):
    list := []
    hof(): list.push(_)
    list

// coroutine utility functions:

// generally don't want to use this function, since you'd be better off directly using the HOF the coroutine is based on
function coroutine_for(co, f):
    while(co.active):
        f(co.returnvalue)
        co.resume
    co.returnvalue

// error checking

function fatal(msg):
    if(msg):
        print("fatal error: " + msg)
        return from program

function check(val, msg):
    if(!val): fatal(msg) else: val

