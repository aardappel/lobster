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

struct Deserializer {
    VM &vm;
    vector<Value> stack;
    vector<bool> is_ref;

    ~Deserializer() {
        assert(stack.size() == is_ref.size());
        for (size_t i = 0; i < stack.size(); i++) {
            if (is_ref[i]) stack[i].ref()->Dec(vm);
        }
    }

    void PushV(Value v, bool ir = false) {
        stack.emplace_back(v);
        is_ref.push_back(ir);
    }

    Value PopV() {
        auto v = stack.back();
        stack.pop_back();
        is_ref.pop_back();
        return v;
    }

    void PopVN(size_t len) {
        for (size_t i = 0; i < len; i++) {
            stack.pop_back();
            is_ref.pop_back();
        }
    }

    Deserializer(VM &vm) : vm(vm) {
        stack.reserve(16);
        is_ref.reserve(16);
    }

    // The slots of a struct or class value, all 0 (which is 0, 0.0 and nil for every kind of
    // slot, and what the bits no field has of a slot must be, see FieldInfo), for its fields to
    // be stored into as they are parsed, see StoreFieldFromTop. For a struct in an abstract
    // struct family, the first slot holds the family index of the member it is.
    size_t ReserveSlots(const TypeInfo &ti) {
        auto base = stack.size();
        for (int i = 0; i < ti.len; i++) PushV(Value(0));
        if (ti.IsFamilyStruct()) stack[base] = Value((iint)ti.family_index);
        return base;
    }

    // Moves the value on top of the stack (all of its slots, for a struct) into field `f` of
    // the struct or class value whose slots start at `base`, see ReserveSlots.
    void StoreFieldFromTop(size_t base, const FieldInfo &f) {
        auto &fti = vm.GetTypeInfo(f.type);
        if (RTIsStruct(fti.t)) {
            auto n = (size_t)fti.len;
            auto from = stack.size() - n;
            for (size_t j = 0; j < n; j++) {
                stack[base + f.slot + j] = stack[from + j];
                is_ref[base + f.slot + j] = is_ref[from + j];
            }
            PopVN(n);
        } else {
            auto ir = is_ref.back();
            auto v = PopV();
            StoreField(&stack[base], f, v);
            is_ref[base + f.slot] = ir;
        }
    }

    // Replaces the slots of a class value on top of the stack (see ReserveSlots) by the object
    // holding them.
    void PushObject(const TypeInfo &ti, type_elem_t typeoff) {
        auto obj = vm.NewObject(ti.len, typeoff);
        if (ti.len) obj->CopyElemsShallow(&stack[stack.size() - (size_t)ti.len], ti.len);
        PopVN(ti.len);
        PushV(obj, true);
    }

    // Pushes the default value of a field (see FieldInfo::defval), or of a type when `f` is
    // just that, false when there is none. For a field that is a struct or class, that is a
    // value with the defaults of all its fields, which come from `slots`: the slots of the
    // struct the field is in when given, which hold the defaults a constructor default of the
    // field gave them (see CodeGen::PushDefaultValues), else from its own type, or for a class
    // from the constructor default the field's default is.
    bool PushDefault(const FieldInfo &f, const TIField *slots) {
        auto &ti = vm.GetTypeInfo(f.type);
        if (ti.is_nil) {
            PushV(NilVal());
            return true;
        }
        switch (ti.t) {
            case RTT_INT:
            case RTT_FLOAT:
                PushV(vm.FieldDefault(f));
                return true;
            case RTT_STRING:
                PushV(vm.NewString(0), true);
                return true;
            case RTT_VECTOR:
                PushV(vm.NewVec(0, 0, f.type), true);
                return true;
            case RTT_STRUCT_S:
            case RTT_STRUCT_R:
            case RTT_CLASS: {
                vector<TIField> agg;
                const TIField *fslots;
                if (ti.t == RTT_CLASS) {
                    fslots = ti.elemtypes;
                    if (f.defval) {
                        // Its own slots, with the defaults of the constructor, per slot.
                        auto dvs = (const type_elem_t *)&vm.GetTypeInfo(f.defval);
                        for (int i = 0; i < ti.len; i++) {
                            agg.push_back(ti.elemtypes[i]);
                            agg.back().defval = dvs[i];
                        }
                        fslots = agg.data();
                    }
                } else {
                    fslots = slots ? slots + f.slot : ti.elemtypes;
                }
                auto dti = &ti;
                if (ti.IsFamilyStruct()) {
                    // The default of the type slot says which member the default value is
                    // (the type itself, or another member the constructor default is of), by
                    // family index, and there is none for the abstract ones (which the root,
                    // at index 0, is).
                    auto index = FamilyIndexOf(vm.GetDefaultScalar<iint>(fslots[0].defval));
                    if (!index) return false;
                    dti = &vm.GetTypeInfo(vm.GetTypeInfo(ti.family_root).FamilyMembers()[index]);
                }
                auto base = ReserveSlots(*dti);
                auto ok = true;
                vm.ForEachField(*dti, fslots, [&](const FieldInfo &sf) {
                    if (!ok) return;
                    if (!PushDefault(sf, fslots)) {
                        ok = false;
                        return;
                    }
                    StoreFieldFromTop(base, sf);
                });
                if (!ok) return false;
                if (ti.t == RTT_CLASS) PushObject(ti, f.type);
                return true;
            }
            default:
                return false;
        }
    }

