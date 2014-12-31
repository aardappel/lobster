// this file runs some test cases exercising different parts of the language
// can be included in any lobster program to help verify there are no bugs in the compiler
// will assert if anything is wrong - if this ever happens, contact Wouter

include "std.lobster"
include "exception.lobster"
include "vec.lobster"
include "astar.lobster"

function run_test_cases():
    //trace_bytecode(1)

    value testa: [ a ]
    value testb: testa [ b ]

    function tf(x:int, y): 1
    function tf(x:string, y:float): 2
    function tf(x:string, y:int): 3
    function tf(x:testa, y): 4
    function tf(x:testb, y): 9
    function tf(x:testb, y:float): 5
    function tf(x:testb, y:int): 6
    function tf(x, y): 7

    function tf(x): 8

    assert(tf(1, 0) == 1)
    assert(tf("", 0.0) == 2)
    assert(tf("", 0) == 3)
    assert(tf([ 0 ]:testa, 0) == 4)
    assert(tf([ super [ 0 ]:testa, 0 ]:testb, "") == 9)
    assert(tf([ super [ 0 ]:testa, 0 ]:testb, 0.0) == 5)
    assert(tf([ super [ 0 ]:testa, 0 ]:testb, 0) == 6)
    assert(tf("", "") == 7)

    assert(tf("") == 8)

    direct := [1, 2, [3.0, 4.0, 5.0]:xyz, "hello, world!\n\"\'\r\t\\\xC0", nil, true]
    parsed, err := parse_data("" + direct)
    //print(parsed)
    //print(direct)
    if(err):
        print(err)
    assert(equal(parsed, direct))

    unicodetests := [0x30E6, 0x30FC, 0x30B6, 0x30FC, 0x5225, 0x30B5, 0x30A4, 0x30C8]
    assert(equal(string2unicode(unicode2string(unicodetests)), unicodetests))

    compres1, comperr1 := compile_run_code("1 + 2")
    if(comperr1):
        print(comperr1)
    assert(compres1 == "3")

    /*
    // this test makes it dependent on this file even in shipping builds, so off by default
    compres2, comperr2 := compile_run_file("plugintest.lobster")
    if(comperr2):
        print(comperr2)
    assert(compres2 == "plugintest return value")
    */

    function scopetest():
        x := 1
        y := 1
        function f():
            assert(x == 1)
            assert(y == 2)
        function g():
            x := 2
            y <- 2
            f()
        g()
    scopetest()
    a := 0
    assert(a++ + ++a + a-- + --a == 4)
    b := [0]
    assert(b[0]++ + ++b[0] + b[0]-- + --b[0] == 4)


    testvector := [ 3, 9, 5, 4, 1, 3, 9, 5, 4, 1 ]

    sorted1 := testvector.qsort(): _a < _b
    sorted2 := copy(testvector)
    sorted2.insertion_sort(): _a < _b
    sorted3 := copy(testvector)
    sorted3.qsort_in_place(): _a < _b

    assert(equal(sorted1, [1,1,3,3,4,4,5,5,9,9]))
    assert(equal(sorted1, sorted2))
    assert(equal(sorted1, sorted3))

    found, findex := sorted1.binarysearch(1)
    assert(found == 2 & findex == 0)
    found, findex = sorted1.binarysearch(9)
    assert(found == 2 & findex == 8)
    found, findex = sorted1.binarysearch(3)
    assert(found == 2 & findex == 2)

    assert(44 == sum(testvector))
    assert(264 == sum(testvector.map(): _ * _))


    function factorial(n): 1 > n | factorial(n - 1) * n
    assert(7.factorial == 5040)

    assert(((function(f): f(4))(function(x): x * x)) == 16)


    // ////////////////////////////////////////////////////////////////////////
    // typechecking

    function block(f): f()

    block():
        function h(i): i()
        function g(f):
            // Here, the function passed to h appears the same each time, but it has a freevar f that is different.
            h():
                f()
        assert((g(): 1) == 1)
        assert((g(): "a") == "a")

    // ////////////////////////////////////////////////////////////////////////
    // A* 2D test

    initworld := [
        ".................................",
        "................S................",
        "........#..............#.........",
        "........#..............#/////////",
        "........#..............#.........",
        "........#..............#.........",
        "........################.........",
        ".................................",
        "...............///...............",
        ".............../E/...............",
        "...............///...............",
        "................................."
    ]

    expected_result := [
        "......OXXXXXXXXXXXXXXXXXXXO......",
        ".....OXPPPPPPPPPPXXXXXXXXXXO.....",
        ".....OXP#XXXXXXXXXXXXXX#XXXO.....",
        ".....OXP#XXXXXXXXXXXXXX#XOO//////",
        ".....OXP#XXXXXXXXXXXXXX#XO.......",
        ".....OXP#XXXXXXXXXXXXXX#XO.......",
        ".....OXP################XO.......",
        ".....OXPPPPPPPPPPXXXXXXXXO.......",
        ".....OXXXXXXXXXOPOXXXXXXXO.......",
        ".....OXXXXXXXXXOPOXXXXXXXO.......",
        "......OXXXXXXXXO//OOOOOOO........",
        ".......OOOOOOOO.................."
    ]

    struct pathingcell: astar_node [ path, c ]

    worldsize := [ initworld[0].length, initworld.length ]:xy

    startpos := xy_0i
    endpos := xy_0i

    world := worldsize.y.map() y: worldsize.x.map() x:
        c := initworld[y][x]
        if(c == 'S'): startpos = [ x, y ]:xy
        if(c == 'E'): endpos   = [ x, y ]:xy
        [ super new_astar_node([ x, y ]:xy, 0.0), false, c ]:pathingcell

    path :=
        astar_2dgrid(false, worldsize, world[startpos], world[endpos], function(): world[_]) (n, nn::pathingcell):
            (c == '#' & -1) | (c == '/' & 5) | 1

    assert(path.length == 27)

    for(path) n:
        n.path = true

    astar_result := map(world) row: fold(row, "") accum, n:
        accum + ((n.path   & "P") |
                 (n.closed & "X") |
                 (n.open   & "O") |
                 unicode2string([ n.c ]))

    assert(equal(astar_result, expected_result))


    // ////////////////////////////////////////////////////////////////////////
    // GOAP

    enum wolves, skins, money, flour, bread, pizza, hungry

    goapactions := [
        [ "kill wolf",  function(s): s[wolves] >= 1, function(s): s[wolves]--;   s[skins]++ ]:goapaction,
        [ "sell skin",  function(s): s[skins]  >= 1, function(s): s[money] += 2; s[skins]-- ]:goapaction,
        [ "buy pizza",  function(s): s[money]  >= 2, function(s): s[money] -= 2; s[pizza]++ ]:goapaction,
        [ "buy flour",  function(s): s[money]  >= 1, function(s): s[money]--;    s[flour]++ ]:goapaction,
        [ "bake bread", function(s): s[flour]  >= 1, function(s): s[flour]--;    s[bread]++ ]:goapaction,
        [
            "eat",
            function(s): (s[bread] > 0 | s[pizza] > 0) & s[hungry] > 0,
            function(s):
                if(s[bread] > 0): s[bread]-- else: s[pizza]--
                s[hungry]--
        ]:goapaction
    ]

    goap_path :=
        astar_goap(goapactions, [ 3, 0, 0, 0, 0, 0, 2 ]) state:
            state[hungry]
        endcondition s:
            s[hungry] <= 0

    assert(goap_path.length == 9)
    goap_path.pop()
    assert(equal(goap_path.map(): _.delta?.name | "",
        ["eat","buy pizza","sell skin","kill wolf","eat","buy pizza","sell skin","kill wolf"]))


    // ////////////////////////////////////////////////////////////////////////
    // coroutines test

    /*

    function mycoro(f):
        forrange(3, 6): f(_)
        1337

    co1 := coroutine mycoro()   // out of nested HOFs
    co2 := coroutine for(4)     // out of a builtin function

    function interleave(co1, co2, f):   // test using 2 at once
        while(co1.active | co2.active):
            if(co1.active):
                f(co1.returnvalue)
                co1.resume(true)
            if(co2.active):
                co1, co2 = co2, co1

    //interleave(co1, co2): print(_)

    assert(equal(collect() f: interleave(co1, co2, f), [ 3, 0, 4, 1, 5, 2, 3 ]))
    assert(!co1.active)
    assert(!co2.active)
    assert(co1.returnvalue == 1337)

    function pass_thru(co, f):
        coroutine_for(co): f(_)

    co := coroutine pass_thru(coroutine for(10))    // test consuming 1 coro with another and then passing that on
    sum := 0
    while(co.active):       // we currently can't use coroutine_for here, will be a runtime error (nesting)
        sum += co.returnvalue
        co.resume
    assert(sum == 45)

    function loctest(f):
        a := 1
        for(10) i:
            b := 2
            f()

    co = coroutine loctest()
    assert(co.a@loctest + co.i@loctest + co.b@loctest == 3)

    */

    // ////////////////////////////////////////////////////////////////////////
    // structure offsets test

    struct __testa: [ __a, __b, __c, __d ]
    struct __testb: [ __a, __c, __b, __d ]
    struct __testc: [ __a, __b, __d, __c, __e ]
    struct __testd: [ __a, __b, __d, __e ]
    // should generate conditional warning for b & e, and table warning for c & d in verbose mode

    sot := [ 1, 2, 3, 4, 5 ]:__testc
    assert(equal(sot, [sot.__a, sot.__b, sot.__d, sot.__c, sot.__e]:__testc))

    // ////////////////////////////////////////////////////////////////////////
    // misc test

    aa := 5
    aa %= 2
    assert(aa == 1)

    // multiple return values
    bb, cc, dd, ee := 1     // all set to 1
    assert(bb == 1 & bb == cc & dd == ee)
    ff, gg := 2, 3
    assert(ff == 2 & gg == 3)
    ff, gg = 8, 9
    assert(ff == 8 & gg == 9)
    function multiret():
        if(false): return 4, 5
        return 6, 7
    hh, ii := multiret()
    assert(hh == 6 & ii == 7)
    hh = multiret()
    assert(hh == 6)
    function multiretpassthru(): return multiret()  // doesn't work without the return!
    jj, kk := multiretpassthru()
    assert(jj == 6 & kk == 7)

    // non-local named returns: limited form of exception handling
    function nlr1():
        nlr2()
        2
    function nlr2():
        return 1 from nlr1
    assert(nlr1() == 1)

    function recursive_exceptions(n):
        try():
            if(n): recursive_exceptions(n - 1)
            else: throw(0)
        catch v:
            assert(v == n)
            throw(n + 1)
    try():
        recursive_exceptions(10)

    starttime := seconds_elapsed()
    1000.for():
        7.factorial
        testvector.fold(0): _x + _y

    assert(seconds_elapsed() - starttime > 0)

    function cycletest():
        struct cyclist: [ name, loop, next ]
        cycle := [ "cycleouter", nil, [ "cycleinner", nil, nil ]:cyclist ]:cyclist
        cycle.loop = cycle
        cycle.next.loop = cycle.next

    cycletest()
    assert(collect_garbage() == 4) // 2 vectors, 2 strings

    //"press enter to continue...".print
    //getline()

run_test_cases()
