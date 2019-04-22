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

// Include this first to ensure it is free of dependencies.
#include "lobster/wasm_binary_writer.h"

#include "lobster/stdafx.h"

#include "lobster/disasm.h"  // Some shared bytecode utilities.
#include "lobster/compiler.h"
#include "lobster/tonative.h"

namespace lobster {

class WASMGenerator : public NativeGenerator {
    WASM::BinaryWriter bw;

    size_t import_erccm  = 0, import_snct = 0, import_gnct = 0, import_tmmt = 0;

  public:

    explicit WASMGenerator(vector<uint8_t> &dest) : bw(dest) {}

    enum {
        TI_I_,
        TI_I_I,
        TI_I_II,
        TI_I_III,
        TI_I_IIII,
        TI_I_IIIII,
        TI_V_,
        TI_V_I,
        TI_V_II,
        TI_V_III,
        TI_V_IIII,
    };

    void FileStart() override {
        bw.BeginSection(WASM::Section::Type);
        // NOTE: this must match the enum above.
        bw.AddType({}, { WASM::I32 });
        bw.AddType({ WASM::I32 }, { WASM::I32 });
        bw.AddType({ WASM::I32, WASM::I32 }, { WASM::I32 });
        bw.AddType({ WASM::I32, WASM::I32, WASM::I32 }, { WASM::I32 });
        bw.AddType({ WASM::I32, WASM::I32, WASM::I32, WASM::I32 }, { WASM::I32 });
        bw.AddType({ WASM::I32, WASM::I32, WASM::I32, WASM::I32, WASM::I32 }, { WASM::I32 });
        bw.AddType({}, {});
        bw.AddType({ WASM::I32 }, {});
        bw.AddType({ WASM::I32, WASM::I32 }, {});
        bw.AddType({ WASM::I32, WASM::I32, WASM::I32 }, {});
        bw.AddType({ WASM::I32, WASM::I32, WASM::I32, WASM::I32 }, {});
        bw.EndSection(WASM::Section::Type);

        bw.BeginSection(WASM::Section::Import);
        #define F(N, A) bw.AddImportLinkFunction("CVM_" #N, TI_V_II);
            LVALOPNAMES
        #undef F
        #define F(N, A) bw.AddImportLinkFunction("CVM_" #N, TI_V_II);
            ILBASENAMES
        #undef F
        #define F(N, A) bw.AddImportLinkFunction("CVM_" #N, TI_V_III);
            ILCALLNAMES
        #undef F
        #define F(N, A) bw.AddImportLinkFunction("CVM_" #N, TI_I_I);
            ILJUMPNAMES
        #undef F
        import_erccm = bw.AddImportLinkFunction("EngineRunCompiledCodeMain", TI_I_IIIII);
        import_snct = bw.AddImportLinkFunction("CVM_SetNextCallTarget", TI_V_II);
        import_gnct = bw.AddImportLinkFunction("CVM_GetNextCallTarget", TI_I_I);
        import_tmmt = bw.AddImportLinkFunction("CVM_TempMMTable", TI_V_III);
        bw.EndSection(WASM::Section::Import);

        bw.BeginSection(WASM::Section::Function);
        bw.AddFunction(TI_I_II);  // main(), defined function 0.
        // All blocks follow here, which have id's 1..N-1.
    }

    void DeclareBlock(int /*id*/) override {
        bw.AddFunction(TI_I_I);
    }

    void BeforeBlocks(int start_id, string_view bytecode_buffer) override {
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
        bw.EmitGetLocal(0 /*argc*/);
        bw.EmitGetLocal(1 /*argv*/);
        bw.EmitI32FunctionRef(bw.GetNumImports() + start_id);
        bw.EmitI32ConstDataRef(0, 0);  // Bytecode, for data refs.
        bw.EmitI32Const((int)bytecode_buffer.size());
        bw.EmitCall(import_erccm);
        bw.EmitEndFunction();
    }

    void FunStart(const bytecode::Function * /*f*/) override {
    }

    void BlockStart(int id) override {
        bw.AddCode({}, "block" + std::to_string(id), true);
    }

    void InstStart() override {
    }

    void EmitJump(int id) override {
        if (id <= current_block_id) {
            // A backwards jump, go via the trampoline.
            bw.EmitI32FunctionRef(bw.GetNumImports() + id);
        } else {
            // A forwards call, should be safe to tail-call.
            bw.EmitGetLocal(0 /*VM*/);
            bw.EmitCall(bw.GetNumImports() + id);
        }
        bw.EmitReturn();
    }

    void EmitConditionalJump(int opc, int id) override {
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitCall((size_t)opc);
        bw.EmitIf(WASM::VOID);
        EmitJump(id);
        bw.EmitEnd();
    }

    void EmitOperands(const char *base, const int *args, int arity) override {
        bw.EmitGetLocal(0 /*VM*/);
        if (arity) bw.EmitI32ConstDataRef(0, (const char *)args - base);
        else bw.EmitI32Const(0);  // nullptr
    }

    void EmitMultiMethodDispatch(const vector<int> &mmtable) override {
        // FIXME: this is terrible, but a quick way to make it work.
        for (auto [i, id] : enumerate(mmtable)) {
            bw.EmitGetLocal(0 /*VM*/);
            bw.EmitI32FunctionRef(bw.GetNumImports() + id);
            bw.EmitI32Const((int)i);
            bw.EmitCall(import_tmmt);
        }
    }

    void SetNextCallTarget(int id) override {
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitI32FunctionRef(bw.GetNumImports() + id);
        bw.EmitCall(import_snct);
    }

    void EmitGenericInst(int opc, int /*arity*/, int target) override {
        if (target >= 0) { bw.EmitI32FunctionRef(bw.GetNumImports() + target); }
        bw.EmitCall((size_t)opc);  // Opcodes are the 0..N of imports.
    }

    void EmitCall(int id) override {
        EmitJump(id);
    }

    void EmitCallIndirect() override {
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitCall(import_gnct);
        bw.EmitReturn();
    }

    void EmitCallIndirectNull() override {
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitCall(import_gnct);
        bw.EmitIf(WASM::VOID);
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitCall(import_gnct);
        bw.EmitReturn();
        bw.EmitEnd();
    }

    void InstEnd() override {
    }

    void BlockEnd(int id, bool already_returned, bool is_exit) override {
        if (!already_returned) {
            if (is_exit) {
                bw.EmitGetLocal(0 /*VM*/);
                bw.EmitCall(import_gnct);
                bw.EmitReturn();
            } else {
                EmitJump(id);
            }
        }
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
