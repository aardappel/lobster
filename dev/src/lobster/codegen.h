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

#define FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
#include "lobster/bytecode_generated.h"

enum {
    CODEGEN_SPECIAL_FUNCTION_ID_START = 10000000,
    CODEGEN_SPECIAL_FUNCTION_ID_DUMMY = CODEGEN_SPECIAL_FUNCTION_ID_START + 1,
    CODEGEN_SPECIAL_FUNCTION_ID_ENTRY = CODEGEN_SPECIAL_FUNCTION_ID_START + 2,
};

struct CodeGen  {
    vector<metadata::SpecIdent> sids;
    Parser &parser;
    vector<const Node *> linenumbernodes;
    SymbolTable &st;
    vector<type_elem_t> type_table;
    vector<type_elem_t> ser_ids;
    map<small_vector<type_elem_t, 2>, type_elem_t> type_lookup;  // Wasteful, but simple.
    map<iint, type_elem_t> default_ints_lookup;
    map<double, type_elem_t> default_floats_lookup;
    map<small_vector<type_elem_t, 3>, type_elem_t> default_aggregate_lookup;
    vector<TypeLT> rettypes, temptypestack;
    vector<const Node *> loops;
    vector<int> breaks;
    vector<string_view> stringtable;  // sized strings.
    vector<const Node *> node_context;
    int runtime_checks;
    vector<int> vtables;  // -1 = uninit, -2 and lower is case idx, positive is code offset.
    vector<ILOP> tstack;
    size_t tstack_max = 0;
    const SubFunction *cursf = nullptr;
    bool cpp = false;

    // C/C++ codegen related.
    string &c_codegen;
    string cb;
    ILOP opc = IL_UNUSED;
    int regso = 0;
    int f_function_idx = -1;
    int f_regs_max = -1;
    vector<int> f_args;
    vector<int> f_defs;
    int f_keepvars = -1;
    vector<int> ownedvars;
    vector<int> funstarttables;
    vector<int> var_to_local;
    bool has_profile = false;
    string sdt;
    int numlocals = 0;
    int nlabel = 0;

    int TempStackSize() {
        return (int)tstack.size();
    }

    ILOP PopTemp() {
        assert(!tstack.empty());
        auto op = tstack.back();
        tstack.pop_back();
        return op;
    }

    void PushTemp(ILOP op) {
        tstack.push_back(op);
        tstack_max = std::max(tstack_max, tstack.size());
    }

    struct BlockStack {
        vector<ILOP> &tstack;
        size_t start;
        size_t max;
        BlockStack(vector<ILOP> &s) : tstack(s), start(s.size()), max(s.size()) {}
        void Start() { tstack.resize(start); }
        void End() { max = std::max(max, tstack.size()); }
        void Exit(CodeGen &cg) {
            assert(max >= tstack.size());
            while (tstack.size() < max) {
                // A value from something that doesn't return.
                cg.PushTemp(IL_EXIT);
            }
        }
    };

    void EmitOp(ILOP op, int useslots = ILUNKNOWN, int defslots = ILUNKNOWN) {
        opc = op;
        regso = TempStackSize();

        auto uses = ILUses()[op];
        if (uses == ILUNKNOWN) {
            assert(useslots != ILUNKNOWN);
            uses = useslots;
        } else {
            // FIXME: this would be good to enforce, but GenAssign is very sloppy with these numbers..
            //assert(useslots == ILUNKNOWN || useslots == uses);
        }
        for (int i = 0; i < uses; i++) PopTemp();

        auto defs = ILDefs()[op];
        if (defs == ILUNKNOWN) {
            assert(defslots != ILUNKNOWN);
            defs = defslots;
        } else {
            assert(defslots == ILUNKNOWN || defslots == defs);
        }
        for (int i = 0; i < defs; i++) { PushTemp(op); }

        //LOG_DEBUG("cg: ", ILNames()[op], " ", uses, "/", defs, " -> ", tstack.size());
    }

    const int ti_num_udt_fields = 6;
    const int ti_num_udt_per_field = 3;

    type_elem_t PushDefaultValue(ValueType vt, Value val) {
        switch (vt) {
            case V_INT: {
                auto iv = val.ival();
                auto &it = default_ints_lookup[iv];
                if (!it) {
                    it = (type_elem_t)type_table.size();
                    type_table.insert(type_table.end(), (type_elem_t *)&iv,
                                      (type_elem_t *)(&iv + 1));
                }
                return it;
            }
            case V_FLOAT: {
                auto fv = val.fval();
                auto &it = default_floats_lookup[fv];
                if (!it) {
                    it = (type_elem_t)type_table.size();
                    type_table.insert(type_table.end(), (type_elem_t *)&fv,
                                      (type_elem_t *)(&fv + 1));
                }
                return it;
            }
            default:
                return (type_elem_t)0;
        }
    }

    type_elem_t PushDefaultValues(const SField &sfield) {
        auto dv = sfield.defaultval;
        if (!dv) {
            return (type_elem_t)0;
        }
        // TODO: support more types of default values!
        Value val = NoVal();
        auto cons = Is<ObjectConstructor>(dv);
        if (cons) {
            vector<pair<ValueType, Value>> vals;
            for (auto n : cons->children) {
                auto vt = n->ConstVal(nullptr, val);
                if (vt == V_INT || vt == V_FLOAT) {
                    vals.push_back({ vt, val });
                } else {
                    return (type_elem_t)0;
                }
            }
            vector<type_elem_t> idxs;
            for (auto [vt, aval] : vals) {
                idxs.push_back(PushDefaultValue(vt, aval));
            }
            auto &it = default_aggregate_lookup[idxs];
            if (!it) {
                it = (type_elem_t)type_table.size();
                type_table.insert(type_table.end(), idxs.begin(), idxs.end());
            }
            return it;
        }
        auto vt = dv->ConstVal(nullptr, val);
        return PushDefaultValue(vt, val);
    }

    void PushFields(UDT *udt, small_vector<type_elem_t, 2> &tt,
                    type_elem_t parent = (type_elem_t)-1,
                    type_elem_t dvs_overrides = (type_elem_t)0) {
        for (auto [i, sfield] : enumerate(udt->sfields)) {
            auto ti = GetTypeTableOffset(sfield.type);
            auto dvs = PushDefaultValues(sfield);
            if (IsStruct(sfield.type->t)) {
                // FIXME: in this case, we don't actually need to store the "dvs" list since it's used inline.
                PushFields(sfield.type->udt, tt, parent < 0 ? ti : parent, dvs);
            } else {
                tt.push_back(ti);
                tt.push_back(parent);
                tt.push_back(dvs_overrides ? type_table[dvs_overrides + i] : dvs);
            }
        }
    }

    // Make a table for use as VM runtime type.
    type_elem_t GetTypeTableOffset(TypeRef type) {
        small_vector<type_elem_t, 2> tt;
        tt.push_back((type_elem_t)type->t);
        switch (type->t) {
            case V_INT:
                tt.push_back((type_elem_t)(type->e ? type->e->idx : -1));
                break;
            case V_NIL:
            case V_VECTOR:
                tt.push_back(GetTypeTableOffset(type->sub));
                break;
            case V_FUNCTION:
                tt.push_back((type_elem_t)type->sf->idx);
                break;
            case V_CLASS:
            case V_STRUCT_R:
            case V_STRUCT_S: {
                auto udt = type->udt;
                if (udt->typeinfo >= 0)
                    return udt->typeinfo;
                udt->typeinfo = (type_elem_t)type_table.size();
                // Reserve space, so other types can be added afterwards safely.
                assert(udt->numslots >= 0);
                auto ttsize = (udt->numslots * ti_num_udt_per_field) + ti_num_udt_fields;
                type_table.insert(type_table.end(), ttsize, (type_elem_t)0);
                tt.push_back((type_elem_t)udt->idx);
                tt.push_back((type_elem_t)udt->numslots);
                if (type->t == V_CLASS)
                    tt.push_back((type_elem_t)udt->vtable_start);
                else
                    tt.push_back((type_elem_t)ComputeBitMask(*udt));
                tt.push_back(!udt->ssuperclass
                    ? (type_elem_t)-1
                    : GetTypeTableOffset(&udt->ssuperclass->thistype));
                tt.push_back((type_elem_t)udt->serializable_id);
                PushFields(udt, tt);
                assert(ssize(tt) == ttsize);
                std::copy(tt.begin(), tt.end(), type_table.begin() + udt->typeinfo);
                return udt->typeinfo;
            }
            case V_VAR:
            case V_UNDEFINED:
                // This happens for values/types that are never accessed, common case are
                // [] or nil. It would be nice to ensure this is impossible, but it is too
                // fragile to have to ensure all vars allways get bound, given how types are
                // copied arbitrarily in the type checker.
                // Sadly that means if there are any bugs where this value is indeed used it
                // will only show up till runtime, but at least V_ANY will make it clear
                // what is happening, and a breakpoint can be placed here.
                return TYPE_ELEM_ANY;
            default:
                assert(IsRuntime(type->t));
                break;
        }
        // For everything that's not a struct:
        auto it = type_lookup.find(tt);
        if (it != type_lookup.end()) return it->second;
        auto offset = (type_elem_t)type_table.size();
        type_lookup[tt] = offset;
        type_table.insert(type_table.end(), tt.begin(), tt.end());
        return offset;
    }

    CodeGen(Parser &_p, SymbolTable &_st, bool return_value, int runtime_checks, bool cpp,
            uint64_t src_hash, string &c_codegen, string_view custom_pre_init_name)
        : parser(_p), st(_st), runtime_checks(runtime_checks), cpp(cpp), c_codegen(c_codegen) {
        node_context.push_back(parser.root);
        // Reserve space and index for all vtables.
        for (auto udt : st.udttable) {
            udt->vtable_start = (int)vtables.size();
            vtables.insert(vtables.end(), udt->dispatch_table.size(), -1);
        }
        // Pre-load some types into the table, must correspond to order of type_elem_t enums.
                                                            GetTypeTableOffset(type_int);
                                                            GetTypeTableOffset(type_float);
                                                            GetTypeTableOffset(type_string);
                                                            GetTypeTableOffset(type_resource);
                                                            GetTypeTableOffset(type_any);
        Type type_valuebuf(V_VALUEBUF);                     GetTypeTableOffset(&type_valuebuf);
                                                            GetTypeTableOffset(type_vector_int);
                                                            GetTypeTableOffset(type_vector_float);
        Type type_vec_str(V_VECTOR, &*type_string);         GetTypeTableOffset(&type_vec_str);
        Type type_v_v_int(V_VECTOR, &*type_vector_int);     GetTypeTableOffset(&type_v_v_int);
        Type type_v_v_float(V_VECTOR, &*type_vector_float); GetTypeTableOffset(&type_v_v_float);
                                                            GetTypeTableOffset(type_vector_resource);
        assert(type_table.size() == TYPE_ELEM_FIXED_OFFSET_END);
        for (auto f : parser.st.functiontable) {
            if (!f->istype) {
                for (auto ov : f->overloads) for (auto sf = ov->sf; sf; sf = sf->next) {
                    if (sf->typechecked) {
                        // We only set this here, because any inlining of anonymous functions in
                        // the optimizers is likely to reduce the amount of vars for which this is
                        // true a great deal.
                        for (auto &fv : sf->freevars) fv.sid->used_as_freevar = true;
                    }
                }
            }
        }
        int sidx = 0;
        for (auto sid : st.specidents) {
            if (!sid->type.Null()) {  // Null ones are in unused functions.
                auto tti = GetTypeTableOffset(sid->type);
                assert(!IsStruct(sid->type->t) || sid->type->udt->numslots >= 0);
                sid->sidx = sidx;
                auto ns = ValWidth(sid->type);
                sidx += ns;
                for (int i = 0; i < ns; i++)
                    sids.push_back(metadata::SpecIdent(sid->id->idx, tti, sid->used_as_freevar, sid->idx));
            }
        }
        auto max_ser_ids = parser.serializable_id_max + 1;
        ser_ids.resize(max_ser_ids, (type_elem_t)-1);
        for (auto udt : parser.st.udttable) {
            if (!udt->g.is_abstract) {
                // We generate a type table for every UDT regardless of whether it is referred to
                // anywhere, for example (sub)classes may be constructed by deserializing them and
                // not in code.
                udt->ComputeSizes();
                auto typeoff = GetTypeTableOffset(&udt->thistype);
                if (udt->serializable_id >= 0) {
                    assert(ser_ids[udt->serializable_id] < 0);
                    ser_ids[udt->serializable_id] = typeoff;
                }
            }
        }

        Prologue(c_codegen);

        // Start of the actual bytecode.
        linenumbernodes.push_back(parser.root);
        // Generate a dummmy function for function values that are never called.
        // Would be good if the optimizer guarantees these don't exist, but for now this is
        // more debuggable if it does happen to get called.
        f_function_idx = CODEGEN_SPECIAL_FUNCTION_ID_DUMMY;
        f_regs_max = 0;
        f_args.clear();
        f_defs.clear();
        f_keepvars = 0;
        EmitOp0(IL_ABORT);
        DefineFunction(c_codegen, false);

        // Generate all used functions.
        vector<SubFunction *> sf_used;
        for (auto f : parser.st.functiontable) {
            if (!f->istype) {
                for (auto ov : f->overloads) for (auto sf = ov->sf; sf; sf = sf->next) {
                    if (sf->typechecked) {
                        sf_used.push_back(sf);
                        DeclareFunction(*sf, c_codegen);
                    }
                }
            }
        }
        var_to_local.resize(sids.size(), -1);
        for (auto sf : sf_used) {
            GenScope(*sf);
            DefineFunction(c_codegen, true);
        }

        // Emit the root function.
        f_function_idx = CODEGEN_SPECIAL_FUNCTION_ID_ENTRY;
        f_regs_max = 1;
        f_args.clear();
        f_defs.clear();
        f_keepvars = 0;
        Gen(parser.root, return_value);
        auto type = parser.root->exptype;
        assert(type->NumValues() == (size_t)return_value);
        EmitOp1(IL_EXIT, return_value ? GetTypeTableOffset(type) : -1, int(return_value));
        linenumbernodes.pop_back();
        DefineFunction(c_codegen, false);

        // Now fill in the vtables.
        for (auto udt : st.udttable) {
            for (auto [i, de] : enumerate(udt->dispatch_table)) {
                if (de->sf) {
                    vtables[udt->vtable_start + i] = de->sf->idx;
                    assert(!de->is_switch_dispatch);
                } else if (de->case_index >= 0) {
                    vtables[udt->vtable_start + i] = -de->case_index - 2;
                    assert(de->is_switch_dispatch);
                }
            }
        }

        Epilogue(c_codegen, custom_pre_init_name, src_hash);
    }

