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

#include "flatbuffers/idl.h"

#ifdef _WIN32
    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#else
    #ifndef __ANDROID__
        #include <glob.h>
    #endif
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include <chrono>
#include <version>
#include <time.h>

namespace lobster {

template<typename T, bool B> T Read(VM &vm, iint i, const LString *s) {
    if ((uint64_t)i > (uint64_t)s->len - sizeof(T)) vm.IDXErr(i, s->len - sizeof(T), s);
    return ReadValLE<T, B>(s, i);
}

template<typename T, bool B> iint WriteVal(StackPtr &sp, VM &vm, Value str, Value idx,
                                           Value val) {
    auto i = idx.ival();
    if (i < 0) vm.IDXErr(i, 0, str.sval());
    Push(sp, WriteValLE<T, B>(vm, str.sval(), i, val.ifval<T>()));
    return i + ssizeof<T>();
}

template<bool B> iint WriteStr(StackPtr &sp, VM &vm, Value str, Value idx, LString *s,
                               iint extra) {
    auto i = idx.ival();
    if (i < 0) vm.IDXErr(i, 0, str.sval());
    Push(sp, WriteMem<B>(vm, str.sval(), i, s->data(), s->len + extra));
    return i + s->len + extra;
}

template<typename T, bool B> iint ReadVal(StackPtr &sp, VM &vm, Value str, Value idx) {
    auto i = idx.ival();
    auto val = Read<T, B>(vm, i, str.sval());
    Push(sp, val);
    return i + ssizeof<T>();
}

template<typename T, bool IF, bool OF, bool ST>
Value ReadField(VM &vm, Value str, Value idx, Value vidx, Value def) {
    auto i = idx.ival();
    auto vtable = Read<flatbuffers::soffset_t, false>(vm, i, str.sval());
    auto vi = i - vtable;
    auto vtable_size = Read<flatbuffers::voffset_t, false>(vm, vi, str.sval());
    auto vo = vidx.ival();
    if ((uint64_t)vo < (uint64_t)vtable_size) {
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

iint FieldPresent(VM &vm, Value str, Value idx, Value vidx) {
    auto i = idx.ival();
    auto vtable = Read<flatbuffers::soffset_t, false>(vm, i, str.sval());
    auto vi = i - vtable;
    auto vtable_size = Read<flatbuffers::voffset_t, false>(vm, vi, str.sval());
    auto vo = vidx.ival();
    if ((uint64_t)vo < (uint64_t)vtable_size) {
        auto field_offset = Read<flatbuffers::voffset_t, false>(vm, vi + vo, str.sval());
        if (field_offset) {
            return true;
        }
    }
    return false;
}

LString *GetString(VM &vm, iint fi, LString *buf) {
    if (fi) {
        auto len = Read<flatbuffers::uoffset_t, false>(vm, fi, buf);
        auto fdata = fi + ssizeof<flatbuffers::uoffset_t>();
        // Read zero terminator just to make sure all string data is in bounds.
        Read<char, false>(vm, fdata + len, buf);
        return vm.NewString(buf->strv().substr(fdata, len));
    } else {
        return vm.NewString(0);
    }
}

Value ParseSchemas(VM &vm, flatbuffers::Parser &parser, Value schema,
                   Value includes) {
    vector<string> dirs_storage;
    for (iint i = 0; i < includes.vval()->len; i++) {
        auto dir = flatbuffers::ConCatPathFileName(string(ProjectDir()),
                                                   string(includes.vval()->AtS(i).sval()->strv()));
        dirs_storage.push_back(dir);
    }
    vector<const char *> dirs;
    for (auto &dir : dirs_storage) dirs.push_back(dir.c_str());
    dirs.push_back(nullptr);
    if (!parser.Parse(schema.sval()->data(), dirs.data())) {
        return Value(vm.NewString(parser.error_));
    }
    return NilVal();
}

BuiltinGroup file_builtins;
#define BUILTIN_GROUP file_builtins
#define BUILTIN_SYM(name) builtin_##name

BUILTIN(format_time, "format,time,localtime", "SIB", "S",
    "convert a time in seconds since 00:00:00 UTC, Thursday, 1 January 1970 into a string,"
    " using the same format string syntax as POSIX strftime. If localtime is true, then"
    " the time will be displayed using the local timezone, otherwise it will use UTC."
    " Returns an empty string on error.")
(VM &vm, LString *format, iint time, iint use_localtime) {
    chrono::system_clock::time_point tp { chrono::seconds(time) };
    time_t tt = chrono::system_clock::to_time_t(tp);
    tm ctm{};
    bool ok = false;
    #ifdef _WIN32
        ok = ((use_localtime != 0) ? localtime_s(&ctm, &tt) : gmtime_s(&ctm, &tt)) == 0;
    #else
        ok = ((use_localtime != 0) ? localtime_r(&tt, &ctm) : gmtime_r(&tt, &ctm)) != nullptr;
    #endif
    if (!ok) return vm.NewString(0);
    // TODO: using strftime to avoid pulling in std::format(); maybe we should reconsider that?
    char buf[1024];
    auto written = strftime(buf, sizeof(buf), format->data(), &ctm);
    // TODO: written may be zero if the format string was too long; in that
    // case maybe we want to try again with a larger buf?
    if (written == 0) return vm.NewString(0);
    auto s = vm.NewString(buf);
    return s;
}

BUILTIN(scan_folder, "folder,rel", "SB?", "S]?I]?I]?",
    "returns three vectors representing all elements in a folder, the first vector containing all"
    " names, the second vector containing sizes in bytes (or -1 if a directory), and the third as"
    " the number of seconds since 00:00:00 UTC, Thursday, 1 January 1970, not including leap seconds."
    " set rel use a relative path, default is absolute."
    " Returns nil if folder couldn't be scanned.")
(StackPtr &sp, VM &vm, LString *fld, iint rel) {
    vector<DirectoryInfo> dir;
    auto ok = (rel != 0)
        ? ScanDir(fld->strv(), dir)
        : ScanDirAbs(fld->strv(), dir);
    if (!ok) {
        Push(sp, NilVal());
        Push(sp, NilVal());
        return nullptr;
    }
    auto nlist = (LVector *)vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);
    auto slist = (LVector *)vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_INT);
    auto tlist = (LVector *)vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_INT);
    for (auto &entry : dir) {
        nlist->Push(vm, Value(vm.NewString(entry.name)));
        slist->Push(vm, Value(entry.size));
        // For a fun change of pace, MSVC C++ standard library has support
        // for clock_cast but libstdc++ and libc++ don't. This is a
        // workaround for converting between the file_clock time (used for
        // the filesystem) and system_clock time (used for formatting
        // times). Sadly, even if we could use clock_cast, it seems to have
        // a known memory leak on Windows:
        // See https://developercommunity.visualstudio.com/t/reported-memory-leak-when-converting-file-time-typ/1467739
        //
        // According to https://stackoverflow.com/a/73748610 (written by Howard Hinnant,
        // the designer of the chrono library):
        //     "I believe the Windows file_clock epoch is 1601-01-01 00:00:00 UTC. The
        //     difference between that and the system_clock epoch (1970-01-01 00:00:00
        //     UTC) is 13,4774 days or 3'234'576h."
        using namespace literals;
        #if defined(_WIN32)
            const chrono::duration file_to_system_clock_epoch_offset = 3'234'576h;
        #elif defined(__GLIBCXX__)  // libstdc++
            // From the same stack overflow article above: "On gcc I believe
            // the epoch is 2174-01-01 00:00:00 UTC".
            // I calculated the following value locally on my linux laptop.
            const chrono::duration file_to_system_clock_epoch_offset = -1'788'240h;
        #else  // libc++ or other
            const chrono::duration file_to_system_clock_epoch_offset = 0h;
        #endif
        auto system_time = chrono::system_clock::time_point{
            chrono::duration_cast<chrono::system_clock::duration>(
                entry.last_write_time.time_since_epoch() -
                file_to_system_clock_epoch_offset)
        };
        tlist->Push(vm, Value((int64_t)chrono::duration_cast<chrono::seconds>(
                                  system_time.time_since_epoch())
                                  .count()));
    }
    Push(sp, Value(nlist));
    Push(sp, Value(slist));
    return tlist;
}

