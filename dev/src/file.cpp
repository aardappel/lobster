// Copyright 2014 Wouter van Oortmerssen. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lobster/stdafx.h"

#include "lobster/natreg.h"

#include "stdint.h"

#ifdef _WIN32
    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#else
    #ifndef __ANDROID__
        #include <glob.h>
    #endif
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace lobster {

template<typename T, bool B> T Read(VM &vm, intp i, const LString *s) {
    if ((uintp)i > (uintp)s->len - sizeof(T)) vm.IDXErr(i, s->len - sizeof(T), s);
    return ReadValLE<T, B>(s, i);
}

template<typename T, bool B, bool IF> Value WriteVal(VM &vm, const Value &str, const Value &idx,
                                                     const Value &val) {
    auto i = idx.ival();
    if (i < 0) vm.IDXErr(i, 0, str.sval());
    vm.Push(WriteValLE<T, B>(vm, str.sval(), i, val.ifval<T, IF>()));
    return Value(i + (intp)sizeof(T));
}

template<bool B> Value WriteStr(VM &vm, const Value &str, const Value &idx, const LString *s,
                                intp extra) {
    auto i = idx.ival();
    if (i < 0) vm.IDXErr(i, 0, str.sval());
    vm.Push(WriteMem<B>(vm, str.sval(), i, s->data(), s->len + extra));
    return Value(i + s->len + extra);
}

template<typename T, bool B> Value ReadVal(VM &vm, const Value &str, const Value &idx) {
    auto i = idx.ival();
    auto val = Read<T, B>(vm, i, str.sval());
    str.DECRT(vm);
    vm.Push(val);
    return Value(i + (intp)sizeof(T));
}

template<typename T, bool IF, bool OF, bool ST>
Value ReadField(VM &vm, const Value &str, const Value &idx, const Value &vidx, const Value &def) {
    auto i = idx.ival();
    auto vtable = Read<flatbuffers::soffset_t, false>(vm, i, str.sval());
    auto vi = i - vtable;
    auto vtable_size = Read<flatbuffers::voffset_t, false>(vm, vi, str.sval());
    auto vo = vidx.ival();
    if ((uintp)vo < (uintp)vtable_size) {
        auto field_offset = Read<flatbuffers::voffset_t, false>(vm, vi + vo, str.sval());
        if (field_offset) {
            auto start = i + field_offset;
            if constexpr (ST) return Value(start);
            auto val = Read<T, false>(vm, start, str.sval());
            if constexpr (OF) return Value (val + start);
            return Value(val);
        }
    }
    return def;
}

LString *GetString(VM &vm, intp fi, LString *buf) {
    if (fi) {
        auto len = Read<flatbuffers::uoffset_t, false>(vm, fi, buf);
        auto fdata = fi + sizeof(flatbuffers::uoffset_t);
        // Read zero terminator just to make sure all string data is in bounds.
        Read<char, false>(vm, fdata + len, buf);
        return vm.NewString(buf->strv().substr(fdata, len));
    } else {
        return vm.NewString(0);
    }
}

void AddFile(NativeRegistry &natreg) {
    STARTDECL(scan_folder) (VM &vm, Value &fld, Value &divisor) {
        vector<pair<string, int64_t>> dir;
        auto ok = ScanDirAbs(fld.sval()->strv(), dir);
        fld.DECRT(vm);
        if (!ok) {
            vm.Push(Value());
            return Value();
        }
        if (divisor.ival() <= 0) divisor.setival(1);
        auto nlist = (LVector *)vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);
        auto slist = (LVector *)vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_INT);
        for (auto &[name, size] : dir) {
            nlist->Push(vm, Value(vm.NewString(name)));
            if (size >= 0) {
                size /= divisor.ival();
                if (sizeof(intp) == sizeof(int) && size > 0x7FFFFFFF) size = 0x7FFFFFFF;
            }
            slist->Push(vm, Value(size));
        }
        vm.Push(Value(nlist));
        return Value(slist);
    }
    ENDDECL2(scan_folder, "folder,divisor", "SI", "S]?I]?",
        "returns two vectors representing all elements in a folder, the first vector containing all"
        " names, the second vector containing sizes (or -1 if a directory)."
        " Specify 1 as divisor to get sizes in bytes, 1024 for kb etc. Values > 0x7FFFFFFF will be"
        " clamped in 32-bit builds. Returns nil if folder couldn't be scanned.");

    STARTDECL(read_file) (VM &vm, Value &file) {
        string buf;
        auto l = LoadFile(file.sval()->strv(), &buf);
        file.DECRT(vm);
        if (l < 0) return Value();
        auto s = vm.NewString(buf);
        return Value(s);
    }
    ENDDECL1(read_file, "file", "S", "S?",
        "returns the contents of a file as a string, or nil if the file can't be found."
        " you may use either \\ or / as path separators");

    STARTDECL(write_file) (VM &vm, Value &file, Value &contents) {
        auto ok = WriteFile(file.sval()->strv(), true, contents.sval()->strv());
        file.DECRT(vm);
        contents.DECRT(vm);
        return Value(ok);
    }
    ENDDECL2(write_file, "file,contents", "SS", "I",
        "creates a file with the contents of a string, returns false if writing wasn't possible");

    STARTDECL(ensure_size) (VM &vm, Value &str, Value &size, Value &c, Value &extra) {
        auto asize = abs(size.ival());
        return str.sval()->len >= asize
            ? str
            : Value(vm.ResizeString(str.sval(), asize + extra.ival(), c.intval(), size.ival() < 0));
    }
    ENDDECL4(ensure_size, "string,size,char,extra", "SIII?", "S",
             "ensures a string is at least size characters. if it is, just returns the existing"
             " string, otherwise returns a new string of that size (with optionally extra bytes"
             " added), with any new characters set to"
             " char. You can specify a negative size to mean relative to the end, i.e. new"
             " characters will be added at the start. ");

    auto write_val_desc1 =
        "writes a value as little endian to a string at location i. Uses ensure_size to"
        " make the string twice as long (with extra 0 bytes) if no space. Returns"
        " new string if resized,"
        " and the index of the location right after where the value was written. The"
        " _back version writes relative to the end (and writes before the index)";
    auto write_val_desc2 = "(see write_int64_le)";
    #define WRITEOP(N, T, B, D, S) \
    STARTDECL(N) (VM &vm, Value &str, Value &idx, Value &val) { \
        return WriteVal<T, B, S[0] == 'F'>(vm, str, idx, val); \
    } \
    ENDDECL3(N, "string,i,val", "SI" S, "SI", D);
    WRITEOP(write_int64_le, int64_t, false, write_val_desc1, "I")
    WRITEOP(write_int32_le, int32_t, false, write_val_desc2, "I")
    WRITEOP(write_int16_le, int16_t, false, write_val_desc2, "I")
    WRITEOP(write_int8_le, int8_t, false, write_val_desc2, "I")
    WRITEOP(write_float64_le, double, false, write_val_desc2, "F")
    WRITEOP(write_float32_le, float, false, write_val_desc2, "F")
    WRITEOP(write_int64_le_back, int64_t, true, write_val_desc2, "I")
    WRITEOP(write_int32_le_back, int32_t, true, write_val_desc2, "I")
    WRITEOP(write_int16_le_back, int16_t, true, write_val_desc2, "I")
    WRITEOP(write_int8_le_back, int8_t, true, write_val_desc2, "I")
    WRITEOP(write_float64_le_back, double, true, write_val_desc2, "F")
    WRITEOP(write_float32_le_back, float, true, write_val_desc2, "F")

    STARTDECL(write_substring) (VM &vm, Value &str, Value &idx, Value &val, Value &term) {
        return WriteStr<false>(vm, str, idx, val.sval(), term.True());
    }
    ENDDECL4(write_substring, "string,i,substr,nullterm", "SISI", "SI",
             "writes a substring into another string at i (see also write_int64_le)");
    STARTDECL(write_substring_back) (VM &vm, Value &str, Value &idx, Value &val, Value &term) {
        return WriteStr<true>(vm, str, idx, val.sval(), term.True());
    }
    ENDDECL4(write_substring_back, "string,i,substr,nullterm", "SISI", "SI", "");

    STARTDECL(compare_substring) (VM &vm, Value &str1, Value &idx1, Value &str2, Value &idx2,
                                  Value &len) {
        auto s1 = str1.sval();
        auto s2 = str2.sval();
        auto i1 = idx1.ival();
        auto i2 = idx2.ival();
        auto l = len.ival();
        if (l < 0 || i1 < 0 || i2 < 0 || i1 + l > s1->len || i2 + l > s2->len)
            vm.Error("compare_substring: index out of bounds");
        auto eq = memcmp(s1->data() + i1, s2->data() + i2, l);
        return eq;
    }
    ENDDECL5(compare_substring, "string_a,i_a,string_b,i_b,len", "SISII", "I",
             "returns if the two substrings are equal (0), or a < b (-1) or a > b (1).");

    auto read_val_desc1 =
        "reads a value as little endian from a string at location i. The value must be within"
        " bounds of the string. Returns the value, and the index of the location right after where"
        " the value was read. The"
        " _back version reads relative to the end (and reads before the index)";
    auto read_val_desc2 = "(see read_int64_le)";
    #define READOP(N, T, B, D, S) \
    STARTDECL(N) (VM &vm, Value &str, Value &idx) { return ReadVal<T, B>(vm, str, idx); } \
    ENDDECL2(N, "string,i", "SI", S "I", D);
    READOP(read_int64_le, int64_t, false, read_val_desc1, "I")
    READOP(read_int32_le, int32_t, false, read_val_desc2, "I")
    READOP(read_int16_le, int16_t, false, read_val_desc2, "I")
    READOP(read_int8_le, int8_t, false, read_val_desc2, "I")
    READOP(read_float64_le, double, false, read_val_desc2, "F")
    READOP(read_float32_le, float, false, read_val_desc2, "F")
    READOP(read_int64_le_back, int64_t, true, read_val_desc2, "I")
    READOP(read_int32_le_back, int32_t, true, read_val_desc2, "I")
    READOP(read_int16_le_back, int16_t, true, read_val_desc2, "I")
    READOP(read_int8_le_back, int8_t, true, read_val_desc2, "I")
    READOP(read_float64_le_back, double, true, read_val_desc2, "F")
    READOP(read_float32_le_back, float, true, read_val_desc2, "F")

    auto read_field_desc1 =
        "reads a flatbuffers field from a string at table location tablei, field vtable offset vo,"
        " and default value def. The value must be within"
        " bounds of the string. Returns the value (or default if the field was not present)";
    auto read_field_desc2 = "(see flatbuffers_field_int64)";
    #define READFOP(N, T, D, S) \
    STARTDECL(N) (VM &vm, Value &str, Value &idx, Value &vidx, Value &def) { \
        auto val = ReadField<T, S[0] == 'F', false, false>(vm, str, idx, vidx, def); \
        str.DECRT(vm); \
        return Value(val); \
    } \
    ENDDECL4(N, "string,tablei,vo,def", "SII" S, S, D);
    READFOP(flatbuffers_field_int64, int64_t, read_field_desc1, "I")
    READFOP(flatbuffers_field_int32, int32_t, read_field_desc2, "I")
    READFOP(flatbuffers_field_int16, int16_t, read_field_desc2, "I")
    READFOP(flatbuffers_field_int8, int8_t, read_field_desc2, "I")
    READFOP(flatbuffers_field_float64, double, read_field_desc2, "F")
    READFOP(flatbuffers_field_float32, float, read_field_desc2, "F")
    STARTDECL(flatbuffers_field_string) (VM &vm, Value &str, Value &idx, Value &vidx) {
        auto fi = ReadField<flatbuffers::uoffset_t, false, true, false>(vm, str, idx, vidx,
                                                                        Value(0)).ival();
        auto ret = Value(GetString(vm, fi, str.sval()));
        str.DECRT(vm);
        return ret;
    }
    ENDDECL3(flatbuffers_field_string, "string,tablei,vo", "SII", "S",
             "reads a flatbuffer string field, returns \"\" if not present");
    STARTDECL(flatbuffers_field_vector_len) (VM &vm, Value &str, Value &idx, Value &vidx) {
        auto fi = ReadField<flatbuffers::uoffset_t, false, true, false>(vm, str, idx, vidx,
                                                                        Value(0)).ival();
        Value ret(fi ? Read<flatbuffers::uoffset_t, false>(vm, fi, str.sval()) : 0);
        str.DECRT(vm);
        return ret;
    }
    ENDDECL3(flatbuffers_field_vector_len, "string,tablei,vo", "SII", "I",
             "reads a flatbuffer vector field length, or 0 if not present");
    STARTDECL(flatbuffers_field_vector) (VM &vm, Value &str, Value &idx, Value &vidx) {
        auto fi = ReadField<flatbuffers::uoffset_t, false, true, false>(vm, str, idx, vidx,
                                                                        Value(0)).ival();
        Value ret(fi ? fi + sizeof(flatbuffers::uoffset_t) : 0);
        str.DECRT(vm);
        return ret;
    }
    ENDDECL3(flatbuffers_field_vector, "string,tablei,vo", "SII", "I",
             "returns a flatbuffer vector field element start, or 0 if not present");
    STARTDECL(flatbuffers_field_table) (VM &vm, Value &str, Value &idx, Value &vidx) {
        auto ret = ReadField<flatbuffers::uoffset_t, false, true, false>(vm, str, idx, vidx,
                                                                         Value(0));
        str.DECRT(vm);
        return ret;
    }
    ENDDECL3(flatbuffers_field_table, "string,tablei,vo", "SII", "I",
             "returns a flatbuffer table field start, or 0 if not present");
    STARTDECL(flatbuffers_field_struct) (VM &vm, Value &str, Value &idx, Value &vidx) {
        auto ret = ReadField<flatbuffers::uoffset_t, false, false, true>(vm, str, idx, vidx,
                                                                         Value(0));
        str.DECRT(vm);
        return ret;
    }
    ENDDECL3(flatbuffers_field_struct, "string,tablei,vo", "SII", "I",
             "returns a flatbuffer struct field start, or 0 if not present");
    STARTDECL(flatbuffers_indirect) (VM &vm, Value &str, Value &idx) {
        auto off = Read<flatbuffers::uoffset_t, false>(vm, idx.ival(), str.sval());
        str.DECRT(vm);
        return off + idx.ival();
    }
    ENDDECL2(flatbuffers_indirect, "string,index", "SI", "I",
             "returns a flatbuffer offset at index relative to itself");
    STARTDECL(flatbuffers_string) (VM &vm, Value &str, Value &idx) {
        auto off = Read<flatbuffers::uoffset_t, false>(vm, idx.ival(), str.sval());
        auto ret = GetString(vm, off + idx.ival(), str.sval());
        str.DECRT(vm);
        return ret;
    }
    ENDDECL2(flatbuffers_string, "string,index", "SI", "S",
             "returns a flatbuffer string whose offset is at given index");
}

}
