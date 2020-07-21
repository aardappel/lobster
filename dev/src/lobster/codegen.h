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

struct CodeGen  {
    vector<int> code;
    vector<uint8_t> code_attr;
    vector<bytecode::LineInfo> lineinfo;
    vector<bytecode::SpecIdent> sids;
    Parser &parser;
    vector<const Node *> linenumbernodes;
    vector<tuple<int, const SubFunction *>> call_fixups;
    SymbolTable &st;
    vector<type_elem_t> type_table, vint_typeoffsets, vfloat_typeoffsets;
    map<vector<type_elem_t>, type_elem_t> type_lookup;  // Wasteful, but simple.
    vector<TypeLT> rettypes, temptypestack;
    vector<const Node *> loops;
    vector<int> breaks;
    vector<string_view> stringtable;  // sized strings.
    vector<const Node *> node_context;
    int keepvars = 0;
    int runtime_checks;
    vector<int> vtables;

    int Pos() { return (int)code.size(); }

    void GrowCodeAttr(size_t mins) {
        while (mins > code_attr.size()) code_attr.push_back(bytecode::Attr_NONE);
    }

    void Emit(int i) {
        auto &ln = linenumbernodes.back()->line;
        if (lineinfo.empty() || ln.line != lineinfo.back().line() ||
            ln.fileidx != lineinfo.back().fileidx())
            lineinfo.push_back(bytecode::LineInfo(ln.line, ln.fileidx, Pos()));
        code.push_back(i);
        GrowCodeAttr(code.size());
    }

    void SplitAttr(int at) {
        GrowCodeAttr(at + 1);
        code_attr[at] |= bytecode::Attr_SPLIT;
    }

    void Emit(int i, int j) { Emit(i); Emit(j); }
    void Emit(int i, int j, int k) { Emit(i); Emit(j); Emit(k); }
    void Emit(int i, int j, int k, int l) { Emit(i); Emit(j); Emit(k); Emit(l); }

    void SetLabel(int jumploc) {
        code[jumploc - 1] = Pos();
        SplitAttr(Pos());
    }

    const int ti_num_udt_fields = 4;
    const int ti_num_udt_per_field = 2;

    void PushFields(UDT *udt, vector<type_elem_t> &tt, type_elem_t parent = (type_elem_t)-1) {
        for (auto &field : udt->fields) {
            auto ti = GetTypeTableOffset(field.resolvedtype);
            if (IsStruct(field.resolvedtype->t)) {
                PushFields(field.resolvedtype->udt, tt, ti);
            } else {
                tt.insert(tt.begin() + (ssize(tt) - ti_num_udt_fields) / ti_num_udt_per_field +
                          ti_num_udt_fields, ti);
                tt.push_back(parent);
            }
        }
    }

    // Make a table for use as VM runtime type.
    type_elem_t GetTypeTableOffset(TypeRef type) {
        vector<type_elem_t> tt;
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
                if (type->udt->typeinfo >= 0)
                    return type->udt->typeinfo;
                type->udt->typeinfo = (type_elem_t)type_table.size();
                // Reserve space, so other types can be added afterwards safely.
                assert(type->udt->numslots >= 0);
                auto ttsize = (type->udt->numslots * ti_num_udt_per_field) + ti_num_udt_fields;
                type_table.insert(type_table.end(), ttsize, (type_elem_t)0);
                tt.push_back((type_elem_t)type->udt->idx);
                tt.push_back((type_elem_t)type->udt->numslots);
                tt.push_back((type_elem_t)type->udt->vtable_start);
                PushFields(type->udt, tt);
                assert(ssize(tt) == ttsize);
                std::copy(tt.begin(), tt.end(), type_table.begin() + type->udt->typeinfo);
                return type->udt->typeinfo;
            }
            case V_VAR:
                // This can happen with an empty [] vector that was never bound to anything.
                // Should be benign to use any, since it is never accessed anywhere.
                // FIXME: would be even better to check this case before codegen, since this may
                // mask bugs.
                return GetTypeTableOffset(type_any);
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

    CodeGen(Parser &_p, SymbolTable &_st, bool return_value, int runtime_checks)
        : parser(_p), st(_st), runtime_checks(runtime_checks) {
        // Reserve space and index for all vtables.
        for (auto udt : st.udttable) {
            udt->vtable_start = (int)vtables.size();
            vtables.insert(vtables.end(), udt->dispatch.size(), -1);
        }
        // Pre-load some types into the table, must correspond to order of type_elem_t enums.
                                                            GetTypeTableOffset(type_int);
                                                            GetTypeTableOffset(type_float);
                                                            GetTypeTableOffset(type_string);
                                                            GetTypeTableOffset(type_resource);
                                                            GetTypeTableOffset(type_any);
        Type type_valuebuf(V_VALUEBUF);                     GetTypeTableOffset(&type_valuebuf);
        Type type_stackframebuf(V_STACKFRAMEBUF);           GetTypeTableOffset(&type_stackframebuf);
                                                            GetTypeTableOffset(type_vector_int);
                                                            GetTypeTableOffset(type_vector_float);
        Type type_vec_str(V_VECTOR, &*type_string);         GetTypeTableOffset(&type_vec_str);
        Type type_v_v_int(V_VECTOR, &*type_vector_int);     GetTypeTableOffset(&type_v_v_int);
        Type type_v_v_float(V_VECTOR, &*type_vector_float); GetTypeTableOffset(&type_v_v_float);
        assert(type_table.size() == TYPE_ELEM_FIXED_OFFSET_END);
        for (auto type : st.default_int_vector_types[0])
            vint_typeoffsets.push_back(!type.Null() ? GetTypeTableOffset(type) : (type_elem_t)-1);
        for (auto type : st.default_float_vector_types[0])
            vfloat_typeoffsets.push_back(!type.Null() ? GetTypeTableOffset(type) : (type_elem_t)-1);
        int sidx = 0;
        for (auto sid : st.specidents) {
            if (!sid->type.Null()) {  // Null ones are in unused functions.
                auto tti = GetTypeTableOffset(sid->type);
                assert(!IsStruct(sid->type->t) || sid->type->udt->numslots >= 0);
                sid->sidx = sidx;
                auto ns = ValWidth(sid->type);
                sidx += ns;
                for (int i = 0; i < ns; i++)
                    sids.push_back(bytecode::SpecIdent(sid->id->idx, tti));
            }
        }
        linenumbernodes.push_back(parser.root);
        SplitAttr(0);
        Emit(IL_JUMP, 0);
        auto fundefjump = Pos();
        SplitAttr(Pos());
        for (auto f : parser.st.functiontable) {
            if (f->bytecodestart <= 0 && !f->istype) {
                f->bytecodestart = Pos();
                for (auto sf : f->overloads) for (; sf; sf = sf->next) {
                    if (sf && sf->typechecked) GenScope(*sf);
                }
            }
        }
        // Generate a dummmy function for function values that are never called.
        // Would be good if the optimizer guarantees these don't exist, but for now this is
        // more debuggable if it does happen to get called.
        auto dummyfun = Pos();
        SplitAttr(dummyfun);
        Emit(IL_FUNSTART, -1, 0, 0);
        Emit(0, 0);  // keepvars, ownedvars
        Emit(IL_ABORT);
        // Emit the root function.
        SetLabel(fundefjump);
        SplitAttr(Pos());
        Gen(parser.root, return_value);
        auto type = parser.root->exptype;
        assert(type->NumValues() == (size_t)return_value);
        Emit(IL_EXIT, return_value ? GetTypeTableOffset(type): -1);
        SplitAttr(Pos());  // Allow off by one indexing.
        linenumbernodes.pop_back();
        for (auto &[loc, sf] : call_fixups) {
            auto bytecodestart = sf->subbytecodestart;
            if (!bytecodestart) bytecodestart = dummyfun;
            assert(!code[loc]);
            code[loc] = bytecodestart;
        }
        // Now fill in the vtables.
        for (auto udt : st.udttable) {
            for (auto [i, de] : enumerate(udt->dispatch)) {
                if (de.sf) {
                    vtables[udt->vtable_start + i] = de.sf->subbytecodestart;
                }
            }
        }
    }