    ~CodeGen() {
    }

    // FIXME: remove.
    void Dummy(size_t retval) {
        assert(!retval);
        while (retval--) EmitOp0(IL_PUSHNIL);
    }

    void GenStatDebug(const Node *c) {
        if (runtime_checks >= RUNTIME_STACK_TRACE) {
            if (cpp) {
                append(cb, "    vm.last = {", c->line.line, ", ", c->line.fileidx, "};\n");
            } else {
                append(cb, "    vm->last_line = ", c->line.line, ";\n");
                append(cb, "    vm->last_fileidx = ", c->line.fileidx, ";\n");
            }
        }
    }

    void GenScope(SubFunction &sf) {
        cursf = &sf;
        tstack_max = 0;
        if (!sf.typechecked) {
            auto s = DumpNode(*sf.sbody, 0, false);
            LOG_DEBUG("untypechecked: ", sf.parent->name, " : ", s);
            assert(0);
        }

        f_function_idx = sf.idx;
        f_regs_max = 0;  // Not valid until end of codegen of this function.
        f_keepvars = 0;  // Not valid until end of codegen of this function.

        linenumbernodes.push_back(sf.sbody);
        auto ret = AssertIs<Return>(sf.sbody->children.back());
        auto ir = sf.consumes_vars_on_return ? AssertIs<IdentRef>(ret->child) : nullptr;

        #ifndef NDEBUG
            var_to_local.clear();
            var_to_local.resize(sids.size(), -1);
        #endif
        auto emitvars = [&](const vector<Arg> &v, vector<int> &f_ad) {
            f_ad.clear();
            for (auto &arg : v) {
                auto n = ValWidth(arg.sid->type);
                for (int i = 0; i < n; i++) {
                    auto varidx = arg.sid->Idx() + i;
                    f_ad.push_back(varidx);
                    if (ShouldDec(IsStruct(arg.sid->type->t)
                                      ? TypeLT { FindSlot(*arg.sid->type->udt, i)->type,
                                                 arg.sid->lt }
                                      : TypeLT { *arg.sid }) && (!ir || arg.sid != ir->sid)) {
                        ownedvars.push_back(arg.sid->Idx() + i);
                    }
                    if (!sids[varidx].used_as_freevar()) {
                        var_to_local[varidx] = numlocals++;
                    }
                }
            }
        };
        emitvars(sf.args, f_args);
        emitvars(sf.locals, f_defs);

        auto profile = sf.attributes.find("profile");
        if (profile != sf.attributes.end() && LOBSTER_FRAME_PROFILER) {
            auto str = string(profile->second);
            if (str.empty()) {
                str = sf.parent->name;
                if (!sf.args.empty() && !sf.parent->overloads.empty()) {
                    append(str, "(", TypeName(sf.args[0].spec_type), (sf.args.size() > 1 ? ", .." : ""), ")");
                }
            }
            stringtable.push_back(st.StoreName(str));
            EmitPROFILE((int)stringtable.size() - 1);
        }

        if (sf.sbody) for (auto c : sf.sbody->children) {
            GenStatDebug(c);
            Gen(c, 0);
            assert(tstack.empty());
        }
        else Dummy(sf.reqret);

        assert(temptypestack.empty());
        assert(breaks.empty());
        assert(tstack.empty());
        f_regs_max = (int)tstack_max;
        linenumbernodes.pop_back();
        cursf = nullptr;
    }

