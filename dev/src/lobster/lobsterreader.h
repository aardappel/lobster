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

    // Pushes a value of a struct in an abstract struct family, see TypeInfo::IsFamilyStruct:
    // `field(f, slot, eti)` pushes the value of each field of `dti` (the member it is) in
    // turn, in field order, which is the order the serialized forms have them in. Those
    // then get moved to the slots the layout has the fields at (in whatever order, see
    // SymbolTable::LayoutFamily), with the family index of the member in the first slot,
    // and 0 or nil in the padding, by the kind of value the slot holds.
    template<typename F> void PushFamilyStruct(const TypeInfo &dti, bool push, F field) {
        auto &fields = vm.vma.meta->udts[dti.structidx].fields;
        auto start = stack.size();
        vector<size_t> starts;
        for (size_t f = 0; f < fields.size(); f++) {
            starts.push_back(stack.size() - start);
            auto slot = fields[f].offset;
            field(f, slot, dti.GetElemOrParent(slot));
        }
        if (!push) return;
        vector<Value> vals(stack.begin() + start, stack.end());
        vector<bool> refs(is_ref.begin() + start, is_ref.end());
        stack.erase(stack.begin() + start, stack.end());
        is_ref.erase(is_ref.begin() + start, is_ref.end());
        for (int s = 0; s < dti.len;) {
            if (!s) {
                PushV((iint)dti.family_index);
                s++;
                continue;
            }
            size_t f = 0;
            while (f < fields.size() && fields[f].offset != s) f++;
            if (f < fields.size()) {
                auto &ti = vm.GetTypeInfo(dti.GetElemOrParent(s));
                auto width = RTIsStruct(ti.t) ? ti.len : 1;
                for (int j = 0; j < width; j++) {
                    auto k = starts[f] + j;
                    // A field the parser could not complete leaves a gap, which the error
                    // it reports covers.
                    if (k < vals.size()) PushV(vals[k], refs[k]); else PushV(NilVal());
                }
                s += width;
            } else {
                auto &pti = vm.GetTypeInfo(dti.elemtypes[s].type);
                if (pti.t == RTT_INT) PushV((iint)0); else PushV(NilVal());
                s++;
            }
        }
    }

    bool PushDefault(type_elem_t typeoff, type_elem_t defval, const TIField *fields) {
        auto &ti = vm.GetTypeInfo(typeoff);
        if (ti.is_nil) {
            PushV(NilVal());
            return true;
        }
        if (ti.IsFamilyStruct()) {
            // The default of the type slot says which member the default value is (the
            // type itself, or another member the parent's default constructs), by family
            // index, and there is none for the abstract ones (which the root, at index 0,
            // is).
            auto index = vm.GetDefaultScalar<iint>(fields ? fields[0].defval
                                                          : ti.elemtypes[0].defval);
            if (!index) return false;
            auto &root = vm.GetTypeInfo(ti.family_root);
            auto &dti = vm.GetTypeInfo(root.FamilyMembers()[index]);
            auto ok = true;
            PushFamilyStruct(dti, true, [&](size_t, int slot, type_elem_t eti) {
                auto dv = fields ? fields[slot].defval : dti.elemtypes[slot].defval;
                if (ok && !PushDefault(eti, dv, fields ? &fields[slot] : &dti.elemtypes[slot]))
                    ok = false;
            });
            return ok;
        }
        switch (ti.t) {
            case RTT_INT: {
                auto dv = vm.GetDefaultScalar<iint>(defval);
                PushV(dv);
                break;
            }
            case RTT_FLOAT: {
                auto dv = vm.GetDefaultScalar<double>(defval);
                PushV(dv);
                break;
            }
            case RTT_STRING:
                PushV(vm.NewString(0), true);
                break;
            case RTT_VECTOR:
                PushV(vm.NewVec(0, 0, typeoff), true);
                break;
            case RTT_STRUCT_S:
            case RTT_STRUCT_R:
            case RTT_CLASS: {
                 if (defval) {
                    // The parent's field's default value was a constructor with all constant values.
                    for (int i = 0; i < ti.len; i++) {
                        auto dv = ti.t == RTT_CLASS
                            ? ((type_elem_t *)&vm.GetTypeInfo(defval))[i]
                            : fields[i].defval;
                        auto ok = PushDefault(ti.elemtypes[i].type, dv, nullptr);
                        assert(ok);  // Codegen should only have emitted these for types we can handle.
                        if (!ok)
                            return false;
                    }
                } else {
                    // No constructor default, can only succeed if all fields have default values.
                    for (int i = 0; i < ti.len; i++) {
                        // This deals with structs inline.
                        if (!PushDefault(ti.elemtypes[i].type, ti.elemtypes[i].defval, nullptr))
                            return false;
                    }
                }
                if (ti.t == RTT_CLASS) {
                    auto vec = vm.NewObject(ti.len, typeoff);
                    if (ti.len) vec->CopyElemsShallow(&stack[stack.size() - (size_t)ti.len], ti.len);
                    PopVN(ti.len);
                    PushV(vec, true);
                }
                break;
            }
            default:
                return false;
        }
        return true;
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
                    auto stack_start = stack.size();
                    auto NumElems = [&]() { return iint(stack.size() - stack_start); };
                    for (int i = 0; NumElems() != ti->len; i++) {
                        auto eti = ti->GetElemOrParent(NumElems());
                        if (NumElems() >= elen) {
                            if (!PushDefault(eti, ti->elemtypes[NumElems()].defval,
                                             &ti->elemtypes[NumElems()]))
                                Error("no default value exists for missing field " +
                                      vm.LookupField(ti->structidx, i));
                        } else {
                            ParseElem(data, end, eti);
                        }
                    }
                    if (elen > NumElems()) {
                        // We have fields from a future version of this class, sadly we don't
                        // know how to read past these fields since we have no type data.
                        Error("extra fields presents in " + vm.StructName(*ti));
                    }
                    auto len = NumElems();
                    auto vec = vm.NewObject(len, typeoff);
                    if (len) vec->CopyElemsShallow(stack.size() - len + stack.data(), len);
                    PopVN(len);
                    PushV(vec, true);
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
                    PushFamilyStruct(*ti, true, [&](size_t, int, type_elem_t eti) {
                        ParseElem(data, end, eti);
                    });
                    break;
                }
                auto stack_start = stack.size();
                auto NumElems = [&]() { return iint(stack.size() - stack_start); };
                // NOTE: this provides no protection against structs changing in size,
                // unlike classes. It will simply parse wrong.
                while (NumElems() != ti->len) {
                    auto eti = ti->GetElemOrParent(NumElems());
                    ParseElem(data, end, eti);
                }
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