    ~CodeGen() {
    }

    // FIXME: remove.
    void Dummy(size_t retval) {
        assert(!retval);
        while (retval--) Emit(IL_PUSHNIL);
    }

    void GenScope(SubFunction &sf) {
        if (sf.subbytecodestart > 0) return;
        keepvars = 0;
        sf.subbytecodestart = Pos();
        if (!sf.typechecked) {
            auto s = DumpNode(*sf.body, 0, false);
            LOG_DEBUG("untypechecked: ", sf.parent->name, " : ", s);
            assert(0);
        }
        vector<int> ownedvars;
        linenumbernodes.push_back(sf.body);
        SplitAttr(Pos());
        Emit(IL_FUNSTART);
        Emit(sf.parent->idx);
        auto ret = AssertIs<Return>(sf.body->children.back());
        auto ir = sf.consumes_vars_on_return ? AssertIs<IdentRef>(ret->child) : nullptr;
        auto emitvars = [&](const vector<Arg> &v) {
            auto nvarspos = Pos();
            Emit(0);
            auto nvars = 0;
            for (auto &arg : v) {
                auto n = ValWidth(arg.sid->type);
                for (int i = 0; i < n; i++) {
                    Emit(arg.sid->Idx() + i);
                    nvars++;
                    if (ShouldDec(IsStruct(arg.sid->type->t)
                                  ? TypeLT { FindSlot(*arg.sid->type->udt, i)->resolvedtype,
                                             arg.sid->lt }
                                  : TypeLT { *arg.sid }) && (!ir || arg.sid != ir->sid)) {
                        ownedvars.push_back(arg.sid->Idx() + i);
                    }
                }
            }
            code[nvarspos] = nvars;
        };
        emitvars(sf.args);
        emitvars(sf.locals);
        auto keepvarspos = Pos();
        Emit(0);
        // FIXME: don't really want to emit these.. instead should ensure someone takes
        // ownership of them.
        Emit((int)ownedvars.size());
        for (auto si : ownedvars) Emit(si);
        if (sf.body) for (auto c : sf.body->children) {
            Gen(c, 0);
            if (runtime_checks >= RUNTIME_ASSERT_PLUS)
                Emit(IL_ENDSTATEMENT, c->line.line, c->line.fileidx);
        }
        else Dummy(sf.reqret);
        assert(temptypestack.empty());
        assert(breaks.empty());
        code[keepvarspos] = keepvars;
        linenumbernodes.pop_back();
    }

    // This must be called explicitly when any values are consumed.
    void TakeTemp(size_t n, bool can_handle_structs) {
        assert(node_context.size());
        for (; n; n--) {
            auto tlt = temptypestack.back();
            temptypestack.pop_back();
            assert(can_handle_structs || ValWidth(tlt.type) == 1); (void)tlt;
            (void)can_handle_structs;
        }
    }

    int StackDepth() {
        int stack_depth = 0;
        for (auto &tlt : temptypestack) stack_depth += ValWidth(tlt.type);
        return stack_depth;
    }

    void GenFixup(const SubFunction *sf) {
        assert(sf->body);
        auto pos = Pos() - 1;
        if (!code[pos]) call_fixups.push_back({ pos, sf });
    }

    void GenUnwind(const SubFunction &sf) {
        SplitAttr(Pos());
        Emit(IL_JUMPIFUNWOUND, sf.parent->idx, 0);
        auto loc = Pos();
        if (!temptypestack.empty()) {
            Emit(IL_SAVERETS);
            for (auto &tse : reverse(temptypestack)) {
                GenPop(tse);
            }
            Emit(IL_RESTORERETS);
        }
        Emit(IL_RETURNANY);
        SetLabel(loc);
    }