BUILTIN(read_file, "file,textmode", "SI?", "S?",
    "returns the contents of a file as a string, or nil if the file can't be found."
    " you may use either \\ or / as path separators")
(VM &vm, LString *file, iint textmode) {
    string buf;
    auto l = LoadFile(file->strv(), &buf, 0, -1, (textmode == 0));
    if (l < 0) return nullptr;
    auto s = vm.NewString(buf);
    return s;
}

BUILTIN(write_file, "file,contents,textmode,absolute_path", "SSI?I?", "B",
    "creates a file with the contents of a string, returns false if writing wasn't possible")
(VM &, LString *file, LString *contents, iint textmode, iint absolute) {
    auto ok = WriteFile(file->strv(), (textmode == 0), contents->strv(), (absolute != 0));
    return ok;
}

BUILTIN(rename_file, "old_file,new_file", "SS", "B",
    "renames a file, returns false if it wasn't possible")
(VM &, LString *old_file, LString *new_file) {
    auto ok = RenameFile(old_file->strv(), new_file->strv());
    return ok;
}

BUILTIN(delete_file, "file", "S", "B", "deletes a file, returns false if it wasn't possible. Will search in all import dirs.")
(VM &, LString *file) {
    auto ok = FileDelete(file->strv());
    return ok;
}

