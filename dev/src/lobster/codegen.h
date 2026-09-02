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

// The bitwise operators, in the order of the LvalOp modifiers for them.
enum BitOp { BIT_AND, BIT_OR, BIT_XOR, BIT_ASL, BIT_ASR };

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
    // The parameter each slot of the arguments comes in as, in f_args order, and what the
    // function returns, see FunSignature.
    vector<string> f_arg_names;
    int f_outw = 0;
    int f_ret_kind = 0;
    // How many slot variables the function spells, and whether any helper needed the stack as
    // memory, both only known at the end of its codegen like f_regs_max.
    int f_slot_max = 0;
    bool f_uses_vals = false;
    vector<int> f_args;
    vector<int> f_defs;
    int f_keepvars = -1;
    // The lvalue the op chain currently being emitted produced: a local or global, which need no
    // code at all beyond a note of where the assignment that follows writes to, or an address
    // computed into the "lv" local, which f_uses_lval says the function then needs. A struct
    // occupies consecutive slots from there.
    enum LvalKind { LVK_NONE, LVK_LOCAL, LVK_GLOBAL, LVK_PTR };
    LvalKind f_lval_kind = LVK_NONE;
    int f_lval_idx = 0;
    bool f_uses_lval = false;
    vector<int> ownedvars;
    vector<int> funstarttables;
    vector<int> var_to_local;
    // The C variable each local (by var_to_local index) lives in, and every name the function
    // has handed out, to keep them apart.
    vector<string> local_names;
    set<string> f_names_used;
    // A struct local staged in the stack array to be indexed at runtime, which goes back into
    // its variables once the modifier has written it, see EmitLvalStructIndex.
    struct { int idx = 0, width = 0, base = 0; } f_writeback;
    // How far up the stack array such staging reaches, beyond what the stack itself needs.
    int f_vals_max = 0;
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
        f_arg_names.clear();
        f_outw = 0;
        f_ret_kind = RK_VOID;
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
        tstack_max = 0;
        f_args.clear();
        f_defs.clear();
        f_arg_names.clear();
        f_outw = 0;
        f_ret_kind = RK_VOID;
        f_keepvars = 0;
        Gen(parser.root, return_value);
        auto type = parser.root->exptype;
        assert(type->NumValues() == (size_t)return_value);
        if (return_value) {
            TrackUseDef(1, 0);
            append(cb, "    RtExit(vm, ", Slot(1), ", (type_elem_t)", GetTypeTableOffset(type),
                   ");\n");
        } else {
            TrackUseDef(0, 0);
            append(cb, "    RtExitVoid(vm);\n");
        }
        f_regs_max = (int)tstack_max;
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
        local_names.clear();
        f_names_used.clear();
        f_arg_names.clear();
        f_outw = ReturnSlots(sf);
        f_ret_kind = RetKindOf(f_outw);
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
                        local_names.push_back(LocalName(*arg.sid, i));
                        if (&f_ad == &f_args) f_arg_names.push_back(local_names.back());
                    } else if (&f_ad == &f_args) {
                        // Lives in a global while the function runs, so the parameter only
                        // holds the old value of that meanwhile, see DefineFunction.
                        f_arg_names.push_back(LocalName(*arg.sid, i));
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
                "    extern \"C\" iint GLFrame(VMRef vm);\n"
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
                "#define LSTRING_DATA(S) ((unsigned char *)((S) + 1))\n";
            // This needs to correspond to the C++ VMBase, enforced in Entry().
            append(sd,
                "typedef struct {\n"
                "    int last_line;\n"
                "    int last_fileidx;\n"
                "    int ret_unwind_to;\n"
                "    Value *fvars_ptr;\n"
                "    Value *constant_strings_ptr;\n"
                "    Value ret_buf[", MAX_RETURN_SLOTS, "];\n"
                "} VMBase;\n");
            sd +=
                "typedef Value *StackPtr;\n"
                "typedef VMBase *VMRef;\n"
                "typedef void (*fun_base_t)(VMRef);\n"
                // An offset into the type table, which is what the helpers take one as.
                "typedef int type_elem_t;\n"
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
                "LString *RtPushStr(VMRef, int);\n"
                "void RtNativeCallV(VMRef, StackPtr, int);\n"
                "Value RtNativeCall0(VMRef, int);\n"
                "Value RtNativeCall1(VMRef, int, Value);\n"
                "Value RtNativeCall2(VMRef, int, Value, Value);\n"
                "Value RtNativeCall3(VMRef, int, Value, Value, Value);\n"
                "Value RtNativeCall4(VMRef, int, Value, Value, Value, Value);\n"
                "Value RtNativeCall5(VMRef, int, Value, Value, Value, Value, Value);\n"
                "Value RtNativeCall6(VMRef, int, Value, Value, Value, Value, Value, Value);\n"
                "Value RtNativeCall7(VMRef, int, Value, Value, Value, Value, Value, Value, Value);\n"
                "void RtNativeCall0Rets(VMRef, StackPtr, int);\n"
                "void RtNativeCall1Rets(VMRef, StackPtr, int);\n"
                "void RtNativeCall2Rets(VMRef, StackPtr, int);\n"
                "void RtNativeCall3Rets(VMRef, StackPtr, int);\n"
                "void RtNativeCall4Rets(VMRef, StackPtr, int);\n"
                "void RtNativeCall5Rets(VMRef, StackPtr, int);\n"
                "void RtNativeCall6Rets(VMRef, StackPtr, int);\n"
                "void RtNativeCall7Rets(VMRef, StackPtr, int);\n"
                "LVector *RtNewVec(VMRef, Value *, type_elem_t, int);\n"
                "RefObj *RtNewObject(VMRef, Value *, type_elem_t);\n"
                "void RtExit(VMRef, Value, type_elem_t);\n"
                "void RtExitVoid(VMRef);\n"
                "void RtAbort(VMRef);\n"
                "long long RtIDiv(VMRef, long long, long long);\n"
                "long long RtIMod(VMRef, long long, long long);\n"
                "double RtFMod(double, double);\n"
                "LString *RtSAdd(VMRef, Value, Value);\n"
                "long long RtSLt(Value, Value);\n"
                "long long RtSGt(Value, Value);\n"
                "long long RtSLe(Value, Value);\n"
                "long long RtSGe(Value, Value);\n"
                "long long RtSEq(Value, Value);\n"
                "long long RtSNe(Value, Value);\n"
                "long long RtSnEq(Value, Value);\n"
                "long long RtSnNe(Value, Value);\n"
                "LString *RtStrConcatN(VMRef, Value *, int);\n"
                "LString *RtToString(VMRef, Value, type_elem_t);\n"
                "LString *RtStructToString(VMRef, Value *, type_elem_t);\n"
                "Value RtIndexStruct(VMRef, Value *, long long, int);\n"
                "long long RtIsSubType(VMRef, Value, int, int, int);\n"
                "fun_base_t RtDynDispatch(VMRef, Value, int);\n"
                "void RtEnumRangeErr(VMRef);\n"
                "Value *RtLvalIndexClass(VMRef, Value, long long, int);\n"
                "void RtLvSAdd(VMRef, Value *, Value);\n"
                "int RtStaticSetThisFrame(VMRef, int);\n"
                "int RtMemberSetThisFrame(VMRef, Value, int);\n"
                ;

            sd += "extern void Entry(int, int, int, int, int, int);\n"
                  "extern void IDXErr(VMRef, long long, long long, RefObj *);\n"
                  "extern void IDXErrS(VMRef, long long, long long);\n"
                  "extern long long GLFrame(VMRef);\n"
                  "extern void BackupVar(VMRef, int);\n"
                  "extern void DecOwned(VMRef, int);\n"
                  "extern void DecDelete(VMRef, RefObj *);\n"
                  "extern void AssertFailed(VMRef, int, int, int);\n"
                  "extern void DecVal(VMRef, Value);\n"
                  "extern void RestoreBackup(VMRef, int);\n"
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

    // What a function returns: nothing, a Value, or several values thru an array it is given.
    enum RetKind { RK_VOID, RK_VALUE, RK_MULTI };
    static RetKind RetKindOf(int outw) {
        return outw == 0 ? RK_VOID : outw == 1 ? RK_VALUE : RK_MULTI;
    }

    // The C signature of a function: its return value if it has one, the array several land in
    // otherwise, then an argument per slot of its arguments. With names for the definition,
    // without for a declaration.
    string FunSignature(string_view name, RetKind rk, int nargslots,
                        const vector<string> *names) {
        string s = cat("static ", rk == RK_VALUE ? "Value " : "void ", name, "(VMRef");
        if (names) s += " vm";
        if (rk == RK_MULTI) append(s, ", Value *", names ? "rets" : "");
        for (int i = 0; i < nargslots; i++) {
            s += ", Value";
            if (names) append(s, " ", (*names)[i]);
        }
        return s + ")";
    }

    // The same as the type of a pointer to one, for calling a function value or the function
    // a dynamic dispatch lands in.
    string FunPtrType(RetKind rk, int nargslots) {
        string s = cat(rk == RK_VALUE ? "Value" : "void", " (*)(VMRef");
        if (rk == RK_MULTI) s += ", Value *";
        for (int i = 0; i < nargslots; i++) s += ", Value";
        return s + ")";
    }

    // How many slots the arguments of a function take, and its return values.
    int ArgSlots(const SubFunction &sf) {
        int n = 0;
        for (auto &arg : sf.args) if (!arg.sid->constprop) n += ValWidth(arg.sid->type);
        return n;
    }

    int ReturnSlots(const SubFunction &sf) {
        return ValWidthMulti(sf.returntype, sf.returntype->NumValues());
    }

    void DeclareFunction(SubFunction &sf, string &sd) {
        append(sd, FunSignature(cat("fun_", sf.idx), RetKindOf(ReturnSlots(sf)), ArgSlots(sf),
                                nullptr), ";\n");
    }

    // A declaration of Values, a line per 12 of them to keep it readable.
    void GenValueDecls(string &sd, const vector<string> &names) {
        for (size_t i = 0; i < names.size(); i++) {
            if (i % 12 == 0) sd += i ? ";\n    Value " : "    Value ";
            else sd += ", ";
            sd += names[i];
        }
        if (!names.empty()) sd += ";\n";
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

    // The name of slot `slot` of a struct: the field it is in, and for a nested struct that
    // field's own slot name behind it.
    string SlotName(const UDT &udt, int slot) {
        for (auto [k, sfield] : enumerate(udt.sfields)) {
            if (slot >= sfield.slot && slot < sfield.slot + ValWidth(sfield.type)) {
                string name = udt.g.fields[k].id->name;
                if (IsStruct(sfield.type->t)) {
                    append(name, "_", SlotName(*sfield.type->udt, slot - sfield.slot));
                }
                return name;
            }
        }
        assert(false);
        return cat("slot", slot);
    }

    // Whether the generated code uses a name for something of its own, which a local may then
    // not be called: the keywords of C and C++, what the prologues declare, the helpers, and the
    // names the emitters make up, which all end in a number or start with an underscore.
    static bool IsReservedName(string_view name) {
        static const set<string_view> reserved = {
            "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
            "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long",
            "register", "restrict", "return", "short", "signed", "sizeof", "static", "struct",
            "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
            "alignas", "alignof", "and", "and_eq", "asm", "bitand", "bitor", "bool", "catch",
            "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "consteval",
            "constexpr", "constinit", "const_cast", "co_await", "co_return", "co_yield",
            "decltype", "delete", "dynamic_cast", "explicit", "export", "false", "friend",
            "import", "module", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
            "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
            "reinterpret_cast", "requires", "static_assert", "static_cast", "template", "this",
            "thread_local", "throw", "true", "try", "typeid", "typename", "using", "virtual",
            "wchar_t", "xor", "xor_eq", "override", "final", "NULL",
            "vm", "lv", "vals", "locals", "ctx", "tsld", "top", "rs", "ret", "rets",
            "epilogue", "main", "argc", "argv", "vmmeta", "Value", "VMRef", "StackPtr",
            "RefObj", "LVector", "LString", "VMBase", "fun_base_t", "type_elem_t", "vtables",
            "funinfo_table", "compiled_entry_point", "type_table", "stringtable", "file_names",
            "function_names", "udts", "specidents", "enums", "ser_ids",
            "subfunctions_to_function", "iint", "int2float64", "lobster", "std", "string_view",
            "span", "uint64_t", "int64_t", "memcpy", "memmove", "GLFrame", "Entry", "IDXErr",
            "IDXErrS", "BackupVar", "DecOwned", "DecDelete", "AssertFailed", "DecVal",
            "RestoreBackup", "GetTypeSwitchID", "PushFunId", "PopFunId", "StartProfile",
            "EndProfile", "LSTRING_DATA",
        };
        if (reserved.count(name)) return true;
        if (name[0] == '_' || name.substr(0, 2) == "Rt") return true;
        auto numbered = [&](string_view prefix) {
            if (name.size() <= prefix.size() || name.substr(0, prefix.size()) != prefix) return false;
            for (auto c : name.substr(prefix.size())) if (!isdigit((uint8_t)c)) return false;
            return true;
        };
        return numbered("r") || numbered("keep") || numbered("block") || numbered("fun_");
    }

    // A C name for a local that is unique within the function: the name it has in the program,
    // made a C identifier if it is not one, with a number behind it if that is taken. A prefix
    // the generated code claims for itself gets a letter in front instead, since a number
    // behind it would not lose it.
    string UniqueName(string name) {
        for (auto &c : name) if (!isalnum((uint8_t)c) && c != '_') c = '_';
        if (name.empty() || isdigit((uint8_t)name[0]) || name[0] == '_' ||
            name.substr(0, 2) == "Rt" || name == "fun" || name.substr(0, 4) == "fun_") {
            name = "v" + name;
        }
        auto base = name;
        for (int n = 2; IsReservedName(name) || f_names_used.count(name); n++) {
            name = cat(base, "_", n);
        }
        f_names_used.insert(name);
        return name;
    }

    // Slot `slot` of the variable sid, which for a struct is one of its fields.
    string LocalName(const SpecIdent &sid, int slot) {
        string name = sid.id->name;
        if (IsStruct(sid.type->t)) append(name, "_", SlotName(*sid.type->udt, slot));
        return UniqueName(name);
    }

    // The stack slots, counted the way the emitters do: Slot(1) is the top of the stack before
    // the current op, Slot(0) the first one above it, Slot(-1) the one after that. Each is a
    // variable of its own, which is what lets the C compiler keep them in registers: nothing
    // ever takes their address.
    string Slot(int off) { return SlotVar(regso - off); }
    string SlotVar(int k) {
        f_slot_max = std::max(f_slot_max, k + 1);
        return cat("r", k);
    }
    // The stack as memory, for the helpers that take a pointer to it, see GenStackCall.
    string StackArray() { return "vals"; }
    string StackSlot(int k) { return cat(StackArray(), "[", k, "]"); }
    // A local variable, a global, and the temporaries a function keeps references alive in. The
    // locals are variables of their own like the stack slots are, see LocalName.
    string Local(int i) { return local_names[i]; }
    // With stack traces on, every write to a local also lands in an array, since that is where
    // a trace dumps them from, see PushFunId.
    bool ShadowLocals() { return runtime_checks >= RUNTIME_STACK_TRACE; }
    string Shadow(int i) { return cat("locals[", i, "]"); }
    void LocalWritten(int idx, int width) {
        if (!ShadowLocals()) return;
        for (int i = 0; i < width; i++) CopyValue(cb, Shadow(idx + i), Local(idx + i));
    }
    // The C++ backend addresses the VM's own array of globals at a constant offset, which is why
    // that array sits at the end of the VM; the C one has no way to know where that is, so it
    // goes thru the pointer to it that VMBase carries for that purpose.
    string Global(int offset) {
        return cpp ? cat("vm.fvars[", offset, "]") : cat("vm->fvars_ptr[", offset, "]");
    }
    string KeepVar(int i) { return cat("keep", i); }
    // The fields of a Value, which the C++ backend reads thru accessors.
    string IVal(string_view v) { return cat(v, cpp ? ".ival()" : ".ival"); }
    string FVal(string_view v) { return cat(v, cpp ? ".fval()" : ".fval"); }
    string Val(bool isfloat, string_view v) { return isfloat ? FVal(v) : IVal(v); }
    // The bits of a reference or a function value for comparing, without the accessor's type
    // check in the C++ backend, since either side may be nil or an int false value.
    string AnyOf(string_view v) { return cat(v, cpp ? ".any()" : ".ref"); }
    string IpOf(string_view v) { return cat(v, cpp ? ".ip()" : ".ival"); }
    void comment(string_view c) { append(cb, " // ", c, "\n"); };
    string_view vmref() { return string_view(cpp ? "vm." : "vm->"); };

    // A call to a helper that works on the stack: it takes `uses` values off the top and leaves
    // `defs` behind. Since the slots are variables, the operands are copied into the array kept
    // for this purpose and the results back out of it around the call, which `call` renders
    // given the pointer into that array to hand the helper.
    template<typename F> void GenStackCall(int uses, int defs, F call, string_view cmt = {}) {
        TrackUseDef(uses, defs);
        f_uses_vals = true;
        for (int i = regso - uses; i < regso; i++) CopyValue(cb, StackSlot(i), SlotVar(i));
        append(cb, "    ", call(string_view(cat(StackArray(), " + ", regso))), ";");
        if (cmt.empty()) cb += "\n"; else comment(cmt);
        for (int i = regso - uses; i < regso - uses + defs; i++)
            CopyValue(cb, SlotVar(i), StackSlot(i));
    }

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

    // A copy of one Value to another. The C backend copies the fields rather than the struct,
    // since libtcc turns a struct assignment into a memcpy call, even for a single 64-bit value.
    string CopyValueText(string_view dest, string_view src) {
        if (cpp) return cat(dest, " = ", src, ";");
        auto s = cat(dest, ".ival = ", src, ".ival;");
        #if RTT_ENABLED
            append(s, " ", dest, ".type = ", src, ".type;");
        #endif
        return s;
    }

    void CopyValue(string &sd, string_view dest, string_view src, string_view lf = "\n") {
        append(sd, "    ", CopyValueText(dest, src), lf);
    }

    // In C the runtime type field, when there is one, is ours to keep correct on every write.
    string SetType(string_view lv, RTType t) {
        #if RTT_ENABLED
            return cat(" ", lv, ".type = ", (int)t, ";");
        #else
            (void)lv;
            (void)t;
            return {};
        #endif
    }

    // Writing a scalar: the C++ backend goes thru Value's constructor, which picks the runtime
    // type up from the expression, the C one writes the field.
    void SetInt(string &sd, string_view lv, string_view expr, string_view lf = "\n") {
        if (cpp) append(sd, "    ", lv, " = Value(", expr, ");", lf);
        else append(sd, "    ", lv, ".ival = ", expr, ";", SetType(lv, RTT_INT), lf);
    }

    void SetFloat(string &sd, string_view lv, string_view expr, string_view lf = "\n") {
        if (cpp) append(sd, "    ", lv, " = Value(", expr, ");", lf);
        else append(sd, "    ", lv, ".fval = ", expr, ";", SetType(lv, RTT_FLOAT), lf);
    }

    void SetNil(string &sd, string_view lv) {
        if (cpp) append(sd, "    ", lv, " = Value(0, lobster::RTT_NIL);\n");
        else append(sd, "    ", lv, ".ival = 0;", SetType(lv, RTT_NIL), "\n");
    }

    // Writing a reference a helper returned, whose type the code generator knows statically.
    void SetRef(string &sd, string_view lv, string_view expr, RTType t, string_view lf = "\n") {
        if (cpp) append(sd, "    ", lv, " = Value(", expr, ");", lf);
        else append(sd, "    ", lv, ".ref = (RefObj *)", expr, ";", SetType(lv, t), lf);
    }

    // Writing a whole Value a helper returned. In C its field comes out of the call's result
    // directly, unless there is more than one to copy, which would call it twice.
    void SetValue(string &sd, string_view lv, string_view expr, string_view lf = "\n") {
        if (cpp) {
            append(sd, "    ", lv, " = ", expr, ";", lf);
        } else {
            #if RTT_ENABLED
                append(sd, "    { Value _v = ", expr, "; ", CopyValueText(lv, "_v"), " }", lf);
            #else
                append(sd, "    ", lv, ".ival = ", expr, ".ival;", lf);
            #endif
        }
    }

    // The operands of a helper that works on a run of values, copied into the stack array,
    // which is where it gets its pointer to them. Returns that pointer.
    string StageRange(int first, int n) {
        f_uses_vals = true;
        for (int i = first; i < first + n; i++) CopyValue(cb, StackSlot(i), SlotVar(i));
        return cat(StackArray(), " + ", first);
    }

    // The same for the top `uses` values the current op consumes.
    string StageArgs(int uses) { return StageRange(regso - uses, uses); }

    // A binary operator on scalars is the C operator, except for integer division and modulo,
    // which check their divisor, and float modulo, which is fmod.
    string BinExpr(bool isfloat, MathOp op, string_view a, string_view b) {
        static const char *cops[] = { "+", "-", "*", "/", "%",
                                      "<", ">", "<=", ">=", "==", "!=" };
        if (op == MOP_MOD) {
            // The only helper without an argument the C++ backend could find it thru.
            return isfloat ? cat(cpp ? "lobster::" : "", "RtFMod(", a, ", ", b, ")")
                           : cat("RtIMod(vm, ", a, ", ", b, ")");
        }
        if (op == MOP_DIV && !isfloat) return cat("RtIDiv(vm, ", a, ", ", b, ")");
        return cat(a, " ", cops[op], " ", b);
    }

    // Writes the result of one to dest; a comparison produces an int whatever it compared.
    void SetBinOp(bool isfloat, MathOp op, string_view dest, string_view a, string_view b) {
        auto expr = BinExpr(isfloat, op, a, b);
        if (isfloat && op < MOP_LT) SetFloat(cb, dest, expr);
        else SetInt(cb, dest, expr);
    }

    // The shifts mask their count to the width of an int, see MaskedShiftLeft.
    string BitExpr(BitOp op, string_view a, string_view b) {
        switch (op) {
            case BIT_AND: return cat(a, " & ", b);
            case BIT_OR:  return cat(a, " | ", b);
            case BIT_XOR: return cat(a, " ^ ", b);
            case BIT_ASL: return cat("(long long)((unsigned long long)", a, " << (", b, " & 63))");
            default:      return cat(a, " >> (", b, " & 63)");
        }
    }

    void GenPushVar(size_t retval, TypeRef type, int offset, bool used_as_freevar) {
        if (!retval) return;
        auto width = ValWidth(type);
        TrackUseDef(0, width);
        for (int i = 0; i < width; i++) {
            if (used_as_freevar) {
                // A global is addressed by number, so say which it is.
                CopyValue(cb, Slot(-i), Global(offset + i), "");
                comment(IdName(offset + i, false, type));
            } else {
                CopyValue(cb, Slot(-i), Local(var_to_local[offset + i]));
            }
        }
    }

    // Slot i of the lvalue the op chain produced, see f_lval_kind.
    string Lval(int i) {
        switch (f_lval_kind) {
            case LVK_LOCAL: return Local(f_lval_idx + i);
            case LVK_GLOBAL: return Global(f_lval_idx + i);
            default: return cat("lv[", i, "]");
        }
    }

    // The same as an address, for the helpers that take one, which a local never has since it
    // is a variable.
    string LvalPtr() {
        switch (f_lval_kind) {
            case LVK_GLOBAL: return cpp ? cat("vm.fvars + ", f_lval_idx)
                                        : cat("vm->fvars_ptr + ", f_lval_idx);
            case LVK_PTR: return "lv";
            default: assert(false); return {};
        }
    }

    // A local is a variable, so this needs no code at all, just a note of which one the
    // assignment that follows writes to.
    void EmitLvalLocal(int offset) {
        TrackUseDef(0, 0);
        f_lval_kind = LVK_LOCAL;
        f_lval_idx = var_to_local[offset];
    }

    // A global is at a known address too, once the generated code can get at the array.
    void EmitLvalGlobal(int offset, TypeRef type) {
        TrackUseDef(0, 0);
        f_lval_kind = LVK_GLOBAL;
        f_lval_idx = offset;
        append(cb, "    // lval: ", IdName(offset, false, type), "\n");
    }

    // A field as an lvalue is at a constant offset from the object, whose fields sit right behind
    // its header, same as reading one. That does lose a debug only range check.
    void EmitLvalField(int slot) {
        TrackUseDef(1, 0);
        f_uses_lval = true;
        if (cpp) {
            append(cb, "    lv = &", Slot(1), ".oval()->AtR(", slot, ");\n");
        } else {
            append(cb, "    lv = (Value *)((RefObj *)", Slot(1), ".ref + 1) + ", slot, ";\n");
        }
        f_lval_kind = LVK_PTR;
    }

    // The elements of the vector in _o, and the index of one of them or of a slot of one, at
    // the width the vector holds them at.
    string Elems() { return cpp ? "_o->Elems()" : "_o->elems"; }
    string ElemIndex(int width, int slot) {
        if (width == 1) return slot ? cat("_i + ", slot) : "_i";
        return cat("_i * ", width, " + ", slot);
    }

    // Steps into the vector in `vec` with the indices above it on the stack, one nested vector
    // per level, with a range check per level whose failure path stays a call, leaving the
    // innermost in _o and the index into it in _i, inside a block the caller closes. A struct
    // index has its components used back to front, the first one landing on the element.
    void GenVectorDescent(int levels, string_view vec) {
        if (cpp) {
            append(cb, "    {\n    auto _o = ", vec, ".vval();\n    iint _i;\n");
        } else {
            append(cb, "    {\n    LVector *_o = (LVector *)", vec, ".ref;\n    long long _i;\n");
        }
        for (int j = levels - 1; j >= 0; j--) {
            append(cb, "    _i = ", IVal(Slot(levels - j)), ";\n");
            if (cpp) {
                append(cb, "    if ((uint64_t)_i >= (uint64_t)_o->len)"
                           " vm.IDXErr(_i, _o->len, _o);\n");
                if (j) append(cb, "    _o = _o->AtS(_i).vval();\n");
            } else {
                append(cb, "    if ((unsigned long long)_i >= (unsigned long long)_o->len)"
                           " IDXErr(vm, _i, _o->len, &_o->ro);\n");
                if (j) append(cb, "    _o = (LVector *)_o->elems[_i].ref;\n");
            }
        }
    }

    // Indexing to get an lvalue hands the address to whatever follows thru a local rather than
    // thru the VM, so they read as a chain of assignments, and none of them leave anything on
    // the stack. An element of a vector is at the width the vector holds its elements at plus
    // wherever in one the assignment lands.
    void EmitLvalVectorIndex(int levels, int offset, int width) {
        TrackUseDef(levels + 1, 0);
        f_uses_lval = true;
        GenVectorDescent(levels, Slot(levels + 1));
        append(cb, "    lv = ", Elems(), " + ", ElemIndex(width, offset), ";\n    }\n");
        f_lval_kind = LVK_PTR;
    }

    // A class indexed at runtime, whose range check needs the type info, so it stays a helper.
    void EmitLvalClassIndex(int offset) {
        TrackUseDef(2, 0);
        f_uses_lval = true;
        append(cb, "    lv = RtLvalIndexClass(vm, ", Slot(2), ", ", IVal(Slot(1)), ", ", offset,
               ");\n");
        f_lval_kind = LVK_PTR;
    }

    // A struct indexed at runtime, the one case that steps into the lvalue it was handed.
    void EmitLvalStructIndex(int offset, int numslots) {
        TrackUseDef(1, 0);
        f_uses_lval = true;
        string base;
        if (f_lval_kind == LVK_LOCAL) {
            // A struct in variables has to be in memory to be indexed at runtime, so it goes
            // thru the stack array above what is in use, and comes back out once the modifier
            // has written it, see GenLvalWriteBack.
            f_writeback = { f_lval_idx, numslots, regso };
            f_uses_vals = true;
            f_vals_max = std::max(f_vals_max, regso + numslots);
            for (int j = 0; j < numslots; j++) {
                CopyValue(cb, StackSlot(regso + j), Local(f_lval_idx + j));
            }
            base = cat(StackArray(), " + ", regso);
        } else {
            base = LvalPtr();
        }
        append(cb, "    {\n    long long _i = ", IVal(Slot(1)), ";\n");
        if (cpp) {
            append(cb, "    if ((uint64_t)_i >= ", numslots, ") vm.IDXErrS(_i, ", numslots,
                   ");\n");
        } else {
            append(cb, "    if ((unsigned long long)_i >= ", numslots, ") IDXErrS(vm, _i, ",
                   numslots, ");\n");
        }
        append(cb, "    lv = ", base, " + _i", offset ? cat(" + ", offset) : string(),
               ";\n    }\n");
        f_lval_kind = LVK_PTR;
    }

    // What a modifier wrote thru the lvalue lands where it belongs: a struct staged in the
    // stack array back in its variables, and any local in its shadow, see LocalWritten.
    void GenLvalWriteBack(TypeRef type) {
        if (f_writeback.width) {
            for (int j = 0; j < f_writeback.width; j++) {
                CopyValue(cb, Local(f_writeback.idx + j), StackSlot(f_writeback.base + j));
            }
            LocalWritten(f_writeback.idx, f_writeback.width);
            f_writeback.width = 0;
        } else if (f_lval_kind == LVK_LOCAL) {
            LocalWritten(f_lval_idx, IsStruct(type->t) ? ValWidth(type) : 1);
        }
    }

    void EmitPushStr(int stringtableindex) {
        auto sv = stringtable[stringtableindex];
        sv = sv.substr(0, 50);
        string q;
        EscapeAndQuote(sv, q, true);
        if (STRING_CONSTANTS_KEEP) {
            // Still has a reference to take, so leave it to the helper.
            TrackUseDef(0, 1);
            SetRef(cb, Slot(0), cat("RtPushStr(vm, ", stringtableindex, ")"), RTT_STRING, "");
            comment(q);
        } else {
            // Borrowed, so all that is left is the copy out of the VM's table of them.
            TrackUseDef(0, 1);
            CopyValue(cb, Slot(0), cat(vmref(), "constant_strings_ptr[", stringtableindex, "]"),
                      "");
            comment(q);
        }
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
        append(cb, "    if (", onfail ? "!" : "", cpp ? cat(Slot(1), ".True()") : IVal(Slot(1)),
               ") goto block", lab, ";\n");
        return lab;
    }
    int EmitJumpFail(int defslots) { return EmitJumpCond(true, defslots); }
    int EmitJumpNoFail(int defslots) { return EmitJumpCond(false, defslots); }

    // Jump over the initializer of a member or static that has already run this frame. The
    // member version reads the object it belongs to off the stack, the static one needs nothing.
    int EmitJumpIfSetThisFrame(bool member, int varidx) {
        auto lab = Label();
        if (member) {
            TrackUseDef(1, 0);
            append(cb, "    if (!RtMemberSetThisFrame(vm, ", Slot(1), ", ", varidx,
                   ")) goto block", lab, ";\n");
        } else {
            TrackUseDef(0, 0);
            append(cb, "    if (!RtStaticSetThisFrame(vm, ", varidx, ")) goto block", lab, ";\n");
        }
        return lab;
    }

    // There is one helper per number of arguments a native takes, which get them by value and
    // return the result, plus one that leaves its results on the stack for the natives that
    // have several, and a V one for those that take a variable number, which is what a
    // negative count asks for.
    void EmitNativeCall(int nargs, NativeFun *nf, int has_ret, int useslots, int defslots) {
        if (nf->IsGLFrame()) {
            TrackUseDef(useslots, defslots);
            SetInt(cb, Slot(0), "GLFrame(vm)", "");
        } else if (nargs < 0 || defslots > 1) {
            GenStackCall(useslots, defslots, [&](string_view sp) {
                auto s = string("RtNativeCall");
                if (nargs < 0) s += "V"; else append(s, nargs, "Rets");
                return cat(s, "(vm, ", sp, ", ", nf->idx, ")");
            }, nf->name);
            return;
        } else {
            assert(useslots == nargs);
            TrackUseDef(useslots, defslots);
            auto call = cat("RtNativeCall", nargs, "(vm, ", nf->idx);
            for (int i = 0; i < nargs; i++) append(call, ", ", Slot(nargs - i));
            call += ")";
            if (has_ret) SetValue(cb, Slot(nargs), call, "");
            else append(cb, "    ", call, ";");
        }
        comment(nf->name);
    }

    void EmitKeep(int stack_offset, int keep_index_add) {
        // Inside a loop the slot may already hold a reference from a previous iteration.
        auto inloop = !loops.empty();
        TrackUseDef(0, 0);
        auto offset = f_keepvars++ + keep_index_add;
        if (inloop) append(cb, "    DecVal(vm, ", KeepVar(offset), ");\n");
        CopyValue(cb, KeepVar(offset), Slot(stack_offset + 1));
    }

    // Slot i of the values a non-local return passes to the function it returns from.
    string RetBufSlot(int i) {
        return cpp ? cat("vm.ret_buf.v[", i, "]") : cat("vm->ret_buf[", i, "]");
    }

    // The function's return values, from wherever `src` says slot i of them is.
    template<typename F> void GenReturnValues(F src) {
        if (f_ret_kind == RK_VALUE) {
            CopyValue(cb, "ret", src(0));
        } else {
            for (int i = 0; i < f_outw; i++) CopyValue(cb, cat("rets[", i, "]"), src(i));
        }
    }

    // A return. The values come off the stack into the function's own return channel, or for
    // a non-local return into the VM's buffer, marked for the function they return from,
    // parent_idx, whose caller picks them up, see GenUnwind. RET_ANY passes such a return on
    // to the caller, which has nothing to copy.
    void EmitReturn(ReturnKind kind, int nretslots, int parent_idx, int useslots) {
        TrackUseDef(useslots, 0);
        // FIXME: emit epilogue stuff only once at end of function.
        if (kind == RET_LOCAL) {
            GenReturnValues([&](int i) { return Slot(nretslots - i); });
        } else if (kind == RET_NONLOCAL) {
            if (nretslots > MAX_RETURN_SLOTS) {
                parser.ErrorAt(node_context.back(),
                               "too many values returned thru a non-local return");
            }
            for (int i = 0; i < nretslots; i++) {
                CopyValue(cb, RetBufSlot(i), Slot(nretslots - i));
            }
            append(cb, "    ", vmref(), "ret_unwind_to = ", parent_idx, ";\n");
            // Nothing of its own to return, but the caller expects a value all the same.
            if (f_ret_kind == RK_VALUE) SetNil(cb, "ret");
        }
        for (auto varidx : ownedvars) {
            if (sids[varidx].used_as_freevar()) {
                append(cb, "    DecOwned(vm, ", varidx, ");\n");
            } else {
                append(cb, "    DecVal(vm, ", Local(var_to_local[varidx]), ");\n");
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
            // What the call we are passing thru from left is still on the tstack.
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
            append(cb, "    ", Slot(0), " = Value((fun_base_t)fun_", fidx, ");\n");
        } else {
            append(cb, "    ", Slot(0), ".ival = (long long)fun_", fidx, ";",
                   SetType(Slot(0), RTT_FUNCTION), "\n");
        }
    }

    // A call to `callee`, an expression for the function, with the arguments on the stack and
    // the result landing where they were. `nargslots` says how many of the top `uses` slots
    // are arguments, since a function value sits above its own.
    void EmitCallTo(string_view callee, int nargslots, int uses, int defs,
                    string_view cmt = {}) {
        TrackUseDef(uses, defs);
        auto rk = RetKindOf(defs);
        auto base = regso - uses;
        string call = cat(callee, "(vm");
        if (rk == RK_MULTI) {
            // Several results come back thru the stack array.
            f_uses_vals = true;
            append(call, ", ", StackArray(), " + ", base);
        }
        for (int i = 0; i < nargslots; i++) append(call, ", ", Slot(uses - i));
        call += ")";
        if (rk == RK_VALUE) SetValue(cb, Slot(uses), call, "");
        else append(cb, "    ", call, ";");
        if (cmt.empty()) cb += "\n"; else comment(cmt);
        if (rk == RK_MULTI) {
            for (int i = 0; i < defs; i++) CopyValue(cb, SlotVar(base + i), StackSlot(base + i));
        }
    }

    void EmitCall(int fidx, int uses, int defs) {
        EmitCallTo(cat("fun_", fidx), uses, uses, defs,
                   "call: " + Signature(*st.subfunctiontable[fidx]));
    }

    // The function value on top of the stack is called with what sits below it, cast to the
    // signature the type checker gave it.
    void EmitCallValue(int uses, int defs) {
        TrackUseDef(0, 0);
        auto ptr = cat("((", FunPtrType(RetKindOf(defs), uses - 1), ")", IpOf(Slot(1)), ")");
        EmitCallTo(ptr, uses - 1, uses, defs);
    }

    // Same, for the function a dispatch on the class of the first argument lands in.
    void EmitDynDispatch(int vtable_idx, int uses, int defs) {
        TrackUseDef(0, 0);
        auto ptr = cat("((", FunPtrType(RetKindOf(defs), uses), ")RtDynDispatch(vm, ",
                       Slot(uses), ", ", vtable_idx, "))");
        EmitCallTo(ptr, uses, uses, defs);
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

    void TypeComment(TypeRef type) {
        if (IsUDT(type->t)) comment(type->udt->name); else cb += "\n";
    }

    // The optimizer guarantees what is tested is never a scalar, so it is a reference or nil,
    // and whether nil matches was decided statically.
    void EmitIsType(int type_idx, int nilres, TypeRef type) {
        TrackUseDef(1, 1);
        auto v = Slot(1);
        SetInt(cb, v, cpp ? cat(v, ".refnil() ? ", v, ".ref()->tti == (type_elem_t)", type_idx,
                                " : ", nilres)
                          : cat(v, ".ref ? ", v, ".ref->typeinfo == ", type_idx, " : ", nilres),
               "");
        TypeComment(type);
    }

    void EmitIsSubType(int start, int end, int nilres, TypeRef type) {
        TrackUseDef(1, 1);
        SetInt(cb, Slot(1), cat("RtIsSubType(vm, ", Slot(1), ", ", start, ", ", end, ", ",
                                nilres, ")"), "");
        comment(type->udt->name);
    }

    void EmitNewObject(int type_idx, int uses, TypeRef type) {
        TrackUseDef(uses, 1);
        auto fields = StageArgs(uses);
        SetRef(cb, Slot(uses), cat("RtNewObject(vm, ", fields, ", (type_elem_t)", type_idx, ")"),
               RTT_CLASS, "");
        TypeComment(type);
    }

    void EmitStructToString(int type_idx, int uses, TypeRef type) {
        TrackUseDef(uses, 1);
        auto vals = StageArgs(uses);
        SetRef(cb, Slot(uses), cat("RtStructToString(vm, ", vals, ", (type_elem_t)", type_idx,
                                   ")"), RTT_STRING, "");
        TypeComment(type);
    }

    void EmitPushInt(int val) {
        TrackUseDef(0, 1);
        SetInt(cb, Slot(0), to_string(val));
    }

    void GenFloat(double f) {
        if ((float)f == f && isfinite(f)) {
            TrackUseDef(0, 1);
            // We're printing the float as text which seems dangerous, but this path is only
            // taken where double and float are identical, meaning typically whole numbers and
            // other precisely representable ones.
            SetFloat(cb, Slot(0), to_string_hexfloat((float)f));
        } else {
            int2float64 i2f(f);
            EmitPushConst64(true, i2f.i, to_string_float(f));
        }
    }

    // Only the decrement itself is worth emitting: what happens when it reaches zero is a good
    // deal more code, and stays a call.
    void GenDecRef(string_view v) {
        if (cpp) {
            append(cb, "    ", v, ".LTDECRTNIL(vm);\n");
        } else {
            append(cb, "    { RefObj *_r = ", v, ".ref;"
                       " if (_r && --_r->refc <= 0) DecDelete(vm, _r); }\n");
        }
    }

    void GenIncRef(string_view v) {
        if (cpp) {
            append(cb, "    ", v, ".LTINCRTNIL();\n");
        } else {
            append(cb, "    { RefObj *_r = ", v, ".ref; if (_r) _r->refc++; }\n");
        }
    }

    void EmitIncRef(int off) {
        TrackUseDef(0, 0);
        // FIXME: even when the static type is IsRef (i.e. no NIL or scalar), at runtime it is
        // still possible we get passed an int false value due to the way and/or are compiled?
        // See e.g. astar_result in the test.
        // Would be great to remove this case since the if-check is not needed in almost all cases.
        GenIncRef(Slot(off + 1));
    }

    // The ones below are a move or a test on the stack and nothing else. Calling a helper for
    // one of those costs more than the work itself, and pushes its operand and result thru
    // memory where the compiler could otherwise keep them in a register.

    void EmitPushNil() {
        TrackUseDef(0, 1);
        SetNil(cb, Slot(0));
    }

    void EmitPopRef() {
        TrackUseDef(1, 0);
        GenDecRef(Slot(1));
    }

    // These write an int over what may have been a reference, so in C, where we have to keep any
    // runtime type field correct ourselves, say so. Turning a reference into a bool can drop it
    // first: what is left only gets tested against nil, which does not need the value alive.
    void EmitBoolTest(string_view test, bool decref) {
        TrackUseDef(1, 1);
        if (decref) GenDecRef(Slot(1));
        SetInt(cb, Slot(1), cat(IVal(Slot(1)), " ", test));
    }

    void EmitIntToFloat() {
        TrackUseDef(1, 1);
        SetFloat(cb, Slot(1), cat("(double)", IVal(Slot(1))));
    }

    // A 64 bit constant, which only reaches us split in two because that is all an op argument
    // holds. For a float we write the bit pattern rather than a literal, since a decimal one
    // would not round trip exactly and not every C compiler we feed this to takes a hex float.
    void EmitPushConst64(bool isfloat, int64_t bits, string_view cmt = {}) {
        TrackUseDef(0, 1);
        string hex;
        to_string_hex(hex, (uint64_t)bits);
        if (cpp) {
            if (isfloat) {
                SetFloat(cb, Slot(0), cat("int2float64((int64_t)", hex, "ULL).f"), "");
            } else {
                SetInt(cb, Slot(0), cat("(iint)", hex, "ULL"), "");
            }
        } else {
            append(cb, "    ", Slot(0), ".ival = (long long)", hex, "ULL;",
                   SetType(Slot(0), isfloat ? RTT_FLOAT : RTT_INT));
        }
        if (cmt.empty()) cb += "\n"; else comment(cmt);
    }

    // All that is left of an assert in the common case is the test; the reporting is a call.
    void EmitAssert(int defslots, int line, int fileidx, int stringidx) {
        TrackUseDef(1, defslots);
        if (cpp) {
            append(cb, "    if (!", Slot(1), ".True()) vm.AssertFailed(", line, ", ", fileidx,
                   ", ", stringidx, ");\n");
        } else {
            append(cb, "    if (!", Slot(1), ".ival) AssertFailed(vm, ", line, ", ", fileidx,
                   ", ", stringidx, ");\n");
        }
    }

    void DefineFunction(string &sd, bool label) {
        sd += "\n";
        auto sf_idx = f_function_idx;
        if (sf_idx < CODEGEN_SPECIAL_FUNCTION_ID_START)
            append(sd, "// ", Signature(*st.subfunctiontable[sf_idx]), "\n");
        assert(f_arg_names.size() == f_args.size());
        append(sd, FunSignature(cat("fun_", sf_idx), RetKind(f_ret_kind), (int)f_args.size(),
                                &f_arg_names), " {\n");
        // NOTE: f_keepvars, f_slot_max, f_uses_vals, f_vals_max and f_regs_max are not known
        // until the end of codegen of the function!
        vector<string> slots, keeps, locals;
        for (int i = 0; i < f_slot_max; i++) slots.push_back(SlotVar(i));
        for (int i = 0; i < f_keepvars; i++) keeps.push_back(KeepVar(i));
        // The arguments are the parameters, so only the locals after them are declared here.
        int nargs_local = 0;
        for (auto varidx : f_args) if (!sids[varidx].used_as_freevar()) nargs_local++;
        assert((int)local_names.size() == numlocals);
        for (int i = nargs_local; i < numlocals; i++) locals.push_back(local_names[i]);
        GenValueDecls(sd, slots);
        if (f_uses_vals) {
            append(sd, "    Value vals[", std::max({ 1, f_regs_max, f_vals_max }), "];\n");
        }
        GenValueDecls(sd, keeps);
        GenValueDecls(sd, locals);
        if (ShadowLocals() && numlocals) append(sd, "    Value locals[", numlocals, "];\n");
        if (f_uses_lval) append(sd, "    Value *lv = 0;\n");
        if (f_ret_kind == RK_VALUE) sd += "    Value ret;\n";
        for (int i = 0; i < (int)f_args.size(); i++) {
            auto varidx = f_args[i];
            auto &name = f_arg_names[i];
            if (sids[varidx].used_as_freevar()) {
                // The argument is the global for the duration of the call, whose old value the
                // parameter holds meanwhile, to go back at the end.
                append(sd, "    { Value _t; ", CopyValueText("_t", name), " ",
                       CopyValueText(name, Global(varidx)), " ",
                       CopyValueText(Global(varidx), "_t"), " }\n");
            } else if (ShadowLocals()) {
                CopyValue(sd, Shadow(var_to_local[varidx]), name);
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
                auto k = var_to_local[varidx];
                SetNil(sd, Local(k));
                if (ShadowLocals()) SetNil(sd, Shadow(k));
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
            SetNil(sd, KeepVar(i));
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
            append(sd, "    DecVal(vm, ", KeepVar(i), ");\n");
        }
        for (int i = 0; i < (int)f_args.size(); i++) {
            auto varidx = f_args[i];
            if (sids[varidx].used_as_freevar()) CopyValue(sd, Global(varidx), f_arg_names[i]);
        }
        if (runtime_checks >= RUNTIME_STACK_TRACE && f_function_idx < CODEGEN_SPECIAL_FUNCTION_ID_START) {
            append(sd, "    PopFunId(vm);\n");
        }
        if (f_ret_kind == RK_VALUE) sd += "    return ret;\n";
        sd += "}\n";
        ownedvars.clear();
        f_keepvars = -1;
        f_slot_max = 0;
        f_uses_vals = false;
        f_vals_max = 0;
        f_uses_lval = false;
        f_lval_kind = LVK_NONE;
        local_names.clear();
        f_names_used.clear();
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
                append(sd, "(fun_base_t)fun_", id);
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
        sd += "void compiled_entry_point(VMRef vm) {\n";
        if (cpp) {
            append(sd, "    if (vm.vma.nfr.HashAll() != ", parser.natreg.HashAll(),
                   "ULL) vm.BuiltinError(\"code compiled with mismatching builtin function library\");\n");
        } else {
            // The offsets are what the reads we emit inline depend on; the sizes catch a field
            // being added or widened past the last one we read.
            sd += "    Entry(sizeof(Value), sizeof(VMBase), sizeof(RefObj), sizeof(LVector),\n"
                  "          (int)(long long)&((LVector *)0)->elems, sizeof(LString));\n";
        }
        append(sd, "    fun_", CODEGEN_SPECIAL_FUNCTION_ID_ENTRY, "(vm);\n}\n\n");
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

    // After a call to a function a non-local return can come out of. If one is in flight this
    // function is done too: either it is the one being returned from, in which case what it
    // returns is in the VM's buffer and goes out thru its own return channel, or it passes the
    // return on to its caller, see EmitReturn.
    void GenUnwind(int outw) {
        auto lab = Label();
        TrackUseDef(0, 0);
        append(cb, "    if (", vmref(), "ret_unwind_to < 0) goto block", lab, ";\n");
        // Here we are emitting code executed only if we're unwinding, so temp modify the
        // tstack to match that.
        auto tstackbackup = tstack_size;
        // Only a Lobster function can be returned from, the entry function is never the target.
        if (cursf) {
            append(cb, "    if (", vmref(), "ret_unwind_to == ", cursf->parent->idx, ") {\n");
            append(cb, "    ", vmref(), "ret_unwind_to = -1;\n");
            GenReturnValues([&](int i) { return RetBufSlot(i); });
            if (f_ret_kind == RK_VALUE) {
                // Passing it on, with nothing of its own to return.
                cb += "    } else {\n";
                SetNil(cb, "ret");
            }
            cb += "    }\n";
        } else if (f_ret_kind == RK_VALUE) {
            SetNil(cb, "ret");
        }
        EmitReturn(RET_ANY, outw, -1, 0);
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
        if (inw != ArgSlots(sf)) {
            parser.ErrorAt(node_context.back(), "internal error: call to ", Q(f.name),
                           " passes ", inw, " slots where it takes ", ArgSlots(sf));
        }
        if (call.vtable_idx < 0) {
            EmitCall(sf.idx, inw, outw);
            if (sf.returned_thru_to_max >= 0) {
                GenUnwind(outw);
            }
        } else {
            EmitDynDispatch(call.vtable_idx, inw, outw);
            // We get the dispatch from arg 0, since sf is an arbitrary overloads and
            // doesn't necessarily point to the dispatch root (which may not even have an sf).
            auto dispatch_type = call.children[0]->exptype;
            assert(IsUDT(dispatch_type->t));
            auto de = dispatch_type->udt->dispatch_table[call.vtable_idx].get();
            assert(de->dispatch_root && !de->returntype.Null() && de->subudts_size);
            if (de->returned_thru_to_max >= 0) {
                // This works because all overloads of a DD sit under a single Function.
                GenUnwind(outw);
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

    // Calling a helper for something this small costs more than the work itself, and pushes
    // both operands and the result thru memory where the compiler could otherwise keep them in
    // registers, so emit the operator directly instead, see BinExpr. Takes two operands off
    // the stack and leaves the result.
    void GenScalarBinOp(bool isfloat, MathOp op) {
        TrackUseDef(2, 1);
        SetBinOp(isfloat, op, Slot(2), Val(isfloat, Slot(2)), Val(isfloat, Slot(1)));
    }

    // Comparing two structs is a compare per slot, on the raw bits the same way a helper would.
    void GenStructCompare(bool eq, int len) {
        append(cb, "    { long long _c = ", eq ? "1" : "0", ";\n");
        for (int j = 0; j < len; j++) {
            append(cb, "    _c = _c ", eq ? "&&" : "||", " ", IVal(Slot(len * 2 - j)), " ",
                   eq ? "==" : "!=", " ", IVal(Slot(len - j)), ";\n");
        }
        // Only written after all the reads, since the result lands in the first slot of the left
        // hand side.
        SetInt(cb, Slot(len * 2), "_c != 0");
        cb += "    }\n";
    }

    // Reading a field is a load at a constant offset from the object, whose fields sit right
    // behind its header. It lands in the slot the object was in, so where the copy is more than
    // one statement the address is taken first.
    void GenPushField(int offset) {
        TrackUseDef(1, 1);
        if (cpp) {
            append(cb, "    ", Slot(1), " = ", Slot(1), ".oval()->At(", offset, ");\n");
        } else {
            #if RTT_ENABLED
                append(cb, "    { Value *_f = (Value *)((RefObj *)", Slot(1), ".ref + 1) + ",
                       offset, "; ", CopyValueText(Slot(1), "_f[0]"), " }\n");
            #else
                CopyValue(cb, Slot(1),
                          cat("((Value *)((RefObj *)", Slot(1), ".ref + 1))[", offset, "]"));
            #endif
        }
    }

    // Same, once per slot the struct field occupies. The object is only needed to find them, so
    // it is read out of the stack slot the first one lands in before that gets overwritten.
    void GenPushFieldStruct(int offset, int fwidth) {
        TrackUseDef(1, fwidth);
        append(cb, "    {\n    ", cpp ? "auto " : "RefObj *", "_o = ", Slot(1), ".",
               cpp ? "oval()" : "ref", ";\n");
        for (int i = 0; i < fwidth; i++) {
            CopyValue(cb, Slot(1 - i), cpp ? cat("_o->Elems()[", offset + i, "]")
                                           : cat("((Value *)(_o + 1))[", offset + i, "]"));
        }
        cb += "    }\n";
    }

    // Where the stack top is is something we track statically, so popping a slot needs no code
    // at all, just the bookkeeping.
    void GenPopSlot() { TrackUseDef(1, 0); }

    // The C expression for how many times a loop over this value runs, which for a vector or a
    // string comes out of the object itself, see the mirrors of those in Prologue.
    string LenOf(ValueType itertype, string_view v) {
        switch (itertype) {
            case V_INT:
                return IVal(v);
            case V_VECTOR:
                return cpp ? cat(v, ".vval()->len") : cat("((LVector *)", v, ".ref)->len");
            case V_STRING:
                return cpp ? cat(v, ".sval()->len") : cat("((LString *)", v, ".ref)->len");
            default:
                assert(false);
                return {};
        }
    }

    // The loop condition is an increment and a compare, small enough to be worth not calling for
    // the same reasons as GenSimpleBinOp.
    int GenForCond(ValueType itertype) {
        // Reads the counter and the object being iterated, and leaves both for the body.
        TrackUseDef(2, 2);
        auto lab = Label();
        auto len = LenOf(itertype, Slot(1));
        SetInt(cb, Slot(2), cat(IVal(Slot(2)), " + 1"));
        append(cb, "    if (!(", IVal(Slot(2)), " < ", len, ")) goto block", lab, ";\n");
        return lab;
    }

    // Both of these copy the loop counter to the top of the stack, which is a whole Value and
    // so carries any runtime type field with it.
    void GenForCounter(int useslots, int defslots) {
        TrackUseDef(useslots, defslots);
        CopyValue(cb, Slot(0), Slot(2));
    }

    // The element the loop is on, at the counter below the object being iterated. The loop
    // condition already established the counter is in range, so this needs no check.
    // The bitmask says which slots of the element hold a reference the loop now owns.
    void GenForElem(bool isstring, int defslots, int bitmask) {
        TrackUseDef(2, defslots);
        // Everything but the counter and the object being iterated is the element.
        auto width = defslots - 2;
        auto idx = IVal(Slot(2));
        if (isstring) {
            auto data = cpp ? cat("((unsigned char *)", Slot(1), ".sval()->data())")
                            : cat("LSTRING_DATA((LString *)", Slot(1), ".ref)");
            SetInt(cb, Slot(0), cat("(long long)", data, "[", idx, "]"));
            return;
        }
        auto elems = cpp ? cat(Slot(1), ".vval()->Elems()")
                         : cat("((LVector *)", Slot(1), ".ref)->elems");
        if (width > 1) {
            // A struct element is the same load per slot it occupies, at the width the vector
            // holds them at, which is what the element type says it is.
            append(cb, "    {\n    Value *_e = ", elems, " + ", idx, " * ", width, ";\n");
            for (int i = 0; i < width; i++) CopyValue(cb, Slot(-i), cat("_e[", i, "]"));
            cb += "    }\n";
            for (int i = 0; i < width; i++) if ((1 << i) & bitmask) GenIncRef(Slot(-i));
            return;
        }
        CopyValue(cb, Slot(0), cat(elems, "[", idx, "]"));
        if (bitmask & 1) GenIncRef(Slot(0));
    }

    // Reading an element out of a vector, or just the part of it asked for, with the index
    // arbitrary, unlike the loop above. Indexing with a struct steps thru nested vectors, see
    // GenVectorDescent.
    void GenPushIdxNested(int levels, int elemwidth, int subwidth, int offset) {
        // The vector plus one index per level it steps thru, replaced by the element.
        TrackUseDef(levels + 1, subwidth);
        GenVectorDescent(levels, Slot(levels + 1));
        for (int i = 0; i < subwidth; i++) {
            CopyValue(cb, Slot(levels + 1 - i),
                      cat(Elems(), "[", ElemIndex(elemwidth, offset + i), "]"));
        }
        cb += "    }\n";
    }

    // The same for a single level, or for a string. The object is read out into a local first,
    // since the element lands in the slot it came from.
    void GenPushIdx(bool str, int elemwidth, int subwidth, int offset) {
        // The object and the index it is subscripted with, replaced by the element.
        TrackUseDef(2, subwidth);
        // A string index may read the terminating 0-byte, one past its length.
        auto bound = str ? "_o->len + 1" : "_o->len";
        if (cpp) {
            append(cb, "    {\n    auto _o = ", Slot(2), ".", str ? "sval()" : "vval()",
                   "; auto _i = ", Slot(1), ".ival();\n");
            append(cb, "    if ((uint64_t)_i >= (uint64_t)(", bound, ")) vm.IDXErr(_i, ", bound,
                   ", _o);\n");
        } else {
            append(cb, "    {\n    ", str ? "LString" : "LVector", " *_o = (",
                   str ? "LString" : "LVector", " *)", Slot(2), ".ref; long long _i = ", Slot(1),
                   ".ival;\n");
            append(cb, "    if ((unsigned long long)_i >= (unsigned long long)(", bound,
                   ")) IDXErr(vm, _i, ", bound, ", &_o->ro);\n");
        }
        if (str) {
            SetInt(cb, Slot(2), cpp ? "(long long)((unsigned char *)_o->data())[_i]"
                                    : "LSTRING_DATA(_o)[_i]");
        } else {
            for (int i = 0; i < subwidth; i++) {
                CopyValue(cb, Slot(2 - i), cat(Elems(), "[", ElemIndex(elemwidth, offset + i), "]"));
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
                // A struct of scalars is just slots to give up.
                TrackUseDef(typelt.type->udt->numslots, 0);
            }
        } else {
            if (ShouldDec(typelt)) EmitPopRef(); else GenPopSlot();
        }
    }

    void GenDup(TypeLT tlt) {
        TrackUseDef(1, 2);
        CopyValue(cb, Slot(0), Slot(1));
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

    // The modifiers read/modify/write the lvalue (see Lval) with the operator they are, the
    // same deal as GenScalarBinOp.
    void GenLvalModifier(LvalOp op, TypeRef type) {
        auto width = ValWidth(type);
        TrackUseDef(LvalModifierUses(op, width), 0);
        if (op == LV_WRITE) {
            CopyValue(cb, Lval(0), Slot(1));
        } else if (op == LV_WRITEREF) {
            // Whatever was there loses a reference to make way for what is written over it.
            GenDecRef(Lval(0));
            CopyValue(cb, Lval(0), Slot(1));
        } else if (op == LV_WRITEV || op == LV_WRITEREFV) {
            // Same copy, one per slot of the struct being written, preceded by a decrement for
            // each of those slots that holds a reference, which the bitmask says which are.
            if (op == LV_WRITEREFV) {
                auto bitmask = BitMaskForRefStuct(type);
                for (int i = 0; i < width; i++)
                    if ((1 << i) & bitmask) GenDecRef(Lval(i));
            }
            for (int i = 0; i < width; i++)
                CopyValue(cb, Lval(i), Slot(width - i));
        } else if (op == LV_SADD) {
            if (f_lval_kind == LVK_LOCAL) {
                // The old string is an operand, so it loses its reference only once the new
                // one exists.
                auto v = Lval(0);
                append(cb, "    {\n    ", cpp ? "auto" : "LString *", " _s = RtSAdd(vm, ", v,
                       ", ", Slot(1), ");\n");
                GenDecRef(v);
                SetRef(cb, v, "_s", RTT_STRING);
                cb += "    }\n";
            } else {
                // Appending to a string in memory can free the old one, so it stays a call.
                append(cb, "    RtLvSAdd(vm, ", LvalPtr(), ", ", Slot(1), ");\n");
            }
        } else if (op >= LV_IPP) {
            auto isfloat = op >= LV_FPP;
            auto c = op == LV_IPP || op == LV_FPP ? " + 1" : " - 1";
            if (isfloat) SetFloat(cb, Lval(0), FVal(Lval(0)) + c);
            else SetInt(cb, Lval(0), IVal(Lval(0)) + c);
        } else if (op >= LV_BINAND && op <= LV_ASR) {
            SetInt(cb, Lval(0), BitExpr(BitOp(op - LV_BINAND), IVal(Lval(0)), IVal(Slot(1))));
        } else {
            // The arithmetic families, each in MathOp order: an int or float scalar, or a
            // struct of either with a struct or a scalar on the right, one operator per slot.
            bool isfloat, isvec = true, withscalar = false;
            MathOp mop;
            if (op <= LV_IMOD) {
                isfloat = false; isvec = false; mop = MathOp(op - LV_IADD);
            } else if (op <= LV_FMOD) {
                isfloat = true; isvec = false; mop = MathOp(op - LV_FADD);
            } else if (op <= LV_IVVMOD) {
                isfloat = false; mop = MathOp(op - LV_IVVADD);
            } else if (op <= LV_FVVMOD) {
                isfloat = true; mop = MathOp(op - LV_FVVADD);
            } else if (op <= LV_IVSMOD) {
                isfloat = false; withscalar = true; mop = MathOp(op - LV_IVSADD);
            } else {
                isfloat = true; withscalar = true; mop = MathOp(op - LV_FVSADD);
            }
            auto n = isvec ? width : 1;
            for (int i = 0; i < n; i++) {
                auto rhs = isvec && !withscalar ? Slot(width - i) : Slot(1);
                SetBinOp(isfloat, mop, Lval(i), Val(isfloat, Lval(i)), Val(isfloat, rhs));
            }
        }
        GenLvalWriteBack(type);
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
                case V_VECTOR: {
                    // An int index is a single level, a struct one a level per component.
                    auto levels = ValWidth(indexing->index->exptype);
                    auto etype = indexing->object->exptype;
                    for (int i = 0; i < levels; i++) etype = etype->Element();
                    EmitLvalVectorIndex(levels, offset, ValWidth(etype));
                    break;
                }
                case V_CLASS:
                    assert(indexing->index->exptype->t == V_INT &&
                           indexing->object->exptype->udt->sametype->Numeric());
                    EmitLvalClassIndex(offset);
                    assert(!IsStruct(type->t));
                    break;
                case V_STRUCT_R:
                case V_STRUCT_S:
                    assert(indexing->index->exptype->t == V_INT &&
                           indexing->object->exptype->udt->sametype->Numeric());
                    EmitLvalStructIndex(offset, indexing->object->exptype->udt->numslots);
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
            auto width = ValWidth(type);
            TrackUseDef(0, width);
            for (int i = 0; i < width; i++) CopyValue(cb, Slot(-i), Lval(i));
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
        auto nstrs = (int)strs.size();
        TrackUseDef(nstrs, 1);
        if (nstrs == 2) {
            SetRef(cb, Slot(2), cat("RtSAdd(vm, ", Slot(2), ", ", Slot(1), ")"), RTT_STRING);
        } else {
            auto strs = StageArgs(nstrs);
            SetRef(cb, Slot(nstrs), cat("RtStrConcatN(vm, ", strs, ", ", nstrs, ")"),
                   RTT_STRING);
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
            GenScalarBinOp(false, op);
        } else if (rtype->t == V_FLOAT && ltype->t == V_FLOAT) {
            GenScalarBinOp(true, op);
        } else if (rtype->t == V_STRING && ltype->t == V_STRING) {
            // Only comparisons get here, concatenation has its own path. Nillable version
            // handled below.
            assert(op >= MOP_LT);
            TrackUseDef(2, 1);
            SetInt(cb, Slot(2), cat(MathOpName("S", op), "(", Slot(2), ", ", Slot(1), ")"));
        } else if ((rtype->t == V_FUNCTION && ltype->t == V_FUNCTION)) {
            assert(op == MOP_EQ || op == MOP_NE);
            TrackUseDef(2, 1);
            SetInt(cb, Slot(2), cat(IpOf(Slot(2)), op == MOP_EQ ? " == " : " != ",
                                    IpOf(Slot(1))));
        } else if ((rtype->t == V_TYPEID && ltype->t == V_TYPEID)) {
            assert(op == MOP_EQ || op == MOP_NE);
            GenScalarBinOp(false, op);
        } else if (op >= MOP_EQ) {
            if (IsStruct(ltype->t)) {
                // Comparing two structs is one compare per slot, so this never becomes a
                // call at all.
                auto width = ValWidth(ltype);
                TrackUseDef(width * 2, 1);
                GenStructCompare(op == MOP_EQ, width);
            } else {
                assert(IsRefNil(ltype->t) && IsRefNil(rtype->t));
                TrackUseDef(2, 1);
                if ((ltype->t == V_NIL && ltype->sub->t == V_STRING) ||
                    (rtype->t == V_NIL && rtype->sub->t == V_STRING)) {
                    SetInt(cb, Slot(2), cat(MathOpName("Sn", op), "(", Slot(2), ", ", Slot(1),
                                            ")"));
                } else {
                    // References compare by identity.
                    SetInt(cb, Slot(2), cat(AnyOf(Slot(2)), op == MOP_EQ ? " == " : " != ",
                                            AnyOf(Slot(1))));
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
            auto isfloat = sub->t == V_FLOAT;
            TrackUseDef(inw, outw);
            // The same operator once per slot of the struct, with the results landing where
            // the left operand was.
            if (!withscalar) {
                for (int j = 0; j < width; j++) {
                    auto a = Slot(width * 2 - j);
                    SetBinOp(isfloat, op, a, Val(isfloat, a), Val(isfloat, Slot(width - j)));
                }
            } else if (leftisvec) {
                for (int j = 0; j < width; j++) {
                    auto a = Slot(width + 1 - j);
                    SetBinOp(isfloat, op, a, Val(isfloat, a), Val(isfloat, Slot(1)));
                }
            } else {
                // The scalar sits below the struct, in the slot the first result lands in, so
                // it is read into a local first.
                append(cb, "    { ", isfloat ? "double" : "long long", " _s = ",
                       Val(isfloat, Slot(width + 1)), ";\n");
                for (int j = 0; j < width; j++) {
                    SetBinOp(isfloat, op, Slot(width + 1 - j), "_s",
                             Val(isfloat, Slot(width - j)));
                }
                cb += "    }\n";
            }
        }
    }

    void GenBitOp(const BinOp *n, size_t retval, BitOp op) {
        Gen(n->left, retval);
        Gen(n->right, retval);
        if (retval) {
            TakeTemp(2, false);
            TrackUseDef(2, 1);
            SetInt(cb, Slot(2), BitExpr(op, IVal(Slot(2)), IVal(Slot(1))));
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
            EmitLvalLocal(sid.Idx() + offset);
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
            // The field is on the stack already as part of the struct, so this moves its slots
            // down to where the struct starts, in ascending order since those overlap.
            TrackUseDef(swidth, fwidth);
            auto base = regso - swidth;
            if (offset) {
                for (int i = 0; i < fwidth; i++)
                    CopyValue(cb, SlotVar(base + i), SlotVar(base + offset + i));
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
                auto elemwidth = ValWidth(etype);
                // An int index is a single level, a struct one a level per component.
                auto levels = ValWidth(index->exptype);
                // Either the whole element or just the part of it asked for.
                auto subwidth = struct_elem_sub_width < 0 ? elemwidth : struct_elem_sub_width;
                auto suboffset = struct_elem_sub_width < 0 ? 0 : struct_elem_sub_offset;
                if (levels == 1) GenPushIdx(false, elemwidth, subwidth, suboffset);
                else GenPushIdxNested(levels, elemwidth, subwidth, suboffset);
                break;
            }
            case V_STRUCT_S: {
                auto width = ValWidth(object->exptype);
                assert(index->exptype->t == V_INT && object->exptype->udt->sametype->Numeric());
                TrackUseDef(width + 1, 1);
                auto vals = StageRange(regso - width - 1, width);
                SetValue(cb, Slot(width + 1), cat("RtIndexStruct(vm, ", vals, ", ",
                                                   IVal(Slot(1)), ", ", width, ")"));
                break;
            }
            case V_STRING:
                assert(index->exptype->t == V_INT);
                GenPushIdx(true, 1, 1, 0);
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
            cg.SetInt(cg.cb, cg.Slot(1), "-" + cg.IVal(cg.Slot(1)));
            break;
        case V_FLOAT:
            cg.TrackUseDef(1, 1);
            cg.SetFloat(cg.cb, cg.Slot(1), "-" + cg.FVal(cg.Slot(1)));
            break;
        case V_STRUCT_S: {
            auto isfloat = ctype->udt->sametype->t == V_FLOAT;
            auto width = ValWidth(ctype);
            cg.TrackUseDef(width, width);
            for (int i = 0; i < width; i++) {
                auto v = cg.Slot(width - i);
                if (isfloat) cg.SetFloat(cg.cb, v, "-" + cg.FVal(v));
                else cg.SetInt(cg.cb, v, "-" + cg.IVal(v));
            }
            break;
        }
        default: assert(false);
    }
}

void BitAnd    ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, BIT_AND); }
void BitOr     ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, BIT_OR); }
void Xor       ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, BIT_XOR); }
void ShiftLeft ::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, BIT_ASL); }
void ShiftRight::Generate(CodeGen &cg, size_t retval) const { cg.GenBitOp(this, retval, BIT_ASR); }

void Negate::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, false);
    cg.TrackUseDef(1, 1);
    cg.SetInt(cg.cb, cg.Slot(1), "~" + cg.IVal(cg.Slot(1)));
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
            auto ti = (int)cg.GetTypeTableOffset(child->exptype->ElementIfNil());
            cg.TrackUseDef(1, 1);
            cg.SetRef(cg.cb, cg.Slot(1), cat("RtToString(vm, ", cg.Slot(1), ", (type_elem_t)",
                                             ti, ")"), RTT_STRING);
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
            append(cg.cb, "    switch (GetTypeSwitchID(vm, ", cg.Slot(1), ", ", vtable_idx, ")) {\n");
        } else {
            append(cg.cb, "    { int top = GetTypeSwitchID(vm, ", cg.Slot(1), ", ", vtable_idx, "); switch (top) {\n");
        }
    } else {
        if (cg.cpp) {
            append(cg.cb, "    switch (", cg.IVal(cg.Slot(1)), ") {\n");
        } else {
            append(cg.cb, "    { long long top = ", cg.IVal(cg.Slot(1)), "; switch (top) {\n");
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
    auto elems = cg.StageArgs(arg_width);
    cg.SetRef(cg.cb, cg.Slot(arg_width), cat("RtNewVec(vm, ", elems, ", (type_elem_t)",
                                             (int)offset, ", ", (int)Arity(), ")"), RTT_VECTOR);
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
    if (sf == cg.cursf) {
        cg.EmitReturn(RET_LOCAL, nretslots, -1, nretslots);
    } else {
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
