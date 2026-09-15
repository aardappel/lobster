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

#include "lobster/codegen_base.h"
#include "lobster/codegen_builtin.h"
#include "lobster/codegen_type_table.h"
#include "lobster/codegen_ops.h"
#include "lobster/codegen_refcount.h"
#include "lobster/codegen_prologue.h"
#include "lobster/codegen_functions.h"
#include "lobster/codegen_lval.h"
#include "lobster/codegen_flow.h"
#include "lobster/codegen_constructors.h"

namespace lobster {

// Each shared virtual operation has one implementation in its owning topic.
// MSVC warns about that intentional dominance through the virtual base.
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4250)
#endif

// Assemble the independent topics. Gen and what it keeps track of live in the base; each topic
// keeps the generation of its nodes beside the helpers they use. The constructor generates the
// whole program.
struct CodeGen final : CodeGenBuiltin, CodeGenTypeTable, CodeGenOps, CodeGenRefCount,
                       CodeGenPrologue, CodeGenFunctions, CodeGenLval, CodeGenFlow,
                       CodeGenConstructors {
    using CodeGenBuiltin::Generate;
    using CodeGenTypeTable::Generate;
    using CodeGenOps::Generate;
    using CodeGenRefCount::Generate;
    using CodeGenPrologue::Generate;
    using CodeGenFunctions::Generate;
    using CodeGenLval::Generate;
    using CodeGenFlow::Generate;
    using CodeGenConstructors::Generate;

    CodeGen(Parser &_p, SymbolTable &_st, const CompileOptions &opts, uint64_t src_hash,
            string &c_codegen)
        : CodeGenBase(*this, _p, _st, opts) {
        node_context.push_back(parser.root);
        udt_type_offsets.resize(st.udttable.size(), (type_elem_t)-1);
        udt_nil_type_offsets.resize(st.udttable.size(), (type_elem_t)-1);
        AssignSubtypeIDs();

        // Reserve space and index for all vtables. The members of an abstract struct family
        // get theirs at one stride from the root's, by family index, such that a dispatch
        // on one is a single load from what its type slot holds, see EmitDynDispatch.
        for (auto udt : st.udttable) {
            if (udt->family_root) continue;
            udt->vtable_start = (int)vtables.size();
            vtables.insert(vtables.end(), udt->dispatch_table.size(), -1);
        }
        for (auto root : st.udttable) {
            if (root->family_root != root) continue;
            size_t stride = 0;
            for (auto udt : st.udttable) {
                if (udt->family_root == root)
                    stride = std::max(stride, udt->dispatch_table.size());
            }
            root->family_vtable_stride = (int)stride;
            auto base = vtables.size();
            vtables.insert(vtables.end(),
                           stride * (root->subtype_dfs_end - root->subtype_dfs + 1), -1);
            for (auto udt : st.udttable) {
                if (udt->family_root == root)
                    udt->vtable_start = (int)(base + udt->FamilyIndex() * stride);
            }
        }

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
        // The entry the string constants are of, see TYPE_ELEM_STRING_CONST: string's again,
        // added behind the lookup's back so that nothing that asks for string ever gets it.
        assert((int)type_table.size() == TYPE_ELEM_STRING_CONST);
        type_table.push_back((type_elem_t)RTT_STRING);
        type_table.push_back((type_elem_t)0);

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
                    sids.push_back(SpecIdentInfo{ sid->id->idx, tti, sid->used_as_freevar, sid->idx });
                    var_types.push_back(SlotType(sid->type, i));
                }
            }
        }

        // Keep the empty table's existing sentinel slot. Grow from the ids of the UDTs
        // being serialized, without carrying a separate maximum out of the parser.
        ser_ids.resize(1, (type_elem_t)-1);
        for (auto udt : st.udttable) {
            udt->ComputeSizes(st);
            if (!udt->g.is_abstract) {
                // We generate a type table for every UDT regardless of whether it is referred to
                // anywhere, for example (sub)classes may be constructed by deserializing them and
                // not in code.
                auto typeoff = GetTypeTableOffset(&udt->thistype);
                if (udt->serializable_id >= 0) {
                    if ((size_t)udt->serializable_id >= ser_ids.size())
                        ser_ids.resize((size_t)udt->serializable_id + 1, (type_elem_t)-1);
                    // The declchecker checked these are unique.
                    assert(ser_ids[udt->serializable_id] < 0);
                    ser_ids[udt->serializable_id] = typeoff;
                }
            }
        }

        Prologue(c_codegen);
        auto natives_decl_offset = c_codegen.size();

        // Start of the actual generated code.
        // Generate a dummmy function for function values that are never called.
        // Would be good if the optimizer guarantees these don't exist, but for now this is
        // more debuggable if it does happen to get called.
        f_function_idx = CODEGEN_SPECIAL_FUNCTION_ID_DUMMY;
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
        f_args.clear();
        f_defs.clear();
        f_arg_places.clear();
        f_ret_types.clear();
        f_keeps.clear();
        auto return_value = opts.return_value;
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
        DefineFunction(c_codegen, false);

        // Now fill in the vtables.
        for (auto udt : st.udttable) {
            for (auto [i, de] : enumerate(udt->dispatch_table)) {
                if (de->sf) {
                    vtables[udt->vtable_start + i] = de->sf->idx;
                    assert(de->case_index < 0);
                } else if (de->case_index >= 0) {
                    vtables[udt->vtable_start + i] = -de->case_index - 2;
                }
            }
        }

        EmitObjectDecs(c_codegen);
        Epilogue(c_codegen, opts.custom_pre_init_name, src_hash);

        string decls;
        DeclareNatives(decls);
        DeclareRetStructs(decls);
        DeclareUDTStructs(decls);
        EmitConstantStrings(decls);
        if (!decls.empty()) c_codegen.insert(natives_decl_offset, decls + "\n");
    }
};

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// The AST virtual methods forward to the overload of the topic that generates the node.
#define CODEGEN_NODE(T) \
    void T::Generate(CodeGen &cg, size_t retval) const { cg.Generate(*this, retval); }

