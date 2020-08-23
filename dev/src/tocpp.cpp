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
    vector<const int *> jumptables;
    bool cpp;  // If not, plain C.

  public:

    explicit CPPGenerator(string &sd, bool cpp) : sd(sd), cpp(cpp) {}

    void FileStart() override {
        if (cpp) {
            sd +=
                "#include \"lobster/stdafx.h\"\n"
                "#include \"lobster/vmdata.h\"\n"
                "#include \"lobster/vmops.h\"\n"
                "#include \"lobster/compiler.h\"\n"
                "\n"
                "typedef lobster::StackPtr StackPtr;\n"
                "typedef lobster::VM &VMRef;\n"
                "typedef lobster::fun_base_t fun_base_t;\n"
                "\n"
                "#if LOBSTER_ENGINE\n"
                "    // FIXME: This makes SDL not modular, but without it it will miss the SDLMain indirection.\n"
                "    #include \"lobster/sdlincludes.h\"\n"
                "    #include \"lobster/sdlinterface.h\"\n"
                "    extern \"C\" StackPtr GLFrame(StackPtr sp, VMRef vm);\n"
                "#endif\n"
                "\n"
                ;
        } else {
            sd +=
                "typedef long long *StackPtr;\n"
                "typedef void *VMRef;\n"
                "typedef StackPtr(*fun_base_t)(VMRef, StackPtr);\n"
                "extern  StackPtr GLFrame(StackPtr sp, VMRef vm);\n"
                "\n"
                ;

            auto args = [&](int A) {
                if (A == 9) sd += ", const int *";
                else for (int i = 0; i < A; i++) sd += ", int";
            };

            #define F(N, A) \
                sd += "StackPtr U_" #N "(VMRef, StackPtr"; args(A); sd += ");\n";
                LVALOPNAMES
            #undef F
            #define F(N, A) \
                sd += "StackPtr U_" #N "(VMRef, StackPtr"; args(A); sd += ");\n";
                ILBASENAMES
            #undef F
            #define F(N, A) \
                sd += "StackPtr U_" #N "(VMRef, StackPtr"; args(A); sd += ", fun_base_t);\n";
                ILCALLNAMES
            #undef F
            #define F(N, A) \
                sd += "StackPtr U_" #N "(VMRef, StackPtr);\n";
                ILJUMPNAMES1
            #undef F
            #define F(N, A) \
                sd += "StackPtr U_" #N "(VMRef, StackPtr, int);\n";
                ILJUMPNAMES2
            #undef F

            sd += "fun_base_t GetNextCallTarget(VMRef);\n"
                  "StackPtr Drop(StackPtr);\n"
                  "\n";
        }
    }

    void DeclareFun(int id) override {
        append(sd, "static StackPtr fun_", id, "(VMRef, StackPtr);\n");
    }

    void BeforeBlocks(int /*start_id*/, string_view /*bytecode_buffer*/) override {
        sd += "\n";
    }

    void FunStart(const bytecode::Function *f, int id) override {
        sd += "\n";
        if (f) append(sd, "// ", f->name()->string_view(), "\n");
        append(sd, "static StackPtr fun_", id,
            "(VMRef vm, StackPtr sp) {\n");
    }

    void InstStart() override {
        sd += "    ";
    }

    void EmitJump(int id) override {
        append(sd, "goto block", id, ";");
    }

    void EmitDynJump() {
        if (cpp) sd += "sp = vm.next_call_target(vm, sp);";
        else sd += "sp = GetNextCallTarget(vm)(vm, sp);";
    }

    void EmitConditionalJump(int opc, int id, int df) override {
        append(sd, "sp = U_", ILNames()[opc], "(vm, sp");
        if (df >= 0) append(sd, ", ", df);
        if (cpp) {
            append(sd, "); if (Pop(sp).False()) ");
            EmitJump(id);
        } else {
            // FIXME: simplify.
            append(sd, "); { long long top = *sp; sp = Drop(sp); if (!top) ");
            EmitJump(id);
            sd += " }";
        }
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
            append(sd, "fun_", target);
        }
        sd += ");";
    }

    void EmitCall(int id) override {
        append(sd, " sp = fun_", id, "(vm, sp);");
    }

    void EmitCallIndirect() override {
        sd += " ";
        EmitDynJump();
    }

    void EmitCallIndirectNull() override {
        if (cpp) sd += " if (vm.next_call_target) ";
        else sd += " if (GetNextCallTarget(vm)) ";
        EmitDynJump();
    }

    void EmitExternCall(string_view name) override {
        append(sd, "sp = ", name, "(sp, vm);");
    }

    void EmitJumpTable(const int *args) override {
        if (cpp) {
            sd += "switch (Pop(sp).ival()) {";
        } else {
            // FIXME: simplify.
            append(sd, "{ long long top = *sp; sp = Drop(sp); switch (top) {");
        }

        jumptables.push_back(args);
    }

    void EmitHint(NativeHint h, int id) override {
        switch (h) {
            case NH_BLOCK_START:
                append(sd, "block", id, ":");
                break;
            case NH_JUMPTABLE_CASE_START: {
                assert(!jumptables.empty());
                auto t = jumptables.back();
                auto mini = *t++;
                auto maxi = *t++;
                for (auto i = mini; i <= maxi; i++) {
                    if (*t++ == id) {
                        append(sd, "case ", i, ":");
                    }
                }
                if (*t++ == id) {
                    append(sd, "default:");
                }
                break;
            }
            case NH_JUMPTABLE_END:
                if (cpp) sd += "} // switch";
                else sd += "}} // switch";
                jumptables.pop_back();
                break;
            default:
                append(sd, "// hint: ", NHNames()[h]);
                break;
        }
    }

    void EmitReturn() override {
        sd += " return sp;";
    }

    void InstEnd() override {
        sd += "\n";
    }

    void Exit() override {
        sd += "    return sp;\n";
    }

    void FunEnd() override {
        sd += "}\n";
    }

    void CodeEnd() override {
    }

    void VTables(vector<int> &vtables) override {
        if (cpp) sd += "\nstatic";
        else sd += "\nextern ";
        sd += " const fun_base_t vtables[] = {\n";
        for (auto id : vtables) {
            sd += "    ";
            if (id >= 0) append(sd, "fun_", id);
            else sd += "0";
            sd += ",\n";
        }
        sd += "    0\n};\n";  // Make sure table is never empty.
    }

    void FileEnd(int start_id, string_view bytecode_buffer) override {
        if (cpp) {
            // FIXME: this obviously does NOT need to include the actual bytecode, just the metadata.
            // in fact, it be nice if those were in readable format in the generated code.
            sd += "\nstatic const int bytecodefb[] = {";
            auto bytecode_ints = (const int *)bytecode_buffer.data();
            for (size_t i = 0; i < bytecode_buffer.length() / sizeof(int); i++) {
                if ((i & 0xF) == 0) sd += "\n ";
                append(sd, " ", bytecode_ints[i], ",");
            }
            sd += "\n};\n\n";
        }
        if (cpp) sd += "extern \"C\" ";
        sd += "StackPtr compiled_entry_point(VMRef vm, StackPtr sp) {\n";
        append(sd, "    return fun_", start_id, "(vm, sp);\n}\n\n");
        if (cpp) {
            sd += "int main(int argc, char *argv[]) {\n";
            sd += "    // This is hard-coded to call compiled_entry_point()\n";
            sd += "    return RunCompiledCodeMain(argc, argv, ";
            append(sd, "(uint8_t *)bytecodefb, ", bytecode_buffer.size(), ", vtables);\n}\n");
        }
    }

    void Annotate(string_view comment) override {
        append(sd, " /* ", comment, " */");
    }
};

string ToCPP(NativeRegistry &natreg, string &sd, string_view bytecode_buffer, bool cpp) {
    CPPGenerator cppgen(sd, cpp);
    return ToNative(natreg, cppgen, bytecode_buffer);
}

}  // namespace lobster