    void GenCall(const SubFunction &sf, int vtable_idx, const List *args, size_t retval) {
        auto &f = *sf.parent;
        for (auto c : args->children) {
            Gen(c, 1);
        }
        size_t nargs = args->children.size();
        if (f.nargs() != nargs)
            parser.Error(cat("call to function ", f.name, " needs ", f.nargs(),
                             " arguments, ", nargs, " given"), node_context.back());
        TakeTemp(nargs, true);
        if (vtable_idx < 0) {
            Emit(IL_CALL, sf.subbytecodestart);
            GenFixup(&sf);
            if (sf.returned_thru) {
                GenUnwind(sf);
            }
        } else {
            int stack_depth = -1;
            for (auto c : args->children) stack_depth += ValWidth(c->exptype);
            Emit(IL_DDCALL, vtable_idx, stack_depth);
            // We get the dispatch from arg 0, since sf is an arbitrary overloads and
            // doesn't necessarily point to the dispatch root (which may not even have an sf).
            auto dispatch_type = args->children[0]->exptype;
            assert(IsUDT(dispatch_type->t));
            auto &de = dispatch_type->udt->dispatch[vtable_idx];
            assert(de.is_dispatch_root && !de.returntype.Null() && de.subudts_size);
            if (de.returned_thru) {
                // This works because all overloads of a DD sit under a single Function.
                GenUnwind(sf);
            }
        }
        SplitAttr(Pos());
        auto nretvals = sf.returntype->NumValues();
        for (size_t i = 0; i < nretvals; i++) {
            if (retval) {
                rettypes.push_back({ sf, i });
            } else {
                // FIXME: better if this is impossible by making sure typechecker makes it !reqret.
                GenPop({ sf, i });
            }
        }
    };

    void GenFloat(double f) {
        if ((float)f == f) {
            Emit(IL_PUSHFLT);
            int2float i2f;
            i2f.f = (float)f;
            Emit(i2f.i);
        } else {
            Emit(IL_PUSHFLT64);
            int2float64 i2f;
            i2f.f = f;
            Emit((int)i2f.i);
            Emit((int)(i2f.i >> 32));
        }
    }

    bool ShouldDec(TypeLT typelt) {
        return IsRefNil(typelt.type->t) && typelt.lt == LT_KEEP;
    }

    void GenPop(TypeLT typelt) {
        if (IsStruct(typelt.type->t)) {
            Emit(typelt.type->t == V_STRUCT_R ? IL_POPVREF : IL_POPV, typelt.type->udt->numslots);
        } else {
            Emit(ShouldDec(typelt) ? IL_POPREF : IL_POP);
        }
    }

