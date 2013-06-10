#include "stdafx.h"

#include "vmdata.h"

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
        case V_UNKNOWN:     return true;
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
        case V_UNKNOWN:   return "<UNDEFINED>";
        default:            return string("<") + inttoa(type) + ">";
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

