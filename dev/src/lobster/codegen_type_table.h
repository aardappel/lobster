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

// Runtime type tables, field defaults, subtype numbering, and the type tests and type values
// made from them.
// Shared state and emission helpers come from CodeGenBase in codegen_base.h.

namespace lobster {

struct CodeGenTypeTable : virtual CodeGenBase {
    map<small_vector<type_elem_t, 2>, type_elem_t> type_lookup;  // Wasteful, but simple.
    map<iint, type_elem_t> default_ints_lookup;
    map<double, type_elem_t> default_floats_lookup;
    map<small_vector<type_elem_t, 3>, type_elem_t> default_aggregate_lookup;
    map<vector<type_elem_t>, type_elem_t> packed_fields_lookup;

    const int ti_num_udt_fields = 12;
    const int ti_num_udt_per_field = 4;

    CodeGenTypeTable() {}

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
            // The defaults per slot (see PushFields), a slot that several fields share
            // getting the whole slot with all of theirs in it, see TIField::defval.
            UDT *udt;
            if (IsFamilyStruct(cons->exptype)) {
                // This may be a different member than the static type of the field (which
                // is what its exptype was set to, see EnsureUDTChecked), so the type slot
                // says which: its family index is what makes the default value that
                // member. A given type that still involves type variables can't say.
                auto ctype = cons->giventype;
                if (!IsUDT(ctype->t) || !ctype->udt->family_root) return (type_elem_t)0;
                udt = ctype->udt;
            } else {
                if (!IsUDT(cons->exptype->t)) return (type_elem_t)0;
                udt = cons->exptype->udt;
            }
            udt->ComputeSizes(st);
            if (vals.size() != udt->sfields.size()) return (type_elem_t)0;
            vector<type_elem_t> idxs(udt->numslots, (type_elem_t)0);
            vector<uint64_t> words(udt->numslots, 0);
            vector<bool> shared(udt->numslots, false);
            if (udt->family_root) {
                // The type slot, which fields may share, see UDT::family_type_slot_shared.
                words[0] = (uint64_t)udt->FamilyIndex();
                shared[0] = true;
            }
            for (auto [i, v] : enumerate(vals)) {
                auto &fsf = udt->sfields[i];
                if (fsf.bits) {
                    words[fsf.slot] |= PackedFieldBits(fsf, v.first, v.second);
                    shared[fsf.slot] = true;
                } else {
                    idxs[fsf.slot] = PushDefaultValue(v.first, v.second);
                }
            }
            for (auto [s, word] : enumerate(words)) {
                if (shared[s]) idxs[s] = PushDefaultValue(V_INT, VTValue((int64_t)word));
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

    // The bits a constant default of a field stored in part of its slot (see SField::bits)
    // contributes to the default of the whole slot, see TIField::defval.
    static uint64_t PackedFieldBits(const SField &sfield, ValueType vt, VTValue val) {
        auto bits = vt == V_FLOAT ? (uint64_t)PackFloat32(val.f) : (uint64_t)val.i;
        return (bits & ((1ULL << sfield.bits) - 1)) << sfield.bitoff;
    }

    // The entries describing the fields a slot holds in part of itself (see TIField::packed),
    // interned like the defaults are.
    type_elem_t PushPackedFields(const vector<const SField *> &fields) {
        vector<type_elem_t> entries;
        entries.push_back((type_elem_t)fields.size());
        for (auto sfield : fields) {
            entries.push_back(GetTypeTableOffset(sfield->type));
            entries.push_back((type_elem_t)sfield->bitoff);
            entries.push_back((type_elem_t)sfield->bits);
        }
        auto &it = packed_fields_lookup[entries];
        if (!it) {
            it = (type_elem_t)type_table.size();
            type_table.insert(type_table.end(), entries.begin(), entries.end());
        }
        return it;
    }

    // The entry of a slot that holds fields in part of itself (see TIField::packed): an int
    // slot, whatever the fields are, whose default is the whole slot with the constant defaults
    // of all of them in it (a field without one contributing 0, which is what no default means
    // for a scalar anyway), and `extra` besides: the family index for the type slot of a
    // struct in an abstract struct family, see UDT::family_type_slot_shared.
    void PushPackedSlot(UDT *udt, int slot, small_vector<type_elem_t, 2> &tt, type_elem_t parent,
                        type_elem_t dvs_overrides, uint64_t extra = 0) {
        vector<const SField *> fields;
        for (auto &sfield : udt->sfields) {
            if (sfield.slot == slot && sfield.bits) fields.push_back(&sfield);
        }
        type_elem_t dvs;
        if (dvs_overrides) {
            dvs = type_table[dvs_overrides + slot];
        } else {
            auto word = extra;
            for (auto sfield : fields) {
                VTValue val;
                auto vt = sfield->defaultval ? sfield->defaultval->ConstVal(nullptr, val)
                                             : V_UNDEFINED;
                if (vt == V_INT || vt == V_FLOAT) word |= PackedFieldBits(*sfield, vt, val);
            }
            dvs = PushDefaultValue(V_INT, VTValue((int64_t)word));
        }
        tt.push_back(TYPE_ELEM_INT);
        tt.push_back(parent);
        tt.push_back(dvs);
        tt.push_back(PushPackedFields(fields));
    }

    // The entries of the fields of a type, one per slot (see TypeInfo::elemtypes), those of a
    // field that is a struct flattened in, all with that struct as their parent. The defaults
    // are per slot too: from `dvs_overrides` when the type is the default value of a field of
    // another (see PushDefaultValues), else those of the fields themselves.
    void PushFields(UDT *udt, small_vector<type_elem_t, 2> &tt,
                    type_elem_t parent = (type_elem_t)-1,
                    type_elem_t dvs_overrides = (type_elem_t)0) {
        if (udt->family_root) {
            PushFamilyFields(udt, tt, parent, dvs_overrides);
            return;
        }
        for (auto &sfield : udt->sfields) {
            if (sfield.type.Null()) {
                // An inferred field of a class declared in a function that is never used, so it
                // never got a type. Nothing can construct it, and ComputeSizes already gave it
                // the single slot described here.
                tt.push_back(TYPE_ELEM_ANY);
                tt.push_back(parent);
                tt.push_back((type_elem_t)0);
                tt.push_back((type_elem_t)-1);
                continue;
            }
            if (sfield.bits) {
                // The first field in the slot describes it along with the others in it.
                if (!sfield.bitoff) PushPackedSlot(udt, sfield.slot, tt, parent, dvs_overrides);
                continue;
            }
            auto ti = GetTypeTableOffset(sfield.type);
            auto dvs = PushDefaultValues(sfield);
            if (IsStruct(sfield.type->t)) {
                // FIXME: in this case, we don't actually need to store the "dvs" list since it's used inline.
                PushFields(sfield.type->udt, tt, parent < 0 ? ti : parent, dvs);
            } else {
                tt.push_back(ti);
                tt.push_back(parent);
                tt.push_back(dvs_overrides ? type_table[dvs_overrides + sfield.slot] : dvs);
                tt.push_back((type_elem_t)-1);
            }
        }
    }

    // The same for a member of an abstract struct family, whose slots are not all fields,
    // and whose fields are in whatever slots the layout gave them (see
    // SymbolTable::LayoutFamily), so this goes by slot. The hidden slots (see
    // UDT::hidden_sfields) are described by their own type, and get no default except the
    // type slot, whose default is the family index of this very type, since that is what a
    // default constructed value of it holds (nothing for an abstract one, which no value
    // is). Defaults given for a field of such a type are per slot as well, since they may
    // be for another member than the field's type, see PushDefaultValues.
    void PushFamilyFields(UDT *udt, small_vector<type_elem_t, 2> &tt, type_elem_t parent,
                          type_elem_t dvs_overrides) {
        for (int s = 0; s < udt->numslots;) {
            const SField *sfield = nullptr;
            // Of the fields sharing a slot, the lowest describes it (see PushPackedSlot),
            // which in the type slot sits above the family index.
            for (auto &sf : udt->sfields) {
                if (sf.slot == s && (!sfield || sf.bitoff < sfield->bitoff)) sfield = &sf;
            }
            if (!sfield) {
                tt.push_back(GetTypeTableOffset(FindSlot(*udt, s)->type));
                tt.push_back(parent);
                if (dvs_overrides) {
                    tt.push_back(type_table[dvs_overrides + s]);
                } else if (!s && !udt->g.is_abstract) {
                    tt.push_back(PushDefaultValue(V_INT, VTValue((int64_t)udt->FamilyIndex())));
                } else {
                    tt.push_back((type_elem_t)0);
                }
                tt.push_back((type_elem_t)-1);
                s++;
            } else if (sfield->type.Null()) {
                // See PushFields.
                tt.push_back(TYPE_ELEM_ANY);
                tt.push_back(parent);
                tt.push_back((type_elem_t)0);
                tt.push_back((type_elem_t)-1);
                s++;
            } else if (sfield->bits) {
                // The type slot holds the family index besides the fields sharing it.
                auto index = !s && !udt->g.is_abstract ? (uint64_t)udt->FamilyIndex() : 0;
                PushPackedSlot(udt, s, tt, parent, dvs_overrides, index);
                s++;
            } else {
                auto ti = GetTypeTableOffset(sfield->type);
                auto dvs = PushDefaultValues(*sfield);
                if (IsStruct(sfield->type->t)) {
                    PushFields(sfield->type->udt, tt, parent < 0 ? ti : parent, dvs);
                } else {
                    tt.push_back(ti);
                    tt.push_back(parent);
                    tt.push_back(dvs_overrides ? type_table[dvs_overrides + s] : dvs);
                    tt.push_back((type_elem_t)-1);
                }
                s += ValWidth(sfield->type);
            }
        }
    }

    // Make a table for use as VM runtime type.
    type_elem_t GetTypeTableOffset(TypeRef type,
                                   type_elem_t non_nil_version = (type_elem_t)0) override {
        small_vector<type_elem_t, 2> tt;
        tt.push_back((type_elem_t)VT2RT(type->t));
        tt.push_back(non_nil_version);
        switch (type->t) {
            case V_INT:
                tt.push_back((type_elem_t)(type->e ? type->e->idx : -1));
                break;
            case V_NIL:
                // The element of a nil may still be an unbound variable (a nil nothing ever
                // gave a type), but the typechecker never binds one to a type that cannot
                // be nilable, see TypeChecker::UnifyVar.
                assert(type->sub->t == V_VAR || type->sub->t == V_UNDEFINED ||
                       st.IsNillable(type->sub));
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
                auto &typeinfo = (non_nil_version ? udt_nil_type_offsets
                                                 : udt_type_offsets)[udt->idx];
                if (typeinfo >= 0)
                    return typeinfo;
                typeinfo = (type_elem_t)type_table.size();
                // Reserve space, so other types can be added afterwards safely.
                assert(udt->numslots >= 0);
                // The root of an abstract struct family also lists the family's members,
                // by family index, see TypeInfo::FamilyMembers.
                auto root = udt->family_root;
                auto family_size = root == udt ? root->subtype_dfs_end - root->subtype_dfs + 1 : 0;
                auto ttsize = (udt->numslots * ti_num_udt_per_field) + ti_num_udt_fields +
                              family_size;
                type_table.insert(type_table.end(), ttsize, (type_elem_t)0);
                tt.push_back((type_elem_t)udt->idx);
                tt.push_back((type_elem_t)udt->numslots);
                // A vtable is only of use to a value that carries its dynamic type, which
                // a struct does only in an abstract struct family.
                tt.push_back((type_elem_t)(type->t == V_CLASS || root ? udt->vtable_start : -1));
                tt.push_back((type_elem_t)(type->t == V_CLASS ? 0 : ComputeBitMask(*udt)));
                tt.push_back(!udt->ssuperclass
                    ? (type_elem_t)-1
                    : GetTypeTableOffset(&udt->ssuperclass->thistype));
                tt.push_back((type_elem_t)udt->serializable_id);
                tt.push_back((type_elem_t)udt->subtype_dfs);
                tt.push_back(root ? GetTypeTableOffset(&root->thistype) : (type_elem_t)-1);
                tt.push_back((type_elem_t)(root ? udt->FamilyIndex() : -1));
                tt.push_back((type_elem_t)family_size);
                PushFields(udt, tt);
                if (family_size) {
                    vector<UDT *> members;
                    for (auto m : st.udttable) {
                        if (m->family_root == root) members.push_back(m);
                    }
                    sort(members.begin(), members.end(),
                         [](UDT *a, UDT *b) { return a->subtype_dfs < b->subtype_dfs; });
                    assert(ssize(members) == family_size);
                    for (auto m : members) tt.push_back(GetTypeTableOffset(&m->thistype));
                }
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

    void AssignSubtypeIDs() {
        // Assign ids to all UDTs in depth-first pre-order over the inheritance
        // forest, such that the ids of any UDT's subtree (including itself)
        // form a contiguous range, allowing ISSUBTYPE to test for subtype
        // membership with a single range check. The set of UDTs is complete
        // here, so subclasses participate regardless of where they were
        // declared relative to uses of "is". The same ids, relative to the root, are what
        // the members of an abstract struct family go by, see UDT::FamilyIndex.
        // Only the resulting ranges belong to the UDTs. The child lists are scratch for
        // this traversal, derived from the already resolved superclass links.
        vector<vector<UDT *>> children(st.udttable.size());
        for (auto udt : st.udttable)
            if (udt->ssuperclass) children[udt->ssuperclass->idx].push_back(udt);
        int subtype_id = 0;
        auto assign_ids = [&](UDT *udt, auto &&assign_ids) -> void {
            udt->subtype_dfs = subtype_id++;
            // Preserve the reverse declaration order of the former prepended child lists.
            for (auto sub : reverse(children[udt->idx]))
                assign_ids(sub, assign_ids);
            udt->subtype_dfs_end = subtype_id - 1;
        };
        for (auto udt : st.udttable)
            if (!udt->ssuperclass) assign_ids(udt, assign_ids);
    }

    // The optimizer guarantees what is tested is never a scalar, so it is a reference or nil,
    // and whether nil matches was decided statically. A string constant has a type index of its
    // own, so a test for string accepts both, see TYPE_ELEM_STRING_CONST.
    void EmitIsType(int type_idx, int nilres, TypeRef type, TypeRef vtype) {
        TrackUseDef(1, 1);
        auto v = Read(Slot(1, vtype));
        auto ti = cat(v, cpp ? "->tti == (type_elem_t)" : "->typeinfo == ");
        auto test = cat(ti, type_idx);
        if (type_idx == TYPE_ELEM_STRING) {
            test = cat("(", test, " || ", ti, (int)TYPE_ELEM_STRING_CONST, ")");
        }
        Write(cb, Slot(1, VK_INT), cat(v, " ? ", test, " : ", nilres), "");
        TypeComment(type);
    }

    void EmitIsSubType(int start, int end, int nilres, TypeRef type, TypeRef vtype) {
        TrackUseDef(1, 1);
        Write(cb, Slot(1, VK_INT), cat("RtIsSubType(vm, ", ReadAs(Slot(1, vtype), VK_OBJECT), ", ", start,
                                      ", ", end, ", ", nilres, ")"), "");
        comment(type->udt->name);
    }

    // The same test on a value of a struct in an abstract struct family, whose type slot
    // (the first of its `width` slots, which it gives up, being borrowed) holds its family
    // index, see UDT::FamilyIndex: a single range check when the tested type has subtypes,
    // since their indices are contiguous, a compare against the one index otherwise.
    void EmitIsTypeStruct(int width, TypeRef type) {
        TrackUseDef(width, 1);
        auto udt = type->udt;
        auto id = FamilyIndexText(Slot(width, VK_INT), *udt->family_root);
        auto lo = udt->FamilyIndex();
        auto hi = lo + udt->subtype_dfs_end - udt->subtype_dfs;
        auto test = lo == hi ? cat(id, " == ", lo)
                             : cat("(unsigned long long)(", id, " - ", lo, ") <= ", hi - lo);
        Write(cb, Slot(width, VK_INT), test, "");
        comment(udt->name);
    }

    void Generate(const IsType &node, size_t retval) {
        Gen(node.child, retval);
        // If the value was a scalar, then it always results in a compile time type check,
        // which means this T_IS would have been optimized out. Which means from here on we
        // can assume its a ref.
        assert(!IsUnBoxed(node.child->exptype->t));
        if (retval) {
            if (IsStruct(node.child->exptype->t)) {
                // Only a struct in an abstract struct family gets here (see IsType::ConstVal),
                // whose type slot decides, against a tested type that is one of its subtypes.
                TakeTemp(1, true);
                EmitIsTypeStruct(ValWidth(node.child->exptype), node.resolvedtype);
                return;
            }
            TakeTemp(1, false);
            // Whether a nil value matches is resolved at compile time, so both ops
            // only ever compare against the non-nil type.
            int nilres = node.AcceptsNil();
            TypeRef te = node.resolvedtype->ElementIfNil();
            if (te->t == V_CLASS && te->udt->subtype_dfs_end > te->udt->subtype_dfs) {
                // The tested type has subclasses, so test the value's type against
                // the id range of the tested type's subtree.
                EmitIsSubType(te->udt->subtype_dfs, te->udt->subtype_dfs_end, nilres, te,
                              node.child->exptype);
            } else {
                EmitIsType(GetTypeTableOffset(te), nilres, te, node.child->exptype);
            }
        }
    }

    void Generate(const TypeOf &node, size_t /*retval*/) {
        // This is typically a TypeAnnotation or IdentRef, but
        // the IdentRef can also have been folded into various constants.
        EmitPushInt(GetTypeTableOffset(node.child->exptype));
    }
};

}  // namespace lobster