    bool PushDefault(type_elem_t typeoff) {
        return PushDefault(FieldInfo{ typeoff, 0, 0, 0, false, (type_elem_t)0 }, nullptr);
    }
    pair<const TypeInfo *, type_elem_t> LookupSubClass(string_view sname,
            const TypeInfo *ti, type_elem_t typeoff) {
        // Attempt to find this a subsclass.
        vm.EnsureUDTLookupPopulated();
        auto &udts = vm.UDTLookup[sname];
        for (auto udt : udts) {
            for (auto ludt = udt;;) {
                auto super_idx = ludt->super_idx;
                if (super_idx < 0) break;
                if (super_idx == ti->structidx) {
                    // Note: this field only not -1 for UDTs actually constructed/used.
                    typeoff = (type_elem_t)udt->typeidx;
                    if (typeoff >= 0) {
                        return { &vm.GetTypeInfo(typeoff), typeoff };
                    }
                }
                ludt = &vm.vma.meta->udts[super_idx];
            }
        }
        return { nullptr, (type_elem_t)-1 };
    }
};

struct LobsterBinaryParser : Deserializer {

    LobsterBinaryParser(VM &vm) : Deserializer(vm) {}

    Value Parse(type_elem_t typeoff, const uint8_t *data, const uint8_t *end) {
        ParseElem(data, end, typeoff);
        assert(stack.size() == 1);
        return PopV();
    }

    void Error(const string &s) {
        // FIXME: not great on non-exception platforms, this should not abort.
        THROW_OR_ABORT(cat("lobster_binary_to_value: ", s));
    }

    void Truncated() {
        Error("data truncated");
    }

    void ParseElem(const uint8_t *&data, const uint8_t *end, type_elem_t typeoff) {
        auto base_ti = &vm.GetTypeInfo(typeoff);
        auto ti = base_ti;
        if (ti->is_nil) {
            ti = &vm.GetTypeInfo(typeoff = ti->is_nil);
        }
        if (end == data) Truncated();
        switch (ti->t) {
            case RTT_INT: {
                PushV(DecodeVarintS(data, end));
                break;
            }
            case RTT_FLOAT: {
                float f;
                if (end - data < (ptrdiff_t)sizeof(float)) Truncated();
                memcpy(&f, data, sizeof(float));
                data += sizeof(float);
                PushV(f);
                break;
            }
            case RTT_STRING: {
                auto len = DecodeVarintU(data, end);
                if (!len && base_ti->is_nil) {
                    PushV(NilVal());
                } else {
                    if (len > (uint64_t)(end - data)) Truncated();
                    auto str = vm.NewString(string_view((const char *)data, (size_t)len));
                    data += len;
                    PushV(str, true);
                }
                break;
            }
            case RTT_VECTOR: {
                auto len = DecodeVarintU(data, end);
                if (!len && base_ti->is_nil) {
                    PushV(NilVal());
                } else {
                    auto stack_start = stack.size();
                    for (size_t i = 0; i < len; i++) {
                        ParseElem(data, end, ti->subt);
                    }
                    auto &sti = vm.GetTypeInfo(ti->subt);
                    auto width = RTIsStruct(sti.t) ? sti.len : 1;
                    auto len = iint(stack.size() - stack_start);
                    auto n = len / width;
                    auto vec = vm.NewVec(n, n, typeoff);
                    if (len) vec->CopyElemsShallow(stack.size() - len + stack.data());
                    PopVN(len);
                    PushV(vec, true);
                }
                break;
            }
            case RTT_CLASS: {
                auto elen = (int)DecodeVarintU(data, end);
                if (!elen && base_ti->is_nil) {
                    PushV(NilVal());
                } else {
                    auto ser_id = DecodeVarintU(data, end);
                    typeoff = vm.GetSubClassFromSerID(typeoff, (uint32_t)ser_id);
                    if (typeoff < 0)
                        Error(cat("serialization id ", ser_id, " is not a sub-class of ",
                                  vm.StructName(*ti)));
                    ti = &vm.GetTypeInfo(typeoff);
                    // The fields the writer had (see LObject::ToLobsterBinary), any this
                    // version of the class has beyond them getting their defaults.
                    auto base = ReserveSlots(*ti);
                    int fi = 0;
                    vm.ForEachField(*ti, [&](const FieldInfo &f) {
                        if (fi >= elen) {
                            if (!PushDefault(f, ti->elemtypes))
                                Error("no default value exists for missing field " +
                                      vm.LookupField(ti->structidx, fi));
                        } else {
                            ParseElem(data, end, f.type);
                        }
                        StoreFieldFromTop(base, f);
                        fi++;
                    });
                    if (elen > fi) {
                        // We have fields from a future version of this class, sadly we don't
                        // know how to read past these fields since we have no type data.
                        Error("extra fields presents in " + vm.StructName(*ti));
                    }
                    PushObject(*ti, typeoff);
                }
                break;
            }
            case RTT_STRUCT_S:
            case RTT_STRUCT_R: {
                if (ti->IsFamilyStruct()) {
                    // The member it is by its serializable id, then its fields, like an
                    // object, see VM::StructToLobsterBinary.
                    auto ser_id = DecodeVarintU(data, end);
                    typeoff = vm.GetSubClassFromSerID(typeoff, (uint32_t)ser_id);
                    if (typeoff < 0)
                        Error(cat("serialization id ", ser_id, " is not a sub-struct of ",
                                  vm.StructName(*ti)));
                    ti = &vm.GetTypeInfo(typeoff);
                }
                // NOTE: this provides no protection against structs changing in size,
                // unlike classes. It will simply parse wrong.
                auto base = ReserveSlots(*ti);
                vm.ForEachField(*ti, [&](const FieldInfo &f) {
                    ParseElem(data, end, f.type);
                    StoreFieldFromTop(base, f);
                });
                break;
            }
            default:
                Error("can\'t convert to value: " + ti->Debug(vm, false));
                PushV(NilVal());
                break;
        }
    }
};

}
