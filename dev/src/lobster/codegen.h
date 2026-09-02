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

// The binary operators, in the order the helpers specialized for them come, so that a type
// prefix plus the name of one of these is the name of a helper, see MathOpName.
enum MathOp {
    MOP_ADD, MOP_SUB, MOP_MUL, MOP_DIV, MOP_MOD, MOP_LT, MOP_GT, MOP_LE, MOP_GE, MOP_EQ, MOP_NE
};

// The ops that modify the value an lvalue points at. Which one an assignment uses depends on
// both the operator and the type it works on, see GenAssign, and most of them we emit inline
// rather than call. The name of each is the name of its op.
enum LvalOp {
    LV_DUP, LV_DUPV,
    LV_WRITE, LV_WRITEREF, LV_WRITEV, LV_WRITEREFV,
    // The arithmetic ones come in MathOp order per family, which is what lets GenAssign
    // retarget an operator at the type it is used on by adding a family offset.
    LV_IADD, LV_ISUB, LV_IMUL, LV_IDIV, LV_IMOD,
    LV_BINAND, LV_BINOR, LV_XOR, LV_ASL, LV_ASR,
    LV_FADD, LV_FSUB, LV_FMUL, LV_FDIV, LV_FMOD,
    LV_IVVADD, LV_IVVSUB, LV_IVVMUL, LV_IVVDIV, LV_IVVMOD,
    LV_FVVADD, LV_FVVSUB, LV_FVVMUL, LV_FVVDIV, LV_FVVMOD,
    LV_IVSADD, LV_IVSSUB, LV_IVSMUL, LV_IVSDIV, LV_IVSMOD,
    LV_FVSADD, LV_FVSSUB, LV_FVSMUL, LV_FVSDIV, LV_FVSMOD,
    LV_SADD,
    LV_IPP, LV_IMM, LV_FPP, LV_FMM,
};