BUILTIN(exists_file, "file", "S", "B", "checks whether a file exists.")
(VM &, LString *file) {
    auto ok = FileExists(file->strv(), false);
    return ok;
}

BUILTIN_V(launch_subprocess, "commandline,stdin", "S]S?", "IS",
    "launches a sub process, with optionally a stdin for the process, and returns its"
    " return code (or -1 if it couldn't launch at all), and any output")
(StackPtr &sp, VM &vm, LVector *commandline, LString *stdins) {
    vector<const char *> cmdl;
    for (iint i = 0; i < commandline->len; i++) {
        cmdl.push_back(commandline->AtS(i).sval()->data());
    }
    cmdl.push_back(nullptr);
    string out;
    auto ret = LaunchSubProcess(cmdl.data(), (stdins != nullptr) ? stdins->data() : nullptr, out);
    Push(sp, ret);
    Push(sp, vm.NewString(out));
}

BUILTIN(vector_to_buffer, "vec,width,offset,len", "A]*I?:4I?I?", "S",
    "converts a vector of ints/floats (or structs of them) to a buffer, where"
    " each scalar is written with \"width\" bytes (1/2/4/8, default 4). Returns nil if the"
    " type couldn't be converted. Uses native endianness."
    " Offset and len can specify a slice of the input, but if not specified default to all.")
(VM &vm, LVector *v, iint width, iint offset, iint _len) {
    auto w = (int)width;
    if (offset > v->len)
        vm.Error("vector_to_buffer: offset out of range");
    auto len = _len;
    if (!len) len = v->len - offset;
    else if (len > v->len - offset)
        vm.Error("vector_to_buffer: len out of range");
    if (w != 1 && w != 2 && w != 4 && w != 8)
        vm.Error("vector_to_buffer: width out of range");
    auto &vect = v->ti(vm);
    auto eto = vect.subt;
    auto &ti = vm.GetTypeInfo(eto);
    int64_t float_mask = 0;
    if (ti.t == RTT_INT) {
    } else if (ti.t == RTT_FLOAT) {
        float_mask = 1;
    } else if (ti.t == RTT_STRUCT_S) {
        if (ti.len >= 63) vm.Error("vector_to_buffer: struct too big");
        for (int i = 0; i < ti.len; i++) {
            auto &seti = vm.GetTypeInfo(ti.elemtypes[i].type);
            if (seti.t == RTT_INT) {
            } else if (seti.t == RTT_FLOAT) {
                float_mask |= int64_t(1) << i;
            } else {
                vm.Error("vector_to_buffer: struct field of non-numeric data");
            }
        }
    } else {
        vm.Error("vector_to_buffer: vector of non-numeric data");
    }
    if (float_mask && (w == 1 || w == 2))
        vm.Error("vector_to_buffer: 8/16 floats not supported yet");
    auto s = vm.NewString(len * v->width * w);
    auto buf = (uint8_t *)s->data();
    for (iint i = offset; i < offset + len; i++) {
        for (int j = 0; j < (int)v->width; j++) {
            auto is_float = float_mask & (int64_t(1) << j);
            if (is_float) {
                auto x = v->AtSub(i, j).fval();
                if (w == sizeof(double)) {
                    memcpy(buf, &x, sizeof(double));
                } else {
                    auto xf = (float)x;
                    memcpy(buf, &xf, sizeof(float));
                }
            } else {
                auto x = v->AtSub(i, j).ival();
                #if FLATBUFFERS_LITTLEENDIAN
                    memcpy(buf, &x, w);
                #else
                    memcpy(buf, (uint8_t *)&x + (8 - w), w);
                #endif
            }
            buf += w;
        }
    }
    return s;
}