    void Prologue(string &sd) {
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
                // This needs to correspond to the C++ RefObj, enforced in Entry().
                "typedef struct {\n"
                "    int typeinfo;\n"
                "    int refc;\n"
                "} RefObj;\n"
                // This needs to correspond to the C++ Value, enforced in Entry().
                "typedef struct {\n"
                "    union {\n"
                "        long long ival;\n"
                "        double fval;\n"
                "        RefObj *ref;\n"
                "    };\n"
                #if RTT_ENABLED
                "    int type;\n"
                #endif
                "} Value;\n"
                // This needs to correspond to the C++ VMBase, enforced in Entry().
                "typedef struct {\n"
                "    int last_line;\n"
                "    int last_fileidx;\n"
                "    int ret_unwind_to;\n"
                "    int ret_slots;\n"
                "    Value *temp_lval;\n"
                "} VMBase;\n"
                "typedef Value *StackPtr;\n"
                "typedef VMBase *VMRef;\n"
                "typedef void(*fun_base_t)(VMRef, StackPtr);\n"
                "#define Pop(sp) (*--(sp))\n"
                "#define Push(sp, V) (*(sp)++ = (V))\n"
                "#define TopM(sp, N) (*((sp) - (N) - 1))\n"
                // If you don't explicitly copy, libtcc will generate memcpy call for single 64-bit values :(
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
                  "extern void Entry(int, int, int);\n"
                  "extern void GLFrame(StackPtr, VMRef);\n"
                  "extern void SwapVars(VMRef, int, StackPtr, int);\n"
                  "extern void BackupVar(VMRef, int);\n"
                  "extern void DecOwned(VMRef, int);\n"
                  "extern void DecVal(VMRef, Value);\n"
                  "extern void RestoreBackup(VMRef, int);\n"
                  "extern StackPtr PopArg(VMRef, int, StackPtr);\n"
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

    void DeclareFunction(SubFunction &sf, string &sd) {
        append(sd, "static void fun_", sf.idx, "(VMRef, StackPtr);\n");
    }

    string IdName(int i, bool is_whole_struct) {
        auto ididx = sids[i].ididx();
        auto idx = sids[i].idx();
        auto &basename = st.identtable[ididx]->name;
        auto ti = (TypeInfo *)(&type_table[sids[i].typeidx()]);
        if (is_whole_struct || !IsStruct(ti->t)) {
            return basename;
        } else {
            int j = i;
            while (j && sids[j - 1].idx() == idx) j--;
            return cat(basename, "+", i - j);
        }
    };

    string sp(int off = 0) { return cat("regs + ", regso - off); };
    string spslot(int off) { return cat("regs[", regso - off, "]"); };
    void comment(string_view c) { append(cb, " // ", c, "\n"); };
    string_view vmref() { return string_view(cpp ? "vm." : "vm->"); };
    string_view ref() { return string_view(cpp ? "ref()" : "ref"); };
    string_view refnil() { return string_view(cpp ? "refnil()" : "ref"); };
    string_view lnamespace() { return string_view(cpp ? "lobster::" : ""); };
    string_view refobj() { return string_view(cpp ? "auto " : "RefObj *"); };

    int Label() { return nlabel++; }

    void EmitLabelDef(int lab) {
        EmitOp(IL_LABEL);
        append(cb, "    block", lab, ":;\n");
    }

    void EmitLabelDefs(vector<int> &labs) {
        for (auto lab : labs) {
            EmitLabelDef(lab);
        }
        labs.clear();
    }

    int EmitLabelDefBackwards() {
        auto lab = Label();
        EmitLabelDef(lab);
        return lab;
    }

    void GenValueCopy(string &sd, string_view dest, string_view src, string_view lf = "\n") {
        if (cpp) {
            append(sd, "    *(", dest, ") = *(", src, ");", lf);
        } else {
            #if RTT_ENABLED
                append(sd, "    { StackPtr _d = ", dest, "; StackPtr _s = ", src, "; _d->ival = _s->ival; _d->type = _s->type; }", lf);
            #else
                append(sd, "    (", dest, ")->ival = (", src, ")->ival;", lf);
            #endif
        }
    }

    void GenPushVar(size_t retval, TypeRef type, int offset, bool used_as_freevar) {
        if (!retval) return;
        if (IsStruct(type->t)) {
            auto width = ValWidth(type);
            if (used_as_freevar) {
                EmitOp(IL_PUSHVARVF, 0, width);
                append(cb, "    U_PUSHVARVF(vm, ", sp(), ", ", offset, ", ", width, ");");
                comment(IdName(offset, false));
            } else {
                EmitOp(IL_PUSHVARVL, 0, width);
                for (int i = 0; i < width; i++) {
                    GenValueCopy(cb, sp(-i), cat("locals + ", var_to_local[offset + i]), "");
                    comment(cat(IdName(offset, true), ".", i));
                }
            }
        } else {
            if (used_as_freevar) {
                EmitOp(IL_PUSHVARF);
                append(cb, "    U_PUSHVARF(vm, ", sp(), ", ", offset, ");");
                comment(IdName(offset, false));
            } else {
                EmitOp(IL_PUSHVARL);
                GenValueCopy(cb, sp(0), cat("locals + ", var_to_local[offset]), "");
                comment(IdName(offset, false));
            }
        }
    }

    void EmitLVAL_VARL(int offset) {
        EmitOp(IL_LVAL_VARL);
        append(cb, "    ", vmref(), "temp_lval = locals + ", var_to_local[offset], ";");
        comment(IdName(offset, false));
    }

    void EmitLVAL_VARF(int offset) {
        EmitOp(IL_LVAL_VARF);
        append(cb, "    U_LVAL_VARF(vm, ", sp(), ", ", offset, ");");
        comment(IdName(offset, false));
    }

    void EmitPUSHSTR(int stringtableindex) {
        EmitOp(IL_PUSHSTR);
        append(cb, "    U_PUSHSTR(vm, ", sp(), ", ", stringtableindex, ");");
        auto sv = stringtable[stringtableindex];
        sv = sv.substr(0, 50);
        string q;
        EscapeAndQuote(sv, q, true);
        comment(q);
    }

    int EmitJUMP() {
        EmitOp(IL_JUMP);
        auto lab = Label();
        append(cb, "    goto block", lab, ";\n");
        return lab;
    }

    int EmitJUMPback(int lab) {
        EmitOp(IL_JUMP);
        append(cb, "    goto block", lab, ";\n");
        return lab;
    }

    int EmitOpJump1(ILOP op) {
        EmitOp(op);
        auto lab = Label();
        append(cb, "    if (");
        auto test = cpp ? "True()" : "ival";
        switch (op) {
            case IL_JUMPFAIL:
            case IL_JUMPFAILR:
                append(cb, "!(" + sp(1) + ")->", test);
                break;
            case IL_JUMPNOFAIL:
            case IL_JUMPNOFAILR:
                append(cb, "(" + sp(1) + ")->", test);
                break;
            default:
                append(cb, "!U_", ILNames()[opc], "(vm, ", sp(), ")");
                break;
        }
        append(cb, ") goto block", lab, ";\n");
        return lab;
    }

    int EmitOpJump2(ILOP op, int a1) {
        EmitOp(op);
        auto lab = Label();
        append(cb, "    if (");
        switch (op) {
            case IL_JUMPIFUNWOUND:
                append(cb, vmref(), "ret_unwind_to == ", a1);
                break;
            default:
                append(cb, "!U_", ILNames()[opc], "(vm, ", sp(), ", ", a1, ")");
                break;
        }
        append(cb, ") goto block", lab, ";\n");
        return lab;
    }

    void EmitBCALLRET(ILOP op, NativeFun *nf, int has_ret, int useslots, int defslots) {
        EmitOp(op, useslots, defslots);
        if (nf->IsGLFrame()) {
            append(cb, "    GLFrame(", sp(), ", vm);\n");
        } else {
            append(cb, "    U_", ILNames()[opc], "(vm, ", sp(), ", ", nf->idx, ", ", has_ret, ");");
            comment(nf->name);
        }
    }

    void EmitKeep(int stack_offset, int keep_index_add) {
        auto op = !loops.empty() ? IL_KEEPREFLOOP : IL_KEEPREF;
        EmitOp(op);
        auto offset = f_keepvars++ + keep_index_add;
        if (opc == IL_KEEPREFLOOP) append(cb, "    DecVal(vm, keepvar[", offset, "]);\n");
        GenValueCopy(cb, cat("keepvar + ", offset), sp(stack_offset + 1));

    }

    void EmitRETURN(ILOP op, int nretslots, int parent_idx = -1, int useslots = ILUNKNOWN, int defslots = ILUNKNOWN) {
        EmitOp(op, useslots, defslots);
        // FIXME: emit epilogue stuff only once at end of function.
        if (opc == IL_RETURNLOCAL) {
            #if VM_EXTRA_CHECKING
                append(cb, "    ", vmref(), "ret_slots = -9;\n");
                append(cb, "    ", vmref(), "ret_unwind_to = -9;\n");
            #endif
        } else if (opc == IL_RETURNNONLOCAL) {
            append(cb, "    ", vmref(), "ret_slots = ", nretslots, ";\n");
            append(cb, "    ", vmref(), "ret_unwind_to = ", parent_idx, ";\n");
        }
        for (auto varidx : ownedvars) {
            if (sids[varidx].used_as_freevar()) {
                append(cb, "    DecOwned(vm, ", varidx, ");\n");
            } else {
                append(cb, "    DecVal(vm, locals[", var_to_local[varidx], "]);\n");
            }
        }
        auto nargs = (int)f_args.size();
        auto freevars = f_args.data() + nargs;
        while (nargs--) {
            auto varidx = *--freevars;
            if (sids[varidx].used_as_freevar()) {
                append(cb, "    psp = PopArg(vm, ", varidx, ", psp);\n");
            } else {
                // TODO: move to when we obtain the arg?
                append(cb, "    Pop(psp);\n");
            }
        }
        if (opc == IL_RETURNANY) {
            append(cb, "    { int rs = RetSlots(vm); for (int i = 0; i < rs; i++) "
                       "Push(psp, regs[i + ", regso - nretslots, "]); }\n");
        } else {
            for (int i = 0; i < nretslots; i++) {
                GenValueCopy(cb, "psp++", sp(nretslots - i));
            }
        }
        sdt.clear();  // FIXME: remove
        for (int i = (int)f_defs.size() - 1; i >= 0; i--) {
            auto varidx = f_defs[i];
            if (sids[varidx].used_as_freevar()) {
                append(sdt, "    RestoreBackup(vm, ", varidx, ");\n");
            }
        }
        if (opc == IL_RETURNANY) {
            // The above has taken care of falling thru retvals, but the normal retvals are
            // still on the tstack.
            for (int i = 0; i < nretslots; i++)
                PopTemp();
            for (auto &tse : reverse(temptypestack)) {
                GenPop(tse);
            }
        }
        append(cb, "    goto epilogue;\n");
    }

    void EmitPUSHFUN(int fidx) {
        EmitOp(IL_PUSHFUN);
        append(cb, "    U_PUSHFUN(vm, ", sp(), ", 0, ", "fun_", fidx, ");\n");
    }

    void EmitCALL(int fidx, int uses, int defs) {
        EmitOp(IL_CALL, uses, defs);
        append(cb, "    fun_", fidx, "(vm, ", sp(), ");");
        comment("call: " + st.subfunctiontable[fidx]->parent->name);
    }

    void EmitCALLV(int uses, int defs) {
        EmitOp(IL_CALLV, uses, defs);
        append(cb, "    U_CALLV(vm, ", sp(), "); ");
        if (cpp) append(cb, "vm.next_call_target(vm, ", sp(1), ");\n");
        else append(cb, "GetNextCallTarget(vm)(vm, ", sp(1), ");\n");
    }

    void EmitDDCALL(int vtable_idx, int nargs, int uses, int defs) {
        EmitOp(IL_DDCALL, uses, defs);
        append(cb, "    U_DDCALL(vm, ", sp(), ", ", vtable_idx, ", ", nargs, "); ");
        if (cpp) append(cb, "vm.next_call_target(vm, ", sp(), ");\n");
        else append(cb, "GetNextCallTarget(vm)(vm, ", sp(), ");\n");
    }

    void EmitPROFILE(int stringtable_idx) {
        EmitOp(IL_PROFILE);
        string name;
        EscapeAndQuote(stringtable[stringtable_idx], name, true);
        append(cb, "    static struct ___tracy_source_location_data tsld = { ", name, ", ", name,
               ", \"\", 0, 0x888800 }; struct ___tracy_c_zone_context ctx = ",
               cpp ? "lobster::" : "", "StartProfile(&tsld);\n");
        has_profile = true;
    }

    void EmitISTYPE(int type_idx) {
        EmitOp(IL_ISTYPE);
        append(cb, "    U_ISTYPE(vm, ", sp(), ", ", type_idx, ");");
        auto ti = ((TypeInfo *)(&type_table[type_idx]));
        if (IsUDT(ti->t)) comment(st.udttable[ti->structidx]->name);
        else cb += "\n";
    }

    void EmitNEWOBJECT(int type_idx, int uses) {
        EmitOp(IL_NEWOBJECT, uses);
        append(cb, "    U_NEWOBJECT(vm, ", sp(), ", ", type_idx, ");");
        auto ti = ((TypeInfo *)(&type_table[type_idx]));
        if (IsUDT(ti->t)) comment(st.udttable[ti->structidx]->name);
        else cb += "\n";
    }

    void EmitST2S(int type_idx, int uses, int defs) {
        EmitOp(IL_ST2S, uses, defs);
        append(cb, "    U_ST2S(vm, ", sp(), ", ", type_idx, ");");
        auto ti = ((TypeInfo *)(&type_table[type_idx]));
        if (IsUDT(ti->t)) comment(st.udttable[ti->structidx]->name);
        else cb += "\n";
    }

    string SetType(ValueType t) {
        #if RTT_ENABLED
            return cat(" _sp->type = ", (int)t, ";");
        #else
            (void)t;
            return {};
        #endif
    }

    void EmitPUSHINT(int val) {
        EmitOp(IL_PUSHINT);
        if (cpp) {
            append(cb, "    *(", sp(), ") = Value(", val, ");\n");
        } else {
            append(cb, "    { StackPtr _sp = ", sp(), "; _sp->ival = ", val, ";", SetType(V_INT), " }\n");
        }
    }

    void GenFloat(double f) {
        if ((float)f == f && isfinite(f)) {
            EmitOp(IL_PUSHFLT);
            // We're printint the float as text which seems dangerous, but PUSHFLT is only used in cases where
            // double and float are identical, meaning typically whole numbers and other precisely representable
            // numbers.
            if (cpp) {
                append(cb, "    *(", sp(), ") = Value(", (float)f, ");\n");
            } else {
                append(cb, "    { StackPtr _sp = ", sp(), "; _sp->fval = ", (float)f, ";", SetType(V_FLOAT), " }\n");
            }
        } else {
            int2float64 i2f(f);
            EmitOp(IL_PUSHFLT64);
            append(cb, "    U_PUSHFLT64(vm, ", sp(), ", ", (int)i2f.i, ", ", (int)(i2f.i >> 32), ");");
            comment(to_string_float(f));
        }
    }

    void SetToNil(string &sd, string_view target) {
        if (cpp)
            append(sd, "    *(", target, ") = Value(0, lobster::V_NIL);\n");
        else
            append(sd, "    { StackPtr _sp = ", target, "; _sp->ival = 0;", SetType(V_NIL), " }\n");
    }

    void EmitINCREF(int off, TypeRef type) {
        EmitOp(IL_INCREF);
        // FIXME: even when the static type is IsRef (i.e. no NIL or scalar), at runtime it is
        // still possible we get passed an int false value due to the way and/or are compiled?
        // See e.g. astar_result in the test.
        // Would be great to remove this case since the if-check is not needed in almost all cases.
        auto could_be_nil = true || !IsRef(type->t);
        if (cpp) {
            if (could_be_nil) {
                append(cb, "    (", sp(off + 1), ")->LTINCRTNIL();\n");
            } else {
                append(cb, "    (", sp(off + 1), ")->LTINCRT();\n");
            }
        } else {
            if (could_be_nil) {
                append(cb, "    { RefObj *_r = (", sp(off + 1), ")->ref; if (_r) _r->refc++; }\n");
            } else {
                append(cb, "    (", sp(off + 1), ")->ref->refc++;\n");
            }
        }
    }

    void EmitOp0(ILOP op, int useslots = ILUNKNOWN, int defslots = ILUNKNOWN) {
        EmitOp(op, useslots, defslots);
        append(cb, "    U_", ILNames()[op], "(vm, ", sp(), ");\n");
    }

    void EmitOp1(ILOP op, int a1, int useslots = ILUNKNOWN, int defslots = ILUNKNOWN) {
        EmitOp(op, useslots, defslots);
        append(cb, "    U_", ILNames()[op], "(vm, ", sp(), ", ", a1, ");\n");
    }

    void EmitOp2(ILOP op, int a1, int a2, int useslots = ILUNKNOWN, int defslots = ILUNKNOWN) {
        EmitOp(op, useslots, defslots);
        append(cb, "    U_", ILNames()[op], "(vm, ", sp(), ", ", a1, ", ", a2, ");\n");
    }

    void EmitOp3(ILOP op, int a1, int a2, int a3, int useslots = ILUNKNOWN, int defslots = ILUNKNOWN) {
        EmitOp(op, useslots, defslots);
        append(cb, "    U_", ILNames()[op], "(vm, ", sp(), ", ", a1, ", ", a2, ", ", a3, ");\n");
    }

    void DefineFunction(string &sd, bool label) {
        sd += "\n";
        auto sf_idx = f_function_idx;
        if (sf_idx < CODEGEN_SPECIAL_FUNCTION_ID_START)
            append(sd, "// ", st.subfunctiontable[sf_idx]->parent->name, "\n");
        append(sd, "static void fun_", sf_idx, "(VMRef vm, StackPtr psp) {\n");
        // NOTE: f_keepvars and f_regs_max are not known until end of codegen of function!
        // FIXME: don't emit array.
        // (there may be functions that don't use regs yet still refer to sp?)
        append(sd, "    Value regs[", std::max(1, f_regs_max), "];\n");
        if (!f_regs_max) append(sd, "    (void)regs;\n");
        if (f_keepvars) append(sd, "    Value keepvar[", f_keepvars, "];\n");
        if (numlocals) append(sd, "    Value locals[", numlocals, "];\n");
        for (int i = 0; i < (int)f_args.size(); i++) {
            auto varidx = f_args[i];
            if (sids[varidx].used_as_freevar()) {
                append(sd, "    SwapVars(vm, ", varidx, ", psp, ", (int)f_args.size() - i, ");\n");
            } else {
                GenValueCopy(sd, cat("locals + ", var_to_local[varidx]),
                                 cat("psp - ", (int)f_args.size() - i));
            }
        }
        for (int i = 0; i < (int)f_defs.size(); i++) {
            // for most locals, this just saves an nil, only in recursive cases it has an
            // actual value.
            auto varidx = f_defs[i];
            if (sids[varidx].used_as_freevar()) {
                append(sd, "    BackupVar(vm, ", varidx, ");\n");
            } else {
                // FIXME: it should even be unnecessary to initialize them, but its possible
                // there is a return before they're fully initialized, and then the decr of
                // owned vars may cause these to be accessed.
                SetToNil(sd, cat("locals + ", var_to_local[varidx]));
            }
        }
        if (runtime_checks >= RUNTIME_STACK_TRACE && sf_idx < CODEGEN_SPECIAL_FUNCTION_ID_START) {
            // FIXME: can make this just and index and instead store funinfo_table ref in
            // VM. Calling this here because now locals have been fully initialized.
            append(sd, "    PushFunId(vm, funinfo_table + ", funstarttables.size(), ", ",
                   numlocals ? "locals" : "0", ");\n");
            // This can be any format we want, see VM::DumpStackFrame
            funstarttables.push_back(f_function_idx);
            funstarttables.push_back((int)f_args.size());
            funstarttables.insert(funstarttables.end(), f_args.begin(), f_args.end());
            funstarttables.push_back((int)f_defs.size());
            funstarttables.insert(funstarttables.end(), f_defs.begin(), f_defs.end());
        }
        for (int i = 0; i < f_keepvars; i++) {
            SetToNil(sd, cat("keepvar + ", i));
        }

        sd += cb;
        cb.clear();


        if (label) sd += "    epilogue:;\n";
        if (has_profile) {
            append(sd, "    ", cpp ? "lobster::" : "", "EndProfile(ctx);\n");
        }
        if (!sdt.empty()) append(sd, sdt);
        sdt.clear();
        for (int i = 0; i < f_keepvars; i++) {
            append(sd, "    DecVal(vm, keepvar[", i, "]);\n");
        }
        if (runtime_checks >= RUNTIME_STACK_TRACE && f_function_idx < CODEGEN_SPECIAL_FUNCTION_ID_START) {
            append(sd, "    PopFunId(vm);\n");
        }
        sd += "}\n";
        ownedvars.clear();
        f_keepvars = -1;
        numlocals = 0;
        nlabel = 0;
        has_profile = false;
        opc = IL_UNUSED;
    }

    void Epilogue(string &sd, string_view custom_pre_init_name, uint64_t src_hash) {
        if (cpp) sd += "\nstatic";
        else sd += "\nextern";
        sd += " const fun_base_t vtables[] = {\n";
        for (auto id : vtables) {
            sd += "    ";
            if (id >= 0) {
                append(sd, "fun_", id);
            } else if (id <= -2) {
                append(sd, "(fun_base_t)", -id - 2);  // Bit of a hack, would be nice to separate.
            } else {
                sd += "0";
            }
            sd += ",\n";
        }
        sd += "    0\n};\n\n";  // Make sure table is never empty.

        if (runtime_checks >= RUNTIME_STACK_TRACE) {
            append(sd, "const int funinfo_table[] = {\n    ");
            for (auto [i, d] : enumerate(funstarttables)) {
                if (i && (i & 15) == 0) append(sd, "\n    ");
                append(sd, d, ", ");
            }
            append(sd, "    0\n};\n\n");
        }

        // Output the metadata.
        auto gen_string = [&](string_view s) {
            sd += "string_view(";
            EscapeAndQuote(s, sd, true);
            append(sd, ", ", s.size(), ")");
        };
        if (cpp) {
            sd += "static const int type_table[] = {";
            for (auto [i, x] : enumerate(type_table)) {
                if ((i & 0xF) == 0) sd += "\n ";
                append(sd, " ", x, ",");
            }
            sd += "\n};\n\n";
            sd += "static const string_view stringtable[] = {\n";
            for (auto s : stringtable) {
                sd += "    ";
                gen_string(s);
                sd += ",\n";
            }
            sd += "};\n\n";
            sd += "static const string_view file_names[] = {\n";
            for (auto [s, _] : parser.lex.filenames) {
                sd += "    ";
                gen_string(s);
                sd += ",\n";
            }
            sd += "};\n\n";
            sd += "static const string_view function_names[] = {\n";
            for (auto f : st.functiontable) {
                sd += "    ";
                gen_string(f->name);
                sd += ",\n";
            }
            sd += "};\n\n";
            auto fieldsname = [](UDT *udt) {
                auto n = cat(udt->name, "_fields", udt->idx);
                std::replace(n.begin(), n.end(), '.', '_');
                return n;
            };
            for (auto udt : st.udttable) {
                if (udt->sfields.empty()) continue;
                append(sd, "static const lobster::VMField ", fieldsname(udt), "[] = {\n");
                for (auto [i, sfield] : enumerate(udt->sfields)) {
                    sd += "    { ";
                    gen_string(udt->g.fields[i].id->name);
                    append(sd, ", ", sfield.slot, " },\n");
                }
                sd += "};\n\n";
            }
            sd += "static const lobster::VMUDT udts[] = {\n";
            for (auto udt : st.udttable) {
                sd += "    { ";
                gen_string(udt->name);
                auto fspan = udt->sfields.empty() ? "{}" : cat("span(", fieldsname(udt), ")");
                append(sd, ", ", udt->idx, ", ", udt->numslots, ", ",
                           (udt->ssuperclass ? udt->ssuperclass->idx : -1), ", ", udt->typeinfo, ", ",
                           fspan, " },\n");
            }
            sd += "};\n\n";
            sd += "static const lobster::VMSpecIdent specidents[] = {\n";
            for (auto &sid : sids) {
                auto id = st.identtable[sid.ididx()];
                sd += "    { ";
                gen_string(id->name);
                append(sd, ", ", sid.idx(), ", ", sid.typeidx(), ", ", sid.used_as_freevar(), ", ",
                       id->constant, ", ", id->scopelevel == 1, " },\n");
            }
            sd += "};\n\n";
            auto enumvalsname = [](Enum *e) {
                auto n = cat(e->name, "_vals", e->idx);
                std::replace(n.begin(), n.end(), '.', '_');
                return n;
            };
            for (auto e : st.enumtable) {
                if (e->vals.empty()) continue;
                append(sd, "static const lobster::VMEnumVal ", enumvalsname(e), "[] = {\n");
                for (auto [i, ev] : enumerate(e->vals)) {
                    sd += "    { ";
                    gen_string(ev->name);
                    append(sd, ", ", ev->val, " },\n");
                }
                sd += "};\n\n";
            }
            sd += "static const lobster::VMEnum enums[] = {\n";
            for (auto e : st.enumtable) {
                sd += "    { ";
                gen_string(e->name);
                auto fspan = e->vals.empty() ? "{}" : cat("span(", enumvalsname(e), ")");
                append(sd, ", ", fspan, ", ", e->flags, " },\n");
            }
            sd += "};\n\n";
            sd += "static const int ser_ids[] = {";
            for (auto [i, x] : enumerate(ser_ids)) {
                if ((i & 0xF) == 0) sd += "\n ";
                append(sd, " ", x, ",");
            }
            sd += "\n};\n\n";
            sd += "static const int subfunctions_to_function[] = {";
            vector<int> subfunctions_to_function;
            for (auto [i, sf] : enumerate(st.subfunctiontable)) {
                if ((i & 0xF) == 0) sd += "\n ";
                append(sd, " ", sf->parent->idx, ",");
            }
            sd += "\n};\n\n";
        }
        if (cpp) sd += "extern \"C\" ";
        sd += "void compiled_entry_point(VMRef vm, StackPtr sp) {\n";
        if (cpp) {
            append(sd, "    if (vm.vma.nfr.HashAll() != ", parser.natreg.HashAll(),
                   "ULL) vm.BuiltinError(\"code compiled with mismatching builtin function library\");\n");
        } else {
            sd += "    Entry(sizeof(Value), sizeof(VMBase), sizeof(RefObj));\n";
        }
        append(sd, "    fun_", CODEGEN_SPECIAL_FUNCTION_ID_ENTRY, "(vm, sp);\n}\n\n");
        if (cpp) {
            string build_info;
            auto time = std::time(nullptr);
            if (time) {
                auto tm = std::localtime(&time);
                if (tm) {
                    auto ts = std::asctime(tm);
                    build_info = string(ts, 24);
                }
            }
            sd += "int main(int argc, char *argv[]) {\n";
            sd += "    // This is hard-coded to call compiled_entry_point()\n";
            if (custom_pre_init_name != "nullptr") append(sd, "    void ", custom_pre_init_name, "(lobster::NativeRegistry &);\n");
            sd += "    lobster::VMMetaData vmmeta = {\n";
            sd += "        " + to_string(LOBSTER_METADATA_FORMAT_VERSION) + ",\n";
            sd += "        span((const lobster::type_elem_t *)&type_table, sizeof(type_table) / sizeof(int)),\n";
            sd += "        span(stringtable),\n";
            sd += "        span(file_names),\n";
            sd += "        span(function_names),\n";
            sd += "        span(udts),\n";
            sd += "        span(specidents),\n";
            sd += "        span(enums),\n";
            sd += "        span(ser_ids),\n";
            sd += "        ";
            gen_string(build_info);
            sd += ",\n";
            sd += "        ";
            to_string_hex(sd, src_hash);
            sd += ",\n";
            sd += "        span(subfunctions_to_function),\n";
            sd += "    };\n";
            sd += "    return RunCompiledCodeMain(argc, argv, ";
            append(sd, "&vmmeta, vtables, ", custom_pre_init_name, ", \"",
                   (!cpp ? "main.lobster" : ""), "\");\n}\n");
        }
    }



    // This must be called explicitly when any values are consumed.
    void TakeTemp(size_t n, bool can_handle_structs) {
        for (; n; n--) {
            auto tlt = temptypestack.back();
            temptypestack.pop_back();
            assert(can_handle_structs || ValWidth(tlt.type) == 1); (void)tlt;
            (void)can_handle_structs;
        }
    }

    void GenUnwind(const SubFunction &sf, int nretslots_unwind_max, int nretslots_norm) {
        // We're in an odd position here, because what is on the stack can either be from
        // the function we're calling (if we're not falling thru) or from any function above it
        // with different number of return values (and there can be multiple such paths, with
        // different retvals, hence "max").
        // Then, below it, may be temps.
        // If we're falling thru, we actually want to 1) unwind, 2) copy rets, 3) pop temps
        // We manage the tstack as if we're not falling thru.
        // Need to ensure there's enough space for either path.
        for (int i = nretslots_norm; i < nretslots_unwind_max; i++)
            PushTemp(IL_CALL);
        auto lab = EmitOpJump2(IL_JUMPIFUNWOUND, sf.parent->idx);
        for (int i = nretslots_norm; i < nretslots_unwind_max; i++)
            PopTemp();
        // Here we are emitting code executed only if we're falling thru,
        // so temp modify the tstack to match that.
        auto tstackbackup = tstack;
        EmitRETURN(IL_RETURNANY, nretslots_norm);
        EmitLabelDef(lab);
        tstack = tstackbackup;
    }

    void GenCall(const Call &call, size_t retval) {
        auto &sf = *call.sf;
        auto &f = *sf.parent;
        int inw = 0;
        int outw = ValWidthMulti(sf.returntype, sf.returntype->NumValues());
        for (auto c : call.children) {
            Gen(c, 1);
            inw += ValWidth(c->exptype);
        }
        size_t nargs = call.children.size();
        if (f.nargs() != nargs)
            parser.ErrorAt(node_context.back(),
                           "call to function ", Q(f.name), " needs ", f.nargs(),
                           " arguments, ", nargs, " given");
        TakeTemp(nargs, true);
        if (call.vtable_idx < 0) {
            EmitCALL(sf.idx, inw, outw);
            if (sf.returned_thru_to_max >= 0) {
                GenUnwind(sf, sf.returned_thru_to_max, outw);
            }
        } else {
            EmitDDCALL(call.vtable_idx, inw - 1, inw, outw);
            // We get the dispatch from arg 0, since sf is an arbitrary overloads and
            // doesn't necessarily point to the dispatch root (which may not even have an sf).
            auto dispatch_type = call.children[0]->exptype;
            assert(IsUDT(dispatch_type->t));
            auto de = dispatch_type->udt->dispatch_table[call.vtable_idx].get();
            assert(de->dispatch_root && !de->returntype.Null() && de->subudts_size);
            if (de->returned_thru_to_max >= 0) {
                // This works because all overloads of a DD sit under a single Function.
                GenUnwind(sf, de->returned_thru_to_max, outw);
            }
        }
        auto nretvals = sf.returntype->NumValues();
        for (size_t i = 0; i < nretvals; i++) {
            if (retval) {
                rettypes.push_back({ sf, i });
            } else {
                // FIXME: better if this is impossible by making sure typechecker makes it !reqret.
                GenPop({ sf, i });
            }
        }
        for (size_t i = nretvals; i < retval; i++) {
            // This can happen in a function that ends in a non-local return (thus nretvals==0)
            // but retval>0 because it is inside an if-then-else branch.
            // FIXME: take care of this in Gen() instead? Are there other nodes for which this
            // can happen?
            PushTemp(IL_EXIT);
        }
    };

    bool ShouldDec(TypeLT typelt) {
        return IsRefNil(typelt.type->t) && typelt.lt == LT_KEEP;
    }

    void GenPop(TypeLT typelt) {
        if (IsStruct(typelt.type->t)) {
            if (typelt.type->t == V_STRUCT_R) {
                // TODO: alternatively emit a single op with a list or bitmask? see BitMaskForRefStuct
                for (int j = typelt.type->udt->numslots - 1; j >= 0; j--) {
                    EmitOp0(IsRefNil(FindSlot(*typelt.type->udt, j)->type->t) ? IL_POPREF
                                                                             : IL_POP);
                }
            } else {
                EmitOp1(IL_POPV, typelt.type->udt->numslots, typelt.type->udt->numslots, 0);
            }
        } else {
            EmitOp0(ShouldDec(typelt) ? IL_POPREF : IL_POP);
        }
    }

    void GenDup(TypeLT tlt) {
        EmitOp0(IL_DUP);
        temptypestack.push_back(tlt);
    }

    void Gen(const Node *n, size_t retval) {
        // Generate() below generate no retvals if retval==0, otherwise they generate however many
        // they can irrespective of retval, optionally record that in rettypes for the more complex
        // cases. Then at the end of this function the two get matched up.
        auto tempstartsize = temptypestack.size();
        linenumbernodes.push_back(n);

        node_context.push_back(n);
        n->Generate(*this, retval);
        node_context.pop_back();

        assert(n->exptype->t != V_UNDEFINED);

        assert(tempstartsize == temptypestack.size());
        (void)tempstartsize;
        // If 0, the above code already made sure to not generate value(s).
        if (retval) {
            // default case, no rettypes specified.
            if (rettypes.empty()) {
                for (size_t i = 0; i < n->exptype->NumValues(); i++)
                    rettypes.push_back(TypeLT { *n, i });
            }
            // if the caller doesn't want all return values, just pop em
            if (rettypes.size() > retval) {
                while (rettypes.size() > retval) {
                    GenPop(rettypes.back());
                    rettypes.pop_back();
                }
            }
            assert(rettypes.size() == retval);
            // Copy return types on temp stack.
            while (rettypes.size()) {
                temptypestack.push_back(rettypes.front());
                rettypes.erase(rettypes.begin());
            }
        }
        assert(rettypes.empty());
        linenumbernodes.pop_back();
    }

    int ComputeBitMask(const UDT &udt) {
        int bits = 0;
        for (int j = 0; j < udt.numslots; j++) {
            if (IsRefNil(FindSlot(udt, j)->type->t)) {
                if (j > 31)
                    parser.ErrorAt(node_context.back(),
                                   "internal error: struct with too many reference fields");
                bits |= 1 << j;
            }
        }
        return bits;
    }

    int BitMaskForRefStuct(TypeRef type) {
        assert(type->t == V_STRUCT_R);
        return ComputeBitMask(*type->udt);
    }

    void GenValueWidth(TypeRef type) {
        // FIXME: struct variable size.
        EmitPUSHINT( ValWidth(type));
    }

    void GenLvalModifierOpWithStructInfo(ILOP op, TypeRef type) {
        EmitOp(op, ValWidth(type));
        if (op == IL_LV_WRITE) {
            GenValueCopy(cb, cat(vmref(), "temp_lval"), sp(1));
        } else {
            append(cb, "    U_", ILNames()[opc], "(vm, ", sp());
            if (IsStruct(type->t)) append(cb, ", ", ValWidth(type));
            if (op == IL_LV_WRITEREFV) append(cb, ", ", BitMaskForRefStuct(type));
            cb += ");\n";
        }
    }

    void GenAssignBasic(const SpecIdent &sid) {
        TakeTemp(1, true);
        GenLvalVar(sid, 0);
        auto op = AssignBaseOp({ sid });
        GenLvalModifierOpWithStructInfo(op, sid.type);
    }

    void GenAssignLvalRec(const Node *lval, int offset, int take_temp, TypeRef type) {
        if (auto idr = Is<IdentRef>(lval)) {
            TakeTemp(take_temp, true);
            GenLvalVar(*idr->sid, offset);
        } else if (auto dot = Is<Dot>(lval)) {
            auto stype = dot->child->exptype;
            assert(IsUDT(stype->t));  // Ensured by typechecker.
            auto idx = stype->udt->g.Has(dot->fld);
            assert(idx >= 0);
            auto &sfield = stype->udt->sfields[idx];
            if (stype->t == V_CLASS) {
                Gen(dot->child, 1);
                TakeTemp(take_temp + 1, true);
                EmitOp1(IL_LVAL_FLD, sfield.slot + offset);
            } else {
                GenAssignLvalRec(dot->child, sfield.slot + offset, take_temp, type);
            }
        } else if (auto indexing = Is<Indexing>(lval)) {
            Gen(indexing->object, 1);
            Gen(indexing->index, 1);
            TakeTemp(take_temp + 2, true);
            switch (indexing->object->exptype->t) {
                case V_VECTOR:
                    if (indexing->index->exptype->t == V_INT) {
                        EmitOp1(IL_LVAL_IDXVI, offset, 2);
                    } else {
                        assert(IsStruct(indexing->index->exptype->t));
                        auto width = ValWidth(indexing->index->exptype);
                        EmitOp2(IL_LVAL_IDXVV, offset, width, width + 1);
                    }
                    break;
                case V_CLASS:
                    assert(indexing->index->exptype->t == V_INT &&
                           indexing->object->exptype->udt->sametype->Numeric());
                    EmitOp1(IL_LVAL_IDXNI, offset);
                    assert(!IsStruct(type->t));
                    break;
                case V_STRUCT_R:
                case V_STRUCT_S:
                case V_STRING:
                    // FIXME: Would be better to catch this in typechecking, but typechecker does
                    // not currently distinquish lvalues.
                    parser.ErrorAt(lval, "cannot use this type as lvalue");
                    [[fallthrough]];
                default:
                    assert(false);
            }
        } else {
            parser.ErrorAt(lval, "lvalue required");
        }
    }

    void GenAssign(const Node *lval, ILOP lvalop, size_t retval,
                   const Node *rhs, int take_temp, bool post) {
        assert(node_context.back()->exptype->NumValues() >= retval);
        auto type = lval->exptype;
        if (lvalop >= IL_LV_IADD && lvalop <= IL_LV_IMOD) {
            if (type->t == V_INT) {
            } else if (type->t == V_FLOAT) {
                lvalop = GENOP(lvalop + (IL_LV_FADD - IL_LV_IADD));
            } else if (type->t == V_STRING) {
                assert(lvalop == IL_LV_IADD); lvalop = IL_LV_SADD;
            } else if (type->t == V_STRUCT_S) {
                auto sub = type->udt->sametype;
                bool withscalar = IsScalar(rhs->exptype->t);
                if (sub->t == V_INT) {
                    lvalop = GENOP(lvalop + ((withscalar ? IL_LV_IVSADD : IL_LV_IVVADD) - IL_LV_IADD));
                } else if (sub->t == V_FLOAT) {
                    lvalop = GENOP(lvalop + ((withscalar ? IL_LV_FVSADD : IL_LV_FVVADD) - IL_LV_IADD));
                } else assert(false);
            } else {
                assert(false);
            }
        } else if (lvalop >= IL_LV_IPP && lvalop <= IL_LV_IMM) {
            if (type->t == V_FLOAT) lvalop = GENOP(lvalop + (IL_LV_FPP - IL_LV_IPP));
            else assert(type->t == V_INT);
        }
        if (rhs) Gen(rhs, 1);
        GenAssignLvalRec(lval, 0, take_temp, type);
        if (!post) {
            GenLvalModifierOpWithStructInfo(lvalop, type);
        }
        if (retval) {
            // FIXME: it seems these never need a refcount increase because they're always
            // borrowed? Be good to assert that somehow.
            if (IsStruct(type->t)) {
                auto width = ValWidth(type);
                EmitOp1(IL_LV_DUPV, width, 0, width);
            } else {
                EmitOp0(IL_LV_DUP, 0, 1);
            }
        }
        if (post) {
            GenLvalModifierOpWithStructInfo(lvalop, type);
        }
    }

    void GenConcatOp(const BinOp *n, size_t retval) {
        // Exception to the code below, since we want to generate an efficient concatenation
        // of any number of strings.
        node_small_vector strs;
        strs.push_back(n->left);
        strs.push_back(n->right);
        for (;;) {
            auto c = strs[0];
            if (auto lt = Is<ToLifetime>(c)) {
                assert(lt->decref == 1 && lt->incref == 0);
                c = lt->child;
            }
            auto p = Is<Plus>(c);
            if (p && p->left->exptype->t == V_STRING && p->right->exptype->t == V_STRING) {
                strs.erase(0);
                strs.insert(0, p->right);
                strs.insert(0, p->left);
            } else {
                break;
            }
        }
        // TODO: we can even detect any ToString nodes here and generate an even more efficient
        // call that does I2S etc inline with even fewer allocations.
        for (auto s : strs) {
            Gen(s, retval);
            TakeTemp(retval, false);
        }
        if (!retval) return;
        if (strs.size() == 2) {
            // We still need this op for += and it's marginally more efficient, might as well use it.
            EmitOp0(IL_SADD);
        } else {
            EmitOp1(IL_SADDN, (int)strs.size(), (int) strs.size(), 1);
        }
    }

    void GenMathOp(const BinOp *n, size_t retval, int op) {
        Gen(n->left, retval);
        Gen(n->right, retval);
        if (retval) GenMathOp(n->left->exptype, n->right->exptype, n->exptype, op);
    }

    void GenMathOp(TypeRef ltype, TypeRef rtype, TypeRef ptype, int op) {
        TakeTemp(2, true);
        // Have to check right and left because comparison ops generate ints for node
        // overall.
        if (rtype->t == V_INT && ltype->t == V_INT) {
            EmitOp0(GENOP(IL_IADD + op));
        } else if (rtype->t == V_FLOAT && ltype->t == V_FLOAT) {
            EmitOp0(GENOP(IL_FADD + op));
        } else if (rtype->t == V_STRING && ltype->t == V_STRING) {
            // Nillable version handled below.
            EmitOp0(GENOP(IL_SADD + op));
        } else if ((rtype->t == V_FUNCTION && ltype->t == V_FUNCTION) ||
                   (rtype->t == V_TYPEID && ltype->t == V_TYPEID)) {
            assert(op == MOP_EQ || op == MOP_NE);
            EmitOp0(GENOP(IL_LEQ + (op - MOP_EQ)));
        } else {
            if (op >= MOP_EQ) {  // EQ/NEQ
                if (IsStruct(ltype->t)) {
                    auto width = ValWidth(ltype);
                    EmitOp1(GENOP(IL_STEQ + op - MOP_EQ), width, width * 2, 1);
                } else {
                    assert(IsRefNil(ltype->t) &&
                           IsRefNil(rtype->t));
                    if ((ltype->t == V_NIL && ltype->sub->t == V_STRING) ||
                        (rtype->t == V_NIL && rtype->sub->t == V_STRING)) {
                        EmitOp0(GENOP(IL_SNEQ + op - MOP_EQ));
                    } else {
                        EmitOp0(GENOP(IL_AEQ + op - MOP_EQ));
                    }
                }
            } else {
                bool leftisvec = ltype->t == V_STRUCT_S;
                // If this is a comparison op, be sure to use the child type.
                TypeRef vectype = op >= MOP_LT ? (leftisvec ? ltype : rtype) : ptype;
                assert(vectype->t == V_STRUCT_S);
                auto sub = vectype->udt->sametype;
                bool withscalar = IsScalar(rtype->t) || IsScalar(ltype->t);
                auto outw = ValWidth(ptype);
                auto inw = withscalar ? outw + 1 : outw * 2;
                auto width = ValWidth(vectype);
                if (sub->t == V_INT) 
                    EmitOp1(GENOP((withscalar ? (leftisvec ? IL_IVSADD : IL_SIVADD) : IL_IVVADD) + op), width, inw, outw);
                else if (sub->t == V_FLOAT)
                    EmitOp1(GENOP((withscalar ? (leftisvec ? IL_FVSADD : IL_SFVADD) : IL_FVVADD) + op), width, inw, outw);
                else assert(false);
            }
        }
    }

    void GenBitOp(const BinOp *n, size_t retval, ILOP op) {
        Gen(n->left, retval);
        Gen(n->right, retval);
        if (retval) {
            TakeTemp(2, false);
            EmitOp0(op);
        }
    }

    ILOP AssignBaseOp(TypeLT typelt) {
        auto dec = ShouldDec(typelt);
        return IsStruct(typelt.type->t)
            ? (dec ? IL_LV_WRITEREFV : IL_LV_WRITEV)
            : (dec ? IL_LV_WRITEREF : IL_LV_WRITE);
    }

    void GenLvalVar(const SpecIdent &sid, int offset) {
        if (sid.used_as_freevar)
            EmitLVAL_VARF(sid.Idx() + offset);
        else
            EmitLVAL_VARL(sid.Idx() + offset);
    }

    void GenPushField(size_t retval, Node *object, TypeRef stype, TypeRef ftype, int offset) {
        auto fwidth = ValWidth(ftype);
        auto swidth = ValWidth(stype);
        if (IsStruct(stype->t)) {
            // Attempt to not generate object at all, by reading the field inline.
            if (auto idr = Is<IdentRef>(object)) {
                GenPushVar(retval, ftype, idr->sid->Idx() + offset, idr->sid->used_as_freevar);
                return;
            } else if (auto dot = Is<Dot>(object)) {
                auto sstype = dot->child->exptype;
                assert(IsUDT(sstype->t));
                auto idx = sstype->udt->g.Has(dot->fld);
                assert(idx >= 0);
                auto &sfield = sstype->udt->sfields[idx];
                assert(sfield.slot >= 0);
                GenPushField(retval, dot->child, sstype, ftype, sfield.slot + offset);
                return;
            } else if (auto indexing = Is<Indexing>(object)) {
                // For now only do this for vectors.
                if (indexing->object->exptype->t == V_VECTOR) {
                    GenPushIndex(retval, indexing->object, indexing->index, fwidth, offset);
                    return;
                }
            }
        }
        Gen(object, retval);
        if (!retval) return;
        TakeTemp(1, true);
        if (IsStruct(stype->t)) {
            if (IsStruct(ftype->t)) {
                EmitOp3(IL_PUSHFLDV2V, offset, fwidth, swidth, swidth, fwidth);
            } else {
                EmitOp2(IL_PUSHFLDV, offset, swidth, swidth, 1);
            }
        } else {
            if (IsStruct(ftype->t)) {
                EmitOp2(IL_PUSHFLD2V, offset, fwidth, 1, fwidth);
            } else {
                EmitOp1(IL_PUSHFLD, offset);
            }
        }
    }

    void GenPushIndex(size_t retval, Node *object, Node *index, int struct_elem_sub_width = -1,
                      int struct_elem_sub_offset = -1) {
        Gen(object, retval);
        Gen(index, retval);
        if (!retval) return;
        TakeTemp(2, true);
        switch (object->exptype->t) {
            case V_VECTOR: {
                auto etype = object->exptype;
                if (index->exptype->t == V_INT) {
                    etype = etype->Element();
                } else {
                    auto &udt = *index->exptype->udt;
                    for (auto &sfield : udt.sfields) {
                        (void)sfield;
                        etype = etype->Element();
                    }
                }
                auto inw = ValWidth(index->exptype) + 1;
                auto elemwidth = ValWidth(etype);
                if (struct_elem_sub_width < 0) {
                    if (index->exptype->t == V_INT) {
                        EmitOp0(elemwidth == 1 ? IL_VPUSHIDXI : IL_VPUSHIDXI2V, inw, elemwidth);
                    } else {
                        assert(IsStruct(index->exptype->t));
                        EmitOp1(IL_VPUSHIDXV, ValWidth(index->exptype), inw, elemwidth);
                    }
                } else {
                    // We're indexing a sub-part of the element.
                    if (index->exptype->t == V_INT) {
                        if (elemwidth == 1) {
                            EmitOp1(IL_VPUSHIDXIS, struct_elem_sub_offset,
                                    inw, struct_elem_sub_width);
                        } else {
                            EmitOp2(IL_VPUSHIDXIS2V, struct_elem_sub_width, struct_elem_sub_offset,
                                    inw, struct_elem_sub_width);
                        }
                    } else {
                        assert(IsStruct(index->exptype->t));
                        EmitOp3(IL_VPUSHIDXVS, ValWidth(index->exptype), struct_elem_sub_width,
                                struct_elem_sub_offset, inw, struct_elem_sub_width);
                    }
                }
                break;
            }
            case V_STRUCT_S: {
                auto width = ValWidth(object->exptype);
                assert(index->exptype->t == V_INT && object->exptype->udt->sametype->Numeric());
                EmitOp1(IL_NPUSHIDXI, width, width + 1);
                break;
            }
            case V_STRING:
                assert(index->exptype->t == V_INT);
                EmitOp0(IL_SPUSHIDXI);
                break;
            default:
                assert(false);
        }
    }

    size_t LoopTemps() {
        size_t t = 0;
        for (auto n : loops) if (Is<For>(n)) t += 2;
        return t;
    }

    void ApplyBreaks(size_t level) {
        while (breaks.size() > level) {
            EmitLabelDef(breaks.back());
            breaks.pop_back();
        }
    }
};

void Nil::Generate(CodeGen &cg, size_t retval) const {
    if (retval) { cg.EmitOp0(IL_PUSHNIL); }
}

void IntConstant::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    if (integer == (int)integer) {
        cg.EmitPUSHINT((int)integer);
    } else {
        cg.EmitOp2(IL_PUSHINT64, (int)integer, (int)(integer >> 32));
    }
}