// A return either ends the function it is in, returns past it to a specific function further up,
// or is the code at a callsite that picks up such a return passing thru.
enum ReturnKind { RET_LOCAL, RET_NONLOCAL, RET_ANY };

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
    vector<int> continues;
    vector<string_view> stringtable;  // sized strings.
    vector<const Node *> node_context;
    int runtime_checks;
    vector<int> vtables;  // -1 = uninit, -2 and lower is case idx, positive is code offset.
    // How many stack slots the code emitted so far leaves in use, and the most it has ever
    // held, which is how many registers the function needs.
    size_t tstack_size = 0;
    size_t tstack_max = 0;
    const SubFunction *cursf = nullptr;
    bool cpp = false;
    // Set when the C output is going to be fed to MIR rather than libtcc, for any places where
    // the two need different code.
    bool mir = false;
    size_t nil_type_table_size = 0;

    // C/C++ codegen related.
    string &c_codegen;
    string cb;
    int regso = 0;
    int f_function_idx = -1;
    int f_regs_max = -1;
    vector<int> f_args;
    vector<int> f_defs;
    int f_keepvars = -1;
    // The C expression for the address the lvalue op chain currently being emitted produced,
    // and whether any of them needed the "lv" local to hold it.
    string f_lval;
    bool f_uses_lval = false;
    vector<int> ownedvars;
    vector<int> funstarttables;
    vector<int> var_to_local;
    bool has_profile = false;
    string sdt;
    int numlocals = 0;
    int nlabel = 0;

    int TempStackSize() {
        return (int)tstack_size;
    }

    void PopTemp() {
        assert(tstack_size);
        tstack_size--;
    }

    void PushTemp() {
        tstack_size++;
        tstack_max = std::max(tstack_max, tstack_size);
    }

    struct BlockStack {
        size_t &tstack_size;
        size_t start;
        size_t max;
        BlockStack(size_t &s) : tstack_size(s), start(s), max(s) {}
        void Start() { tstack_size = start; }
        void End() { max = std::max(max, tstack_size); }
        void Exit(CodeGen &cg) {
            assert(max >= tstack_size);
            // Values from something that does not return.
            while (tstack_size < max) cg.PushTemp();
        }
    };

    // Track the stack the generated code works on: the caller says how many slots the code it is
    // about to emit reads and how many it leaves behind.
    void TrackUseDef(int useslots, int defslots) {
        regso = TempStackSize();
        for (int i = 0; i < useslots; i++) PopTemp();
        for (int i = 0; i < defslots; i++) PushTemp();
        //LOG_DEBUG("cg: ", useslots, "/", defslots, " -> ", tstack_size);
    }

    const int ti_num_udt_fields = 8;
    const int ti_num_udt_per_field = 3;

    type_elem_t PushDefaultValue(ValueType vt, VTValue val) {
        switch (vt) {
            case V_INT: {
                auto iv = val.i;
                auto &it = default_ints_lookup[iv];
                if (!it) {
                    it = (type_elem_t)type_table.size();
                    type_table.insert(type_table.end(), (type_elem_t *)&iv,
                                      (type_elem_t *)(&iv + 1));
                }
                return it;
            }
            case V_FLOAT: {
                auto fv = val.f;
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
        VTValue val;
        auto cons = Is<ObjectConstructor>(dv);
        if (cons) {
            vector<pair<ValueType, VTValue>> vals;
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
    type_elem_t GetTypeTableOffset(TypeRef type, type_elem_t non_nil_version = (type_elem_t)0) {
        small_vector<type_elem_t, 2> tt;
        tt.push_back((type_elem_t)VT2RT(type->t));
        tt.push_back(non_nil_version);
        switch (type->t) {
            case V_INT:
                tt.push_back((type_elem_t)(type->e ? type->e->idx : -1));
                break;
            case V_NIL:
                return GetTypeTableOffset(type->sub, GetTypeTableOffset(type->sub));
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
                auto &typeinfo = non_nil_version ? udt->typeinfonil : udt->typeinfonon;
                if (typeinfo >= 0)
                    return typeinfo;
                typeinfo = (type_elem_t)type_table.size();
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
                tt.push_back((type_elem_t)udt->subtype_dfs);
                PushFields(udt, tt);
                assert(ssize(tt) == ttsize);
                std::copy(tt.begin(), tt.end(), type_table.begin() + typeinfo);
                if (non_nil_version) nil_type_table_size += ttsize;
                return typeinfo;
            }
            case V_TYPEID:
                // These are not strongly typed at runtime.
                // TODO: could change this into an opaque enum type? Could even generate the enum?
                return TYPE_ELEM_INT;
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
        if (non_nil_version) nil_type_table_size += tt.size();
        return offset;
    }

    CodeGen(Parser &_p, SymbolTable &_st, bool return_value, int runtime_checks, bool cpp,
            uint64_t src_hash, string &c_codegen, string_view custom_pre_init_name, bool mir)
        : parser(_p), st(_st), runtime_checks(runtime_checks), cpp(cpp), mir(mir),
          c_codegen(c_codegen) {
        node_context.push_back(parser.root);

        // Reserve space and index for all vtables.
        for (auto udt : st.udttable) {
            udt->vtable_start = (int)vtables.size();
            vtables.insert(vtables.end(), udt->dispatch_table.size(), -1);
        }

        // Assign ids to all UDTs in depth-first pre-order over the inheritance
        // forest, such that the ids of any UDT's subtree (including itself)
        // form a contiguous range, allowing ISSUBTYPE to test for subtype
        // membership with a single range check. The set of UDTs is complete
        // here, so subclasses participate regardless of where they were
        // declared relative to uses of "is".
        for (auto udt : st.udttable) {
            if (udt->ssuperclass) {
                udt->next_subclass = udt->ssuperclass->first_subclass;
                udt->ssuperclass->first_subclass = udt;
            }
        }
        int subtype_id = 0;
        auto assign_ids = [&](UDT *udt, auto &&assign_ids) -> void {
            udt->subtype_dfs = subtype_id++;
            for (auto sub = udt->first_subclass; sub; sub = sub->next_subclass)
                assign_ids(sub, assign_ids);
            udt->subtype_dfs_end = subtype_id - 1;
        };
        for (auto udt : st.udttable)
            if (!udt->ssuperclass) assign_ids(udt, assign_ids);

        // Pre-load some types into the table, must correspond to type_elem_t enums.
        Type type_valuebuf(V_VALUEBUF);
        Type type_vec_str(V_VECTOR, &*type_string);        
        Type type_v_v_int(V_VECTOR, &*type_vector_int);    
        Type type_v_v_float(V_VECTOR, &*type_vector_float);
        TypeRef type_vector_float4 = st.GetVectorType(V_FLOAT, 1, 4);
        TypeRef type_vector_vector_float4 = st.GetVectorType(V_FLOAT, 2, 4);
        type_elem_t o = TYPE_ELEM_UNDEFINED;
        o = GetTypeTableOffset(type_int);                  assert(o == TYPE_ELEM_INT);
        o = GetTypeTableOffset(type_float);                assert(o == TYPE_ELEM_FLOAT);
        o = GetTypeTableOffset(type_string);               assert(o == TYPE_ELEM_STRING);
        o = GetTypeTableOffset(type_resource);             assert(o == TYPE_ELEM_RESOURCE);
        o = GetTypeTableOffset(type_any);                  assert(o == TYPE_ELEM_ANY);
        o = GetTypeTableOffset(&type_valuebuf);            assert(o == TYPE_ELEM_VALUEBUF);
        o = GetTypeTableOffset(type_vector_int);           assert(o == TYPE_ELEM_VECTOR_OF_INT);
        o = GetTypeTableOffset(type_vector_float);         assert(o == TYPE_ELEM_VECTOR_OF_FLOAT);
        o = GetTypeTableOffset(&type_vec_str);             assert(o == TYPE_ELEM_VECTOR_OF_STRING);
        o = GetTypeTableOffset(&type_v_v_int);             assert(o == TYPE_ELEM_VECTOR_OF_VECTOR_OF_INT);
        o = GetTypeTableOffset(&type_v_v_float);           assert(o == TYPE_ELEM_VECTOR_OF_VECTOR_OF_FLOAT);
        o = GetTypeTableOffset(type_vector_resource);      assert(o == TYPE_ELEM_VECTOR_OF_RESOURCE);
        o = GetTypeTableOffset(type_vector_float4);        assert(o == TYPE_ELEM_VECTOR_OF_FLOAT4);
        o = GetTypeTableOffset(type_vector_vector_float4); assert(o == TYPE_ELEM_VECTOR_OF_VECTOR_OF_FLOAT4);
        (void)o;

        for (auto f : parser.st.functiontable) {
            if (!f->istype) {
                for (auto ov : f->overloads) for (auto sf = ov->sf; sf; sf = sf->next) {
                    if (sf->typechecked) {
                        // We only set this here, because any inlining of anonymous functions in
                        // the optimizers is likely to reduce the amount of vars for which this is
                        // true a great deal.
                        for (auto &fv : sf->freevars) {
                            fv.sid->used_as_freevar = true;
                        }
                    }
                }
            }
        }

        int sidx = 0;
        for (auto sid : st.specidents) {
            if (!sid->type.Null() && !sid->constprop) {  // Null ones are in unused functions.
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
                    if (ser_ids[udt->serializable_id] >= 0) {
                        // TODO: this is niche, so probably ok here, but even better moved to Parser.
                        parser.Error(cat(udt->name,
                            " has \"attribute serializable\" with index that is already in use: ",
                            udt->serializable_id));
                    }
                    ser_ids[udt->serializable_id] = typeoff;
                }
            }
        }

        Prologue(c_codegen);

        // Start of the actual generated code.
        linenumbernodes.push_back(parser.root);
        // Generate a dummmy function for function values that are never called.
        // Would be good if the optimizer guarantees these don't exist, but for now this is
        // more debuggable if it does happen to get called.
        f_function_idx = CODEGEN_SPECIAL_FUNCTION_ID_DUMMY;
        f_regs_max = 0;
        f_args.clear();
        f_defs.clear();
        f_keepvars = 0;
        TrackUseDef(0, 0);
        append(cb, "    RtAbort(vm);\n");
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
        TrackUseDef(int(return_value), 0);
        append(cb, "    RtExit(vm, ", sp(), ", (type_elem_t)",
               return_value ? GetTypeTableOffset(type) : -1, ");\n");
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
        while (retval--) EmitPushNil();
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
                if (arg.sid->constprop) {
                    continue;
                }
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
            EmitProfile((int)stringtable.size() - 1);
        }

        if (sf.sbody) for (auto c : sf.sbody->children) {
            GenStatDebug(c);
            Gen(c, 0);
            assert(!tstack_size);
        }
        else Dummy(sf.reqret);

        assert(temptypestack.empty());
        assert(breaks.empty());
        assert(continues.empty());
        assert(!tstack_size);
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
                "typedef lobster::type_elem_t type_elem_t;\n"
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
                // These need to correspond to the C++ LVector and LString, enforced in Entry().
                // We mirror them so that reading a length or an element is a load rather than a
                // call. Only the fields up to the last one we read have to be right, but keeping
                // the whole type here means the size check covers the tail as well.
                "typedef struct {\n"
                "    RefObj ro;\n"
                "    long long len;\n"
                "    long long maxl;\n"
                "    long long width;\n"
                "    Value *elems;\n"
                "} LVector;\n"
                "typedef struct {\n"
                "    RefObj ro;\n"
                "    long long len;\n"
                "} LString;\n"
                // The characters of a string follow directly behind its header.
                "#define LSTRING_DATA(S) ((unsigned char *)((S) + 1))\n"
                // This needs to correspond to the C++ VMBase, enforced in Entry().
                "typedef struct {\n"
                "    int last_line;\n"
                "    int last_fileidx;\n"
                "    int ret_unwind_to;\n"
                "    int ret_slots;\n"
                "    Value *fvars_ptr;\n"
                "    Value *constant_strings_ptr;\n"
                "} VMBase;\n"
                "typedef Value *StackPtr;\n"
                "typedef VMBase *VMRef;\n"
                "typedef void(*fun_base_t)(VMRef, StackPtr);\n"
                // An offset into the type table, which is what the helpers take one as.
                "typedef int type_elem_t;\n"
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

            // Every runtime helper the generated code can call. These mirror the Rt functions in
            // vmops.h, which is what the JIT links them to, see vm_ops_jit_table.
            sd +=
                "void RtPushFloat(StackPtr, long long);\n"
                "void RtPushStr(VMRef, StackPtr, int);\n"
                "void RtIndexVecSub(VMRef, StackPtr, int);\n"
                "void RtIndexVecSubV(VMRef, StackPtr, int, int);\n"
                "void RtIndexVecNestSubV(VMRef, StackPtr, int, int, int);\n"
                "void RtIndexStruct(VMRef, StackPtr, int);\n"
                "void RtPushFieldMRef(VMRef, StackPtr, int);\n"
                "void RtPushFieldV(StackPtr, int, int);\n"
                "void RtPushFieldV2V(StackPtr, int, int, int);\n"
                "void RtNativeCallV(VMRef, StackPtr, int, int);\n"
                "void RtNativeCall0(VMRef, StackPtr, int, int);\n"
                "void RtNativeCall1(VMRef, StackPtr, int, int);\n"
                "void RtNativeCall2(VMRef, StackPtr, int, int);\n"
                "void RtNativeCall3(VMRef, StackPtr, int, int);\n"
                "void RtNativeCall4(VMRef, StackPtr, int, int);\n"
                "void RtNativeCall5(VMRef, StackPtr, int, int);\n"
                "void RtNativeCall6(VMRef, StackPtr, int, int);\n"
                "void RtNativeCall7(VMRef, StackPtr, int, int);\n"
                "void RtNewVec(VMRef, StackPtr, type_elem_t, int);\n"
                "void RtNewObject(VMRef, StackPtr, type_elem_t);\n"
                "void RtPopV(StackPtr, int);\n"
                "void RtExit(VMRef, StackPtr, type_elem_t);\n"
                "void RtAbort(VMRef);\n"
                "void RtIAdd(VMRef, StackPtr);\n"
                "void RtISub(VMRef, StackPtr);\n"
                "void RtIMul(VMRef, StackPtr);\n"
                "void RtIDiv(VMRef, StackPtr);\n"
                "void RtIMod(VMRef, StackPtr);\n"
                "void RtILt(VMRef, StackPtr);\n"
                "void RtIGt(VMRef, StackPtr);\n"
                "void RtILe(VMRef, StackPtr);\n"
                "void RtIGe(VMRef, StackPtr);\n"
                "void RtIEq(VMRef, StackPtr);\n"
                "void RtINe(VMRef, StackPtr);\n"
                "void RtFAdd(VMRef, StackPtr);\n"
                "void RtFSub(VMRef, StackPtr);\n"
                "void RtFMul(VMRef, StackPtr);\n"
                "void RtFDiv(VMRef, StackPtr);\n"
                "void RtFMod(VMRef, StackPtr);\n"
                "void RtFLt(VMRef, StackPtr);\n"
                "void RtFGt(VMRef, StackPtr);\n"
                "void RtFLe(VMRef, StackPtr);\n"
                "void RtFGe(VMRef, StackPtr);\n"
                "void RtFEq(VMRef, StackPtr);\n"
                "void RtFNe(VMRef, StackPtr);\n"
                "void RtSAdd(VMRef, StackPtr);\n"
                "void RtSSub(VMRef, StackPtr);\n"
                "void RtSMul(VMRef, StackPtr);\n"
                "void RtSDiv(VMRef, StackPtr);\n"
                "void RtSMod(VMRef, StackPtr);\n"
                "void RtSLt(VMRef, StackPtr);\n"
                "void RtSGt(VMRef, StackPtr);\n"
                "void RtSLe(VMRef, StackPtr);\n"
                "void RtSGe(VMRef, StackPtr);\n"
                "void RtSEq(VMRef, StackPtr);\n"
                "void RtSNe(VMRef, StackPtr);\n"
                "void RtStrConcatN(VMRef, StackPtr, int);\n"
                "void RtIvvAdd(VMRef, StackPtr, int);\n"
                "void RtIvvSub(VMRef, StackPtr, int);\n"
                "void RtIvvMul(VMRef, StackPtr, int);\n"
                "void RtIvvDiv(VMRef, StackPtr, int);\n"
                "void RtIvvMod(VMRef, StackPtr, int);\n"
                "void RtIvvLt(VMRef, StackPtr, int);\n"
                "void RtIvvGt(VMRef, StackPtr, int);\n"
                "void RtIvvLe(VMRef, StackPtr, int);\n"
                "void RtIvvGe(VMRef, StackPtr, int);\n"
                "void RtFvvAdd(VMRef, StackPtr, int);\n"
                "void RtFvvSub(VMRef, StackPtr, int);\n"
                "void RtFvvMul(VMRef, StackPtr, int);\n"
                "void RtFvvDiv(VMRef, StackPtr, int);\n"
                "void RtFvvMod(VMRef, StackPtr, int);\n"
                "void RtFvvLt(VMRef, StackPtr, int);\n"
                "void RtFvvGt(VMRef, StackPtr, int);\n"
                "void RtFvvLe(VMRef, StackPtr, int);\n"
                "void RtFvvGe(VMRef, StackPtr, int);\n"
                "void RtIvsAdd(VMRef, StackPtr, int);\n"
                "void RtIvsSub(VMRef, StackPtr, int);\n"
                "void RtIvsMul(VMRef, StackPtr, int);\n"
                "void RtIvsDiv(VMRef, StackPtr, int);\n"
                "void RtIvsMod(VMRef, StackPtr, int);\n"
                "void RtIvsLt(VMRef, StackPtr, int);\n"
                "void RtIvsGt(VMRef, StackPtr, int);\n"
                "void RtIvsLe(VMRef, StackPtr, int);\n"
                "void RtIvsGe(VMRef, StackPtr, int);\n"
                "void RtFvsAdd(VMRef, StackPtr, int);\n"
                "void RtFvsSub(VMRef, StackPtr, int);\n"
                "void RtFvsMul(VMRef, StackPtr, int);\n"
                "void RtFvsDiv(VMRef, StackPtr, int);\n"
                "void RtFvsMod(VMRef, StackPtr, int);\n"
                "void RtFvsLt(VMRef, StackPtr, int);\n"
                "void RtFvsGt(VMRef, StackPtr, int);\n"
                "void RtFvsLe(VMRef, StackPtr, int);\n"
                "void RtFvsGe(VMRef, StackPtr, int);\n"
                "void RtSivAdd(VMRef, StackPtr, int);\n"
                "void RtSivSub(VMRef, StackPtr, int);\n"
                "void RtSivMul(VMRef, StackPtr, int);\n"
                "void RtSivDiv(VMRef, StackPtr, int);\n"
                "void RtSivMod(VMRef, StackPtr, int);\n"
                "void RtSivLt(VMRef, StackPtr, int);\n"
                "void RtSivGt(VMRef, StackPtr, int);\n"
                "void RtSivLe(VMRef, StackPtr, int);\n"
                "void RtSivGe(VMRef, StackPtr, int);\n"
                "void RtSfvAdd(VMRef, StackPtr, int);\n"
                "void RtSfvSub(VMRef, StackPtr, int);\n"
                "void RtSfvMul(VMRef, StackPtr, int);\n"
                "void RtSfvDiv(VMRef, StackPtr, int);\n"
                "void RtSfvMod(VMRef, StackPtr, int);\n"
                "void RtSfvLt(VMRef, StackPtr, int);\n"
                "void RtSfvGt(VMRef, StackPtr, int);\n"
                "void RtSfvLe(VMRef, StackPtr, int);\n"
                "void RtSfvGe(VMRef, StackPtr, int);\n"
                "void RtAEq(StackPtr);\n"
                "void RtANe(StackPtr);\n"
                "void RtSnEq(StackPtr);\n"
                "void RtSnNe(StackPtr);\n"
                "void RtStEq(StackPtr, int);\n"
                "void RtStNe(StackPtr, int);\n"
                "void RtLEq(StackPtr);\n"
                "void RtLNe(StackPtr);\n"
                "void RtIUMinus(StackPtr);\n"
                "void RtFUMinus(StackPtr);\n"
                "void RtIvUMinus(VMRef, StackPtr, int);\n"
                "void RtFvUMinus(VMRef, StackPtr, int);\n"
                "void RtBinAnd(StackPtr);\n"
                "void RtBinOr(StackPtr);\n"
                "void RtXor(StackPtr);\n"
                "void RtAsl(StackPtr);\n"
                "void RtAsr(StackPtr);\n"
                "void RtNeg(StackPtr);\n"
                "void RtToString(VMRef, StackPtr, type_elem_t);\n"
                "void RtStructToString(VMRef, StackPtr, type_elem_t);\n"
                "void RtIsType(StackPtr, type_elem_t, int);\n"
                "void RtIsSubType(VMRef, StackPtr, int, int, int);\n"
                "void RtCallValue(VMRef, StackPtr);\n"
                "void RtDynDispatch(VMRef, StackPtr, int, int);\n"
                "void RtEnumRangeErr(VMRef);\n"
                "Value *RtLvalIndexVecV(VMRef, StackPtr, int, int);\n"
                "Value *RtLvalIndexClass(VMRef, StackPtr, int);\n"
                "Value *RtLvalIndexStruct(VMRef, StackPtr, Value *, int, int);\n"
                "void RtLvDupV(StackPtr, Value *, int);\n"
                "void RtLvIDiv(VMRef, StackPtr, Value *);\n"
                "void RtLvIMod(VMRef, StackPtr, Value *);\n"
                "void RtLvAsl(VMRef, StackPtr, Value *);\n"
                "void RtLvAsr(VMRef, StackPtr, Value *);\n"
                "void RtLvFMod(VMRef, StackPtr, Value *);\n"
                "void RtLvIvvAdd(VMRef, StackPtr, Value *, int);\n"
                "void RtLvIvvSub(VMRef, StackPtr, Value *, int);\n"
                "void RtLvIvvMul(VMRef, StackPtr, Value *, int);\n"
                "void RtLvIvvDiv(VMRef, StackPtr, Value *, int);\n"
                "void RtLvIvvMod(VMRef, StackPtr, Value *, int);\n"
                "void RtLvFvvAdd(VMRef, StackPtr, Value *, int);\n"
                "void RtLvFvvSub(VMRef, StackPtr, Value *, int);\n"
                "void RtLvFvvMul(VMRef, StackPtr, Value *, int);\n"
                "void RtLvFvvDiv(VMRef, StackPtr, Value *, int);\n"
                "void RtLvFvvMod(VMRef, StackPtr, Value *, int);\n"
                "void RtLvIvsAdd(VMRef, StackPtr, Value *, int);\n"
                "void RtLvIvsSub(VMRef, StackPtr, Value *, int);\n"
                "void RtLvIvsMul(VMRef, StackPtr, Value *, int);\n"
                "void RtLvIvsDiv(VMRef, StackPtr, Value *, int);\n"
                "void RtLvIvsMod(VMRef, StackPtr, Value *, int);\n"
                "void RtLvFvsAdd(VMRef, StackPtr, Value *, int);\n"
                "void RtLvFvsSub(VMRef, StackPtr, Value *, int);\n"
                "void RtLvFvsMul(VMRef, StackPtr, Value *, int);\n"
                "void RtLvFvsDiv(VMRef, StackPtr, Value *, int);\n"
                "void RtLvFvsMod(VMRef, StackPtr, Value *, int);\n"
                "void RtLvSAdd(VMRef, StackPtr, Value *);\n"
                "int RtStaticSetThisFrame(VMRef, int);\n"
                "int RtMemberSetThisFrame(VMRef, StackPtr, int);\n"
                ;

            sd += "extern fun_base_t GetNextCallTarget(VMRef);\n"
                  "extern void Entry(int, int, int, int, int, int);\n"
                  "extern void IDXErr(VMRef, long long, long long, RefObj *);\n"
                  "extern void GLFrame(StackPtr, VMRef);\n"
                  "extern void SwapVars(VMRef, int, StackPtr, int);\n"
                  "extern void BackupVar(VMRef, int);\n"
                  "extern void DecOwned(VMRef, int);\n"
                  "extern void DecDelete(VMRef, RefObj *);\n"
                  "extern void AssertFailed(VMRef, int, int, int);\n"
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

    string IdName(int i, bool is_whole_struct, TypeRef type) {
        auto ididx = sids[i].ididx();
        auto idx = sids[i].idx();
        auto &basename = st.identtable[ididx]->name;
        if (is_whole_struct || !IsStruct(type->t)) {
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
        TrackUseDef(0, 0);
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

    // Where a global lives. The C++ backend addresses the VM's own array at a constant offset,
    // which is why that array sits at the end of the VM; the C one has no way to know where that
    // is, so it goes thru the pointer to it that VMBase carries for that purpose.
    string FVar(int offset) {
        return cpp ? cat("vm.fvars + ", offset) : cat("vm->fvars_ptr + ", offset);
    }

    void GenPushVar(size_t retval, TypeRef type, int offset, bool used_as_freevar) {
        if (!retval) return;
        auto slot = [&](int i) {
            return used_as_freevar ? FVar(offset + i)
                                   : cat("locals + ", var_to_local[offset + i]);
        };
        if (IsStruct(type->t)) {
            auto width = ValWidth(type);
            TrackUseDef(0, width);
            for (int i = 0; i < width; i++) {
                GenValueCopy(cb, sp(-i), slot(i), "");
                comment(cat(IdName(offset, true, type), ".", i));
            }
        } else {
            TrackUseDef(0, 1);
            GenValueCopy(cb, sp(0), slot(0), "");
            comment(IdName(offset, false, type));
        }
    }

    // A local is at a known address, so this needs no code at all, just a note of where the
    // assignment that follows writes to.
    void EmitLvalLocal(int offset, TypeRef type) {
        TrackUseDef(0, 0);
        f_lval = cat("locals + ", var_to_local[offset]);
        append(cb, "    // lval: ", IdName(offset, false, type), "\n");
    }

    // A global is at a known address too, once the generated code can get at the array.
    void EmitLvalGlobal(int offset, TypeRef type) {
        TrackUseDef(0, 0);
        f_lval = FVar(offset);
        append(cb, "    // lval: ", IdName(offset, false, type), "\n");
    }

    // A field as an lvalue is at a constant offset from the object, whose fields sit right behind
    // its header, same as reading one. That does lose a debug only range check.
    void EmitLvalField(int slot) {
        TrackUseDef(1, 0);
        f_uses_lval = true;
        if (cpp) {
            append(cb, "    lv = &(", sp(1), ")->oval()->AtR(", slot, ");\n");
        } else {
            append(cb, "    lv = (Value *)((RefObj *)(", sp(1), ")->ref + 1) + ", slot, ";\n");
        }
        f_lval = "lv";
    }

    // An element of a vector as an lvalue: the range check, then the address, at the width
    // the vector holds its elements at plus wherever in one the assignment lands.
    void EmitLvalVectorIndex(int offset, int width) {
        TrackUseDef(2, 0);
        f_uses_lval = true;
        if (cpp) {
            append(cb, "    {\n    auto _o = (", sp(2), ")->vval(); auto _i = (", sp(1),
                   ")->ival();\n");
            append(cb, "    if ((uint64_t)_i >= (uint64_t)_o->len) vm.IDXErr(_i, _o->len, _o);\n");
            append(cb, "    lv = _o->Elems() + _i * ", width, " + ", offset, ";\n    }\n");
        } else {
            append(cb, "    {\n    LVector *_o = (LVector *)(", sp(2), ")->ref; long long _i = (",
                   sp(1), ")->ival;\n");
            append(cb, "    if ((unsigned long long)_i >= (unsigned long long)_o->len)"
                       " IDXErr(vm, _i, _o->len, &_o->ro);\n");
            append(cb, "    lv = _o->elems + _i * ", width, " + ", offset, ";\n    }\n");
        }
        f_lval = "lv";
    }

    // Indexing to get an lvalue hands the address to whatever follows thru a local rather than
    // thru the VM, so they read as a chain of assignments. Only the struct one steps into an
    // address it is given, which is what chained says; the rest start a fresh one and ignore it.
    // None of them leave anything on the stack.
    void EmitLvalIndex(string_view opname, bool chained, std::initializer_list<int> args,
                       int useslots) {
        TrackUseDef(useslots, 0);
        f_uses_lval = true;
        append(cb, "    lv = ", opname, "(vm, ", sp());
        if (chained) append(cb, ", ", f_lval);
        for (auto a : args) append(cb, ", ", a);
        cb += ");\n";
        f_lval = "lv";
    }

    void EmitPushStr(int stringtableindex) {
        TrackUseDef(0, 1);
        if (STRING_CONSTANTS_KEEP) {
            // Still has a reference to take, so leave it to the helper.
            append(cb, "    RtPushStr(vm, ", sp(), ", ", stringtableindex, ");");
        } else {
            // Borrowed, so all that is left is the copy out of the VM's table of them.
            GenValueCopy(cb, sp(0), cpp ? cat("vm.constant_strings_ptr + ", stringtableindex)
                                        : cat("vm->constant_strings_ptr + ", stringtableindex),
                         "");
        }
        auto sv = stringtable[stringtableindex];
        sv = sv.substr(0, 50);
        string q;
        EscapeAndQuote(sv, q, true);
        comment(q);
    }

    int EmitJump() {
        TrackUseDef(0, 0);
        auto lab = Label();
        append(cb, "    goto block", lab, ";\n");
        return lab;
    }

    int EmitJumpBack(int lab) {
        TrackUseDef(0, 0);
        append(cb, "    goto block", lab, ";\n");
        return lab;
    }

    // Jump on the value on top of the stack testing false, or on it testing true for the
    // "no fail" version. Small enough to be worth not calling for. defslots is 1 when the value
    // stays on the stack for whoever follows.
    int EmitJumpCond(bool onfail, int defslots) {
        TrackUseDef(1, defslots);
        auto lab = Label();
        append(cb, "    if (", onfail ? "!" : "", "(", sp(1), ")->", cpp ? "True()" : "ival",
               ") goto block", lab, ";\n");
        return lab;
    }
    int EmitJumpFail(int defslots) { return EmitJumpCond(true, defslots); }
    int EmitJumpNoFail(int defslots) { return EmitJumpCond(false, defslots); }

    // Jump over the code that copies return values when the function we called returned past us
    // to somewhere further up instead.
    int EmitJumpIfUnwound(int parent_idx) {
        TrackUseDef(0, 0);
        auto lab = Label();
        append(cb, "    if (", vmref(), "ret_unwind_to == ", parent_idx, ") goto block", lab,
               ";\n");
        return lab;
    }

    // Jump over the initializer of a member or static that has already run this frame. The
    // member version reads the object it belongs to off the stack, the static one needs nothing.
    int EmitJumpIfSetThisFrame(bool member, int varidx) {
        TrackUseDef(member ? 1 : 0, 0);
        auto lab = Label();
        append(cb, "    if (!");
        if (member) append(cb, "RtMemberSetThisFrame(vm, ", sp(), ", ");
        else append(cb, "RtStaticSetThisFrame(vm, ");
        append(cb, varidx, ")) goto block", lab, ";\n");
        return lab;
    }

    // There is one helper per number of arguments a native takes, plus a V one for those that
    // take a variable number, which is what a negative count asks for.
    void EmitNativeCall(int nargs, NativeFun *nf, int has_ret, int useslots, int defslots) {
        TrackUseDef(useslots, defslots);
        if (nf->IsGLFrame()) {
            append(cb, "    GLFrame(", sp(), ", vm);\n");
        } else {
            append(cb, "    RtNativeCall");
            if (nargs < 0) cb += "V"; else append(cb, nargs);
            append(cb, "(vm, ", sp(), ", ", nf->idx, ", ", has_ret, ");");
            comment(nf->name);
        }
    }

    void EmitKeep(int stack_offset, int keep_index_add) {
        // Inside a loop the slot may already hold a reference from a previous iteration.
        auto inloop = !loops.empty();
        TrackUseDef(0, 0);
        auto offset = f_keepvars++ + keep_index_add;
        if (inloop) append(cb, "    DecVal(vm, keepvar[", offset, "]);\n");
        GenValueCopy(cb, cat("keepvar + ", offset), sp(stack_offset + 1));

    }

    void EmitReturn(ReturnKind kind, int nretslots, int parent_idx, int useslots) {
        TrackUseDef(useslots, 0);
        // FIXME: emit epilogue stuff only once at end of function.
        if (kind == RET_LOCAL) {
            #if VM_EXTRA_CHECKING
                append(cb, "    ", vmref(), "ret_slots = -9;\n");
                append(cb, "    ", vmref(), "ret_unwind_to = -9;\n");
            #endif
        } else if (kind == RET_NONLOCAL) {
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
        if (kind == RET_ANY) {
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
        if (kind == RET_ANY) {
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

    void EmitPushFun(int fidx) {
        TrackUseDef(0, 1);
        if (cpp) {
            append(cb, "    *(", sp(), ") = Value((fun_base_t)fun_", fidx, ");\n");
        } else {
            append(cb, "    { StackPtr _sp = ", sp(), "; _sp->ival = (long long)fun_",
                   fidx, ";", SetType(RTT_FUNCTION), " }\n");
        }
    }

    void EmitCall(int fidx, int uses, int defs) {
        TrackUseDef(uses, defs);
        append(cb, "    fun_", fidx, "(vm, ", sp(), ");");
        comment("call: " + Signature(*st.subfunctiontable[fidx]));
    }

    void EmitCallValue(int uses, int defs) {
        TrackUseDef(uses, defs);
        append(cb, "    RtCallValue(vm, ", sp(), "); ");
        if (cpp) append(cb, "vm.next_call_target(vm, ", sp(1), ");\n");
        else append(cb, "GetNextCallTarget(vm)(vm, ", sp(1), ");\n");
    }

    void EmitDynDispatch(int vtable_idx, int nargs, int uses, int defs) {
        TrackUseDef(uses, defs);
        append(cb, "    RtDynDispatch(vm, ", sp(), ", ", vtable_idx, ", ", nargs, "); ");
        if (cpp) append(cb, "vm.next_call_target(vm, ", sp(), ");\n");
        else append(cb, "GetNextCallTarget(vm)(vm, ", sp(), ");\n");
    }

    void EmitProfile(int stringtable_idx) {
        TrackUseDef(0, 0);
        string name;
        EscapeAndQuote(stringtable[stringtable_idx], name, true);
        append(cb, "    static struct ___tracy_source_location_data tsld = { ", name, ", ", name,
               ", \"\", 0, 0x888800 }; struct ___tracy_c_zone_context ctx = ",
               cpp ? "lobster::" : "", "StartProfile(&tsld);\n");
        has_profile = true;
    }

    void EmitIsType(int type_idx, int nilres, TypeRef type) {
        TrackUseDef(1, 1);
        append(cb, "    RtIsType(", sp(), ", (type_elem_t)", type_idx, ", ", nilres, ");");
        if (IsUDT(type->t)) comment(type->udt->name);
        else cb += "\n";
    }

    void EmitIsSubType(int start, int end, int nilres, TypeRef type) {
        TrackUseDef(1, 1);
        append(cb, "    RtIsSubType(vm, ", sp(), ", ", start, ", ", end, ", ", nilres, ");");
        comment(type->udt->name);
    }

    void EmitNewObject(int type_idx, int uses, TypeRef type) {
        TrackUseDef(uses, 1);
        append(cb, "    RtNewObject(vm, ", sp(), ", (type_elem_t)", type_idx, ");");
        if (IsUDT(type->t)) comment(type->udt->name);
        else cb += "\n";
    }

    void EmitStructToString(int type_idx, int uses, TypeRef type) {
        TrackUseDef(uses, 1);
        append(cb, "    RtStructToString(vm, ", sp(), ", (type_elem_t)", type_idx, ");");
        if (IsUDT(type->t)) comment(type->udt->name);
        else cb += "\n";
    }

    string SetType(RTType t) {
        #if RTT_ENABLED
            return cat(" _sp->type = ", (int)t, ";");
        #else
            (void)t;
            return {};
        #endif
    }

    void EmitPushInt(int val) {
        TrackUseDef(0, 1);
        if (cpp) {
            append(cb, "    *(", sp(), ") = Value(", val, ");\n");
        } else {
            append(cb, "    { StackPtr _sp = ", sp(), "; _sp->ival = ", val, ";", SetType(RTT_INT), " }\n");
        }
    }

    void GenFloat(double f) {
        if ((float)f == f && isfinite(f)) {
            TrackUseDef(0, 1);
            // We're printing the float as text which seems dangerous, but this path is only
            // taken where double and float are identical, meaning typically whole numbers and
            // other precisely representable ones.
            if (cpp) {
                append(cb, "    *(", sp(), ") = Value(", to_string_hexfloat((float)f), ");\n");
            } else {
                append(cb, "    { StackPtr _sp = ", sp(), "; _sp->fval = ", to_string_hexfloat((float)f), ";", SetType(RTT_FLOAT), " }\n");
            }
        } else {
            int2float64 i2f(f);
            EmitPushConst64(true, i2f.i);
            comment(to_string_float(f));
        }
    }

    void SetToNil(string &sd, string_view target) {
        if (cpp)
            append(sd, "    *(", target, ") = Value(0, lobster::RTT_NIL);\n");
        else
            append(sd, "    { StackPtr _sp = ", target, "; _sp->ival = 0;", SetType(RTT_NIL), " }\n");
    }

    // Only the decrement itself is worth emitting: what happens when it reaches zero is a good
    // deal more code, and stays a call.
    void GenDecRef(string_view slot) {
        if (cpp) {
            append(cb, "    (", slot, ")->LTDECRTNIL(vm);\n");
        } else {
            append(cb, "    { RefObj *_r = (", slot, ")->ref;"
                       " if (_r && --_r->refc <= 0) DecDelete(vm, _r); }\n");
        }
    }

    void GenIncRef(string_view slot) {
        if (cpp) {
            append(cb, "    (", slot, ")->LTINCRTNIL();\n");
        } else {
            append(cb, "    { RefObj *_r = (", slot, ")->ref; if (_r) _r->refc++; }\n");
        }
    }

    void EmitIncRef(int off) {
        TrackUseDef(0, 0);
        // FIXME: even when the static type is IsRef (i.e. no NIL or scalar), at runtime it is
        // still possible we get passed an int false value due to the way and/or are compiled?
        // See e.g. astar_result in the test.
        // Would be great to remove this case since the if-check is not needed in almost all cases.
        GenIncRef(sp(off + 1));
    }

    // The ones below are a move or a test on the stack and nothing else. Calling a helper for
    // one of those costs more than the work itself, and pushes its operand and result thru
    // memory where the compiler could otherwise keep them in a register.

    void EmitPushNil() {
        TrackUseDef(0, 1);
        SetToNil(cb, sp(0));
    }

    void EmitPopRef() {
        TrackUseDef(1, 0);
        GenDecRef(sp(1));
    }

    // These write an int over what may have been a reference, so in C, where we have to keep any
    // runtime type field correct ourselves, say so, and read the operand for what it is
    // rather than as an int: all the test asks is whether it is nil. Turning a reference into a
    // bool can drop it first, since the test does not need the value alive.
    void EmitBoolTest(string_view test, bool decref) {
        TrackUseDef(1, 1);
        if (decref) GenDecRef(sp(1));
        if (cpp) {
            append(cb, "    *(", sp(1), ") = Value((", sp(1), ")->bits() ", test, ");\n");
        } else {
            append(cb, "    { StackPtr _sp = ", sp(1), "; _sp->ival = _sp->ival ", test, ";",
                   SetType(RTT_INT), " }\n");
        }
    }

    void EmitIntToFloat() {
        TrackUseDef(1, 1);
        if (cpp) {
            append(cb, "    *(", sp(1), ") = Value((double)(", sp(1), ")->ival());\n");
        } else {
            append(cb, "    { StackPtr _sp = ", sp(1), "; double _d = (double)_sp->ival;"
                       " _sp->fval = _d;", SetType(RTT_FLOAT), " }\n");
        }
    }

    // A 64 bit constant, which only reaches us split in two because that is all an op argument
    // holds. For a float we write the bit pattern rather than a literal, since a decimal one
    // would not round trip exactly and not every C compiler we feed this to takes a hex float.
    void EmitPushConst64(bool isfloat, int64_t bits) {
        TrackUseDef(0, 1);
        string hex;
        to_string_hex(hex, (uint64_t)bits);
        if (cpp && isfloat) {
            // Nothing is gained by inlining here, the helper is already inline in C++.
            append(cb, "    RtPushFloat(", sp(), ", ", hex, "ULL);");
        } else if (cpp) {
            append(cb, "    *(", sp(), ") = Value((iint)", hex, "ULL);");
        } else {
            append(cb, "    { StackPtr _sp = ", sp(), "; _sp->ival = (long long)", hex, "ULL;",
                   SetType(isfloat ? RTT_FLOAT : RTT_INT), " }");
        }
    }

    // All that is left of an assert in the common case is the test; the reporting is a call.
    void EmitAssert(int defslots, int line, int fileidx, int stringidx) {
        TrackUseDef(1, defslots);
        if (cpp) {
            append(cb, "    if (!(", sp(1), ")->True()) vm.AssertFailed(", line, ", ", fileidx,
                   ", ", stringidx, ");\n");
        } else {
            append(cb, "    if (!(", sp(1), ")->ival) AssertFailed(vm, ", line, ", ", fileidx,
                   ", ", stringidx, ");\n");
        }
    }

    void DefineFunction(string &sd, bool label) {
        sd += "\n";
        auto sf_idx = f_function_idx;
        if (sf_idx < CODEGEN_SPECIAL_FUNCTION_ID_START)
            append(sd, "// ", Signature(*st.subfunctiontable[sf_idx]), "\n");
        append(sd, "static void fun_", sf_idx, "(VMRef vm, StackPtr psp) {\n");
        // NOTE: f_keepvars and f_regs_max are not known until end of codegen of function!
        // FIXME: don't emit array.
        // (there may be functions that don't use regs yet still refer to sp?)
        append(sd, "    Value regs[", std::max(1, f_regs_max), "];\n");
        if (!f_regs_max) append(sd, "    (void)regs;\n");
        if (f_keepvars) append(sd, "    Value keepvar[", f_keepvars, "];\n");
        if (numlocals) append(sd, "    Value locals[", numlocals, "];\n");
        if (f_uses_lval) append(sd, "    Value *lv = 0;\n");
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
        f_uses_lval = false;
        f_lval.clear();
        numlocals = 0;
        nlabel = 0;
        has_profile = false;
    }

    void Epilogue(string &sd, string_view custom_pre_init_name, uint64_t src_hash) {
        if (cpp) sd += "\nstatic";
        // c2mir turns a file scope declaration that has both `extern` and an initializer into a
        // mere import, dropping the definition, so for MIR we rely on the default external
        // linkage of a file scope object instead.
        else if (mir) sd += "\n";
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
                           (udt->ssuperclass ? udt->ssuperclass->idx : -1), ", ", udt->typeinfonon, ", ",
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
            // The offsets are what the reads we emit inline depend on; the sizes catch a field
            // being added or widened past the last one we read.
            sd += "    Entry(sizeof(Value), sizeof(VMBase), sizeof(RefObj), sizeof(LVector),\n"
                  "          (int)(long long)&((LVector *)0)->elems, sizeof(LString));\n";
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
            PushTemp();
        auto lab = EmitJumpIfUnwound(sf.parent->idx);
        for (int i = nretslots_norm; i < nretslots_unwind_max; i++)
            PopTemp();
        // Here we are emitting code executed only if we're falling thru,
        // so temp modify the tstack to match that.
        auto tstackbackup = tstack_size;
        EmitReturn(RET_ANY, nretslots_norm, -1, 0);
        EmitLabelDef(lab);
        tstack_size = tstackbackup;
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
            EmitCall(sf.idx, inw, outw);
            if (sf.returned_thru_to_max >= 0) {
                GenUnwind(sf, sf.returned_thru_to_max, outw);
            }
        } else {
            EmitDynDispatch(call.vtable_idx, inw - 1, inw, outw);
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
            PushTemp();
        }
    };

    bool ShouldDec(TypeLT typelt) {
        return IsRefNil(typelt.type->t) && typelt.lt == LT_KEEP;
    }

    // Field for the result and the operands in the C backend, accessor for them in the C++ one,
    // and the operator itself. Integer division and both kinds of modulo are absent: they check
    // for div by zero.
    // A comparison writes an int over what may have been a float, so in C, where we would have to
    // keep any runtime type field correct ourselves, only do this without one. The C++ backend
    // goes thru Value, which maintains it either way.
    bool SimpleBinOpC(bool isfloat, MathOp op, string_view &res, string_view &arg,
                      string_view &cop) {
        if (!cpp && RTT_ENABLED) return false;
        if (op == MOP_MOD || (op == MOP_DIV && !isfloat)) return false;
        static const char *cops[] = { "+", "-", "*", "/", "%",
                                      "<", ">", "<=", ">=", "==", "!=" };
        cop = cops[op];
        arg = isfloat ? "fval" : "ival";
        // A comparison produces an int whatever it compared.
        res = isfloat && op < MOP_LT ? "fval" : "ival";
        return true;
    }

    void GenBinOpSlot(string_view res, string_view arg, string_view cop, string_view dest,
                      string_view a, string_view b) {
        if (cpp) {
            // Value's constructor picks the runtime type up from the expression.
            append(cb, "    *(", dest, ") = Value((", a, ")->", arg, "() ", cop, " (", b, ")->",
                   arg, "());\n");
        } else {
            append(cb, "    (", dest, ")->", res, " = (", a, ")->", arg, " ", cop, " (", b, ")->",
                   arg, ";\n");
        }
    }

    // Calling a helper for something this small costs more than the work itself, and pushes
    // both operands and the result thru memory where the compiler could otherwise keep them in
    // registers, so emit the operator directly instead. Either way this takes two operands off
    // the stack and leaves the result.
    void GenSimpleBinOp(bool isfloat, MathOp op, string_view opname) {
        TrackUseDef(2, 1);
        string_view res, arg, cop;
        if (!SimpleBinOpC(isfloat, op, res, arg, cop)) {
            append(cb, "    ", opname, "(vm, ", sp(), ");\n");
            return;
        }
        GenBinOpSlot(res, arg, cop, sp(2), sp(2), sp(1));
    }

    // The struct versions are the same operator, once per slot of the struct. VV has one on
    // both sides, VS a struct and a scalar. SV, a scalar on the left, is left to the helper: it
    // writes its results over the slot its left operand is in, so it does not unroll as
    // directly, and nothing we have measured emits it.
    bool GenVecBinOp(bool isfloat, bool withscalar, bool leftisvec, MathOp op, int len) {
        if (withscalar && !leftisvec) return false;
        string_view res, arg, cop;
        if (!SimpleBinOpC(isfloat, op, res, arg, cop)) return false;
        for (int j = 0; j < len; j++) {
            auto a = sp(withscalar ? len + 1 - j : len * 2 - j);
            GenBinOpSlot(res, arg, cop, a, a, withscalar ? sp(1) : sp(len - j));
        }
        return true;
    }

    // Comparing two structs is a compare per slot, on the raw bits the same way a helper would.
    // Which is what it has to be: the slots of a struct do not all hold the same type.
    void GenStructCompare(bool eq, int len) {
        auto field = cpp ? "bits()" : "ival";
        append(cb, "    { long long _c = ", eq ? "1" : "0", ";\n");
        for (int j = 0; j < len; j++) {
            append(cb, "    _c = _c ", eq ? "&&" : "||", " (", sp(len * 2 - j), ")->", field, " ",
                   eq ? "==" : "!=", " (", sp(len - j), ")->", field, ";\n");
        }
        // Only written after all the reads, since the result lands in the first slot of the left
        // hand side.
        if (cpp) {
            append(cb, "    *(", sp(len * 2), ") = Value(_c != 0);\n    }\n");
        } else {
            append(cb, "    { StackPtr _sp = ", sp(len * 2), "; _sp->ival = _c;",
                   SetType(RTT_INT), " }\n    }\n");
        }
    }

    // Reading a field is a load at a constant offset from the object, whose fields sit right
    // behind its header. Copies the whole Value, so it carries any runtime type field along.
    void GenPushField(int offset) {
        TrackUseDef(1, 1);
        if (cpp) {
            append(cb, "    *(", sp(1), ") = (", sp(1), ")->oval()->At(", offset, ");\n");
        } else {
            append(cb, "    *(", sp(1), ") = ((Value *)((RefObj *)(", sp(1), ")->ref + 1))[",
                   offset, "];\n");
        }
    }

    // Same, once per slot the struct field occupies. The object is only needed to find them, so
    // it is read out of the stack slot the first one lands in before that gets overwritten.
    void GenPushFieldStruct(int offset, int fwidth) {
        TrackUseDef(1, fwidth);
        append(cb, "    {\n    ", cpp ? "auto " : "RefObj *", "_o = (", sp(1), ")->",
               cpp ? "oval()" : "ref", ";\n");
        for (int i = 0; i < fwidth; i++) {
            GenValueCopy(cb, sp(1 - i), cpp ? cat("_o->Elems() + ", offset + i)
                                            : cat("((Value *)(_o + 1)) + ", offset + i));
        }
        cb += "    }\n";
    }

    // Where the stack top is is something we track statically, so popping a slot needs no code
    // at all, just the bookkeeping.
    void GenPopSlot() { TrackUseDef(1, 0); }

    // The C expression for how many times a loop over this value runs, which for a vector or a
    // string comes out of the object itself, see the mirrors of those in Prologue.
    string LenOf(ValueType itertype, string_view slot) {
        switch (itertype) {
            case V_INT:
                return cat("(", slot, ")->", cpp ? "ival()" : "ival");
            case V_VECTOR:
                return cpp ? cat("(", slot, ")->vval()->len")
                           : cat("((LVector *)(", slot, ")->ref)->len");
            case V_STRING:
                return cpp ? cat("(", slot, ")->sval()->len")
                           : cat("((LString *)(", slot, ")->ref)->len");
            default:
                assert(false);
                return {};
        }
    }

    // The loop condition is an increment and a compare, small enough to be worth not calling for
    // the same reasons as GenSimpleBinOp. The counter stays an int here, so this needs no care
    // around a runtime type field.
    int GenForCond(ValueType itertype) {
        // Reads the counter and the object being iterated, and leaves both for the body.
        TrackUseDef(2, 2);
        auto lab = Label();
        auto len = LenOf(itertype, sp(1));
        if (cpp) {
            append(cb, "    *(", sp(2), ") = Value((", sp(2), ")->ival() + 1);\n");
            append(cb, "    if (!((", sp(2), ")->ival() < ", len, ")) goto block", lab, ";\n");
        } else {
            append(cb, "    (", sp(2), ")->ival = (", sp(2), ")->ival + 1;\n");
            append(cb, "    if (!((", sp(2), ")->ival < ", len, ")) goto block", lab, ";\n");
        }
        return lab;
    }

    // Both of these copy the loop counter to the top of the stack, which is a whole Value and
    // so carries any runtime type field with it.
    void GenForCounter(int useslots, int defslots) {
        TrackUseDef(useslots, defslots);
        append(cb, "    *(", sp(0), ") = *(", sp(2), ");\n");
    }

    // The element the loop is on, at the counter below the object being iterated. The loop
    // condition already established the counter is in range, so this needs no check.
    // The bitmask says which slots of the element hold a reference the loop now owns.
    void GenForElem(bool isstring, int defslots, int bitmask) {
        TrackUseDef(2, defslots);
        // Everything but the counter and the object being iterated is the element.
        auto width = defslots - 2;
        auto idx = cat("(", sp(2), ")->", cpp ? "ival()" : "ival");
        if (isstring) {
            auto data = cpp ? cat("((unsigned char *)(", sp(1), ")->sval()->data())")
                            : cat("LSTRING_DATA((LString *)(", sp(1), ")->ref)");
            if (cpp) {
                append(cb, "    *(", sp(0), ") = Value((iint)", data, "[", idx, "]);\n");
            } else {
                append(cb, "    { StackPtr _sp = ", sp(0), "; _sp->ival = ", data, "[", idx, "];",
                       SetType(RTT_INT), " }\n");
            }
            return;
        }
        auto elems = cpp ? cat("(", sp(1), ")->vval()->Elems()")
                         : cat("((LVector *)(", sp(1), ")->ref)->elems");
        if (width > 1) {
            // A struct element is the same load per slot it occupies, at the width the vector
            // holds them at, which is what the element type says it is.
            append(cb, "    {\n    Value *_e = ", elems, " + ", idx, " * ", width, ";\n");
            for (int i = 0; i < width; i++) GenValueCopy(cb, sp(-i), cat("_e + ", i));
            cb += "    }\n";
            for (int i = 0; i < width; i++) if ((1 << i) & bitmask) GenIncRef(sp(-i));
            return;
        }
        GenValueCopy(cb, sp(0), cat(elems, " + ", idx));
        if (bitmask & 1) GenIncRef(sp(0));
    }

    // Reading an element out of a vector or a string. Unlike the loop above the index is
    // arbitrary, so it needs the range check, whose failure path stays a call. The object is
    // read out into a local first, since the element lands in the slot it came from.
    // Indexing a vector with a struct: every component of it but the first steps into a nested
    // vector, and the first one lands on the element. They sit above the vector on the stack and
    // are used back to front, see VM::GrabIndex, with one range check per level.
    void GenPushIdxNested(int levels, int width) {
        // The vector plus one index per level it steps thru, replaced by the element.
        TrackUseDef(levels + 1, width);
        auto vec = sp(levels + 1);
        if (cpp) {
            append(cb, "    {\n    auto _o = (", vec, ")->vval();\n    iint _i;\n");
            for (int j = levels - 1; j >= 0; j--) {
                append(cb, "    _i = (", sp(levels - j), ")->ival();\n");
                append(cb, "    if ((uint64_t)_i >= (uint64_t)_o->len)"
                           " vm.IDXErr(_i, _o->len, _o);\n");
                if (j) append(cb, "    _o = _o->AtS(_i).vval();\n");
            }
            for (int i = 0; i < width; i++)
                GenValueCopy(cb, sp(levels + 1 - i), cat("_o->Elems() + _i * ", width, " + ", i));
        } else {
            append(cb, "    {\n    LVector *_o = (LVector *)(", vec,
                   ")->ref;\n    long long _i;\n");
            for (int j = levels - 1; j >= 0; j--) {
                append(cb, "    _i = (", sp(levels - j), ")->ival;\n");
                append(cb, "    if ((unsigned long long)_i >= (unsigned long long)_o->len)"
                           " IDXErr(vm, _i, _o->len, &_o->ro);\n");
                if (j) append(cb, "    _o = (LVector *)(_o->elems + _i)->ref;\n");
            }
            for (int i = 0; i < width; i++)
                GenValueCopy(cb, sp(levels + 1 - i), cat("_o->elems + _i * ", width, " + ", i));
        }
        cb += "    }\n";
    }

    void GenPushIdx(bool str, int width) {
        // The object and the index it is subscripted with, replaced by the element.
        TrackUseDef(2, width);
        // A string index may read the terminating 0-byte, one past its length.
        auto bound = str ? "_o->len + 1" : "_o->len";
        if (cpp) {
            append(cb, "    {\n    auto _o = (", sp(2), ")->", str ? "sval()" : "vval()",
                   "; auto _i = (", sp(1), ")->ival();\n");
            append(cb, "    if ((uint64_t)_i >= (uint64_t)(", bound, ")) vm.IDXErr(_i, ", bound,
                   ", _o);\n");
            if (str) {
                append(cb, "    *(", sp(2), ") = Value((iint)((unsigned char *)"
                           "_o->data())[_i]);\n");
            } else {
                for (int i = 0; i < width; i++)
                    GenValueCopy(cb, sp(2 - i), cat("_o->Elems() + _i * ", width, " + ", i));
            }
        } else {
            append(cb, "    {\n    ", str ? "LString" : "LVector", " *_o = (",
                   str ? "LString" : "LVector", " *)(", sp(2), ")->ref; long long _i = (", sp(1),
                   ")->ival;\n");
            append(cb, "    if ((unsigned long long)_i >= (unsigned long long)(", bound,
                   ")) IDXErr(vm, _i, ", bound, ", &_o->ro);\n");
            if (str) {
                append(cb, "    { StackPtr _sp = ", sp(2), "; _sp->ival = LSTRING_DATA(_o)[_i];",
                       SetType(RTT_INT), " }\n");
            } else {
                for (int i = 0; i < width; i++)
                    GenValueCopy(cb, sp(2 - i), cat("_o->elems + _i * ", width, " + ", i));
            }
        }
        cb += "    }\n";
    }

    void GenPop(TypeLT typelt) {
        if (IsStruct(typelt.type->t)) {
            if (typelt.type->t == V_STRUCT_R) {
                // TODO: alternatively call a single helper with a list or bitmask?
                // See BitMaskForRefStuct.
                for (int j = typelt.type->udt->numslots - 1; j >= 0; j--) {
                    if (IsRefNil(FindSlot(*typelt.type->udt, j)->type->t)) EmitPopRef();
                    else GenPopSlot();
                }
            } else {
                auto numslots = typelt.type->udt->numslots;
                TrackUseDef(numslots, 0);
                append(cb, "    RtPopV(", sp(), ", ", numslots, ");\n");
            }
        } else {
            if (ShouldDec(typelt)) EmitPopRef(); else GenPopSlot();
        }
    }

    void GenDup(TypeLT tlt) {
        TrackUseDef(1, 2);
        GenValueCopy(cb, sp(0), sp(1));
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

    // The modifiers that read/modify/write thru the address in f_lval with a single operator,
    // the same deal as GenSimpleBinOp. The ones absent here either check for division by zero,
    // decrement a reference (which can free it), or are not scalar.
    static bool SimpleLvalOpC(LvalOp op, string_view &fld, string_view &cop) {
        switch (op) {
            case LV_IADD:   fld = "ival"; cop = "+";  return true;
            case LV_ISUB:   fld = "ival"; cop = "-";  return true;
            case LV_IMUL:   fld = "ival"; cop = "*";  return true;
            case LV_BINAND: fld = "ival"; cop = "&";  return true;
            case LV_BINOR:  fld = "ival"; cop = "|";  return true;
            case LV_XOR:    fld = "ival"; cop = "^";  return true;
            case LV_FADD:   fld = "fval"; cop = "+";  return true;
            case LV_FSUB:   fld = "fval"; cop = "-";  return true;
            case LV_FMUL:   fld = "fval"; cop = "*";  return true;
            case LV_FDIV:   fld = "fval"; cop = "/";  return true;
            default: return false;
        }
    }

    // What the modifier takes off the stack: the increment/decrement ops work in place and have
    // no right hand side at all, the ones that write or operate on a whole struct consume every
    // slot of it, and the rest a single value, the vector-with-scalar ops included since their
    // right hand side is that scalar. None of them leave anything behind, since the address they
    // work on lives in a local rather than on the stack.
    static int LvalModifierUses(LvalOp op, int width) {
        switch (op) {
            case LV_IPP:
            case LV_IMM:
            case LV_FPP:
            case LV_FMM:
                return 0;
            case LV_WRITEV:
            case LV_WRITEREFV:
                return width;
            default:
                return op >= LV_IVVADD && op <= LV_FVVMOD ? width : 1;
        }
    }

    void GenLvalModifier(LvalOp op, TypeRef type) {
        auto width = ValWidth(type);
        TrackUseDef(LvalModifierUses(op, width), 0);
        auto &lval = f_lval;
        string_view fld, cop;
        if (op == LV_WRITE) {
            GenValueCopy(cb, lval, sp(1));
        } else if (op == LV_WRITEREF) {
            // Whatever was there loses a reference to make way for what is written over it.
            GenDecRef(lval);
            GenValueCopy(cb, lval, sp(1));
        } else if (op == LV_WRITEV || op == LV_WRITEREFV) {
            // Same copy, one per slot of the struct being written, preceded by a decrement for
            // each of those slots that holds a reference, which the bitmask says which are.
            if (op == LV_WRITEREFV) {
                auto bitmask = BitMaskForRefStuct(type);
                for (int i = 0; i < width; i++)
                    if ((1 << i) & bitmask) GenDecRef(cat(lval, " + ", i));
            }
            for (int i = 0; i < width; i++)
                GenValueCopy(cb, cat(lval, " + ", i), sp(width - i));
        } else if (SimpleLvalOpC(op, fld, cop)) {
            // These keep the type of what is already in the slot, so no care is needed around a
            // runtime type field.
            if (cpp) {
                append(cb, "    *(", lval, ") = Value((", lval, ")->", fld, "() ", cop, " (",
                       sp(1), ")->", fld, "());\n");
            } else {
                append(cb, "    (", lval, ")->", fld, " = (", lval, ")->", fld, " ", cop, " (",
                       sp(1), ")->", fld, ";\n");
            }
        } else if (op == LV_IPP || op == LV_IMM || op == LV_FPP || op == LV_FMM) {
            auto f = op == LV_IPP || op == LV_IMM ? "ival" : "fval";
            auto c = op == LV_IPP || op == LV_FPP ? "+" : "-";
            if (cpp) {
                append(cb, "    *(", lval, ") = Value((", lval, ")->", f, "() ", c,
                       " 1);\n");
            } else {
                append(cb, "    (", lval, ")->", f, " = (", lval, ")->", f, " ", c,
                       " 1;\n");
            }
        } else {
            // What is left is one helper per operator per type, too many to name individually
            // here, and the one we need is computed from the operator anyway.
            static const char *lvnames[] = {
                "RtLvDup", "RtLvDupV",
                "RtLvWrite", "RtLvWriteRef", "RtLvWriteV", "RtLvWriteRefV",
                "RtLvIAdd", "RtLvISub", "RtLvIMul", "RtLvIDiv", "RtLvIMod",
                "RtLvBinAnd", "RtLvBinOr", "RtLvXor", "RtLvAsl", "RtLvAsr",
                "RtLvFAdd", "RtLvFSub", "RtLvFMul", "RtLvFDiv", "RtLvFMod",
                "RtLvIvvAdd", "RtLvIvvSub", "RtLvIvvMul", "RtLvIvvDiv", "RtLvIvvMod",
                "RtLvFvvAdd", "RtLvFvvSub", "RtLvFvvMul", "RtLvFvvDiv", "RtLvFvvMod",
                "RtLvIvsAdd", "RtLvIvsSub", "RtLvIvsMul", "RtLvIvsDiv", "RtLvIvsMod",
                "RtLvFvsAdd", "RtLvFvsSub", "RtLvFvsMul", "RtLvFvsDiv", "RtLvFvsMod",
                "RtLvSAdd",
                "RtLvIPp", "RtLvIMm", "RtLvFPp", "RtLvFMm",
            };
            append(cb, "    ", lvnames[op], "(vm, ", sp(), ", ", lval);
            if (IsStruct(type->t)) append(cb, ", ", width);
            cb += ");\n";
        }
    }

    void GenAssignBasic(const SpecIdent &sid) {
        TakeTemp(1, true);
        GenLvalVar(sid, 0);
        auto op = AssignBaseOp({ sid });
        GenLvalModifier(op, sid.type);
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
                EmitLvalField(sfield.slot + offset);
            } else {
                GenAssignLvalRec(dot->child, sfield.slot + offset, take_temp, type);
            }
        } else if (auto indexing = Is<Indexing>(lval)) {
            if (IsStruct(indexing->object->exptype->t)) {
                // This generates an LVAL producing OP which is then indexed below and turned into another LVAL!
                GenAssignLvalRec(indexing->object, offset, take_temp, type);
                Gen(indexing->index, 1);
                TakeTemp(1, true);
            } else {
                Gen(indexing->object, 1);
                Gen(indexing->index, 1);
                TakeTemp(take_temp + 2, true);
            }
            switch (indexing->object->exptype->t) {
                case V_VECTOR:
                    if (indexing->index->exptype->t == V_INT) {
                        EmitLvalVectorIndex(offset, ValWidth(indexing->object->exptype->Element()));
                    } else {
                        assert(IsStruct(indexing->index->exptype->t));
                        auto width = ValWidth(indexing->index->exptype);
                        EmitLvalIndex("RtLvalIndexVecV", false, { offset, width }, width + 1);
                    }
                    break;
                case V_CLASS:
                    assert(indexing->index->exptype->t == V_INT &&
                           indexing->object->exptype->udt->sametype->Numeric());
                    EmitLvalIndex("RtLvalIndexClass", false, { offset }, 2);
                    assert(!IsStruct(type->t));
                    break;
                case V_STRUCT_R:
                case V_STRUCT_S:
                    assert(indexing->index->exptype->t == V_INT &&
                           indexing->object->exptype->udt->sametype->Numeric());
                    EmitLvalIndex("RtLvalIndexStruct", true, { offset,
                                                indexing->object->exptype->udt->numslots }, 1);
                    assert(!IsStruct(type->t));
                    break;
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

    void GenAssign(const Node *lval, LvalOp lvalop, size_t retval,
                   const Node *rhs, int take_temp, bool post) {
        assert(node_context.back()->exptype->NumValues() >= retval);
        auto type = lval->exptype;
        if (lvalop >= LV_IADD && lvalop <= LV_IMOD) {
            if (type->t == V_INT) {
            } else if (type->t == V_FLOAT) {
                lvalop = LvalOp(lvalop + (LV_FADD - LV_IADD));
            } else if (type->t == V_STRING) {
                assert(lvalop == LV_IADD); lvalop = LV_SADD;
            } else if (type->t == V_STRUCT_S) {
                auto sub = type->udt->sametype;
                bool withscalar = IsScalar(rhs->exptype->t);
                if (sub->t == V_INT) {
                    lvalop = LvalOp(lvalop + ((withscalar ? LV_IVSADD : LV_IVVADD) - LV_IADD));
                } else if (sub->t == V_FLOAT) {
                    lvalop = LvalOp(lvalop + ((withscalar ? LV_FVSADD : LV_FVVADD) - LV_IADD));
                } else assert(false);
            } else {
                assert(false);
            }
        } else if (lvalop >= LV_IPP && lvalop <= LV_IMM) {
            if (type->t == V_FLOAT) lvalop = LvalOp(lvalop + (LV_FPP - LV_IPP));
            else assert(type->t == V_INT);
        }
        if (rhs) Gen(rhs, 1);
        GenAssignLvalRec(lval, 0, take_temp, type);
        if (!post) {
            GenLvalModifier(lvalop, type);
        }
        if (retval) {
            // FIXME: it seems these never need a refcount increase because they're always
            // borrowed? Be good to assert that somehow.
            if (IsStruct(type->t)) {
                auto width = ValWidth(type);
                TrackUseDef(0, width);
                append(cb, "    RtLvDupV(", sp(), ", ", f_lval, ", ", width, ");\n");
            } else {
                TrackUseDef(0, 1);
                GenValueCopy(cb, sp(0), f_lval);
            }
        }
        if (post) {
            GenLvalModifier(lvalop, type);
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
            // We still need this helper for += and it is marginally more efficient here too.
            TrackUseDef(2, 1);
            append(cb, "    RtSAdd(vm, ", sp(), ");\n");
        } else {
            auto nstrs = (int)strs.size();
            TrackUseDef(nstrs, 1);
            append(cb, "    RtStrConcatN(vm, ", sp(), ", ", nstrs, ");\n");
        }
    }

    void GenMathOp(const BinOp *n, size_t retval, MathOp op) {
        Gen(n->left, retval);
        Gen(n->right, retval);
        if (retval) GenMathOp(n->left->exptype, n->right->exptype, n->exptype, op);
    }

    // The type specialized helpers below come one per MathOp, in that order, so the name of each
    // is the prefix for the types it works on followed by the name of the operator.
    static string MathOpName(string_view prefix, MathOp op) {
        static const char *ops[] = { "Add", "Sub", "Mul", "Div", "Mod",
                                     "Lt", "Gt", "Le", "Ge", "Eq", "Ne" };
        return cat("Rt", prefix, ops[op]);
    }

    void GenMathOp(TypeRef ltype, TypeRef rtype, TypeRef ptype, MathOp op) {
        TakeTemp(2, true);
        // Have to check right and left because comparison ops generate ints for node
        // overall.
        if (rtype->t == V_INT && ltype->t == V_INT) {
            GenSimpleBinOp(false, op, MathOpName("I", op));
        } else if (rtype->t == V_FLOAT && ltype->t == V_FLOAT) {
            GenSimpleBinOp(true, op, MathOpName("F", op));
        } else if (rtype->t == V_STRING && ltype->t == V_STRING) {
            // Nillable version handled below.
            TrackUseDef(2, 1);
            append(cb, "    ", MathOpName("S", op), "(vm, ", sp(), ");\n");
        } else if ((rtype->t == V_FUNCTION && ltype->t == V_FUNCTION)) {
            assert(op == MOP_EQ || op == MOP_NE);
            TrackUseDef(2, 1);
            append(cb, "    ", MathOpName("L", op), "(", sp(), ");\n");
        } else if ((rtype->t == V_TYPEID && ltype->t == V_TYPEID)) {
            assert(op == MOP_EQ || op == MOP_NE);
            GenSimpleBinOp(false, op, MathOpName("I", op));
        } else {
            if (op >= MOP_EQ) {  // EQ/NEQ
                if (IsStruct(ltype->t)) {
                    // Comparing two structs is one compare per slot, so this never becomes a
                    // call at all.
                    auto width = ValWidth(ltype);
                    TrackUseDef(width * 2, 1);
                    GenStructCompare(op == MOP_EQ, width);
                } else {
                    assert(IsRefNil(ltype->t) &&
                           IsRefNil(rtype->t));
                    if ((ltype->t == V_NIL && ltype->sub->t == V_STRING) ||
                        (rtype->t == V_NIL && rtype->sub->t == V_STRING)) {
                        TrackUseDef(2, 1);
                        append(cb, "    ", MathOpName("Sn", op), "(", sp(), ");\n");
                    } else {
                        TrackUseDef(2, 1);
                        append(cb, "    ", MathOpName("A", op), "(", sp(), ");\n");
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
                assert(sub->t == V_INT || sub->t == V_FLOAT);
                auto isint = sub->t == V_INT;
                auto prefix = isint ? (withscalar ? (leftisvec ? "Ivs" : "Siv") : "Ivv")
                                    : (withscalar ? (leftisvec ? "Fvs" : "Sfv") : "Fvv");
                TrackUseDef(inw, outw);
                // Most of these are the same operator once per slot of the struct, which makes
                // them a fixed number of the ones above rather than a call that loops.
                if (!GenVecBinOp(!isint, withscalar, leftisvec, op, width))
                    append(cb, "    ", MathOpName(prefix, op), "(vm, ", sp(), ", ", width,
                           ");\n");
            }
        }
    }

    void GenBitOp(const BinOp *n, size_t retval, string_view opname) {
        Gen(n->left, retval);
        Gen(n->right, retval);
        if (retval) {
            TakeTemp(2, false);
            TrackUseDef(2, 1);
            append(cb, "    ", opname, "(", sp(), ");\n");
        }
    }

    LvalOp AssignBaseOp(TypeLT typelt) {
        auto dec = ShouldDec(typelt);
        return IsStruct(typelt.type->t)
            ? (dec ? LV_WRITEREFV : LV_WRITEV)
            : (dec ? LV_WRITEREF : LV_WRITE);
    }

    void GenLvalVar(const SpecIdent &sid, int offset) {
        if (sid.used_as_freevar)
            EmitLvalGlobal(sid.Idx() + offset, sid.type);
        else
            EmitLvalLocal(sid.Idx() + offset, sid.type);
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
                TrackUseDef(swidth, fwidth);
                append(cb, "    RtPushFieldV2V(", sp(), ", ", offset, ", ", fwidth, ", ", swidth,
                       ");\n");
            } else {
                TrackUseDef(swidth, 1);
                append(cb, "    RtPushFieldV(", sp(), ", ", offset, ", ", swidth, ");\n");
            }
        } else {
            if (IsStruct(ftype->t)) {
                GenPushFieldStruct(offset, fwidth);
            } else {
                GenPushField(offset);
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
                        GenPushIdx(false, elemwidth);
                    } else {
                        assert(IsStruct(index->exptype->t));
                        GenPushIdxNested(ValWidth(index->exptype), elemwidth);
                    }
                } else {
                    // We're indexing a sub-part of the element.
                    if (index->exptype->t == V_INT) {
                        if (elemwidth == 1) {
                            TrackUseDef(inw, struct_elem_sub_width);
                            append(cb, "    RtIndexVecSub(vm, ", sp(), ", ",
                                   struct_elem_sub_offset, ");\n");
                        } else {
                            TrackUseDef(inw, struct_elem_sub_width);
                            append(cb, "    RtIndexVecSubV(vm, ", sp(), ", ",
                                   struct_elem_sub_width, ", ", struct_elem_sub_offset, ");\n");
                        }
                    } else {
                        assert(IsStruct(index->exptype->t));
                        TrackUseDef(inw, struct_elem_sub_width);
                        append(cb, "    RtIndexVecNestSubV(vm, ", sp(), ", ",
                               ValWidth(index->exptype), ", ", struct_elem_sub_width, ", ",
                               struct_elem_sub_offset, ");\n");
                    }
                }
                break;
            }
            case V_STRUCT_S: {
                auto width = ValWidth(object->exptype);
                assert(index->exptype->t == V_INT && object->exptype->udt->sametype->Numeric());
                TrackUseDef(width + 1, 1);
                append(cb, "    RtIndexStruct(vm, ", sp(), ", ", width, ");\n");
                break;
            }
            case V_STRING:
                assert(index->exptype->t == V_INT);
                GenPushIdx(true, 1);
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
    if (retval) { cg.EmitPushNil(); }
}

void IntConstant::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    if (integer == (int)integer) {
        cg.EmitPushInt((int)integer);
    } else {
        cg.EmitPushConst64(false, integer);
    }
}

void FloatConstant::Generate(CodeGen &cg, size_t retval) const {
    if (retval) { cg.GenFloat(flt); };
}

void StringConstant::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    cg.stringtable.push_back(str);
    cg.EmitPushStr((int)cg.stringtable.size() - 1);
}

void DefaultVal::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    cg.EmitPushNil();
}

void IdentRef::Generate(CodeGen &cg, size_t retval) const {
    cg.GenPushVar(retval, sid->type, sid->Idx(), sid->used_as_freevar);
}

void FreeVarRef::Generate(CodeGen &, size_t) const {
    assert(false);  // Should be replaced by typechecker.
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
        // The marker is the var right after this one.
        auto lab = cg.EmitJumpIfSetThisFrame(true, sfield.slot + ValWidth(sfield.type));
        cg.Gen(child, 1);
        cg.GenPushVar(1, this_sid->type, this_sid->Idx(), this_sid->used_as_freevar);
        cg.TakeTemp(1, true);
        cg.EmitLvalField(sfield.slot);
        cg.GenLvalModifier(cg.AssignBaseOp({ sfield.type, LT_KEEP }), sfield.type);
        cg.EmitLabelDef(lab);
    }
    if (!retval) return;
    cg.EmitPushNil();
}

void Static::Generate(CodeGen &cg, size_t retval) const {
    if (frame) {
        assert(sid->used_as_freevar);  // Since we'll access these from the freevar buf.
        // The marker is the var right after this one.
        auto lab = cg.EmitJumpIfSetThisFrame(false, sid->Idx() + ValWidth(sid->type));
        cg.Gen(child, 1);
        cg.GenAssignBasic(*sid);
        cg.EmitLabelDef(lab);
    }
    if (!retval) return;
    cg.EmitPushNil();
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
    if (tsids.size() == 1 && tsids[0].sid->constprop) {
        return;
    }
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
    cg.GenAssign(left, LV_IADD, retval, right, 1, false);
}
void MinusEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LV_ISUB, retval, right, 1, false);
}
void MultiplyEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LV_IMUL, retval, right, 1, false);
}
void DivideEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LV_IDIV, retval, right, 1, false);
}
void ModEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LV_IMOD, retval, right, 1, false);
}
void AndEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LV_BINAND, retval, right, 1, false);
}
void OrEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LV_BINOR, retval, right, 1, false);
}
void XorEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LV_XOR, retval, right, 1, false);
}
void ShiftLeftEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LV_ASL, retval, right, 1, false);
}
void ShiftRightEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LV_ASR, retval, right, 1, false);
}

