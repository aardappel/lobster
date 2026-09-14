
import vec

// `+=` on strings: the pieces of the right hand side go onto the string one by one, growing it
// in place when nothing else refers to it, see VM::AppendString and CodeGen::GenStringAppend.
// What is appended: concatenation operands and values converted as they go on, thru a call
// the optimizer inlined down to its value as well.

class AppendHolder:
    s:string
    v:[string]

var append_global = ""

def append_line(i:int, name:string) -> string:
    return "line {i}: " + name + "\n"

def append_pick(i:int) -> string:
    if i % 2 == 0: return "even {i}"
    return "odd " + string(i)

def append_nested(i:int) -> string:
    return append_pick(i) + "!"

// Recursive, so never inlined: the value is a string of its own.
def append_recursive(n:int) -> string:
    if n == 0: return "."
    return append_recursive(n - 1) + "{n}"

def append_tests():
    suite("append")
    // Concatenations and conversions of every kind go on piece by piece.
    var s = "a"
    s += "b" + "c" + "d"
    test_eq(s, "abcd")
    s += 12
    s += 3.5
    test_eq(s, "abcd123.5")
    let i = 7
    let f = 0.25
    let v = [ 1, 2 ]
    let v2 = int2 { 3, 4 }
    s += " {i} {f} {v} {v2} {i > 3}"
    test_eq(s, "abcd123.5 7 0.25 [1, 2] int2\{3, 4\} true")
    // An inlined call: what each return leaves is appended there.
    var t = ""
    for(4) j: t += append_line(j, "n{j}")
    test_eq(t, "line 0: n0\nline 1: n1\nline 2: n2\nline 3: n3\n")
    t = ""
    for(4) j:
        t += append_pick(j)
        t += ","
    test_eq(t, "even 0,odd 1,even 2,odd 3,")
    t = ""
    t += append_nested(5)
    t += append_recursive(3)
    test_eq(t, "odd 5!.123")
    // Every operand is evaluated before anything is appended.
    var order = ""
    def side(x:string) -> string:
        order += "[" + x + "]"
        return x
    order += side("a") + side("b")
    test_eq(order, "[a][b]ab")
    // `s = s + ..` is the same append.
    var u = "x"
    u = u + "y" + 1 + "z"
    test_eq(u, "xy1z")
    u = "p" + u
    test_eq(u, "pxy1z")
    // Fields, elements and globals as the target.
    let h = AppendHolder { "h", [ "e0", "e1" ] }
    h.s += "1" + "2"
    h.s += 3
    h.v[1] += "x{i}"
    test_eq(h.s, "h123")
    test_seq(h.v, [ "e0", "e1x7" ])
    append_global += "g" + "{i}"
    append_global += 4.5
    test_eq(append_global, "g74.5")
    // A string something else still refers to keeps its value.
    var shared = "ab"
    let alias = shared
    shared += "cd"
    test_eq(alias, "ab")
    test_eq(shared, "abcd")
    var held = "ab"
    h.v.push(held)
    held += "cd"
    test_eq(h.v[2], "ab")
    test_eq(held, "abcd")
    // Growing in place, past the small allocation sizes and on.
    var grown = ""
    for(1000) j: grown += "{j % 10}"
    test_eq(grown.length, 1000)
    test_eq(grown.substring(995, 5), "56789")
    var big = ""
    for(20) j: big += "x".repeat_string(10000)
    test_eq(big.length, 200000)
    test_eq(big[199999], 'x')
    // A constant is never grown, so every evaluation of the literal starts out the same.
    var lit = ""
    for(3) j:
        var c = "c"
        c += "{j}"
        lit += c
    test_eq(lit, "c0c1c2")

append_tests()
