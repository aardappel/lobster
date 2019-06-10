// Copyright 2019 Wouter van Oortmerssen. All rights reserved.
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

#ifndef WASM_BINARY_WRITER_H
#define WASM_BINARY_WRITER_H

#include "assert.h"
#include "vector"
#include "string"
#include "string_view"

using namespace std::string_view_literals;

// Stand-alone single header WASM module writer class.
// Takes care of the "heavy lifting" of generating the binary format
// correctly, and provide a friendly code generation API

namespace WASM {

enum class Section {
    None = -1,
    Custom = 0,
    Type,
    Import,
    Function,
    Table,
    Memory,
    Global,
    Export,
    Start,
    Element,
    Code,
    Data,
};

enum {
    I32 = 0x7F,
    I64 = 0x7E,
    F32 = 0x7D,
    F64 = 0x7C,
    ANYFUNC = 0x70,
    FUNC = 0x60,
    VOID = 0x40,
};

class BinaryWriter {
    std::vector<uint8_t> &buf;
    Section cur_section = Section::None;
    Section last_known_section = Section::None;
    size_t section_size = 0;
    size_t section_count = 0;
    size_t section_data = 0;
    size_t section_index_in_file = 0;
    size_t section_index_in_file_code = 0;
    size_t section_index_in_file_data = 0;
    size_t segment_payload_start = 0;
    size_t num_imports = 0;
    size_t num_function_decls = 0;
    struct Function {
        std::string name;
        bool import;
        bool local;
    };
    std::vector<Function> function_symbols;
    size_t function_body_start = 0;
    size_t data_section_size = 0;
    struct DataSegment {
        std::string name;
        size_t align;
        size_t size;
        bool local;
    };
    std::vector<DataSegment> data_segments;
    struct Reloc {
        uint8_t type;
        size_t offset;
        size_t index;
        size_t addend;
        bool is_function;  // What index refers to.
    };
    std::vector<Reloc> code_relocs;
    std::vector<Reloc> data_relocs;

    template<typename T> void UInt8(T v) {
        buf.push_back(static_cast<uint8_t>(v));
    }

    template<typename T> void UInt16(T v) {
        UInt8(v & 0xFF);
        UInt8(v >> 8);
    }

    template<typename T> void UInt32(T v) {
        UInt16(v & 0xFFFF);
        UInt16(v >> 16);
    }

    template<typename T> void UInt64(T v) {
        UInt32(v & 0xFFFFFFFF);
        UInt32(v >> 32);
    }

    template<typename T> void ULEB(T v) {
        for (;;) {
            UInt8(v & 0x7F);
            v = (T)(v >> 7);
            if (!v) break;
            buf.back() |= 0x80;
        }
    }

    template<typename T> void SLEB(T v) {
        auto negative = v < 0;
        for (;;) {
            UInt8(v & 0x7F);
            auto sign = v & 0x40;
            v = (T)(v >> 7);
            if (negative) v |= T(0x7F) << (sizeof(T) * 8 - 7);
            if ((!v && !sign) || (v == -1 && sign)) break;
            buf.back() |= 0x80;
        }
    }

    enum { PATCHABLE_ULEB_SIZE = 5 };

    size_t PatchableLEB() {
        auto pos = buf.size();
        for (size_t i = 0; i < PATCHABLE_ULEB_SIZE - 1; i++) UInt8(0x80);
        UInt8(0x00);
        return pos;
    }

    void PatchULEB(size_t pos, size_t v) {
        for (size_t i = 0; i < PATCHABLE_ULEB_SIZE; i++) {
            buf[pos + i] |= v & 0x7F;
            v >>= 7;
        }
        assert(!v);
    }

    void Chars(std::string_view chars) {
        for (auto c : chars) UInt8(c);
    }

    size_t LenChars(std::string_view chars) {
        ULEB(chars.size());
        auto pos = buf.size();
        Chars(chars);
        return pos;
    }

    bool StartsWithCount() {
        return cur_section != Section::Custom &&
               cur_section != Section::Start;
    }

    enum {
        R_WASM_FUNCTION_INDEX_LEB = 0,
        R_WASM_TABLE_INDEX_SLEB = 1,
        R_WASM_TABLE_INDEX_I32 = 2,
        R_WASM_MEMORY_ADDR_LEB = 3,
        R_WASM_MEMORY_ADDR_SLEB = 4,
        R_WASM_MEMORY_ADDR_I32 = 5,
        R_WASM_TYPE_INDEX_LEB = 6,
        R_WASM_GLOBAL_INDEX_LEB = 7,
        R_WASM_FUNCTION_OFFSET_I32 = 8,
        R_WASM_SECTION_OFFSET_I32 = 9,
        R_WASM_EVENT_INDEX_LEB = 10,
    };