void PostDecr::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, LV_IMM, retval, nullptr, 0, true); }
void PostIncr::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, LV_IPP, retval, nullptr, 0, true); }
void PreDecr ::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, LV_IMM,  retval, nullptr, 0, false); }
void PreIncr ::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, LV_IPP,  retval, nullptr, 0, false); }

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
        case V_INT:
            cg.TrackUseDef(1, 1);
            append(cg.cb, "    RtIUMinus(", cg.sp(), ");\n");
            break;
        case V_FLOAT:
            cg.TrackUseDef(1, 1);
            append(cg.cb, "    RtFUMinus(", cg.sp(), ");\n");
            break;
        case V_STRUCT_S: {
            auto isint = ctype->udt->sametype->t == V_INT;
            auto inw = ValWidth(ctype);
            cg.TrackUseDef(inw, inw);
            append(cg.cb, "    ", isint ? "RtIvUMinus" : "RtFvUMinus", "(vm, ", cg.sp(), ", ",
                   inw, ");\n");
            break;
        }
        default: assert(false);
    }
}

void BitAnd    ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, "RtBinAnd"); }
void BitOr     ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, "RtBinOr"); }
void Xor       ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, "RtXor"); }
void ShiftLeft ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, "RtAsl"); }
void ShiftRight::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, "RtAsr"); }