void FloatConstant::Generate(CodeGen &cg, size_t retval) const {
    if (retval) { cg.GenFloat(flt); };
}

void StringConstant::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    cg.stringtable.push_back(str);
    cg.EmitPUSHSTR((int)cg.stringtable.size() - 1);
}

void DefaultVal::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    cg.EmitOp0(IL_PUSHNIL);
}

void IdentRef::Generate(CodeGen &cg, size_t retval) const {
    cg.GenPushVar(retval, sid->type, sid->Idx(), sid->used_as_freevar);
}

void Dot::Generate(CodeGen &cg, size_t retval) const {
    auto stype = child->exptype;
    assert(IsUDT(stype->t));
    auto idx = stype->udt->g.Has(fld);
    assert(idx >= 0);
    auto &sfield = stype->udt->sfields[idx];
    assert(sfield.slot >= 0);
    cg.GenPushField(retval, child, stype, sfield.type, sfield.slot);
}

void Indexing::Generate(CodeGen &cg, size_t retval) const {
    cg.GenPushIndex(retval, object, index);
}

void GenericCall::Generate(CodeGen &, size_t /*retval*/) const {
    assert(false);
}

void Member::Generate(CodeGen &cg, size_t retval) const {
    if (frame) {
        cg.GenPushVar(1, this_sid->type, this_sid->Idx(), this_sid->used_as_freevar);
        auto &sfield = this_sid->type->udt->sfields[field_idx];
        auto lab = cg.EmitOpJump2(IL_JUMPIFMEMBERLF, sfield.slot + ValWidth(sfield.type));  // It's the var after this one.
        cg.Gen(child, 1);
        cg.GenPushVar(1, this_sid->type, this_sid->Idx(), this_sid->used_as_freevar);
        cg.TakeTemp(1, true);
        cg.EmitOp1(IL_LVAL_FLD, sfield.slot);
        cg.GenLvalModifierOpWithStructInfo(cg.AssignBaseOp({ sfield.type, LT_KEEP }), sfield.type);
        cg.EmitLabelDef(lab);
    }
    if (!retval) return;
    cg.EmitOp0(IL_PUSHNIL);
}