    void RelocULEB(uint8_t reloc_type, size_t sym_index, size_t addend, bool is_function) {
        code_relocs.push_back({ reloc_type,
                                buf.size() - section_data,
                                sym_index,
                                addend,
                                is_function });
        // A relocatable LEB typically can be 0, since all information about
        // this value is stored in the relocation itself. But putting
        // a meaningful value here will help with reading the output of
        // objdump.
        PatchULEB(PatchableLEB(), is_function ? sym_index : addend);
    };

  public:

    explicit BinaryWriter(std::vector<uint8_t> &dest) : buf(dest) {
        Chars(std::string_view("\0asm", 4));
        UInt32(1);
    }

    // Call Begin/EndSection pairs for each segment type, in order.
    // In between, call the Add functions below corresponding to the section
    // type.
    void BeginSection(Section st, std::string_view name = "") {
        // Call EndSection before calling another BeginSection.
        assert(cur_section == Section::None);
        cur_section = st;
        if (st == Section::Code)
            section_index_in_file_code = section_index_in_file;
        if (st == Section::Data)
            section_index_in_file_data = section_index_in_file;
        UInt8(st);
        section_size = PatchableLEB();
        if (st == Section::Custom) {
            LenChars(name);
        } else {
            // Known sections must be created in order and only once.
            assert(st > last_known_section);
            last_known_section = st;
        }
        section_count = 0;
        section_data = buf.size();
        if (StartsWithCount()) PatchableLEB();
    }

    void EndSection(Section st) {
        assert(cur_section == st);
        (void)st;
        // Most sections start with a "count" field.
        if (StartsWithCount()) {
            PatchULEB(section_data, section_count);
        }
        // Patch up the size of this section.
        PatchULEB(section_size, buf.size() - section_size - PATCHABLE_ULEB_SIZE);
        cur_section = Section::None;
        section_index_in_file++;
    }

    size_t AddType(const std::vector<unsigned> &params, const std::vector<unsigned> &returns) {
        assert(cur_section == Section::Type);
        ULEB(FUNC);
        ULEB(params.size());
        for (auto p : params) ULEB(p);
        ULEB(returns.size());
        for (auto r : returns) ULEB(r);
        return section_count++;
    }

    size_t AddImportLinkFunction(std::string_view name, size_t tidx) {
        LenChars("");  // Module, unused.
        LenChars(name);
        UInt8(0);  // Function.
        ULEB(tidx);
        function_symbols.push_back({ std::string(name), true, true });
        section_count++;
        return num_imports++;
    }

    size_t GetNumImports() { return num_imports; }

    void AddFunction(size_t tidx) {
        assert(cur_section == Section::Function);
        ULEB(tidx);
        num_function_decls++;
        section_count++;
    }

    size_t GetNumDefinedFunctions() { return num_function_decls; }

    void AddMemory(size_t initial_pages) {
        assert(cur_section == Section::Memory);
        ULEB(0);  // Flags: no maximum.
        ULEB(initial_pages);
        section_count++;
    }

    void AddStart(size_t fidx) {
        assert(cur_section == Section::Start);
        ULEB(fidx);
    }

    // Use the Emit Functions below to add to the function body, and be sure
    // to end with EmitEndFunction.
    void AddCode(const std::vector<unsigned> &locals, std::string_view name,
                 bool local) {
        assert(cur_section == Section::Code);
        assert(!function_body_start);
        function_body_start = PatchableLEB();
        std::vector<std::pair<unsigned, unsigned>> entries;
        for (auto l : locals) {
            if (entries.empty() || entries.back().second != l) {
                entries.emplace_back(std::pair { 1, l });
            } else {
                entries.back().first++;
            }
        }
        ULEB(entries.size());
        for (auto &e : entries) {
            ULEB(e.first);
            ULEB(e.second);
        }
        function_symbols.push_back({ std::string(name), false, local });
        section_count++;
    }

    void EmitI32Const(int32_t v) { UInt8(0x41); SLEB(v); }
    void EmitI64Const(int64_t v) { UInt8(0x42); SLEB(v); }

    void EmitI32ConstDataRef(size_t segment, size_t addend) {
        UInt8(0x41);
        RelocULEB(R_WASM_MEMORY_ADDR_SLEB, segment, addend, false );
    }

    // fun_idx is 0..N-1 imports followed by N..M-1 defined functions.
    void EmitI32FunctionRef(size_t fun_idx) {
        UInt8(0x41);
        RelocULEB(R_WASM_TABLE_INDEX_SLEB, fun_idx, 0, true);
    }