void Negate::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, false);
    cg.TrackUseDef(1, 1);
    append(cg.cb, "    RtNeg(", cg.sp(), ");\n");
}

void ToFloat::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, false);
    cg.EmitIntToFloat();
}

void ToString::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, true);
    switch (child->exptype->t) {
        case V_STRUCT_R:
        case V_STRUCT_S: {
            // TODO: can also roll these into A2S?
            cg.EmitStructToString(cg.GetTypeTableOffset(child->exptype),
                                  ValWidth(child->exptype), child->exptype);
            break;
        }
        default: {
            cg.TrackUseDef(1, 1);
            append(cg.cb, "    RtToString(vm, ", cg.sp(), ", (type_elem_t)",
                   (int)cg.GetTypeTableOffset(child->exptype->ElementIfNil()), ");\n");
            break;
        }
    }
}

void ToBool::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, false);
    cg.EmitBoolTest("!= 0", cg.ShouldDec(TypeLT(*child, 0)));
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
                    // TODO: alternatively call a single helper with a list or bitmask?
                    // See BitMaskForRefStuct.
                    for (int j = 0; j < type->udt->numslots; j++) {
                        auto stype = FindSlot(*type->udt, j)->type;
                        if (IsRefNil(stype->t)) {
                            cg.EmitIncRef(stack_offset + type->udt->numslots - 1 - j);
                        }
                    }
                } else {
                    cg.EmitIncRef(stack_offset);
                }
            }
            if (decref & (1LL << i)) {
                assert(IsRefNil(type->t));
                if (type->t == V_STRUCT_R) {
                    // TODO: alternatively call a single helper with a list or bitmask?
                    // See BitMaskForRefStuct.
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
        cg.EmitPushFun(sf->idx);
    } else {
        cg.EmitPushFun(CODEGEN_SPECIAL_FUNCTION_ID_DUMMY);
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
            cg.EmitAssert(!!retval, child->line.line, child->line.fileidx,
                          (int)cg.stringtable.size());
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
    auto start = cg.tstack_size;
    for (auto [i, c] : enumerate(children)) {
        cg.Gen(c, 1);
        if ((IsStruct(c->exptype->t) ||
             nf->args[i].flags & NF_PUSHVALUEWIDTH) &&
            !Is<DefaultVal>(c)) {
            // FIXME: struct variable size.
            cg.EmitPushInt(ValWidth(c->exptype));
            cg.temptypestack.push_back({ type_int, LT_ANY });
            numstructs++;
        }
    }
    auto inw = int(cg.tstack_size - start);
    size_t nargs = children.size();
    cg.TakeTemp(nargs + numstructs, true);
    assert(nargs == nf->args.size() && (nf->fun.fnargs < 0 || nargs <= 7));
    cg.EmitNativeCall(nf->fun.fnargs >= 0 ? (int)nargs : -1, nf, !nf->retvals.empty(), inw,
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
    cg.EmitCallValue(arg_width + 1, ValWidthMulti(exptype, sf->returntype->NumValues()));
    if (sf->reqret) {
        if (!retval) cg.GenPop({ exptype, lt });
    } else {
        cg.Dummy(retval);
    }
}

void Block::Generate(CodeGen &cg, size_t retval) const {
    auto tstack_start = cg.tstack_size;
    (void)tstack_start;
    for (auto c : children) {
        cg.GenStatDebug(c);
        if (c != children.back()) {
            // Not the last element.
            cg.Gen(c, 0);
            assert(tstack_start == cg.tstack_size);
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
    auto lab = cg.EmitJumpFail(!!retval);
    if (retval) cg.GenPopSlot();
    cg.Gen(right, retval);
    if (retval) cg.TakeTemp(1, false);
    cg.EmitLabelDef(lab);
}

void Or::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(left, 1);
    cg.TakeTemp(1, false);
    auto lab = cg.EmitJumpNoFail(!!retval);
    if (retval) cg.GenPopSlot();
    cg.Gen(right, retval);
    if (retval) cg.TakeTemp(1, false);
    cg.EmitLabelDef(lab);
}

void Not::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (retval) {
        cg.TakeTemp(1, false);
        cg.EmitBoolTest("== 0", false);
    }
}

void IfThen::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    auto lab = cg.EmitJumpFail(0);
    assert(!retval); (void)retval;
    cg.Gen(truepart, 0);
    cg.EmitLabelDef(lab);
}

void IfElse::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    auto lab = cg.EmitJumpFail(0);
    CodeGen::BlockStack bs(cg.tstack_size);
    bs.Start();
    cg.Gen(truepart, retval);
    bs.End();
    if (retval) cg.TakeTemp(retval, true);
    auto lab2 = cg.EmitJump();
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
    cg.continues.push_back(loopback);
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    auto jumpout = cg.EmitJumpFail(0);
    auto break_level = cg.breaks.size();
    cg.Gen(wbody, 0);
    cg.loops.pop_back();
    cg.continues.pop_back();
    cg.EmitJumpBack(loopback);
    cg.EmitLabelDef(jumpout);
    cg.ApplyBreaks(break_level);
    cg.Dummy(retval);
}

