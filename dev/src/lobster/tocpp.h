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

namespace lobster {

void CodeGen::EmitCForPrev() {
    if (last_op_start == (size_t)-1) return;
    if (temp_codegen.size() <= last_op_start) temp_codegen.resize(last_op_start + 1);
    const int *ip = code.data() + last_op_start;
    int opc = *ip++;
    const int *args = ip + 1;
    int regso = -1;
    /* auto arity = */ParseOpAndGetArity(opc, ip, regso);
    // We could store the value of ip here, to verify it is the same as next instr start.
    string sd;
    auto comment = [&](string_view c) { append(sd, " // ", c); };
    sp = cat("regs + ", regso);
    switch (opc) {
        case IL_BCALLRETV:
        case IL_BCALLRET0:
        case IL_BCALLRET1:
        case IL_BCALLRET2:
        case IL_BCALLRET3:
        case IL_BCALLRET4:
        case IL_BCALLRET5:
        case IL_BCALLRET6:
        case IL_BCALLRET7:
            if (parser.natreg.nfuns[args[0]]->IsGLFrame()) {
                append(sd, "GLFrame(", sp, ", vm);");
            } else {
                append(sd, "U_", ILNames()[opc], "(vm, ", sp, ", ", args[0], ", ", args[1], ");");
                comment(parser.natreg.nfuns[args[0]]->name);
            }
            break;
        case IL_GOTOFUNEXIT:
            append(sd, "goto epilogue;");
            break;
        case IL_KEEPREFLOOP:
            append(sd, "DecVal(vm, keepvar[", args[1], "]); ");
            [[fallthrough]];
        case IL_KEEPREF:
            append(sd, "keepvar[", args[1], "] = TopM(", sp, ", ", args[0], ");");
            break;
        case IL_PUSHFUN:
            append(sd, "U_PUSHFUN(vm, ", sp, ", 0, ", "fun_", args[0], ");");
            break;
        case IL_CALL: {
            append(sd, "fun_", args[0], "(vm, ", sp, ");");
            auto sf_idx = args[0];
            comment("call: " + st.subfunctiontable[sf_idx]->parent->name);
            break;
        }
        case IL_CALLV:
            append(sd, "U_CALLV(vm, ", sp, "); ");
            if (cpp) append(sd, "vm.next_call_target(vm, regs + ", regso - 1, ");");
            else append(sd, "GetNextCallTarget(vm)(vm, regs + ", regso - 1, ");");
            break;
        case IL_DDCALL:
            append(sd, "U_DDCALL(vm, ", sp, ", ", args[0], ", ", args[1], "); ");
            if (cpp) append(sd, "vm.next_call_target(vm, ", sp, ");");
            else append(sd, "GetNextCallTarget(vm)(vm, ", sp, ");");
            break;
    }
    temp_codegen[last_op_start] = std::move(sd);
}

string CodeGen::IdName(int i, bool is_whole_struct) {
    auto idx = sids[i].ididx();
    auto &basename = st.identtable[idx]->name;
    auto ti = (TypeInfo *)(&type_table[sids[i].typeidx()]);
    if (is_whole_struct || !IsStruct(ti->t)) {
        return basename;
    } else {
        int j = i;
        // FIXME: this theoretically can span 2 specializations of the same var.
        while (j && sids[j - 1].ididx() == idx) j--;
        return cat(basename, "+", i - j);
    }
};

void CodeGen::Prologue(string &sd) {
    if (cpp) {
        sd +=
            "#include \"lobster/stdafx.h\"\n"
            "#include \"lobster/vmdata.h\"\n"
            "#include \"lobster/vmops.h\"\n"
            "#include \"lobster/compiler.h\"\n"
            "\n"
            "typedef lobster::Value Value;\n"
            "typedef lobster::StackPtr StackPtr;\n"
            "typedef lobster::VM &VMRef;\n"
            "typedef lobster::fun_base_t fun_base_t;\n"
            "\n"
            "#if LOBSTER_ENGINE\n"
            "    // FIXME: This makes SDL not modular, but without it it will miss the SDLMain indirection.\n"
            "    #include \"lobster/sdlincludes.h\"\n"
            "    #include \"lobster/sdlinterface.h\"\n"
            "    extern \"C\" void GLFrame(StackPtr sp, VMRef vm);\n"
            "#endif\n"
            "\n"
            ;
    } else {
        sd +=
            // This needs to correspond to the C++ Value, enforced in Entry().
            "typedef struct {\n"
            "    union {\n"
            "        long long ival;\n"
            "        double fval;\n"
            "        void *rval;\n"
            "    };\n"
            #if RTT_ENABLED
            "    int type;\n"
            #endif
            "} Value;\n"
            "typedef Value *StackPtr;\n"
            "typedef void *VMRef;\n"
            "typedef void(*fun_base_t)(VMRef, StackPtr);\n"
            "#define Pop(sp) (*--(sp))\n"
            "#define Push(sp, V) (*(sp)++ = (V))\n"
            "#define TopM(sp, N) (*((sp) - (N) - 1))\n"
            "struct ___tracy_source_location_data {\n"
            "    const char *name;\n"
            "    const char *function;\n"
            "    const char *file;\n"
            "    unsigned int line;\n"
            "    unsigned int color;\n"
            "};\n"
            "struct ___tracy_c_zone_context {\n"
            "    unsigned int id;\n"
            "    int active;\n"
            "};\n"
            "\n"
            ;

        auto args = [&](int A) {
            for (int i = 0; i < A; i++) sd += ", int";
        };

        #define F(N, A, USE, DEF) \
            sd += "void U_" #N "(VMRef, StackPtr"; args(A); sd += ");\n";
            ILBASENAMES
        #undef F
        #define F(N, A, USE, DEF) \
            sd += "void U_" #N "(VMRef, StackPtr"; args(A); sd += ", fun_base_t);\n";
            ILCALLNAMES
        #undef F
        #define F(N, A, USE, DEF) \
            sd += "void U_" #N "(VMRef, StackPtr, const int *);\n";
            ILVARARGNAMES
        #undef F
        #define F(N, A, USE, DEF) \
            sd += "int U_" #N "(VMRef, StackPtr);\n";
            ILJUMPNAMES1
        #undef F
        #define F(N, A, USE, DEF) \
            sd += "int U_" #N "(VMRef, StackPtr, int);\n";
            ILJUMPNAMES2
        #undef F

        sd += "extern fun_base_t GetNextCallTarget(VMRef);\n"
              "extern void Entry(int);\n"
              "extern void GLFrame(StackPtr, VMRef);\n"
              "extern void SwapVars(VMRef, int, StackPtr, int);\n"
              "extern void BackupVar(VMRef, int);\n"
              "extern void NilVal(Value *);\n"
              "extern void DecOwned(VMRef, int);\n"
              "extern void DecVal(VMRef, Value);\n"
              "extern void RestoreBackup(VMRef, int);\n"
              "extern StackPtr PopArg(VMRef, int, StackPtr);\n"
              "extern void SetLVal(VMRef, Value *);\n"
              "extern int RetSlots(VMRef);\n"
              "extern int GetTypeSwitchID(VMRef, Value, int);\n"
              "extern void PushFunId(VMRef, const int *, StackPtr);\n"
              "extern void PopFunId(VMRef);\n"
              #if LOBSTER_FRAME_PROFILER
              "extern struct ___tracy_c_zone_context StartProfile(struct ___tracy_source_location_data *);\n"
              "extern void EndProfile(struct ___tracy_c_zone_context);\n"
              #endif
              "\n";
    }

    if (runtime_checks >= RUNTIME_STACK_TRACE) {
        append(sd, "extern const int funinfo_table[];\n\n");
    }
}

void CodeGen::ToCPP(string &sd) {
    vector<int> var_to_local;
    var_to_local.resize(sids.size(), -1);

    const int *ip = code.data();
    // Skip past 1st jump.
    assert(*ip == IL_JUMP);
    ip += 2;
    auto starting_ip = code.data() + *ip++;
    while (ip < code.data() + code.size()) {
        int id = (int)(ip - code.data());
        if (*ip == IL_FUNSTART || ip == starting_ip) {
            auto sf_idx = ip == starting_ip ? 8888888 : ip[2];
            if (sf_idx >= 0) append(sd, "static void fun_", sf_idx, "(VMRef, StackPtr);\n");
            starting_point = sf_idx;
        }
        int opc = *ip++;
        if (opc < 0 || opc >= IL_MAX_OPS) {
            THROW_OR_ABORT(cat("Corrupt bytecode: ", opc, " at: ", id));
        }
        int regso = -1;
        ParseOpAndGetArity(opc, ip, regso);
    }
    sd += "\n";
    vector<const int *> jumptables;
    ip = code.data() + 3;  // Past first IL_JUMP.
    const int *funstart = nullptr;
    int nkeepvars = 0;
    string sdt;
    int opc = -1;
    const int *args = nullptr;
    bool has_profile = false;
    auto comment = [&](string_view c) { append(sd, " // ", c); };
    while (ip < code.data() + code.size()) {
        int id = (int)(ip - code.data());
        bool is_start = ip == starting_ip;
        opc = *ip++;
        args = ip + 1;
        if (opc == IL_FUNSTART || is_start) {
            funstart = args;
            nkeepvars = 0;
            sdt.clear();
            has_profile = false;
            sd += "\n";
            auto sf_idx = is_start ? 8888888 : *funstart;
            auto name =
                sf_idx < 8888888 ? st.subfunctiontable[sf_idx]->parent->name : string_view{};
            append(sd, "// ", name, "\n");
            append(sd, "static void fun_", sf_idx, "(VMRef vm, StackPtr psp) {\n");
            const int *funstartend = nullptr;
            int numlocals = 0;
            if (opc == IL_FUNSTART) {
                auto fip = funstart;
                fip++;  // sf.idx
                auto regs_max = *fip++;
                auto nargs_fun = *fip++;
                auto nargs = fip;
                fip += nargs_fun;
                int ndefsave = *fip++;
                auto defs = fip;
                fip += ndefsave;
                nkeepvars = *fip++;
                funstartend = fip;
                #ifndef NDEBUG
                var_to_local.clear();
                var_to_local.resize(sids.size(), -1);
                #endif
                for (int j = 0; j < 2; j++) {
                    auto vars = j ? defs : nargs;
                    auto len = j ? ndefsave : nargs_fun;
                    for (int i = 0; i < len; i++) {
                        auto varidx = vars[i];
                        if (!sids[varidx].used_as_freevar()) {
                            var_to_local[varidx] = numlocals++;
                        }
                    }
                }
                // FIXME: don't emit array.
                // (there may be functions that don't use regs yet still refer to sp?)
                append(sd, "    Value regs[", std::max(1, regs_max), "];\n");
                if (!regs_max) append(sd, "    (void)regs;\n");
                if (nkeepvars) append(sd, "    Value keepvar[", nkeepvars, "];\n");
                if (numlocals) append(sd, "    Value locals[", numlocals, "];\n");
            } else {
                // Final program return at most 1 value.
                append(sd, "    Value regs[1];\n");
            }
            if (opc == IL_FUNSTART) {
                auto fip = funstart;
                fip++;  // sf.idx
                fip++;  // regs_max.
                auto nargs_fun = *fip++;
                for (int i = 0; i < nargs_fun; i++) {
                    auto varidx = fip[i];
                    if (sids[varidx].used_as_freevar()) {
                        append(sd, "    SwapVars(vm, ", varidx, ", psp, ", nargs_fun - i, ");\n");
                    } else {
                        append(sd, "    locals[", var_to_local[varidx], "] = *(psp - ",
                               nargs_fun - i, ");\n");
                    }
                }
                fip += nargs_fun;
                int ndefsave = *fip++;
                for (int i = 0; i < ndefsave; i++) {
                    // for most locals, this just saves an nil, only in recursive cases it has an
                    // actual value.
                    auto varidx = *fip++;
                    if (sids[varidx].used_as_freevar()) {
                        append(sd, "    BackupVar(vm, ", varidx, ");\n");
                    } else {
                        // FIXME: it should even be unnecessary to initialize them, but its possible
                        // there is a return before they're fully initialized, and then the decr of
                        // owned vars may cause these to be accessed.
                        if (cpp)
                            append(sd, "    locals[", var_to_local[varidx],
                                   "] = lobster::NilVal();\n");  // FIXME ns
                        else
                            append(sd, "    NilVal(&locals[", var_to_local[varidx], "]);\n");
                    }
                }
                if (runtime_checks >= RUNTIME_STACK_TRACE) {
                    // FIXME: can make this just and index and instead store funinfo_table ref in
                    // VM. Calling this here because now locals have been fully initialized.
                    append(sd, "    PushFunId(vm, funinfo_table + ", funstarttables.size(), ", ",
                           numlocals ? "locals" : "0", ");\n");
                    // TODO: this doesn't need to correspond to to funstart, can stick any info we
                    // want in here.
                    funstarttables.insert(funstarttables.end(), funstart, funstartend);
                }
                nkeepvars = *fip++;
                for (int i = 0; i < nkeepvars; i++) {
                    if (cpp)
                        append(sd, "    keepvar[", i, "] = lobster::NilVal();\n");  // FIXME ns
                    else
                        append(sd, "    NilVal(&keepvar[", i, "]);\n");
                }
            }
        }
        int regso = -1;
        auto arity = ParseOpAndGetArity(opc, ip, regso);
        if (opc == IL_FUNSTART) continue;
        append(sd, "    ");

        if (id < temp_codegen.size() && !temp_codegen[id].empty()) {
            append(sd, temp_codegen[id], "\n");
            continue;
        }

        sp = cat("regs + ", regso);
        switch (opc) {
            case IL_PUSHVARL:
                append(sd, "regs[", regso, "] = locals[", var_to_local[args[0]], "];");
                comment(IdName(args[0], false));
                break;
            case IL_PUSHVARVL:
                for (int i = 0; i < args[1]; i++) {
                    append(sd, "regs[", regso + i, "] = locals[", var_to_local[args[0] + i], "];");
                }
                comment(IdName(args[0], true));
                break;
            case IL_LVAL_VARL:
                append(sd, "SetLVal(vm, &locals[", var_to_local[args[0]], "]);");
                comment(IdName(args[0], false));
                break;
            case IL_JUMP:
                append(sd, "goto block", args[0], ";");
                break;
            case IL_JUMPFAIL:
            case IL_JUMPFAILR:
            case IL_JUMPNOFAIL:
            case IL_JUMPNOFAILR:
            case IL_IFOR:
            case IL_SFOR:
            case IL_VFOR:
            case IL_JUMPIFUNWOUND:
            case IL_JUMPIFSTATICLF:
            case IL_JUMPIFMEMBERLF: {
                auto id = args[opc >= IL_JUMPIFUNWOUND ? 1 : 0];
                assert(id >= 0);
                append(sd, "if (!U_", ILNames()[opc], "(vm, ", sp);
                if (opc >= IL_JUMPIFUNWOUND) append(sd, ", ", args[0]);
                append(sd, ")) goto block", id, ";");
                break;
            }
            case IL_BLOCK_START:
                // FIXME: added ";" because blocks may end up just before "}" at the end of a
                // switch, and generate warnings/errors. Ideally not generate this block at all.
                append(sd, "block", id, ":;");
                break;
            case IL_JUMP_TABLE_DISPATCH:
                if (cpp) {
                    append(sd, "switch (GetTypeSwitchID(vm, regs[", regso - 1, "], ", args[0],
                           ")) {");
                } else {
                    append(sd, "{ int top = GetTypeSwitchID(vm, regs[", regso - 1, "], ", args[0],
                           "); switch (top) {");
                }
                jumptables.push_back(args + 1);
                break;
            case IL_JUMP_TABLE:
                if (cpp) {
                    append(sd, "switch (regs[", regso - 1, "].ival()) {");
                } else {
                    append(sd, "{ long long top = regs[", regso - 1, "].ival; switch (top) {");
                }
                jumptables.push_back(args);
                break;
            case IL_JUMP_TABLE_CASE_START: {
                auto t = jumptables.back();
                auto mini = *t++;
                auto maxi = *t++;
                for (auto i = mini; i <= maxi; i++) {
                    if (*t++ == id) append(sd, "case ", i, ":");
                }
                if (*t++ == id) append(sd, "default:");
                break;
            }
            case IL_JUMP_TABLE_END: {
                if (cpp)
                    sd += "} // switch";
                else
                    sd += "}} // switch";
                jumptables.pop_back();
                break;
            }
            case IL_BCALLRETV:
            case IL_BCALLRET0:
            case IL_BCALLRET1:
            case IL_BCALLRET2:
            case IL_BCALLRET3:
            case IL_BCALLRET4:
            case IL_BCALLRET5:
            case IL_BCALLRET6:
            case IL_BCALLRET7:
                assert(false);
                break;
            case IL_RETURNLOCAL:
            case IL_RETURNNONLOCAL:
            case IL_RETURNANY: {
                // FIXME: emit epilogue stuff only once at end of function.
                assert(funstart);
                auto fip = funstart;
                fip++;  // function id.
                fip++;  // regs_max.
                auto nargs = *fip++;
                auto freevars = fip + nargs;
                fip += nargs;
                auto ndef = *fip++;
                auto defvars = fip;
                fip += ndef;
                fip++;  // nkeepvars, already parsed above
                int nrets = args[0];
                if (opc == IL_RETURNLOCAL) {
                    append(sd, "U_RETURNLOCAL(vm, 0, ", nrets, ");");
                } else if (opc == IL_RETURNNONLOCAL) {
                    append(sd, "U_RETURNNONLOCAL(vm, 0, ", nrets, ", ", args[1], ");");
                } else {
                    append(sd, "U_RETURNANY(vm, 0, ", nrets, ");");
                }
                auto ownedvars = *fip++;
                for (int i = 0; i < ownedvars; i++) {
                    auto varidx = *fip++;
                    if (sids[varidx].used_as_freevar()) {
                        append(sd, "\n    DecOwned(vm, ", varidx, ");");
                    } else {
                        append(sd, "\n    DecVal(vm, locals[", var_to_local[varidx], "]);");
                    }
                }
                while (nargs--) {
                    auto varidx = *--freevars;
                    if (sids[varidx].used_as_freevar()) {
                        append(sd, "\n    psp = PopArg(vm, ", varidx, ", psp);");
                    } else {
                        // TODO: move to when we obtain the arg?
                        append(sd, "\n    Pop(psp);");
                    }
                }
                if (opc == IL_RETURNANY) {
                    append(sd,
                           "\n    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) "
                           "Push(psp, regs[i + ",
                           regso - nrets, "]); }");
                } else {
                    for (int i = 0; i < nrets; i++) {
                        append(sd, "\n    Push(psp, regs[", i + regso - nrets, "]);");
                    }
                }
                sdt.clear();  // FIXME: remove
                for (int i = ndef - 1; i >= 0; i--) {
                    auto varidx = defvars[i];
                    if (sids[varidx].used_as_freevar()) {
                        append(sdt, "    RestoreBackup(vm, ", varidx, ");\n");
                    }
                }
                if (opc != IL_RETURNANY) {
                    append(sd, "\n    goto epilogue;");
                }
                break;
            }
            case IL_GOTOFUNEXIT:
                assert(false);
                break;
            case IL_KEEPREFLOOP:
                assert(false);
                break;
            case IL_KEEPREF:
                assert(false);
                break;
            case IL_PUSHFUN:
                assert(false);
                break;
            case IL_CALL:
                assert(false);
                break;
            case IL_CALLV:
                assert(false);
                break;
            case IL_DDCALL:
                assert(false);
                break;
            case IL_PROFILE: {
                string name;
                EscapeAndQuote(stringtable[args[0]], name);
                append(sd, "static struct ___tracy_source_location_data tsld = { ", name, ", ",
                       name, ", \"\", 0, 0x888800 }; struct ___tracy_c_zone_context ctx = ",
                       cpp ? "lobster::" : "", "StartProfile(&tsld);");
                has_profile = true;
                break;
            }
            default:
                assert(ILArity()[opc] != ILUNKNOWN);
                append(sd, "U_", ILNames()[opc], "(vm, ", sp, "");
                for (int i = 0; i < arity; i++) {
                    sd += ", ";
                    append(sd, args[i]);
                }
                sd += ");";
                switch (opc) {
                    case IL_PUSHVARF:
                    case IL_PUSHVARVF:
                    case IL_LVAL_VARF:
                        comment(IdName(args[0], false));
                        break;
                    case IL_PUSHSTR: {
                        auto sv = stringtable[args[0]];
                        sv = sv.substr(0, 50);
                        string q;
                        EscapeAndQuote(sv, q);
                        comment(q);
                        break;
                    }
                    case IL_ISTYPE:
                    case IL_NEWOBJECT:
                    case IL_ST2S:
                        auto ti = ((TypeInfo *)(&type_table[args[0]]));
                        if (IsUDT(ti->t)) comment(st.udttable[ti->structidx]->name);
                        break;
                }
                break;
        }

        sd += "\n";
        if (ip == code.data() + code.size() || *ip == IL_FUNSTART || ip == starting_ip) {
            if (opc != IL_EXIT && opc != IL_ABORT) sd += "    epilogue:;\n";
            if (has_profile) {
                append(sd, "    ", cpp ? "lobster::" : "", "EndProfile(ctx);\n");
            }
            if (!sdt.empty()) append(sd, sdt);
            for (int i = 0; i < nkeepvars; i++) {
                append(sd, "    DecVal(vm, keepvar[", i, "]);\n");
            }
            assert(funstart);
            if (*(funstart - 2) == IL_FUNSTART && runtime_checks >= RUNTIME_STACK_TRACE) {
                append(sd, "    PopFunId(vm);\n");
            }
            sd += "}\n";
        }
    }
}

void CodeGen::Epilogue(string &sd, string_view metadata_buffer, string_view custom_pre_init_name) {
    if (cpp) sd += "\nstatic";
    else sd += "\nextern";
    sd += " const fun_base_t vtables[] = {\n";
    for (auto id : vtables) {
        sd += "    ";
        if (id >= 0) {
            auto funstart = code.data() + id;
            append(sd, "fun_", funstart[2]);
        } else if (id <= -2) {
            append(sd, "(fun_base_t)", -id - 2);  // Bit of a hack, would be nice to separate.
        } else {
            sd += "0";
        }
        sd += ",\n";
    }
    sd += "    0\n};\n";  // Make sure table is never empty.

    if (runtime_checks >= RUNTIME_STACK_TRACE) {
        append(sd, "const int funinfo_table[] = {\n    ");
        for (auto [i, d] : enumerate(funstarttables)) {
            if (i && (i & 15) == 0) append(sd, "\n    ");
            append(sd, d, ", ");
        }
        append(sd, "    0\n};\n\n");
    }

    if (cpp) {
        // Output the metadata.
        // TODO: it be nice if this metadate were in readable format in the generated code.
        sd += "\nstatic const int bytecodefb[] = {";
        auto bytecode_ints = (const int *)metadata_buffer.data();
        for (size_t i = 0; i < metadata_buffer.size() / sizeof(int); i++) {
            if ((i & 0xF) == 0) sd += "\n ";
            auto x = bytecode_ints[i];
            sd += " ";
            if (x < 0x10000 && x > -0x10000) {
                append(sd, x);
            } else {
                if (x < 0) sd += "(int)";
                to_string_hex(sd, (uint32_t)x);
            }
            sd += ",";
        }
        sd += "\n};\n\n";
        sd += "static const string_view function_names[] = {\n";
        for (auto f : st.functiontable) {
            sd += "    ";
            EscapeAndQuote(f->name, sd);
            sd += ",\n";
        }
        sd += "};\n\n";
    }
    if (cpp) sd += "extern \"C\" ";
    sd += "void compiled_entry_point(VMRef vm, StackPtr sp) {\n";
    if (cpp) {
        append(sd, "    if (vm.nfr.HashAll() != ", parser.natreg.HashAll(),
               "ULL) vm.BuiltinError(\"code compiled with mismatching builtin function library\");\n");
    } else {
        sd += "    Entry(sizeof(Value));\n";
    }
    append(sd, "    fun_", starting_point, "(vm, sp);\n}\n\n");
    if (cpp) {
        sd += "int main(int argc, char *argv[]) {\n";
        sd += "    // This is hard-coded to call compiled_entry_point()\n";
        if (custom_pre_init_name != "nullptr") append(sd, "    void ", custom_pre_init_name, "(lobster::NativeRegistry &);\n");
        sd += "    lobster::VMMetaData vmmeta = { (uint8_t *)bytecodefb, gsl::make_span(function_names) };\n";
        sd += "    return RunCompiledCodeMain(argc, argv, ";
        append(sd, "&vmmeta, ", metadata_buffer.size(), ", vtables, ", custom_pre_init_name, ", \"",
               (!cpp ? "main.lobster" : ""), "\");\n}\n");
    }
}

}  // namespace lobster

