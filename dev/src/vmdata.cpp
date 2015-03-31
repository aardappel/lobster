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
    assert(ref->refc == 0);
    switch (type)
    {
        case V_VECTOR:    vval->deleteself();     break;
        case V_STRING:    sval->deleteself();     break;
        case V_COROUTINE: cval->deleteself(true); break;
        default:          assert(0);
    }
}

bool Value::Equal(const Value &o, bool structural) const
{
    if (type != o.type)
        return false;

    switch (type)
    {
        case V_INT:         return ival == o.ival;
        case V_FLOAT:       return fval == o.fval;

        case V_STRING:      return (*sval) == (*o.sval);
        case V_VECTOR:      return vval == o.vval || (structural && vval->Equal(*o.vval));
        case V_COROUTINE:   return cval == o.cval;

        case V_NIL:         return true;
        case V_FUNCTION:    return ip == o.ip;
        case V_UNDEFINED:   return true;
        default: assert(0); return false;
    }
}

string Value::ToString(PrintPrefs &pp) const
{
    switch (type)
    {
        case V_INT:       return inttoa(ival);
        case V_FLOAT:     return flttoa(fval, pp.decimals);

        case V_STRING:    return sval->ToString(pp);
        case V_VECTOR:    return vval->ToString(pp);
        case V_COROUTINE: return "(coroutine)";

        case V_NIL:       return "nil";
        case V_FUNCTION:  return "<FUNCTION>";
        case V_UNDEFINED: return "<UNDEFINED>";
        default:          return string("(") + BaseTypeName(type) + ")";
    }
}

void Value::Mark()
{
    switch (type)
    {
        case V_STRING:    sval->Mark(); break; 
        case V_VECTOR:    vval->Mark(); break;
        case V_COROUTINE: cval->Mark(); break;
        default:          break;
    }
}

