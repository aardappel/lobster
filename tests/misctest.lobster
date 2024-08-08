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
    assert (1 & 3 == 1) and
           (1 | 3 == 3) and
           (1 ^ 2 == 3) and
           (1 << 1 == 2) and
           (2 >> 1 == 1) and
           (~1 == -2)

    var vardef = 1
    vardef++
    let constdef = 1
    // constdef++ // error!

    assert test_namespace.f() == 1
    assert (test_namespace.g {}).x == 1
    assert (test_namespace.g {}).m() == 2

    class testa:
        a:int
    class testb : testa
        attribute serializable
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
        attribute serializable = 123
        h:bool
        a:int
        b:float
        c:float3
        d:string
        e:[int]
        f:string?
        g:int
        s:testa  // Test subclasses.

    let direct = parsetest {
        false, 1, 2.0, float3 { 3.0, 4.0, 5.0 }, "hello, world!\n\"\'\r\t\\\xC0",
        [ 0, -64 ], nil, true, testb { 6, 7 }
    }
    var parsed, err = parse_data(typeof direct, "{direct}")
    if err: print err
    assert equal(parsed, direct)
    // Test defaults:
    parsed, err = parse_data(typeof direct, "parsetest {{ true, 1, 1.0, float3 {{}}, \"\", [], nil, -1, testb {{ 6, 7 }} }}")
    if err: print err
    let groundv = parsetest { true, 1, 1.0, float3 { 0.0, 0.0, 0.0 }, "", [], nil, -1, testb { 6, 7 } }
    assert equal(parsed, groundv)
    let flex = flexbuffers_value_to_binary(parsed)
    let json = flexbuffers_binary_to_json(flex, false, "")
    assert json
    // Fields that are nil or scalar default value should be missing!
    let groundj = """{ a: 1, b: 1.0, d: "", e: [  ], g: -1, h: 1, s: { _type: "testb", a: 6, b: 7 } }"""
    assert json == groundj
    let flex2, ferr = flexbuffers_json_to_binary(json)
    assert not ferr
    let json2 = flexbuffers_binary_to_json(flex2, false, "")
    assert json2
    assert json2 == groundj
    let fval, fverr = flexbuffers_binary_to_value(typeof direct, flex2)
    assert not fverr
    assert equal(fval, groundv)
    let lb = lobster_value_to_binary(parsed)
    let lbval, lberr = lobster_binary_to_value(typeof direct, lb)
    assert not lberr
    assert equal(lbval, groundv)
    // Just to see if it can parse a namespaced id.
    assert parse_data(typeof test_namespace.g, "test_namespace.g {{}}")

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
    assert sign(float3 { -6.0, 0.0, 6.0 }) == int3 { -1, 0, 1 }

    // multiple return values
    let bb, cc, dd, ee = 1, 1, 1, 1
    assert bb == 1 and bb == cc and dd == ee
    var ff, gg = 2, 3
    assert ff == 2 and gg == 3
    ff, gg = 8, 9
    assert ff == 8 and gg == 9
    def multiret():
        if sin(0.0): return 4, 5
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
            else: throw ""
        fn v:
            assert v.length == n
            throw v + "*"
    try():
        recursive_exceptions(10)

    for 10:
        7.factorial
        testvector.fold(0): _x + _y

    assert in_range(0, 1)
    assert in_range(0, 1, 0)  // Pass int to nillable int type just for builtins.

    // Modulo operator extended to vectors and floats.
    assert int2 { 10, 20 } % int2 { 3, 6 } == int2 { 1, 2 }
    assert int2 { 10, 20 } % 3 == int2 { 1, 2 }
    assert abs(4.0 % 1.5 - 1.0) < 0.01
    assert max(abs(float2 { 10.1, 20.1 } % float2 { 3.0, 6.0 } - float2 { 1.1, 2.1 })) < 0.01
    assert max(abs(float2 { 10.1, 20.1 } % 3.0 - float2 { 1.1, 2.1 })) < 0.01

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
    let dict = dictionary<float, float>(17)
    // Check get before first set()
    assert dict.get(1.0, -1.0) == -1.0
    rnd_seed(0)
    for(32):
        let key = rnd_float()  // 0..1
        dict.set(key, key)
    rnd_seed(0)
    for(32):
        let key = rnd_float()
        let value = dict.get(key, -1.0)
        assert value == key
    // Test using nil as default value.
    let dict2 = dictionary<string, string?>(17)
    dict2.set("a", "b")
    let val = dict2.get("a", nil)
    assert val and val == "b"
    // Test using structs!
    let dict3 = dictionary<int3, int3>(17)
    dict3.set(int3 { 1, 2, 3 }, int3 { 4, 5, 6 })
    assert dict3.get(int3 { 1, 2, 3 }, int3_0) == int3 { 4, 5, 6 }

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

    // Multiple different paths thru same unwind point.

    def outer_dest2(mode):
        def outer_dest1():
            def return_past() -> float:
                switch mode:
                    case 0: return "a" from outer_dest1
                    case 1: return int2 { 4, 5 } from outer_dest2  // Different number and type.
                    default: return 3.1  // local return.
            // The unwind check of the return_past call must deal with different return types and sizes.
            return string(return_past())
        return int2 { length(outer_dest1()), 0 }
    assert outer_dest2(0) == int2 { 1, 0 }
    assert outer_dest2(1) == int2 { 4, 5 }
    assert outer_dest2(2) == int2 { 3, 0 }

    // Like Python, but actually a built-in function.
    if false:
        pass()

    def function_with_attributes():
        attribute foo
        attribute bar = baz
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

    assert equal(tokenize(" A | B C |", "|", " ", true), ["A", "B C", ""])
    assert equal(tokenize("; A ; B C;; ", ";", " "), ["", "A", "B C", ""])
    assert equal(tokenize(", A , B C,, ", ",", " ", true), ["", "A", "B C", "", ""])

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
    assert "{(float2_1 / float2_0).y}" == "inf"

    do():
        let divisor = 2.0
        // mod with floats and vectors.
        assert 5.0 % divisor == 1.0
        assert float2_1 * 5.0 % divisor == float2_1
        assert int2_1 * 5 % 2 == int2_1

    do():
        // Reflection
        class Reflect:
            c = float2_1
            s = "hello"
            v = [ 2 ]
        let r = Reflect {}
        assert type_string(r) == "Reflect"
        assert type_field_count(r) == 4  // Structs are inline
        assert type_field_string(r, 0) == "float"
        assert type_field_string(r, 3) == "[int]"
        assert type_field_name(r, 2) == "s"
        assert type_field_value(r, 0) == "1.0"
        assert type_element_string(r.v) == "int"

    do():
        // Calling function values from native.
        var state = 1
        let fv = fn() -> void:
            // Executed from within native code.
            state = 2
        call_function_value(fv)
        assert state == 2

    do():
        // Guards.
        let gs = map(4) i:
            guard i >= 2:
                guard i >= 1:
                    10
                20
            guard i >= 3:
                30
            40
        assert equal(gs, [ 10, 20, 30, 40 ])
        guard true
        guard false
        assert false

    do():
        // Parse >> correctly.
        assert (dictionary<string, dictionary<string, int>>(17)).get("foo") == nil
