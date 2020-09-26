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

string ToCPP(NativeRegistry &natreg, string &sd, string_view bytecode_buffer, bool cpp) {
    auto bcf = bytecode::GetBytecodeFile(bytecode_buffer.data());
    if (!FLATBUFFERS_LITTLEENDIAN) return "native code gen requires little endian";
    auto code = (const int *)bcf->bytecode()->Data();  // Assumes we're on a little-endian machine.
    auto typetable = (const type_elem_t *)bcf->typetable()->Data();  // Same.
    auto function_lookup = CreateFunctionLookUp(bcf);

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
            for (int i = 0; i < A; i++) sd += ", int";
        };

        #define F(N, A, USE, DEF) \
            sd += "StackPtr U_" #N "(VMRef, StackPtr"; args(A); sd += ");\n";
            ILBASENAMES
        #undef F
        #define F(N, A, USE, DEF) \
            sd += "StackPtr U_" #N "(VMRef, StackPtr"; args(A); sd += ", fun_base_t);\n";
            ILCALLNAMES
        #undef F
        #define F(N, A, USE, DEF) \
            sd += "StackPtr U_" #N "(VMRef, StackPtr, const int *);\n";
            ILVARARGNAMES
        #undef F
        #define F(N, A, USE, DEF) \
            sd += "StackPtr U_" #N "(VMRef, StackPtr);\n";
            ILJUMPNAMES1
        #undef F
        #define F(N, A, USE, DEF) \
            sd += "StackPtr U_" #N "(VMRef, StackPtr, int);\n";
            ILJUMPNAMES2
        #undef F

        sd += "fun_base_t GetNextCallTarget(VMRef);\n"
                "StackPtr Drop(StackPtr);\n"
                "\n";
    }

    auto len = bcf->bytecode()->Length();
    auto ip = code;
    // Skip past 1st jump.
    assert(*ip == IL_JUMP);
    ip++;
    auto starting_ip = code + *ip++;
    int starting_point = -1;
    while (ip < code + len) {
        int id = (int)(ip - code);
        if (*ip == IL_FUNSTART || ip == starting_ip) {
            append(sd, "static StackPtr fun_", id, "(VMRef, StackPtr);\n");
            starting_point = id;
        }
        if ((false)) {  // Debug corrupt bytecode.
            string da;
            DisAsmIns(natreg, da, ip, code, typetable, bcf, true);
            LOG_DEBUG(da);
        }
        int opc = *ip++;
        if (opc < 0 || opc >= IL_MAX_OPS) {
            return cat("Corrupt bytecode: ", opc, " at: ", id);
        }
        ParseOpAndGetArity(opc, ip);
    }
    sd += "\n";
    vector<const int *> jumptables;
    ip = code + 2;  // Past first IL_JUMP.
    while (ip < code + len) {
        int id = (int)(ip - code);
        int opc = *ip++;
        if (opc == IL_FUNSTART || ip - 1 == starting_ip) {
            auto it = function_lookup.find(id);
            auto f = it != function_lookup.end() ? it->second : nullptr;
            sd += "\n";
            if (f) append(sd, "// ", f->name()->string_view(), "\n");
            append(sd, "static StackPtr fun_", id, "(VMRef vm, StackPtr sp) {\n");
        }
        auto args = ip;
        auto arity = ParseOpAndGetArity(opc, ip);
        sd += "    ";
        if (opc == IL_FUNSTART) {
            sd += "static int args[] = {";
            for (int i = 0; i < arity; i++) {
                if (i) sd += ", ";
                append(sd, args[i]);
            }
            append(sd, "}; sp = U_", ILNames()[opc], "(vm, sp, args);");
        } else if (opc == IL_JUMP) {
            append(sd, "goto block", args[0], ";");
        } else if (CONDJUMP(opc)) {
            auto id = args[opc == IL_JUMPIFUNWOUND ? 1 : 0];
            assert(id >= 0);
            auto df = opc == IL_JUMPIFUNWOUND ? args[0] : -1;
            append(sd, "sp = U_", ILNames()[opc], "(vm, sp");
            if (df >= 0) append(sd, ", ", df);
            if (cpp) {
                append(sd, "); if (Pop(sp).False()) ");
                append(sd, "goto block", id, ";");
            } else {
                // FIXME: simplify.
                append(sd, "); { long long top = *sp; sp = Drop(sp); if (!top) ");
                append(sd, "goto block", id, ";");
                sd += " }";
            }
        } else if (opc == IL_BLOCK_START) {
            // FIXME: added ";" because blocks may end up just before "}" at the end of a
            // switch, and generate warnings/errors. Ideally not generate this block at all.
            append(sd, "block", id, ":;");
        } else if (opc == IL_JUMP_TABLE) {
            if (cpp) {
                sd += "switch (Pop(sp).ival()) {";
            } else {
                // FIXME: simplify.
                append(sd, "{ long long top = *sp; sp = Drop(sp); switch (top) {");
            }
            jumptables.push_back(args);
        } else if (opc == IL_JUMP_TABLE_CASE_START) {
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
        } else if (opc == IL_JUMP_TABLE_END) {
            if (cpp) sd += "} // switch";
            else sd += "}} // switch";
            jumptables.pop_back();
        } else if (ISBCALL(opc) && natreg.nfuns[args[0]]->IsGLFrame()) {
            append(sd, "sp = GLFrame(sp, vm);");
        } else {
            assert(ILArity()[opc] != ILUNKNOWN);
            append(sd, "sp = U_", ILNames()[opc], "(vm, sp");
            for (int i = 0; i < arity; i++) {
                sd += ", ";
                append(sd, args[i]);
            }
            if (opc == IL_PUSHFUN) {
                sd += ", ";
                append(sd, "fun_", args[0]);
            }
            sd += ");";

            string comment;
            if (opc == IL_PUSHVAR || opc == IL_PUSHVARV || opc == IL_LVAL_VAR) {
                comment = IdName(bcf, args[0], typetable, false);
            } else if (opc == IL_PUSHSTR) {
                auto sv = bcf->stringtable()->Get(args[0])->string_view();
                sv = sv.substr(0, 50);
                EscapeAndQuote(sv, comment, true);
            } else if (opc == IL_CALL) {
                auto fs = code + args[0];
                assert(*fs == IL_FUNSTART);
                fs++;
                comment = bcf->functions()->Get(*fs)->name()->string_view();
            } else if (ISBCALL(opc)) {
                comment = natreg.nfuns[args[0]]->name;
            }
            if (!comment.empty()) append(sd, " /* ", comment, " */");

            if (opc == IL_CALL) {
                append(sd, " sp = fun_", args[0], "(vm, sp);");
            } else if (opc == IL_CALLV || opc == IL_DDCALL) {
                sd += " ";
                if (cpp) sd += "sp = vm.next_call_target(vm, sp);";
                else sd += "sp = GetNextCallTarget(vm)(vm, sp);";
            } else if (opc == IL_RETURN || opc == IL_RETURNANY) {
                sd += " return sp;";
            }
        }
        sd += "\n";
        if (opc == IL_EXIT || opc == IL_ABORT) sd += "    return sp;\n";
        if (ip == code + len || *ip == IL_FUNSTART || ip == starting_ip) sd += "}\n";
    }

    if (cpp) sd += "\nstatic";
    else sd += "\nextern ";
    sd += " const fun_base_t vtables[] = {\n";
    for (auto id : *bcf->vtables()) {
        sd += "    ";
        if (id >= 0) append(sd, "fun_", id);
        else sd += "0";
        sd += ",\n";
    }
    sd += "    0\n};\n";  // Make sure table is never empty.

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
    append(sd, "    return fun_", starting_point, "(vm, sp);\n}\n\n");
    if (cpp) {
        sd += "int main(int argc, char *argv[]) {\n";
        sd += "    // This is hard-coded to call compiled_entry_point()\n";
        sd += "    return RunCompiledCodeMain(argc, argv, ";
        append(sd, "(uint8_t *)bytecodefb, ", bytecode_buffer.size(), ", vtables);\n}\n");
    }

    return "";
}

}  // namespace lobster

