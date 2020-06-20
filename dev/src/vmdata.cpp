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

#include "lobster/stdafx.h"

#include "lobster/vmdata.h"

namespace lobster {

LString::LString(iint _l) : RefObj(TYPE_ELEM_STRING), len(_l) { ((char *)data())[_l] = 0; }

LResource::LResource(void *v, const ResourceType *t)
    : RefObj(TYPE_ELEM_RESOURCE), val(v), type(t) {}

char HexChar(char i) { return i + (i < 10 ? '0' : 'A' - 10); }

void EscapeAndQuote(string_view s, string &sd) {
    sd += '\"';
    for (auto c : s) switch(c) {
        case '\n': sd += "\\n"; break;
        case '\t': sd += "\\t"; break;
        case '\r': sd += "\\r"; break;
        case '\\': sd += "\\\\"; break;
        case '\"': sd += "\\\""; break;
        case '\'': sd += "\\\'"; break;
        default:
            if (c >= ' ' && c <= '~') {
                sd += c;
            } else {
                sd += "\\x";
                sd += HexChar(((uint8_t)c) >> 4);
                sd += HexChar(c & 0xF);
            }
            break;
    }
    sd += "\"";
}

void LString::DeleteSelf(VM &vm) {
    vm.pool.dealloc(this, ssizeof<LString>() + len + 1);
}

void LString::ToString(string &sd, PrintPrefs &pp) {
    if (CycleCheck(sd, pp)) return;
    auto sv = strv();
    auto dd = string_view();
    if (len > pp.budget) {
        sv = sv.substr(0, (size_t)pp.budget);
        dd = "..";
    }
    if (pp.quoted) {
        EscapeAndQuote(sv, sd);
    } else {
        sd += sv;
    }
    sd += dd;
}

LVector::LVector(VM &vm, iint _initial, iint _max, type_elem_t _tti)
    : RefObj(_tti), len(_initial), maxl(_max) {
    auto &sti = vm.GetTypeInfo(ti(vm).subt);
    width = IsStruct(sti.t) ? sti.len : 1;
    v = maxl ? AllocSubBuf<Value>(vm, maxl * width, TYPE_ELEM_VALUEBUF) : nullptr;
}

void LVector::Resize(VM &vm, iint newmax) {
    // FIXME: check overflow
    auto mem = AllocSubBuf<Value>(vm, newmax * width, TYPE_ELEM_VALUEBUF);
    if (len) t_memcpy(mem, v, len * width);
    DeallocBuf(vm);
    maxl = newmax;
    v = mem;
}

void LVector::Append(VM &vm, LVector *from, iint start, iint amount) {
    if (len + amount > maxl) Resize(vm, len + amount);  // FIXME: check overflow
    assert(width == from->width);
    t_memcpy(v + len * width, from->v + start * width, amount * width);
    auto et = from->ElemType(vm).t;
    if (IsRefNil(et)) {
        for (int i = 0; i < amount * width; i++) {
            v[len * width + i].LTINCRTNIL();
        }
    }
    len += amount;
}

void LVector::Remove(StackPtr &sp, VM &vm, iint i, iint n, iint decfrom, bool stack_ret) {
    assert(n >= 0 && n <= len && i >= 0 && i <= len - n);
    if (stack_ret) {
        tsnz_memcpy(TopPtr(sp), v + i * width, width);
        PushN(sp,  (int)width);
    }
    auto et = ElemType(vm).t;
    if (IsRefNil(et)) {
        for (iint j = decfrom * width; j < n * width; j++) DecSlot(vm, i * width + j, et);
    }
    t_memmove(v + i * width, v + (i + n) * width, (len - i - n) * width);
    len -= n;
}

void LVector::AtVW(StackPtr &sp, iint i) const {
    auto src = AtSt(i);
    tsnz_memcpy(TopPtr(sp), src, width);
    PushN(sp, (int)width);
}

void LVector::AtVWInc(StackPtr &sp, iint i) const {
    auto src = AtSt(i);
    for (int j = 0; j < width; j++) {
        auto e = src[j];
        e.LTINCRTNIL();
        lobster::Push(sp, e);
    }
}

void LVector::AtVWSub(StackPtr &sp, iint i, int w, int off) const {
    auto src = AtSt(i);
    tsnz_memcpy(TopPtr(sp), src + off, w);
    PushN(sp,  w);
}

void LVector::DeleteSelf(VM &vm) {
    auto et = ElemType(vm).t;
    if (IsRefNil(et)) {
        for (iint i = 0; i < len * width; i++) DecSlot(vm, i, et);
    }
    DeallocBuf(vm);
    vm.pool.dealloc_small(this);
}

void LObject::DeleteSelf(VM &vm) {
    auto len = Len(vm);
    for (iint i = 0; i < len; i++) {
        AtS(i).LTDECTYPE(vm, ElemTypeS(vm, i).t);
    }
    vm.pool.dealloc(this, ssizeof<LObject>() + ssizeof<Value>() * len);
}

void LResource::DeleteSelf(VM &vm) {
    type->deletefun(val);
    vm.pool.dealloc(this, sizeof(LResource));
}

void RefObj::DECDELETENOW(VM &vm) {
    switch (ti(vm).t) {
        case V_STRING:     ((LString *)this)->DeleteSelf(vm); break;
        case V_COROUTINE:  ((LCoRoutine *)this)->DeleteSelf(vm); break;
        case V_VECTOR:     ((LVector *)this)->DeleteSelf(vm); break;
        case V_CLASS:      ((LObject *)this)->DeleteSelf(vm); break;
        case V_RESOURCE:   ((LResource *)this)->DeleteSelf(vm); break;
        default:           assert(false);
    }
}

void RefObj::DECDELETE(VM &vm) {
    if (refc) {
        //vm.DumpVal(this, "double delete");
        vm.SeriousError("double delete");
        assert(false);
    }
    #if DELETE_DELAY
        vm.DumpVal(this, "delay delete");
        vm.delete_delay.push_back(this);
    #else
        DECDELETENOW(vm);
    #endif
}

void RefObj::DECSTAT(VM &vm) { vm.vm_count_decref++; }

bool RefEqual(VM &vm, const RefObj *a, const RefObj *b, bool structural) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->tti != b->tti) return false;
    switch (a->ti(vm).t) {
        case V_STRING:      return *((LString *)a) == *((LString *)b);
        case V_COROUTINE:   return false;
        case V_VECTOR:      return structural && ((LVector *)a)->Equal(vm, *(LVector *)b);
        case V_CLASS:       return structural && ((LObject *)a)->Equal(vm, *(LObject *)b);
        default:            assert(0); return false;
    }
}

