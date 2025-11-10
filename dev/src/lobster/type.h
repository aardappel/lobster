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

#ifndef LOBSTER_TYPE
#define LOBSTER_TYPE

namespace lobster {

// This enum used to be used at both compile time and runtime, but is now only compiletime.
// There is now a RTType equivalent, to which these values get mapped during codegen.
enum ValueType : int {
    // refc types are negative
    V_MINVTTYPES = -8,
    V_ANY = -7,         // any other reference type.
    V_VALUEBUF = -6,    // only used as memory type for vector/coro buffers, not used by Value.
    V_STRUCT_R = -5,
    V_RESOURCE = -4,
    V_STRING = -3,
    V_CLASS = -2,
    V_VECTOR = -1,
    V_NIL = 0,          // Nillable type of the above.
    V_INT,
    V_FLOAT,
    V_FUNCTION,
    V_STRUCT_S,
    V_STRUCT_NUM,       // [typechecker only] like V_STRUCT_S but an unknown set of ints or floats, used by builtins.
    V_VAR,              // [typechecker only] like V_ANY, except idx refers to a type variable
    V_TYPEVAR,          // [typechecker only] refers to an explicit type variable in code, e.g. "T".
    V_TYPEID,           // [typechecker only] a typetable offset. Present at runtime as V_INT.
    V_VOID,             // [typechecker/codegen only] this exp does not produce a value.
    V_TUPLE,            // [typechecker/codegen only] this exp produces >1 value.
    V_UUDT,             // [parser/typechecker only] udt with unresolved generics.
    V_UNDEFINED,        // [typechecker only] this type should never be accessed.
    V_MAXVTTYPES
};

inline bool IsScalar(ValueType t) { return t == V_INT || t == V_FLOAT; }
inline bool IsUnBoxed(ValueType t) { return t == V_INT || t == V_FLOAT || t == V_FUNCTION; }
inline bool IsRef(ValueType t) { return t <  V_NIL; }
inline bool IsRefNil(ValueType t) { return t <= V_NIL; }
inline bool IsRefNilVar(ValueType t) { return t <= V_NIL || t == V_VAR; }
inline bool IsRefNilNoStruct(ValueType t) { return t <= V_NIL && t != V_STRUCT_R; }
inline bool IsStruct(ValueType t) { return t == V_STRUCT_R || t == V_STRUCT_S; }
inline bool IsUDT(ValueType t) { return t == V_CLASS || IsStruct(t); }
inline bool IsUnBoxedOrStruct(ValueType t) { return IsUnBoxed(t) || IsStruct(t); }
inline bool IsRuntime(ValueType t) { return t < V_STRUCT_NUM; }
inline bool IsRuntimeConcrete(ValueType t) { return t < V_STRUCT_NUM && t >= V_STRUCT_R; }

// This would be mostly called during codegen.
// For now a direct mapping, later something more complicated with asserts etc.
inline RTType VT2RT(ValueType t) {
    switch (t) {
        case V_STRUCT_NUM:
            return RTT_STRUCT_S;
        case V_TYPEID:
            return RTT_INT;
        case V_VAR:
        case V_UNDEFINED:
            // This happens when converting compiler values that are never accessed.
            // Would be better to assert here and remove those cases, but for now
            // emit a type the runtime can't do anything with.
            return RTT_INVALID;
        default:
            assert(IsRuntime(t));
            return (RTType)t;
    }
}

// The compile time equivalent to Value, used in ConstVal etc.
union VTValue {
    int64_t i;
    double f;

    VTValue() : i(0) {}
    VTValue(int64_t i) : i(i) {}
    VTValue(double f) : f(f) {}
    VTValue(bool b) : i(b) {}

    void ToString(string &sd, ValueType t) const {
        switch (t) {
            default:
                append(sd, i);
                break;
            case V_FLOAT:
                sd += to_string_float(f);
                break;
        }
    }
};

}  // namespace lobster

#endif  // LOBSTER_TYPE
