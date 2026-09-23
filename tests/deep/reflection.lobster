import std

import vec

// Reflection and type-system features: typeof, type_* builtins, is,
// compile-time if, string conversion preferences.

class DRefl:
    c = float2_1
    s = "hello"
    v = [ 2 ]

class DReflSub : DRefl
    extra = 1

def dref_as_base(x:DRefl) -> DRefl: return x

def reflection_tests():
    suite("reflection")

    // is on scalars and references.
    test_true(1 is int)
    test_true(1.5 is float)
    test_true("s" is string)
    test_true([ 1 ] is [int])
    test_true(int2_1 is int2)
    test_true(not (1 is float))
    test_true(not (1.5 is int))
    // `is` on a value whose type no use settled yet (as an element of a `[]` or a `nil` may
    // have) decides only where no type the value could still get changes the outcome, and
    // doesn't settle the type (see the errors below).
    var nu = nil
    test_true(not (nu is int))  // A nil can't get a type that can't be nilable.
    test_true(nu is int?)       // Which leaves a nil check.
    let vu = []
    test_true(not (vu is string))
    nu = "s"
    vu.push(1.5)
    test_true(not (nu is int?))
    test_true(vu[0] is float)
    // A test decided at compile time is a bool like one decided at run time, also where the
    // side effects of the tested value are kept.
    test_eq("{1 is int}", "true")
    test_eq(string(1 is float), "false")
    let isb = 1 is int
    test_true(isb is bool)
    test_true((1 is int and 1.5 is float) is bool)
    var isn = 0
    def is_bump():
        isn++
        return 1
    test_eq("{is_bump() is int}", "true")
    test_eq(isn, 1)
    def is_ov(b:bool): return "bool"
    def is_ov(i:int): return "int"
    test_eq(is_ov(1 is int), "bool")
    let isv = [ 1 is int, dref_as_base(DReflSub {}) is DReflSub ]
    test_true(isv is [bool])

    // Compile-time if: branches with type errors for a given specialization
    // are culled, and the return type is just the taken branch.
    def ct_if(x):
        return if x is int or x is float:
            1 / x
        else:
            x
    test_true(ct_if(1) is int)
    test_true(ct_if("") is string)
    test_eq(ct_if(8), 0)
    test_eq(ct_if("s"), "s")

    // typeof: int is always the first entry in the type table.
    test_eq(typeof int, 0)
    // typeof also works on a variable, yielding the typeid of its type. It
    // must be a variable whose value is not a compile-time constant though:
    var tv = 1
    tv = length([ tv ])  // Defeat constant propagation.
    test_eq(typeof tv, typeof int)
    var tvs = "s"
    tvs = tvs + string(tv)
    test_eq(typeof tvs, typeof string)
    // typeof on an ident that will get constant-folded.
    let tconst = 1
    test_eq(typeof tconst, typeof int)

    // type_string / type_element_string / type_id.
    let r = DRefl {}
    test_eq(type_string(r), "DRefl")
    test_eq(type_string([ 1 ]), "[int]")
    test_eq(type_string("x"), "string")
    test_eq(type_element_string(r.v), "int")
    test_eq(type_element_string([ 1.5 ]), "float")
    test_eq(type_id(r), type_id(DRefl {}))
    test_true(type_id([ 1 ]) != type_id([ 1.5 ]))
    // Dynamic type of a subclass value, seen through a superclass type.
    test_true(type_id(dref_as_base(DReflSub {})) == type_id(DReflSub {}))

    // type_field_*: structs count as multiple inline fields.
    test_eq(type_field_count(r), 4)  // c expands to x, y.
    test_eq(type_field_count("str"), 0)
    test_eq(type_field_count([ 1 ]), 0)
    test_eq(type_field_string(r, 0), "float")
    test_eq(type_field_string(r, 3), "[int]")
    test_eq(type_field_name(r, 2), "s")
    test_eq(type_field_value(r, 2), "hello")
    test_eq(type_field_value(r, 0), "1.0")
    // A subclass value reports its own (longer) field list.
    let rsub = DReflSub {}
    test_eq(type_field_count(rsub), 5)
    test_eq(type_field_name(rsub, 4), "extra")

    // Printing preferences: setters return the old value, effects apply to
    // string conversion of reference values. All restored afterwards.
    let vnest = [ [ [ 1 ] ] ]
    let olddepth = set_print_depth(1)
    let nested1 = "{vnest}"
    set_print_depth(olddepth)
    test_eq(nested1, "[[..]]")
    test_eq("{vnest}", "[[[1]]]")
    let olddec = set_print_decimals(2)
    let decs = "{1.0 / 3.0}"
    set_print_decimals(olddec)
    test_eq(decs, "0.33")
    test_eq("{1.0 / 4.0}", "0.25")
    // quoted applies to top level strings converted inside the VM,
    // observable via type_field_value.
    let oldquoted = set_print_quoted(true)
    let quoted = type_field_value(r, 2)
    set_print_quoted(oldquoted)
    test_eq(quoted, "\"hello\"")
    // Strings nested in values are always quoted.
    test_eq("{[ "a" ]}", "[\"a\"]")
    // length budget truncates long conversions with "..".
    let oldlen = set_print_length(4)
    let longstr = string([ "aaaaaaaaaa" ])
    set_print_length(oldlen)
    test_eq(longstr, "[\"aaaa\"..]")
    let oldind = set_print_indent(4)
    let indented = "{[ [ 1, 2 ] ]}"
    set_print_indent(oldind)
    test_eq(indented, "[\n    [\n        1,\n        2\n    ]\n]")

    // set_print_* return the previous defaults.
    test_eq(olddepth, 10)
    test_eq(olddec, -1)
    test_eq(oldquoted, false)
    test_eq(oldlen, 100000)
    test_eq(oldind, 0)

    // Compile errors (see typecheck_recovery.lobster for the mechanism).
    guard not vm_compiled_mode()
    // `is` on a value whose type no use settled yet, where the outcome depends on it. That
    // the test does not settle it is why `push` gets no error, and the promotion of `n` in
    // the branch still happens.
    expect_errors("""
        class C:
            x = 1
        let v = []
        let n = nil
        print(v[0] is int)
        print(if n is C: n.x else: 0)
        print(v is [int])
        v.push("s")
        return 0
        """, [ "`is` cannot test a value whose type is not known yet: `unknown`",
               "`is` cannot test a value whose type is not known yet: `nil`",
               "`is` cannot test a value whose type is not known yet: `[]`" ])
    // The same for a tested type no use settled yet.
    expect_errors("""
        def f<T>(a:T, b): return b is T
        return f([], [ 1 ])
        """, [ "`is` cannot test against a type that is not known yet: `[]`",
               "can\'t infer `1st` argument of call to `f`" ])

reflection_tests()
