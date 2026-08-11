
import vec

// Byte-level string builtins: ensure_size, write_/read_ int/float variants,
// substring writes/compares, vector_to_buffer.

// A struct with mixed int/float fields for vector_to_buffer.
struct DBMix:
    i:int
    f:float

def binary_tests():
    suite("binary")

    // ensure_size: grows to size with fill char, negative size fills at the start.
    test_eq(ensure_size("abc", 3, 'z'), "abc")
    test_eq(ensure_size("abc", 5, 'z'), "abczz")
    test_eq(ensure_size("abc", -5, 'z'), "zzabc")
    test_eq(ensure_size("", 3, 'x'), "xxx")
    test_eq(ensure_size("ab", 4, 'x', 2), "abxxxx")  // Extra bytes on top.

    // write_int*_le: little endian, auto-growing, returns (string, next index).
    var ws, wi = "".write_int16_le(0, 1027)
    test_eq(ws, "\x03\x04\x00\x00")  // Doubled in size while growing.
    test_eq(wi, 2)
    test_eq(ws.read_int16_le(0), 1027)
    test_eq(ws.read_int32_le(0), 1027)
    ws, wi = ws.write_int16_le(wi, 513)
    test_eq(wi, 4)
    test_eq(ws.read_int16_le(2), 513)
    // 8/32/64 bit, negative values, unsigned reads.
    ws, wi = "".write_int8_le(0, -1)
    test_eq(ws.read_int8_le(0), -1)
    test_eq(ws.read_uint8_le(0), 255)
    ws, wi = "".write_int32_le(0, -2)
    test_eq(ws.read_int32_le(0), -2)
    test_eq(ws.read_uint32_le(0), 0xFFFFFFFE)
    ws, wi = "".write_int64_le(0, 0x1122334455667788)
    test_eq(wi, 8)
    test_eq(ws.read_int64_le(0), 0x1122334455667788)
    test_eq(ws.read_int8_le(0), 0x88 - 256)  // Low byte first: little endian.
    test_eq(ws.read_uint8_le(7), 0x11)
    // read_uint64_le of a negative value stays the same bit pattern.
    ws, wi = "".write_int64_le(0, -1)
    test_eq(ws.read_uint64_le(0), -1)

    // _back variants: relative to the end, writing before the index.
    ws, wi = "".write_int16_le_back(0, 2050)
    test_eq(ws, "\x00\x00\x02\x08")
    test_eq(wi, 2)
    test_eq(ws.read_int16_le_back(0), 2050)

    // Floats, 32 and 64 bit, also the _back variants.
    ws, wi = "".write_float32_le(0, 1.5)
    test_eq(wi, 4)
    test_eq(ws.read_float32_le(0), 1.5)
    ws, wi = "".write_float64_le(0, -1.25e10)
    test_eq(wi, 8)
    test_eq(ws.read_float64_le(0), -1.25e10)
    // float32 rounds to nearest representable value.
    ws, wi = "".write_float32_le(0, 0.1)
    test_true(abs(ws.read_float32_le(0) - 0.1) < 1.0e-7)
    ws, wi = "".write_float32_le_back(0, 2.5)
    test_eq(wi, 4)
    test_eq(ws.read_float32_le_back(0), 2.5)
    ws, wi = "".write_float64_le_back(0, -0.5)
    test_eq(ws.read_float64_le_back(0), -0.5)

    // write_substring, with and without 0-terminator.
    ws, wi = "".write_substring(0, "hi", false)
    test_eq(wi, 2)
    ws, wi = ws.write_substring(wi, "!", true)
    test_eq(wi, 4)  // One more for the terminator.
    test_eq(ws.substring(0, 3), "hi!")
    test_eq(ws.read_int8_le(3), 0)
    // write_substring_back writes ending at the given index from the end.
    ws, wi = "".write_substring_back(0, "ab", false)
    test_eq(wi, 2)
    test_eq(ws.length, 4)  // Doubled in size while growing.
    test_eq(ws.substring(2, 2), "ab")

    // compare_substring: 0 equal, -1/1 for less/greater.
    test_eq(compare_substring("xabcx", 1, "abc", 0, 3), 0)
    test_true(compare_substring("aab", 1, "abb", 1, 2) < 0)
    test_true(compare_substring("xb", 1, "xa", 1, 1) > 0)

    // vector_to_buffer: ints at various widths, floats, and structs.
    let vb1 = vector_to_buffer([ 1, 2 ], 1)
    test_eq(vb1.length, 2)
    test_eq(vb1.read_int8_le(1), 2)
    let vb2 = vector_to_buffer([ 258 ], 2)
    test_eq(vb2.length, 2)
    test_eq(vb2.read_int16_le(0), 258)
    let vb8 = vector_to_buffer([ 1, 2 ], 8)
    test_eq(vb8.length, 16)
    test_eq(vb8.read_int64_le(8), 2)
    let vbf = vector_to_buffer([ 1.5, 2.5 ], 4)
    test_eq(vbf.length, 8)
    test_eq(vbf.read_float32_le(4), 2.5)
    let vbf8 = vector_to_buffer([ 1.5, -2.5 ], 8)
    test_eq(vbf8.length, 16)
    test_eq(vbf8.read_float64_le(8), -2.5)
    let vbs = vector_to_buffer([ int2 { 1, 2 }, int2 { 3, 4 } ], 4)
    test_eq(vbs.length, 16)
    test_eq(vbs.read_int32_le(12), 4)
    // offset / len args.
    let vbo = vector_to_buffer([ 1, 2, 3, 4 ], 1, 1, 2)
    test_eq(vbo.length, 2)
    test_eq(vbo.read_int8_le(0), 2)
    // Mixed int/float struct fields keep their own formats.
    let vbm = vector_to_buffer([ DBMix { 7, 1.5 } ], 4)
    test_eq(vbm.read_int32_le(0), 7)
    test_eq(vbm.read_float32_le(4), 1.5)

binary_tests()