    void GenDup(TypeLT tlt) {
        Emit(IL_DUP);
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

    void GenStructIns(TypeRef type) {
        if (IsStruct(type->t)) Emit(ValWidth(type));
    }

    void GenValueSize(TypeRef type) {
        // FIXME: struct variable size.
        Emit(IL_PUSHINT, ValWidth(type));
    }

    void GenAssign(const Node *lval, int lvalop, size_t retval,
                   const Node *rhs, int take_temp) {
        assert(node_context.back()->exptype->NumValues() == retval);
        auto type = lval->exptype;
        if (lvalop >= LVO_IADD && lvalop <= LVO_IMOD) {
            if (type->t == V_INT) {
            } else if (type->t == V_FLOAT)  {
                assert(lvalop != LVO_IMOD); lvalop += LVO_FADD - LVO_IADD;
            } else if (type->t == V_STRING) {
                assert(lvalop == LVO_IADD); lvalop = LVO_SADD;
            } else if (type->t == V_STRUCT_S) {
                auto sub = type->udt->sametype;
                bool withscalar = IsScalar(rhs->exptype->t);
                if (sub->t == V_INT) {
                    lvalop += (withscalar ? LVO_IVSADD : LVO_IVVADD) - LVO_IADD;
                } else if (sub->t == V_FLOAT) {
                    assert(lvalop != LVO_IMOD);
                    lvalop += (withscalar ? LVO_FVSADD : LVO_FVVADD) - LVO_IADD;
                } else assert(false);
            } else {
                assert(false);
            }
        } else if (lvalop >= LVO_IPP && lvalop <= LVO_IMMP) {
            if (type->t == V_FLOAT) lvalop += LVO_FPP - LVO_IPP;
            else assert(type->t == V_INT);
        }
        if (retval) lvalop++;
        if (rhs) Gen(rhs, 1);
        if (auto idr = Is<IdentRef>(lval)) {
            TakeTemp(take_temp, true);
            Emit(GENLVALOP(VAR, lvalop), idr->sid->Idx()); GenStructIns(idr->sid->type);
        } else if (auto dot = Is<Dot>(lval)) {
            auto stype = dot->children[0]->exptype;
            assert(IsUDT(stype->t));  // Ensured by typechecker.
            auto idx = stype->udt->Has(dot->fld);
            assert(idx >= 0);
            auto &field = stype->udt->fields[idx];
            Gen(dot->children[0], 1);
            TakeTemp(take_temp + 1, true);
            Emit(GENLVALOP(FLD, lvalop), field.slot); GenStructIns(field.resolvedtype);
        } else if (auto indexing = Is<Indexing>(lval)) {
            Gen(indexing->object, 1);
            Gen(indexing->index, 1);
            TakeTemp(take_temp + 2, true);
            switch (indexing->object->exptype->t) {
                case V_VECTOR:
                    Emit(indexing->index->exptype->t == V_INT
                         ? GENLVALOP(IDXVI, lvalop)
                         : GENLVALOP(IDXVV, lvalop));
                    GenStructIns(indexing->index->exptype);  // When index is struct.
                    GenStructIns(type);  // When vector elem is struct.
                    break;
                case V_CLASS:
                    assert(indexing->index->exptype->t == V_INT &&
                           indexing->object->exptype->udt->sametype->Numeric());
                    Emit(GENLVALOP(IDXNI, lvalop));
                    break;
                case V_STRUCT_R:
                case V_STRUCT_S:
                case V_STRING:
                    // FIXME: Would be better to catch this in typechecking, but typechecker does
                    // not currently distinquish lvalues.
                    parser.Error("cannot use this type as lvalue", lval);
                default:
                    assert(false);
            }
        } else {
            parser.Error("lvalue required", lval);
        }
    }

    void GenMathOp(const BinOp *n, size_t retval, int opc) {
        Gen(n->left, retval);
        Gen(n->right, retval);
        if (retval) GenMathOp(n->left->exptype, n->right->exptype, n->exptype, opc);
    }

    void GenMathOp(TypeRef ltype, TypeRef rtype, TypeRef ptype, int opc) {
        TakeTemp(2, true);
        // Have to check right and left because comparison ops generate ints for node
        // overall.
        if (rtype->t == V_INT && ltype->t == V_INT) {
            Emit(IL_IADD + opc);
        } else if (rtype->t == V_FLOAT && ltype->t == V_FLOAT) {
            Emit(IL_FADD + opc);
        } else if (rtype->t == V_STRING && ltype->t == V_STRING) {
            Emit(IL_SADD + opc);
        } else if (rtype->t == V_FUNCTION && ltype->t == V_FUNCTION) {
            assert(opc == MOP_EQ || opc == MOP_NE);
            Emit(IL_LEQ + (opc - MOP_EQ));
        } else {
            if (opc >= MOP_EQ) {  // EQ/NEQ
                if (IsStruct(ltype->t)) {
                    Emit(IL_STEQ + opc - MOP_EQ);
                    GenStructIns(ltype);
                } else {
                    assert(IsRefNil(ltype->t) &&
                           IsRefNil(rtype->t));
                    Emit(IL_AEQ + opc - MOP_EQ);
                }
            } else {
                // If this is a comparison op, be sure to use the child type.
                TypeRef vectype = opc >= MOP_LT ? ltype : ptype;
                assert(vectype->t == V_STRUCT_S);
                auto sub = vectype->udt->sametype;
                bool withscalar = IsScalar(rtype->t);
                if (sub->t == V_INT)
                    Emit((withscalar ? IL_IVSADD : IL_IVVADD) + opc);
                else if (sub->t == V_FLOAT)
                    Emit((withscalar ? IL_FVSADD : IL_FVVADD) + opc);
                else assert(false);
                GenStructIns(vectype);
            }
        }
    }

    void GenBitOp(const BinOp *n, size_t retval, ILOP opc) {
        Gen(n->left, retval);
        Gen(n->right, retval);
        if (retval) {
            TakeTemp(2, false);
            Emit(opc);
        }
    }

    int AssignBaseOp(TypeLT typelt) {
        auto dec = ShouldDec(typelt);
        return IsStruct(typelt.type->t)
            ? (dec ? LVO_WRITEREFV : LVO_WRITEV)
            : (dec ? LVO_WRITEREF : LVO_WRITE);
    }

    void EmitKeep(int stack_offset, int keep_index_add) {
        int opc = !loops.empty() ? IL_KEEPREFLOOP : IL_KEEPREF;
        Emit(opc, stack_offset, keepvars++ + StackDepth() + keep_index_add);
    }

    void GenPushVar(size_t retval, TypeRef type, int offset) {
        if (!retval) return;
        if (IsStruct(type->t)) {
            Emit(IL_PUSHVARV, offset);
            GenStructIns(type);
        } else {
            Emit(IL_PUSHVAR, offset);
        }
    }

    void GenPushField(size_t retval, Node *object, TypeRef stype, TypeRef ftype, int offset) {
        if (IsStruct(stype->t)) {
            // Attempt to not generate object at all, by reading the field inline.
            if (auto idr = Is<IdentRef>(object)) {
                GenPushVar(retval, ftype, idr->sid->Idx() + offset);
                return;
            } else if (auto dot = Is<Dot>(object)) {
                auto sstype = dot->children[0]->exptype;
                assert(IsUDT(sstype->t));
                auto idx = sstype->udt->Has(dot->fld);
                assert(idx >= 0);
                auto &field = sstype->udt->fields[idx];
                assert(field.slot >= 0);
                GenPushField(retval, dot->children[0], sstype, ftype, field.slot + offset);
                return;
            } else if (auto indexing = Is<Indexing>(object)) {
                // For now only do this for vectors.
                if (indexing->object->exptype->t == V_VECTOR) {
                    GenPushIndex(retval, indexing->object, indexing->index, ValWidth(ftype), offset);
                    return;
                }
            }
        }
        Gen(object, retval);
        if (!retval) return;
        TakeTemp(1, true);
        if (IsStruct(stype->t)) {
            if (IsStruct(ftype->t)) {
                Emit(IL_PUSHFLDV2V, offset);
                GenStructIns(ftype);
            } else {
                Emit(IL_PUSHFLDV, offset);
            }
            GenStructIns(stype);
        } else {
            if (IsStruct(ftype->t)) {
                Emit(IL_PUSHFLD2V, offset);
                GenStructIns(ftype);
            } else {
                Emit(IL_PUSHFLD, offset);
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
                    for (auto &field : udt.fields) {
                        (void)field;
                        etype = etype->Element();
                    }
                }
                auto elemwidth = ValWidth(etype);
                if (struct_elem_sub_width < 0) {
                    Emit(index->exptype->t == V_INT
                        ? (elemwidth == 1 ? IL_VPUSHIDXI : IL_VPUSHIDXI2V)
                        : IL_VPUSHIDXV);
                    GenStructIns(index->exptype);
                } else {
                    // We're indexing a sub-part of the element.
                    auto op = index->exptype->t == V_INT
                        ? (elemwidth == 1 ? IL_VPUSHIDXIS : IL_VPUSHIDXIS2V)
                        : IL_VPUSHIDXVS;
                    Emit(op);
                    GenStructIns(index->exptype);
                    if (op != IL_VPUSHIDXIS) Emit(struct_elem_sub_width);
                    Emit(struct_elem_sub_offset);
                }
                break;
            }
            case V_STRUCT_S:
                assert(index->exptype->t == V_INT && object->exptype->udt->sametype->Numeric());
                Emit(IL_NPUSHIDXI);
                GenStructIns(object->exptype);
                break;
            case V_STRING:
                assert(index->exptype->t == V_INT);
                Emit(IL_SPUSHIDXI);
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
            SetLabel(breaks.back());
            breaks.pop_back();
        }
    }
};

void Nil::Generate(CodeGen &cg, size_t retval) const {
    if (retval) { cg.Emit(IL_PUSHNIL); }
}

void IntConstant::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    if (integer == (int)integer) cg.Emit(IL_PUSHINT, (int)integer);
    else cg.Emit(IL_PUSHINT64, (int)integer, (int)(integer >> 32));
}

