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

// Variables, fields, and indexing, as values and as lvalues, and the definitions, assignments and
// modifying operators that write to them.
// Shared state and emission helpers come from CodeGenBase in codegen_base.h.

namespace lobster {

struct CodeGenLval : virtual CodeGenBase {
    // The pieces a string append is about to add, evaluated onto the temp stack in this order,
    // for the modifier that writes them, see GenStringAppendOps.
    struct AppendOp {
        enum Kind {
            AK_STRING,     // A string.
            AK_VALUE,      // A value that converts to a string as it is appended.
            AK_STRUCT,     // A struct of scalars that does, its slots staged for the helper.
            AK_SUBSTRING,  // A string, a start and a size: the range of it that goes on.
            AK_NUMBER,     // An int, a base and a number of digits, see number_to_string.
        } kind;
        TypeRef type;   // What a value is read as.
        int ti;         // The type index it converts as.
        int width;      // The slots the piece takes.
        bool borrowed;  // A string (or the string of a range) that is a borrow, which may be
                        // the string being appended to, see GenLvalModifier.
    };
    vector<AppendOp> f_sappend;
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
    // When the lvalue is a field stored in part of its slot (see SField::bits): that field,
    // the slot being what the above says. Whatever modifies the lvalue then reads and writes
    // the field's bits of it, see GenLvalModifierPacked.
    const SField *f_lval_packed = nullptr;
    TypeRef f_lval_elem;
    // Where the elements of an LVK_ELEM lvalue are and its index: the lvec/lidx locals, or
    // the variables they came from, see EmitLvalVectorIndex.
    string f_lval_elems, f_lval_index;
    // A struct local staged in an array to be indexed at runtime, which goes back into its
    // variables once the modifier has written it, see EmitLvalStructIndex.
    struct { int idx = 0, width = 0; VKind k = VK_INT; } f_writeback;

    CodeGenLval() {}

    void GenPushVar(size_t retval, TypeRef type, int offset, bool used_as_freevar) override {
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

    void Generate(const IdentRef &node, size_t retval) {
        GenPushVar(retval, node.sid->type, node.sid->Idx(), node.sid->used_as_freevar);
    }

    // Slot i of the lvalue the op chain produced, see f_lval_kind, holding slot i of a value of
    // the given type.
    Place Lval(int i, TypeRef type) {
        switch (f_lval_kind) {
            case LVK_LOCAL: return Local(f_lval_idx + i);
            case LVK_GLOBAL: return Global(f_lval_idx + i);
            case LVK_FIELD: return Field("lo", *f_lval_udt, f_lval_idx + i);
            case LVK_ELEM: return Elem(f_lval_elems, f_lval_elem, f_lval_index, f_lval_idx + i);
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
        f_lval_packed = nullptr;
    }

    // A global is at a known address too, once the generated code can get at the array.
    void EmitLvalGlobal(int offset, TypeRef type) {
        TrackUseDef(0, 0);
        f_lval_kind = LVK_GLOBAL;
        f_lval_idx = offset;
        f_lval_packed = nullptr;
        append(cb, "    // lval: ", IdName(offset, type), "\n");
    }

    // Reading a field is a load at a constant offset from the object, whose fields sit right
    // behind its header. It lands in the slot the object was in.
    void GenPushField(const UDT &udt, int offset, TypeRef ftype) {
        TrackUseDef(1, 1);
        auto obj = Read(Slot(1, VK_OBJECT));
        CopyValue(cb, Slot(1, ftype), Field(obj, udt, offset));
    }

    // Same, once per slot the struct field occupies. The object is only needed to find them, so
    // it is read out of the stack slot the first one lands in before that gets overwritten.
    void GenPushFieldStruct(const UDT &udt, int offset, TypeRef ftype) {
        auto fwidth = ValWidth(ftype);
        TrackUseDef(1, fwidth);
        append(cb, "    {\n    LObject *_o = ", Read(Slot(1, VK_OBJECT)), ";\n");
        for (int i = 0; i < fwidth; i++) {
            CopyValue(cb, Slot(1 - i, ftype, i), Field("_o", udt, offset + i));
        }
        cb += "    }\n";
    }

    void GenPushField(size_t retval, Node *object, TypeRef stype, TypeRef ftype, int offset,
                      const SField *packed = nullptr) {
        if (packed && packed->bits) {
            // A field stored in part of an int slot (see SField::bits): the slot is what gets
            // pushed, out of which the field is then read as the type it is.
            GenPushField(retval, object, stype, type_int, offset);
            if (!retval) return;
            TrackUseDef(1, 1);
            WriteExpr(Slot(1, ftype), ExtractExpr(Slot(1, VK_INT), *packed));
            return;
        }
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
                    GenPushIndex(retval, indexing->object, indexing->index, ftype, offset);
                    return;
                }
            }
        }
        Gen(object, retval);
        if (!retval) return;
        TakeTemp(1, true);
        if (IsStruct(stype->t)) {
            // The field is on the stack already as part of the struct, so this moves its slots
            // down to where the struct starts, in ascending order since those overlap. Its
            // slots may be held as another kind of value than the field is (a reference field
            // of a struct in an abstract struct family, see SlotTypeOf), which is a copy too.
            TrackUseDef(swidth, fwidth);
            auto base = regso - swidth;
            for (int i = 0; i < fwidth; i++) {
                auto d = SlotVar(base + i, RtTypeOf(SlotType(ftype, i)));
                auto s = SlotVar(base + offset + i, RtTypeOf(SlotType(stype, offset + i)));
                if (offset || d.k() != s.k()) CopyValue(cb, d, s);
            }
        } else {
            if (IsStruct(ftype->t)) {
                GenPushFieldStruct(*stype->udt, offset, ftype);
            } else {
                GenPushField(*stype->udt, offset, ftype);
            }
        }
    }

