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
    if (ti.t == V_VECTOR) return ((LVector *)this)->len;
    assert(ti.t == V_STRUCT);
    return ti.len;
}

Value &ElemObj::At(int i) const
{
    if (ti.t == V_VECTOR) return ((LVector *)this)->At(i);
    assert(ti.t == V_STRUCT);
    return ((LStruct *)this)->At(i);
};

void RefObj::DECDELETE(bool deref)
{
    assert(refc == 0);
    switch (ti.t)
    {
        case V_BOXEDINT:   vmpool->dealloc(this, sizeof(BoxedInt)); break;
        case V_BOXEDFLOAT: vmpool->dealloc(this, sizeof(BoxedFloat)); break;
        case V_STRING:     ((LString *)this)->DeleteSelf(); break;
        case V_COROUTINE:  ((CoRoutine *)this)->DeleteSelf(deref); break;
        case V_VECTOR:     ((LVector *)this)->DeleteSelf(deref); break;
        case V_STRUCT:     ((LStruct *)this)->DeleteSelf(deref); break;
        default:           assert(false);
    }
}

bool RefObj::Equal(const RefObj *o, bool structural) const
{
    if (this == o)
        return true;

    if (!this || !o)
        return false;

    if (&ti != &o->ti)
        return false;

    switch (ti.t)
    {
        case V_BOXEDINT:    return ((BoxedInt *)this)->val == ((BoxedInt *)o)->val;
        case V_BOXEDFLOAT:  return ((BoxedFloat *)this)->val == ((BoxedFloat *)o)->val;
        case V_STRING:      return *((LString *)this) == *((LString *)o);
        case V_COROUTINE:   return false;
        case V_VECTOR:
        case V_STRUCT:      return structural && ((ElemObj *)this)->Equal(*(ElemObj *)o);
        default:            assert(0); return false;
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

    switch (ti.t)
    {
        case V_BOXEDINT:   { auto s = to_string(((BoxedInt *)this)->val);                      return pp.anymark ? "#" + s : s; }
        case V_BOXEDFLOAT: { auto s = to_string_float(((BoxedFloat *)this)->val, pp.decimals); return pp.anymark ? "#" + s : s; }

        case V_STRING:     return ((LString *)this)->ToString(pp);
        case V_COROUTINE:  return "(coroutine)";
        case V_VECTOR:
        case V_STRUCT:     return ((ElemObj *)this)->ToString(pp);
        default:           return string("(") + BaseTypeName(ti.t) + ")";
    }
}

string Value::ToString(ValueType vtype, PrintPrefs &pp) const
{
    if (IsRefNil(vtype)) return ref_ ? ref_->ToString(pp) : "nil";

    switch (vtype)
    {
        case V_INT:        return to_string(ival());                   
        case V_FLOAT:      return to_string_float(fval(), pp.decimals);
        case V_FUNCTION:   return "<FUNCTION>";
        default:           return string("(") + BaseTypeName(vtype) + ")";
    }
}


void RefObj::Mark()
{
    if (refc < 0) return;
    assert(refc);
    refc = -refc;
    switch (ti.t)
    {
        case V_STRUCT:
        case V_VECTOR:     ((ElemObj   *)this)->Mark(); break;
        case V_COROUTINE:  ((CoRoutine *)this)->Mark(); break;
    }
}

void Value::Mark(ValueType vtype)
{
    if (IsRefNil(vtype) && ref_) ref_->Mark();
}

string TypeInfo::Debug(bool rec) const
{
    string s = BaseTypeName(t);
    if (t == V_VECTOR || t == V_NIL)
    {
        s += "[" + g_vm->GetTypeInfo(subt).Debug(false) + "]";
    }
    else if (t == V_STRUCT)
    {
        auto sname = g_vm->StructName(*this);
        s += ":" + sname;
        if (rec)
        {
            s += "{";
            for (int i = 0; i < len; i++)
                s += g_vm->GetTypeInfo(elems[i]).Debug(false) + ",";
            s += "}";
        }
    }
    return s;
}

}  // namespace lobster