void FloatConstant::Generate(CodeGen &cg, size_t retval) const {
    if (retval) { cg.GenFloat(flt); };
}

void StringConstant::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    cg.Emit(IL_PUSHSTR, (int)cg.stringtable.size());
    cg.stringtable.push_back(str);
}

void DefaultVal::Generate(CodeGen &cg, size_t retval) const {
    if (!retval) return;
    // Optional args are indicated by being nillable, but for structs passed to builtins the type
    // has already been made non-nil.
    switch (exptype->ElementIfNil()->t) {
        case V_INT:   cg.Emit(IL_PUSHINT, 0); break;
        case V_FLOAT: cg.GenFloat(0); break;
        default:      cg.Emit(IL_PUSHNIL); break;
    }
}

void IdentRef::Generate(CodeGen &cg, size_t retval) const {
    cg.GenPushVar(retval, sid->type, sid->Idx());
}

void Dot::Generate(CodeGen &cg, size_t retval) const {
    auto stype = children[0]->exptype;
    assert(IsUDT(stype->t));
    auto idx = stype->udt->Has(fld);
    assert(idx >= 0);
    auto &field = stype->udt->fields[idx];
    assert(field.slot >= 0);
    cg.GenPushField(retval, children[0], stype, field.resolvedtype, field.slot);
}

void Indexing::Generate(CodeGen &cg, size_t retval) const {
    cg.GenPushIndex(retval, object, index);
}

void GenericCall::Generate(CodeGen &, size_t /*retval*/) const {
    assert(false);
}

void AssignList::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(children.back(), children.size() - 1);
    for (size_t i = children.size() - 1; i-- > 0; ) {
        auto left = children[i];
        auto id = Is<IdentRef>(left);
        auto llt = id ? id->sid->lt : LT_KEEP /* Dot */;
        cg.GenAssign(left, cg.AssignBaseOp({ left->exptype, llt }), 0, nullptr, 1);
    }
    assert(!retval);  // Type checker guarantees this.
    (void)retval;
}

void Define::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, sids.size());
    for (size_t i = sids.size(); i-- > 0; ) {
        auto sid = sids[i].first;
        cg.TakeTemp(1, true);
        // FIXME: Sadly, even though FunIntro now guarantees that variables start as V_NIL,
        // we still can't replace this with a WRITE that doesn't have to decrement, since
        // loops with inlined bodies cause this def to be execute multiple times.
        // (also: multiple copies of the same inlined function in one parent).
        // We should emit a specialized opcode for these cases only.
        // NOTE: we already don't decref for borrowed vars generated by the optimizer here (!)
        cg.Emit(GENLVALOP(VAR, cg.AssignBaseOp({ *sid })), sid->Idx());
        cg.GenStructIns(sid->type);
    }
    assert(!retval);  // Parser guarantees this.
    (void)retval;
}

void Assign::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, cg.AssignBaseOp({ *right, 0 }), retval, right, 1);
}

void PlusEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LVO_IADD, retval, right, 1);
}
void MinusEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LVO_ISUB, retval, right, 1);
}
void MultiplyEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LVO_IMUL, retval, right, 1);
}
void DivideEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LVO_IDIV, retval, right, 1);
}
void ModEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LVO_IMOD, retval, right, 1);
}
void AndEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LVO_BINAND, retval, right, 1);
}
void OrEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LVO_BINOR, retval, right, 1);
}
void XorEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LVO_XOR, retval, right, 1);
}
void ShiftLeftEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LVO_ASL, retval, right, 1);
}
void ShiftRightEq::Generate(CodeGen &cg, size_t retval) const {
    cg.GenAssign(left, LVO_ASR, retval, right, 1);
}

void PostDecr::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, LVO_IMMP, retval, nullptr, 0); }
void PostIncr::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, LVO_IPPP, retval, nullptr, 0); }
void PreDecr ::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, LVO_IMM,  retval, nullptr, 0); }
void PreIncr ::Generate(CodeGen &cg, size_t retval) const { cg.GenAssign(child, LVO_IPP,  retval, nullptr, 0); }

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
void Plus         ::Generate(CodeGen &cg, size_t retval) const { cg.GenMathOp(this, retval, MOP_ADD); }

void UnaryMinus::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, true);
    auto ctype = child->exptype;
    switch (ctype->t) {
        case V_INT: cg.Emit(IL_IUMINUS); break;
        case V_FLOAT: cg.Emit(IL_FUMINUS); break;
        case V_STRUCT_S: {
            auto elem = ctype->udt->sametype->t;
            cg.Emit(elem == V_INT ? IL_IVUMINUS : IL_FVUMINUS);
            cg.GenStructIns(ctype);
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
    cg.Emit(IL_NEG);
}

void ToFloat::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, false);
    cg.Emit(IL_I2F);
}

void ToString::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, true);
    switch (child->exptype->t) {
        case V_STRUCT_R:
        case V_STRUCT_S: {
            // TODO: can also roll these into A2S?
            cg.Emit(IL_ST2S, cg.GetTypeTableOffset(child->exptype));
            break;
        }
        default: {
            cg.Emit(IL_A2S, cg.GetTypeTableOffset(child->exptype->ElementIfNil()));
            break;
        }
    }
}

