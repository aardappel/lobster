import std


// Enums: values, auto-increment, enum_flags, conversion, names, switches.

enum de_color:
    de_red = 3
    de_green      // 4
    de_blue = 10
    de_cyan       // 11

enum de_zero:
    de_a          // Starts at 0 without explicit value.
    de_b

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

    // Enums are ints in any int context; arithmetic yields int.
    test_eq(de_red + 1, 4)
    test_true(de_green > de_red)
    test_eq(de_f1 | de_f4, 5)

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

    // bool is a regular enum.
    var flag = true
    flag = bool(1)
    test_eq(flag, true)
    test_eq("{true}", "true")
    test_eq("{false}", "false")
    test_true(bool(2) != false)

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

    // Reflection on enum value names.
    test_eq(type_enum_value_name(typeof bool, 1), "true")
    test_eq(type_enum_value_name(typeof de_color, 10), "de_blue")

    // Enums in data: parse_data round trips names.
    let pv, perr = parse_data(typeof [bool], "[true, false]")
    test_eq(perr, nil)
    test_seq(assert pv, [ true, false ])

enums_tests()