void For::Generate(CodeGen &cg, size_t retval) const {
    cg.EmitPushInt(-1);  // i
    cg.temptypestack.push_back({ type_int, LT_ANY });
    cg.Gen(iter, 1);
    cg.loops.push_back(this);
    auto startloop = cg.EmitLabelDefBackwards();
    cg.continues.push_back(startloop);
    auto break_level = cg.breaks.size();
    auto tstack_level = cg.tstack_size;
    int exitloop = -1;
    switch (iter->exptype->t) {
        case V_INT:      exitloop = cg.GenForCond(V_INT); break;
        case V_STRING:   exitloop = cg.GenForCond(V_STRING); break;
        case V_VECTOR:   exitloop = cg.GenForCond(V_VECTOR); break;
        default:         assert(false);
    }
    cg.Gen(fbody, 0);
    cg.EmitJumpBack(startloop);
    cg.EmitLabelDef(exitloop);
    cg.loops.pop_back();
    cg.continues.pop_back();
    cg.TakeTemp(2, false);
    assert(tstack_level == cg.tstack_size); (void)tstack_level;
    cg.PopTemp();
    cg.PopTemp();
    cg.ApplyBreaks(break_level);
    cg.Dummy(retval);
}

void ForLoopElem::Generate(CodeGen &cg, size_t /*retval*/) const {
    auto typelt = cg.temptypestack.back();
    switch (typelt.type->t) {
        case V_INT:
            cg.GenForCounter(2, 3);
            break;
        case V_STRING:
            cg.GenForElem(true, 3, 0);
            break;
        case V_VECTOR: {
            auto sub = typelt.type->sub;
            // A single slot element is a reference the loop owns when its type says so, a
            // struct one has a whole bitmask of them.
            auto bitmask = !IsRefNil(sub->t)  ? 0
                         : IsStruct(sub->t)   ? cg.BitMaskForRefStuct(sub)
                                              : 1;
            cg.GenForElem(false, ValWidth(sub) + 2, bitmask);
            break;
        }
        default:
            assert(false);
    }
}

