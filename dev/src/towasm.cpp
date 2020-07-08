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
#include "lobster/wasm_binary_writer_test.h"

#include "lobster/stdafx.h"

#include "lobster/disasm.h"  // Some shared bytecode utilities.
#include "lobster/compiler.h"
#include "lobster/tonative.h"

namespace lobster {

class WASMGenerator : public NativeGenerator {
    WASM::BinaryWriter bw;
    map<int, int> function_ids;  // FIXME: remove the need for this.
    int num_emitted_functions = 0;
    struct Block {
        bool forward;
        int id;
        NativeHint h;
    };
    vector<Block> blocks;
    int cur_block = -1;
    NativeHint last_conditional_hint = NH_NONE;

    size_t import_erccm  = 0, import_snct = 0, import_gnct = 0, import_drop = 0,
           import_gl_frame = 0;

  public:

    explicit WASMGenerator(vector<uint8_t> &dest) : bw(dest) {}

    enum {
        TI_I_,
        TI_I_I,
        TI_I_II,
        TI_I_III,
        TI_I_IIII,
        TI_I_IIIII,
        TI_I_IIIIII,
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
        bw.AddType({ WASM::I32, WASM::I32, WASM::I32, WASM::I32, WASM::I32, WASM::I32 }, { WASM::I32 });
        bw.AddType({}, {});
        bw.AddType({ WASM::I32 }, {});
        bw.AddType({ WASM::I32, WASM::I32 }, {});
        bw.AddType({ WASM::I32, WASM::I32, WASM::I32 }, {});
        bw.AddType({ WASM::I32, WASM::I32, WASM::I32, WASM::I32 }, {});
        bw.EndSection(WASM::Section::Type);

        bw.BeginSection(WASM::Section::Import);
        #define S_ARGS0 TI_I_II
        #define S_ARGS1 TI_I_III
        #define S_ARGS2 TI_I_IIII
        #define S_ARGS3 TI_I_IIIII
        #define S_ARGS9 TI_I_III  // ILUNKNOWNARITY
        #define S_ARGSN(N) S_ARGS##N
        #define C_ARGS0 TI_I_III
        #define C_ARGS1 TI_I_IIII
        #define C_ARGS2 TI_I_IIIII
        #define C_ARGS3 TI_I_IIIIII
        #define C_ARGS9 TI_I_IIII  // ILUNKNOWNARITY
        #define C_ARGSN(N) C_ARGS##N
        #define F(N, A) bw.AddImportLinkFunction("CVM_" #N, S_ARGSN(A));
            LVALOPNAMES
        #undef F
        #define F(N, A) bw.AddImportLinkFunction("CVM_" #N, S_ARGSN(A));
            ILBASENAMES
        #undef F
        #define F(N, A) bw.AddImportLinkFunction("CVM_" #N, C_ARGSN(A));
            ILCALLNAMES
        #undef F
        #define F(N, A) bw.AddImportLinkFunction("CVM_" #N, TI_I_II);
            ILJUMPNAMES1
        #undef F
        #define F(N, A) bw.AddImportLinkFunction("CVM_" #N, TI_I_III);
            ILJUMPNAMES2
        #undef F
        import_erccm = bw.AddImportLinkFunction("RunCompiledCodeMain", TI_I_IIIII);
        import_snct = bw.AddImportLinkFunction("CVM_SetNextCallTarget", TI_V_II);
        import_gnct = bw.AddImportLinkFunction("CVM_GetNextCallTarget", TI_I_I);
        import_drop = bw.AddImportLinkFunction("CVM_Drop", TI_I_I);
        import_gl_frame = bw.AddImportLinkFunction("GLFrame", TI_I_II);
        bw.EndSection(WASM::Section::Import);

        bw.BeginSection(WASM::Section::Function);
        bw.AddFunction(TI_I_II);  // main(), defined function 0.
        bw.AddFunction(TI_I_II);  // compiled_entry_point(), defined function 1.
        // All functions follow here, which have id's 2..
        num_emitted_functions = 2;
    }

    void DeclareFun(int id) override {
        bw.AddFunction(TI_I_II);
        function_ids[id] = (int)bw.GetNumFunctionImports() + num_emitted_functions++;
    }

    void DeclareBlock(int /*id*/) override {
    }

    void BeforeBlocks(int start_id, string_view bytecode_buffer) override {
        bw.EndSection(WASM::Section::Function);

        // We need this (and Element below) to be able to use call_indirect.
        bw.BeginSection(WASM::Section::Table);
        bw.AddTable();
        bw.EndSection(WASM::Section::Table);

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

        // This initializes the Table declared above. Needed for call_indirect.
        // For now we use a utility function that maps all functions ids 1:1 to the table.
        bw.BeginSection(WASM::Section::Element);
        bw.AddElementAllFunctions();
        bw.EndSection(WASM::Section::Element);

        bw.BeginSection(WASM::Section::Code);

        // Emit main().
        bw.AddCode({}, "main", false);
        bw.EmitGetLocal(0 /*argc*/);
        bw.EmitGetLocal(1 /*argv*/);
        bw.EmitI32ConstDataRef(1, 0);  // Bytecode, for data refs.
        bw.EmitI32Const((int)bytecode_buffer.size());
        bw.EmitI32ConstDataRef(0, 0);  // vtables.
        bw.EmitCall(import_erccm);
        bw.EmitEndFunction();

        // Emit compiled_entry_point
        bw.AddCode({}, "compiled_entry_point", false);
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitGetLocal(1 /*sp*/);
        bw.EmitCall(function_ids[start_id]);
        bw.EmitEndFunction();
    }