    // fun_idx is 0..N-1 imports followed by N..M-1 defined functions.
    void EmitCall(size_t fun_idx) {
        UInt8(0x10);
        RelocULEB(R_WASM_FUNCTION_INDEX_LEB, fun_idx, 0, true);
    }

    void EmitReturn() { UInt8(0x0F); }

    void EmitEnd() { UInt8(0x0B); }

    void EmitGetLocal(size_t local) { UInt8(0x20); ULEB(local); }

    void EmitIf(uint8_t block_type) { UInt8(0x04); UInt8(block_type); }

    void EmitEndFunction() {
        assert(cur_section == Section::Code);
        EmitEnd();
        assert(function_body_start);
        PatchULEB(function_body_start,
            buf.size() - function_body_start - PATCHABLE_ULEB_SIZE);
        function_body_start = 0;
    }

    void AddData(std::string_view data, std::string_view symbol, size_t align,
                 bool local = true) {
        assert(cur_section == Section::Data);
        ULEB(0);  // Linear memory index.
        // Init exp: must use 32-bit for wasm32 target.
        EmitI32Const(static_cast<int32_t>(data_section_size));
        EmitEnd();
        segment_payload_start = LenChars(data);
        data_section_size += data.size();
        data_segments.push_back({ std::string(symbol), align, data.size(), local });
        section_count++;
    }

    // "off" is relative to the data in the last AddData call.
    void DataFunctionRef(size_t fid, size_t off) {
        assert(segment_payload_start);
        data_relocs.push_back({ R_WASM_TABLE_INDEX_I32,
                                off + (segment_payload_start - section_data),
                                fid,
                                0,
                                true });
    }