void Static::Generate(CodeGen &cg, size_t retval) const {
    if (frame) {
        assert(sid->used_as_freevar);  // Since we'll access these from the freevar buf.
        auto lab = cg.EmitOpJump2(IL_JUMPIFSTATICLF, sid->Idx() + ValWidth(sid->type));  // It's the var after this one.
        cg.Gen(child, 1);
        cg.GenAssignBasic(*sid);
        cg.EmitLabelDef(lab);
    }
    if (!retval) return;
    cg.EmitOp0(IL_PUSHNIL);
}

void AssignList::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(children.back(), children.size() - 1);
    for (size_t i = children.size() - 1; i-- > 0; ) {
        auto left = children[i];
        auto id = Is<IdentRef>(left);
        auto llt = id ? id->sid->lt : LT_KEEP /* Dot */;
        cg.GenAssign(left, cg.AssignBaseOp({ left->exptype, llt }), 0, nullptr, 1, false);
    }
    assert(!retval);  // Type checker guarantees this.
    (void)retval;
}

void Define::Generate(CodeGen &cg, size_t retval) const {
    if (Is<DefaultVal>(child)) {
        return;  // Pre-decl. 
    }
    cg.Gen(child, tsids.size());
    for (size_t i = tsids.size(); i-- > 0; ) {
        auto sid = tsids[i].sid;
        // FIXME: Sadly, even though FunIntro now guarantees that variables start as V_NIL,
        // we still can't replace this with a WRITE that doesn't have to decrement, since
        // loops with inlined bodies cause this def to be execute multiple times.
        // (also: multiple copies of the same inlined function in one parent).
        // We should emit a specialized opcode for these cases only.
        // NOTE: we already don't decref for borrowed vars generated by the optimizer here (!)
        cg.GenAssignBasic(*sid);
    }
    assert(!retval);  // Parser guarantees this.
    (void)retval;
}