void ForLoopCounter::Generate(CodeGen &cg, size_t /*retval*/) const {
    cg.GenForCounter(1, 2);
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
        // The loop's own two slots come off here, but the code after the break still expects
        // them to be there, so put them back once the jump is emitted.
        cg.GenPop(cg.temptypestack[cg.temptypestack.size() - 1]);
        cg.GenPop(cg.temptypestack[cg.temptypestack.size() - 2]);
        lab = cg.EmitJump();
        cg.PushTemp();
        cg.PushTemp();
    } else {
        lab = cg.EmitJump();
    }
    cg.breaks.push_back(lab);
}

void Continue::Generate(CodeGen &cg, size_t retval) const {
    assert(!retval);
    (void)retval;
    assert(!cg.rettypes.size());
    assert(!cg.loops.empty());
    int startloop = cg.continues.back();
    cg.EmitJumpBack(startloop);
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
    cg.PopTemp();
    CodeGen::BlockStack bs(cg.tstack_size);
    for (auto n : cases->children) {
        bs.Start();
        cg.PushTemp();
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
                loc = cg.EmitJumpFail(0);
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
                auto lab = cg.EmitJumpFail(0);
                nextcase.push_back(lab);
            } else {
                auto lab = cg.EmitJumpNoFail(0);
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
            auto lab = cg.EmitJump();
            exitswitch.push_back(lab);
        }
    }
    cg.EmitLabelDefs(nextcase);
    if (!have_default) {
        bs.Start();
        cg.PushTemp();
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
    // We clamp ranges to this, to cause too big ranges to make load factor sparse.
    const int64_t max_range_weight = 10;
    for (auto n : cases->children) {
        auto cas = AssertIs<Case>(n);
        for (auto c : cas->pattern->children) {
            auto [istart, iend] = get_range(c);
            if (!istart || !iend || istart->integer > iend->integer)
                return false;
            num += std::min(max_range_weight, iend->integer - istart->integer + 1);
            mini = std::min(mini, istart->integer);
            maxi = std::max(maxi, iend->integer);
        }
    }
    // Decide if jump table is economic.
    const int64_t min_vals = 3;     // Minimum to do jump table.
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
    cg.TrackUseDef(1, 0);
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
    CodeGen::BlockStack bs(cg.tstack_size);
    for (auto [i, n] : enumerate(cases->children)) {
        bs.Start();
        auto cas = AssertIs<Case>(n);
        auto lab = labels[i];
        // Don't actually emit labels because C code turns these into "case"
        if (cas->pattern->children.empty()) {
            lab = deflab;
        }
        cg.TrackUseDef(0, 0);
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
            auto lab = cg.EmitJump();
            exitswitch.push_back(lab);
        }
    }
    cg.TrackUseDef(0, 0);
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
    cg.TrackUseDef(1, 0);
    int range = (int)cases->children.size();
    GenerateJumpTableMain(cg, retval, range, 0, range - 1);
}