void ToBool::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (!retval) return;
    cg.TakeTemp(1, false);
    cg.Emit(IsRefNil(child->exptype->t) ? IL_E2BREF : IL_E2B);
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
                    // TODO: alternatively emit a single op with a list or bitmask?
                    for (int j = 0; j < type->udt->numslots; j++) {
                        if (IsRefNil(FindSlot(*type->udt, j)->resolvedtype->t))
                            cg.Emit(IL_INCREF, stack_offset + j);
                    }
                } else {
                    cg.Emit(IL_INCREF, stack_offset);
                }
            }
            if (decref & (1LL << i)) {
                assert(IsRefNil(type->t));
                if (type->t == V_STRUCT_R) {
                    // TODO: alternatively emit a single op with a list or bitmask?
                    for (int j = 0; j < type->udt->numslots; j++) {
                        if (IsRefNil(FindSlot(*type->udt, j)->resolvedtype->t))
                            cg.EmitKeep(stack_offset + j, 0);
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
    if (sf->parent->anonymous && sf->body) {
        cg.Emit(IL_PUSHFUN, sf->subbytecodestart);
        cg.GenFixup(sf);
    } else {
        cg.Dummy(retval);
    }
}

void EnumRef::Generate(CodeGen &cg, size_t retval) const {
    cg.Dummy(retval);
}

void UDTRef::Generate(CodeGen &cg, size_t retval) const {
    cg.Dummy(retval);
}

void NativeCall::Generate(CodeGen &cg, size_t retval) const {
    if (nf->IsAssert()) {
        // FIXME: lift this into a language feature.
        auto c = children[0];
        if (retval || cg.runtime_checks >= RUNTIME_ASSERT) {
            cg.Gen(c, 1);
            cg.TakeTemp(1, false);
            if (cg.runtime_checks >= RUNTIME_ASSERT) {
                cg.Emit(IL_ASSERT + (!!retval), c->line.line, c->line.fileidx);
                cg.Emit((int)cg.stringtable.size());
                // FIXME: would be better to use the original source code here.
                cg.stringtable.push_back(cg.st.StoreName(DumpNode(*c, 0, true)));
            }
        } else {
            cg.Gen(c, 0);
        }
        return;
    }
    // TODO: could pass arg types in here if most exps have types, cheaper than
    // doing it all in call instruction?
    size_t numstructs = 0;
    for (auto [i, c] : enumerate(children)) {
        cg.Gen(c, 1);
        if ((IsStruct(c->exptype->t) ||
             nf->args[i].flags & NF_PUSHVALUEWIDTH) &&
            !Is<DefaultVal>(c)) {
            cg.GenValueSize(c->exptype);
            cg.temptypestack.push_back({ type_int, LT_ANY });
            numstructs++;
        }
    }
    size_t nargs = children.size();
    cg.TakeTemp(nargs + numstructs, true);
    assert(nargs == nf->args.size() && (nf->fun.fnargs < 0 || nargs <= 7));
    int vmop = nf->fun.fnargs >= 0 ? IL_BCALLRET0 + (int)nargs * 3 : IL_BCALLRETV;
    if (nf->cont1) { // graphics.h
        auto lastarg = children.empty() ? nullptr : children.back();
        if (!Is<DefaultVal>(lastarg)) {
            cg.Emit(vmop, nf->idx);
            // Note: this call is still conditional, since some of these functions dynamically
            // decide to pass a nil lambda even if statically one is given.
            // These functions return the function they're passed + a string (if the function)
            // isn't nil), so we must store the string in a keepvar.
            cg.EmitKeep(1, 2);
            cg.Emit(IL_CALLVCOND);
            cg.SplitAttr(cg.Pos());
            assert(lastarg->exptype->t == V_FUNCTION);
            assert(!lastarg->exptype->sf->reqret);  // We never use the retval.
            cg.Emit(IL_CONT1, nf->idx);  // Never returns a value.
            cg.Dummy(retval);
        } else {
            if (!retval) vmop += 2;  // These always return nil.
            cg.Emit(vmop, nf->idx);
        }
    } else if (nf->CanChangeControlFlow()) {
        cg.Emit(vmop, nf->idx);
        cg.SplitAttr(cg.Pos());
        if (!retval) cg.GenPop({ nattype, natlt });
    } else {
        auto last = nattype->NumValues() - 1;
        auto tlt = TypeLT { nattype->Get(last), nattype->GetLifetime(last, natlt) };
        // FIXME: simplify.
        auto val_width_1 = !IsStruct(tlt.type->t) || tlt.type->udt->numslots == 1;
        auto var_width_void = nf->fun.fnargs < 0 && nf->retvals.empty();
        if (!retval && val_width_1 && !var_width_void) {
            // Generate version that never produces top of stack (but still may have
            // additional return values)
            vmop++;
            if (!cg.ShouldDec(tlt))
                vmop++;
        }
        cg.Emit(vmop, nf->idx);
        if (!retval && !val_width_1) {
            cg.GenPop(tlt);
        }
    }
    if (nf->retvals.size() > 1) {
        assert(nf->retvals.size() == nattype->NumValues());
        for (size_t i = 0; i < nattype->NumValues(); i++) {
            cg.rettypes.push_back({ nattype->Get(i), nattype->GetLifetime(i, natlt) });
        }
    } else {
        assert(nf->retvals.size() >= retval);
    }
    if (!retval) {
        // Top of stack has already been removed by op, but still need to pop any
        // additional values.
        if (cg.rettypes.size()) cg.rettypes.pop_back();
        while (cg.rettypes.size()) {
            cg.GenPop(cg.rettypes.back());
            cg.rettypes.pop_back();
        }
    }
}

void Call::Generate(CodeGen &cg, size_t retval) const {
    cg.GenCall(*sf, vtable_idx, this, retval);
}

void DynCall::Generate(CodeGen &cg, size_t retval) const {
    assert(sf && sf == sid->type->sf);
    // FIXME: in the future, we can make a special case for istype calls.
    if (!sf->parent->istype) {
        // We statically know which function this is calling.
        // We can now turn this into a normal call.
        cg.GenCall(*sf, -1, this, retval);
    } else {
        for (auto c : children) {
            cg.Gen(c, 1);
        }
        size_t nargs = children.size();
        assert(nargs == sf->args.size());
        cg.Emit(IL_PUSHVAR, sid->Idx());
        cg.TakeTemp(nargs, true);
        cg.Emit(IL_CALLV);
        cg.SplitAttr(cg.Pos());
        if (sf->reqret) {
            if (!retval) cg.GenPop({ exptype, lt });
        } else {
            cg.Dummy(retval);
        }
    }
}

void Block::Generate(CodeGen &cg, size_t retval) const {
    assert(retval <= 1);
    for (auto c : children) {
        if (c != children.back()) {
            // Not the last element.
            cg.Gen(c, 0);
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
    cg.Emit(retval ? IL_JUMPFAILR : IL_JUMPFAIL, 0);
    auto loc = cg.Pos();
    cg.Gen(right, retval);
    if (retval) cg.TakeTemp(1, false);
    cg.SetLabel(loc);
}

void Or::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(left, 1);
    cg.TakeTemp(1, false);
    cg.Emit(retval ? IL_JUMPNOFAILR : IL_JUMPNOFAIL, 0);
    auto loc = cg.Pos();
    cg.Gen(right, retval);
    if (retval) cg.TakeTemp(1, false);
    cg.SetLabel(loc);
}

void Not::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (retval) {
        cg.TakeTemp(1, false);
        cg.Emit(IsRefNil(child->exptype->t) ? IL_LOGNOTREF : IL_LOGNOT);
    }
}

void IfThen::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    cg.Emit(IL_JUMPFAIL, 0);
    auto loc = cg.Pos();
    assert(!retval); (void)retval;
    cg.Gen(truepart, 0);
    cg.SetLabel(loc);
}

void IfElse::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    cg.Emit(IL_JUMPFAIL, 0);
    auto loc = cg.Pos();
    cg.Gen(truepart, retval);
    if (retval) cg.TakeTemp(1, true);
    cg.Emit(IL_JUMP, 0);
    auto loc2 = cg.Pos();
    cg.SetLabel(loc);
    cg.Gen(falsepart, retval);
    if (retval) cg.TakeTemp(1, true);
    cg.SetLabel(loc2);
}

