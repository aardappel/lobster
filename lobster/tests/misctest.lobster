include testing
include std
include exception
include vec

// Misc small tests that don't fit elsewhere.

namespace test_namespace

def f(): return 1
class g { x = 1 }

namespace whatever

run_test("misc"):
    assert 1 & 3 == 1
    assert 1 | 3 == 3
    assert 1 ^ 2 == 3
    assert 1 << 1 == 2
    assert 2 >> 1 == 1
    assert ~1 == -2

    var vardef = 1
    vardef++
    let constdef = 1
    // constdef++ // error!

    assert test_namespace_f() == 1
    assert (test_namespace_g {}).x == 1

    class testa { a:int }
    class testb : testa { b:int }

    def tf(x:int, y:int): return 1
    def tf(x:string, y:float): return 2
    def tf(x:string, y:int): return 3
    def tf(x:testa, y:int): return 4
    def tf(x:testb, y:string): return 9
    def tf(x:testb, y:float): return 5
    def tf(x:testb, y:int): return 6

    def tf(x): return 8

    assert(tf(testa { 0 }, 0) == 4)
    assert(tf(1, 0) == 1)
    assert(tf("", 0.0) == 2)
    assert(tf("", 0) == 3)
    assert(tf(testb { 0, 0 }, "") == 9)
    assert(tf(testb { 0, 0 }, 0.0) == 5)
    assert(tf(testb { 0, 0 }, 0) == 6)

    // force dynamic dispatch
    def to_a(a:testa): return a
    assert(tf(to_a(testb { 0, 0 }), 1) == 6)

    assert(tf("") == 8)

    class parsetest { a:int, b:float, c:xyz_f, d:string, e:[int], f:string?, g:int }
    let direct = parsetest {
        1, 2, xyz { 3.0, 4.0, 5.0 }, "hello, world!\n\"\'\r\t\\\xC0",
        [ 0, -64 ], nil, true
    }
    var parsed, err = parse_data(typeof direct, "" + direct)
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

    let unicodetests = [0x30E6, 0x30FC, 0x30B6, 0x30FC, 0x5225, 0x30B5, 0x30A4, 0x30C8]
    assert equal(string_to_unicode(unicode_to_string(unicodetests)), unicodetests)

    let st = map(10) i: switch i:
            case 1: "no"
            case 2, 3:
                let x = i
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
        var a = 0
        assert a++ + ++a + a-- + --a == 4
        let b = [0]
        assert b[0]++ + ++b[0] + b[0]-- + --b[0] == 4

    let testvector = [ 3, 9, 5, 4, 1, 3, 9, 5, 4, 1 ]

    let sorted1 = testvector.qsort(): _a < _b
    let sorted2 = copy(testvector)
    sorted2.insertion_sort(): _a < _b
    let sorted3 = copy(testvector)
    sorted3.qsort_in_place(): _a < _b

    assert equal(sorted1, [1,1,3,3,4,4,5,5,9,9])
    assert equal(sorted1, sorted2)
    assert equal(sorted1, sorted3)

    var found, findex = sorted1.binary_search(1)
    assert found == 2 and findex == 0
    found, findex = sorted1.binary_search(9)
    assert found == 2 and findex == 8
    found, findex = sorted1.binary_search(3)
    assert found == 2 and findex == 2

    assert 44 == sum(testvector)
    assert 264 == sum(testvector.map(): _ * _)


    def factorial(n): return 1 > n or factorial(n - 1) * n
    assert 7.factorial == 5040

    let applyto4 = def(f): f(4)
    assert 16 == applyto4(def(x): x * x)

    assert ensure_size("abc", 3, 'z') == "abc"
    assert ensure_size("abc", 5, 'z') == "abczz"
    assert ensure_size("abc", -5, 'z') == "zzabc"
    var ns, ni = "".write_int16_le(0, 1027)
    assert ns == "\x03\x04\x00\x00" and ni == 2
    assert ns.read_int32_le(0) == 1027
    ns, ni = "".write_int16_le_back(0, 2050)
    assert ns == "\x00\x00\x02\x08" and ni == 2
    assert ns.read_int16_le_back(0) == 2050

    var aa = 5
    aa %= 2
    assert aa == 1
    assert sign(xyz_f { -6, 0, 6 }) == xyz_i { -1, 0, 1 }

    // multiple return values
    let bb, cc, dd, ee = 1, 1, 1, 1
    assert bb == 1 and bb == cc and dd == ee
    var ff, gg = 2, 3
    assert ff == 2 and gg == 3
    ff, gg = 8, 9
    assert ff == 8 and gg == 9
    def multiret():
        if sin(0): return 4, 5
        return 6, 7
    let hh, ii = multiret()
    assert hh == 6 and ii == 7
    let ll = multiret()
    assert ll == 6
    def multiretpassthru(): return multiret()  // doesn't work without the return!
    let jj, kk = multiretpassthru()
    assert jj == 6 and kk == 7

    // Union return types (should infer string?):
    def union_return(n):
        for(10) a:
            if a == n: return "foo"
        return nil
    assert union_return(5)

    // non-local named returns: limited form of exception handling
    def nlr1():
        nlr2()
        return 2
    def nlr2():
        return 1 from nlr1
    assert nlr1() == 1
    nlr1()

    do():
        for(2) i:
            if i:
                return from do

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
    var wnt = 0
    whilenot wnt == 10: wnt++
    assert wnt == 10

    let starttime = seconds_elapsed()
    for 10:
        7.factorial
        testvector.fold(0): _x + _y
    assert(seconds_elapsed() - starttime > 0)