void Case::Generate(CodeGen &cg, size_t retval) const {
    if (cbody->Arity()) {
        cg.Gen(cbody, retval);
        if (retval) cg.TakeTemp(1, true);
    } else {
        // An empty default case signals runtime error for enums. An `out_of_range` case takes
        // the exact same slot, but has a user block to run instead, so it is never empty.
        assert(pattern->children.empty() && !out_of_range);
        // FIXME: would be great to ensure the offending value is still on the stack for
        // this instruction to have access to.
        cg.TrackUseDef(0, 0);
        append(cg.cb, "    RtEnumRangeErr(vm);\n");
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
    cg.TrackUseDef(arg_width, 1);
    append(cg.cb, "    RtNewVec(vm, ", cg.sp(), ", (type_elem_t)", (int)offset, ", ",
           (int)Arity(), ");\n");
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
        cg.EmitNewObject(offset, arg_width, exptype);
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
        // Whether a nil value matches is resolved at compile time, so both ops
        // only ever compare against the non-nil type.
        int nilres = resolvedtype->t == V_NIL;
        TypeRef te = resolvedtype->ElementIfNil();
        if (te->t == V_CLASS && te->udt->subtype_dfs_end > te->udt->subtype_dfs) {
            // The tested type has subclasses, so test the value's type against
            // the id range of the tested type's subtree.
            cg.EmitIsSubType(te->udt->subtype_dfs, te->udt->subtype_dfs_end, nilres, te);
        } else {
            cg.EmitIsType(cg.GetTypeTableOffset(te), nilres, te);
        }
    }
}