void While::Generate(CodeGen &cg, size_t retval) const {
    cg.SplitAttr(cg.Pos());
    auto loopback = cg.Pos();
    cg.loops.push_back(this);
    cg.Gen(condition, 1);
    cg.TakeTemp(1, false);
    cg.Emit(IL_JUMPFAIL, 0);
    auto jumpout = cg.Pos();
    auto break_level = cg.breaks.size();
    cg.Gen(body, 0);
    cg.loops.pop_back();
    cg.Emit(IL_JUMP, loopback);
    cg.SetLabel(jumpout);
    cg.ApplyBreaks(break_level);
    cg.Dummy(retval);
}

void For::Generate(CodeGen &cg, size_t retval) const {
    cg.Emit(IL_PUSHINT, -1);   // i
    cg.temptypestack.push_back({ type_int, LT_ANY });
    cg.Gen(iter, 1);
    cg.loops.push_back(this);
    cg.SplitAttr(cg.Pos());
    auto startloop = cg.Pos();
    auto break_level = cg.breaks.size();
    switch (iter->exptype->t) {
        case V_INT:      cg.Emit(IL_IFOR, 0); break;
        case V_STRING:   cg.Emit(IL_SFOR, 0); break;
        case V_VECTOR:   cg.Emit(IL_VFOR, 0); break;
        default:         assert(false);
    }
    auto exitloop = cg.Pos();
    cg.Gen(body, 0);
    cg.Emit(IL_JUMP, startloop);
    cg.SetLabel(exitloop);
    cg.loops.pop_back();
    cg.TakeTemp(2, false);
    cg.ApplyBreaks(break_level);
    cg.Dummy(retval);
}

void ForLoopElem::Generate(CodeGen &cg, size_t /*retval*/) const {
    auto typelt = cg.temptypestack.back();
    switch (typelt.type->t) {
        case V_INT:    cg.Emit(IL_IFORELEM); break;
        case V_STRING: cg.Emit(IL_SFORELEM); break;
        case V_VECTOR: cg.Emit(IsRefNil(typelt.type->sub->t)
            ? (IsStruct(typelt.type->sub->t) ? IL_VFORELEMREF2S : IL_VFORELEMREF)
            : (IsStruct(typelt.type->sub->t) ? IL_VFORELEM2S : IL_VFORELEM));
            break;
        default:       assert(false);
    }
}

void ForLoopCounter::Generate(CodeGen &cg, size_t /*retval*/) const {
    cg.Emit(IL_FORLOOPI);
}

void Break::Generate(CodeGen &cg, size_t retval) const {
    assert(!retval);
    (void)retval;
    assert(!cg.rettypes.size());
    assert(!cg.loops.empty());
    assert(cg.temptypestack.size() == cg.LoopTemps());
    if (Is<For>(cg.loops.back())) {
        cg.GenPop(cg.temptypestack[cg.temptypestack.size() - 1]);
        cg.GenPop(cg.temptypestack[cg.temptypestack.size() - 2]);
    }
    cg.Emit(IL_JUMP, 0);
    cg.breaks.push_back(cg.Pos());
}

