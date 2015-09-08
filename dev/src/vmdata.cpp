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

#include "stdafx.h"

#include "vmdata.h"

namespace lobster {

int ElemObj::Len() const
{
    auto &ti = GetTypeInfo();
    if (ti.t == V_VECTOR) return ((LVector *)this)->len;
    assert(ti.t == V_STRUCT);
    return g_vm->StructLen(typeoff);
}

Value &ElemObj::At(int i) const
{
    auto &ti = GetTypeInfo();
    if (ti.t == V_VECTOR) return ((LVector *)this)->At(i);
    assert(ti.t == V_STRUCT);
    return ((LStruct *)this)->At(i);
};

void RefObj::DECDELETE(bool deref)
{
    assert(refc == 0);
    switch (typeoff)
    {
        case TYPE_ELEM_BOXEDINT:   vmpool->dealloc(this, sizeof(BoxedInt)); break;
        case TYPE_ELEM_BOXEDFLOAT: vmpool->dealloc(this, sizeof(BoxedFloat)); break;
        case TYPE_ELEM_STRING:     ((LString *)this)->DeleteSelf(); break;
        case TYPE_ELEM_COROUTINE:  ((CoRoutine *)this)->DeleteSelf(deref); break;
        default:
        {
            auto &ti = GetTypeInfo();
            if (ti.t == V_VECTOR) return ((LVector *)this)->DeleteSelf(deref);
            assert(ti.t == V_STRUCT);
            return ((LStruct *)this)->DeleteSelf(deref);
        }
    }
}

bool RefObj::Equal(const RefObj *o, bool structural) const
{
    if (this == o)
        return true;

    if (!this || !o)
        return false;

    if (typeoff != o->typeoff)
        return false;

    switch (typeoff)
    {
        case TYPE_ELEM_BOXEDINT:    return ((BoxedInt *)this)->val == ((BoxedInt *)o)->val;
        case TYPE_ELEM_BOXEDFLOAT:  return ((BoxedFloat *)this)->val == ((BoxedFloat *)o)->val;
        case TYPE_ELEM_STRING:      return *((LString *)this) == *((LString *)o);
        case TYPE_ELEM_COROUTINE:   return false;
        default:
        {
            assert(IsVector(BaseType()));
            return structural && ((ElemObj *)this)->Equal(*(ElemObj *)o);
        }
    }
}

bool Value::Equal(ValueType vtype, const Value &o, ValueType otype, bool structural) const
{
    if (vtype != otype) return false;
    switch (vtype)
    {
        case V_INT: return ival_ == o.ival_;
        case V_FLOAT: return fval_ == o.fval_;
        case V_FUNCTION: return ip_ == o.ip_;
        default: return refnil()->Equal(o.ref_, structural);
    }
}

string RefObj::ToString(PrintPrefs &pp) const
{
    if (!this) return "nil";

    switch (typeoff)
    {
        case TYPE_ELEM_BOXEDINT:   { auto s = to_string(((BoxedInt *)this)->val);                      return pp.anymark ? "#" + s : s; }
        case TYPE_ELEM_BOXEDFLOAT: { auto s = to_string_float(((BoxedFloat *)this)->val, pp.decimals); return pp.anymark ? "#" + s : s; }

        case TYPE_ELEM_STRING:     return ((LString *)this)->ToString(pp);
        case TYPE_ELEM_COROUTINE:  return "(coroutine)";
        default:
        {
            auto &ti = GetTypeInfo();
            if (IsVector((ValueType)ti.t)) return ((ElemObj *)this)->ToString(pp);
            return string("(") + BaseTypeName(BaseType()) + ")";
        }
    }
}

string Value::ToString(ValueType vtype, PrintPrefs &pp) const
{
    if (IsRefNil(vtype)) return ref_ ? ref_->ToString(pp) : "nil";

    switch (vtype)
    {
        case V_INT:        return to_string(ival_);                   
        case V_FLOAT:      return to_string_float(fval_, pp.decimals);
        case V_FUNCTION:   return "<FUNCTION>";
        default:           return string("(") + BaseTypeName(vtype) + ")";
    }
}


void RefObj::Mark()
{
    if (refc < 0) return;
    assert(refc);
    refc = -refc;
    auto &ti = GetTypeInfo();
    switch (ti.t)
    {
        case V_STRUCT:
        case V_VECTOR:     ((ElemObj   *)this)->Mark(); break;
        case V_COROUTINE:  ((CoRoutine *)this)->Mark(); break;
    }
}

void Value::Mark(ValueType vtype)
{
    if (IsRef(vtype) && ref_) ref_->Mark();
}

string TypeInfoDebug(type_elem_t offset, bool rec)
{
    auto &ti = g_vm->GetTypeInfo(offset);
    string s = BaseTypeName(ti.t);
    if (ti.t == V_VECTOR || ti.t == V_NIL)
    {
        s += "[" + TypeInfoDebug(ti.subt, false) + "]";
    }
    else if (ti.t == V_STRUCT)
    {
        auto nargs = g_vm->StructLen(offset);
        auto sname = g_vm->StructName(offset);
        s += ":" + sname;
        if (rec)
        {
            s += "{";
            for (int i = 0; i < nargs; i++)
                s += TypeInfoDebug(ti.elems[i], false) + ",";
            s += "}";
        }
    }
    return s;
}

}  // namespace lobster
