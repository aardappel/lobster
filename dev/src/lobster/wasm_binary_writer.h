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
    std::vector<Reloc> relocs;

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
            if (negative) v |= ~T(0) << (sizeof(T) * 8 - 7);
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

    void LenChars(std::string_view chars) {
        ULEB(chars.size());
        Chars(chars);
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

    void AddType(const std::vector<unsigned> &params, const std::vector<unsigned> &returns) {
        assert(cur_section == Section::Type);
        ULEB(FUNC);
        ULEB(params.size());
        for (auto p : params) ULEB(p);
        ULEB(returns.size());
        for (auto r : returns) ULEB(r);
        section_count++;
    }

    void AddImportLinkFunction(std::string_view name, size_t tidx) {
        LenChars("");  // Module, unused.
        LenChars(name);
        UInt8(0);  // Function.
        ULEB(tidx);
        function_symbols.push_back({ std::string(name), true, true });
        num_imports++;
        section_count++;
    }

    void AddFunction(size_t tidx) {
        assert(cur_section == Section::Function);
        ULEB(tidx);
        num_function_decls++;
        section_count++;
    }

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
        relocs.push_back({ R_WASM_MEMORY_ADDR_SLEB,
                           buf.size() - section_data,
                           segment,
                           addend,
                           false });
        PatchableLEB();
    }

    void EmitCall(size_t fun_idx) {
        UInt8(0x10);
        relocs.push_back({ R_WASM_FUNCTION_INDEX_LEB,
                           buf.size() - section_data,
                           fun_idx,
                           0,
                           true });
        ULEB(fun_idx);
    }

    void EmitEnd() { UInt8(0x0B); }

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
        LenChars(data);
        data_section_size += data.size();
        data_segments.push_back({ std::string(symbol), align, data.size(), local });
        section_count++;
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
        // Reloc section
        {
            BeginSection(Section::Custom, "reloc.CODE");
            ULEB(section_index_in_file_code);
            ULEB(relocs.size());
            for (auto &r : relocs) {
                UInt8(r.type);
                ULEB(r.offset);
                ULEB(r.index + (r.is_function ? data_segments.size() : 0));
                if (r.type == R_WASM_MEMORY_ADDR_LEB ||
                    r.type == R_WASM_MEMORY_ADDR_SLEB ||
                    r.type == R_WASM_MEMORY_ADDR_I32 ||
                    r.type == R_WASM_FUNCTION_OFFSET_I32 ||
                    r.type == R_WASM_SECTION_OFFSET_I32)
                    SLEB(r.addend);
            }
            EndSection(Section::Custom);  // reloc.CODE
        }
    }

};

}  // namespace WASM

#endif  // WASM_BINARY_WRITER_H