BUILTIN(ensure_size, "string,size,char,extra", "SkIII?", "S",
    "ensures a string is at least size characters. if it is, just returns the existing"
    " string, otherwise returns a new string of that size (with optionally extra bytes"
    " added), with any new characters set to"
    " char. You can specify a negative size to mean relative to the end, i.e. new"
    " characters will be added at the start. ")
(VM &vm, LString *str, iint size, iint c, iint extra) {
    auto asize = std::abs(size);
    return str->len >= asize
        ? str
        : vm.ResizeString(str, asize + extra, (int)c, size < 0);
}

static const char *write_val_desc1 =
    "writes a value as little endian to a string at location i. Uses ensure_size to"
    " make the string twice as long (with extra 0 bytes) if no space. Returns"
    " new string if resized,"
    " and the index of the location right after where the value was written. The"
    " _back version writes relative to the end (and writes before the index)";
static const char *write_val_desc2 = "(see write_int64_le)";
#define WRITEOP(N, T, B, D, S, VT) \
    BUILTIN(N, "string,i,val", "SkI" S, "SI", D) \
    (StackPtr &sp, VM &vm, LString *str, iint idx, VT val) { \
        return WriteVal<T, B>(sp, vm, str, idx, val); \
    }
WRITEOP(write_int64_le, int64_t, false, write_val_desc1, "I", iint)
WRITEOP(write_int32_le, int32_t, false, write_val_desc2, "I", iint)
WRITEOP(write_int16_le, int16_t, false, write_val_desc2, "I", iint)
WRITEOP(write_int8_le, int8_t, false, write_val_desc2, "I", iint)
WRITEOP(write_float64_le, double, false, write_val_desc2, "F", double)
WRITEOP(write_float32_le, float, false, write_val_desc2, "F", double)
WRITEOP(write_int64_le_back, int64_t, true, write_val_desc2, "I", iint)
WRITEOP(write_int32_le_back, int32_t, true, write_val_desc2, "I", iint)
WRITEOP(write_int16_le_back, int16_t, true, write_val_desc2, "I", iint)
WRITEOP(write_int8_le_back, int8_t, true, write_val_desc2, "I", iint)
WRITEOP(write_float64_le_back, double, true, write_val_desc2, "F", double)
WRITEOP(write_float32_le_back, float, true, write_val_desc2, "F", double)

BUILTIN(write_substring, "string,i,substr,nullterm", "SkISI", "SI",
    "writes a substring into another string at i (see also write_int64_le)")
(StackPtr &sp, VM &vm, LString *str, iint idx, LString *val, iint term) {
    return WriteStr<false>(sp, vm, str, idx, val, (term != 0));
}

BUILTIN(write_substring_back, "string,i,substr,nullterm", "SkISI", "SI",
    "")
(StackPtr &sp, VM &vm, LString *str, iint idx, LString *val, iint term) {
    return WriteStr<true>(sp, vm, str, idx, val, (term != 0));
}

BUILTIN(compare_substring, "string_a,i_a,string_b,i_b,len", "SISII", "I",
    "returns if the two substrings are equal (0), or a < b (-1) or a > b (1).")
