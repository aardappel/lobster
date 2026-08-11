import std


// The flatbuffers.* builtins: generating binary flatbuffers from JSON with a
// runtime-parsed schema, converting back to JSON, and reading fields directly
// from the binary with the field accessors.

def flatbuffers_tests():
    suite("flatbuffers")

    // Schema with scalar fields, a string and a vector. The first table field
    // has vtable offset 4, each subsequent field 2 more.
    let schema = "table T \{ i:int; s:string; v:[int]; b:bool; \} root_type T;"
    let fbin, fberr = flatbuffers.json_to_binary(
        schema, "\{ i: 42, s: \"hi\", v: [ 1, 2, 3 ], b: true \}", [])
    test_eq(fberr, nil)
    test_true(fbin.length > 0)
    // The root table offset sits at index 0.
    let ti = fbin.flatbuffers.indirect(0)
    test_true(ti > 0)
    // Present fields, with their values.
    test_eq(fbin.flatbuffers.field_present(ti, 4), true)
    test_eq(fbin.flatbuffers.field_int32(ti, 4, -1), 42)
    test_eq(fbin.flatbuffers.field_string(ti, 6), "hi")
    test_eq(fbin.flatbuffers.field_int8(ti, 10, -1), 1)
    // The vector field: length, then elements at the returned offset.
    test_eq(fbin.flatbuffers.field_vector_len(ti, 8), 3)
    let voff = fbin.flatbuffers.field_vector(ti, 8)
    test_eq(fbin.read_int32_le(voff), 1)
    test_eq(fbin.read_int32_le(voff + 4), 2)
    test_eq(fbin.read_int32_le(voff + 8), 3)

    // Missing fields: not present, accessors return the default.
    let mbin, mberr = flatbuffers.json_to_binary(schema, "\{ i: 1 \}", [])
    test_eq(mberr, nil)
    let mti = mbin.flatbuffers.indirect(0)
    test_eq(mbin.flatbuffers.field_present(mti, 6), false)
    test_eq(mbin.flatbuffers.field_int32(mti, 10, -7), -7)
    test_eq(mbin.flatbuffers.field_string(mti, 6), "")
    test_eq(mbin.flatbuffers.field_vector_len(mti, 8), 0)

    // Round trip back to JSON preserves the values.
    let json, jerr = flatbuffers.binary_to_json(schema, fbin, [])
    test_eq(jerr, nil)
    test_true(json.find_string("i: 42") >= 0)
    test_true(json.find_string("\"hi\"") >= 0)

    // Nested tables, structs, and vectors of strings.
    let schema2 = "struct P \{ x:int; y:int; \} table Inner \{ j:int; \} " +
                  "table T2 \{ inner:Inner; p:P; names:[string]; \} root_type T2;"
    let fbin2, fberr2 = flatbuffers.json_to_binary(
        schema2, "\{ inner: \{ j: 7 \}, p: \{ x: 1, y: 2 \}, names: [ \"aa\", \"bb\" ] \}", [])
    test_eq(fberr2, nil)
    let ti2 = fbin2.flatbuffers.indirect(0)
    // A table field yields another table location for the field accessors.
    let inner = fbin2.flatbuffers.field_table(ti2, 4)
    test_true(inner > 0)
    test_eq(fbin2.flatbuffers.field_int32(inner, 4, -1), 7)
    // A struct field is stored inline: read its members directly.
    let p = fbin2.flatbuffers.field_struct(ti2, 6)
    test_eq(fbin2.read_int32_le(p), 1)
    test_eq(fbin2.read_int32_le(p + 4), 2)
    // A vector of strings holds offsets; flatbuffers.string follows one.
    test_eq(fbin2.flatbuffers.field_vector_len(ti2, 8), 2)
    let noff = fbin2.flatbuffers.field_vector(ti2, 8)
    test_eq(fbin2.flatbuffers.string(noff), "aa")
    test_eq(fbin2.flatbuffers.string(noff + 4), "bb")

    // All scalar field widths, signed and unsigned, and both float widths.
    let schema3 = "table W \{ i8:byte; u8:ubyte; i16:short; u16:ushort; " +
                  "i32:int; u32:uint; i64:long; u64:ulong; f32:float; f64:double; \} root_type W;"
    let wjson = "\{ i8: -1, u8: 255, i16: -2, u16: 65535, i32: -3, u32: 4000000000, " +
                "i64: -4, u64: 5000000000, f32: 1.5, f64: -2.5 \}"
    let wbin, wberr = flatbuffers.json_to_binary(schema3, wjson, [])
    test_eq(wberr, nil)
    let wti = wbin.flatbuffers.indirect(0)
    test_eq(wbin.flatbuffers.field_int8(wti, 4, 0), -1)
    test_eq(wbin.flatbuffers.field_uint8(wti, 6, 0), 255)
    test_eq(wbin.flatbuffers.field_int16(wti, 8, 0), -2)
    test_eq(wbin.flatbuffers.field_uint16(wti, 10, 0), 65535)
    test_eq(wbin.flatbuffers.field_int32(wti, 12, 0), -3)
    test_eq(wbin.flatbuffers.field_uint32(wti, 14, 0), 4000000000)
    test_eq(wbin.flatbuffers.field_int64(wti, 16, 0), -4)
    test_eq(wbin.flatbuffers.field_uint64(wti, 18, 0), 5000000000)
    test_eq(wbin.flatbuffers.field_float32(wti, 20, 0.0), 1.5)
    test_eq(wbin.flatbuffers.field_float64(wti, 22, 0.0), -2.5)

    // A malformed schema reports an error.
    let ebin, eerr = flatbuffers.json_to_binary("this is not a schema", "\{\}", [])
    test_true(eerr and eerr.length)
    test_eq(ebin, "")
    // As does JSON not matching the schema.
    let jbin, jberr = flatbuffers.json_to_binary(schema, "\{ nofield: 1 \}", [])
    test_true(jberr and jberr.length)
    test_eq(jbin, "")

flatbuffers_tests()
