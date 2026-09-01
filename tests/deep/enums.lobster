import std

import dictionary

// Enums: values, auto-increment, enum_flags, conversion, names, switches.

enum de_color:
    de_red = 3
    de_green      // 4
    de_blue = 10
    de_cyan       // 11

enum de_zero:
    de_a          // Starts at 0 without explicit value.
    de_b

enum de_neg:
    de_n = -3
    de_m          // Auto-increment continues from negative values: -2.

enum_flags de_bits:
    de_f1         // 1
    de_f2         // 2
    de_f4         // 4
    de_f16 = 16
    de_f32        // 32: continues doubling from the explicit value.

def enums_tests():
    suite("enums")

    // Values.
    test_eq(de_red, 3)
    test_eq(de_green, 4)
    test_eq(de_blue, 10)
    test_eq(de_cyan, 11)
    test_eq(de_a, 0)
    test_eq(de_b, 1)
    test_eq(de_f1, 1)
    test_eq(de_f2, 2)
    test_eq(de_f4, 4)
    test_eq(de_f16, 16)
    test_eq(de_f32, 32)
    test_eq(de_n, -3)
    test_eq(de_m, -2)

    // Enums are ints in any int context; arithmetic yields int.
    test_eq(de_red + 1, 4)
    test_true(de_green > de_red)
    test_eq(de_f1 | de_f4, 5)
    test_eq(min(de_red, de_green), 3)
    // enum_flags values combine and test like bit sets.
    let flags = de_f1 | de_f16
    test_true(flags & de_f16)
    test_true(not (flags & de_f4))
    test_eq(flags & ~de_f1, 16)
    test_eq(de_f2 ^ de_f2, 0)

    // Explicit int -> enum conversion; equality with the enum value.
    test_true(de_color(3) == de_red)
    test_true(de_bits(4) == de_f4)
    // A forged value that doesn't correspond to any case.
    let forged = de_zero(2)
    test_eq(forged, 2)

    // Names: string conversion and interpolation use the value name.
    test_eq(string(de_red), "de_red")
    test_eq("{de_cyan}", "de_cyan")
    test_eq("{de_zero(0)}", "de_a")

    // Enums in other type positions: vector elements, dictionary keys,
    // default arguments, class fields.
    let colv = [ de_red ]
    colv.push(de_blue)
    test_eq(colv[1], de_blue)
    test_true(colv[1] is de_color)
    let cold = dictionary<de_color, int>(7)
    cold.set(de_green, 5)
    test_eq(cold.get(de_green, -1), 5)
    test_eq(cold.get(de_cyan, -1), -1)
    def colarg(c = de_blue): return c
    test_eq(colarg(), de_blue)
    test_eq(colarg(de_red), de_red)
    class ColHolder:
        c = de_green
    test_eq((ColHolder {}).c, de_green)

    // bool is a regular enum.
    var flag = true
    flag = bool(1)
    test_eq(flag, true)
    test_eq("{true}", "true")
    test_eq("{false}", "false")
    test_true(bool(2) != false)
    test_eq(true + true, 2)  // Arithmetic on bools yields plain ints.

    // switch over an enum must be exhaustive (this compiles == test passes),
    // and a default may replace any number of cases.
    def colname(c:de_color): return switch c:
        case de_red: "r"
        case de_green: "g"
        case de_blue: "b"
        case de_cyan: "c"
    test_eq(colname(de_red), "r")
    test_eq(colname(de_color(11)), "c")
    def coldefault(c:de_color): return switch c:
        case de_red: "r"
        default: "other"
    test_eq(coldefault(de_blue), "other")
    def boolname(b:bool): return switch b:
        case true: "t"
        case false: "f"
    test_eq(boolname(true), "t")
    test_eq(boolname(2 == 2), "t")
    def colrange(c:de_color): return switch c:
        case de_red..de_green: "low"
        case de_blue..de_cyan: "high"
    test_eq(colrange(de_red), "low")
    test_eq(colrange(de_green), "low")
    test_eq(colrange(de_blue), "high")

    // `out_of_range` runs for values that aren't the enum's, and only those. Unlike `default`
    // it doesn't switch off exhaustiveness checking (see errors.lobster), so all four cases
    // below are still required. de_color is dense enough for the jump table version.
    def coloor(c:de_color): return switch c:
        case de_red: "r"
        case de_green: "g"
        case de_blue: "b"
        case de_cyan: "c"
        out_of_range: "?"
    test_eq(coloor(de_red), "r")
    test_eq(coloor(de_green), "g")
    test_eq(coloor(de_blue), "b")
    test_eq(coloor(de_cyan), "c")
    test_eq(coloor(de_color(0)), "?")     // Below the range of the values.
    test_eq(coloor(de_color(5)), "?")     // A hole between two of them.
    test_eq(coloor(de_color(999)), "?")   // Above the range.
    // de_zero has too few values for a jump table, so this is the linear compare version.
    def zerooor(z:de_zero): return switch z:
        case de_a: "a"
        case de_b: "b"
        out_of_range: "?"
    test_eq(zerooor(de_a), "a")
    test_eq(zerooor(de_b), "b")
    test_eq(zerooor(de_zero(2)), "?")
    // The value decays to a plain int inside the block, since it is by definition not one of
    // the enum's values. Overload selection on its static type observes this, and string
    // conversion gives the number rather than a name.
    def oor_type(x:de_color): return "de_color"
    def oor_type(x:int): return "int"
    def coltype(c:de_color): return switch c:
        case de_red, de_green, de_blue, de_cyan: "{oor_type(c)} {c}"
        out_of_range: "{oor_type(c)} {c} {c + 1}"
    test_eq(coltype(de_blue), "de_color de_blue")
    test_eq(coltype(de_color(42)), "int 42 43")
    // Only the block sees the int: after a switch every case can fall out of, the value is
    // an enum again.
    def coltype_after(c:de_color) -> string:
        var s = ""
        switch c:
            case de_red, de_green, de_blue, de_cyan: s = "in"
            out_of_range: s = "out"
        return "{s} {oor_type(c)}"
    test_eq(coltype_after(de_red), "in de_color")
    test_eq(coltype_after(de_color(42)), "out de_color")
    // Statements rather than a value, and `out_of_range` written before the cases.
    var oor_seen = -1
    def colstmt(c:de_color):
        switch c:
            out_of_range: oor_seen = c
            case de_red, de_green, de_blue, de_cyan: oor_seen = -1
    colstmt(de_cyan)
    test_eq(oor_seen, -1)
    colstmt(de_color(7))
    test_eq(oor_seen, 7)

    // Reflection on enum value names; forged values have no name.
    test_eq(type_enum_value_name(typeof bool, 1), "true")
    test_eq(type_enum_value_name(typeof de_color, 10), "de_blue")
    test_eq(type_enum_value_name(typeof de_color, 999), "")
    // type_enum_value_valid is the same test without the string allocation: true for exactly
    // those values type_enum_value_name gives a name to.
    test_true(type_enum_value_valid(typeof de_color, de_red))
    test_true(type_enum_value_valid(typeof de_color, de_green))
    test_true(type_enum_value_valid(typeof de_color, de_blue))
    test_true(type_enum_value_valid(typeof de_color, de_cyan))
    test_true(not type_enum_value_valid(typeof de_color, 0))
    test_true(not type_enum_value_valid(typeof de_color, 5))
    test_true(not type_enum_value_valid(typeof de_color, 999))
    test_true(type_enum_value_valid(typeof de_zero, de_a))    // A value of 0 that is declared.
    test_true(not type_enum_value_valid(typeof de_zero, 2))
    test_true(type_enum_value_valid(typeof de_neg, de_n))     // Negative values.
    test_true(not type_enum_value_valid(typeof de_neg, -1))
    test_true(type_enum_value_valid(typeof bool, false))
    test_true(not type_enum_value_valid(typeof bool, 2))
    // For an enum_flags, any combination of declared bits is a value, as for the name.
    test_true(type_enum_value_valid(typeof de_bits, de_f1))
    test_true(type_enum_value_valid(typeof de_bits, de_f1 | de_f4))
    test_true(type_enum_value_valid(typeof de_bits, de_f1 | de_f2 | de_f4 | de_f16 | de_f32))
    test_true(not type_enum_value_valid(typeof de_bits, 8))            // Undeclared bit.
    test_true(not type_enum_value_valid(typeof de_bits, de_f1 | 8))    // Declared and not.
    test_true(not type_enum_value_valid(typeof de_bits, 0))            // No bits set at all.
    // Nothing has a name in a type that isn't an enum.
    test_true(not type_enum_value_valid(typeof int, 1))
    test_true(not type_enum_value_valid(typeof string, 1))
    for(20) i:
        let v = i - 5
        test_eq(type_enum_value_valid(typeof de_color, v),
                type_enum_value_name(typeof de_color, v).length != 0)
        test_eq(type_enum_value_valid(typeof de_bits, v),
                type_enum_value_name(typeof de_bits, v).length != 0)

    // Enums in data: parse_data round trips names, unknown names error.
    let pv, perr = parse_data(typeof [bool], "[true, false]")
    test_eq(perr, nil)
    test_seq(assert pv, [ true, false ])
    let bv, berr = parse_data(typeof [de_color], "[de_red, de_cyan]")
    test_eq(berr, nil)
    test_seq(assert bv, [ de_red, de_cyan ])
    let uv, uerr = parse_data(typeof [de_color], "[de_no_such_value]")
    test_true(uv == nil)
    test_true(uerr and uerr.length)

    // and/or on an enum (e.g. bool) mixed with a plain int (or another enum)
    // forces the result type to bool, rather than letting the enum decay to int.
    // Overload selection on the static type of the result observes this.
    def andor_type(x:bool): return "bool"
    def andor_type(x:int): return "int"
    def andor_type(x:de_color): return "de_color"
    test_eq(andor_type(1 > 0 and 1), "bool")
    test_eq(andor_type(1 and 1 > 0), "bool")
    test_eq(andor_type(1 > 0 or 1), "bool")
    test_eq(andor_type(de_red and 1), "bool")      // Enum and int.
    test_eq(andor_type(de_red and 1 > 0), "bool")  // Two different enums.
    test_eq(andor_type(de_red and de_blue), "de_color")  // Same enum is preserved.
    test_eq(andor_type(1 and 2), "int")            // Plain ints are preserved.
    // The and-or selection idiom still yields the value types.
    test_eq(andor_type(1 > 2 and 5 or 6), "int")
    test_eq(1 > 2 and 5 or 6, 6)
    test_eq(1 < 2 and 5 or 6, 5)
    // Truthiness semantics are unchanged.
    test_true(1 > 0 and 42)
    test_true(not (1 > 2 and 42))
    test_eq(de_red and de_blue, de_blue)

enums_tests()
