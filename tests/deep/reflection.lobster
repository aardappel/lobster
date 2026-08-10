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
    test_eq(type_field_string(r, 0), "float")
    test_eq(type_field_string(r, 3), "[int]")
    test_eq(type_field_name(r, 2), "s")
    test_eq(type_field_value(r, 2), "hello")
    test_eq(type_field_value(r, 0), "1.0")

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

reflection_tests()
