
import vec

// `+=` on strings: the pieces of the right hand side go onto the string one by one, growing it
// in place when nothing else refers to it, see VM::AppendString and CodeGen::GenStringAppend.
// What is appended: concatenation operands and values converted as they go on (thru a call
// the optimizer inlined down to its value and an explicit string() as well), and the text
// substring and number_to_string would make.

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
    // The right hand side may refer to the string itself, which it sees as it was: for the
    // appends the string then keeps a reference of its own, so it is copied rather than grown.
    var self = repeat_string("a", 111)  // Fills its allocation, so growing it moves it.
    self += "x" + self
    test_eq(self, repeat_string("a", 111) + "x" + repeat_string("a", 111))
    var self2 = repeat_string("b", 4)  // Has room to grow in place.
    self2 += self2 + self2
    test_eq(self2, "bbbbbbbbbbbb")
    self2 += substring(self2, 0, 2) + "!" + self2.substring(11, -1)
    test_eq(self2, "bbbbbbbbbbbbbb!b")
    let sh = AppendHolder { repeat_string("c", 111), [ repeat_string("d", 111) ] }
    sh.s += "x" + sh.s
    test_eq(sh.s.length, 223)
    sh.v[0] += sh.v[0] + "y"
    test_eq(sh.v[0].length, 223)
    append_global = repeat_string("e", 111)
    append_global += "!" + append_global
    test_eq(append_global.length, 223)
    // Also thru a parameter that borrows it.
    def append_self_param(bs:string):
        append_global += "?" + bs
    append_self_param(append_global)
    test_eq(append_global.length, 447)
    // A piece that converts a vector or object holding the string sees it as it was too.
    let sv = AppendHolder { "ab", [ "cd" ] }
    let sv_before = string(sv.v)
    sv.s += "-" + string(sv.v)
    test_eq(sv.s, "ab-" + sv_before)
    sv.v[0] += "-" + string(sv.v)
    test_eq(sv.v[0], "cd-" + sv_before)
    // What goes on without a string of its own: an explicit string(), a struct of scalars,
    // substring and number_to_string.
    var ex = ""
    ex += string(42) + string(1.5) + string(int3 { 1, 2, 3 }) + string([ 4 ]) + string("s")
    test_eq(ex, "421.5int3\{1, 2, 3\}[4]s")
    let f3 = float3 { 0.5, 1.5, 2.5 }
    ex += "/{f3}"
    ex += f3
    test_eq(ex, "421.5int3\{1, 2, 3\}[4]s/float3\{0.5, 1.5, 2.5\}float3\{0.5, 1.5, 2.5\}")
    let src = "hello world"
    ex = ""
    ex += substring(src, 6, 5) + " " + src.substring(0, -1) + number_to_string(255, 16, 4)
    ex += number_to_string(5, 2, 0)
    test_eq(ex, "world hello world00FF101")
    // The same onto a field, an element and a global, which hold their string differently.
    let eh = AppendHolder { "", [ "" ] }
    eh.s += substring(src, 0, 5) + int2 { 1, 2 } + number_to_string(10, 10, 3)
    eh.v[0] += substring(src, 0, 5) + int2 { 1, 2 } + number_to_string(10, 10, 3)
    append_global = ""
    append_global += substring(src, 0, 5) + int2 { 1, 2 } + number_to_string(10, 10, 3)
    test_eq(eh.s, "helloint2\{1, 2\}010")
    test_eq(eh.v[0], eh.s)
    test_eq(append_global, eh.s)
    // A constant is never grown, so every evaluation of the literal starts out the same.
    var lit = ""
    for(3) j:
        var c = "c"
        c += "{j}"
        lit += c
    test_eq(lit, "c0c1c2")

append_tests()
