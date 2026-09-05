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

    // C/C++ codegen related.
    string &c_codegen;
    string cb;
    // The builtins the code calls, by index, which get declared at the spot in the prologue
    // reserved for them once it is known which they are.
    map<int, NativeFun *> natives_used;
    size_t natives_decl_offset = 0;
    // The object types whose fields the code names, which get a struct of their own at the
    // same spot, see UDTName, with the members it has, see Members.
    struct UDTMember;
    map<int, pair<const UDT *, vector<UDTMember>>> udts_used;
    // Whether the function calls a builtin that gets the profiler hooks, see EmitNativeCall.
    bool f_uses_pctx = false;
    int regso = 0;
    int f_function_idx = -1;
    int f_regs_max = -1;
    // The C type a value is kept in, which its static type decides: the scalars as themselves,
    // every reference as a pointer to its header, and a function value as a pointer to it. Only
    // memory that any type can land in, the elements of a vector and the globals, holds Values.
    // The references split by what the static type says they are, so the generated code can
    // use them as such; VK_REF is what is left: a resource, a nil of no type, or any.
    enum VKind { VK_INT, VK_FLOAT, VK_REF, VK_FUN, VK_STRING, VK_VECTOR, VK_OBJECT, VK_COUNT };
    // The runtime type of each slot of a run of values, which says both the kind of variable it
    // is kept in and the tag it carries when it is written to memory, see RtTypeOf.
    typedef vector<RTType> Types;

    // The struct a function with more than one return value comes back in, by its name, which
    // the kinds of those values give it, see RetStruct.
    map<string, Types> rets_used;
    // How each argument of a builtin reaches it, see EmitNativeCall: -1 for one that is a
    // single value, otherwise how many slots its values take, which it is passed as a vector
    // of that width.
    typedef vector<int> NativeArgs;
    // Where a value lives: a variable of its kind, or a Value in memory, which is read thru the
    // field for the kind and written along with the tag its static type says it carries.
    struct Place {
        string s;
        RTType rtt;
        // Whether it is read and written as the type its value is rather than as a Value, and
        // whether it is a variable, which is what lets it be named more than once and be
        // remembered in place of a write, see Defer. A field of an object is the first without
        // being the second.
        bool typed;
        bool var = false;
        int slot = -1;
        VKind k() const { return Kind(rtt); }
    };
    // A value pushed onto the stack, or computed from what is on it, is not written to its
    // slot: the slot remembers the expression instead, and whatever reads the slot uses that
    // in its place, see Defer, which is how the operators of one expression in the program
    // end up as one expression in C. The write is emitted after all where the expression may
    // stop holding: before a write to a variable it reads, and at control flow, see Flush.
    struct Pending {
        Place slot;
        string expr;
        // The variables `expr` reads, which for a constant are none.
        vector<string> vars;
        // The precedence of the operator on top of `expr`, 0 for an atom, see Operand.
        int prec;
    };
    vector<Pending> pending;

    // An expression an emitter builds out of what is on the stack: what it reads, whether it
    // could be deferred (no loads or calls in it), and its precedence.
    struct Expr {
        string text;
        vector<string> vars;
        bool pure;
        int prec;
    };
    // The parameter each slot of the arguments comes in as, in f_args order, and what the
    // function returns, see FunSignature.
    vector<Place> f_arg_places;
    Types f_ret_types;
    // Which kinds of value each stack slot holds, a bit per VKind, which decides the variables
    // the function declares for it, only known at the end of its codegen like f_regs_max.
    vector<int> f_slot_kinds;
    vector<int> f_args;
    vector<int> f_defs;
    // What each temporary that keeps a reference alive holds, see EmitKeep.
    Types f_keeps;
    // The lvalue the op chain currently being emitted produced: a local or global, which need no
    // code at all beyond a note of where the assignment that follows writes to, or an address
    // computed into the "lv" local, which f_uses_lval says the function then needs. A struct
    // occupies consecutive slots from there.
    // A field of an object is neither: it is a member of the struct for its type, which the
    // object it belongs to sits in "lo" for. LVK_NUMPTR is a pointer to slots that hold the one
    // numeric type they are rather than Values, which is what indexing such a field gives.
    enum LvalKind { LVK_NONE, LVK_LOCAL, LVK_GLOBAL, LVK_PTR, LVK_FIELD, LVK_NUMPTR,
                    LVK_ELEM };
    LvalKind f_lval_kind = LVK_NONE;
    int f_lval_idx = 0;
    const UDT *f_lval_udt = nullptr;
    TypeRef f_lval_elem;
    bool f_uses_lval = false;
    bool f_uses_lobj = false;
    bool f_uses_lelem = false;
    vector<int> ownedvars;
    vector<int> funstarttables;
    // The C name of each function, by its SubFunction index, see FunName.
    vector<string> fun_names;
    vector<int> var_to_local;
    // The C variable each local (by var_to_local index) lives in and its kind, and every name
    // the function has handed out, to keep them apart.
    vector<string> local_names;
    Types local_types;
    set<string> f_names_used;
    // The static type of each slot of every variable, by its index in sids.
    vector<TypeRef> var_types;
    // A struct local staged in an array to be indexed at runtime, which goes back into its
    // variables once the modifier has written it, see EmitLvalStructIndex.
    struct { int idx = 0, width = 0; VKind k = VK_INT; } f_writeback;
    // How wide that array has to be, per numeric type one can be of, which is only known at the
    // end of the codegen of the function like f_regs_max is.
    int f_stage_max[2] = { 0, 0 };
    bool has_profile = false;
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
        // What the previous op consumed is gone, whether or not it ever got written.
        for (int i = regso; i < (int)pending.size(); i++) pending[i].expr.clear();
        for (int i = 0; i < useslots; i++) PopTemp();
        for (int i = 0; i < defslots; i++) PushTemp();
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

        for (auto f : st.functiontable) {
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
                for (int i = 0; i < ns; i++) {
                    sids.push_back(metadata::SpecIdent(sid->id->idx, tti, sid->used_as_freevar, sid->idx));
                    var_types.push_back(SlotType(sid->type, i));
                }
            }
        }

        auto max_ser_ids = parser.serializable_id_max + 1;
        ser_ids.resize(max_ser_ids, (type_elem_t)-1);
        for (auto udt : st.udttable) {
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
        natives_decl_offset = c_codegen.size();

        // Start of the actual generated code.
        // Generate a dummmy function for function values that are never called.
        // Would be good if the optimizer guarantees these don't exist, but for now this is
        // more debuggable if it does happen to get called.
        f_function_idx = CODEGEN_SPECIAL_FUNCTION_ID_DUMMY;
        f_regs_max = 0;
        f_args.clear();
        f_defs.clear();
        f_arg_places.clear();
        f_ret_types.clear();
        f_keeps.clear();
        TrackUseDef(0, 0);
        append(cb, "    RtAbort(vm);\n");
        DefineFunction(c_codegen, false);

        // Generate all used functions.
        vector<SubFunction *> sf_used;
        fun_names.resize(st.subfunctiontable.size());
        set<string> fun_names_used;
        for (auto f : st.functiontable) {
            if (!f->istype) {
                for (auto ov : f->overloads) for (auto sf = ov->sf; sf; sf = sf->next) {
                    if (sf->typechecked) {
                        sf_used.push_back(sf);
                        fun_names[sf->idx] = UniqueFunName(*sf, fun_names_used);
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
        f_arg_places.clear();
        f_ret_types.clear();
        f_keeps.clear();
        Gen(parser.root, return_value);
        auto type = parser.root->exptype;
        assert(type->NumValues() == (size_t)return_value);
        if (return_value) {
            TrackUseDef(1, 0);
            auto p = Slot(1, type);
            append(cb, "    RtExit", KindName(p.k()), "(vm, ", ReadTyped(p), ", (type_elem_t)",
                   GetTypeTableOffset(type), ");\n");
        } else {
            TrackUseDef(0, 0);
            append(cb, "    RtExitVoid(vm);\n");
        }
        f_regs_max = (int)tstack_max;
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

        // The builtins the code calls by their symbol, see EmitNativeCall, whose definitions
        // live in other translation units. The C side sees the references they take as
        // pointers, and with MSVC gets a Value returned thru a pointer it passes first, see
        // SretValues.
        string decls;
        for (auto [idx, nf] : natives_used) {
            auto rt = NativeRetCType(nf);
            auto sep = rt.back() == '*' ? "" : " ";
            if (cpp) {
                append(decls, "extern \"C\" ", rt, sep, nf->symbol, "(VMRef");
            } else if (SretValues(nf)) {
                append(decls, "void ", nf->symbol, "(", rt, " *, VMRef");
            } else {
                append(decls, rt, sep, nf->symbol, "(VMRef");
            }
            // The values it does not return it writes thru a pointer of the type each is.
            for (int i = 0; i < nf->OutValues(); i++) {
                append(decls, ", ", NativeArgCType(nf->RetValKind(i), nf->RetValWidth(i)), " *");
            }
            for (size_t i = 0; i < nf->args.size(); i++) {
                auto kind = nf->ArgKind(i);
                auto width = kind == BAK_IVEC || kind == BAK_FVEC ? nf->ArgWidth(i) : 0;
                append(decls, ", ", NativeArgCType(kind, width));
            }
            decls += ");\n";
        }
        // The structs the functions that return several values return, see RetStruct.
        for (auto &[name, rets] : rets_used) {
            decls += "typedef struct {\n";
            for (auto [i, r] : enumerate(rets)) {
                auto ct = CType(Kind(r));
                append(decls, "    ", ct, ct.back() == '*' ? "" : " ", "r", i, ";\n");
            }
            append(decls, "} ", name, ";\n");
        }
        // The object types whose fields the code names, see UDTName. Each is the header every
        // reference carries and a Value per slot, which is what an object is; Entry() checks
        // the size of both of those, so nothing here needs a check of its own.
        for (auto &[idx, um] : udts_used) {
            auto &[udt, members] = um;
            decls += "typedef struct {\n";
            if (!udt->g.is_struct) decls += "    int typeinfo;\n    int refc;\n";
            for (auto &m : members) {
                append(decls, "    ", m.ctype, m.ctype.back() == '*' ? "" : " ", m.name);
                if (m.count > 1) append(decls, "[", m.count, "]");
                decls += ";\n";
            }
            append(decls, "} ", UDTName(*udt), ";  // ", udt->name, "\n");
        }
        if (!decls.empty()) c_codegen.insert(natives_decl_offset, decls + "\n");
    }

    // FIXME: remove.
    void Dummy(size_t retval) {
        assert(!retval);
        while (retval--) EmitPushNil(type_undefined);
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
        f_keeps.clear();  // Not valid until end of codegen of this function.

        auto ret = AssertIs<Return>(sf.sbody->children.back());
        auto ir = sf.consumes_vars_on_return ? AssertIs<IdentRef>(ret->child) : nullptr;

        #ifndef NDEBUG
            var_to_local.clear();
            var_to_local.resize(sids.size(), -1);
        #endif
        local_names.clear();
        local_types.clear();
        f_names_used.clear();
        f_arg_places.clear();
        f_ret_types = ReturnTypes(sf);
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
                    auto vtype = var_types[varidx];
                    if (!sids[varidx].used_as_freevar()) {
                        var_to_local[varidx] = numlocals++;
                        local_names.push_back(LocalName(*arg.sid, i));
                        local_types.push_back(RtTypeOf(vtype));
                        if (&f_ad == &f_args) f_arg_places.push_back(Local(numlocals - 1));
                    } else if (&f_ad == &f_args) {
                        // Lives in a global while the function runs, so the parameter only
                        // holds the old value of that meanwhile, see DefineFunction.
                        f_arg_places.push_back(Var(LocalName(*arg.sid, i), RtTypeOf(vtype)));
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
                "typedef lobster::object_dec_t object_dec_t;\n"
                "typedef lobster::type_elem_t type_elem_t;\n"
                "typedef lobster::RefObj RefObj;\n"
                "typedef lobster::LObject LObject;\n"
                "typedef lobster::LVector LVector;\n"
                "typedef lobster::LString LString;\n"
                "typedef lobster::LResource LResource;\n"
                "\n"
                // A program is free to assign a variable it never reads, or to compare a
                // variable with itself.
                "#if defined(__clang__) || defined(__GNUC__)\n"
                "    #pragma GCC diagnostic ignored \"-Wunused-but-set-variable\"\n"
                "    #pragma GCC diagnostic ignored \"-Wunused-variable\"\n"
                "    #pragma GCC diagnostic ignored \"-Wtautological-compare\"\n"
                "#endif\n"
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
                "typedef struct LString LString;\n"
                "typedef struct LVector LVector;\n"
                "typedef struct LObject LObject;\n"
                "typedef struct {\n"
                "    union {\n"
                "        long long ival;\n"
                "        double fval;\n"
                "        RefObj *ref;\n"
                "        LString *sval;\n"
                "        LVector *vval;\n"
                "        LObject *oval;\n"
                "    };\n"
                "} Value;\n"
                // These need to correspond to the C++ LVector, LString and LObject, enforced in
                // Entry(). We mirror them so that reading a length, an element or a field is a
                // load rather than a call, and they carry the header inline rather than as a
                // RefObj so a reference count is at the same name whatever the type. Only the
                // fields up to the last one we read have to be right, but keeping the whole type
                // here means the size check covers the tail as well.
                "struct LVector {\n"
                "    int typeinfo;\n"
                "    int refc;\n"
                "    long long len;\n"
                "    long long maxl;\n"
                "    long long width;\n"
                "    Value *elems;\n"
                "};\n"
                "struct LString {\n"
                "    int typeinfo;\n"
                "    int refc;\n"
                "    long long len;\n"
                "};\n"
                "struct LObject {\n"
                "    int typeinfo;\n"
                "    int refc;\n"
                "};\n"
                // The characters of a string follow directly behind its header. Not a trailing
                // array member, since the C compilers we feed this to do not agree on what
                // those do to the size of the struct. The fields of an object follow behind
                // its own header the same way, which is what the struct emitted per object
                // type says, see UDTName.
                "#define STRING_DATA(S) ((unsigned char *)((S) + 1))\n";
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
            // The numeric struct a builtin takes an argument of that type as, which mirrors the
            // C++ vec of the same element type and width, so it is passed the same way, along
            // with the helper that makes one, since C has no constructors.
            // The fields are named rather than an array, since a C compiler may pass a struct
            // holding an array differently from one holding the same fields by name, and the
            // C++ side is the latter, see the vec union in geom.h.
            for (auto isint : { true, false }) {
                for (auto w = 1; w <= 4; w++) {
                    auto elem = isint ? "long long" : "double";
                    auto name = cat(isint ? "ivec" : "fvec", w);
                    append(sd, "typedef struct { ", elem);
                    for (auto i = 0; i < w; i++) append(sd, i ? ", " : " ", VecField(i));
                    append(sd, "; } ", name, ";\nstatic ", name, " mk", name, "(");
                    for (auto i = 0; i < w; i++) append(sd, i ? ", " : "", elem, " a", i);
                    append(sd, ") { ", name, " r;");
                    for (auto i = 0; i < w; i++) append(sd, " r.", VecField(i), " = a", i, ";");
                    sd += " return r; }\n";
                }
            }
            sd +=
                "typedef Value *StackPtr;\n"
                "typedef VMBase *VMRef;\n"
                "typedef void (*fun_base_t)(VMRef);\n"
                "typedef void (*object_dec_t)(VMRef, LObject *);\n"
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

            // A value of a type only known at runtime, which is all a handful of builtins take
            // and one returns, made from what the slot it comes from holds. A helper rather than
            // a compound literal, which libtcc compiles into a call to memset. One per name
            // KindName hands out, the reference one taking the type every reference has.
            for (auto k : { VK_INT, VK_FLOAT, VK_FUN, VK_REF }) {
                append(sd, "static Value mkval", KindName(k), "(", CType(k), " a) { Value v; v.",
                       Member(k), " = ", k == VK_FUN ? "(long long)a" : "a", "; return v; }\n");
            }
            // Every runtime helper the generated code can call. These mirror the Rt functions in
            // vmops.h, which is what the JIT links them to, see vm_ops_jit_table. Where the
            // C++ side takes any reference, it is void here, since the generated code holds
            // them as the type they are.
            sd +=
                "LString *RtPushStr(VMRef, int);\n"
                #if LOBSTER_NATIVE_PROFILE
                "struct ___tracy_c_zone_context RtNativeProfileStart(VMRef, int);\n"
                "void RtNativeProfileEnd(struct ___tracy_c_zone_context);\n"
                #endif
                "LVector *RtNewVec(VMRef, type_elem_t, int);\n"
                "LObject *RtNewObject(VMRef, type_elem_t);\n"
                "void RtVectorGrow(VMRef, LVector *);\n"
                "void RtVectorResize(VMRef, LVector *, long long);\n"
                "void RtVectorEmptyErr(VMRef, int);\n"
                "void RtVectorIdxErr(VMRef, int, long long, long long);\n"
                "void RtVectorErase(LVector *, long long);\n"
                "void RtVectorInsert(VMRef, LVector *, int, long long);\n"
                "void RtExitInt(VMRef, long long, type_elem_t);\n"
                "void RtExitFloat(VMRef, double, type_elem_t);\n"
                "void RtExitFun(VMRef, fun_base_t, type_elem_t);\n"
                "void RtExitRef(VMRef, RefObj *, type_elem_t);\n"
                "void RtExitVoid(VMRef);\n"
                "void RtAbort(VMRef);\n"
                "long long RtIDiv(VMRef, long long, long long);\n"
                "long long RtIMod(VMRef, long long, long long);\n"
                "double RtFMod(double, double);\n"
                "double RtSqrt(double);\n"
                "LString *RtSAdd(VMRef, LString *, LString *);\n"
                "long long RtSLt(LString *, LString *);\n"
                "long long RtSGt(LString *, LString *);\n"
                "long long RtSLe(LString *, LString *);\n"
                "long long RtSGe(LString *, LString *);\n"
                "long long RtSEq(LString *, LString *);\n"
                "long long RtSNe(LString *, LString *);\n"
                "long long RtSnEq(LString *, LString *);\n"
                "long long RtSnNe(LString *, LString *);\n"
                "LString *RtStrConcatN(VMRef, LString **, int);\n"
                "LString *RtIntToString(VMRef, long long, type_elem_t);\n"
                "LString *RtFloatToString(VMRef, double, type_elem_t);\n"
                "LString *RtFunToString(VMRef, fun_base_t, type_elem_t);\n"
                "LString *RtRefToString(VMRef, RefObj *, type_elem_t);\n"
                "LString *RtStructToString(VMRef, Value *, type_elem_t);\n"
                "long long RtIsSubType(VMRef, LObject *, int, int, int);\n"
                "fun_base_t RtDynDispatch(VMRef, LObject *, int);\n"
                "void RtEnumRangeErr(VMRef);\n"
                "Value *RtLvalIndexClass(VMRef, LObject *, long long, int);\n"
                "void RtLvSAdd(VMRef, Value *, LString *);\n"
                "int RtStaticSetThisFrame(VMRef, int);\n"
                "int RtMemberSetThisFrame(VMRef, LObject *, int);\n"
                ;

            sd += "extern void Entry(int, int, int, int, int, int);\n"
                  "extern void IDXErr(VMRef, long long, long long, void *);\n"
                  "extern void IDXErrS(VMRef, long long, long long);\n"
                  "extern long long GLFrame(VMRef);\n"
                  "extern void BackupVar(VMRef, int);\n"
                  "extern void DecOwned(VMRef, int);\n"
                  "extern void DecDelete(VMRef, void *);\n"
                  "extern void AssertFailed(VMRef, int, int, int);\n"
                  "extern void RestoreBackup(VMRef, int);\n"
                  "extern int GetTypeSwitchID(VMRef, LObject *, int);\n"
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

    // What a function returns: nothing, a value, or several values in a struct.
    enum RetKind { RK_VOID, RK_VALUE, RK_MULTI };
    static RetKind RetKindOf(int outw) {
        return outw == 0 ? RK_VOID : outw == 1 ? RK_VALUE : RK_MULTI;
    }

    // A letter per kind of value, which is what names the struct several of them come back in.
    static char KindCode(VKind k) {
        static const char codes[] = { 'i', 'f', 'p', 'n', 's', 'v', 'o' };
        return codes[k];
    }

    // The struct a function returns several values in, one per list of kinds, whose members are
    // those values. A struct rather than an array the caller passes a pointer to, since only
    // this way can they stay in registers, and nothing else names the type.
    string RetStruct(const Types &rets) {
        string name = "ret_";
        for (auto r : rets) name += KindCode(Kind(r));
        rets_used.insert({ name, rets });
        return name;
    }

    // The runtime type of a value of this type, for a nilable one that of what it holds. The
    // types that only exist at compile time have no variable to live in, but a nil of one is
    // still pushed here and there.
    static RTType RtTypeOf(TypeRef type) {
        switch (type->t) {
            case V_INT:
            case V_TYPEID: return RTT_INT;
            case V_FLOAT: return RTT_FLOAT;
            case V_FUNCTION: return RTT_FUNCTION;
            case V_STRING: return RTT_STRING;
            case V_VECTOR: return RTT_VECTOR;
            case V_CLASS: return RTT_CLASS;
            case V_RESOURCE: return RTT_RESOURCE;
            case V_STRUCT_R: return RTT_STRUCT_R;
            case V_STRUCT_S: return RTT_STRUCT_S;
            case V_NIL: return type->sub ? RtTypeOf(type->sub) : RTT_NIL;
            default: return RTT_INVALID;
        }
    }

    static VKind Kind(RTType t) {
        switch (t) {
            case RTT_INT: return VK_INT;
            case RTT_FLOAT: return VK_FLOAT;
            case RTT_FUNCTION: return VK_FUN;
            case RTT_STRING: return VK_STRING;
            case RTT_VECTOR: return VK_VECTOR;
            case RTT_CLASS: return VK_OBJECT;
            default: return VK_REF;
        }
    }

    static bool IsRefKind(VKind k) { return k != VK_INT && k != VK_FLOAT && k != VK_FUN; }

    static VKind KindOf(TypeRef type) { return Kind(RtTypeOf(type)); }

    // The runtime type of a kind, for the places whose exact type nothing needs: a reference
    // that is only ever read, or passed on.
    static RTType Rtt(VKind k) {
        static const RTType types[] = { RTT_INT, RTT_FLOAT, RTT_INVALID, RTT_FUNCTION,
                                        RTT_STRING, RTT_VECTOR, RTT_CLASS };
        return types[k];
    }

    // Slot i of a value of this type, which for a struct is one of its fields.
    static TypeRef SlotType(TypeRef type, int i) {
        return IsStruct(type->t) ? FindSlot(*type->udt, i)->type : type;
    }

    // The runtime type of each slot of a value of this type, or of that many of them.
    static void AddTypes(Types &ts, TypeRef type) {
        for (int i = 0; i < ValWidth(type); i++) ts.push_back(RtTypeOf(SlotType(type, i)));
    }
    static Types TypesOf(TypeRef type, size_t nvals) {
        Types ts;
        for (size_t v = 0; v < nvals; v++) AddTypes(ts, type->Get(v));
        return ts;
    }

    string CType(VKind k) {
        static const char *names[] = { "long long", "double", "RefObj *", "fun_base_t",
                                       "LString *", "LVector *", "LObject *" };
        return k == VK_INT && cpp ? "iint" : names[k];
    }

    // The field of a Value in memory a kind is read and written thru, and the accessor the
    // C++ backend uses instead, which allows nil for the references.
    static const char *Member(VKind k) {
        static const char *names[] = { "ival", "fval", "ref", "ival", "sval", "vval", "oval" };
        return names[k];
    }
    static const char *Accessor(VKind k) {
        static const char *names[] = { "ival()", "fval()", "refany()", "ip()", "svalnil()",
                                       "vvalnil()", "ovalnil()" };
        return names[k];
    }
    // The same for reading a value whose tag may still say nil, which is what a global holds
    // before its initializer has run, see DefineFunction.
    static const char *AccessorNil(VKind k) {
        static const char *names[] = { "ivalnil()", "fvalnil()", "refany()", "ipnil()",
                                       "svalnil()", "vvalnil()", "ovalnil()" };
        return names[k];
    }

    static Place Var(string s, RTType rtt) { return { std::move(s), rtt, true, true }; }
    static Place Var(string s, VKind k) { return Var(std::move(s), Rtt(k)); }
    static Place Mem(string s, RTType rtt) { return { std::move(s), rtt, false }; }
    static Place Mem(string s, TypeRef type) { return Mem(std::move(s), RtTypeOf(type)); }
    // Memory that holds the type it says, which is what a field of an object is.
    static Place Direct(string s, RTType rtt) { return { std::move(s), rtt, true }; }

    bool HasPending(int slot) {
        return slot >= 0 && slot < (int)pending.size() && !pending[slot].expr.empty();
    }

    // Remembers `expr` as what slot `d` holds in place of writing it there.
    void Defer(const Place &d, string expr, vector<string> vars, int prec) {
        assert(d.slot >= 0);
        if ((int)pending.size() <= d.slot) pending.resize(d.slot + 1);
        pending[d.slot] = { d, std::move(expr), std::move(vars), prec };
    }
    void Defer(const Place &d, string expr, string var) {
        vector<string> vars;
        if (!var.empty()) vars.push_back(std::move(var));
        Defer(d, std::move(expr), std::move(vars), 0);
    }

    // Emits the write a pending slot stands for, since from here on its expression may not
    // hold any more.
    void Materialize(Pending &p) {
        auto expr = std::move(p.expr);
        p.expr.clear();
        Write(cb, p.slot, expr);
    }

    static bool Reads(const Pending &p, string_view var) {
        for (auto &v : p.vars) if (v == var) return true;
        return false;
    }

    // Every slot still on the stack gets its value, for control flow: whatever reads it past a
    // label can come from either path. The ones above the stack were consumed already.
    void Flush() {
        for (auto [i, p] : enumerate(pending)) {
            if (p.expr.empty()) continue;
            if ((int)i < (int)tstack_size) Materialize(p); else p.expr.clear();
        }
    }

    // The same for the slots that read the variable about to be written, whether it is a
    // local or another slot.
    void FlushVar(string_view var) {
        for (auto &p : pending) if (!p.expr.empty() && Reads(p, var)) Materialize(p);
    }

    // The C precedence of the operators the emitters build expressions from, lower binding
    // tighter: 1 a call or index, 2 a prefix operator or cast, 3 * / %, 4 + -, 5 the shifts,
    // 6 the relational and 7 the equality comparisons, 8 &, 9 ^, 10 |, 11 &&, 12 ||,
    // 13 the conditional operator.
    // Which pairs C groups in a way that reads wrong to most people, and that compilers warn
    // about under -Wparentheses even where the grouping is the one we mean: a comparison in a
    // comparison, arithmetic in a shift, and arithmetic, a comparison or a tighter bitwise
    // operator in a bitwise one.
    static bool Surprising(int prec, int operand) {
        auto comparison = [](int p) { return p == 6 || p == 7; };
        switch (prec) {
            case 5: return operand == 4;
            case 6:
            case 7: return comparison(operand);
            case 8: return operand == 4 || comparison(operand);
            case 9:
            case 10:
                return operand == 4 || comparison(operand) || (operand >= 8 && operand < prec);
            default: return false;
        }
    }

    // An expression as an operand of an operator of precedence `prec`, in parentheses when it
    // binds looser, or as loose on the right, which keeps the grouping, or when the pair would
    // be surprising as written.
    static Expr &Parens(Expr &e, int prec, bool right = false) {
        if (e.prec > prec || (e.prec == prec && right) || Surprising(prec, e.prec)) {
            e.text = cat("(", e.text, ")");
            e.prec = 0;
        }
        return e;
    }

    // A place as an operand of an operator of precedence `prec`. A `prec` past the operators,
    // 15, is a context that groups by itself, such as an argument or a whole condition.
    Expr Operand(const Place &p, int prec, bool right = false) {
        Expr e;
        if (HasPending(p.slot)) {
            auto &q = pending[p.slot];
            e = { q.expr, q.vars, true, q.prec };
        } else if (p.var) {
            e = { p.s, { p.s }, true, 0 };
        } else {
            e = { Read(p), {}, false, 0 };
        }
        return Parens(e, prec, right);
    }

    // What an operator makes of its operands.
    static Expr Combine(int prec, string text, const Expr &a, const Expr &b) {
        Expr e = { std::move(text), a.vars, a.pure && b.pure, prec };
        e.vars.insert(e.vars.end(), b.vars.begin(), b.vars.end());
        return e;
    }

    // A prefix operator on a place; a repeated minus gets parentheses, since two in a row
    // would read as a decrement.
    Expr Unary(string_view op, const Place &v) {
        auto e = Operand(v, 2);
        if (e.text[0] == op[0] && op[0] == '-') e.text = cat("(", e.text, ")");
        e.text = cat(op, e.text);
        e.prec = 2;
        return e;
    }

    // Writes an expression to a slot, which for an int or float that can be deferred is
    // remembering it, unless it has grown long enough to be worth a line of its own.
    void WriteExpr(const Place &d, const Expr &e) {
        if (d.slot >= 0 && e.pure && (d.k() == VK_INT || d.k() == VK_FLOAT) &&
            e.text.size() <= 80) {
            Defer(d, e.text, e.vars, e.prec);
        } else {
            Write(cb, d, e.text);
        }
    }

    // A place, as the C expression of its kind.
    string Read(const Place &p) {
        if (HasPending(p.slot)) return pending[p.slot].expr;
        if (p.typed) return p.s;
        if (cpp) return cat(p.s, ".", Accessor(p.k()));
        if (p.k() == VK_FUN) return cat("(fun_base_t)", p.s, ".ival");
        return cat(p.s, ".", Member(p.k()));
    }

    // The same where the value may still be a nil, which only the C++ backend asserts against,
    // since the C one reads the union field without a tag check either way.
    string ReadNil(const Place &p) {
        if (!cpp || p.typed || HasPending(p.slot)) return Read(p);
        return cat(p.s, ".", AccessorNil(p.k()));
    }

    // The same as a value of another kind, which for two kinds of reference is a cast.
    string ReadAs(const Place &p, VKind k) {
        auto r = Read(p);
        if (p.k() == k || !IsRefKind(k) || !IsRefKind(p.k())) return r;
        return cat("(", CType(k), ")", r);
    }

    // Writing an expression of the place's kind to it. The C++ backend goes thru Value's
    // constructor for memory, which sets the tag from the type, the C one writes the field.
    string WriteText(const Place &d, string_view expr) {
        if (d.slot >= 0) {
            // Only the slots that get written need a variable.
            if ((int)f_slot_kinds.size() <= d.slot) f_slot_kinds.resize(d.slot + 1, 0);
            f_slot_kinds[d.slot] |= 1 << d.k();
        }
        if (d.typed) return cat(d.s, " = ", expr, ";");
        if (cpp) {
            if (IsRefKind(d.k())) {
                return cat(d.s, " = Value(", expr, ");");
            }
            return cat(d.s, " = Value(", expr, ");");
        }
        switch (d.k()) {
            case VK_INT: return cat(d.s, ".ival = ", expr, ";");
            case VK_FLOAT: return cat(d.s, ".fval = ", expr, ";");
            case VK_FUN: return cat(d.s, ".ival = (long long)(", expr, ");");
            default: return cat(d.s, ".", Member(d.k()), " = ", expr, ";");
        }
    }

    void Write(string &sd, const Place &d, string_view expr, string_view lf = "\n") {
        if (HasPending(d.slot)) pending[d.slot].expr.clear();
        if (d.typed) FlushVar(d.s);
        append(sd, "    ", WriteText(d, expr), lf);
    }

    // A copy of one place to another. Between two in memory the C backend copies the field and
    // the tag rather than the struct, since libtcc turns a struct assignment into a memcpy call.
    string CopyValueText(const Place &d, const Place &s) {
        assert(d.k() == s.k() || (IsRefKind(d.k()) && IsRefKind(s.k())));
        if (d.typed || s.typed) return WriteText(d, ReadAs(s, d.k()));
        if (cpp) return cat(d.s, " = ", s.s, ";");
        auto fld = cat(".", Member(s.k()));
        return cat(d.s, fld, " = ", s.s, fld, ";");
    }

    void CopyValue(string &sd, const Place &d, const Place &s, string_view lf = "\n") {
        if (d.slot >= 0 && s.var && &sd == &cb && d.k() == s.k()) {
            // A push of a variable, or of a slot that is itself pending, is only remembered.
            // A computed expression is written first rather than computed twice.
            if (HasPending(s.slot) && pending[s.slot].prec) Materialize(pending[s.slot]);
            if (HasPending(s.slot)) {
                auto &q = pending[s.slot];
                Defer(d, q.expr, q.vars, q.prec);
            } else {
                Defer(d, s.s, s.s);
            }
        } else if (d.typed || s.typed) {
            Write(sd, d, ReadAs(s, d.k()), lf);
        } else {
            append(sd, "    ", CopyValueText(d, s), lf);
        }
    }

    void SetNil(string &sd, const Place &d) {
        if (d.slot >= 0 && &sd == &cb) Defer(d, d.k() == VK_FLOAT ? "0.0" : "0", "");
        else if (d.typed) Write(sd, d, d.k() == VK_FLOAT ? "0.0" : "0");
        else if (cpp) append(sd, "    ", d.s, " = Value(0, lobster::RTT_NIL);\n");
        else append(sd, "    ", d.s, ".ival = 0;\n");
    }

    // A Value a helper returned, as the expression of a kind.
    // A Value a helper returned as the kind the slot it goes into holds.
    string Unbox(string_view expr, VKind k) {
        if (cpp) return cat(expr, ".", Accessor(k));
        if (k == VK_FUN) return cat("(fun_base_t)", expr, ".ival");
        return cat(expr, ".", Member(k));
    }

    void SetValue(string &sd, const Place &d, string_view expr, string_view lf = "\n") {
        assert(d.typed);
        Write(sd, d, Unbox(expr, d.k()), lf);
    }

    // The C signature of a function: its return value if it has one, the array several land in
    // otherwise, then an argument per slot of its arguments. With names for the definition,
    // without for a declaration.
    string FunSignature(string_view name, const Types &args, const Types &rets,
                        const vector<Place> *params) {
        auto rk = RetKindOf((int)rets.size());
        string s = cat("static ", rk == RK_VALUE ? CType(Kind(rets[0]))
                               : rk == RK_MULTI ? RetStruct(rets) : string("void"),
                       " ", name, "(VMRef");
        if (params) s += " vm";
        for (auto [i, a] : enumerate(args)) {
            auto type = CType(Kind(a));
            append(s, ", ", type);
            if (params) append(s, type.back() == '*' ? "" : " ", (*params)[i].s);
        }
        return s + ")";
    }

    // The same as the type of a pointer to one, for calling a function value or the function
    // a dynamic dispatch lands in.
    string FunPtrType(const Types &args, const Types &rets) {
        auto rk = RetKindOf((int)rets.size());
        string s = cat(rk == RK_VALUE ? CType(Kind(rets[0]))
                     : rk == RK_MULTI ? RetStruct(rets) : string("void"), " (*)(VMRef");
        for (auto &a : args) append(s, ", ", CType(Kind(a)));
        return s + ")";
    }

    // The slots the arguments of a function take, and its return values.
    static Types ArgTypes(const SubFunction &sf) {
        Types ks;
        for (auto &arg : sf.args) if (!arg.sid->constprop) AddTypes(ks, arg.sid->type);
        return ks;
    }

    static Types ReturnTypes(const SubFunction &sf) {
        return TypesOf(sf.returntype, sf.returntype->NumValues());
    }

    // A function is named after what it is called in the program, made a C identifier and
    // told apart from its other specializations and overloads by a number. The prefix keeps
    // it clear of everything else in the file, since no local starts with it, see UniqueName.
    string UniqueFunName(const SubFunction &sf, set<string> &used) {
        string base;
        for (auto c : sf.parent->name) {
            if (isalnum((uint8_t)c)) base += c;
            else if (base.empty() || base.back() != '_') base += '_';
        }
        if (!base.empty() && base.back() == '_') base.pop_back();
        if (!base.empty() && base[0] == '_') base.erase(0, 1);
        auto name = "fun_" + base;
        auto unique = name;
        for (int n = 2; used.count(unique); n++) unique = cat(name, "_", n);
        used.insert(unique);
        return unique;
    }

    string FunName(int idx) {
        switch (idx) {
            case CODEGEN_SPECIAL_FUNCTION_ID_DUMMY: return "fun_dummy";
            case CODEGEN_SPECIAL_FUNCTION_ID_ENTRY: return "fun_entry";
            default: assert(!fun_names[idx].empty()); return fun_names[idx];
        }
    }

    void DeclareFunction(SubFunction &sf, string &sd) {
        append(sd, FunSignature(FunName(sf.idx), ArgTypes(sf), ReturnTypes(sf), nullptr), ";\n");
    }

    // A declaration of the variables of one kind, a line per 12 of them to keep it readable.
    void GenDecls(string &sd, VKind k, const vector<string> &names) {
        if (names.empty()) return;
        auto type = CType(k);
        // A pointer type needs its star on each of the variables.
        string star;
        if (type.back() == '*') {
            star = "*";
            type.resize(type.size() - 2);
        }
        for (size_t i = 0; i < names.size(); i++) {
            if (i % 12 == 0) append(sd, i ? ";\n    " : "    ", type, " ");
            else sd += ", ";
            append(sd, star, names[i]);
        }
        sd += ";\n";
    }

    // The same for places of any kind, grouped by it.
    void GenPlaceDecls(string &sd, const vector<Place> &places) {
        for (int k = 0; k < VK_COUNT; k++) {
            vector<string> names;
            for (auto &p : places) if (p.k() == k) names.push_back(p.s);
            GenDecls(sd, VKind(k), names);
        }
    }

    // The name of variable slot `i`, which for a slot of a struct says which one it is.
    string IdName(int i, TypeRef type) {
        auto ididx = sids[i].ididx();
        auto idx = sids[i].idx();
        auto &basename = st.identtable[ididx]->name;
        if (!IsStruct(type->t)) return basename;
        int j = i;
        while (j && sids[j - 1].idx() == idx) j--;
        return cat(basename, "+", i - j);
    }

    // The name of slot `slot` of a struct: the field it is in, and for a nested struct that
    // field's own slot name behind it.
    string StructSlotName(const UDT &udt, int slot) {
        for (auto [k, sfield] : enumerate(udt.sfields)) {
            if (slot >= sfield.slot && slot < sfield.slot + ValWidth(sfield.type)) {
                string name = udt.g.fields[k].id->name;
                if (IsStruct(sfield.type->t)) {
                    append(name, "_", StructSlotName(*sfield.type->udt, slot - sfield.slot));
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
            "vm", "lv", "locals", "ctx", "tsld", "top", "rs", "ret",
            "epilogue", "main", "argc", "argv", "vmmeta", "Value", "VMRef", "StackPtr",
            "RefObj", "LVector", "LString", "LObject", "VMBase", "fun_base_t", "type_elem_t",
            "vtables", "object_decs", "funinfo_table", "compiled_entry_point",
            "type_table", "stringtable",
            "file_names", "function_names", "udts", "specidents", "enums", "ser_ids",
            "subfunctions_to_function", "iint", "int2float64", "lobster", "std", "string_view",
            "span", "uint64_t", "int64_t", "memcpy", "memmove", "GLFrame", "Entry", "IDXErr",
            "IDXErrS", "BackupVar", "DecOwned", "DecDelete", "AssertFailed",
            "RestoreBackup", "GetTypeSwitchID", "PushFunId", "PopFunId", "StartProfile",
            "EndProfile", "STRING_DATA", "pctx",
        };
        if (reserved.count(name)) return true;
        if (name[0] == '_' || name.substr(0, 2) == "Rt" || name.substr(0, 8) == "builtin_")
            return true;
        auto numbered = [&](string_view prefix) {
            if (name.size() <= prefix.size() || name.substr(0, prefix.size()) != prefix) return false;
            for (auto c : name.substr(prefix.size())) if (!isdigit((uint8_t)c)) return false;
            return true;
        };
        return numbered("i") || numbered("f") || numbered("p") || numbered("fn") ||
               numbered("s") || numbered("v") || numbered("o") || numbered("keep") ||
               numbered("block") || numbered("fun_") || name.substr(0, 4) == "udt_";
    }

    // A C name for a local that is unique within the function: the name it has in the program,
    // made a C identifier if it is not one, with a number behind it if that is taken. A prefix
    // the generated code claims for itself gets a letter in front instead, since a number
    // behind it would not lose it.
    string UniqueName(string name) {
        for (auto &c : name) if (!isalnum((uint8_t)c) && c != '_') c = '_';
        if (name.empty() || isdigit((uint8_t)name[0]) || name[0] == '_' ||
            name.substr(0, 2) == "Rt" || name == "fun" || name.substr(0, 4) == "fun_" ||
            name.substr(0, 8) == "builtin_") {
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
        if (IsStruct(sid.type->t)) append(name, "_", StructSlotName(*sid.type->udt, slot));
        return UniqueName(name);
    }

    // The stack slots, counted the way the emitters do: Slot(1) is the top of the stack before
    // the current op, Slot(0) the first one above it, Slot(-1) the one after that. Each is a
    // variable per kind of value it holds, i5, f5, s5, v5, o5, p5 and fn5 for slot 5, which
    // is what lets the C compiler keep them in registers: nothing ever takes their address.
    Place SlotVar(int idx, RTType rtt) {
        static const char *prefix[] = { "i", "f", "p", "fn", "s", "v", "o" };
        auto p = Var(cat(prefix[Kind(rtt)], idx), rtt);
        p.slot = idx;
        return p;
    }
    Place SlotVar(int idx, VKind k) { return SlotVar(idx, Rtt(k)); }
    Place Slot(int off, RTType rtt) { return SlotVar(regso - off, rtt); }
    Place Slot(int off, VKind k) { return SlotVar(regso - off, k); }
    // The slot holding slot i of a value of this type.
    Place Slot(int off, TypeRef type, int i = 0) {
        return SlotVar(regso - off, RtTypeOf(SlotType(type, i)));
    }
    // The array a struct local goes into to be indexed at runtime, one per numeric type its
    // fields can all be of, see EmitLvalStructIndex.
    static string StageArray(VKind k) { return k == VK_FLOAT ? "_lsf" : "_lsi"; }
    // A local variable, a global, and the temporaries a function keeps references alive in. The
    // locals are variables of their own like the stack slots are, see LocalName.
    Place Local(int i) { return Var(local_names[i], local_types[i]); }
    // With stack traces on, every write to a local also lands in an array, since that is where
    // a trace dumps them from, see PushFunId.
    bool ShadowLocals() { return runtime_checks >= RUNTIME_STACK_TRACE; }
    Place Shadow(int i) { return Mem(cat("locals[", i, "]"), local_types[i]); }
    void LocalWritten(int idx, int width) {
        if (!ShadowLocals()) return;
        for (int i = 0; i < width; i++) CopyValue(cb, Shadow(idx + i), Local(idx + i));
    }
    // The C++ backend addresses the VM's own array of globals at a constant offset, which is why
    // that array sits at the end of the VM; the C one has no way to know where that is, so it
    // goes thru the pointer to it that VMBase carries for that purpose.
    Place Global(int varidx) {
        return Mem(cpp ? cat("vm.fvars[", varidx, "]") : cat("vm->fvars_ptr[", varidx, "]"),
                   var_types[varidx]);
    }
    Place KeepVar(int i) { return Var(cat("keep", i), f_keeps[i]); }
    // Slot i of the values a non-local return passes to the function it returns from.
    Place RetBufSlot(int i, RTType rtt) {
        return Mem(cpp ? cat("vm.ret_buf.v[", i, "]") : cat("vm->ret_buf[", i, "]"), rtt);
    }
    // The function's own return value, and slot i of it when it returns several, which are the
    // members of the struct it returns them in, see RetStruct.
    Place RetVar() { return Var("ret", f_ret_types[0]); }
    Place RetSlot(int i, RTType rtt) { return Var(cat("ret.r", i), rtt); }
    void comment(string_view c) { append(cb, " // ", c, "\n"); };
    string_view vmref() { return string_view(cpp ? "vm." : "vm->"); };

    // The operands of a helper that works on a run of values, in an array of its own that it
    // gets a pointer to, since the slots they come from are variables. Declares the array, so
    // whoever calls this opens a block for it and closes it after the call.
    void StageValues(string_view arr, const Types &ts) {
        auto first = regso - (int)ts.size();
        append(cb, "    {\n    Value ", arr, "[", ts.size(), "];\n");
        for (auto [i, t] : enumerate(ts)) {
            CopyValue(cb, Mem(cat(arr, "[", i, "]"), t), SlotVar(first + (int)i, t));
        }
    }

    // The helpers that work on a value of any type come one per kind of value rather than
    // taking a Value, so the name of each ends in the kind it is for, and the value goes in as
    // the type the code holds it as, references all as the one type they have in common.
    static string KindName(VKind k) {
        switch (k) {
            case VK_INT: return "Int";
            case VK_FLOAT: return "Float";
            case VK_FUN: return "Fun";
            default: return "Ref";
        }
    }
    string ReadTyped(const Place &p) {
        return IsRefKind(p.k()) ? ReadAs(p, VK_REF) : Read(p);
    }

    // A typed value as a Value, for the few builtins that take a value whose type is only known
    // at runtime: the C++ backend constructs it, the C one goes thru a maker, see Prologue.
    string Box(const Place &p) {
        if (cpp) return cat("Value(", Read(p), ")");
        return cat("mkval", KindName(p.k()), "(", ReadTyped(p), ")");
    }

    int Label() { return nlabel++; }

    void EmitLabelDef(int lab) {
        TrackUseDef(0, 0);
        Flush();
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

    // A binary operator on scalars is the C operator, except for integer division and modulo,
    // which check their divisor and so have to run where they are, and float modulo, which
    // is fmod.
    Expr BinExpr(bool isfloat, MathOp op, const Place &a, const Place &b) {
        static const char *cops[] = { "+", "-", "*", "/", "%",
                                      "<", ">", "<=", ">=", "==", "!=" };
        static const int precs[] = { 4, 4, 3, 3, 3, 6, 6, 6, 6, 7, 7 };
        if (op == MOP_MOD || (op == MOP_DIV && !isfloat)) {
            auto x = Operand(a, 15), y = Operand(b, 15);
            // The only helper without an argument the C++ backend could find it thru.
            auto call = op == MOP_DIV ? cat("RtIDiv(vm, ", x.text, ", ", y.text, ")")
                      : isfloat ? cat(cpp ? "lobster::" : "", "RtFMod(", x.text, ", ", y.text, ")")
                                : cat("RtIMod(vm, ", x.text, ", ", y.text, ")");
            auto e = Combine(1, call, x, y);
            e.pure = e.pure && isfloat;
            return e;
        }
        auto prec = precs[op];
        auto x = Operand(a, prec), y = Operand(b, prec, true);
        return Combine(prec, cat(x.text, " ", cops[op], " ", y.text), x, y);
    }

    // What one produces: an int for a comparison whatever it compared.
    static VKind BinKind(bool isfloat, MathOp op) {
        return isfloat && op < MOP_LT ? VK_FLOAT : VK_INT;
    }
    static VKind ScalarKind(bool isfloat) { return isfloat ? VK_FLOAT : VK_INT; }

    // The shifts mask their count to the width of an int, see MaskedShiftLeft, and both shift
    // what they are given as 64 bits, which a constant is not by itself.
    Expr BitExpr(BitOp op, const Place &a, const Place &b) {
        switch (op) {
            case BIT_AND:
            case BIT_OR:
            case BIT_XOR: {
                auto prec = op == BIT_AND ? 8 : op == BIT_XOR ? 9 : 10;
                auto x = Operand(a, prec), y = Operand(b, prec, true);
                auto cop = op == BIT_AND ? " & " : op == BIT_XOR ? " ^ " : " | ";
                return Combine(prec, cat(x.text, cop, y.text), x, y);
            }
            case BIT_ASL: {
                auto x = Operand(a, 2), y = Operand(b, 8);
                return Combine(2, cat("(long long)((unsigned long long)", x.text, " << (", y.text,
                                      " & 63))"), x, y);
            }
            default: {
                auto x = Operand(a, 2), y = Operand(b, 8);
                return Combine(5, cat("(long long)", x.text, " >> (", y.text, " & 63)"), x, y);
            }
        }
    }

    void GenPushVar(size_t retval, TypeRef type, int offset, bool used_as_freevar) {
        if (!retval) return;
        auto width = ValWidth(type);
        TrackUseDef(0, width);
        for (int i = 0; i < width; i++) {
            if (used_as_freevar) {
                // A global is addressed by number, so say which it is.
                CopyValue(cb, Slot(-i, type, i), Global(offset + i), "");
                comment(IdName(offset + i, type));
            } else {
                CopyValue(cb, Slot(-i, type, i), Local(var_to_local[offset + i]));
            }
        }
    }

    // Slot i of the lvalue the op chain produced, see f_lval_kind, holding slot i of a value of
    // the given type.
    Place Lval(int i, TypeRef type) {
        switch (f_lval_kind) {
            case LVK_LOCAL: return Local(f_lval_idx + i);
            case LVK_GLOBAL: return Global(f_lval_idx + i);
            case LVK_FIELD: return Field("lo", *f_lval_udt, f_lval_idx + i, SlotType(type, i));
            case LVK_ELEM: return Elem("lvec", f_lval_elem, "lidx", f_lval_idx + i);
            case LVK_NUMPTR:
                return Direct(cat("((", SlotCType(type, i), " *)lv)[", i, "]"),
                              RtTypeOf(SlotType(type, i)));
            default: return Mem(cat("((Value *)lv)[", i, "]"), SlotType(type, i));
        }
    }

    // The same as an address, for the helpers that take one, which a local never has since it
    // is a variable.
    string LvalPtr() {
        switch (f_lval_kind) {
            case LVK_GLOBAL: return cpp ? cat("vm.fvars + ", f_lval_idx)
                                        : cat("vm->fvars_ptr + ", f_lval_idx);
            case LVK_PTR: return "(Value *)lv";
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
        append(cb, "    // lval: ", IdName(offset, type), "\n");
    }

    // The struct the generated code gets for a type, whose members are its fields by the names
    // they have in the program. It is emitted once the code is known to name them, the way the
    // builtins it calls are, see udts_used. An object carries the header every reference does
    // in front of its fields, where a struct is only the fields, since a run of them is what
    // the elements of a vector are, see Elem.
    string UDTName(const UDT &udt) {
        MembersOf(udt);
        auto name = cat(udt.g.is_struct ? "elem_" : "udt_", udt.name, udt.idx);
        for (auto &c : name) if (!isalnum((uint8_t)c) && c != '_') c = '_';
        return name;
    }

    // Whether every slot of a value of this type holds the same C type, which makes a run of
    // them a flat array of it, so that a slot is an index into that and no struct is needed.
    bool UniformSlots(TypeRef type, int width) {
        auto ct = SlotCType(type, 0);
        for (int i = 1; i < width; i++) if (SlotCType(type, i) != ct) return false;
        return true;
    }

    // Slot `slot` of the element at `idx` of a vector whose slots start at `elems`.
    Place Elem(string_view elems, TypeRef elemtype, string_view idx, int slot) {
        auto width = ValWidth(elemtype);
        auto rtt = RtTypeOf(SlotType(elemtype, slot));
        if (UniformSlots(elemtype, width)) {
            auto i = width == 1
                ? (slot ? cat("(", idx, ") + ", slot) : string(idx))
                : cat("(", idx, ") * ", width, slot ? cat(" + ", slot) : string());
            auto ct = SlotCType(elemtype, 0);
            return Direct(cat("((", ct, ct.back() == '*' ? "" : " ", "*)", elems, ")[", i,
                              "]"), rtt);
        }
        // A struct of more than one type is an array of the struct emitted for it.
        return Direct(cat("((", UDTName(*elemtype->udt), " *)", elems, ")[", idx, "].",
                          MemberAt(*elemtype->udt, slot)), rtt);
    }

    // A name from the program as a member of one, kept clear of the names C has of its own.
    // Members are in a namespace of their own, so that is all it takes for one to be unique.
    string MemberName(string name) {
        for (auto &c : name) if (!isalnum((uint8_t)c) && c != '_') c = '_';
        if (name.empty() || isdigit((uint8_t)name[0]) || IsReservedName(name)) name += "_";
        return name;
    }

    // The C type slot `i` of a value of this type holds.
    string SlotCType(TypeRef type, int i) {
        return CType(KindOf(SlotType(type, i)));
    }

    // One member of the struct for an object type, which is one field, or one slot of a field
    // when that field is a struct of more than one type.
    struct UDTMember {
        string name;
        string ctype;
        int slot;
        int count;  // More than one for a field that is an array, see Members.
    };

    // All of them. A field that is a struct of one type over and over is an array of it, which
    // is what lets the program index one at runtime be a real index, see EmitLvalStructIndex.
    vector<UDTMember> Members(const UDT &udt) {
        vector<UDTMember> ms;
        for (auto [k, sfield] : enumerate(udt.sfields)) {
            auto width = ValWidth(sfield.type);
            auto ct = SlotCType(sfield.type, 0);
            auto same = true;
            for (int i = 1; i < width; i++) same = same && SlotCType(sfield.type, i) == ct;
            if (same) {
                ms.push_back({ MemberName(string(udt.g.fields[k].id->name)), ct, sfield.slot,
                               width });
            } else {
                for (int i = 0; i < width; i++) {
                    ms.push_back({ MemberName(StructSlotName(udt, sfield.slot + i)),
                                   SlotCType(sfield.type, i), sfield.slot + i, 1 });
                }
            }
        }
        return ms;
    }

    // Kept once the code is known to name them, since finding one is a walk over the fields.
    const vector<UDTMember> &MembersOf(const UDT &udt) {
        auto it = udts_used.find(udt.idx);
        if (it == udts_used.end()) it = udts_used.insert({ udt.idx, { &udt, Members(udt) } }).first;
        return it->second.second;
    }

    // The member slot `slot` lives in, and where in it when that member is an array.
    string MemberAt(const UDT &udt, int slot) {
        for (auto &m : MembersOf(udt)) {
            if (slot >= m.slot && slot < m.slot + m.count) {
                return m.count > 1 ? cat(m.name, "[", slot - m.slot, "]") : m.name;
            }
        }
        // A slot no field claims, which only a malformed type would have.
        assert(false);
        return cat("slot", slot);
    }

    // The same as an expression at the object `obj`.
    string FieldName(string_view obj, const UDT &udt, int slot) {
        return cat("((", UDTName(udt), " *)", obj, ")->", MemberAt(udt, slot));
    }

    // One of those members as a place. It holds the type the field is, except with the runtime
    // types on, where it is a Value so that it carries the tag, see SlotCType. Either way the
    // runtime only ever reads it a byte at a time, see LoadSlot.
    Place Field(string_view obj, const UDT &udt, int slot, RTType rtt) {
        return Direct(FieldName(obj, udt, slot), rtt);
    }
    Place Field(string_view obj, const UDT &udt, int slot, TypeRef type) {
        return Field(obj, udt, slot, RtTypeOf(type));
    }

    // The member a field that is an array starts at, which is where a runtime index into it
    // goes from, and which decays to a pointer to what it holds.
    string FieldArray(string_view obj, const UDT &udt, int slot) {
        auto s = cat("((", UDTName(udt), " *)", obj, ")->");
        for (auto &m : MembersOf(udt)) {
            if (slot >= m.slot && slot < m.slot + m.count) { s += m.name; return s; }
        }
        assert(false);
        return s;
    }

    // A field as an lvalue is at a constant offset from the object, same as reading one. That
    // does lose a debug only range check.
    void EmitLvalField(const UDT &udt, int slot) {
        TrackUseDef(1, 0);
        // The object goes in a local, since what follows may write the slot it came in, and
        // its fields are members rather than something an address can point at.
        f_uses_lobj = true;
        append(cb, "    lo = ", Read(Slot(1, VK_OBJECT)), ";\n");
        f_lval_kind = LVK_FIELD;
        f_lval_udt = &udt;
        f_lval_idx = slot;
    }

    // The slots the elements of the vector in _o live in, which Elem reads as what they hold.
    string Elems() { return cpp ? "_o->ElemSlots()" : "_o->elems"; }

    // Steps into the vector in `vec` with the indices above it on the stack, one nested vector
    // per level, with a range check per level whose failure path stays a call, leaving the
    // innermost in _o and the index into it in _i, inside a block the caller closes. A struct
    // index has its components used back to front, the first one landing on the element.
    void GenVectorDescent(int levels, const Place &vec) {
        append(cb, "    {\n    LVector *_o = ", Read(vec), ";\n    long long _i;\n");
        for (int j = levels - 1; j >= 0; j--) {
            append(cb, "    _i = ", Read(Slot(levels - j, VK_INT)), ";\n");
            if (cpp) {
                append(cb, "    if ((uint64_t)_i >= (uint64_t)_o->len)"
                           " vm.IDXErr(_i, _o->len, _o);\n");
                if (j) append(cb, "    _o = ((LVector **)_o->ElemSlots())[_i];\n");
            } else {
                append(cb, "    if ((unsigned long long)_i >= (unsigned long long)_o->len)"
                           " IDXErr(vm, _i, _o->len, _o);\n");
                if (j) append(cb, "    _o = ((LVector **)_o->elems)[_i];\n");
            }
        }
    }

    // Indexing to get an lvalue hands the address to whatever follows thru a local rather than
    // thru the VM, so they read as a chain of assignments, and none of them leave anything on
    // the stack. An element of a vector is at the width the vector holds its elements at plus
    // wherever in one the assignment lands.
    void EmitLvalVectorIndex(int levels, int offset, TypeRef etype) {
        TrackUseDef(levels + 1, 0);
        f_uses_lelem = true;
        GenVectorDescent(levels, Slot(levels + 1, VK_VECTOR));
        // The element outlives the block the descent opened, so where it is goes in locals.
        append(cb, "    lvec = ", Elems(), "; lidx = _i;\n    }\n");
        f_lval_kind = LVK_ELEM;
        f_lval_elem = etype;
        f_lval_idx = offset;
    }

    // A class indexed at runtime, whose range check needs the type info, so it stays a helper.
    // Every field of one is of the same numeric type, which is what its slots hold.
    void EmitLvalClassIndex(int offset) {
        TrackUseDef(2, 0);
        f_uses_lval = true;
        append(cb, "    lv = RtLvalIndexClass(vm, ", Read(Slot(2, VK_OBJECT)), ", ",
               Read(Slot(1, VK_INT)), ", ", offset, ");\n");
        f_lval_kind = LVK_NUMPTR;
    }

    // A struct indexed at runtime, the one case that steps into the lvalue it was handed.
    void EmitLvalStructIndex(int offset, int numslots) {
        TrackUseDef(1, 0);
        f_uses_lval = true;
        string base;
        auto typed = f_lval_kind == LVK_FIELD || f_lval_kind == LVK_NUMPTR ||
                     f_lval_kind == LVK_ELEM || f_lval_kind == LVK_LOCAL;
        auto kind = typed ? LVK_NUMPTR : LVK_PTR;
        if (f_lval_kind == LVK_FIELD) {
            // A field of a struct type is an array of what its slots hold, so indexing it is
            // an index into that, see Members.
            base = FieldArray("lo", *f_lval_udt, f_lval_idx);
        } else if (f_lval_kind == LVK_ELEM) {
            // The same for an element, which a vector of one type holds in a flat run of them.
            base = cat("&", Elem("lvec", f_lval_elem, "lidx", f_lval_idx).s);
        } else if (f_lval_kind == LVK_LOCAL) {
            // A struct in variables has to be in memory to be indexed at runtime, so it goes
            // thru an array of the one numeric type all of its fields are, and comes back out
            // once the modifier has written it, see GenLvalWriteBack.
            auto k = Local(f_lval_idx).k();
            f_writeback = { f_lval_idx, numslots, k };
            f_stage_max[k] = std::max(f_stage_max[k], numslots);
            for (int j = 0; j < numslots; j++) {
                auto l = Local(f_lval_idx + j);
                append(cb, "    ", StageArray(k), "[", j, "] = ", Read(l), ";\n");
            }
            base = StageArray(k);
        } else {
            base = LvalPtr();
        }
        append(cb, "    {\n    long long _i = ", Read(Slot(1, VK_INT)), ";\n");
        if (cpp) {
            append(cb, "    if ((uint64_t)_i >= ", numslots, ") vm.IDXErrS(_i, ", numslots,
                   ");\n");
        } else {
            append(cb, "    if ((unsigned long long)_i >= ", numslots, ") IDXErrS(vm, _i, ",
                   numslots, ");\n");
        }
        append(cb, "    lv = ", base, " + _i", offset ? cat(" + ", offset) : string(),
               ";\n    }\n");
        f_lval_kind = kind;
    }

    // What a modifier wrote thru the lvalue lands where it belongs: a staged struct back in
    // its variables, and any local in its shadow, see LocalWritten.
    void GenLvalWriteBack(TypeRef type) {
        if (f_writeback.width) {
            for (int j = 0; j < f_writeback.width; j++) {
                Write(cb, Local(f_writeback.idx + j),
                      cat(StageArray(f_writeback.k), "[", j, "]"));
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
        TrackUseDef(0, 1);
        auto d = SlotVar(regso, RTT_STRING);
        if (STRING_CONSTANTS_KEEP) {
            // Still has a reference to take, so leave it to the helper.
            Write(cb, d, cat("RtPushStr(vm, ", stringtableindex, ")"), "");
        } else {
            // Borrowed, so all that is left is the copy out of the VM's table of them.
            CopyValue(cb, d, Mem(cat(vmref(), "constant_strings_ptr[", stringtableindex, "]"),
                                 RTT_STRING), "");
        }
        comment(q);
    }

    int EmitJump() {
        TrackUseDef(0, 0);
        Flush();
        auto lab = Label();
        append(cb, "    goto block", lab, ";\n");
        return lab;
    }

    int EmitJumpBack(int lab) {
        TrackUseDef(0, 0);
        Flush();
        append(cb, "    goto block", lab, ";\n");
        return lab;
    }

    // Jump on the value on top of the stack testing false, or on it testing true for the
    // "no fail" version. Small enough to be worth not calling for. defslots is 1 when the value
    // stays on the stack for whoever follows.
    // The value the jump keeps as the result can be of another kind than the result, when the
    // typechecker decided only its truth matters, see TypeCheckAndOr, in which case the jump
    // converts it: a false scalar is the nil, and either scalar the other.
    int EmitJumpCond(bool onfail, int defslots, VKind k, VKind resk) {
        TrackUseDef(1, defslots);
        auto lab = Label();
        auto v = Slot(1, k);
        // Read as the operand of the ! it may get, and otherwise as the whole condition, which
        // needs no parentheses of its own. When the value stays on the stack it is written to
        // its slot first, since that write is what its expression may read, and the test then
        // reads the slot; when it is consumed here the expression is used as is, since the
        // flush only drops it.
        if (defslots) Flush();
        auto e = Operand(v, onfail ? 2 : 15);
        if (!defslots) Flush();
        if (k == resk || !defslots) {
            append(cb, "    if (", onfail ? "!" : "", e.text, ") goto block", lab, ";\n");
            return lab;
        }
        // The test stays the whole condition, while the conversion below takes the value as
        // the operand of a cast.
        auto test = e.text;
        auto cond = Parens(e, 2).text;
        string conv;
        switch (resk) {
            case VK_FLOAT: conv = cat("(double)", cond); break;
            case VK_INT: conv = k == VK_FLOAT ? cat("(long long)", cond)
                                              : cat("(long long)(", cond, " != 0)"); break;
            default:
                // Between two kinds of reference only a false one gets kept, but say what it is.
                conv = IsRefKind(k) ? cat("(", CType(resk), ")", cond) : "0";
                break;
        }
        append(cb, "    if (", onfail ? "!" : "", test, ") { ", WriteText(Slot(1, resk), conv),
               " goto block", lab, "; }\n");
        return lab;
    }
    int EmitJumpFail(int defslots, VKind k, VKind resk) {
        return EmitJumpCond(true, defslots, k, resk);
    }
    int EmitJumpNoFail(int defslots, VKind k, VKind resk) {
        return EmitJumpCond(false, defslots, k, resk);
    }
    int EmitJumpFail(int defslots, VKind k) { return EmitJumpCond(true, defslots, k, k); }
    int EmitJumpNoFail(int defslots, VKind k) { return EmitJumpCond(false, defslots, k, k); }

    // Jump over the initializer of a member or static that has already run this frame. The
    // member version reads the object it belongs to off the stack, the static one needs nothing.
    int EmitJumpIfSetThisFrame(bool member, int varidx) {
        auto lab = Label();
        if (member) {
            TrackUseDef(1, 0);
            auto self = Read(Slot(1, VK_OBJECT));
            Flush();
            append(cb, "    if (!RtMemberSetThisFrame(vm, ", self, ", ", varidx, ")) goto block",
                   lab, ";\n");
        } else {
            TrackUseDef(0, 0);
            Flush();
            append(cb, "    if (!RtStaticSetThisFrame(vm, ", varidx, ")) goto block", lab, ";\n");
        }
        return lab;
    }

    // Whether the C code gets a Value a builtin returns thru a pointer it passes as the first
    // argument: MSVC returns a class with constructors that way, where C returns the struct it
    // mirrors Value with in a register. The C++ backend agrees with itself, and only the
    // builtins that return an untyped Value are returned indirectly at all.
    bool SretValues(NativeFun *nf) {
        #ifdef _MSC_VER
            // A numeric struct fails MSVC's rules for a return in a register on its base
            // class alone, and an untyped Value on its private members, so both come back thru
            // a pointer the caller passes first.
            return !cpp && nf->ReturnsValue() &&
                   (nf->RetWidth() || nf->RetKind() == BAK_VALUE);
        #else
            (void)nf;
            return false;
        #endif
    }

    // The profiler hooks around a call to a builtin, when compiled in.
    void EmitNativeProfile(bool start, int nfi) {
        #if LOBSTER_NATIVE_PROFILE
            f_uses_pctx = true;
            auto ns = cpp ? "lobster::" : "";
            if (start) append(cb, "    pctx = ", ns, "RtNativeProfileStart(vm, ", nfi, ");\n");
            else append(cb, "    ", ns, "RtNativeProfileEnd(pctx);\n");
        #else
            (void)start;
            (void)nfi;
        #endif
    }

    // The types a builtin declares its arguments as, see BuiltinSig. The C side has no name
    // for a resource, whose fields it never reads, and holds a reference as a pointer to its
    // header, hence the mirrors of the other three, see Prologue. A numeric struct becomes a
    // vector of its width, which the C side has its own layout compatible type for.
    string NativeArgCType(BuiltinArgKind k, int width) {
        switch (k) {
            case BAK_VALUE:    return "Value";
            case BAK_REF:      return "RefObj *";
            case BAK_FUNCTION: return "fun_base_t";
            case BAK_INT:      return cpp ? "iint" : "long long";
            case BAK_FLOAT:    return "double";
            case BAK_STRING:   return "LString *";
            case BAK_VECTOR:   return "LVector *";
            case BAK_RESOURCE: return cpp ? "LResource *" : "void *";
            case BAK_IVEC:     return cpp ? cat("vec<iint, ", width, ">") : cat("ivec", width);
            case BAK_FVEC:     return cpp ? cat("vec<double, ", width, ">") : cat("fvec", width);
            // Only a builtin the generated code writes out itself takes one of these, so it
            // has no C type of its own, see the check in BuiltinDef.
            case BAK_VALUEVEC: break;
        }
        assert(false);
        return "";
    }

    // The type a builtin returns, which is that of its last return value, see BuiltinRet.
    string NativeRetCType(NativeFun *nf) {
        return nf->ReturnsValue() ? NativeArgCType(nf->RetKind(), nf->RetWidth()) : "void";
    }

    // A typed expression as a value of another kind, which between two kinds of reference is
    // a cast: a builtin declares a resource or a vector where the slot it lands in may know
    // the exact class it holds.
    string CastAs(string_view expr, VKind from, VKind to) {
        if (from == to || !IsRefKind(from) || !IsRefKind(to)) return string(expr);
        return cat("(", CType(to), ")", expr);
    }

    // The kind of value a builtin hands back, for the slot it goes into.
    static VKind NativeValueKind(BuiltinArgKind k) {
        switch (k) {
            case BAK_INT:    return VK_INT;
            case BAK_FLOAT:  return VK_FLOAT;
            case BAK_STRING: return VK_STRING;
            case BAK_VECTOR: return VK_VECTOR;
            case BAK_FUNCTION: return VK_FUN;
            default:         return VK_REF;
        }
    }
    VKind NativeRetKind(NativeFun *nf) { return NativeValueKind(nf->RetKind()); }

    // The name of element `i` of a vector, which both the C mirror and the C++ vec give its
    // fields rather than an array, see Prologue.
    static const char *VecField(int i) {
        static const char *fields[] = { "x", "y", "z", "w" };
        return fields[i];
    }

    // The arguments of a call to a builtin, whose values start at slot `base`. Each is the
    // slot it lives in, as the type the builtin takes it as. A numeric struct becomes a vector
    // built from the slots its values are in.
    string NativeArgList(int base, NativeFun *nf, const Types &args, const NativeArgs &nargs) {
        string s;
        auto slot = base;
        for (auto [i, len] : enumerate(nargs)) {
            auto kind = nf->ArgKind(i);
            if (len >= 0) {
                // C has no constructors, so it makes one thru a helper, see Prologue.
                auto ctor = NativeArgCType(kind, len);
                append(s, ", ", cpp ? ctor : "mk" + ctor, "(");
                for (auto k = 0; k < len; k++) {
                    append(s, k ? ", " : "", Read(SlotVar(slot + k, args[slot + k - base])));
                }
                s += ")";
                slot += len;
                continue;
            }
            auto p = SlotVar(slot, args[slot - base]);
            switch (kind) {
                case BAK_INT:
                case BAK_FLOAT:    append(s, ", ", Read(p)); break;
                case BAK_STRING:   append(s, ", ", ReadAs(p, VK_STRING)); break;
                case BAK_VECTOR:   append(s, ", ", ReadAs(p, VK_VECTOR)); break;
                case BAK_RESOURCE: append(s, ", ", cpp ? "(LResource *)" : "", Read(p)); break;
                case BAK_REF:      append(s, ", ", ReadAs(p, VK_REF)); break;
                case BAK_VALUE:    append(s, ", ", Box(p)); break;
                default:           append(s, ", ", Read(p)); break;
            }
            slot++;
        }
        assert(slot - base == (int)args.size());
        return s;
    }

    // The builtins the generated code writes out itself, and the one that picks between
    // them, which live in a file of their own.
    #include "lobster/codegen_builtin.h"

    // One of the values a builtin hands back, out of the temporary it lands in, into the slots
    // it lives in. A numeric struct takes as many of them as it is wide.
    void EmitNativeValue(const Types &rets, int base, int slot, int width,
                         BuiltinArgKind kind, string_view tmp) {
        for (int i = 0; i < std::max(1, width); i++) {
            auto d = SlotVar(slot + i, rets[slot + i - base]);
            if (width) Write(cb, d, cat(tmp, ".", VecField(i)));
            else if (kind == BAK_VALUE) SetValue(cb, d, tmp);
            else Write(cb, d, CastAs(tmp, NativeValueKind(kind), d.k()));
        }
    }

    // A call to a builtin, which the code makes directly by its symbol, declared in the
    // prologue, see natives_used. It returns its last return value as the type that value is,
    // and writes the ones before it thru a pointer per value that the caller passes ahead of
    // the arguments, so every one of them stays the type it is.
    void EmitNativeCall(NativeFun *nf, const Types &args, const Types &rets,
                        const NativeArgs &nargs, TypeRef elemtype) {
        auto uses = (int)args.size();
        auto defs = (int)rets.size();
        TrackUseDef(uses, defs);
        if (EmitCodegenBuiltin(nf, args, rets, elemtype)) return;
        natives_used[nf->idx] = nf;
        auto sret = SretValues(nf);
        EmitNativeProfile(true, nf->idx);
        auto base = regso - uses;
        auto argstr = NativeArgList(base, nf, args, nargs);
        auto nouts = nf->OutValues();
        auto retslots = nf->RetSlots();
        // Every temporary the call needs goes in a block of its own: one per value it writes
        // thru a pointer, and one for a value it returns that does not go straight into a slot.
        auto tmps = nouts || sret || nf->RetWidth();
        if (tmps) {
            append(cb, "    {");
            comment(nf->name);
        }
        string outs;
        for (int i = 0; i < nouts; i++) {
            auto k = nf->RetValKind(i);
            // A vec has no default constructor, so the C++ backend gives the temporary for one
            // a value it does not use, where the C mirror needs none.
            auto init = cpp && (k == BAK_IVEC || k == BAK_FVEC)
                            ? cat("((", k == BAK_IVEC ? "iint" : "double", ")0)") : string();
            append(cb, "    ", NativeArgCType(k, nf->RetValWidth(i)), " _o", i, init, ";\n");
            append(outs, ", &_o", i);
        }
        auto call = cat(nf->symbol, "(", sret ? "&_nr, " : "", "vm", outs, argstr, ")");
        // The name of the builtin is on the line that opens the block when there is one.
        auto endl = [&]() { if (tmps) cb += "\n"; else comment(nf->name); };
        if (!retslots) {
            append(cb, "    ", call, ";");
            endl();
        } else if (nf->RetWidth() || sret) {
            // A numeric struct is read a field at a time into the slots its values live in, and
            // one the host compiler returns thru a pointer has nowhere else to land.
            auto ct = NativeRetCType(nf);
            if (sret) append(cb, "    ", ct, " _nr;\n    ", call, ";\n");
            else append(cb, "    ", ct, " _nr = ", call, ";\n");
            EmitNativeValue(rets, base, base + defs - retslots, nf->RetWidth(),
                            nf->RetKind(), "_nr");
        } else {
            // The value it returns lands in the last of the slots the call leaves behind.
            auto ret = SlotVar(base + defs - 1, rets[defs - 1]);
            auto e = nf->RetKind() == BAK_VALUE ? Unbox(call, ret.k())
                                               : CastAs(call, NativeRetKind(nf), ret.k());
            Write(cb, ret, e, "");
            endl();
        }
        // What it wrote thru a pointer comes out of the temporaries into their own slots.
        auto slot = base;
        for (int i = 0; i < nouts; i++) {
            EmitNativeValue(rets, base, slot, nf->RetValWidth(i), nf->RetValKind(i),
                            cat("_o", i));
            slot += nf->RetValSlots(i);
        }
        if (tmps) cb += "    }\n";
        EmitNativeProfile(false, nf->idx);
    }

    // Keeps the reference at the given depth in a temporary of its type until the function
    // returns, since the reference is only borrowed by whatever consumes it.
    void EmitKeep(int stack_offset, RTType rtt) {
        // Inside a loop the slot may already hold a reference from a previous iteration.
        auto inloop = !loops.empty();
        TrackUseDef(0, 0);
        auto offset = (int)f_keeps.size();
        f_keeps.push_back(rtt);
        if (inloop) GenDecRef(cb, KeepVar(offset));
        CopyValue(cb, KeepVar(offset), Slot(stack_offset + 1, rtt));
    }

    // The function's return values, from wherever `src` says slot i of them is.
    template<typename F> void GenReturnValues(F src) {
        if (f_ret_types.size() == 1) {
            CopyValue(cb, RetVar(), src(0));
        } else {
            for (auto [i, kr] : enumerate(f_ret_types)) {
                CopyValue(cb, RetSlot((int)i, kr), src((int)i));
            }
        }
    }

    // A return. The values come off the stack into the function's own return channel, or for
    // a non-local return into the VM's buffer, marked for the function they return from,
    // parent_idx, whose frame then sends them out thru its own channel, see GenUnwind. RET_ANY
    // passes such a return on to the caller, which has nothing to copy. `rets` says what the
    // values are, for the function they return from.
    void EmitReturn(ReturnKind kind, const Types &rets, int parent_idx, int useslots) {
        auto nretslots = (int)rets.size();
        TrackUseDef(useslots, 0);
        // FIXME: emit epilogue stuff only once at end of function.
        if (kind == RET_LOCAL) {
            if (nretslots) {
                GenReturnValues([&](int i) { return SlotVar(regso - nretslots + i, rets[i]); });
            } else if (f_ret_types.size() == 1) {
                // A return without a value out of a function whose result is not used.
                SetNil(cb, RetVar());
            }
        } else if (kind == RET_NONLOCAL) {
            if (nretslots > MAX_RETURN_SLOTS) {
                parser.ErrorAt(node_context.back(),
                               "too many values returned thru a non-local return");
            }
            for (int i = 0; i < nretslots; i++) {
                CopyValue(cb, RetBufSlot(i, rets[i]), SlotVar(regso - nretslots + i, rets[i]));
            }
            append(cb, "    ", vmref(), "ret_unwind_to = ", parent_idx, ";\n");
            // Nothing of its own to return, but the caller expects a value all the same.
            if (f_ret_types.size() == 1) SetNil(cb, RetVar());
        }
        for (auto varidx : ownedvars) {
            if (sids[varidx].used_as_freevar()) {
                append(cb, "    DecOwned(vm, ", varidx, ");\n");
            } else {
                GenDecRef(cb, Local(var_to_local[varidx]));
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
        Flush();
        append(cb, "    goto epilogue;\n");
    }

    void EmitPushFun(int fidx) {
        TrackUseDef(0, 1);
        Defer(Slot(0, VK_FUN), cat("(fun_base_t)", FunName(fidx)), "");
    }

    // A call to `callee`, an expression for the function, with the arguments on the stack and
    // the result landing where they were. `uses` is what the call consumes, which is the
    // arguments plus the function value above them when there is one.
    void EmitCallTo(string_view callee, const Types &args, const Types &rets, int uses,
                    string_view cmt = {}) {
        auto defs = (int)rets.size();
        TrackUseDef(uses, defs);
        auto rk = RetKindOf(defs);
        auto base = regso - uses;
        string call = cat(callee, "(vm");
        for (auto [i, a] : enumerate(args)) append(call, ", ", Read(SlotVar(base + (int)i, a)));
        call += ")";
        if (rk == RK_MULTI) {
            // Several results come back in a struct, whose members go into the slots they
            // belong in.
            append(cb, "    {");
            if (cmt.empty()) cb += "\n"; else comment(cmt);
            append(cb, "    ", RetStruct(rets), " _r = ", call, ";\n");
            for (int i = 0; i < defs; i++) {
                Write(cb, SlotVar(base + i, rets[i]), cat("_r.r", i));
            }
            cb += "    }\n";
            return;
        }
        if (rk == RK_VALUE) Write(cb, SlotVar(base, rets[0]), call, "");
        else append(cb, "    ", call, ";");
        if (cmt.empty()) cb += "\n"; else comment(cmt);
    }

    void EmitCall(const SubFunction &sf, int inw) {
        EmitCallTo(FunName(sf.idx), ArgTypes(sf), ReturnTypes(sf), inw,
                   "call: " + Signature(sf));
    }

    // The function value on top of the stack is called with what sits below it, cast to the
    // signature the type checker gave it.
    void EmitCallValue(const Types &args, const Types &rets) {
        TrackUseDef(0, 0);
        auto ptr = cat("((", FunPtrType(args, rets), ")", Read(Slot(1, VK_FUN)), ")");
        EmitCallTo(ptr, args, rets, (int)args.size() + 1);
    }

    // Same, for the function a dispatch on the class of the first argument lands in.
    void EmitDynDispatch(int vtable_idx, const Types &args, const Types &rets) {
        TrackUseDef(0, 0);
        auto ptr = cat("((", FunPtrType(args, rets), ")RtDynDispatch(vm, ",
                       ReadAs(Slot((int)args.size(), args[0]), VK_OBJECT), ", ", vtable_idx, "))");
        EmitCallTo(ptr, args, rets, (int)args.size());
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
    void EmitIsType(int type_idx, int nilres, TypeRef type, TypeRef vtype) {
        TrackUseDef(1, 1);
        auto v = Read(Slot(1, vtype));
        Write(cb, Slot(1, VK_INT),
              cat(v, " ? ", v, cpp ? "->tti == (type_elem_t)" : "->typeinfo == ", type_idx,
                  " : ", nilres), "");
        TypeComment(type);
    }

    void EmitIsSubType(int start, int end, int nilres, TypeRef type, TypeRef vtype) {
        TrackUseDef(1, 1);
        Write(cb, Slot(1, VK_INT), cat("RtIsSubType(vm, ", ReadAs(Slot(1, vtype), VK_OBJECT), ", ", start,
                                      ", ", end, ", ", nilres, ")"), "");
        comment(type->udt->name);
    }

    // A new object or vector gets its fields or elements written into it from wherever they
    // are once it exists. It is held in a local until they all are, since the slot it ends up
    // in is the first of theirs.
    void EmitNewObject(int type_idx, const Types &args, TypeRef type) {
        auto n = (int)args.size();
        TrackUseDef(n, 1);
        auto base = regso - n;
        append(cb, "    {\n    LObject *_o = RtNewObject(vm, (type_elem_t)", type_idx, ");");
        TypeComment(type);
        for (int i = 0; i < n; i++) {
            CopyValue(cb, Field("_o", *type->udt, i, args[i]), SlotVar(base + i, args[i]));
        }
        Write(cb, SlotVar(base, RtTypeOf(type)), "_o");
        cb += "    }\n";
    }

    void EmitNewVec(int type_idx, const Types &args, int len, TypeRef elemtype) {
        auto n = (int)args.size();
        auto width = ValWidth(elemtype);
        TrackUseDef(n, 1);
        auto base = regso - n;
        append(cb, "    {\n    LVector *_v = RtNewVec(vm, (type_elem_t)", type_idx, ", ", len,
               ");\n");
        auto elems = cpp ? "_v->ElemSlots()" : "_v->elems";
        for (int i = 0; i < n; i++) {
            CopyValue(cb, Elem(elems, elemtype, cat(i / width), i % width),
                      SlotVar(base + i, args[i]));
        }
        Write(cb, SlotVar(base, RTT_VECTOR), "_v");
        cb += "    }\n";
    }

    // The fields of a struct are of the types they are, so the one helper that takes all of
    // them at once gets them as the Values the slots of everything else in memory hold.
    void EmitStructToString(int type_idx, const Types &args, TypeRef type) {
        TrackUseDef((int)args.size(), 1);
        StageValues("_ss", args);
        Write(cb, SlotVar(regso - (int)args.size(), RTT_STRING),
              cat("RtStructToString(vm, _ss, (type_elem_t)", type_idx, ")"), "");
        TypeComment(type);
        cb += "    }\n";
    }

    // A constant is an expression like any other, so it goes in parentheses when it starts
    // with a sign, which would otherwise pair up with an operator in front of it.
    static string Parenthesized(string lit) {
        return lit[0] == '-' ? cat("(", lit, ")") : lit;
    }

    void EmitPushInt(int64_t val) {
        TrackUseDef(0, 1);
        // The most negative value has no literal of its own, since its negation does not fit.
        auto lit = val == INT64_MIN ? string("(-9223372036854775807LL - 1)")
                 : val == (int)val  ? to_string(val)
                                    : cat(val, "LL");
        Defer(Slot(0, VK_INT), Parenthesized(lit), "");
    }

    // A double as a C literal: its decimal form when that reads back to the same value, the
    // hex float with the decimal alongside otherwise.
    static string FloatLiteral(double f) {
        auto dec = to_string_float(f);
        if (dec.find_first_of(".eE") == string::npos) dec += ".0";
        if (strtod(dec.c_str(), nullptr) == f) return Parenthesized(dec);
        char hex[64];
        snprintf(hex, sizeof hex, "%a", f);
        return Parenthesized(cat(hex, " /* ", dec, " */"));
    }

    void GenFloat(double f) {
        TrackUseDef(0, 1);
        if (isfinite(f)) {
            Defer(Slot(0, VK_FLOAT), FloatLiteral(f), "");
            return;
        }
        // An infinity or a nan has no literal, so this goes thru its bits.
        string hex;
        to_string_hex(hex, (uint64_t)int2float64(f).i);
        if (cpp) {
            Defer(Slot(0, VK_FLOAT), cat("int2float64((int64_t)", hex, "ULL).f"), "");
        } else {
            append(cb, "    { Value _v; _v.ival = (long long)", hex, "ULL; ",
                   WriteText(Slot(0, VK_FLOAT), "_v.fval"), " }\n");
        }
    }

    // Only the decrement itself is worth emitting: what happens when it reaches zero is a good
    // deal more code, and stays a call.
    // A pending reference that is a constant can only be nil, which has no count.
    bool IsNilConstant(const Place &p) {
        return HasPending(p.slot) && pending[p.slot].vars.empty() && !pending[p.slot].prec;
    }

    void GenDecRef(string &sd, const Place &p) {
        if (IsNilConstant(p)) return;
        auto r = Read(p);
        // Only a variable is free to be named more than once, so the rest go thru a local.
        if (p.var) {
            if (cpp) append(sd, "    if (", r, ") ", r, "->Dec(vm);\n");
            else append(sd, "    if (", r, " && --", r, "->refc <= 0) DecDelete(vm, ", r, ");\n");
        } else if (cpp && !p.typed) {
            append(sd, "    ", p.s, ".LTDECRTNIL(vm);\n");
        } else if (cpp) {
            append(sd, "    { ", CType(p.k()), "_r = ", r, "; if (_r) _r->Dec(vm); }\n");
        } else {
            append(sd, "    { ", CType(p.k()), "_r = ", r, ";"
                       " if (_r && --_r->refc <= 0) DecDelete(vm, _r); }\n");
        }
    }

    void GenIncRef(const Place &p) {
        if (IsNilConstant(p)) return;
        auto r = Read(p);
        if (p.var) {
            if (cpp) append(cb, "    if (", r, ") ", r, "->Inc();\n");
            else append(cb, "    if (", r, ") ", r, "->refc++;\n");
        } else if (cpp && !p.typed) {
            append(cb, "    ", p.s, ".LTINCRTNIL();\n");
        } else if (cpp) {
            append(cb, "    { ", CType(p.k()), "_r = ", r, "; if (_r) _r->Inc(); }\n");
        } else {
            append(cb, "    { ", CType(p.k()), "_r = ", r, "; if (_r) _r->refc++; }\n");
        }
    }

    void EmitIncRef(int off, RTType rtt) {
        TrackUseDef(0, 0);
        // FIXME: even when the static type is IsRef (i.e. no NIL or scalar), at runtime it is
        // still possible we get passed an int false value due to the way and/or are compiled?
        // See e.g. astar_result in the test.
        // Would be great to remove this case since the if-check is not needed in almost all cases.
        GenIncRef(Slot(off + 1, rtt));
    }

    // The ones below are a move or a test on the stack and nothing else. Calling a helper for
    // one of those costs more than the work itself, and pushes its operand and result thru
    // memory where the compiler could otherwise keep them in a register.

    // What is pushed is a nil of the type, a single slot whatever the type, so this does not
    // look at the slots of a struct.
    void EmitPushNil(TypeRef type) {
        TrackUseDef(0, 1);
        SetNil(cb, SlotVar(regso, RtTypeOf(type)));
    }

    void EmitPopRef(RTType rtt) {
        TrackUseDef(1, 0);
        GenDecRef(cb, Slot(1, rtt));
    }

    // Turning a reference into a bool can drop it first: what is left only gets tested against
    // nil, which does not need the value alive.
    void EmitBoolTest(string_view test, bool decref, VKind k) {
        TrackUseDef(1, 1);
        auto v = Slot(1, k);
        if (decref) GenDecRef(cb, v);
        auto e = Operand(v, 7);
        e.text = cat(e.text, " ", test);
        e.prec = 7;
        WriteExpr(Slot(1, VK_INT), e);
    }

    void EmitIntToFloat() {
        TrackUseDef(1, 1);
        auto e = Operand(Slot(1, VK_INT), 2);
        e.text = "(double)" + e.text;
        e.prec = 2;
        WriteExpr(Slot(1, VK_FLOAT), e);
    }

    // All that is left of an assert in the common case is the test; the reporting is a call.
    void EmitAssert(int defslots, int line, int fileidx, int stringidx, VKind k) {
        TrackUseDef(1, defslots);
        append(cb, "    if (!", Operand(Slot(1, k), 2).text, ") ",
               cpp ? "vm.AssertFailed(" : "AssertFailed(vm, ",
               line, ", ", fileidx, ", ", stringidx, ");\n");
    }

    void DefineFunction(string &sd, bool label) {
        sd += "\n";
        auto sf_idx = f_function_idx;
        if (sf_idx < CODEGEN_SPECIAL_FUNCTION_ID_START)
            append(sd, "// ", Signature(*st.subfunctiontable[sf_idx]), "\n");
        assert(f_arg_places.size() == f_args.size());
        // The body is complete, so a slot whose write is still deferred is never read:
        // dropping those here also keeps the writes to locals below from flushing them
        // into the body, which is already past the declarations they would need.
        pending.clear();
        Types argtypes;
        for (auto &p : f_arg_places) argtypes.push_back(p.rtt);
        append(sd, FunSignature(FunName(sf_idx), argtypes, f_ret_types, &f_arg_places), " {\n");
        // NOTE: f_keeps, f_slot_kinds, f_stage_max and f_regs_max are not known until the
        // end of codegen of the function!
        vector<Place> slots, keeps, locals;
        for (auto [i, kinds] : enumerate(f_slot_kinds)) {
            for (int k = 0; k < VK_COUNT; k++) {
                if (kinds & (1 << k)) slots.push_back(SlotVar((int)i, VKind(k)));
            }
        }
        for (int i = 0; i < (int)f_keeps.size(); i++) keeps.push_back(KeepVar(i));
        // The arguments are the parameters, so only the locals after them are declared here.
        int nargs_local = 0;
        for (auto varidx : f_args) if (!sids[varidx].used_as_freevar()) nargs_local++;
        assert((int)local_names.size() == numlocals);
        for (int i = nargs_local; i < numlocals; i++) locals.push_back(Local(i));
        GenPlaceDecls(sd, slots);
        for (auto k : { VK_INT, VK_FLOAT }) {
            if (f_stage_max[k])
                append(sd, "    ", CType(k), " ", StageArray(k), "[", f_stage_max[k], "];\n");
        }
        if (f_uses_pctx) append(sd, "    ", cpp ? "" : "struct ", "___tracy_c_zone_context pctx;\n");
        GenPlaceDecls(sd, keeps);
        GenPlaceDecls(sd, locals);
        if (ShadowLocals() && numlocals) append(sd, "    Value locals[", numlocals, "];\n");
        if (f_uses_lval) append(sd, "    void *lv = 0;\n");
        if (f_uses_lobj) append(sd, "    LObject *lo = 0;\n");
        if (f_uses_lelem) append(sd, "    void *lvec = 0;\n    long long lidx = 0;\n");
        if (f_ret_types.size() == 1) {
            append(sd, "    ", CType(Kind(f_ret_types[0])), " ret;\n");
        } else if (f_ret_types.size() > 1) {
            append(sd, "    ", RetStruct(f_ret_types), " ret;\n");
        }
        for (int i = 0; i < (int)f_args.size(); i++) {
            auto varidx = f_args[i];
            auto &p = f_arg_places[i];
            if (sids[varidx].used_as_freevar()) {
                // The argument is the global for the duration of the call, whose old value the
                // parameter holds meanwhile, to go back at the end. That old value is only
                // ever put back, so it is read past the tag, which on the first call still
                // says nil, the global not having been initialized yet.
                auto t = Var("_t", p.rtt);
                append(sd, "    { ", CType(p.k()), " _t; ", CopyValueText(t, p), " ",
                       WriteText(p, ReadNil(Global(varidx))), " ",
                       CopyValueText(Global(varidx), t), " }\n");
            } else if (ShadowLocals()) {
                CopyValue(sd, Shadow(var_to_local[varidx]), p);
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
        for (int i = 0; i < (int)f_keeps.size(); i++) {
            SetNil(sd, KeepVar(i));
        }
        // A return that has nothing of its own to say leaves these alone, since the caller then
        // reads what a non-local return left in the buffer instead, see GenUnwind. Written one
        // at a time rather than with an initializer, which libtcc compiles into a memset call.
        if (f_ret_types.size() > 1) {
            for (auto [i, kr] : enumerate(f_ret_types)) SetNil(sd, RetSlot((int)i, kr));
        }

        sd += cb;
        cb.clear();

        if (label) sd += "    epilogue:;\n";
        if (has_profile) {
            append(sd, "    ", cpp ? "lobster::" : "", "EndProfile(ctx);\n");
        }
        // The locals that live in a global get their old value back, see BackupVar above.
        for (int i = (int)f_defs.size() - 1; i >= 0; i--) {
            auto varidx = f_defs[i];
            if (sids[varidx].used_as_freevar()) {
                append(sd, "    RestoreBackup(vm, ", varidx, ");\n");
            }
        }
        for (int i = 0; i < (int)f_keeps.size(); i++) {
            GenDecRef(sd, KeepVar(i));
        }
        for (int i = 0; i < (int)f_args.size(); i++) {
            auto varidx = f_args[i];
            if (sids[varidx].used_as_freevar()) CopyValue(sd, Global(varidx), f_arg_places[i]);
        }
        if (runtime_checks >= RUNTIME_STACK_TRACE && f_function_idx < CODEGEN_SPECIAL_FUNCTION_ID_START) {
            append(sd, "    PopFunId(vm);\n");
        }
        if (!f_ret_types.empty()) sd += "    return ret;\n";
        sd += "}\n";
        ownedvars.clear();
        f_keeps.clear();
        f_slot_kinds.clear();
        f_stage_max[VK_INT] = 0;
        f_stage_max[VK_FLOAT] = 0;
        f_uses_pctx = false;
        f_uses_lval = false;
        f_uses_lobj = false;
        f_uses_lelem = false;
        f_lval_kind = LVK_NONE;
        local_names.clear();
        local_types.clear();
        f_names_used.clear();
        numlocals = 0;
        nlabel = 0;
        has_profile = false;
    }

    // What an object of each type gives up when it is deleted: the references among its
    // fields, which the code knows exactly and which it therefore says outright rather than
    // leaving the runtime to walk the type info for every field, see LObject::DeleteSelf.
    // A type with none of them has no function and a null in the table.
    void EmitObjectDecs(string &sd) {
        vector<string> decs(st.udttable.size());
        for (auto udt : st.udttable) {
            if (udt->g.is_struct || udt->numslots <= 0) continue;
            string body;
            for (int i = 0; i < udt->numslots; i++) {
                auto rtt = RtTypeOf(FindSlot(*udt, i)->type);
                if (RTIsRefNil(rtt)) GenDecRef(body, Field("o", *udt, i, rtt));
            }
            if (body.empty()) continue;
            decs[udt->idx] = UDTName(*udt) + "_dec";
            append(sd, "\nstatic void ", decs[udt->idx], "(VMRef vm, LObject *o) {\n", body,
                   "}\n");
        }
        if (cpp) sd += "\nstatic";
        else if (mir) sd += "\n";
        else sd += "\nextern";
        sd += " const object_dec_t object_decs[] = {\n";
        for (auto &d : decs) {
            if (d.empty()) sd += "    0,\n";
            else append(sd, "    ", d, ",\n");
        }
        // Not a language the empty array is legal in, and a program may have no object at all.
        if (decs.empty()) sd += "    0,\n";
        sd += "};\n";
    }

    void Epilogue(string &sd, string_view custom_pre_init_name, uint64_t src_hash) {
        EmitObjectDecs(sd);
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
                append(sd, "(fun_base_t)", FunName(id));
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
        append(sd, "    ", FunName(CODEGEN_SPECIAL_FUNCTION_ID_ENTRY), "(vm);\n}\n\n");
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
            append(sd, "&vmmeta, vtables, object_decs, ", custom_pre_init_name, ", \"\");\n}\n");
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
    void GenUnwind(const Types &rets) {
        auto lab = Label();
        TrackUseDef(0, 0);
        Flush();
        append(cb, "    if (", vmref(), "ret_unwind_to < 0) goto block", lab, ";\n");
        // Here we are emitting code executed only if we're unwinding, so temp modify the
        // tstack to match that.
        auto tstackbackup = tstack_size;
        // Only a Lobster function can be returned from, the entry function is never the target.
        if (cursf) {
            append(cb, "    if (", vmref(), "ret_unwind_to == ", cursf->parent->idx, ") {\n");
            append(cb, "    ", vmref(), "ret_unwind_to = -1;\n");
            GenReturnValues([&](int i) { return RetBufSlot(i, f_ret_types[i]); });
            if (f_ret_types.size() == 1) {
                // Passing it on, with nothing of its own to return.
                cb += "    } else {\n";
                SetNil(cb, RetVar());
            }
            cb += "    }\n";
        } else if (f_ret_types.size() == 1) {
            SetNil(cb, RetVar());
        }
        EmitReturn(RET_ANY, rets, -1, 0);
        EmitLabelDef(lab);
        tstack_size = tstackbackup;
    }

    void GenCall(const Call &call, size_t retval) {
        auto &sf = *call.sf;
        auto &f = *sf.parent;
        int inw = 0;
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
        auto args = ArgTypes(sf);
        auto rets = ReturnTypes(sf);
        if (inw != (int)args.size()) {
            parser.ErrorAt(node_context.back(), "internal error: call to ", Q(f.name),
                           " passes ", inw, " slots where it takes ", args.size());
        }
        if (call.vtable_idx < 0) {
            EmitCall(sf, inw);
            if (sf.returned_thru_to_max >= 0) {
                GenUnwind(rets);
            }
        } else {
            EmitDynDispatch(call.vtable_idx, args, rets);
            // We get the dispatch from arg 0, since sf is an arbitrary overloads and
            // doesn't necessarily point to the dispatch root (which may not even have an sf).
            auto dispatch_type = call.children[0]->exptype;
            assert(IsUDT(dispatch_type->t));
            auto de = dispatch_type->udt->dispatch_table[call.vtable_idx].get();
            assert(de->dispatch_root && !de->returntype.Null() && de->subudts_size);
            if (de->returned_thru_to_max >= 0) {
                // This works because all overloads of a DD sit under a single Function.
                GenUnwind(rets);
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
        auto k = ScalarKind(isfloat);
        WriteExpr(Slot(2, BinKind(isfloat, op)), BinExpr(isfloat, op, Slot(2, k), Slot(1, k)));
    }

    // Comparing two structs is a compare per slot, of whatever kind it is.
    void GenStructCompare(bool eq, TypeRef type) {
        auto len = ValWidth(type);
        append(cb, "    { long long _c = ", eq ? "1" : "0", ";\n");
        for (int j = 0; j < len; j++) {
            append(cb, "    _c = _c ", eq ? "&&" : "||", " ",
                   Operand(Slot(len * 2 - j, type, j), 7).text, " ", eq ? "==" : "!=", " ",
                   Operand(Slot(len - j, type, j), 7, true).text, ";\n");
        }
        // Only written after all the reads, since the result lands in the first slot of the left
        // hand side.
        Write(cb, Slot(len * 2, VK_INT), "_c != 0");
        cb += "    }\n";
    }

    // Reading a field is a load at a constant offset from the object, whose fields sit right
    // behind its header. It lands in the slot the object was in.
    void GenPushField(const UDT &udt, int offset, TypeRef ftype) {
        TrackUseDef(1, 1);
        auto obj = Read(Slot(1, VK_OBJECT));
        CopyValue(cb, Slot(1, ftype), Field(obj, udt, offset, ftype));
    }

    // Same, once per slot the struct field occupies. The object is only needed to find them, so
    // it is read out of the stack slot the first one lands in before that gets overwritten.
    void GenPushFieldStruct(const UDT &udt, int offset, TypeRef ftype) {
        auto fwidth = ValWidth(ftype);
        TrackUseDef(1, fwidth);
        append(cb, "    {\n    LObject *_o = ", Read(Slot(1, VK_OBJECT)), ";\n");
        for (int i = 0; i < fwidth; i++) {
            CopyValue(cb, Slot(1 - i, ftype, i),
                      Field("_o", udt, offset + i, SlotType(ftype, i)));
        }
        cb += "    }\n";
    }

    // Where the stack top is is something we track statically, so popping a slot needs no code
    // at all, just the bookkeeping.
    void GenPopSlot() { TrackUseDef(1, 0); }

    // The C expression for how many times a loop over this value runs, which for a vector or a
    // string comes out of the object itself, see the mirrors of those in Prologue.
    string LenOf(ValueType itertype, const Place &v) {
        switch (itertype) {
            case V_INT:
                return Read(v);
            case V_VECTOR:
            case V_STRING:
                return cat(Read(v), "->len");
            default:
                assert(false);
                return {};
        }
    }

    // The loop condition is an increment and a compare, small enough to be worth not calling for
    // the same reasons as GenScalarBinOp.
    int GenForCond(ValueType itertype) {
        // Reads the counter and the object being iterated, and leaves both for the body.
        TrackUseDef(2, 2);
        auto lab = Label();
        auto len = LenOf(itertype, Slot(1, itertype == V_INT ? VK_INT
                                         : itertype == V_STRING ? VK_STRING : VK_VECTOR));
        auto i = Slot(2, VK_INT);
        Write(cb, i, cat(Read(i), " + 1"));
        append(cb, "    if (!(", Read(i), " < ", len, ")) goto block", lab, ";\n");
        return lab;
    }

    // Both of these copy the loop counter to the top of the stack.
    void GenForCounter(int useslots, int defslots) {
        TrackUseDef(useslots, defslots);
        CopyValue(cb, Slot(0, VK_INT), Slot(2, VK_INT));
    }

    // The element the loop is on, at the counter below the object being iterated. The loop
    // condition already established the counter is in range, so this needs no check.
    // The bitmask says which slots of the element hold a reference the loop now owns.
    void GenForElem(bool isstring, int defslots, int bitmask, TypeRef elemtype) {
        TrackUseDef(2, defslots);
        // Everything but the counter and the object being iterated is the element.
        auto width = defslots - 2;
        auto idx = Read(Slot(2, VK_INT));
        if (isstring) {
            auto str = Read(Slot(1, VK_STRING));
            auto data = cpp ? cat("((unsigned char *)", str, "->data())")
                            : cat("STRING_DATA(", str, ")");
            Write(cb, Slot(0, VK_INT), cat("(long long)", data, "[", idx, "]"));
            return;
        }
        auto elems = cat(Read(Slot(1, VK_VECTOR)), cpp ? "->ElemSlots()" : "->elems");
        // A struct element is the same load per slot it occupies.
        for (int i = 0; i < width; i++) {
            CopyValue(cb, Slot(-i, elemtype, i), Elem(elems, elemtype, cat(idx), i));
        }
        for (int i = 0; i < width; i++) {
            if ((1 << i) & bitmask) GenIncRef(Slot(-i, elemtype, i));
        }
    }

    // Reading an element out of a vector, or just the part of it asked for, with the index
    // arbitrary, unlike the loop above. Indexing with a struct steps thru nested vectors, see
    // GenVectorDescent.
    void GenPushIdxNested(int levels, TypeRef elemtype, int subwidth, int offset) {
        // The vector plus one index per level it steps thru, replaced by the element.
        TrackUseDef(levels + 1, subwidth);
        GenVectorDescent(levels, Slot(levels + 1, VK_VECTOR));
        for (int i = 0; i < subwidth; i++) {
            CopyValue(cb, Slot(levels + 1 - i, elemtype, offset + i),
                      Elem(Elems(), elemtype, "_i", offset + i));
        }
        cb += "    }\n";
    }

    // A numeric struct indexed with a value only known at runtime, which its fields have to be
    // in memory for. All of them have the one type the struct is of, so that is what the array
    // they go into is of, and the element comes out of it as itself rather than as a Value.
    void EmitIndexStruct(TypeRef stype) {
        auto width = ValWidth(stype);
        auto etype = stype->udt->sametype;
        assert(etype->Numeric());
        auto rtt = RtTypeOf(etype);
        auto uint = string(cpp ? "uint64_t" : "unsigned long long");
        TrackUseDef(width + 1, 1);
        auto base = regso - width - 1;
        append(cb, "    {\n    ", CType(Kind(rtt)), " _s[", width, "];\n");
        for (int i = 0; i < width; i++) {
            append(cb, "    _s[", i, "] = ", Read(SlotVar(base + i, rtt)), ";\n");
        }
        append(cb, "    long long _i = ", Read(SlotVar(base + width, RTT_INT)), ";\n");
        append(cb, "    if ((", uint, ")_i >= (", uint, ")", width, ") ",
               cpp ? cat("vm.IDXErrS(_i, ", width, ");\n")
                   : cat("IDXErrS(vm, _i, ", width, ");\n"));
        Write(cb, SlotVar(base, rtt), "_s[_i]");
        cb += "    }\n";
    }

    // The same for a single level, or for a string. The object is read out into a local first,
    // since the element lands in the slot it came from.
    void GenPushIdx(bool str, TypeRef elemtype, int subwidth, int offset) {
        // The object and the index it is subscripted with, replaced by the element.
        TrackUseDef(2, subwidth);
        // A string index may read the terminating 0-byte, one past its length.
        auto bound = str ? "_o->len + 1" : "_o->len";
        append(cb, "    {\n    ", str ? "LString" : "LVector", " *_o = ",
               Read(Slot(2, str ? VK_STRING : VK_VECTOR)), "; long long _i = ",
               Read(Slot(1, VK_INT)), ";\n");
        if (cpp) {
            append(cb, "    if ((uint64_t)_i >= (uint64_t)(", bound, ")) vm.IDXErr(_i, ", bound,
                   ", _o);\n");
        } else {
            append(cb, "    if ((unsigned long long)_i >= (unsigned long long)(", bound,
                   ")) IDXErr(vm, _i, ", bound, ", _o);\n");
        }
        if (str) {
            Write(cb, Slot(2, VK_INT), cpp ? "(long long)((unsigned char *)_o->data())[_i]"
                                           : "STRING_DATA(_o)[_i]");
        } else {
            for (int i = 0; i < subwidth; i++) {
                CopyValue(cb, Slot(2 - i, elemtype, offset + i),
                          Elem(Elems(), elemtype, "_i", offset + i));
            }
        }
        cb += "    }\n";
    }

    void GenPop(TypeLT typelt) {
        if (IsStruct(typelt.type->t)) {
            if (typelt.type->t == V_STRUCT_R) {
                // TODO: alternatively call a single helper with a list or bitmask?
                // See BitMaskForRefStruct.
                for (int j = typelt.type->udt->numslots - 1; j >= 0; j--) {
                    auto stype = FindSlot(*typelt.type->udt, j)->type;
                    if (IsRefNil(stype->t)) EmitPopRef(RtTypeOf(stype));
                    else GenPopSlot();
                }
            } else {
                // A struct of scalars is just slots to give up.
                TrackUseDef(typelt.type->udt->numslots, 0);
            }
        } else {
            if (ShouldDec(typelt)) EmitPopRef(RtTypeOf(typelt.type)); else GenPopSlot();
        }
    }

    void GenDup(TypeLT tlt) {
        TrackUseDef(1, 2);
        CopyValue(cb, Slot(0, tlt.type), Slot(1, tlt.type));
        temptypestack.push_back(tlt);
    }

    void Gen(const Node *n, size_t retval) {
        // Generate() below generate no retvals if retval==0, otherwise they generate however many
        // they can irrespective of retval, optionally record that in rettypes for the more complex
        // cases. Then at the end of this function the two get matched up.
        auto tempstartsize = temptypestack.size();
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

    int BitMaskForRefStruct(TypeRef type) {
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
            CopyValue(cb, Lval(0, type), Slot(1, type));
        } else if (op == LV_WRITEREF) {
            // Whatever was there loses a reference to make way for what is written over it.
            GenDecRef(cb, Lval(0, type));
            CopyValue(cb, Lval(0, type), Slot(1, type));
        } else if (op == LV_WRITEV || op == LV_WRITEREFV) {
            // Same copy, one per slot of the struct being written, preceded by a decrement for
            // each of those slots that holds a reference, which the bitmask says which are.
            if (op == LV_WRITEREFV) {
                auto bitmask = BitMaskForRefStruct(type);
                for (int i = 0; i < width; i++)
                    if ((1 << i) & bitmask) GenDecRef(cb, Lval(i, type));
            }
            for (int i = 0; i < width; i++)
                CopyValue(cb, Lval(i, type), Slot(width - i, type, i));
        } else if (op == LV_SADD) {
            auto rhs = Read(Slot(1, VK_STRING));
            if (f_lval_kind == LVK_LOCAL || f_lval_kind == LVK_FIELD ||
                f_lval_kind == LVK_ELEM) {
                // The old string is an operand, so it loses its reference only once the new
                // one exists.
                auto v = Lval(0, type);
                append(cb, "    {\n    LString *_s = RtSAdd(vm, ", Read(v), ", ", rhs, ");\n");
                GenDecRef(cb, v);
                Write(cb, v, "_s");
                cb += "    }\n";
            } else {
                // Appending to a string in memory can free the old one, so it stays a call.
                append(cb, "    RtLvSAdd(vm, ", LvalPtr(), ", ", rhs, ");\n");
            }
        } else if (op >= LV_IPP) {
            auto c = op == LV_IPP || op == LV_FPP ? " + 1" : " - 1";
            auto v = Lval(0, type);
            Write(cb, v, Read(v) + c);
        } else if (op >= LV_BINAND && op <= LV_ASR) {
            auto v = Lval(0, type);
            Write(cb, v, BitExpr(BitOp(op - LV_BINAND), v, Slot(1, VK_INT)).text);
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
            auto k = ScalarKind(isfloat);
            auto n = isvec ? width : 1;
            for (int i = 0; i < n; i++) {
                auto rhs = isvec && !withscalar ? Slot(width - i, k) : Slot(1, k);
                auto v = Lval(i, type);
                Write(cb, v, BinExpr(isfloat, mop, v, rhs).text);
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
                EmitLvalField(*stype->udt, sfield.slot + offset);
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
                    EmitLvalVectorIndex(levels, offset, etype);
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
            for (int i = 0; i < width; i++) CopyValue(cb, Slot(-i, type, i), Lval(i, type));
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
        auto d = SlotVar(regso - nstrs, RTT_STRING);
        if (nstrs == 2) {
            Write(cb, d, cat("RtSAdd(vm, ", Read(Slot(2, VK_STRING)), ", ", Read(Slot(1, VK_STRING)),
                             ")"));
        } else {
            append(cb, "    {\n    LString *_cs[", nstrs, "];\n");
            for (int i = 0; i < nstrs; i++) {
                append(cb, "    _cs[", i, "] = ",
                       Read(SlotVar(regso - nstrs + i, RTT_STRING)), ";\n");
            }
            Write(cb, d, cat("RtStrConcatN(vm, _cs, ", nstrs, ")"));
            cb += "    }\n";
        }
    }

    void GenMathOp(const BinOp *n, size_t retval, MathOp op) {
        Gen(n->left, retval);
        Gen(n->right, retval);
        if (retval) GenMathOp(n->left->exptype, n->right->exptype, n->exptype, op);
    }

    // The type specialized helpers below come one per MathOp, in that order, so the name of each
    // is the prefix for the types it works on followed by the name of the operator. The C++
    // backend spells out the namespace, since two nil constants give it no argument to find
    // the helper thru.
    string MathOpName(string_view prefix, MathOp op) {
        static const char *ops[] = { "Add", "Sub", "Mul", "Div", "Mod",
                                     "Lt", "Gt", "Le", "Ge", "Eq", "Ne" };
        return cat(cpp ? "lobster::" : "", "Rt", prefix, ops[op]);
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
            Write(cb, Slot(2, VK_INT), cat(MathOpName("S", op), "(", Read(Slot(2, VK_STRING)), ", ",
                                          Read(Slot(1, VK_STRING)), ")"));
        } else if ((rtype->t == V_FUNCTION && ltype->t == V_FUNCTION)) {
            assert(op == MOP_EQ || op == MOP_NE);
            TrackUseDef(2, 1);
            Write(cb, Slot(2, VK_INT), cat(Read(Slot(2, VK_FUN)), op == MOP_EQ ? " == " : " != ",
                                          Read(Slot(1, VK_FUN))));
        } else if ((rtype->t == V_TYPEID && ltype->t == V_TYPEID)) {
            assert(op == MOP_EQ || op == MOP_NE);
            GenScalarBinOp(false, op);
        } else if (op >= MOP_EQ) {
            if (IsStruct(ltype->t)) {
                // Comparing two structs is one compare per slot, so this never becomes a
                // call at all.
                TrackUseDef(ValWidth(ltype) * 2, 1);
                GenStructCompare(op == MOP_EQ, ltype);
            } else {
                assert(IsRefNil(ltype->t) && IsRefNil(rtype->t));
                TrackUseDef(2, 1);
                if ((ltype->t == V_NIL && ltype->sub->t == V_STRING) ||
                    (rtype->t == V_NIL && rtype->sub->t == V_STRING)) {
                    Write(cb, Slot(2, VK_INT), cat(MathOpName("Sn", op), "(", Read(Slot(2, VK_STRING)),
                                                  ", ", Read(Slot(1, VK_STRING)), ")"));
                } else {
                    // References compare by identity, as whatever kind the left one is.
                    auto l = Slot(2, ltype);
                    Write(cb, Slot(2, VK_INT), cat(Read(l), op == MOP_EQ ? " == " : " != ",
                                                  ReadAs(Slot(1, rtype), l.k())));
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
            auto k = ScalarKind(isfloat);
            auto rk = BinKind(isfloat, op);
            TrackUseDef(inw, outw);
            // The same operator once per slot of the struct, with the results landing where
            // the left operand was.
            if (!withscalar) {
                for (int j = 0; j < width; j++) {
                    WriteExpr(Slot(width * 2 - j, rk),
                              BinExpr(isfloat, op, Slot(width * 2 - j, k), Slot(width - j, k)));
                }
            } else if (leftisvec) {
                for (int j = 0; j < width; j++) {
                    WriteExpr(Slot(width + 1 - j, rk),
                              BinExpr(isfloat, op, Slot(width + 1 - j, k), Slot(1, k)));
                }
            } else {
                // The scalar sits below the struct, in the slot the first result lands in, so
                // it is read into a local first, which the results are computed from where they
                // are since that local does not outlive the block.
                append(cb, "    { ", CType(k), " _s = ", Read(Slot(width + 1, k)), ";\n");
                auto scalar = Var("_s", k);
                for (int j = 0; j < width; j++) {
                    auto e = BinExpr(isfloat, op, scalar, Slot(width - j, k));
                    Write(cb, Slot(width + 1 - j, rk), e.text);
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
            WriteExpr(Slot(2, VK_INT), BitExpr(op, Slot(2, VK_INT), Slot(1, VK_INT)));
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
                for (int i = 0; i < fwidth; i++) {
                    auto t = RtTypeOf(SlotType(stype, offset + i));
                    CopyValue(cb, SlotVar(base + i, t), SlotVar(base + offset + i, t));
                }
            }
        } else {
            if (IsStruct(ftype->t)) {
                GenPushFieldStruct(*stype->udt, offset, ftype);
            } else {
                GenPushField(*stype->udt, offset, ftype);
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
                // An int index is a single level, a struct one a level per component.
                auto levels = ValWidth(index->exptype);
                auto etype = object->exptype;
                for (int i = 0; i < levels; i++) etype = etype->Element();
                auto elemwidth = ValWidth(etype);
                // Either the whole element or just the part of it asked for.
                auto subwidth = struct_elem_sub_width < 0 ? elemwidth : struct_elem_sub_width;
                auto suboffset = struct_elem_sub_width < 0 ? 0 : struct_elem_sub_offset;
                if (levels == 1) GenPushIdx(false, etype, subwidth, suboffset);
                else GenPushIdxNested(levels, etype, subwidth, suboffset);
                break;
            }
            case V_STRUCT_S:
                assert(index->exptype->t == V_INT);
                EmitIndexStruct(object->exptype);
                break;
            case V_STRING:
                assert(index->exptype->t == V_INT);
                GenPushIdx(true, type_int, 1, 0);
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
    if (retval) { cg.EmitPushNil(exptype); }
}

void IntConstant::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    cg.EmitPushInt(integer);
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
    cg.EmitPushNil(exptype);
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
        cg.EmitLvalField(*this_sid->type->udt, sfield.slot);
        cg.GenLvalModifier(cg.AssignBaseOp({ sfield.type, LT_KEEP }), sfield.type);
        cg.EmitLabelDef(lab);
    }
    if (!retval) return;
    cg.EmitPushNil(exptype);
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
    cg.EmitPushNil(exptype);
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
    assert(ctype->t == V_INT || ctype->t == V_FLOAT || ctype->t == V_STRUCT_S);
    // The same negation per slot, of whatever kind each is.
    auto width = ValWidth(ctype);
    cg.TrackUseDef(width, width);
    for (int i = 0; i < width; i++) {
        auto v = cg.Slot(width - i, ctype, i);
        cg.WriteExpr(v, cg.Unary("-", v));
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
    auto v = cg.Slot(1, CodeGen::VK_INT);
    cg.WriteExpr(v, cg.Unary("~", v));
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
                                  CodeGen::TypesOf(child->exptype, 1), child->exptype);
            break;
        }
        default: {
            auto ti = (int)cg.GetTypeTableOffset(child->exptype->ElementIfNil());
            cg.TrackUseDef(1, 1);
            auto p = cg.Slot(1, child->exptype);
            cg.Write(cg.cb, cg.SlotVar(cg.regso - 1, RTT_STRING),
                     cat("Rt", CodeGen::KindName(p.k()), "ToString(vm, ", cg.ReadTyped(p),
                         ", (type_elem_t)", ti, ")"));
            break;
        }
    }
}

void ToBool::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, false);
    cg.EmitBoolTest("!= 0", cg.ShouldDec(TypeLT(*child, 0)), CodeGen::KindOf(child->exptype));
}

void ToInt::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    // No actual opcode needed, this node is purely to store correct types.
    if (retval) cg.TakeTemp(1, false);
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
                    // See BitMaskForRefStruct.
                    for (int j = 0; j < type->udt->numslots; j++) {
                        auto stype = FindSlot(*type->udt, j)->type;
                        if (IsRefNil(stype->t)) {
                            cg.EmitIncRef(stack_offset + type->udt->numslots - 1 - j,
                                          CodeGen::RtTypeOf(stype));
                        }
                    }
                } else {
                    cg.EmitIncRef(stack_offset, CodeGen::RtTypeOf(type));
                }
            }
            if (decref & (1LL << i)) {
                assert(IsRefNil(type->t));
                if (type->t == V_STRUCT_R) {
                    // TODO: alternatively call a single helper with a list or bitmask?
                    // See BitMaskForRefStruct.
                    for (int j = 0; j < type->udt->numslots; j++) {
                        auto stype = FindSlot(*type->udt, j)->type;
                        if (IsRefNil(stype->t)) {
                            cg.EmitKeep(stack_offset + (type->udt->numslots - j - 1),
                                        CodeGen::RtTypeOf(stype));
                        }
                    }
                } else {
                    cg.EmitKeep(stack_offset, CodeGen::RtTypeOf(type));
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
                          (int)cg.stringtable.size(), CodeGen::KindOf(child->exptype));
            // FIXME: would be better to use the original source code here.
            cg.stringtable.push_back(cg.st.StoreName(DumpNode(*child, 0, true)));
        }
    } else {
        cg.Gen(child, 0);
    }
}

void NativeCall::Generate(CodeGen &cg, size_t retval) const {
    // TODO: could pass arg types in here if most exps have types, cheaper than
    // doing it all in call instruction?
    CodeGen::Types args;
    CodeGen::NativeArgs nargtypes;
    for (auto [i, c] : enumerate(children)) {
        auto before = cg.tstack_size;
        cg.Gen(c, 1);
        if (Is<DefaultVal>(c)) {
            // A single nil of the type, whatever it is. A struct argument that was left out is
            // a value of zeroes instead, which the typechecker adds, so this is never one.
            assert(!nf->ArgIsVec(i));
            for (auto n = cg.tstack_size - before; n; n--) {
                args.push_back(CodeGen::RtTypeOf(c->exptype));
            }
        } else {
            CodeGen::AddTypes(args, c->exptype);
        }
        nargtypes.push_back(nf->ArgIsVec(i) ? ValWidth(c->exptype) : -1);
    }
    size_t nargs = children.size();
    cg.TakeTemp(nargs, true);
    assert(nargs == nf->args.size());
    // The ones the code writes out itself work on a vector, whose element type says what its
    // elements hold, see CodeGen::Elem.
    auto elemtype = !children.empty() && children[0]->exptype->t == V_VECTOR
        ? children[0]->exptype->Element()
        : type_undefined;
    cg.EmitNativeCall(nf, args, CodeGen::TypesOf(nattype, nattype->NumValues()), nargtypes,
                      elemtype);
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
    // The function type has no variables for its arguments, so their kinds come from what
    // is passed.
    CodeGen::Types args;
    for (auto c : children) {
        cg.Gen(c, 1);
        CodeGen::AddTypes(args, c->exptype);
    }
    size_t nargs = children.size();
    assert(nargs == sf->args.size());
    cg.GenPushVar(1, type_function_null_void, sid->Idx(), sid->used_as_freevar);
    cg.TakeTemp(nargs, true);
    cg.EmitCallValue(args, CodeGen::ReturnTypes(*sf));
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
            cg.Gen(c, retval);
            cg.TakeTemp(retval, true);
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
    auto lab = cg.EmitJumpFail(!!retval, CodeGen::KindOf(left->exptype),
                               CodeGen::KindOf(exptype));
    if (retval) cg.GenPopSlot();
    cg.Gen(right, retval);
    if (retval) cg.TakeTemp(1, false);
    cg.EmitLabelDef(lab);
}

void Or::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(left, 1);
    cg.TakeTemp(1, false);
    auto lab = cg.EmitJumpNoFail(!!retval, CodeGen::KindOf(left->exptype),
                                 CodeGen::KindOf(exptype));
    if (retval) cg.GenPopSlot();
    cg.Gen(right, retval);
    if (retval) cg.TakeTemp(1, false);
    cg.EmitLabelDef(lab);
}

void Not::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (retval) {
        cg.TakeTemp(1, false);
        cg.EmitBoolTest("== 0", false, CodeGen::KindOf(child->exptype));
    }
}

void IfThen::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    auto lab = cg.EmitJumpFail(0, CodeGen::KindOf(condition->exptype));
    assert(!retval); (void)retval;
    cg.Gen(truepart, 0);
    cg.EmitLabelDef(lab);
}

void IfElse::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    auto lab = cg.EmitJumpFail(0, CodeGen::KindOf(condition->exptype));
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
    auto jumpout = cg.EmitJumpFail(0, CodeGen::KindOf(condition->exptype));
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
            cg.GenForElem(true, 3, 0, type_int);
            break;
        case V_VECTOR: {
            auto sub = typelt.type->sub;
            // A single slot element is a reference the loop owns when its type says so, a
            // struct one has a whole bitmask of them.
            auto bitmask = !IsRefNil(sub->t)  ? 0
                         : IsStruct(sub->t)   ? cg.BitMaskForRefStruct(sub)
                                              : 1;
            cg.GenForElem(false, ValWidth(sub) + 2, bitmask, sub);
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
                loc = cg.EmitJumpFail(0, CodeGen::VK_INT);
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
                auto lab = cg.EmitJumpFail(0, CodeGen::VK_INT);
                nextcase.push_back(lab);
            } else {
                auto lab = cg.EmitJumpNoFail(0, CodeGen::VK_INT);
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
    // The cases are jump targets, so what is on the stack has to be in its slots.
    string on;
    if (vtable_idx >= 0) {
        on = cat("GetTypeSwitchID(vm, ", cg.Read(cg.Slot(1, CodeGen::VK_OBJECT)), ", ", vtable_idx,
                 ")");
    } else {
        on = cg.Read(cg.Slot(1, CodeGen::VK_INT));
    }
    cg.Flush();
    if (cg.cpp) {
        append(cg.cb, "    switch (", on, ") {\n");
    } else {
        append(cg.cb, "    { ", vtable_idx >= 0 ? "int" : "long long", " top = ", on,
               "; switch (top) {\n");
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
        cg.Flush();
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
    CodeGen::Types args;
    for (auto c : children) CodeGen::AddTypes(args, c->exptype);
    assert((int)args.size() == arg_width); (void)arg_width;
    cg.EmitNewVec((int)offset, args, (int)Arity(), exptype->Element());
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
        CodeGen::Types args;
        for (auto c : children) CodeGen::AddTypes(args, c->exptype);
        assert((int)args.size() == arg_width); (void)arg_width;
        cg.EmitNewObject(offset, args, exptype);
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
            cg.EmitIsSubType(te->udt->subtype_dfs, te->udt->subtype_dfs_end, nilres, te,
                             child->exptype);
        } else {
            cg.EmitIsType(cg.GetTypeTableOffset(te), nilres, te, child->exptype);
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
    // The code after the return, reached some other way, still has the pending slots.
    auto pendingbackup = cg.pending;
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
    CodeGen::Types rets;
    if (sf->reqret) {
        auto nretvals = make_void ? 0 : sf->returntype->NumValues();
        if (!Is<DefaultVal>(child)) {
            cg.Gen(child, nretvals);
            cg.TakeTemp(nretvals, true);
        } else {
            cg.EmitPushNil(sf->returntype);
            assert(nretvals == 1);
        }
        rets = CodeGen::TypesOf(sf->returntype, nretvals);
    } else {
        if (!Is<DefaultVal>(child)) cg.Gen(child, 0);
    }
    // FIXME: we could change the VM to instead work with SubFunction ids.
    // Note: this can only work as long as the type checker forces specialization
    // of the functions in between here and the function returned to.
    // Actually, doesn't work with DDCALL and RETURN_THRU.
    // FIXME: shouldn't need any type here if V_VOID, but nretvals is at least 1 ?
    auto nretslots = (int)rets.size();
    if (sf == cg.cursf) {
        cg.EmitReturn(RET_LOCAL, rets, -1, nretslots);
    } else {
        cg.EmitReturn(RET_NONLOCAL, rets, sf->parent->idx, nretslots);
    }

    reset_from_small_vector(cg.temptypestack, typestackbackup);
    cg.tstack_size = tstackbackup;
    cg.pending = pendingbackup;
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
