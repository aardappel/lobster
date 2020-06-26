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
#include "lobster/disasm.h"  // Some shared bytecode utilities.
#include "lobster/compiler.h"
#include "lobster/tonative.h"

namespace lobster {

class CPPGenerator : public NativeGenerator {
    string &sd;
    int tail_calls_in_a_row = 0;

  public:

    explicit CPPGenerator(string &sd) : sd(sd) {}

    void FileStart() override {
        sd +=
            "#include \"lobster/stdafx.h\"\n"
            "#include \"lobster/vmdata.h\"\n"
            "#include \"lobster/vmops.h\"\n"
            "#include \"lobster/compiler.h\"\n"
            "\n"
            "#if LOBSTER_ENGINE\n"
            "    // FIXME: This makes SDL not modular, but without it it will miss the SDLMain indirection.\n"
            "    #include \"lobster/sdlincludes.h\"\n"
            "    #include \"lobster/sdlinterface.h\"\n"
            "#endif\n"         
            "\n"
            "#ifndef VM_COMPILED_CODE_MODE\n"
            "    #error VM_COMPILED_CODE_MODE must be set for the entire code base.\n"
            "#endif\n"
            "\n"
            "#ifdef _WIN32\n"
            "    #pragma warning (disable: 4102)  // Unused label.\n"
            "#endif\n"
            "\n";
    }

    void DeclareBlock(int id) override {
        append(sd, "static void *block", id, "(lobster::VM &, lobster::StackPtr &);\n");
    }

    void BeforeBlocks(int /*start_id*/, string_view /*bytecode_buffer*/) override {
        sd += "\n";
    }

    void FunStart(const bytecode::Function *f) override {
        sd += "\n";
        if (f) append(sd, "// ", f->name()->string_view(), "\n");
    }

    void BlockStart(int id) override {
        append(sd, "static void *block", id,
                   "(lobster::VM &vm, lobster::StackPtr &spr) {\n    auto sp = spr;\n");
    }

    void InstStart() override {
        sd += "    { ";
    }

    void EmitJump(int id) override {
        // FIXME: if we make all forward calls tail calls, then under
        // WASM/Emscripten/V8, we occasionally run out of stack.
        // This bounds the number of tail calls in a simple way,
        // but this is not correct, in that call targets are not necessarily
        // in linear order, though it should catch most long runs of calls.
        // We really need to do this with an algorithm that better understands
        // the call structure instead. Hopefully this bounding will allow
        // us to keep some of the performance advantage of tail calls vs
        // not doing them at all.
        sd += "{ spr = sp; ";
        if (tail_calls_in_a_row > 10 || id <= current_block_id) {
            // A backwards jump, go via the trampoline to be safe
            // (just in-case the compiler doesn't optimize tail calls).
            append(sd, "return (void *)block", id, ";");
            tail_calls_in_a_row = 0;
        } else {
            // A forwards call, should be safe to tail-call.
            append(sd, "return block", id, "(vm, spr);");
            tail_calls_in_a_row++;
        }
        sd += " }";
    }

    void EmitDynJump() {
        sd += "{ spr = sp; return (void *)vm.next_call_target; }";
    }

    void EmitConditionalJump(int opc, int id) override {
        append(sd, "sp = U_", ILNames()[opc], "(vm, sp); if (Pop(sp).False()) ");
        EmitJump(id);
    }

    void EmitOperands(const char * /*base*/, const int *args, int arity, bool is_vararg) override {
        if (is_vararg && arity) {
            sd += "static int args[] = {";
            for (int i = 0; i < arity; i++) {
                if (i) sd += ", ";
                append(sd, args[i]);
            }
            sd += "}; ";
        }
    }

    void SetNextCallTarget(int id) override {
        append(sd, "vm.next_call_target = block", id, "; ");
    }

    void EmitGenericInst(int opc, const int *args, int arity, bool is_vararg, int target) override {
        append(sd, "sp = U_", ILNames()[opc], "(vm, sp");
        if (is_vararg) {
            sd += ", args";
        } else {
            for (int i = 0; i < arity; i++) {
                sd += ", ";
                append(sd, args[i]);
            }
        }
        if (target >= 0) {
            sd += ", ";
            append(sd, "block", target);
        }
        sd += ");";
    }

    void EmitCall(int id) override {
        sd += " ";
        EmitJump(id);
    }

    void EmitCallIndirect() override {
        sd += " ";
        EmitDynJump();
    }

    void EmitCallIndirectNull() override {
        sd += " if (vm.next_call_target) ";
        EmitDynJump();
    }

    void InstEnd() override {
        sd += " }\n";
    }

    void BlockEnd(int id, bool already_returned, bool is_exit) override {
        if (!already_returned) {
            sd += "    { ";
            if (is_exit) EmitDynJump(); else EmitJump(id);
            sd += " }\n";
        }
        sd += "}\n";
    }

    void CodeEnd() override {
    }

    void VTables(vector<int> &vtables) override {
        sd += "\nstatic const lobster::block_base_t vtables[] = {\n";
        for (auto id : vtables) {
            sd += "    ";
            if (id >= 0) append(sd, "block", id);
            else sd += "0";
            sd += ",\n";
        }
        sd += "    0\n};\n";  // Make sure table is never empty.
    }

    void FileEnd(int start_id, string_view bytecode_buffer) override {
        // FIXME: this obviously does NOT need to include the actual bytecode, just the metadata.
        // in fact, it be nice if those were in readable format in the generated code.
        sd += "\nstatic const int bytecodefb[] = {";
        auto bytecode_ints = (const int *)bytecode_buffer.data();
        for (size_t i = 0; i < bytecode_buffer.length() / sizeof(int); i++) {
            if ((i & 0xF) == 0) sd += "\n ";
            append(sd, " ", bytecode_ints[i], ",");
        }
        sd += "\n};\n\n";
        sd += "int main(int argc, char *argv[]){\n";
        sd += "    return RunCompiledCodeMain(argc, argv, (void *)";
        append(sd, "block", start_id);
        append(sd, ", bytecodefb, ", bytecode_buffer.size(), ", vtables);\n}\n");
    }

    void Annotate(string_view comment) override {
        append(sd, " /* ", comment, " */");
    }
};

string ToCPP(NativeRegistry &natreg, string &sd, string_view bytecode_buffer) {
    CPPGenerator cppgen(sd);
    return ToNative(natreg, cppgen, bytecode_buffer);
}

}  // namespace lobster

