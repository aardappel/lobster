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

// Literals, and the constructors of vectors, objects, and structs.
// Shared state and emission helpers come from CodeGenBase in codegen_base.h.

namespace lobster {

struct CodeGenConstructors : virtual CodeGenBase {
    CodeGenConstructors() {}

    void Generate(const IntConstant &node, size_t retval) {
        if (!retval) return;
        EmitPushInt(node.integer);
    }

    void Generate(const FloatConstant &node, size_t retval) {
        if (retval) { GenFloat(node.flt); };
    }

    void Generate(const Nil &node, size_t retval) {
        if (retval) { EmitPushNil(node.exptype); }
    }

    void Generate(const DefaultVal &node, size_t retval) {
        if (!retval) return;
        EmitPushNil(node.exptype);
    }

    // A new object or vector gets its fields or elements written into it from wherever they
    // are once it exists. It is held in a local until they all are, since the slot it ends up
    // in is the first of theirs.
    void EmitNewObject(int type_idx, const Types &args, TypeRef type) {
        auto n = (int)args.size();
        TrackUseDef(n, 1);
        auto base = regso - n;
        auto &udt = *type->udt;
        append(cb, "    {\n    LObject *_o = RtNewObject(vm, (type_elem_t)", type_idx, ", ",
               udt.numslots, ");");
        TypeComment(type);
        // The values as pushed, one slot per scalar field, into the slots of their fields, of
        // which the ones stored in part of a slot go into it together, see GenPackStruct.
        int pos = 0;
        map<int, vector<pair<Place, const SField *>>> packed;
        for (auto &sfield : udt.sfields) {
            auto width = ValWidth(sfield.type);
            if (sfield.bits) {
                packed[sfield.slot].push_back({ SlotVar(base + pos, args[pos]), &sfield });
            } else {
                for (int j = 0; j < width; j++) {
                    CopyValue(cb, Field("_o", udt, sfield.slot + j),
                              SlotVar(base + pos + j, args[pos + j]));
                }
            }
            pos += width;
        }
        assert(pos == n);
        for (auto &[s, vals] : packed) Write(cb, Field("_o", udt, s), PackExpr(vals).text);
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

    void Generate(const VectorConstructor &node, size_t retval) {
        // FIXME: a malicious script can exploit this for a stack overflow.
        int arg_width = 0;
        for (auto c : node.children) {
            Gen(c, retval);
            arg_width += ValWidth(c->exptype);
        }
        if (!retval) return;
        TakeTemp(node.Arity(), true);
        auto offset = GetTypeTableOffset(node.exptype);
        assert(node.exptype->t == V_VECTOR);
        Types args;
        for (auto c : node.children) AddTypes(args, c->exptype);
        assert((int)args.size() == arg_width); (void)arg_width;
        EmitNewVec((int)offset, args, (int)node.Arity(), node.exptype->Element());
    }

    // The values of the fields of a struct as pushed, one slot per scalar field, packed into
    // the slots of the struct where fields share a slot (see SField::bits): each such slot is
    // made from 0 with the values' bits in it, and the whole fields after it move down, which
    // never overwrites a value not moved yet since a field's slot is never above where its
    // value was pushed. Nothing to do for a struct without such fields, whose stack image is
    // its value.
    void GenPackStruct(const UDT &udt, int arg_width) {
        auto any = false;
        for (auto &sfield : udt.sfields) if (sfield.bits) any = true;
        if (!any) {
            assert(arg_width == udt.numslots);
            return;
        }
        TrackUseDef(arg_width, udt.numslots);
        auto base = regso - arg_width;
        int pos = 0;
        vector<int> src(udt.sfields.size());
        for (auto [k, sfield] : enumerate(udt.sfields)) {
            src[k] = pos;
            pos += ValWidth(sfield.type);
        }
        assert(pos == arg_width);
        for (int s = 0; s < udt.numslots; s++) {
            vector<pair<Place, const SField *>> packed;
            for (auto [k, sfield] : enumerate(udt.sfields)) {
                if (sfield.slot != s) continue;
                if (sfield.bits) {
                    packed.push_back({ SlotVar(base + src[k], RtTypeOf(sfield.type)), &sfield });
                    continue;
                }
                if (src[k] == s) continue;  // Already where it goes.
                auto width = ValWidth(sfield.type);
                for (int j = 0; j < width; j++) {
                    auto rtt = RtTypeOf(SlotTypeOf(udt, s + j));
                    CopyValue(cb, SlotVar(base + s + j, rtt), SlotVar(base + src[k] + j, rtt));
                }
            }
            if (!packed.empty()) WriteExpr(SlotVar(base + s, RTT_INT), PackExpr(packed));
        }
    }

    void Generate(const ObjectConstructor &node, size_t retval) {
        if (IsFamilyStruct(node.exptype)) {
            GenerateFamilyStruct(node, retval);
            return;
        }
        int arg_width = 0;
        for (auto c : node.children) {
            Gen(c, retval);
            arg_width += ValWidth(c->exptype);
        }
        if (!retval) return;
        TakeTemp(node.Arity(), true);
        auto offset = GetTypeTableOffset(node.exptype);
        assert(IsUDT(node.exptype->t));
        assert(node.exptype->udt->sfields.size() == node.Arity());
        if (IsStruct(node.exptype->t)) {
            // Struct elements sit inline on the stack, so this is a no-op, except where fields
            // share a slot, which puts them in it.
            GenPackStruct(*node.exptype->udt, arg_width);
        } else {
            Types args;
            for (auto c : node.children) AddTypes(args, c->exptype);
            assert((int)args.size() == arg_width); (void)arg_width;
            EmitNewObject(offset, args, node.exptype);
        }
    }

    // A member of an abstract struct family also sits inline, but has slots that are not fields
    // (see UDT::hidden_sfields): its family index in the first, 0 or nil in the padding. When
    // its fields sit in the slots in declaration order (the common case, see LayoutFamily), what
    // each evaluates to lands right where it goes, with the hidden slots pushed in between.
    // Otherwise every slot is pushed first, and each field moved into its own once evaluated.
    void GenerateFamilyStruct(const ObjectConstructor &node, size_t retval) {
        auto udt = node.exptype->udt;
        assert(udt->sfields.size() == node.Arity());
        if (!retval) {
            for (auto c : node.children) Gen(c, 0);
            return;
        }
        auto push_hidden = [&](int slot) {
            if (!slot) EmitPushInt(udt->FamilyIndex());
            else EmitPushNil(SlotTypeOf(*udt, slot));
        };
        // The value a field evaluates to is held as the kind its own type is, the slots of the
        // struct as the family says (see SlotTypeOf), which for a reference field is a cast.
        auto move_field = [&](size_t i, int from, int to) {
            auto &sfield = udt->sfields[i];
            if (sfield.bits) {
                // Into the bits it has of its slot, which holds 0 (see push_hidden) or the fields
                // before it in there.
                auto d = SlotVar(to, RTT_INT);
                auto s = SlotVar(from, RtTypeOf(sfield.type));
                WriteExpr(d, InsertExpr(d, Operand(s, 15), sfield));
                return 1;
            }
            auto width = ValWidth(sfield.type);
            for (int j = 0; j < width; j++) {
                auto d = SlotVar(to + j, RtTypeOf(SlotTypeOf(*udt, sfield.slot + j)));
                auto s = SlotVar(from + j,
                                 RtTypeOf(SlotType(node.children[i]->exptype, j)));
                if (from != to || d.k() != s.k()) CopyValue(cb, d, s);
            }
            return width;
        };
        auto in_order = true;
        for (size_t i = 0; i < udt->sfields.size(); i++) {
            // Fields sharing a slot are never where they were pushed.
            if (udt->sfields[i].bits) in_order = false;
            if (i && udt->sfields[i].slot < udt->sfields[i - 1].slot) in_order = false;
        }
        if (in_order) {
            int next_slot = 0;
            for (auto [i, c] : enumerate(node.children)) {
                auto &sfield = udt->sfields[i];
                for (; next_slot < sfield.slot; next_slot++) push_hidden(next_slot);
                Gen(c, 1);
                auto top = TempStackSize() - ValWidth(sfield.type);
                next_slot = sfield.slot + move_field(i, top, top);
            }
            for (; next_slot < udt->numslots; next_slot++) push_hidden(next_slot);
            TakeTemp(node.Arity(), true);
            return;
        }
        auto base = TempStackSize();
        for (int s = 0; s < udt->numslots; s++) push_hidden(s);
        for (auto [i, c] : enumerate(node.children)) {
            Gen(c, 1);
            auto top = TempStackSize() - ValWidth(udt->sfields[i].type);
            auto width = move_field(i, top, base + udt->sfields[i].slot);
            TakeTemp(1, true);
            TrackUseDef(width, 0);
        }
    }

    void Generate(const EnumCoercion &node, size_t retval) {
        Gen(node.child, retval);
        if (retval) TakeTemp(1, false);
    }
};

}  // namespace lobster
