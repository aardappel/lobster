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

#include "vector"
#include "string_view"

// Stand-alone single header WASM module writer class.
// Takes care of the "heavy lifting" of generating the binary format
// correctly, and provide a friendly code generation API

namespace WASM {

class BinaryWriter {
    std::vector<uint8_t> &buf;

    void Chars(std::string_view chars) {
        for (auto c : chars) buf.push_back(static_cast<uint8_t>(c));
    }
  public:

    explicit BinaryWriter(std::vector<uint8_t> &dest) : buf(dest) {
        Chars("\0asm");
    }

};

}  // namespace WASM

#endif  // WASM_BINARY_WRITER_H
