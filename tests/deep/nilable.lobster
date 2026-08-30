import std


// nil and nilable types: flow-based promotion, typed nil, and/or with nil.

class DNBox:
    val = 7

def nil_tests():
    suite("nil")

    // Typed nil values and comparisons.
    let ns = nil::string
    let no = nil::DNBox
    test_true(ns == ns)
    test_true(ns == nil)
    test_true(ns != "foo")
    test_true(no != DNBox {})

    // if-promotion: inside the then-branch the value is non-nil.
    def maybe(x) -> string?: return if x: "v" else: nil
    let m1 = maybe(1)
    if m1:
        test_eq(m1.length, 1)  // Dereference fine after promotion.
    else:
        test_true(false, "maybe(1) was nil")
    // not: promotion applies to the else branch.
    let m0 = maybe(0)
    if not m0:
        test_true(true)
    else:
        test_eq(m0, "v")
    // and-chains promote for subsequent operands and the block.
    def slen(s:string): return s.length  // Requires non-nil argument.
    if m1 and slen(m1) == 1:
        test_eq(m1 + "!", "v!")
    // == nil works like a falsiness test.
    if m0 == nil:
        test_true(true)
    else:
        test_true(false, "m0 == nil was false")

    // or with nil as default, and chains of nilables.
    test_eq(maybe(0) or "dflt", "dflt")
    test_eq(maybe(1) or "dflt", "v")
    test_eq(maybe(0) or maybe(0) or "dflt", "dflt")
    test_eq(maybe(0) or maybe(1) or "dflt", "v")
    // a or nil / nil and a: give nil the type of a.
    let orn = m1 or nil
    test_true(orn == m1)
    let andn = nil and m1
    test_true(andn == nil)

    // Unconditional assignment un-nils a variable for the rest of the block.
    var progressive = nil::string
    test_true(progressive == nil)
    progressive = "now set"
    test_eq(progressive.length, 7)  // No check needed anymore.

    // assert promotes to non-nil.
    let asrt = assert maybe(1)
    test_eq(asrt + "", "v")

    // Nilable fields.
    class NilField:
        s:string? = nil
        next:NilField? = nil
    let nf = NilField {}
    test_true(nf.s == nil and nf.next == nil)
    nf.s = "x"
    nf.next = nf
    test_eq(nf.s + "", "x")  // Field assignment promoted the field type too.
    test_true(nf.next == nf)
    // and-chains promote nilable fields for the rhs.
    test_true(nf.next and nf.next.s == "x")
    nf.next = nil  // Break the cycle again (avoids a leak report).
    // A nilable ref field inside a (value) struct.
    struct NilInStruct:
        tag:int
        s:string?
    var nis = NilInStruct { 1, nil }
    test_true(nis.s == nil)
    nis = NilInStruct { 2, "y" }
    test_eq(assert nis.s, "y")

    // Nilables in vectors, of strings and of classes.
    let nv = [ nil, "b" ]
    test_true(nv[0] == nil)
    test_eq(assert nv[1], "b")
    let nov = [ nil, DNBox {} ]
    test_true(nov[0] == nil)
    test_eq((assert nov[1]).val, 7)
    var nilcount = 0
    for(nov) e:
        if not e: nilcount++
    test_eq(nilcount, 1)

    // nil as a default argument value for a nilable parameter.
    def with_nil_default(a, b:string? = nil):
        return if b: a + b else: a
    test_eq(with_nil_default("x"), "x")
    test_eq(with_nil_default("x", "y"), "xy")

    // Functions returning union of nil and value infer a nilable type.
    def union_ret(n):
        for(3) i:
            if i == n: return "found"
        return nil
    test_true(union_ret(1))
    test_true(union_ret(5) == nil)

    // Builtins with nilable int args accept plain ints (in_range bias).
    test_true(in_range(0, 1, 0))

    // Strings compare by value but still accept nil.
    def nilstr_is_w(a:string?): return a == "w"
    test_true(nilstr_is_w(substring("xw", 1, 1)))
    let fresh = substring("xw", 1, 1)
    test_true(fresh == "w")  // Same comparison, by value, when non-nil.

    // What holds after a branching statement is what holds on every path that
    // can reach it, so assigning non-nil on all of them promotes.
    class NVal:
        nv = 3
    var fm1:NVal? = nil
    if not fm1:
        fm1 = NVal {}
    test_eq(fm1.nv, 3)  // No assert needed.
    var fm2:NVal? = nil
    if rnd(2): fm2 = NVal { nv: 1 } else: fm2 = NVal { nv: 2 }
    test_true(fm2.nv > 0)
    var fm3:NVal? = nil
    switch rnd(3):
        case 0: fm3 = NVal { nv: 4 }
        case 1: fm3 = NVal { nv: 5 }
        default: fm3 = NVal { nv: 6 }
    test_true(fm3.nv > 3)
    // A branch that returns can't reach the code after it, so what it did to
    // the flow types does not count there.
    var fm4:NVal? = NVal { nv: 7 }
    def early_out() -> int:
        if fm4:
            if rnd(2):
                fm4 = nil
                return 0
            return fm4.nv  // Only reached when the branch above wasn't taken.
        return -1
    test_true(early_out() >= 0)
    // An assignment in one branch does not demote for a sibling branch, which
    // it cannot have run before.
    var fm5:NVal? = NVal { nv: 8 }
    if fm5:
        if rnd(2):
            fm5 = nil
        elif fm5.nv > 0:
            test_true(true)

nil_tests()
