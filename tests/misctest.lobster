import testing
import std
import exception
import vec
import dictionary

// Misc small tests that don't fit elsewhere.

namespace test_namespace

def f(): return f2()  // Forward call with correct namespace.
def f2(): return 1
class g:
    x = 1
    def m(): return 2

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
    assert (test_namespace_g {}).m() == 2

    class testa:
        a:int
    class testb : testa
        b:int

    def tf(x:int, y:int): return 1
    def tf(x:string, y:int): return 2
    def tf(x:testa, y:int): return 3
    def tf(x:testb, y:int): return 4

    // These will all resolve statically.
    assert(tf(1, 0) == 1)
    assert(tf("", 0) == 2)
    assert(tf(testb { 0, 0 }, 4) == 4)  // No subclasses.

    // This will dispatch dynamically.
    def to_a(a:testa): return a
    assert(tf(to_a(testb { 0, 0 }), 1) == 4)

    class parsetest:
        h:bool
        a:int
        b:float
        c:xyz_f
        d:string
        e:[int]
        f:string?
        g:int

    let direct = parsetest {
        false, 1, 2, xyz { 3.0, 4.0, 5.0 }, "hello, world!\n\"\'\r\t\\\xC0",
        [ 0, -64 ], nil, true
    }
    var parsed, err = parse_data(typeof direct, "{direct}")
    //print(parsed)
    //print(direct)
    if err: print err
    assert equal(parsed, direct)
    // Test defaults:
    parsed, err = parse_data(typeof direct, "parsetest {{ true, 1, 1.0, xyz_f {{}}, \"\", [] }}")
    if err: print err
    let groundv = parsetest { true, 1, 1.0, xyz { 0.0, 0.0, 0.0 }, "", [], nil, 0 }
    assert equal(parsed, groundv)
    let flex = flexbuffers_value_to_binary(parsed)
    let json = flexbuffers_binary_to_json(flex)
    assert json
    let groundj = """{ _type: "parsetest", a: 1, b: 1.0, c: { x: 0.0, y: 0.0, z: 0.0 }, d: "", e: [  ], f: null, g: 0, h: 1 }"""
    assert json == groundj
    let flex2, ferr = flexbuffers_json_to_binary(json)
    assert not ferr
    let json2 = flexbuffers_binary_to_json(flex2)
    assert json2
    assert json2 == groundj
    let fval, fverr = flexbuffers_binary_to_value(typeof direct, flex2)
    assert not fverr
    assert equal(fval, groundv)

    assert switch rnd(2):
        default: true

    let unicodetests = [0x30E6, 0x30FC, 0x30B6, 0x30FC, 0x5225, 0x30B5, 0x30A4, 0x30C8]
    assert equal(string_to_unicode(unicode_to_string(unicodetests)), unicodetests)

    let st = map(10) i: switch i:
            case 1: "no"
            case 2, 3:
                let x = i
                "yes{x}"
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


    def factorial(n:int) -> int: return 1 > n or factorial(n - 1) * n
    assert 7.factorial == 5040

    let applyto4 = fn f: f(4)
    assert 16 == applyto4(fn x: x * x)

    assert ensure_size("abc", 3, 'z') == "abc"
    assert ensure_size("abc", 5, 'z') == "abczz"
    assert ensure_size("abc", -5, 'z') == "zzabc"
    var ns, ni = "".write_int16_le(0, 1027)
    assert ns == "\x03\x04\x00\x00" and ni == 2
    assert ns.read_int32_le(0) == 1027
    ns, ni = "".write_int16_le_back(0, 2050)
    assert ns == "\x00\x00\x02\x08" and ni == 2
    assert ns.read_int16_le_back(0) == 2050

    var s1 = "a"
    var s2 = "b"
    assert (s1 and s2) == s2
    s1, s2 = s2, s1

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
    def multirettyped() -> float, string: return 1.0, "a"
    let nn, mm = multirettyped()
    assert nn == 1.0
    assert mm == "a"
    class mretfields:
        a = ""
        b = 0
        def write():
            a, b = a.write_int16_le(b, 12345)
    (mretfields {}).write()

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

    def recursive_exceptions(n) -> void:
        try():
            if n: recursive_exceptions(n - 1)
            else: throw 0
        fn v:
            assert v == n
            throw n + 1
    try():
        recursive_exceptions(10)

    for 10:
        7.factorial
        testvector.fold(0): _x + _y

    assert in_range(0, 1)
    assert in_range(0, 1, 0)  // Pass int to nillable int type just for builtins.

    // Modulo operator extended to vectors and floats.
    assert xy { 10, 20 } % xy { 3, 6 } == xy { 1, 2 }
    assert xy { 10, 20 } % 3 == xy { 1, 2 }
    assert abs(4.0 % 1.5 - 1.0) < 0.01
    assert max(abs(xy { 10.1, 20.1 } % xy { 3.0, 6.0 } - xy { 1.1, 2.1 })) < 0.01
    assert max(abs(xy { 10.1, 20.1 } % 3.0 - xy { 1.1, 2.1 })) < 0.01

    // Unicode in strings: either as escape code, or also direct, if your editor supports it.
    assert "⺆㋇䬈" == "\u2E86\u32C7\u4B08"

    // Bitwise assignment ops.
    var bit = 1
    assert (bit |= 2) == 3
    assert (bit &= ~2) == 1
    assert (bit ^= 3) == 2
    assert (bit <<= 1) == 4
    assert (bit >>= 1) == 2

    // Dictionary.
    let dict = dictionary<float, float> { 17 }
    rnd_seed(0)
    for(32):
        let key = rnd_float()  // 0..1
        dict.set(key, key)
    rnd_seed(0)
    for(32):
        let key = rnd_float()
        let value = dict.get(key, -1)
        assert value == key
    // Test using nil as default value.
    let dict2 = dictionary<string, string?> { 17 }
    dict2.set("a", "b")
    let val = dict2.get("a", nil)
    assert val and val == "b"
    // Test using structs!
    let dict3 = dictionary<xyz_i, xyz_i> { 17 }
    dict3.set(xyz { 1, 2, 3 }, xyz { 4, 5, 6 })
    assert dict3.get(xyz { 1, 2, 3 }, xyz_0i) == xyz { 4, 5, 6 }

    // Break.
    var lc = 0
    for(10) i:
        if i == 5: break
        lc++
    assert lc == 5
    while lc:
        if lc == 1: break
        lc--
    assert lc == 1

    // Non-local control still works with returning values.
    def outer():
        def error():
            assert false
            return from outer
        let a = if abs(1) == 1: 1 else: error()
        let b = switch a:
                    case 1: 2
                    default: error()
        assert a + b == 3
    outer()

    // Like Python, but actually a built-in function.
    if false:
        pass()

    // default args.
    let dav = 1  // Can even reference vars.
    def da(a:int, b, c = dav + 1): return a + b + c
    def da(a:float, b, c = dav + 1): return int(a) - b - c
    def da(a): return a  // Does not "overlap"
    assert da(1) + da(1, 2) + da(1, 2, 3) == 12

    // More string functions.
    assert find_string("abcdabcd", "BC") == -1
    assert find_string("abcdabcd", "bc") == 1
    assert find_string("abcdabcd", "bc", 4) == 5
    assert find_string_reverse("abcdabcd", "bc") == 5
    assert find_string_reverse("abcdabcd", "bc", 4) == 1
    assert replace_string("abcdabcd", "bc", "X") == "aXdaXd"
    assert replace_string("abcdabcd", "bc", "X", 1) == "aXdabcd"

    // Single return not being the last return.
    def G():
        return H() + 1
    def H():
        if []:
            return 4
        return 3 from G
    assert G() == 5

    // ref lvalues returned from assign.
    var slv = "a"
    assert (slv += "b") == "ab"
    var vlv = [ 1 ]
    assert equal(vlv = [ 2 ], [ 2 ])
    struct STLV:
        a:string
        b:[string]
    var stlv = STLV { "a", [ "a" ] }
    assert (stlv = STLV { "b", [ "b" ] }).b[0] == "b"

    // IEEE754 div by zero behavior
    assert "{1.0 / 0.0}" == "inf"
    assert "{1.0 % 0.0}".find_string("nan") >= 0
    assert "{(xy_1 / xy_0).y}" == "inf"