bool Value::Equal(VM &vm, ValueType vtype, const Value &o, ValueType otype, bool structural) const {
    if (vtype != otype) return false;
    switch (vtype) {
        case V_INT: return ival_ == o.ival_;
        case V_FLOAT: return fval_ == o.fval_;
        case V_FUNCTION: return ip_ == o.ip_;
        default: return RefEqual(vm, refnil(), o.ref_, structural);
    }
}

void RefToString(VM &vm, string &sd, const RefObj *ro, PrintPrefs &pp) {
    if (!ro) { sd += "nil"; return; }
    auto &roti = ro->ti(vm);
    switch (roti.t) {
        case V_STRING:    ((LString *)ro)->ToString(sd, pp);          break;
        case V_COROUTINE: sd += "(coroutine)";                        break;
        case V_VECTOR:    ((LVector *)ro)->ToString(vm, sd, pp);      break;
        case V_CLASS:     ((LObject *)ro)->ToString(vm, sd, pp);      break;
        case V_RESOURCE:  ((LResource *)ro)->ToString(sd);            break;
        default:          append(sd, "(", BaseTypeName(roti.t), ")"); break;
    }
}

void Value::ToString(VM &vm, string &sd, const TypeInfo &ti, PrintPrefs &pp) const {
    if (ti.t == V_INT && ti.enumidx >= 0) {
        auto name = vm.EnumName(ival(), ti.enumidx);
        if (!name.empty()) {
            sd += name;
            return;
        }
    }
    ToStringBase(vm, sd, ti.t, pp);
}

void Value::ToStringBase(VM &vm, string &sd, ValueType t, PrintPrefs &pp) const {
    if (IsRefNil(t)) {
        RefToString(vm, sd, ref_, pp);
    } else switch (t) {
        case V_INT:
            append(sd, ival());
            break;
        case V_FLOAT:
            sd += to_string_float(fval(), (int)pp.decimals);
            break;
        case V_FUNCTION:
            sd += "<FUNCTION>";
            break;
        default:
            append(sd, "(", BaseTypeName(t), ")");
            break;
    }
}


iint RefObj::Hash(VM &vm) {
    switch (ti(vm).t) {
        case V_STRING:      return ((LString *)this)->Hash();
        case V_VECTOR:      return ((LVector *)this)->Hash(vm);
        case V_CLASS:       return ((LObject *)this)->Hash(vm);
        default:            return (iint)this;
    }
}

iint LString::Hash() {
    return FNV1A64(strv());
}

iint Value::Hash(VM &vm, ValueType vtype) {
    switch (vtype) {
        case V_INT: return ival_;
        case V_FLOAT: return ReadMem<iint>(&fval_);
        case V_FUNCTION: return ival_;
        default: return refnil() ? ref()->Hash(vm) : 0;
    }
}

