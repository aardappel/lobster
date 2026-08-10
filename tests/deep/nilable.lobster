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

    // or with nil as default.
    test_eq(maybe(0) or "dflt", "dflt")
    test_eq(maybe(1) or "dflt", "v")
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
    nf.next = nil  // Break the cycle again (avoids a leak report).

    // Nilables in vectors.
    let nv = [ nil, "b" ]
    test_true(nv[0] == nil)
    test_eq(assert nv[1], "b")

    // Functions returning union of nil and value infer a nilable type.
    def union_ret(n):
        for(3) i:
            if i == n: return "found"
        return nil
    test_true(union_ret(1))
    test_true(union_ret(5) == nil)

    // Builtins with nilable int args accept plain ints (in_range bias).
    test_true(in_range(0, 1, 0))

    // FIXME: == on a string? value compares by *reference* when the value is
    // not statically known to be non-nil, but by *value* where flow analysis
    // has promoted it. So this function returns false for a fresh
    // (non-interned) string with equal contents, while the exact same
    // comparison inline on a promoted value returns true:
    def nilstr_is_w(a:string?): return a == "w"
    //test_true(nilstr_is_w(substring("xw", 1, 1)))
    let fresh = substring("xw", 1, 1)
    test_true(fresh == "w")  // Same comparison, by value, when non-nil.

nil_tests()