void Switch::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(value, 1);
    cg.TakeTemp(1, false);
    // See if we should do a jump table version.
    if (value->exptype->t == V_INT) {
        int64_t mini = INT64_MAX / 2, maxi = INT64_MIN / 2;
        int64_t num = 0;
        auto get_range = [&](Node *c) -> pair<IntConstant *, IntConstant *> {
            auto start = c;
            auto end = c;
            if (auto r = Is<Range>(c)) {
                start = r->start;
                end = r->end;
            }
            return { Is<IntConstant>(start), Is<IntConstant>(end) };
        };
        for (auto n : cases->children) {
            auto cas = AssertIs<Case>(n);
            for (auto c : cas->pattern->children) {
                auto [istart, iend] = get_range(c);
                if (!istart || !iend || istart->integer > iend->integer)
                    goto no_jump_table;
                num += iend->integer - istart->integer + 1;
                mini = min(mini, istart->integer);
                maxi = max(maxi, iend->integer);
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
            goto no_jump_table;
        // Emit jump table version.
        // We use vtable storage, as these are essentially bytecode address tables.
        int table_start = (int)cg.vtables.size();
        cg.Emit(IL_JUMP_TABLE, (int)mini, (int)maxi, table_start);
        // Store the default case at end of range.
        // TODO: Could make it an immediate, but native backends need to handle it.
        cg.vtables.insert(cg.vtables.end(), range + 1, -1);
        vector<int> exitswitch;
        int default_pos = -1;
        for (auto n : cases->children) {
            auto cas = AssertIs<Case>(n);
            for (auto c : cas->pattern->children) {
                auto [istart, iend] = get_range(c);
                assert(istart && iend);
                for (auto i = istart->integer; i <= iend->integer; i++) {
                    cg.vtables[table_start + (int)i - (int)mini] = cg.Pos();
                }
            }
            if (cas->pattern->children.empty()) default_pos = cg.Pos();
            cg.SplitAttr(cg.Pos());
            cg.Gen(cas->body, retval);
            if (retval) cg.TakeTemp(1, true);
            if (n != cases->children.back()) {
                cg.Emit(IL_JUMP, 0);
                exitswitch.push_back(cg.Pos());
            }
        }
        for (auto loc : exitswitch) cg.SetLabel(loc);
        if (default_pos < 0) default_pos = cg.Pos();
        for (int i = 0; i < (int)range + 1; i++) {
            if (cg.vtables[table_start + i] == -1)
                cg.vtables[table_start + i] = default_pos;
        }
        return;
    }
    no_jump_table:
    // Do slow fall-back for sparse integers, expressions and strings.
    auto valtlt = TypeLT{ *value, 0 };
    vector<int> nextcase, thiscase, exitswitch;
    bool have_default = false;
    for (auto n : cases->children) {
        for (auto loc : nextcase) cg.SetLabel(loc);
        nextcase.clear();
        cg.temptypestack.push_back(valtlt);
        auto cas = AssertIs<Case>(n);
        if (cas->pattern->children.empty()) have_default = true;
        for (auto c : cas->pattern->children) {
            auto is_last = c == cas->pattern->children.back();
            cg.GenDup(valtlt);
            auto compare_one = [&](MathOp op, Node *cn) {
                cg.Gen(cn, 1);
                cg.GenMathOp(value->exptype, c->exptype, value->exptype, op);
            };
            auto compare_one_jump = [&](MathOp op, Node *cn) {
                compare_one(op, cn);
                cg.Emit(is_last ? IL_JUMPFAIL : IL_JUMPNOFAIL, 0);
                (is_last ? nextcase : thiscase).push_back(cg.Pos());
            };
            if (auto r = Is<Range>(c)) {
                compare_one(MOP_GE, r->start);
                cg.Emit(IL_JUMPFAIL, 0);
                auto loc = cg.Pos();
                if (is_last) nextcase.push_back(loc);
                cg.GenDup(valtlt);
                compare_one_jump(MOP_LE, r->end);
                if (!is_last) cg.SetLabel(loc);
            } else {
                // FIXME: if this is a string, will alloc a temp string object just for the sake of
                // comparison. Better to create special purpose opcode to compare with const string.
                compare_one_jump(MOP_EQ, c);
            }
        }
        for (auto loc : thiscase) cg.SetLabel(loc);
        thiscase.clear();
        cg.GenPop(valtlt);
        cg.TakeTemp(1, false);
        cg.Gen(cas->body, retval);
        if (retval) cg.TakeTemp(1, true);
        if (n != cases->children.back() || !have_default) {
            cg.Emit(IL_JUMP, 0);
            exitswitch.push_back(cg.Pos());
        }
    }
    for (auto loc : nextcase) cg.SetLabel(loc);
    if (!have_default) cg.GenPop(valtlt);
    for (auto loc : exitswitch) cg.SetLabel(loc);
}

void Case::Generate(CodeGen &/*cg*/, size_t /*retval*/) const {
    assert(false);
}

void Range::Generate(CodeGen &/*cg*/, size_t /*retval*/) const {
    assert(false);
}

void Constructor::Generate(CodeGen &cg, size_t retval) const {
    // FIXME: a malicious script can exploit this for a stack overflow.
    for (auto c : children) {
        cg.Gen(c, retval);
    }
    if (!retval) return;
    cg.TakeTemp(Arity(), true);
    auto offset = cg.GetTypeTableOffset(exptype);
    if (IsUDT(exptype->t)) {
        assert(exptype->udt->fields.size() == Arity());
        if (IsStruct(exptype->t)) {
            // This is now a no-op! Struct elements sit inline on the stack.
        } else {
            cg.Emit(IL_NEWOBJECT, offset);
        }
    } else {
        assert(exptype->t == V_VECTOR);
        cg.Emit(IL_NEWVEC, offset, (int)Arity());
    }
}

void IsType::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    // If the value was a scalar, then it always results in a compile time type check,
    // which means this T_IS would have been optimized out. Which means from here on we
    // can assume its a ref.
    assert(!IsUnBoxed(child->exptype->t));
    if (retval) {
        cg.TakeTemp(1, false);
        cg.Emit(IL_ISTYPE, cg.GetTypeTableOffset(resolvedtype));
    }
}

void EnumCoercion::Generate(CodeGen &cg, size_t retval) const {
    cg.Gen(child, retval);
    if (retval) cg.TakeTemp(1, false);
}

void Return::Generate(CodeGen &cg, size_t retval) const {
    assert(!cg.rettypes.size());
    auto typestackbackup = cg.temptypestack;
    if (cg.temptypestack.size()) {
        // We have temps on the stack from an enclosing for.
        // We can't actually remove these from the stack permanently as the parent nodes still
        // expect them to be there.
        // Check that these temps are actually from for loops, to not mask bugs.
        assert(cg.temptypestack.size() == cg.LoopTemps());
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
            cg.Emit(IL_PUSHNIL);
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
    cg.Emit(IL_RETURN, sf->parent->idx, nretslots);
    cg.temptypestack = typestackbackup;
    // We can promise to be providing whatever retvals the caller wants.
    for (size_t i = 0; i < retval; i++) cg.rettypes.push_back({ type_undefined, LT_ANY });
}

void TypeOf::Generate(CodeGen &cg, size_t /*retval*/) const {
    if (auto dv = Is<DefaultVal>(child)) {
        if (cg.node_context.size() >= 2) {
            auto parent = cg.node_context[cg.node_context.size() - 2];
            if (Is<NativeCall>(parent)) {
                cg.Emit(IL_PUSHINT, cg.GetTypeTableOffset(parent->exptype));
                return;
            }
        }
        cg.parser.Error("typeof return out of call context", dv);
    } else  if (auto idr = Is<IdentRef>(child)) {
        cg.Emit(IL_PUSHINT, cg.GetTypeTableOffset(idr->exptype));
    } else {
        auto ta = AssertIs<TypeAnnotation>(child);
        cg.Emit(IL_PUSHINT, cg.GetTypeTableOffset(ta->exptype));
    }
}

}  // namespace lobster