    void FunStart(const bytecode::Function *f, int id) override {
        bw.AddCode({}, "fun_" + std::to_string(id) +
            (f ? "_" + f->name()->string_view() : ""), true);
    }

    void BlockStart(int id, int /*ip*/) override {
        cur_block = id;
    }

    void InstStart() override {
    }

    void EmitJump(int id) override {
        int nesting = 0;
        for (auto &b : reverse(blocks)) {
            if (id <= cur_block) {  // Backwards.
                if (!b.forward) {
                    assert(b.h == NH_LOOP_BACK);
                    bw.EmitBr(nesting);
                    return;
                }
            } else {  // Forwards.
                // This is a break, or the jump skipping an "else" or switch-cases.
                if (b.h == NH_JUMPOUT_START) {
                    // skipping an "else" or switch-cases.
                    bw.EmitBr(nesting);
                    return;
                } else if (!b.forward) {
                    // A break.
                    assert(b.h == NH_LOOP_BACK);
                    bw.EmitBr(nesting + 1);  // The block around the loop.
                    return;
                }
            }
            nesting++;
        }
        assert(false);  // Jump target not found!
    }

    void EmitConditionalJump(int opc, int id, int df) override {
        assert(id > cur_block);  // Only forward currently.
        // FIXME: this is very clumsy, shorten this for common cases!
        size_t blockdepth = 0;
        if (last_conditional_hint != NH_NONE) {
            for (auto &b : reverse(blocks)) {
                if (b.h == last_conditional_hint - 1) goto found;
                blockdepth++;
            }
            assert(false);  // block not found!
            found:
            last_conditional_hint = NH_NONE;
        } else {
            bw.EmitBlock(WASM::VOID);
            blocks.push_back({ true, id, NH_COND_JUMP });
        }
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitGetLocal(1 /*SP*/);
        if (df >= 0) bw.EmitI32Const(df);
        bw.EmitCall((size_t)opc);
        bw.EmitTeeLocal(1 /*SP*/);
        bw.EmitI64Load(0);  // False if we should jump.
        bw.EmitGetLocal(1 /*SP*/);
        bw.EmitCall(import_drop);  // FIXME: don't know sizeof(Value) here (!)
        bw.EmitSetLocal(1 /*SP*/);
        bw.EmitI64Const(0);
        bw.EmitI64Eq();
        bw.EmitBrIf(blockdepth);
    }

    void EmitOperands(const char *base, const int *args, int arity, bool is_vararg) override {
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitGetLocal(1 /*SP*/);
        if (is_vararg) {
            if (arity) bw.EmitI32ConstDataRef(1, (const char *)args - base);
            else bw.EmitI32Const(0);  // nullptr
        }
    }

    void SetNextCallTarget(int id) override {
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitI32ConstFunctionRef(function_ids[id]);
        bw.EmitCall(import_snct);
    }

    void EmitGenericInst(int opc, const int *args, int arity, bool is_vararg, int target) override {
        if (!is_vararg) {
            for (int i = 0; i < arity; i++) bw.EmitI32Const(args[i]);
        }
        if (target >= 0) { bw.EmitI32ConstFunctionRef(function_ids[target]); }
        bw.EmitCall((size_t)opc);  // Opcodes are the 0..N of imports.
        bw.EmitSetLocal(1 /*SP*/);
    }

    void EmitCall(int id) override {
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitGetLocal(1 /*SP*/);
        bw.EmitCall(function_ids[id]);
        bw.EmitSetLocal(1 /*SP*/);
    }

    void EmitCallIndirect() override {
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitGetLocal(1 /*SP*/);
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitCall(import_gnct);
        bw.EmitCallIndirect(TI_I_II);
        bw.EmitSetLocal(1 /*SP*/);
    }

    void EmitCallIndirectNull() override {
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitCall(import_gnct);
        bw.EmitIf(WASM::VOID);
        EmitCallIndirect();
        bw.EmitEnd();
    }

    void EmitExternCall(string_view name) override {
        assert(name == "GLFrame");
        bw.EmitGetLocal(0 /*VM*/);
        bw.EmitGetLocal(1 /*SP*/);
        bw.EmitCall(import_gl_frame);
        bw.EmitSetLocal(1 /*SP*/);
    }