void Assign::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, cg.AssignBaseOp({ *right, 0 }), retval, right, 1, false);
}

void PlusEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, IL_LV_IADD, retval, right, 1, false);
}
void MinusEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, IL_LV_ISUB, retval, right, 1, false);
}
void MultiplyEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, IL_LV_IMUL, retval, right, 1, false);
}
void DivideEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, IL_LV_IDIV, retval, right, 1, false);
}
void ModEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, IL_LV_IMOD, retval, right, 1, false);
}
void AndEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, IL_LV_BINAND, retval, right, 1, false);
}
void OrEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, IL_LV_BINOR, retval, right, 1, false);
}
void XorEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, IL_LV_XOR, retval, right, 1, false);
}
void ShiftLeftEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, IL_LV_ASL, retval, right, 1, false);
}
void ShiftRightEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, IL_LV_ASR, retval, right, 1, false);
}

void PostDecr::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, IL_LV_IMM, retval, nullptr, 0, true); }
void PostIncr::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, IL_LV_IPP, retval, nullptr, 0, true); }
void PreDecr ::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, IL_LV_IMM,  retval, nullptr, 0, false); }
void PreIncr ::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, IL_LV_IPP,  retval, nullptr, 0, false); }

void NotEqual     ::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_NE);  }
void Equal        ::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_EQ);  }
void GreaterThanEq::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_GE);  }
void LessThanEq   ::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_LE);  }
void GreaterThan  ::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_GT);  }
void LessThan     ::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_LT);  }
void Mod          ::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_MOD); }
void Divide       ::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_DIV); }
void Multiply     ::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_MUL); }
void Minus        ::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_SUB); }
void Plus         ::Generate(CodeGen &cg, size_t retval) const {
    if (left->exptype->t == V_STRING && right->exptype->t == V_STRING) {
        cg.GenConcatOp(this, retval);
    } else {
        cg.GenMathOp(this, retval, MOP_ADD);
    }
}

void UnaryMinus::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, true);
    auto ctype = child->exptype;
    switch (ctype->t) {
        case V_INT: cg.EmitOp0(IL_IUMINUS); break;
        case V_FLOAT: cg.EmitOp0(IL_FUMINUS); break;
        case V_STRUCT_S: {
            auto elem = ctype->udt->sametype->t;
            auto inw = ValWidth(ctype);
            cg.EmitOp1(elem == V_INT ? IL_IVUMINUS : IL_FVUMINUS, inw, inw, inw);
            break;
        }
        default: assert(false);
    }
}

void BitAnd    ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, IL_BINAND); }
void BitOr     ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, IL_BINOR); }
void Xor       ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, IL_XOR); }
void ShiftLeft ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, IL_ASL); }
void ShiftRight::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, IL_ASR); }

void Negate::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, false);
    cg.EmitOp0(IL_NEG);
}

void ToFloat::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, false);
    cg.EmitOp0(IL_I2F);
}

void ToString::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, true);
    switch (child->exptype->t) {
        case V_STRUCT_R:
        case V_STRUCT_S: {
            // TODO: can also roll these into A2S?
            cg.EmitST2S(cg.GetTypeTableOffset(child->exptype), ValWidth(child->exptype), 1);
            break;
        }
        default: {
            cg.EmitOp1(IL_A2S, cg.GetTypeTableOffset(child->exptype->ElementIfNil()));
            break;
        }
    }
}

void ToBool::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, false);
    cg.EmitOp0(cg.ShouldDec(TypeLT(*child, 0)) ? IL_E2BREF : IL_E2B);
}

void ToInt::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    // No actual opcode needed, this node is purely to store correct types.
    if (retval) cg.TakeTemp(1, false);
}

void ToStructSuper::Generate(CodeGen &cg, size_t retval) const {
    // TODO: rather than chopping off extra fields, can see if child is an IdentRef and make it
    // not push those fields in the first place.
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, true);
    for (auto i = child->exptype->udt->sfields.size() - 1; i >= exptype->udt->sfields.size(); i--) {
        auto &sfield = child->exptype->udt->sfields[i];
        cg.GenPop({ sfield.type, lt });
    }
}

