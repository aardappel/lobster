import std

let num_funs = 10000

// Some random stuff copied from the unit test.
let body =
"""
do():
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

    assert 1 & 3 == 1
    assert 1 | 3 == 3
    assert 1 ^ 2 == 3
    assert 1 << 1 == 2
    assert 2 >> 1 == 1
    assert ~1 == -2

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

    assert int2 { 10, 20 } % int2 { 3, 6 } == int2 { 1, 2 }
    assert int2 { 10, 20 } % 3 == int2 { 1, 2 }
    assert abs(4.0 % 1.5 - 1.0) < 0.01
    assert max(abs(float2 { 10.1, 20.1 } % float2 { 3.0, 6.0 } - float2 { 1.1, 2.1 })) < 0.01
    assert max(abs(float2 { 10.1, 20.1 } % 3.0 - float2 { 1.1, 2.1 })) < 0.01
"""

let prog = []
prog.push("import std\n")

for(num_funs) i:
    prog.push("def f{i}():\n")
    prog.push(body)

for(num_funs) i:
    prog.push("f{i}()\n")

if false:
    write_file("stress_test.lobster", concat_string(prog, ""), true)
