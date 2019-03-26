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
    size_t function_body_start = 0;

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
            if (!(v = (T)(v >> 7))) break;
            buf.back() |= 0x80;
        }
    }

    template<typename T> void SLEB(T v) {
        auto negative = v < 0;
        for (;;) {
            UInt8(v & 0x7F);
            auto sign = v & 0x40;
            v >>= 7;
            if (negative) v |= (~0 << (sizeof(T) * 8 - 7));
            if ((!v && !sign) || (v == -1 && sign)) break;
            buf.back() |= 0x80;
        }
    }

    enum { PATCHABLE_ULEB_SIZE = 5 };

    size_t PatchableULEB() {
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
        UInt8(st);
        section_size = PatchableULEB();
        if (st == Section::Custom) {
            LenChars(name);
        } else {
            // Known sections must be created in order and only once.
            assert(st > last_known_section);
            last_known_section = st;
        }
        section_count = 0;
        section_data = StartsWithCount() ? PatchableULEB() : 0;
    }

    void EndSection(Section st) {
        assert(cur_section == st);
        // Most sections start with a "count" field.
        if (StartsWithCount()) {
            PatchULEB(section_data, section_count);
        }
        // Patch up the size of this section.
        PatchULEB(section_size, buf.size() - section_size - PATCHABLE_ULEB_SIZE);
        cur_section = Section::None;
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

    void AddImportFunction(std::string_view module, std::string_view name, size_t tidx) {
        LenChars(module);
        LenChars(name);
        UInt8(0);
        ULEB(tidx);
        section_count++;
    }

    void AddFunction(size_t tidx) {
        assert(cur_section == Section::Function);
        ULEB(tidx);
        section_count++;
    }

    void AddStart(size_t fidx) {
        assert(cur_section == Section::Start);
        ULEB(fidx);
    }

    // Use the Emit Functions below to add to the function body, and be sure
    // to end with EmitEndFunction.
    void AddCode(const std::vector<unsigned> &locals) {
        assert(cur_section == Section::Code);
        assert(!function_body_start);
        function_body_start = PatchableULEB();
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
        section_count++;
    }

    void EmitEndFunction() {
        assert(cur_section == Section::Code);
        UInt8(0x0B);
        assert(function_body_start);
        PatchULEB(function_body_start,
            buf.size() - function_body_start - PATCHABLE_ULEB_SIZE);
        function_body_start = 0;
    }

    // Call this last, to finalize the buffer into a valid WASM module.
    void Finish() {
        assert(cur_section == Section::None);
    }

};

}  // namespace WASM

#endif  // WASM_BINARY_WRITER_H