    // Call this last, to finalize the buffer into a valid WASM module,
    // and to add linking/reloc sections based on the previous sections.
    void Finish() {
        assert(cur_section == Section::None);
        // If this assert fails, you likely have not matched the number of
        // AddFunction calls in a Function section with the number of AddCode
        // calls in a Code section.
        assert(num_imports + num_function_decls == function_symbols.size());
        // Linking section.
        {
            BeginSection(Section::Custom, "linking");
            ULEB(2);  // Version.
            enum {
                WASM_SEGMENT_INFO = 5,
                WASM_INIT_FUNCS = 6,
                WASM_COMDAT_INFO = 7,
                WASM_SYMBOL_TABLE = 8,
            };
            // Segment Info.
            {
                UInt8(WASM_SEGMENT_INFO);
                auto sisize = PatchableLEB();
                ULEB(data_segments.size());
                for (auto &ds : data_segments) {
                    LenChars(ds.name);
                    ULEB(ds.align);
                    ULEB(0);  // Flags. FIXME: any valid values?
                }
                PatchULEB(sisize, buf.size() - sisize - PATCHABLE_ULEB_SIZE);
            }
            // Symbol Table.
            {
                UInt8(WASM_SYMBOL_TABLE);
                auto stsize = PatchableLEB();
                enum {
                    SYMTAB_FUNCTION = 0,
                    SYMTAB_DATA = 1,
                    SYMTAB_GLOBAL = 2,
                    SYMTAB_SECTION = 3,
                    SYMTAB_EVENT = 4,
                };
                enum {
                    WASM_SYM_BINDING_WEAK = 1,
                    WASM_SYM_BINDING_LOCAL = 2,
                    WASM_SYM_VISIBILITY_HIDDEN = 4,
                    WASM_SYM_UNDEFINED = 16,
                    WASM_SYM_EXPORTED = 32,
                };
                ULEB(data_segments.size() + function_symbols.size());
                size_t segi = 0;
                for (auto &ds : data_segments) {
                    UInt8(SYMTAB_DATA);
                    ULEB(ds.local ? WASM_SYM_BINDING_LOCAL : WASM_SYM_EXPORTED);
                    LenChars(ds.name);
                    ULEB(segi++);
                    ULEB(0);  // Offset in segment, always 0 (1 seg per sym).
                    ULEB(ds.size);
                }
                size_t wasm_function = 0;
                for (auto &fs : function_symbols) {
                    UInt8(SYMTAB_FUNCTION);
                    ULEB(fs.import
                         ? WASM_SYM_UNDEFINED
                         : (fs.local ? WASM_SYM_BINDING_LOCAL
                                     : WASM_SYM_EXPORTED));
                    ULEB(wasm_function++);
                    if (!fs.import) {
                        LenChars(fs.name);
                    }
                }
                PatchULEB(stsize, buf.size() - stsize - PATCHABLE_ULEB_SIZE);
            }
            EndSection(Section::Custom);  // linking
        }
        // Reloc sections
        {
            auto EncodeReloc = [&](Reloc &r) {
                UInt8(r.type);
                ULEB(r.offset);
                ULEB(r.index + (r.is_function ? data_segments.size() : 0));  // Sym index.
                if (r.type == R_WASM_MEMORY_ADDR_LEB ||
                    r.type == R_WASM_MEMORY_ADDR_SLEB ||
                    r.type == R_WASM_MEMORY_ADDR_I32 ||
                    r.type == R_WASM_FUNCTION_OFFSET_I32 ||
                    r.type == R_WASM_SECTION_OFFSET_I32)
                    SLEB((ptrdiff_t)r.addend);
            };

            BeginSection(Section::Custom, "reloc.CODE");
            ULEB(section_index_in_file_code);
            ULEB(code_relocs.size());
            for (auto &r : code_relocs) EncodeReloc(r);
            EndSection(Section::Custom);  // reloc.CODE

            BeginSection(Section::Custom, "reloc.DATA");
            ULEB(section_index_in_file_data);
            ULEB(data_relocs.size());
            for (auto &r : data_relocs) EncodeReloc(r);
            EndSection(Section::Custom);  // reloc.DATA
        }
    }

};

// This is a very simple test of the instruction encoding. The function returns a binary that
// when written to a file should pass e.g. wasm-validate in WABT.
std::vector<uint8_t> SimpleBinaryWriterTest() {
    std::vector<uint8_t> vec;
    BinaryWriter bw(vec);
    // Write a (function) type section, to be referred to by functions below.
    // For any of these sections, if you write them out of order, or don't match
    // begin/end, you'll get an assert.
    // As with everything, to refer to things in wasm, use a 0 based index.
    bw.BeginSection(WASM::Section::Type);
    // A list of arguments followed by a list of return values.
    // You don't have to use the return value, but it may make referring to this
    // type easier.
    auto type_ii_i = bw.AddType({ WASM::I32, WASM::I32 }, { WASM::I32 });  // 0
    auto type_i_v = bw.AddType({ WASM::I32 }, {});  // 1
    bw.EndSection(WASM::Section::Type);

    // Import some functions, from the runtime compiled in other modules.
    // For our example that will just be the printing function.
    // Note: we assume this function has been declared with: extern "C"
    // You can link against C++ functions as well if you don't mind dealing
    // with name mangling.
    bw.BeginSection(WASM::Section::Import);
    auto import_print = bw.AddImportLinkFunction("print", type_i_v);  // 0
    bw.EndSection(WASM::Section::Import);

    // Declare all the functions we will generate. Note this is just the type,
    // the body of the code will follow below.
    bw.BeginSection(WASM::Section::Function);
    bw.AddFunction(type_ii_i);  // main()
    bw.EndSection(WASM::Section::Function);

    // Declare the linear memory we want to use, with 1 initial page.
    bw.BeginSection(WASM::Section::Memory);
    bw.AddMemory(1);
    bw.EndSection(WASM::Section::Memory);

    // Here we'd normally declare a "Start" section, but the linker will
    // take care for that for us.

    // Now the exciting part: emitting function bodies.
    bw.BeginSection(WASM::Section::Code);

    // A list of 0 local types,
    bw.AddCode({}, "main", false);
    // Refers to data segment 0 at offset 0 below. This emits an i32.const
    // instruction, whose immediate value will get relocated to refer to the
    // data correctly.
    bw.EmitI32ConstDataRef(0, 0);
    bw.EmitCall(import_print);
    bw.EmitI32Const(0);  // Return value.
    bw.EmitEndFunction();

    // Here, call AddCode..EmitEndFunction for more functions.

    bw.EndSection(WASM::Section::Code);

    // Add all our static data.
    bw.BeginSection(WASM::Section::Data);

    // This is our first segment, we referred to this above as 0.
    auto hello = "Hello, World\n\0"sv;
    // Data, name, and alignment.
    bw.AddData(hello, "hello", 0);

    // Create another segment, this time with function references.
    int function_ref = (int)bw.GetNumImports() + 0;  // Refers to main()
    bw.AddData(std::string_view((char *)&function_ref, sizeof(int)), "funids", sizeof(int));
    bw.DataFunctionRef(function_ref, 0);  // Reloc it.

    bw.EndSection(WASM::Section::Data);

    // This call does all the remaining work of generating the linking
    // information, and wrapping up the file.
    bw.Finish();
    return vec;
}

}  // namespace WASM

#endif  // WASM_BINARY_WRITER_H