CODEGEN_NODE(Block)
CODEGEN_NODE(Or)
CODEGEN_NODE(And)
CODEGEN_NODE(IfThen)
CODEGEN_NODE(IfElse)
CODEGEN_NODE(While)
CODEGEN_NODE(For)
CODEGEN_NODE(ForLoopElem)
CODEGEN_NODE(ForLoopCounter)
CODEGEN_NODE(Break)
CODEGEN_NODE(Continue)
CODEGEN_NODE(Switch)
CODEGEN_NODE(Case)
CODEGEN_NODE(Define)
CODEGEN_NODE(Member)
CODEGEN_NODE(Static)
CODEGEN_NODE(AssignList)
CODEGEN_NODE(IntConstant)
CODEGEN_NODE(FloatConstant)
CODEGEN_NODE(StringConstant)
CODEGEN_NODE(Nil)
CODEGEN_NODE(Plus)
CODEGEN_NODE(Minus)
CODEGEN_NODE(Multiply)
CODEGEN_NODE(Divide)
CODEGEN_NODE(Mod)
CODEGEN_NODE(PlusEq)
CODEGEN_NODE(MultiplyEq)
CODEGEN_NODE(MinusEq)
CODEGEN_NODE(DivideEq)
CODEGEN_NODE(ModEq)
CODEGEN_NODE(AndEq)
CODEGEN_NODE(OrEq)
CODEGEN_NODE(XorEq)
CODEGEN_NODE(ShiftLeftEq)
CODEGEN_NODE(ShiftRightEq)
CODEGEN_NODE(NotEqual)
CODEGEN_NODE(Equal)
CODEGEN_NODE(GreaterThanEq)
CODEGEN_NODE(LessThanEq)
CODEGEN_NODE(GreaterThan)
CODEGEN_NODE(LessThan)
CODEGEN_NODE(Not)
CODEGEN_NODE(BitAnd)
CODEGEN_NODE(BitOr)
CODEGEN_NODE(Xor)
CODEGEN_NODE(ShiftLeft)
CODEGEN_NODE(ShiftRight)
CODEGEN_NODE(Negate)
CODEGEN_NODE(PostDecr)
CODEGEN_NODE(PostIncr)
CODEGEN_NODE(PreDecr)
CODEGEN_NODE(PreIncr)
CODEGEN_NODE(UnaryMinus)
CODEGEN_NODE(IdentRef)
CODEGEN_NODE(Assign)
CODEGEN_NODE(Assert)
CODEGEN_NODE(NativeCall)
CODEGEN_NODE(Call)
CODEGEN_NODE(FunRef)
CODEGEN_NODE(DynCall)
CODEGEN_NODE(Return)
CODEGEN_NODE(IsType)
CODEGEN_NODE(VectorConstructor)
CODEGEN_NODE(ObjectConstructor)
CODEGEN_NODE(Dot)
CODEGEN_NODE(Indexing)
CODEGEN_NODE(Seq)
CODEGEN_NODE(TypeOf)
CODEGEN_NODE(EnumCoercion)
CODEGEN_NODE(MultipleReturn)
CODEGEN_NODE(DefaultVal)
CODEGEN_NODE(ToFloat)
CODEGEN_NODE(ToString)
CODEGEN_NODE(ToBool)
CODEGEN_NODE(ToInt)
CODEGEN_NODE(ToLifetime)
CODEGEN_NODE(InlineBlock)
CODEGEN_NODE(InlineReturn)

#undef CODEGEN_NODE

// The declarations produce no code, and the typechecker made them void.
void EnumRef::Generate(CodeGen &, size_t retval) const {
    assert(!retval);
    (void)retval;
}

void GUDTRef::Generate(CodeGen &, size_t retval) const {
    assert(!retval);
    (void)retval;
}

void UDTRef::Generate(CodeGen &, size_t retval) const {
    assert(!retval);
    (void)retval;
}

void ErrorValue::Generate(CodeGen &, size_t) const {
    assert(false);  // Compilation stops before codegen when there were errors.
}

void FreeVarRef::Generate(CodeGen &, size_t) const {
    assert(false);  // Should be replaced by typechecker.
}

void GenericCall::Generate(CodeGen &, size_t /*retval*/) const {
    assert(false);
}

void TypeAnnotation::Generate(CodeGen & /*cg*/, size_t /*retval*/) const {
    assert(false);  // Handled by individual parents.
}

void AutoConstructor::Generate(CodeGen &, size_t) const {
    assert(false);
}

void Range::Generate(CodeGen &/*cg*/, size_t /*retval*/) const {
    assert(false);
}

void List::Generate(CodeGen & /*cg*/, size_t /*retval*/) const {
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

}  // namespace lobster