void ToLifetime::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    int stack_offset = 0;
    for (int fi = 0; fi < (int)retval; fi++) {
        // We have to check for reftype again, since typechecker allowed V_VAR values that may
        // have become scalars by now.
        auto i = (int)(retval - fi - 1);
        assert(i < ssize(cg.temptypestack));
        auto type = cg.temptypestack[cg.temptypestack.size() - 1 - fi].type;
        if (IsRefNil(child->exptype->Get(i)->t)) {
            if (incref & (1LL << i)) {
                assert(IsRefNil(type->t));
                if (type->t == V_STRUCT_R) {
                    // TODO: alternatively emit a single op with a list or bitmask? see BitMaskForRefStuct
                    for (int j = 0; j < type->udt->numslots; j++) {
                        auto stype = FindSlot(*type->udt, j)->type;
                        if (IsRefNil(stype->t)) {
                            cg.EmitINCREF(stack_offset + type->udt->numslots - 1 - j, stype);
                        }
                    }
                } else {
                    cg.EmitINCREF(stack_offset, type);
                }
            }
            if (decref & (1LL << i)) {
                assert(IsRefNil(type->t));
                if (type->t == V_STRUCT_R) {
                    // TODO: alternatively emit a single op with a list or bitmask? see BitMaskForRefStuct
                    for (int j = 0; j < type->udt->numslots; j++) {
                        if (IsRefNil(FindSlot(*type->udt, j)->type->t))
                            cg.EmitKeep(stack_offset + (type->udt->numslots - j - 1), 0);
                    }
                } else {
                    cg.EmitKeep(stack_offset, 0);
                }
            }
        }
        stack_offset += ValWidth(type);
    }
    // We did not consume these, so we have to pass them on.
    for (size_t i = 0; i < retval; i++) {
        // Note: take LT from this node, not existing one on temptypestack, which we just changed!
        cg.rettypes.push_back(TypeLT(*this, i));
        cg.temptypestack.pop_back();
    }
}

void FunRef::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    // If no body, then the function has been optimized away, meaning this
    // function value will never be used.
    // FIXME: instead, ensure such values are removed by the optimizer.
    if (sf->parent->anonymous && sf->sbody && sf->typechecked) {
        cg.EmitPUSHFUN(sf->idx);
    } else {
        cg.EmitPUSHFUN(CODEGEN_SPECIAL_FUNCTION_ID_DUMMY);
    }
}

void EnumRef::Generate(CodeGen &cg, size_t retval) const {
    cg.Dummy(retval);
}

void GUDTRef::Generate(CodeGen &cg, size_t retval) const {
    cg.Dummy(retval);
}

void UDTRef::Generate(CodeGen &cg, size_t retval) const {
    cg.Dummy(retval);
}

void Assert::Generate(CodeGen &cg, size_t retval) const {
    if (retval || cg.runtime_checks >= RUNTIME_ASSERT) {
        cg.Gen(child, 1);
        cg.TakeTemp(1, false);
        if (cg.runtime_checks >= RUNTIME_ASSERT) {
            cg.EmitOp3(GENOP(IL_ASSERT + (!!retval)), child->line.line,
                       child->line.fileidx, (int)cg.stringtable.size());
            // FIXME: would be better to use the original source code here.
            cg.stringtable.push_back(cg.st.StoreName(DumpNode(*child, 0, true)));
        }
    } else {
        cg.Gen(child, 0);
    }
}

void NativeCall::Generate(CodeGen &cg, size_t retval) const {
    if (nf->name == "string") {
        // A frequently used function that doesn't actually do anything by itself, so ensure it
        // doesn't get emitted.
        cg.Gen(children[0], retval);
        if (retval) cg.TakeTemp(1, false);
        return;
    }
    // TODO: could pass arg types in here if most exps have types, cheaper than
    // doing it all in call instruction?
    size_t numstructs = 0;
    auto start = cg.tstack.size();
    for (auto [i, c] : enumerate(children)) {
        cg.Gen(c, 1);
        if ((IsStruct(c->exptype->t) ||
             nf->args[i].flags & NF_PUSHVALUEWIDTH) &&
            !Is<DefaultVal>(c)) {
            cg.GenValueWidth(c->exptype);
            cg.temptypestack.push_back({ type_int, LT_ANY });
            numstructs++;
        }
    }
    auto inw = int(cg.tstack.size() - start);
    size_t nargs = children.size();
    cg.TakeTemp(nargs + numstructs, true);
    assert(nargs == nf->args.size() && (nf->fun.fnargs < 0 || nargs <= 7));
    auto vmop = nf->fun.fnargs >= 0 ? GENOP(IL_BCALLRET0 + (int)nargs) : IL_BCALLRETV;
    cg.EmitBCALLRET(vmop, nf, !nf->retvals.empty(), inw,
                    ValWidthMulti(nattype, nattype->NumValues()));
    if (nf->retvals.size() > 0) {
        assert(nf->retvals.size() == nattype->NumValues());
        for (size_t i = 0; i < nattype->NumValues(); i++) {
            cg.rettypes.push_back({ nattype->Get(i), nattype->GetLifetime(i, natlt) });
        }
    } else {
        assert(nf->retvals.size() >= retval);
    }
    if (!retval) {
        while (cg.rettypes.size()) {
            cg.GenPop(cg.rettypes.back());
            cg.rettypes.pop_back();
        }
    }
}

void Call::Generate(CodeGen &cg, size_t retval) const {
    cg.GenCall(*this, retval);
}

void DynCall::Generate(CodeGen &cg, size_t retval) const {
    assert(sf && sf == sid->type->sf && sf->parent->istype);
    int arg_width = 0;
    for (auto c : children) {
        cg.Gen(c, 1);
        arg_width += ValWidth(c->exptype);
    }
    size_t nargs = children.size();
    assert(nargs == sf->args.size());
    cg.GenPushVar(1, type_function_null_void, sid->Idx(), sid->used_as_freevar);
    cg.TakeTemp(nargs, true);
    cg.EmitCALLV(arg_width + 1, ValWidthMulti(exptype, sf->returntype->NumValues()));
    if (sf->reqret) {
        if (!retval) cg.GenPop({ exptype, lt });
    } else {
        cg.Dummy(retval);
    }
}

void Block::Generate(CodeGen &cg, size_t retval) const {
    auto tstack_start = cg.tstack.size();
    (void)tstack_start;
    for (auto c : children) {
        cg.GenStatDebug(c);
        if (c != children.back()) {
            // Not the last element.
            cg.Gen(c, 0);
            assert(tstack_start == cg.tstack.size());
        } else {
            if (false && c->exptype->t == V_VOID) {
                // This may happen because it is an inlined function whose result is never used,
                // because returns escape out of it, e.g. check in std.lobster.
                cg.Gen(c, 0);
            } else {
                cg.Gen(c, retval);
                cg.TakeTemp(retval, true);
            }
        }
    }
}

void List::Generate(CodeGen & /*cg*/, size_t /*retval*/) const {
    assert(false);  // Handled by individual parents.
}

void TypeAnnotation::Generate(CodeGen & /*cg*/, size_t /*retval*/) const {
    assert(false);  // Handled by individual parents.
}

void Unary::Generate(CodeGen & /*cg*/, size_t /*retval*/) const {
    assert(false);  // Handled by individual parents.
}

void Coercion::Generate(CodeGen & /*cg*/, size_t /*retval*/) const {
    assert(false);  // Handled by individual parents.
}

void BinOp::Generate(CodeGen & /*cg*/, size_t /*retval*/) const {
    assert(false);  // Handled by individual parents.
}

void Seq::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(head, 0);
    cg.Gen(tail, retval);
    if (retval) cg.TakeTemp(1, true);
}

void MultipleReturn::Generate(CodeGen &cg, size_t retval) const {
    for (auto [i, c] : enumerate(children))
        cg.Gen(c, i < retval);
    cg.TakeTemp(retval, true);
    for (auto[i, c] : enumerate(children))
        if (i < retval)
            cg.rettypes.push_back({ c->exptype, c->lt });
}

void And::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(left, 1);
    cg.TakeTemp(1, false);
    auto lab = cg.EmitOpJump1(retval ? IL_JUMPFAILR : IL_JUMPFAIL);
    if (retval) cg.EmitOp0(IL_POP);
    cg.Gen(right, retval);
    if (retval) cg.TakeTemp(1, false);
    cg.EmitLabelDef(lab);
}

void Or::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(left, 1);
    cg.TakeTemp(1, false);
    auto lab = cg.EmitOpJump1(retval ? IL_JUMPNOFAILR : IL_JUMPNOFAIL);
    if (retval) cg.EmitOp0(IL_POP);
    cg.Gen(right, retval);
    if (retval) cg.TakeTemp(1, false);
    cg.EmitLabelDef(lab);
}

void Not::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (retval) {
        cg.TakeTemp(1, false);
        cg.EmitOp0(IsRefNil(child->exptype->t) ? IL_LOGNOTREF : IL_LOGNOT);
    }
}

void IfThen::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    auto lab = cg.EmitOpJump1(IL_JUMPFAIL);
    assert(!retval); (void)retval;
    cg.Gen(truepart, 0);
    cg.EmitLabelDef(lab);
}

void IfElse::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    auto lab = cg.EmitOpJump1(IL_JUMPFAIL);
    CodeGen::BlockStack bs(cg.tstack);
    bs.Start();
    cg.Gen(truepart, retval);
    bs.End();
    if (retval) cg.TakeTemp(retval, true);
    auto lab2 = cg.EmitJUMP();
    cg.EmitLabelDef(lab);
    bs.Start();
    cg.Gen(falsepart, retval);
    bs.End();
    if (retval) cg.TakeTemp(retval, true);
    cg.EmitLabelDef(lab2);
    bs.Exit(cg);
}

void While::Generate(CodeGen &cg, size_t retval) const {
    auto loopback = cg.EmitLabelDefBackwards();
    cg.loops.push_back(this);
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    auto jumpout = cg.EmitOpJump1(IL_JUMPFAIL);
    auto break_level = cg.breaks.size();
    cg.Gen(wbody, 0);
    cg.loops.pop_back();
    cg.EmitJUMPback(loopback);
    cg.EmitLabelDef(jumpout);
    cg.ApplyBreaks(break_level);
    cg.Dummy(retval);
}

void For::Generate(CodeGen &cg, size_t retval) const {
    cg.EmitPUSHINT(-1);  // i
    cg.temptypestack.push_back({ type_int, LT_ANY });
    cg.Gen(iter, 1);
    cg.loops.push_back(this);
    auto startloop = cg.EmitLabelDefBackwards();
    auto break_level = cg.breaks.size();
    auto tstack_level = cg.tstack.size();
    int exitloop = -1;
    switch (iter->exptype->t) {
        case V_INT:      exitloop = cg.EmitOpJump1(IL_IFOR); break;
        case V_STRING:   exitloop = cg.EmitOpJump1(IL_SFOR); break;
        case V_VECTOR:   exitloop = cg.EmitOpJump1(IL_VFOR); break;
        default:         assert(false);
    }
    cg.Gen(fbody, 0);
    cg.EmitJUMPback(startloop);
    cg.EmitLabelDef(exitloop);
    cg.loops.pop_back();
    cg.TakeTemp(2, false);
    assert(tstack_level == cg.tstack.size()); (void)tstack_level;
    cg.PopTemp();
    cg.PopTemp();
    cg.ApplyBreaks(break_level);
    cg.Dummy(retval);
}

void ForLoopElem::Generate(CodeGen &cg, size_t /*retval*/) const {
    auto typelt = cg.temptypestack.back();
    switch (typelt.type->t) {
        case V_INT:
            cg.EmitOp0(IL_IFORELEM);
            break;
        case V_STRING:
            cg.EmitOp0(IL_SFORELEM);
            break;
        case V_VECTOR: {
            auto outw = ValWidth(typelt.type->sub) + 2;
            if (IsRefNil(typelt.type->sub->t)) {
                if (IsStruct(typelt.type->sub->t)) {
                    cg.EmitOp1(IL_VFORELEMREF2S, cg.BitMaskForRefStuct(typelt.type->sub) , 2, outw);
                } else {
                    cg.EmitOp0(IL_VFORELEMREF, 2, outw);
                }
            } else {
                if (IsStruct(typelt.type->sub->t)) {
                    cg.EmitOp0(IL_VFORELEM2S, 2, outw);
                } else {
                    cg.EmitOp0(IL_VFORELEM, 2, outw);
                }
            }
            break;
        }
        default:
            assert(false);
    }
}

void ForLoopCounter::Generate(CodeGen &cg, size_t /*retval*/) const {
    cg.EmitOp0(IL_FORLOOPI);
}

void Break::Generate(CodeGen &cg, size_t retval) const {
    assert(!retval);
    (void)retval;
    assert(!cg.rettypes.size());
    assert(!cg.loops.empty());
    // FIXME: this code below likely doesn't work with inlined blocks
    // whose parents have temps on the stack above the top for loop.
    assert(cg.temptypestack.size() == cg.LoopTemps());
    int lab = -1;
    if (Is<For>(cg.loops.back())) {
        auto fort1 = cg.PopTemp();
        auto fort2 = cg.PopTemp();
        cg.PushTemp(fort2);
        cg.PushTemp(fort1);
        cg.GenPop(cg.temptypestack[cg.temptypestack.size() - 1]);
        cg.GenPop(cg.temptypestack[cg.temptypestack.size() - 2]);
        lab = cg.EmitJUMP();
        cg.PushTemp(fort2);
        cg.PushTemp(fort1);
    } else {
        lab = cg.EmitJUMP();
    }
    cg.breaks.push_back(lab);
}