(VM &vm, LString *s1, iint i1, LString *s2, iint i2, iint l) {
    if (l < 0 || i1 < 0 || i2 < 0 || i1 + l > s1->len || i2 + l > s2->len)
        vm.Error("compare_substring: index out of bounds");
    auto eq = memcmp(s1->data() + i1, s2->data() + i2, l);
    return eq;
}

static const char *read_val_desc1 =
    "reads a value as little endian from a string at location i. The value must be within"
    " bounds of the string. Returns the value, and the index of the location right after where"
    " the value was read. The"
    " _back version reads relative to the end (and reads before the index)";
static const char *read_val_desc2 = "(see read_int64_le)";
#define READOP(N, T, B, D, S) \
    BUILTIN(N, "string,i", "SI", S "I", D) \
    (StackPtr &sp, VM &vm, LString *str, iint idx) { return ReadVal<T, B>(sp, vm, str, idx); }
READOP(read_int64_le, int64_t, false, read_val_desc1, "I")
READOP(read_int32_le, int32_t, false, read_val_desc2, "I")
READOP(read_int16_le, int16_t, false, read_val_desc2, "I")
READOP(read_int8_le, int8_t, false, read_val_desc2, "I")
READOP(read_uint64_le, uint64_t, false, read_val_desc1, "I")
READOP(read_uint32_le, uint32_t, false, read_val_desc2, "I")
READOP(read_uint16_le, uint16_t, false, read_val_desc2, "I")
READOP(read_uint8_le, uint8_t, false, read_val_desc2, "I")
READOP(read_float64_le, double, false, read_val_desc2, "F")
READOP(read_float32_le, float, false, read_val_desc2, "F")
READOP(read_int64_le_back, int64_t, true, read_val_desc2, "I")
READOP(read_int32_le_back, int32_t, true, read_val_desc2, "I")
READOP(read_int16_le_back, int16_t, true, read_val_desc2, "I")
READOP(read_int8_le_back, int8_t, true, read_val_desc2, "I")
READOP(read_uint64_le_back, uint64_t, true, read_val_desc2, "I")
READOP(read_uint32_le_back, uint32_t, true, read_val_desc2, "I")
READOP(read_uint16_le_back, uint16_t, true, read_val_desc2, "I")
READOP(read_uint8_le_back, uint8_t, true, read_val_desc2, "I")
READOP(read_float64_le_back, double, true, read_val_desc2, "F")
READOP(read_float32_le_back, float, true, read_val_desc2, "F")


#undef BUILTIN_GROUP
#undef BUILTIN_SYM
BuiltinGroup flatbuffers_builtins;
#define BUILTIN_GROUP flatbuffers_builtins
#define BUILTIN_SYM(name) builtin_flatbuffers_##name

static const char *read_field_desc1 =
    "reads a flatbuffers field from a string at table location tablei, field vtable offset vo,"
    " and default value def. The value must be within"
    " bounds of the string. Returns the value (or default if the field was not present)";
static const char *read_field_desc2 = "(see flatbuffers.field_int64)";
#define READFOP(N, T, D, S, VT) \
    BUILTIN(N, "string,tablei,vo,def", "SII" S, S, D) \
    (VM &vm, LString *str, iint idx, iint vidx, VT def) { \
        return ReadField<T, S[0] == 'F', false, false>(vm, str, idx, vidx, def).ifval<VT>(); \
    }
READFOP(field_int64, int64_t, read_field_desc1, "I", iint)
READFOP(field_int32, int32_t, read_field_desc2, "I", iint)
READFOP(field_int16, int16_t, read_field_desc2, "I", iint)
READFOP(field_int8, int8_t, read_field_desc2, "I", iint)
READFOP(field_uint64, uint64_t, read_field_desc1, "I", iint)
READFOP(field_uint32, uint32_t, read_field_desc2, "I", iint)
READFOP(field_uint16, uint16_t, read_field_desc2, "I", iint)
READFOP(field_uint8, uint8_t, read_field_desc2, "I", iint)
READFOP(field_float64, double, read_field_desc2, "F", double)
READFOP(field_float32, float, read_field_desc2, "F", double)

BUILTIN(field_string, "string,tablei,vo", "SII", "S",
    "reads a flatbuffer string field, returns \"\" if not present")
