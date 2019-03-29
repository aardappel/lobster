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

#include "lobster/wasm_binary_writer.h"

#include "lobster/stdafx.h"

#include "lobster/disasm.h"  // Some shared bytecode utilities.
#include "lobster/compiler.h"
#include "lobster/tonative.h"

namespace lobster {

class WASMGenerator : public NativeGenerator {
    WASM::BinaryWriter bw;

  public:

    explicit WASMGenerator(vector<uint8_t> &dest) : bw(dest) {}

    enum {
        TI_I_I,
        TI_I_II,
        TI_I_III,
        TI_I_IIII,
    };

    void FileStart() override {
        bw.BeginSection(WASM::Section::Type);
        // NOTE: this must match the enum above.
        bw.AddType({ WASM::I32 }, { WASM::I32 });
        bw.AddType({ WASM::I32 }, { WASM::I32, WASM::I32 });
        bw.AddType({ WASM::I32 }, { WASM::I32, WASM::I32, WASM::I32 });
        bw.AddType({ WASM::I32 }, { WASM::I32, WASM::I32, WASM::I32, WASM::I32 });
        bw.EndSection(WASM::Section::Type);

        bw.BeginSection(WASM::Section::Import);
        #define F(N, A) bw.AddImportLinkFunction("VM_" #N, TI_I_II);
            LVALOPNAMES
        #undef F
        #define F(N, A) bw.AddImportLinkFunction("VM_" #N, TI_I_II);
            ILBASENAMES
        #undef F
        #define F(N, A) bw.AddImportLinkFunction("VM_" #N, TI_I_III);
            ILCALLNAMES
        #undef F
        #define F(N, A) bw.AddImportLinkFunction("VM_" #N, TI_I_I);
            ILJUMPNAMES
        #undef F
        bw.AddImportLinkFunction("EngineRunCompiledCodeMain", TI_I_IIII);
        bw.EndSection(WASM::Section::Import);

        bw.BeginSection(WASM::Section::Function);
        bw.AddFunction(TI_I_II);  // main().
    }

    void DeclareBlock(int /*id*/) override {
        bw.AddFunction(TI_I_I);
    }

    void BeforeBlocks(int start_id) override {
        bw.EndSection(WASM::Section::Function);

        bw.BeginSection(WASM::Section::Memory);
        bw.AddMemory(1);
        bw.EndSection(WASM::Section::Memory);

        // Don't emit a Start section, since this will be determined by the
        // linker (and where-ever the main() symbol ends up).
        /*
        bw.BeginSection(WASM::Section::Start);
        bw.AddStart(0);
        bw.EndSection(WASM::Section::Start);
        */

        bw.BeginSection(WASM::Section::Code);

        // Emit main().
        bw.AddCode({}, "main", false);
        bw.EmitEndFunction();
    }

    void FunStart(const bytecode::Function * /*f*/) override {
    }

    void BlockStart(int id) override {
        bw.AddCode({}, "block" + std::to_string(id), true);
    }

    void InstStart() override {
    }

    void EmitJump(int /*id*/) override {
    }

    void EmitConditionalJump(const char * /*ilname*/, int /*id*/) override {
    }

    void EmitOperands(const char *base, const int *args, int arity) override {
        if (arity) bw.EmitI32ConstDataRef(0, (const char *)args - base);
        else bw.EmitI32Const(0);  // nullptr
    }

    void EmitMultiMethodDispatch(const vector<int> & /*mmtable*/) override {
    }

    void SetNextCallTarget(int /*id*/) override {
    }

    void EmitGenericInst(const char * /*ilname*/, int /*arity*/, int target, int opcode) override {
        if (target >= 0) {}
        bw.EmitCall((size_t)opcode);
    }

    void EmitCall(int /*id*/) override {
    }

    void EmitCallIndirect() override {
    }

    void EmitCallIndirectNull() override {
    }

    void InstEnd() override {
    }

    void BlockEnd(int /*id*/, bool /*already_returned*/, bool /*is_exit*/) override {
        bw.EmitEndFunction();
    }

    void FileEnd(int /*start_id*/, string_view bytecode_buffer) override {
        bw.EndSection(WASM::Section::Code);

        bw.BeginSection(WASM::Section::Data);
        // TODO: don't really want to store all of this.
        bw.AddData(bytecode_buffer, "static_data", 16);
        bw.EndSection(WASM::Section::Data);

        bw.Finish();
    }

    void Annotate(string_view /*comment*/) override {
    }
};

string ToWASM(NativeRegistry &natreg, vector<uint8_t> &dest, string_view bytecode_buffer) {
    if (VM_DISPATCH_METHOD != VM_DISPATCH_TRAMPOLINE)
        return "WASM codegen: can only use trampoline mode";
    WASMGenerator wasmgen(dest);
    return ToNative(natreg, wasmgen, bytecode_buffer);
}

}