void Switch::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(value, 1);
    cg.TakeTemp(1, false);
    // See if we do a type dispatch (always a jump table).
    if (value->exptype->t == V_CLASS) {
        GenerateTypeDispatch(cg, retval);
        return;
    }
    // See if we should do an integer jump table version.
    if (GenerateJumpTable(cg, retval))
        return;
    // Do slow default implementation for sparse integers, expressions and strings.
    auto valtlt = TypeLT{ *value, 0 };
    vector<int> nextcase, thiscase, exitswitch;
    bool have_default = false;
    auto valop = cg.PopTemp();
    CodeGen::BlockStack bs(cg.tstack);
    for (auto n : cases->children) {
        bs.Start();
        cg.PushTemp(valop);
        cg.EmitLabelDefs(nextcase);
        cg.temptypestack.push_back(valtlt);
        auto cas = AssertIs<Case>(n);
        if (cas->pattern->children.empty()) have_default = true;
        for (auto c : cas->pattern->children) {
            auto is_last = c == cas->pattern->children.back();
            cg.GenDup(valtlt);
            int loc = -1;
            auto switchtype = value->exptype;
            if (auto r = Is<Range>(c)) {
                cg.Gen(r->start, 1);
                cg.GenMathOp(switchtype, c->exptype, switchtype, MOP_GE);
                loc = cg.EmitOpJump1(IL_JUMPFAIL);
                if (is_last) nextcase.push_back(loc);
                cg.GenDup(valtlt);
                cg.Gen(r->end, 1);
                cg.GenMathOp(switchtype, c->exptype, switchtype, MOP_LE);
            } else {
                // FIXME: if this is a string, will alloc a temp string object just for the sake of
                // comparison. Better to create special purpose opcode to compare with const string.
                cg.Gen(c, 1);
                cg.GenMathOp(switchtype, c->exptype, switchtype, MOP_EQ);
            }
            if (is_last) {
                auto lab = cg.EmitOpJump1(IL_JUMPFAIL);
                nextcase.push_back(lab);
            } else {
                auto lab = cg.EmitOpJump1(IL_JUMPNOFAIL);
                thiscase.push_back(lab);
            }
            if (Is<Range>(c)) {
                if (!is_last) cg.EmitLabelDef(loc);
            }
        }
        cg.EmitLabelDefs(thiscase);
        cg.GenPop(valtlt);
        cg.TakeTemp(1, false);
        cas->Generate(cg, retval);
        bs.End();
        if (n != cases->children.back() || !have_default) {
            auto lab = cg.EmitJUMP();
            exitswitch.push_back(lab);
        }
    }
    cg.EmitLabelDefs(nextcase);
    if (!have_default) {
        bs.Start();
        cg.PushTemp(valop);
        cg.GenPop(valtlt);
        bs.End();
    }
    cg.EmitLabelDefs(exitswitch);
    bs.Exit(cg);
}

pair<IntConstant *, IntConstant *> get_range(Node *c) {
    auto start = c;
    auto end = c;
    if (auto r = Is<Range>(c)) {
        start = r->start;
        end = r->end;
    }
    return { Is<IntConstant>(start), Is<IntConstant>(end) };
};


bool Switch::GenerateJumpTable(CodeGen &cg, size_t retval) const {
    if (value->exptype->t != V_INT)
        return false;
    int64_t mini = INT64_MAX / 2, maxi = INT64_MIN / 2;
    int64_t num = 0;
    for (auto n : cases->children) {
        auto cas = AssertIs<Case>(n);
        for (auto c : cas->pattern->children) {
            auto [istart, iend] = get_range(c);
            if (!istart || !iend || istart->integer > iend->integer)
                return false;
            num += iend->integer - istart->integer + 1;
            mini = std::min(mini, istart->integer);
            maxi = std::max(maxi, iend->integer);
        }
    }
    // Decide if jump table is economic.
    const int64_t min_vals = 3;  // Minimum to do jump table.
    // TODO: This should be slightly non-linear? More values means you really want the
    // jump table, typically.
    const int64_t min_load_factor = 5;
    int64_t range = maxi - mini + 1;
    if (num < min_vals ||
        range / num > min_load_factor ||
        mini < INT32_MIN ||
        maxi >= INT32_MAX)
        return false;
    // Emit jump table version.
    cg.EmitOp(IL_JUMP_TABLE);
    GenerateJumpTableMain(cg, retval, (int)range, (int)mini, (int)maxi);
    return true;
}

void Switch::GenerateJumpTableMain(CodeGen &cg, size_t retval, int range, int mini, int maxi) const {
    auto deflab = cg.Label();
    vector<int> ilab(range + 1, deflab);
    // Figure out labels first, so we can generate code for it all at once.
    vector<int> labels;
    for (auto [i, n] : enumerate(cases->children)) {
        auto cas = AssertIs<Case>(n);
        auto lab = cg.Label();
        labels.push_back(lab);
        for (auto c : cas->pattern->children) {
            if (value->exptype->t == V_CLASS) {
                ilab[i] = lab;
            } else {
                auto [istart, iend] = get_range(c);
                assert(istart && iend);
                for (auto i = istart->integer; i <= iend->integer; i++) {
                    ilab[i - mini] = lab;
                }
            }
        }
    }
    if (vtable_idx >= 0) {
        if (cg.cpp) {
            append(cg.cb, "    switch (GetTypeSwitchID(vm, ", cg.spslot(1), ", ", vtable_idx, ")) {\n");
        } else {
            append(cg.cb, "    { int top = GetTypeSwitchID(vm, ", cg.spslot(1), ", ", vtable_idx, "); switch (top) {\n");
        }
    } else {
        if (cg.cpp) {
            append(cg.cb, "    switch (", cg.spslot(1), ".ival()) {\n");
        } else {
            append(cg.cb, "    { long long top = ", cg.spslot(1), ".ival; switch (top) {\n");
        }
    }
    vector<int> exitswitch;
    CodeGen::BlockStack bs(cg.tstack);
    for (auto [i, n] : enumerate(cases->children)) {
        bs.Start();
        auto cas = AssertIs<Case>(n);
        auto lab = labels[i];
        // Don't actually emit labels because C code turns these into "case"
        if (cas->pattern->children.empty()) {
            lab = deflab;
        }
        cg.EmitOp(IL_JUMP_TABLE_CASE_START);
        auto t = ilab.data();
        append(cg.cb, "    ");
        for (auto i = mini; i <= maxi; i++) {
            if (*t++ == lab) append(cg.cb, "case ", i, ":");
        }
        if (*t++ == lab) append(cg.cb, "default:");
        cg.cb += "\n";
        cas->Generate(cg, retval);
        bs.End();
        if (n != cases->children.back()) {
            auto lab = cg.EmitJUMP();
            exitswitch.push_back(lab);
        }
    }
    cg.EmitOp(IL_JUMP_TABLE_END);
    if (cg.cpp) cg.cb += "    } // switch\n";
    else cg.cb += "    }} // switch\n";
    cg.EmitLabelDefs(exitswitch);
    bs.Exit(cg);
}

void Switch::GenerateTypeDispatch(CodeGen &cg, size_t retval) const {
    auto dispatch_udt = value->exptype->udt;
    auto de = dispatch_udt->dispatch_table[vtable_idx].get();
    assert(de->dispatch_root && de->is_switch_dispatch &&
           de->subudts_size == dispatch_udt->subudts.size());
    (void)de;
    cg.EmitOp(IL_JUMP_TABLE_DISPATCH);
    int range = (int)cases->children.size();
    GenerateJumpTableMain(cg, retval, range, 0, range - 1);
}

void Case::Generate(CodeGen &cg, size_t retval) const {
    if (cbody->Arity()) {
        cg.Gen(cbody, retval);
        if (retval) cg.TakeTemp(1, true);
    } else {
        // An empty default case signals runtime error for enums.
        assert(pattern->children.empty());
        // FIXME: would be great to ensure the offending value is still on the stack for
        // this instruction to have access to.
        cg.EmitOp0(IL_ENUM_RANGE_ERR);
    }
}

void Range::Generate(CodeGen &/*cg*/, size_t /*retval*/) const {
    assert(false);
}

void VectorConstructor::Generate(CodeGen &cg, size_t retval) const {
    // FIXME: a malicious script can exploit this for a stack overflow.
    int arg_width = 0;
    for (auto c : children) {
        cg.Gen(c, retval);
        arg_width += ValWidth(c->exptype);
    }
    if (!retval) return;
    cg.TakeTemp(Arity(), true);
    auto offset = cg.GetTypeTableOffset(exptype);
    assert(exptype->t == V_VECTOR);
    cg.EmitOp2(IL_NEWVEC, offset, (int)Arity(), arg_width);
}

void ObjectConstructor::Generate(CodeGen &cg, size_t retval) const {
    int arg_width = 0;
    for (auto c : children) {
        cg.Gen(c, retval);
        arg_width += ValWidth(c->exptype);
    }
    if (!retval) return;
    cg.TakeTemp(Arity(), true);
    auto offset = cg.GetTypeTableOffset(exptype);
    assert(IsUDT(exptype->t));
    assert(exptype->udt->sfields.size() == Arity());
    if (IsStruct(exptype->t)) {
        // This is now a no-op! Struct elements sit inline on the stack.
    } else {
        cg.EmitNEWOBJECT(offset, arg_width);
    }
}

void AutoConstructor::Generate(CodeGen &, size_t) const {
    assert(false);
}

void IsType::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    // If the value was a scalar, then it always results in a compile time type check,
    // which means this T_IS would have been optimized out. Which means from here on we
    // can assume its a ref.
    assert(!IsUnBoxed(child->exptype->t));
    if (retval) {
        cg.TakeTemp(1, false);
        cg.EmitISTYPE(cg.GetTypeTableOffset(resolvedtype));
    }
}

void EnumCoercion::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (retval) cg.TakeTemp(1, false);
}

void Return::Generate(CodeGen &cg, size_t retval) const {
    assert(!cg.rettypes.size());
    small_vector<TypeLT, 8> typestackbackup = cg.temptypestack;
    small_vector<ILOP, 8> tstackbackup = cg.tstack;
    if (cg.temptypestack.size()) {
        // We have temps on the stack, these can be from:
        // * an enclosing for.
        // * an (inlined) block, whose caller already had temps on the stack.
        // We can't actually remove these from the stack permanently as the parent nodes still
        // expect them to be there.
        while (!cg.temptypestack.empty()) {
            cg.GenPop(cg.temptypestack.back());
            cg.temptypestack.pop_back();
        }
    }
    int nretslots = 0;
    if (sf->reqret) {
        auto nretvals = make_void ? 0 : sf->returntype->NumValues();
        if (!Is<DefaultVal>(child)) {
            cg.Gen(child, nretvals);
            cg.TakeTemp(nretvals, true);
        } else {
            cg.EmitOp0(IL_PUSHNIL);
            assert(nretvals == 1);
        }
        nretslots = ValWidthMulti(sf->returntype, nretvals);
    } else {
        if (!Is<DefaultVal>(child)) cg.Gen(child, 0);
    }
    // FIXME: we could change the VM to instead work with SubFunction ids.
    // Note: this can only work as long as the type checker forces specialization
    // of the functions in between here and the function returned to.
    // Actually, doesn't work with DDCALL and RETURN_THRU.
    // FIXME: shouldn't need any type here if V_VOID, but nretvals is at least 1 ?
    if (sf == cg.cursf && sf->returned_thru_to_max < 0) {
        cg.EmitRETURN(IL_RETURNLOCAL, nretslots, -1, nretslots);
    } else {
        // This is for both if the return itself is non-local, or if the destination has
        // an unwind check.
        cg.EmitRETURN(IL_RETURNNONLOCAL, nretslots, sf->parent->idx, nretslots);
    }

    reset_from_small_vector(cg.temptypestack, typestackbackup);
    reset_from_small_vector(cg.tstack, tstackbackup);
    // We can promise to be providing whatever retvals the caller wants.
    for (size_t i = 0; i < retval; i++) {
        cg.rettypes.push_back({ type_undefined, LT_ANY });
        cg.PushTemp(IL_RETURNLOCAL);  // FIXME: is this necessary? do more generally?
    }
}

void TypeOf::Generate(CodeGen &cg, size_t /*retval*/) const {
    if (auto idr = Is<IdentRef>(child)) {
        cg.EmitPUSHINT(cg.GetTypeTableOffset(idr->exptype));
    } else {
        auto ta = AssertIs<TypeAnnotation>(child);
        cg.EmitPUSHINT(cg.GetTypeTableOffset(ta->exptype));
    }
}

}  // namespace lobster