void EnumCoercion::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (retval) cg.TakeTemp(1, false);
}

void Return::Generate(CodeGen &cg, size_t retval) const {
    assert(!cg.rettypes.size());
    small_vector<TypeLT, 8> typestackbackup = cg.temptypestack;
    auto tstackbackup = cg.tstack_size;
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
            cg.EmitPushNil();
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
        cg.EmitReturn(RET_LOCAL, nretslots, -1, nretslots);
    } else {
        // This is for both if the return itself is non-local, or if the destination has
        // an unwind check.
        cg.EmitReturn(RET_NONLOCAL, nretslots, sf->parent->idx, nretslots);
    }

    reset_from_small_vector(cg.temptypestack, typestackbackup);
    cg.tstack_size = tstackbackup;
    // We can promise to be providing whatever retvals the caller wants.
    for (size_t i = 0; i < retval; i++) {
        cg.rettypes.push_back({ type_undefined, LT_ANY });
        cg.PushTemp();  // FIXME: is this necessary? do more generally?
    }
}

void TypeOf::Generate(CodeGen &cg, size_t /*retval*/) const {
    // This is typically a TypeAnnotation or IdentRef, but
    // the IdentRef can also have been folded into various constants.
    cg.EmitPushInt(cg.GetTypeTableOffset(child->exptype));
}

}  // namespace lobster