(VM &vm, LString *str, iint idx, iint vidx) {
    auto fi = ReadField<flatbuffers::uoffset_t, false, true, false>(vm, str, idx, vidx,
                                                                    Value(0)).ival();
    return GetString(vm, fi, str);
}

BUILTIN(field_vector_len, "string,tablei,vo", "SII", "I",
    "reads a flatbuffer vector field length, or 0 if not present")
(VM &vm, LString *str, iint idx, iint vidx) {
    auto fi = ReadField<flatbuffers::uoffset_t, false, true, false>(vm, str, idx, vidx,
                                                                    Value(0)).ival();
    return fi ? Read<flatbuffers::uoffset_t, false>(vm, fi, str) : 0;
}

BUILTIN(field_vector, "string,tablei,vo", "SII", "I",
    "returns a flatbuffer vector field element start, or 0 if not present")
(VM &vm, LString *str, iint idx, iint vidx) {
    auto fi = ReadField<flatbuffers::uoffset_t, false, true, false>(vm, str, idx, vidx,
                                                                    Value(0)).ival();
    return fi ? fi + ssizeof<flatbuffers::uoffset_t>() : 0;
}

BUILTIN(field_table, "string,tablei,vo", "SII", "I",
    "returns a flatbuffer table field start, or 0 if not present")
(VM &vm, LString *str, iint idx, iint vidx) {
    return ReadField<flatbuffers::uoffset_t, false, true, false>(vm, str, idx, vidx,
                                                                Value(0)).ival();
}

BUILTIN(field_struct, "string,tablei,vo", "SII", "I",
    "returns a flatbuffer struct field start, or 0 if not present")
(VM &vm, LString *str, iint idx, iint vidx) {
    return ReadField<flatbuffers::uoffset_t, false, false, true>(vm, str, idx, vidx,
                                                                Value(0)).ival();
}

BUILTIN(field_present, "string,tablei,vo", "SII", "B",
    "returns if a flatbuffer field is present (unequal to default)")
(VM &vm, LString *str, iint idx, iint vidx) {
    return FieldPresent(vm, str, idx, vidx);
}

BUILTIN(indirect, "string,index", "SI", "I",
    "returns a flatbuffer offset at index relative to itself")
(VM &vm, LString *str, iint idx) {
    auto off = Read<flatbuffers::uoffset_t, false>(vm, idx, str);
    return off + idx;
}

BUILTIN(string, "string,index", "SI", "S",
    "returns a flatbuffer string whose offset is at given index")
(VM &vm, LString *str, iint idx) {
    auto off = Read<flatbuffers::uoffset_t, false>(vm, idx, str);
    auto ret = GetString(vm, off + idx, str);
    return ret;
}

BUILTIN(binary_to_json, "schemas,binary,includedirs", "SSS]", "SS?",
    "returns a JSON string generated from the given binary and corresponding schema."
    "if there was an error parsing the schema, the error will be in the second return"
    "value, or nil for no error")
(StackPtr &sp, VM &vm, LString *schema, LString *binary, LVector *includes) {
    flatbuffers::Parser parser;
    auto err = ParseSchemas(vm, parser, schema, includes);
    string json;
    if (err.False()) {
        auto e = GenText(parser, binary->data(), &json);
        if (e) {
            err = vm.NewString("unable to generate text for FlatBuffer binary: " + string(e));
        }
    }
    Push(sp, vm.NewString(json));
    return err.svalnil();
}

BUILTIN(json_to_binary, "schema,json,includedirs", "SSS]", "SS?",
    "returns a binary flatbuffer generated from the given json and corresponding schema."
    "if there was an error parsing the schema, the error will be in the second return"
    "value, or nil for no error")
(StackPtr &sp, VM &vm, LString *schema, LString *json, LVector *includes) {
    flatbuffers::Parser parser;
    auto err = ParseSchemas(vm, parser, schema, includes);
    string binary;
    if (err.False()) {
        if (!parser.Parse(json->data())) {
            err = vm.NewString(parser.error_);
        } else {
            binary.assign((const char *)parser.builder_.GetBufferPointer(),
                            parser.builder_.GetSize());
        }
    }
    Push(sp, vm.NewString(binary));
    return err.svalnil();
}

}