    void Generate(const Dot &node, size_t retval) {
        auto stype = node.child->exptype;
        assert(IsUDT(stype->t));
        auto idx = stype->udt->g.Has(node.fld);
        assert(idx >= 0);
        auto &sfield = stype->udt->sfields[idx];
        assert(sfield.slot >= 0);
        GenPushField(retval, node.child, stype, sfield.type, sfield.slot, &sfield);
    }

    // A field as an lvalue is at a constant offset from the object, same as reading one. That
    // does lose a debug only range check.
    void EmitLvalField(const UDT &udt, int slot, const SField *packed = nullptr) {
        TrackUseDef(1, 0);
        // The object goes in a local, since what follows may write the slot it came in, and
        // its fields are members rather than something an address can point at.
        f_uses_lobj = true;
        append(cb, "    lo = ", Read(Slot(1, VK_OBJECT)), ";\n");
        f_lval_kind = LVK_FIELD;
        f_lval_udt = &udt;
        f_lval_idx = slot;
        f_lval_packed = packed && packed->bits ? packed : nullptr;
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
            GenRangeCheck("_o", "_i", "_o->len");
            if (j) append(cb, "    _o = ((LVector **)", Elems(), ")[_i];\n");
        }
    }

    // An operand that is a variable or a constant, which can be named as often as needed rather
    // than copied into a local first.
    static bool PlainOperand(const Expr &e) {
        return e.prec == 0 && e.pure &&
               (e.vars.empty() || (e.vars.size() == 1 && e.text == e.vars[0]));
    }

    // The range check of an index, whose failure path stays a call.
    void GenRangeCheck(string_view vec, string_view idx, string_view bound) {
        if (cpp) {
            append(cb, "    if ((uint64_t)", idx, " >= (uint64_t)(", bound, ")) vm.IDXErr(", idx,
                   ", ", bound, ", ", vec, ");\n");
        } else {
            append(cb, "    if ((unsigned long long)", idx, " >= (unsigned long long)(", bound,
                   ")) IDXErr(vm, ", idx, ", ", bound, ", ", vec, ");\n");
        }
    }

    // Reading an element out of a vector, or just the part of it asked for, with the index
    // arbitrary, unlike the counter of a loop, see GenForElem. Indexing with a struct steps thru
    // nested vectors, see GenVectorDescent.
    // What lands on the stack is of `subtype`: the element, or the field of it asked for at
    // `offset`, which is held as the slots it comes out of are, see Elem, so a field of a
    // more specific type goes thru a cast, see CopyValue.
    void GenPushIdxNested(int levels, TypeRef elemtype, TypeRef subtype, int offset) {
        auto subwidth = ValWidth(subtype);
        // The vector plus one index per level it steps thru, replaced by the element.
        TrackUseDef(levels + 1, subwidth);
        GenVectorDescent(levels, Slot(levels + 1, VK_VECTOR));
        for (int i = 0; i < subwidth; i++) {
            CopyValue(cb, Slot(levels + 1 - i, subtype, i),
                      Elem(Elems(), elemtype, "_i", offset + i));
        }
        cb += "    }\n";
    }

    // The same for a single level, or for a string. The object is read out into a local first,
    // since the element lands in the slot it came from.
    void GenPushIdx(bool str, TypeRef elemtype, TypeRef subtype, int offset) {
        auto subwidth = ValWidth(subtype);
        // The object and the index it is subscripted with, replaced by the element.
        TrackUseDef(2, subwidth);
        // A string index may read the terminating 0-byte, one past its length.
        auto bound = str ? "_o->len + 1" : "_o->len";
        // A vector and index that are plain are named as they are, unless the element lands in
        // one of them.
        auto vo = Operand(Slot(2, str ? VK_STRING : VK_VECTOR), 15);
        auto io = Operand(Slot(1, VK_INT), 15);
        if (PlainOperand(vo) && PlainOperand(io)) {
            bool clobbers = false;
            if (str) {
                auto d = Slot(2, VK_INT).s;
                clobbers = d == vo.text || d == io.text;
            } else {
                for (int i = 0; i < subwidth; i++) {
                    auto d = Slot(2 - i, subtype, i).s;
                    if (d == vo.text || d == io.text) clobbers = true;
                }
            }
            if (!clobbers) {
                GenRangeCheck(vo.text, io.text, cat(vo.text, str ? "->len + 1" : "->len"));
                if (str) {
                    Write(cb, Slot(2, VK_INT),
                          cpp ? cat("(long long)((unsigned char *)", vo.text, "->data())[",
                                    io.text, "]")
                              : cat("STRING_DATA(", vo.text, ")[", io.text, "]"));
                } else {
                    auto elems = cat(vo.text, cpp ? "->ElemSlots()" : "->elems");
                    for (int i = 0; i < subwidth; i++) {
                        CopyValue(cb, Slot(2 - i, subtype, i),
                                  Elem(elems, elemtype, io.text, offset + i));
                    }
                }
                return;
            }
        }
        append(cb, "    {\n    ", str ? "LString" : "LVector", " *_o = ",
               Read(Slot(2, str ? VK_STRING : VK_VECTOR)), "; long long _i = ",
               Read(Slot(1, VK_INT)), ";\n");
        GenRangeCheck("_o", "_i", bound);
        if (str) {
            Write(cb, Slot(2, VK_INT), cpp ? "(long long)((unsigned char *)_o->data())[_i]"
                                           : "STRING_DATA(_o)[_i]");
        } else {
            for (int i = 0; i < subwidth; i++) {
                CopyValue(cb, Slot(2 - i, subtype, i),
                          Elem(Elems(), elemtype, "_i", offset + i));
            }
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

    // Pushes the element, or with `struct_elem_sub` given, just the field of that type at
    // that offset in it.
    void GenPushIndex(size_t retval, Node *object, Node *index, TypeRef struct_elem_sub = nullptr,
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
                // Either the whole element or just the part of it asked for.
                auto subtype = struct_elem_sub.Null() ? etype : struct_elem_sub;
                auto suboffset = struct_elem_sub.Null() ? 0 : struct_elem_sub_offset;
                if (levels == 1) GenPushIdx(false, etype, subtype, suboffset);
                else GenPushIdxNested(levels, etype, subtype, suboffset);
                break;
            }
            case V_STRUCT_S:
                assert(index->exptype->t == V_INT);
                EmitIndexStruct(object->exptype);
                break;
            case V_STRING:
                assert(index->exptype->t == V_INT);
                GenPushIdx(true, type_int, type_int, 0);
                break;
            default:
                assert(false);
        }
    }

    void Generate(const Indexing &node, size_t retval) {
        GenPushIndex(retval, node.object, node.index);
    }

    // Indexing to get an lvalue hands the address to whatever follows thru a local rather than
    // thru the VM, so they read as a chain of assignments, and none of them leave anything on
    // the stack. An element of a vector is at the width the vector holds its elements at plus
    // wherever in one the assignment lands.
    // `last` says the write follows right away, so that nothing can change the vector or the
    // index in between and, when they are plain, they are named directly. Otherwise, or when
    // the index steps thru nested vectors, they go thru the lvec/lidx locals.
    void EmitLvalVectorIndex(int levels, int offset, TypeRef etype, bool last) {
        TrackUseDef(levels + 1, 0);
        f_lval_kind = LVK_ELEM;
        f_lval_elem = etype;
        f_lval_idx = offset;
        f_lval_packed = nullptr;
        if (levels == 1 && last) {
            auto vo = Operand(Slot(2, VK_VECTOR), 15);
            auto io = Operand(Slot(1, VK_INT), 15);
            if (PlainOperand(vo) && PlainOperand(io)) {
                GenRangeCheck(vo.text, io.text, cat(vo.text, "->len"));
                f_lval_elems = cat(vo.text, cpp ? "->ElemSlots()" : "->elems");
                f_lval_index = io.text;
                return;
            }
        }
        f_uses_lelem = true;
        GenVectorDescent(levels, Slot(levels + 1, VK_VECTOR));
        // The element outlives the block the descent opened, so where it is goes in locals.
        append(cb, "    lvec = ", Elems(), "; lidx = _i;\n    }\n");
        f_lval_elems = "lvec";
        f_lval_index = "lidx";
    }

    // A class indexed at runtime, whose range check needs the type info, so it stays a helper.
    // Every field of one is of the same numeric type, which is what its slots hold.
    void EmitLvalClassIndex(int offset) {
        TrackUseDef(2, 0);
        f_uses_lval = true;
        append(cb, "    lv = RtLvalIndexClass(vm, ", Read(Slot(2, VK_OBJECT)), ", ",
               Read(Slot(1, VK_INT)), ", ", offset, ");\n");
        f_lval_kind = LVK_NUMPTR;
        f_lval_packed = nullptr;
    }

    // A struct indexed at runtime, the one case that steps into the lvalue it was handed.
    void EmitLvalStructIndex(int offset, int numslots) {
        TrackUseDef(1, 0);
        f_uses_lval = true;
        f_lval_packed = nullptr;
        string base;
        auto separate_members = false;
        auto typed = f_lval_kind == LVK_FIELD || f_lval_kind == LVK_NUMPTR ||
                     f_lval_kind == LVK_ELEM || f_lval_kind == LVK_LOCAL;
        auto kind = typed ? LVK_NUMPTR : LVK_PTR;
        if (f_lval_kind == LVK_FIELD) {
            // Start at the exact slot, which may be inside an array or a scalar member.
            // A numeric struct nested in a mixed field can span separate members instead.
            f_lval_idx += offset;
            offset = 0;
            base = cat("&", FieldName("lo", *f_lval_udt, f_lval_idx));
            separate_members = !MemberContainsRange(*f_lval_udt, f_lval_idx, numslots);
        } else if (f_lval_kind == LVK_ELEM) {
            // The same for an element, which a vector of one type holds in a flat run of them.
            f_lval_idx += offset;
            offset = 0;
            base = cat("&", Elem(f_lval_elems, f_lval_elem, f_lval_index, f_lval_idx).s);
            separate_members = !UniformSlots(f_lval_elem, ValWidth(f_lval_elem)) &&
                               !MemberContainsRange(*f_lval_elem->udt, f_lval_idx, numslots);
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
        if (separate_members) {
            // Select the address of a real member rather than stepping a pointer across
            // members, even when their types and physical layout happen to match.
            append(cb, "    switch (_i) {\n");
            for (int i = 0; i < numslots; i++) {
                auto member = f_lval_kind == LVK_FIELD
                    ? FieldName("lo", *f_lval_udt, f_lval_idx + i)
                    : Elem(f_lval_elems, f_lval_elem, f_lval_index, f_lval_idx + i).s;
                append(cb, "        case ", i, ": lv = &", member, "; break;\n");
            }
            append(cb, "    }\n");
        } else {
            append(cb, "    lv = ", base, " + _i", offset ? cat(" + ", offset) : string(), ";\n");
        }
        append(cb, "    }\n");
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

    const char *RcLvalKindName() {
        switch (f_lval_kind) {
            case LVK_LOCAL: return "local";
            case LVK_GLOBAL: return "global";
            case LVK_FIELD: return "field";
            case LVK_ELEM: return "elem";
            default: return "other";
        }
    }

    void GenLvalVar(const SpecIdent &sid, int offset) {
        if (sid.used_as_freevar)
            EmitLvalGlobal(sid.Idx() + offset, sid.type);
        else
            EmitLvalLocal(sid.Idx() + offset);
    }

    LvalOp AssignBaseOp(TypeLT typelt) {
        auto dec = ShouldDec(typelt);
        return IsStruct(typelt.type->t)
            ? (dec ? LV_WRITEREFV : LV_WRITEV)
            : (dec ? LV_WRITEREF : LV_WRITE);
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
        // A string append takes the pieces GenStringAppendOps evaluated.
        int sappend_slots = 0;
        for (auto &ao : f_sappend) sappend_slots += ao.width;
        TrackUseDef(op == LV_SADD ? sappend_slots : LvalModifierUses(op, width), 0);
        if (f_lval_packed) {
            GenLvalModifierPacked(op, type);
            return;
        }
        if (op == LV_WRITE) {
            CopyConsumed(cb, Lval(0, type), Slot(1, type));
        } else if (op == LV_WRITEREF) {
            // Whatever was there loses a reference to make way for what is written over it.
            rc_tag = cat("overwrite:", RcLvalKindName());
            GenDecRef(cb, Lval(0, type));
            rc_tag.clear();
            CopyConsumed(cb, Lval(0, type), Slot(1, type));
        } else if (op == LV_WRITEV || op == LV_WRITEREFV) {
            // Same copy, one per slot of the struct being written, preceded by a decrement for
            // each of those slots that holds a reference, which the bitmask says which are.
            if (op == LV_WRITEREFV) {
                auto bitmask = BitMaskForRefStruct(type);
                rc_tag = cat("overwrite:", RcLvalKindName());
                for (int i = 0; i < width; i++)
                    if ((1 << i) & bitmask) GenDecRef(cb, Lval(i, type));
                rc_tag.clear();
            }
            for (int i = 0; i < width; i++)
                CopyConsumed(cb, Lval(i, type), Slot(width - i, type, i));
        } else if (op == LV_SADD) {
            // The pieces GenStringAppendOps evaluated, bottom of the stack first, each appended
            // by the helper for what it is, see AppendOp. Each append takes over the reference
            // to the string so far, which it may grow in place, see RtSAppend.
            auto n = (int)f_sappend.size();
            // Where each piece starts, as an offset from the top of the stack, see Slot.
            vector<int> start;
            for (int i = 0, pos = 0; i < n; i++) {
                start.push_back(sappend_slots - pos);
                pos += f_sappend[i].width;
            }
            auto slot = [&](int i, int j, VKind k) { return Slot(start[i] - j, k); };
            auto helper = [&](int i) -> string {
                auto &ao = f_sappend[i];
                switch (ao.kind) {
                    case AppendOp::AK_STRING: return "";
                    case AppendOp::AK_VALUE: return KindName(Slot(start[i], ao.type).k());
                    case AppendOp::AK_STRUCT: return "Struct";
                    case AppendOp::AK_SUBSTRING: return "Substring";
                    case AppendOp::AK_NUMBER: return "Number";
                }
                return "";
            };
            auto args = [&](int i) -> string {
                auto &ao = f_sappend[i];
                switch (ao.kind) {
                    case AppendOp::AK_STRING:
                        return Read(slot(i, 0, VK_STRING));
                    case AppendOp::AK_VALUE:
                        return cat(ReadTyped(Slot(start[i], ao.type)), ", (type_elem_t)", ao.ti);
                    case AppendOp::AK_STRUCT:
                        return cat("_ss, (type_elem_t)", ao.ti);
                    case AppendOp::AK_SUBSTRING:
                        return cat(Read(slot(i, 0, VK_STRING)), ", ", Read(slot(i, 1, VK_INT)),
                                   ", ", Read(slot(i, 2, VK_INT)));
                    case AppendOp::AK_NUMBER:
                        return cat(Read(slot(i, 0, VK_INT)), ", ", Read(slot(i, 1, VK_INT)),
                                   ", ", Read(slot(i, 2, VK_INT)));
                }
                return "";
            };
            // A struct goes to its helper as the Values the slots of everything else in memory
            // hold, see EmitStructToString.
            auto stage = [&](int i) {
                auto &ao = f_sappend[i];
                if (ao.kind != AppendOp::AK_STRUCT) return;
                auto ts = TypesOf(ao.type, 1);
                append(cb, "    {\n    Value _ss[", ts.size(), "];\n");
                for (auto [j, t] : enumerate(ts)) {
                    CopyValue(cb, Mem(cat("_ss[", j, "]"), t), Slot(start[i] - (int)j, t));
                }
            };
            auto unstage = [&](int i) {
                if (f_sappend[i].kind == AppendOp::AK_STRUCT) cb += "    }\n";
            };
            // A borrowed piece that is the string being appended to would be read as the appends
            // change or free it, so for their duration such a string keeps a reference of its
            // own: it is then neither grown in place nor freed, and the pieces read it as it was.
            // A field or element holds the only reference to its string, and a piece that
            // converts a vector or object can reach it, so there the string keeps the reference
            // whenever there is such a piece, and its place holds it as it was until the end.
            string self;
            for (int i = 0; i < n; i++) {
                auto &ao = f_sappend[i];
                if (!ao.borrowed) continue;
                if (!self.empty()) self += " || ";
                append(self, ReadAs(Slot(start[i], ao.type), VK_STRING), " == _s");
            }
            if (f_lval_kind == LVK_FIELD || f_lval_kind == LVK_ELEM) {
                for (auto &ao : f_sappend) {
                    auto vt = ao.type->ElementIfNil()->t;
                    if (ao.kind == AppendOp::AK_VALUE && (vt == V_VECTOR || vt == V_CLASS)) {
                        self = "1";
                        break;
                    }
                }
            }
            auto hold = [&](string_view target) {
                if (self.empty()) return;
                if (self == "1") append(cb, "    LString *_sa = ", target, ";\n");
                else append(cb, "    LString *_sa = (", self, ") ? ", target, " : 0;\n");
                rc_tag = "sappend:self";
                GenIncRef(cb, Var("_sa", RTT_STRING));
                rc_tag.clear();
            };
            auto release = [&]() {
                if (self.empty()) return;
                rc_tag = "sappend:self";
                GenDecRef(cb, Var("_sa", RTT_STRING));
                rc_tag.clear();
            };
            if (f_lval_kind == LVK_LOCAL || f_lval_kind == LVK_FIELD ||
                f_lval_kind == LVK_ELEM) {
                // The place may hold the string as another kind of reference (a field of a
                // struct in an abstract struct family, see SlotTypeOf), which is a cast
                // both ways.
                auto v = Lval(0, type);
                append(cb, "    {\n    LString *_s = ", ReadAs(v, VK_STRING), ";\n");
                hold("_s");
                for (int i = 0; i < n; i++) {
                    stage(i);
                    append(cb, "    _s = RtSAppend", helper(i), "(vm, _s, ", args(i), ");\n");
                    unstage(i);
                }
                Write(cb, v, v.k() == VK_STRING ? string("_s") : cat("(", CType(v.k()), ")_s"));
                release();
                cb += "    }\n";
            } else {
                // Appending to a string in memory can free the old one, so it stays a call.
                cb += "    {\n";
                if (!self.empty()) {
                    append(cb, "    LString *_s = ", ReadAs(Lval(0, type), VK_STRING), ";\n");
                }
                hold("_s");
                for (int i = 0; i < n; i++) {
                    stage(i);
                    append(cb, "    RtLvSAdd", helper(i), "(vm, ", LvalPtr(), ", ", args(i),
                           ");\n");
                    unstage(i);
                }
                release();
                cb += "    }\n";
            }
            f_sappend.clear();
        } else if (op >= LV_IPP) {
            auto c = op == LV_IPP || op == LV_FPP ? " + 1" : " - 1";
            auto v = Lval(0, type);
            // An int wraps, see BinExpr.
            Write(cb, v, op == LV_IPP || op == LV_IMM
                             ? cat("(", IType(), ")((", UType(), ")", Read(v), c, ")")
                             : Read(v) + c);
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

    // The same on a field stored in part of an int slot (see SField::bits, f_lval_packed): the
    // slot is what gets read and written, the field's bits taken out of and put into it. Only
    // the ops on a single int or float come here, since such a field is neither a struct nor a
    // reference, and the ones that operate on the field take its value as an expression in the
    // slot above the stack, which is free until the next op.
    void GenLvalModifierPacked(LvalOp op, TypeRef type) {
        auto &sfield = *f_lval_packed;
        auto word = Lval(0, type_int);
        auto isfloat = type->t == V_FLOAT;
        auto k = ScalarKind(isfloat);
        auto cur = Slot(0, k);
        auto write = [&](Expr e) {
            if (HasPending(cur.slot)) pending[cur.slot].expr.clear();
            Write(cb, word, InsertExpr(word, std::move(e), sfield).text);
        };
        if (op == LV_WRITE) {
            // The value written goes into the slot and nowhere else, so its expression is
            // dropped rather than written to the slot it was in as well, as CopyConsumed does.
            auto rhs = Slot(1, k);
            auto e = Operand(rhs, 15);
            if (HasPending(rhs.slot)) pending[rhs.slot].expr.clear();
            write(std::move(e));
        } else if (op >= LV_IPP) {
            WriteExpr(cur, ExtractExpr(word, sfield));
            auto e = Operand(cur, 4);
            e.text += op == LV_IPP || op == LV_FPP ? " + 1" : " - 1";
            e.prec = 4;
            write(std::move(e));
        } else if (op >= LV_BINAND && op <= LV_ASR) {
            WriteExpr(cur, ExtractExpr(word, sfield));
            write(BitExpr(BitOp(op - LV_BINAND), cur, Slot(1, VK_INT)));
        } else {
            assert(op >= LV_IADD && op <= LV_FMOD);
            auto mop = op <= LV_IMOD ? MathOp(op - LV_IADD) : MathOp(op - LV_FADD);
            WriteExpr(cur, ExtractExpr(word, sfield));
            write(BinExpr(isfloat, mop, cur, Slot(1, k)));
        }
    }

    void GenAssignBasic(const SpecIdent &sid) {
        TakeTemp(1, true);
        GenLvalVar(sid, 0);
        auto op = AssignBaseOp({ sid });
        GenLvalModifier(op, sid.type);
    }

    // `more` says the lvalue this produces is stepped into further, with code of its own
    // between here and the write, see EmitLvalVectorIndex.
    void GenAssignLvalRec(const Node *lval, int offset, int take_temp, TypeRef type,
                          bool more = false) {
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
                GenAssignLvalRec(dot->child, sfield.slot + offset, take_temp, type, more);
            }
            // A field stored in part of its slot is the last step of the chain, since it is
            // a scalar, so this is what the modifier sees, see GenLvalModifierPacked.
            if (sfield.bits) f_lval_packed = &sfield;
        } else if (auto indexing = Is<Indexing>(lval)) {
            if (IsStruct(indexing->object->exptype->t)) {
                // This generates an LVAL producing OP which is then indexed below and turned into another LVAL!
                GenAssignLvalRec(indexing->object, offset, take_temp, type, true);
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
                    EmitLvalVectorIndex(levels, offset, etype, !more);
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
                default:
                    assert(false);  // The typechecker rejects the rest, see CheckLval.
            }
        } else {
            Error("lvalue required", lval->line);
        }
    }

    // `sappend` is the pieces a string append adds, see GenStringAppendOps, which the modifier
    // gets only once the lvalue is generated, since generating that may append to a string of
    // its own.
    void GenAssign(const Node *lval, LvalOp lvalop, size_t retval,
                   const Node *rhs, int take_temp, bool post,
                   const vector<AppendOp> *sappend = nullptr) {
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
        if (sappend) f_sappend = *sappend;
        if (!post) {
            GenLvalModifier(lvalop, type);
        }
        if (retval) {
            // FIXME: it seems these never need a refcount increase because they're always
            // borrowed? Be good to assert that somehow.
            auto width = ValWidth(type);
            TrackUseDef(0, width);
            if (f_lval_packed) {
                WriteExpr(Slot(0, type), ExtractExpr(Lval(0, type_int), *f_lval_packed));
            } else {
                for (int i = 0; i < width; i++) CopyValue(cb, Slot(-i, type, i), Lval(i, type));
            }
        }
        if (post) {
            GenLvalModifier(lvalop, type);
        }
    }

    // `lval += rhs` on a string: the pieces of the right hand side go onto the string one by
    // one (see RtSAppend), rather than into a string of their own first. A concatenation
    // contributes its operands, and a conversion the value it converts, which is written out
    // onto the string as it is appended. Every operand is evaluated before anything is
    // appended, so a side effect on the string being appended to lands before the append, as
    // it did when the right hand side was a value of its own.
    void GenStringAppend(const Node *lval, const Node *rhs, size_t retval) {
        node_small_vector ops;
        FlattenConcat(rhs, ops);
        GenStringAppendOps(lval, ops, retval);
    }

    // The pieces stay on the temp stack for the assignment to take, the way a right hand side
    // it generated itself would, so the list of them is only handed over once they are all
    // there: generating one may append to a string of its own.
    void GenStringAppendOps(const Node *lval, const node_small_vector &ops, size_t retval) {
        vector<AppendOp> pieces;
        int values = 0;
        // A string on the stack the appends may find to be the one they append to: a borrow,
        // since an owned one holds a reference of its own, and a constant is never grown.
        auto borrowed = [](const Node *n) {
            n = SkipDecrefWrapper(n);
            return IsBorrow(n->lt) && !Is<StringConstant>(n);
        };
        auto tindex = [&](TypeRef t) { return (int)GetTypeTableOffset(t); };
        for (auto op : ops) {
            auto n = SkipDecrefWrapper(op);
            // An explicit `string(x)` is the conversion the typechecker put in for its argument
            // (see BCG_STRING), so it is the value that converts.
            if (auto nc = Is<NativeCall>(n);
                nc && nc->nf->def.codegen == BCG_STRING && Is<ToString>(nc->children[0])) {
                n = nc->children[0];
            }
            auto nc = Is<NativeCall>(n);
            if (auto ts = Is<ToString>(n)) {
                auto ct = ts->child->exptype;
                if (ct->t == V_STRUCT_S) {
                    Gen(ts->child, 1);
                    pieces.push_back({ AppendOp::AK_STRUCT, ct, tindex(ct), ValWidth(ct), false });
                } else if (ct->t == V_STRUCT_R) {
                    // A struct with references converts thru a string of its own: staged, its
                    // slots would be borrowed references, one of which may be the string
                    // appended to.
                    Gen(op, 1);
                    pieces.push_back({ AppendOp::AK_STRING, type_string, 0, 1, false });
                } else {
                    Gen(ts->child, 1);
                    auto nilstr = ct->t == V_NIL && ct->Element()->t == V_STRING;
                    pieces.push_back({ AppendOp::AK_VALUE, ct, tindex(ct->ElementIfNil()), 1,
                                       nilstr && borrowed(ts->child) });
                }
            } else if (nc && nc->nf->name == "substring") {
                assert(nc->children.size() == 3);
                for (auto c : nc->children) Gen(c, 1);
                pieces.push_back({ AppendOp::AK_SUBSTRING, type_string, 0, 3,
                                   borrowed(nc->children[0]) });
                values += 2;
            } else if (nc && nc->nf->name == "number_to_string") {
                assert(nc->children.size() == 3);
                for (auto c : nc->children) Gen(c, 1);
                pieces.push_back({ AppendOp::AK_NUMBER, type_int, 0, 3, false });
                values += 2;
            } else {
                Gen(op, 1);
                pieces.push_back({ AppendOp::AK_STRING, type_string, 0, 1, borrowed(n) });
            }
            values++;
        }
        GenAssign(lval, LV_IADD, retval, nullptr, values, false, &pieces);
    }

    void Generate(const Assign &node, size_t retval) {
        // `s = s + ..` is `s += ..`, which appends in place, see GenStringAppend.
        if (node.left->exptype->t == V_STRING) {
            if (auto idr = Is<IdentRef>(node.left)) {
                node_small_vector ops;
                FlattenConcat(node.right, ops);
                auto first = Is<IdentRef>(ops[0]);
                if (ops.size() > 1 && first && first->sid == idr->sid) {
                    ops.erase(0);
                    GenStringAppendOps(node.left, ops, retval);
                    return;
                }
            }
        }
        GenAssign(node.left, AssignBaseOp({ *node.right, 0 }), retval, node.right, 1, false);
    }

    void Generate(const AssignList &node, size_t retval) {
        Gen(node.children.back(), node.children.size() - 1);
        for (size_t i = node.children.size() - 1; i-- > 0; ) {
            auto left = node.children[i];
            auto id = Is<IdentRef>(left);
            auto llt = id ? id->sid->lt : LT_KEEP /* Dot */;
            GenAssign(left, AssignBaseOp({ left->exptype, llt }), 0, nullptr, 1, false);
        }
        assert(!retval);  // Type checker guarantees this.
        (void)retval;
    }

    void Generate(const Define &node, size_t retval) {
        if (node.tsids.size() == 1 && node.tsids[0].sid->constprop) {
            return;
        }
        if (Is<DefaultVal>(node.child)) {
            return;  // Pre-decl. 
        }
        Gen(node.child, node.tsids.size());
        for (size_t i = node.tsids.size(); i-- > 0; ) {
            auto sid = node.tsids[i].sid;
            // FIXME: Sadly, even though FunIntro now guarantees that variables start as V_NIL,
            // we still can't replace this with a WRITE that doesn't have to decrement, since
            // loops with inlined bodies cause this def to be execute multiple times.
            // (also: multiple copies of the same inlined function in one parent).
            // We should emit a specialized opcode for these cases only.
            // NOTE: we already don't decref for borrowed vars generated by the optimizer here (!)
            GenAssignBasic(*sid);
        }
        assert(!retval);  // Parser guarantees this.
        (void)retval;
    }

    void Generate(const PlusEq &node, size_t retval) {
        if (node.left->exptype->t == V_STRING) {
            GenStringAppend(node.left, node.right, retval);
            return;
        }
        GenAssign(node.left, LV_IADD, retval, node.right, 1, false);
    }

    void Generate(const MinusEq &node, size_t retval) {
        GenAssign(node.left, LV_ISUB, retval, node.right, 1, false);
    }

    void Generate(const MultiplyEq &node, size_t retval) {
        GenAssign(node.left, LV_IMUL, retval, node.right, 1, false);
    }

    void Generate(const DivideEq &node, size_t retval) {
        GenAssign(node.left, LV_IDIV, retval, node.right, 1, false);
    }

    void Generate(const ModEq &node, size_t retval) {
        GenAssign(node.left, LV_IMOD, retval, node.right, 1, false);
    }

    void Generate(const AndEq &node, size_t retval) {
        GenAssign(node.left, LV_BINAND, retval, node.right, 1, false);
    }

    void Generate(const OrEq &node, size_t retval) {
        GenAssign(node.left, LV_BINOR, retval, node.right, 1, false);
    }

    void Generate(const XorEq &node, size_t retval) {
        GenAssign(node.left, LV_XOR, retval, node.right, 1, false);
    }

    void Generate(const ShiftLeftEq &node, size_t retval) {
        GenAssign(node.left, LV_ASL, retval, node.right, 1, false);
    }

    void Generate(const ShiftRightEq &node, size_t retval) {
        GenAssign(node.left, LV_ASR, retval, node.right, 1, false);
    }

    void Generate(const PostDecr &node, size_t retval) {
        GenAssign(node.child, LV_IMM, retval, nullptr, 0, true);
    }

    void Generate(const PostIncr &node, size_t retval) {
        GenAssign(node.child, LV_IPP, retval, nullptr, 0, true);
    }

    void Generate(const PreDecr &node, size_t retval) {
        GenAssign(node.child, LV_IMM, retval, nullptr, 0, false);
    }

    void Generate(const PreIncr &node, size_t retval) {
        GenAssign(node.child, LV_IPP, retval, nullptr, 0, false);
    }

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

    // The node, which Member alone would not name here, that being CodeGenBase::Member.
    void Generate(const struct Member &node, size_t retval) {
        if (node.frame) {
            GenPushVar(1, node.this_sid->type, node.this_sid->Idx(),
                       node.this_sid->used_as_freevar);
            auto &sfield = node.this_sid->type->udt->sfields[node.field_idx];
            // The marker is the var right after this one.
            auto lab = EmitJumpIfSetThisFrame(true, sfield.slot + ValWidth(sfield.type));
            Gen(node.child, 1);
            GenPushVar(1, node.this_sid->type, node.this_sid->Idx(),
                       node.this_sid->used_as_freevar);
            TakeTemp(1, true);
            EmitLvalField(*node.this_sid->type->udt, sfield.slot, &sfield);
            GenLvalModifier(AssignBaseOp({ sfield.type, LT_KEEP }), sfield.type);
            EmitLabelDef(lab);
        }
        if (!retval) return;
        EmitPushNil(node.exptype);
    }

    void Generate(const Static &node, size_t retval) {
        if (node.frame) {
            assert(node.sid->used_as_freevar);  // Since we'll access these from the freevar buf.
            // The marker is the var right after this one.
            auto lab = EmitJumpIfSetThisFrame(false, node.sid->Idx() + ValWidth(node.sid->type));
            Gen(node.child, 1);
            GenAssignBasic(*node.sid);
            EmitLabelDef(lab);
        }
        if (!retval) return;
        EmitPushNil(node.exptype);
    }
};

}  // namespace lobster
