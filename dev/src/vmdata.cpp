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

void Value::DECDELETE() const
{
    assert(ref_->refc == 0);
    switch (type)
    {
        case V_VECTOR:     vval_->deleteself();     break;
        case V_STRING:     sval_->deleteself();     break;
        case V_COROUTINE:  cval_->deleteself(true); break;
        case V_BOXEDINT:   vmpool->dealloc(bival_, sizeof(BoxedInt));   break;
        case V_BOXEDFLOAT: vmpool->dealloc(bfval_, sizeof(BoxedFloat)); break;
        default:           assert(0);
    }
}

bool Value::Equal(const Value &o, bool structural) const
{
    // This does not allow bival == ival, which probably makes sense.
    if (type != o.type)
        return false;

    switch (type)
    {
        case V_INT:         return ival_ == o.ival_;
        case V_FLOAT:       return fval_ == o.fval_;

        case V_BOXEDINT:    return bival_->val == o.bival_->val;
        case V_BOXEDFLOAT:  return bfval_->val == o.bfval_->val;

        case V_STRING:      return (*sval_) == (*o.sval_);
        case V_VECTOR:      return vval_ == o.vval_ || (structural && vval_->Equal(*o.vval_));
        case V_COROUTINE:   return cval_ == o.cval_;

        case V_NIL:         return true;
        case V_FUNCTION:    return ip_ == o.ip_;
        case V_UNDEFINED:   return true;
        default: assert(0); return false;
    }
}

string Value::ToString(PrintPrefs &pp) const
{
    switch (type)
    {
        case V_INT:        return to_string(ival_);
        case V_FLOAT:      return to_string_float(fval_, pp.decimals);

        case V_BOXEDINT:   return "#" + to_string(bival_->val);
        case V_BOXEDFLOAT: return "#" + to_string_float(bfval_->val, pp.decimals);

        case V_STRING:     return sval_->ToString(pp);
        case V_VECTOR:     return vval_->ToString(pp);
        case V_COROUTINE:  return "(coroutine)";

        case V_NIL:        return "nil";
        case V_FUNCTION:   return "<FUNCTION>";
        case V_UNDEFINED:  return "<UNDEFINED>";
        default:           return string("(") + BaseTypeName(type) + ")";
    }
}

void Value::Mark()
{
    switch (type)
    {
        case V_VECTOR:     vval_->Mark(); break;
        case V_COROUTINE:  cval_->Mark(); break;
        case V_STRING:
        case V_BOXEDINT:
        case V_BOXEDFLOAT: ref_->Mark();  break;
        default:                          break;
    }
}