Value Value::Copy(VM &vm, StackPtr &sp) {
    if (!refnil()) return Value();
    auto &ti = ref()->ti(vm);
    switch (ti.t) {
    case V_VECTOR: {
        auto len = vval()->len;
        auto nv = vm.NewVec(len, len, vval()->tti);
        if (len) nv->Init(vm, vval()->Elems(), true);
        return Value(nv);
    }
    case V_CLASS: {
        auto len = oval()->Len(vm);
        auto nv = vm.NewObject(len, oval()->tti);
        if (len) nv->Init(vm, oval()->Elems(), len, true);
        return Value(nv);
    }
    case V_STRING: {
        auto s = vm.NewString(sval()->strv());
        return Value(s);
    }
    case V_COROUTINE:
        vm.Error(sp, "cannot copy coroutine");
        return Value();
    default:
        assert(false);
        return Value();
    }
}

string TypeInfo::Debug(VM &vm, bool rec) const {
    string s;
    s += BaseTypeName(t);
    if (t == V_VECTOR || t == V_NIL) {
        s += "[" + vm.GetTypeInfo(subt).Debug(vm, false) + "]";
    } else if (IsUDT(t)) {
        auto sname = vm.StructName(*this);
        s += ":" + sname;
        if (rec) {
            s += "{";
            for (int i = 0; i < len; i++)
                s += vm.GetTypeInfo(elemtypes[i]).Debug(vm, false) + ",";
            s += "}";
        }
    }
    return s;
}

#define ELEMTYPE(acc, ass) \
    auto &_ti = ti(vm); \
    ass; \
    auto &sti = vm.GetTypeInfo(_ti.acc); \
    return sti.t == V_NIL ? vm.GetTypeInfo(sti.subt) : sti;

const TypeInfo &LObject::ElemTypeS(VM &vm, iint i) const {
    ELEMTYPE(elemtypes[i], assert(i < _ti.len));
}

const TypeInfo &LObject::ElemTypeSP(VM &vm, iint i) const {
    ELEMTYPE(GetElemOrParent(i), assert(i < _ti.len));
}

const TypeInfo &LVector::ElemType(VM &vm) const {
    ELEMTYPE(subt, {})
}

void VectorOrObjectToString(VM &vm, string &sd, PrintPrefs &pp, char openb, char closeb,
                            iint len, iint width, const Value *elems, bool is_vector,
                            std::function<const TypeInfo &(iint)> getti) {
    sd += openb;
    if (pp.indent) sd += '\n';
    auto start_size = sd.size();
    pp.cur_indent += pp.indent;
    auto Indent = [&]() {
        for (int i = 0; i < pp.cur_indent; i++) sd += ' ';
    };
    for (iint i = 0; i < len; i++) {
        if (i) {
            sd += ',';
            sd += (pp.indent ? '\n' : ' ');
        }
        if (pp.indent) Indent();
        if (iint(sd.size() - start_size) > pp.budget) {
            sd += "....";
            break;
        }
        auto &ti = getti(i);
        if (pp.depth || !IsRef(ti.t)) {
            PrintPrefs subpp(pp.depth - 1, pp.budget - iint(sd.size() - start_size), true,
                             pp.decimals);
            subpp.indent = pp.indent;
            subpp.cur_indent = pp.cur_indent;
            if (IsStruct(ti.t)) {
                vm.StructToString(sd, subpp, ti, elems + i * width);
                if (!is_vector) i += ti.len - 1;
            } else {
                elems[i].ToString(vm, sd, ti, subpp);
            }
        } else {
            sd += "..";
        }
    }
    pp.cur_indent -= pp.indent;
    if (pp.indent) { sd += '\n'; Indent(); }
    sd += closeb;
}

void LObject::ToString(VM &vm, string &sd, PrintPrefs &pp) {
    if (CycleCheck(sd, pp)) return;
    sd += vm.ReverseLookupType(ti(vm).structidx);
    if (pp.indent) sd += ' ';
    VectorOrObjectToString(vm, sd, pp, '{', '}', Len(vm), 1, Elems(), false,
        [&](iint i) -> const TypeInfo & {
            return ElemTypeSP(vm, i);
        }
    );
}

void LVector::ToString(VM &vm, string &sd, PrintPrefs &pp) {
    if (CycleCheck(sd, pp)) return;
    VectorOrObjectToString(vm, sd, pp, '[', ']', len, width, v, true,
        [&](iint) -> const TypeInfo & {
            return ElemType(vm);
        }
    );
}

void VM::StructToString(string &sd, PrintPrefs &pp, const TypeInfo &ti, const Value *elems) {
    sd += ReverseLookupType(ti.structidx);
    if (pp.indent) sd += ' ';
    VectorOrObjectToString(*this, sd, pp, '{', '}', ti.len, 1, elems, false,
        [&](iint i) -> const TypeInfo & {
            return GetTypeInfo(ti.GetElemOrParent(i));
        }
    );
}

}  // namespace lobster
