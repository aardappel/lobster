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
    const int dispatch = VM_DISPATCH_METHOD;
    int tail_calls_in_a_row = 0;

    string_view Block() {
        return dispatch == VM_DISPATCH_TRAMPOLINE ? "block" : "";
    }

    void JumpInsVar() {
        if (dispatch == VM_DISPATCH_TRAMPOLINE) {
            sd += "return (void *)vm.next_call_target;";
        } else if (dispatch == VM_DISPATCH_SWITCH_GOTO) {
            sd += "{ ip = vm.next_call_target; continue; }";
        }
    }

  public:

    explicit CPPGenerator(string &sd) : sd(sd) {}

    void FileStart() override {
        sd +=
            "#include \"lobster/stdafx.h\"\n"
            "#include \"lobster/vmdata.h\"\n"
            "#include \"lobster/vmops.h\"\n"
            #if LOBSTER_ENGINE
                "#include \"lobster/engine.h\"\n"
            #else
                "#include \"lobster/compiler.h\"\n"
            #endif
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
        if (dispatch == VM_DISPATCH_TRAMPOLINE) {
            append(sd, "static void *block", id, "(lobster::VM &);\n");
        }
    }

    void BeforeBlocks(int start_id, string_view /*bytecode_buffer*/) override {
        sd += "\n";
        if (dispatch == VM_DISPATCH_SWITCH_GOTO) {
            append(sd, "static void *one_gigantic_function(lobster::VM &vm) {\n",
                       "  lobster::block_t ip = ", start_id,
                       ";\n  for(;;) switch(ip) {\n    default: assert(false); continue;\n");
        }
    }

    void FunStart(const bytecode::Function *f) override {
        sd += "\n";
        if (f) append(sd, "// ", f->name()->string_view(), "\n");
    }

    void BlockStart(int id) override {
        if (dispatch == VM_DISPATCH_TRAMPOLINE) {
            append(sd, "static void *block", id, "(lobster::VM &vm) {\n");
        } else if (dispatch == VM_DISPATCH_SWITCH_GOTO) {
            append(sd, "  case ", id, ": block_label", id, ":\n");
        }
    }

    void InstStart() override {
        sd += "    { ";
    }

    void EmitJump(int id) override {
        if (dispatch == VM_DISPATCH_TRAMPOLINE) {
            // FIXME: if we make all forward calls tail calls, then under
            // WASM/Emscripten/V8, we occasionally run out of stack.
            // This bounds the number of tail calls in a simple way,
            // but this is not correct, in that call targets are not necessarily
            // in linear order, though it should catch most long runs of calls.
            // We really need to do this with an algorithm that better understands
            // the call structure instead. Hopefully this bounding will allow
            // us to keep some of the performance advantage of tail calls vs
            // not doing them at all.
            if (tail_calls_in_a_row > 10 || id <= current_block_id) {
                // A backwards jump, go via the trampoline to be safe
                // (just in-case the compiler doesn't optimize tail calls).
                append(sd, "return (void *)block", id, ";");
                tail_calls_in_a_row = 0;
            } else {
                // A forwards call, should be safe to tail-call.
                append(sd, "return block", id, "(vm);");
                tail_calls_in_a_row++;
            }
        } else if (dispatch == VM_DISPATCH_SWITCH_GOTO) {
            append(sd, "goto block_label", id, ";");
        }
    }

    void EmitConditionalJump(int opc, int id) override {
        append(sd, "if (vm.U_", ILNames()[opc], "()) ");
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
        append(sd, "vm.next_call_target = ", Block(), id, "; ");
    }

    void EmitGenericInst(int opc, const int *args, int arity, bool is_vararg, int target) override {
        append(sd, "vm.U_", ILNames()[opc], "(");
        if (is_vararg) {
            sd += "args";
        } else {
            for (int i = 0; i < arity; i++) {
                if (i) sd += ", ";
                append(sd, args[i]);
            }
        }
        if (target >= 0) {
            if (arity) sd += ", ";
            append(sd, Block(), target);
        }
        sd += ");";
    }

    void EmitCall(int id) override {
        sd += " ";
        EmitJump(id);
    }

    void EmitCallIndirect() override {
        sd += " ";
        JumpInsVar();
    }

    void EmitCallIndirectNull() override {
        sd += " if (vm.next_call_target) ";
        JumpInsVar();
    }

    void InstEnd() override {
        sd += " }\n";
    }

    void BlockEnd(int id, bool already_returned, bool is_exit) override {
        if (dispatch == VM_DISPATCH_TRAMPOLINE) {
            if (!already_returned) {
                sd += "    { ";
                if (is_exit) JumpInsVar(); else EmitJump(id);
                sd += " }\n";
            }
            sd += "}\n";
        }
    }

    void CodeEnd() override {
        if (dispatch == VM_DISPATCH_SWITCH_GOTO) {
            sd += "}\n}\n";  // End of gigantic function.
        }
    }

    void VTables(vector<int> &vtables) override {
        sd += "\nstatic const lobster::block_t vtables[] = {\n";
        for (auto id : vtables) {
            sd += "    ";
            if (id >= 0) append(sd, Block(), id);
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
        sd += "    return ";
        #if LOBSTER_ENGINE
            sd += "EngineRunCompiledCodeMain";
        #else
            sd += "ConsoleRunCompiledCodeMain";
        #endif
        sd += "(argc, argv, (void *)";
        if (dispatch == VM_DISPATCH_SWITCH_GOTO) {
            sd += "one_gigantic_function";
        } else if (dispatch == VM_DISPATCH_TRAMPOLINE) {
            append(sd, Block(), start_id);
        }
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

