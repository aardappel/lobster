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

using namespace lobster;

void RefObj::DECDELETE()
{
    assert(refc == 0);
    switch (rtype)
    {
        case V_STRING:     ((LString *)this)->deleteself(); break;
        case V_COROUTINE:  ((CoRoutine *)this)->deleteself(true); break;
        case V_BOXEDINT:   vmpool->dealloc(this, sizeof(BoxedInt)); break;
        case V_BOXEDFLOAT: vmpool->dealloc(this, sizeof(BoxedFloat)); break;
        default:           assert(rtype >= 0);
                           // FALL-THRU:
        case V_VECTOR:     ((LVector *)this)->deleteself(); break;
    }
}

bool RefObj::Equal(const RefObj *o, bool structural) const
{
    if (this == o)
        return true;

    if (!this || !o)
        return false;

    if (rtype != o->rtype)
        return false;

    switch (rtype)
    {
        case V_BOXEDINT:    return ((BoxedInt *)this)->val == ((BoxedInt *)o)->val;
        case V_BOXEDFLOAT:  return ((BoxedFloat *)this)->val == ((BoxedFloat *)o)->val;

        case V_STRING:      return *((LString *)this) == *((LString *)o);
        default: assert(rtype >= 0);
                 if (rtype < 0) return false;
                 // FALL-THRU:
        case V_VECTOR:      return structural && ((LVector *)this)->Equal(*(LVector *)o);
        case V_COROUTINE:   return false;
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
        default: return ref()->Equal(o.ref_, structural);
    }
}

string RefObj::ToString(PrintPrefs &pp) const
{
    if (!this) return "nil";

    switch (rtype)
    {
        case V_BOXEDINT:   { auto s = to_string(((BoxedInt *)this)->val);                      return pp.anymark ? "#" + s : s; }
        case V_BOXEDFLOAT: { auto s = to_string_float(((BoxedFloat *)this)->val, pp.decimals); return pp.anymark ? "#" + s : s; }

        case V_STRING:     return ((LString *)this)->ToString(pp);
        case V_VECTOR:     return ((LVector *)this)->ToString(pp);
        case V_COROUTINE:  return "(coroutine)";

        default:           return rtype < 0 ? string("(") + BaseTypeName(Type()) + ")"
                                            : ((LVector *)this)->ToString(pp);
    }
}

string Value::ToString(ValueType vtype, PrintPrefs &pp) const
{
    switch (vtype)
    {
        case V_INT:        return to_string(ival_);                   
        case V_FLOAT:      return to_string_float(fval_, pp.decimals);

        case V_STRING:     return sval_->ToString(pp);
        case V_VECTOR:     return vval_->ToString(pp);
        case V_COROUTINE:  return "(coroutine)";

        case V_NIL:        return "nil";
        case V_FUNCTION:   return "<FUNCTION>";
        case V_UNDEFINED:  return "<UNDEFINED>";
        default:           return IsRef(vtype) ? ref_->ToString(pp)
                                               : string("(") + BaseTypeName(vtype) + ")";
    }
}


void RefObj::Mark()
{
    if (refc < 0) return;
    assert(refc);
    refc = -refc;
    switch (rtype)
    {
        default: if (rtype < 0) break;
                 // FALL-THRU:
        case V_VECTOR:     ((LVector   *)this)->Mark(); break;
        case V_COROUTINE:  ((CoRoutine *)this)->Mark(); break;
    }
}

void Value::Mark(ValueType vtype)
{
    if (IsRef(vtype)) ref_->Mark();
}
