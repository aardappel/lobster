// this file runs some test cases exercising different parts of the language
// can be included in any lobster program to help verify there are no bugs in the compiler
// will assert if anything is wrong - if this ever happens, contact Wouter

include std
include exception
include vec
include astar

namespace test_namespace

def f(): 1
struct g { x:int = 1 }

namespace test

def run_test_cases(is_speed_test):
    trace_bytecode(false, true)

    assert 1 & 3 == 1
    assert 1 | 3 == 3
    assert 1 ^ 2 == 3
    assert 1 << 1 == 2
    assert 2 >> 1 == 1
    assert ~1 == -2

    var vardef = 1    // shorter form: vardef := 1
    let constdef = 1  // shorter form: constdef :== 1

    assert test_namespace_f() == 1
    assert (test_namespace_g {}).x == 1

    value testa { a:int }
    value testb : testa { b:int }

    def tf(x:int, y:int): 1
    def tf(x:string, y:float): 2
    def tf(x:string, y:int): 3
    def tf(x:testa, y:int): 4
    def tf(x:testb, y:string): 9
    def tf(x:testb, y:float): 5
    def tf(x:testb, y:int): 6
    def tf(x, y): 7

    def tf(x): 8

    assert(tf(testa { 0 }, 0) == 4)
    assert(tf(1, 0) == 1)
    assert(tf("", 0.0) == 2)
    assert(tf("", 0) == 3)
    assert(tf(testb { 0, 0 }, "") == 9)
    assert(tf(testb { 0, 0 }, 0.0) == 5)
    assert(tf(testb { 0, 0 }, 0) == 6)
    assert(tf("", "") == 7)

    // force dynamic dispatch
    def to_any(a:any): a
    assert(tf(to_any(testb { 0, 0 }), to_any("")) == 9)

    assert(tf("") == 8)

    struct parsetest { a:int, b:float, c:xyz_f, d:string, e:[int], f:string?, g:int }
    direct := parsetest {
        1, 2, xyz { 3.0, 4.0, 5.0 }, "hello, world!\n\"\'\r\t\\\xC0",
        [ 0, -64 ], nil, true
    }
    parsed, err := parse_data(typeof direct, "" + direct)
    //print(parsed)
    //print(direct)
    if err: print err
    assert equal(parsed, direct)
    // Test defaults:
    parsed, err = parse_data(typeof direct, "parsetest { 1, 1.0, xyz_f {}, \"\", [] }")
    if err: print err
    assert equal(parsed, parsetest { 1, 1.0, xyz { 0.0, 0.0, 0.0 }, "", [], nil, 0 })
    assert switch rnd(2):
        default: true



    unicodetests := [0x30E6, 0x30FC, 0x30B6, 0x30FC, 0x5225, 0x30B5, 0x30A4, 0x30C8]
    assert equal(string_to_unicode(unicode_to_string(unicodetests)), unicodetests)

    if !is_speed_test and !vm_compiled_mode():
        compres1, comperr1 := compile_run_code("1 + 2", [])
        if comperr1:
            print comperr1
        assert compres1 == "3"

        /*
        // this test makes it dependent on this file even in shipping builds, so off by default
        compres2, comperr2 := compile_run_file("plugintest.lobster", [])
        if(comperr2):
            print(comperr2)
        assert(compres2 == "plugintest return value")
        */

    def scopetest():
        x := 1
        y := 1
        def f():
            assert x == 1
            assert y == 2
        def g():
            // This used to be legal, but now shadowing is disabled.
            // x := 2
            y <- 2
            f()
        g()
    scopetest()

    st := map(10) i: switch i:
            case 1: "no"
            case 2, 3:
                x := i
                "yes" + x
            case 4..6, 8: "maybe"
            default:
                "what?"
    assert st.equal([ "what?", "no", "yes2", "yes3", "maybe", "maybe", "maybe", "what?", "maybe", "what?" ])
    assert switch st[2]:
        case "yes2": true
        case st[3]: false  // Can use variables and expressions :)
        default: false
    assert switch 3.14:
        case 10.0..20.0: false  // Inclusive float ranges.
        default: true

    do():
        a := 0
        assert a++ + ++a + a-- + --a == 4
        b := [0]
        assert b[0]++ + ++b[0] + b[0]-- + --b[0] == 4

    testvector := [ 3, 9, 5, 4, 1, 3, 9, 5, 4, 1 ]

    sorted1 := testvector.qsort(): _a < _b
    sorted2 := copy(testvector)
    sorted2.insertion_sort(): _a < _b
    sorted3 := copy(testvector)
    sorted3.qsort_in_place(): _a < _b

    assert equal(sorted1, [1,1,3,3,4,4,5,5,9,9])
    assert equal(sorted1, sorted2)
    assert equal(sorted1, sorted3)

    found, findex := sorted1.binary_search(1)
    assert found == 2 and findex == 0
    found, findex = sorted1.binary_search(9)
    assert found == 2 and findex == 8
    found, findex = sorted1.binary_search(3)
    assert found == 2 and findex == 2

    assert 44 == sum(testvector)
    assert 264 == sum(testvector.map(): _ * _)


    def factorial(n): 1 > n or factorial(n - 1) * n
    assert 7.factorial == 5040

    applyto4 := def(f): f(4)
    assert 16 == applyto4(def(x): x * x)

    assert ensure_size("abc", 3, 'z') == "abc"
    assert ensure_size("abc", 5, 'z') == "abczz"
    assert ensure_size("abc", -5, 'z') == "zzabc"
    ns, ni := "".write_int16_le(0, 1027)
    assert ns == "\x03\x04\x00\x00" and ni == 2
    assert ns.read_int32_le(0) == 1027
    ns, ni = "".write_int16_le_back(0, 2050)
    assert ns == "\x00\x00\x02\x08" and ni == 2
    assert ns.read_int16_le_back(0) == 2050

    // ////////////////////////////////////////////////////////////////////////
    // typechecking

    do():
        def h(i): i()
        def g(f):
            // Here, the def passed to h appears the same each time, but it has a freevar f that is different.
            h():
                f()
        assert 1 == (g(): 1)
        assert "a" == (g(): "a")

    def compile_time_if(x):
        if x is int or x is float:
            // The division is a type error if x is a string.
            // But it compiles anyway since this if is compile-time constant,
            // and the type checker ignores this branch.
            // Would be an error in a C++ template, and most other statically typed languages,
            // because they don't cull code without the optimizer.
            1 / x
        else:
            // Also note the return type is just the else branch in the string case,
            // it doesn't need to make a type union between the branches anymore.
            x
    assert compile_time_if(1) is int
    assert compile_time_if("") is string

    assert typeof int == 0  // Always the first one added to the typetable.

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

    struct pathingcell : astar_node(pathingcell?, xy_i, xy_i?):
        path:int
        c:int

    worldsize := xy { initworld[0].length, initworld.length }

    startpos := xy_0i
    endpos := xy_0i

    world := worldsize.y.map() y: worldsize.x.map() x:
        c := initworld[y][x]
        if c == 'S': startpos = xy { x, y }
        if c == 'E': endpos   = xy { x, y }
        pathingcell { xy { x, y }, false, c }

    path :=
        astar_2dgrid(false, worldsize, world[startpos], world[endpos], def(): world[_]) (n, nn::pathingcell):
            (c == '#' and -1) or (c == '/' and 5) or 1

    assert path.length == 27

    for(path) n:
        n.path = true

    astar_result := map(world) row: fold(row, "") accum, n:
        accum + ((n.path   and "P") or
                 (n.closed and "X") or
                 (n.open   and "O") or
                 unicode_to_string([ n.c ]))

    assert equal(astar_result, expected_result)


    // ////////////////////////////////////////////////////////////////////////
    // GOAP

    enum wolves, skins, money, flour, bread, pizza, hungry

    goapactions := [
        goapaction { "kill wolf",  def(s): s[wolves] >= 1, def(s): s[wolves]--;   s[skins]++ },
        goapaction { "sell skin",  def(s): s[skins]  >= 1, def(s): s[money] += 2; s[skins]-- },
        goapaction { "buy pizza",  def(s): s[money]  >= 2, def(s): s[money] -= 2; s[pizza]++ },
        goapaction { "buy flour",  def(s): s[money]  >= 1, def(s): s[money]--;    s[flour]++ },
        goapaction { "bake bread", def(s): s[flour]  >= 1, def(s): s[flour]--;    s[bread]++ },
        goapaction {
            "eat",
            def(s): (s[bread] > 0 or s[pizza] > 0) and s[hungry] > 0,
            def(s): if s[bread] > 0: s[bread]-- else: s[pizza]--; s[hungry]--
        }
    ]

    goap_path :=
        astar_goap(goapactions, [ 3, 0, 0, 0, 0, 0, 2 ]) state:
            state[hungry]
        endcondition s:
            s[hungry] <= 0

    assert goap_path.length == 9
    goap_path.pop()
    assert equal(goap_path.map(): _.delta?.name or "",
        ["eat","buy pizza","sell skin","kill wolf","eat","buy pizza","sell skin","kill wolf"])


    // ////////////////////////////////////////////////////////////////////////
    // coroutines test

    // FIXME: sticking these in a block for now, because collect_garbage() below
    // can't handle coroutines yet.
    do():
        def mycoro(f):
            forrange(3, 6): f(_)
            1337

        def myfor(n, f):
            for n: f(_)
            0

        co1 := coroutine mycoro()   // out of nested HOFs
        co2 := coroutine myfor(4)   // out of a simple function

        def interleave(a, b, f):   // test using 2 at once
            while a.active or b.active:
                if a.active:
                    f(a.return_value)
                    a.resume
                if b.active:
                    f(b.return_value)
                    b.resume

        //interleave(co1, co2): print(_)

        assert equal(collect() f: interleave(co1, co2, f), [ 3, 0, 4, 1, 5, 2, 3 ])
        assert !co1.active
        assert !co2.active
        assert co1.return_value == 1337

        def pass_thru(co, f):
            coroutine_for co: f(_)

        co3 := coroutine pass_thru(coroutine myfor(10))    // test consuming 1 coro with another and then passing that on
        sum := 0
        while co3.active:       // we currently can't use coroutine_for here, will be a runtime error (nesting)
            sum += co3.return_value
            co3.resume
        assert sum == 45

        // Access variables inside coroutines from the outside:
        def loctest(f):
            a := 1
            for(10) i:
                b := 2
                f()

        co4 := coroutine loctest()
        assert co4.active
        assert co4->a + co4->i + co4->b == 3

        // Even functions that don't yield are cool:
        def conoyield(f): 1
        co5 := coroutine conoyield()
        assert !co5.active
        assert co5.return_value == 1

    // ////////////////////////////////////////////////////////////////////////
    // misc test

    aa := 5
    aa %= 2
    assert aa == 1
    assert equal(sign(xyz_f { -6, 0, 6 }), xyz_i { -1, 0, 1 })

    // multiple return values
    bb, cc, dd, ee := 1     // all set to 1
    assert bb == 1 and bb == cc and dd == ee
    ff, gg := 2, 3
    assert ff == 2 and gg == 3
    ff, gg = 8, 9
    assert ff == 8 and gg == 9
    def multiret():
        if sin(0): return 4, 5
        return 6, 7
    hh, ii := multiret()
    assert hh == 6 and ii == 7
    hh = multiret()
    assert hh == 6
    def multiretpassthru(): return multiret()  // doesn't work without the return!
    jj, kk := multiretpassthru()
    assert jj == 6 and kk == 7

    // Union return types (should infer string?):
    def union_return(n):
        for(10) a:
            if a == n: return "foo"
        nil
    assert union_return(5)

    // non-local named returns: limited form of exception handling
    def nlr1():
        nlr2()
        2
    def nlr2():
        return 1 from nlr1
    assert nlr1() == 1

    def recursive_exceptions(n):
        try():
            if n: recursive_exceptions(n - 1)
            else: throw 0
        catch v:
            assert v == n
            throw n + 1
    try():
        recursive_exceptions(10)

    // Define your own HOFs with lazy expressions, much like the built-in while loop:
    def whilenot(cond:lazy_expression, body):
        while not cond(): body()
    wnt := 0
    whilenot wnt == 10: wnt++
    assert wnt == 10

    starttime := seconds_elapsed()
    for 1000:
        7.factorial
        testvector.fold(0): _x + _y

    assert(seconds_elapsed() - starttime > 0)

    if !is_speed_test:
        def cycletest():
            struct cyclist { name:string, loop:cyclist?, next:cyclist? }
            cycle := cyclist { "cycleouter", nil, cyclist { "cycleinner", nil, nil } }
            cycle.loop = cycle
            assert cycle.next
            cycle.next.loop = cycle.next
        cycletest()
        garbage_objects := collect_garbage()
        assert garbage_objects == 4 // 2 vectors, 2 strings
        garbage_objects = collect_garbage()
        assert !garbage_objects

    //"press enter to continue...".print
    //get_line()

if program_name() == "unittest.lobster" or program_name() == "include/unittest.lobster":
    run_test_cases(false)