    void EmitJumpTable(const int *args, vector<int> &block_ids) override {
        auto mini = *args++;
        auto maxi = *args++;
        auto n = maxi - mini + 2;
        vector<size_t> targets;
        set<int> block_order;
        size_t default_target = 0;
        // Find the unique block targets we're going to have, in sorted order
        // (same order as the bytecode).
        for (int i = 0; i < n; i++) {
            auto id = block_ids[args[i]];
            assert(id != -1);
            block_order.insert(id);
        }
        // Now fill in the block nesting counts into the jump table.
        for (int i = 0; i < n; i++) {
            auto block_it = block_order.find(block_ids[args[i]]);
            assert(block_it != block_order.end());
            auto depth = std::distance(block_order.begin(), block_it);  // FIXME: inefficient.
            if (i < n - 1) targets.push_back(depth);
            else default_target = depth;
        }
        // Emit the start of all blocks.
        for (auto id : reverse(block_order)) {
            bw.EmitBlock(WASM::VOID);
            blocks.push_back({ true, id, NH_JUMPTABLE_TO_CASE });
        }
        // Fixme: reduced the cost of this.
        bw.EmitGetLocal(1 /*SP*/);
        bw.EmitI64Load(0);  // Value to switch on
        bw.EmitGetLocal(1 /*SP*/);
        bw.EmitCall(import_drop);  // FIXME: don't know sizeof(Value) here (!)
        bw.EmitSetLocal(1 /*SP*/);
        bw.EmitI64Const(mini);
        bw.EmitI64Sub();
        bw.EmitI32WrapI64();
        bw.EmitBrTable(targets, default_target);
    }

    void EmitHint(NativeHint h) override {
        switch (h) {
            case NH_JUMPTABLE_END:
                break;
            case NH_LOOP_BACK:
                bw.EmitBlock(WASM::VOID);
                bw.EmitLoop(WASM::VOID);
                blocks.push_back({ false, -1, h });
                break;
            case NH_LOOP_REMOVE:
                assert(!blocks.empty() && blocks.back().h == NH_LOOP_BACK);
                bw.EmitEnd();
                bw.EmitEnd();
                blocks.pop_back();
                break;
            // absolute forward jumps.
            case NH_JUMPOUT_START:
                bw.EmitBlock(WASM::VOID);
                blocks.push_back({ true, -1, h });
                break;
            case NH_JUMPOUT_END:
                assert(!blocks.empty() && blocks.back().h == NH_JUMPOUT_START);
                bw.EmitEnd();
                blocks.pop_back();
                break;
            // conditional labelled forward jumps.
            case NH_SWITCH_RANGE_BLOCK:
            case NH_SWITCH_THISCASE_BLOCK:
            case NH_SWITCH_NEXTCASE_BLOCK:
                bw.EmitBlock(WASM::VOID);
                blocks.push_back({ true, -1, h });
                break;
            case NH_SWITCH_RANGE_END:
            case NH_SWITCH_THISCASE_END:
            case NH_SWITCH_NEXTCASE_END:
                assert(!blocks.empty() && blocks.back().h == h - 2);
                bw.EmitEnd();
                blocks.pop_back();
                break;
            case NH_SWITCH_RANGE_JUMP:
            case NH_SWITCH_THISCASE_JUMP:
            case NH_SWITCH_NEXTCASE_JUMP:
                last_conditional_hint = h;
                break;
        }
    }

    void EmitReturn() override {
        bw.EmitGetLocal(1 /*SP*/);
        bw.EmitReturn();
    }

    void InstEnd() override {
    }

    void BlockEnd(int id, bool isexit) override {
        while (!blocks.empty()) {
            auto &b = blocks.back();
            assert(b.id == -1 || b.id >= id);
            if (b.id != id) break;
            bw.EmitEnd();
            blocks.pop_back();
        }
        if (isexit) {
            EmitReturn();
        }
    }

    void FunEnd() override {
        bw.EmitEndFunction();
        assert(blocks.empty());
    }

    void CodeEnd() override {
        bw.EndSection(WASM::Section::Code);
    }

    void VTables(vector<int> &vtables) override {
        bw.BeginSection(WASM::Section::Data);

        vector<int> wid;
        for (auto id : vtables) {
            wid.push_back(id >= 0 ? function_ids[id] : -1);
        }
        bw.AddData(string_view((char *)wid.data(), wid.size() * sizeof(int)), "vtables",
                   sizeof(int));
        for (auto [i, id] : enumerate(vtables)) {
            if (id >= 0) bw.DataFunctionRef(function_ids[id], i * sizeof(int));
        }
    }

    void FileEnd(int /*start_id*/, string_view bytecode_buffer) override {
        // TODO: don't really want to store all of this.
        bw.AddData(bytecode_buffer, "static_data", 16);
        bw.EndSection(WASM::Section::Data);

        bw.Finish();
    }

    void Annotate(string_view /*comment*/) override {
    }
};

string ToWASM(NativeRegistry &natreg, vector<uint8_t> &dest, string_view bytecode_buffer) {
    WASMGenerator wasmgen(dest);
    return ToNative(natreg, wasmgen, bytecode_buffer);
}

}

void unit_test_wasm(bool full) {
    auto vec = WASM::SimpleBinaryWriterTest();
    if (full) {
        auto f = OpenForWriting("simple_binary_writer_test.wasm", true);
        if (f) {
            fwrite(vec.data(), vec.size(), 1, f);
            fclose(f);
        }
    }
}
