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

// Compile time lifetime tracking between values and their recipients.
// If lifetimes correspond, no action is required.
// If recipient wants to keep, but value is borrowed, inc ref or copy or error.
// If recipient wants to borrow, but value is keep, dec ref or delete after recipient is done.
// NOTE: all positive values are an index of the SpecIdent being borrowed.
// If you're borrowing, you are "locking" the modification of the variable you borrow from.
enum Lifetime {
    // Value: you are receiving a value stored elsewhere, do not hold on.
    // Recipient: I do not want to be responsible for managing this value.
    LT_BORROW = -1,
    // Value: you are responsible for this value, you must delete or store.
    // Recipient: I want to hold on to this value (inc ref, or be sole owner).
    LT_KEEP = -2,
    // Value: lifetime shouldn't matter, because type is non-reference.
    // Recipient: I'm cool with any lifetime.
    LT_ANY = -3,
    // Value: there are multiple lifetimes, stored elsewhere.
    LT_MULTIPLE = -4,
    // Lifetime is not valid.
    LT_UNDEF = -5,
};

inline bool IsBorrow(Lifetime lt) { return lt >= LT_BORROW; }
inline Lifetime LifetimeType(Lifetime lt) { return IsBorrow(lt) ? LT_BORROW : lt; }

struct SubFunction;

struct Enum;

struct GUDT;
struct UDT;

struct TypeVariable;

struct Type;

struct SpecUDT {
    GUDT *gudt;
    vector<const Type *> specializers;
    //bool is_generic = false;

    SpecUDT(GUDT *gudt) : gudt(gudt) {}

    bool IsGeneric() const;
    bool Equal(const SpecUDT &o) const;
};

struct NumStruct {
    ValueType t = V_INT;        // Must be V_INT or V_FLOAT.
    int flen = -1;              // Fixed len, or -1 for unknown.
};

struct Type {
    const ValueType t = V_UNDEFINED;

    struct TupleElem { const Type *type; Lifetime lt; };

    union {
        const Type *sub;         // V_VECTOR | V_NIL | V_VAR | V_TYPEID
        SubFunction *sf;         // V_FUNCTION
        UDT *udt;                // V_CLASS | V_STRUCT_*
        NumStruct *ns;           // V_STRUCT_NUM
        Enum *e;                 // V_INT
        vector<TupleElem> *tup;  // V_TUPLE
        ResourceType *rt;        // V_RESOURCE

        // These only if in an UnType.
        SpecUDT *spec_udt;       // V_UUDT
        TypeVariable *tv;        // V_TYPEVAR
    };

    Type()                               :                  sub(nullptr) {}
    explicit Type(ValueType _t)          : t(_t),           sub(nullptr) {}
    Type(ValueType _t, const Type *_s)   : t(_t),           sub(_s)      {}
    Type(ValueType _t, SubFunction *_sf) : t(_t),           sf(_sf)      {}
    Type(NumStruct *_ns)                 : t(V_STRUCT_NUM), ns(_ns)      {}
    Type(ValueType _t, UDT *_udt)        : t(_t),           udt(_udt)    {}
    Type(Enum *_e)                       : t(V_INT),        e(_e)        {}
    Type(ResourceType *_rt)              : t(V_RESOURCE),   rt(_rt)      {}

    protected:
    Type(SpecUDT *_su)                   : t(V_UUDT),       spec_udt(_su){}
    Type(TypeVariable *_tv)              : t(V_TYPEVAR),    tv(_tv)      {}
    public:


    bool Equal(const Type &o, bool allow_unresolved = false) const;

    Type &operator=(const Type &o) {
        // Hack: we want t to be const, but still have a working assignment operator.
        (ValueType &)t = o.t;
        sub = o.sub;
        return *this;
    }

    const Type *Element() const {
        assert(Wrapped());
        return sub;
    }

    const Type *ElementIfNil() const {
        return t == V_NIL ? sub : this;
    }

    Type *Wrap(Type *dest, ValueType with) const {
        assert(dest != this);
        *dest = Type(with, this);
        return dest;
    }

    bool Wrapped() const { return t == V_VECTOR || t == V_NIL; }

    const Type *UnWrapped() const { return Wrapped() ? sub : this; }
    const Type *UnWrapAll() const { return Wrapped() ? sub->UnWrapped() : this; }

    bool Numeric() const { return t == V_INT || t == V_FLOAT; }

    bool IsFunction() const { return t == V_FUNCTION && sf; }

    bool IsEnum() const { return t == V_INT && e; }

    bool IsBoundVar() const { return t == V_VAR && sub; }

    bool HasValueType(ValueType vt) const {
        return t == vt || (Wrapped() && Element()->HasValueType(vt));
    }

    bool IsConcrete() const {
        return IsRuntimeConcrete(t) || (Wrapped() && Element()->IsConcrete());
    }

    bool FlowSensitive() const;

    size_t NumValues() const {
        if (t == V_VOID) return 0;
        if (t == V_TUPLE) return tup->size();
        return 1;
    }

    const Type *Get(size_t i) const {
        return t == V_TUPLE ? (*tup)[i].type : this;
    }

    void Set(size_t i, const Type *type, Lifetime lt) const {
        assert(t == V_TUPLE);
        (*tup)[i] = { type, lt };
    }

    Lifetime GetLifetime(size_t i, Lifetime lt) const {
        return lt == LT_MULTIPLE && t == V_TUPLE ? (*tup)[i].lt : lt;
    }
};

extern const Type g_type_undefined;

struct UnType : Type {
    UnType()                  : Type()    {}
    UnType(SpecUDT *_su)      : Type(_su) {}
    UnType(TypeVariable *_tv) : Type(_tv) {}

    const Type *Resolved() const {
        return t == V_UUDT || t == V_TYPEVAR ? &g_type_undefined : (Type *)this;
    }
};

// This one is allowed to have V_UUDT and V_TYPEVAR in it.
class UnTypeRef {
    protected:
    const Type *type;

    public:
    UnTypeRef()                    : type(&g_type_undefined) {}
    UnTypeRef(const UnType *_type) : type(_type) {}
    UnTypeRef(const Type *_type)   : type(_type) {}

    UnTypeRef &operator=(const UnTypeRef &o) {
        type = o.type;
        return *this;
    }

    const UnType &operator*()  const { return *(const UnType *)type; }
    const UnType *operator->() const { return (const UnType *)type; }

    const UnType *get() const { return (UnType *)type; }

    bool Null() const { return type == nullptr; }
};

// This is essentially a smart-pointer, but behaves a little bit differently:
// - initialized to type_undefined instead of nullptr
// - pointer is const
// - comparisons are by value.
class TypeRef : public UnTypeRef {

    public:
    TypeRef()                  : UnTypeRef() {}
    TypeRef(const Type *_type) : UnTypeRef(_type) {}

    TypeRef &operator=(const TypeRef &o) {
        type = o.type;
        return *this;
    }

    const Type &operator*()  const { return *type; }
    const Type *operator->() const { return type; }

    const Type *get() const { return type; }

    bool Null() const { return type == nullptr; }
};

extern TypeRef type_int;
extern TypeRef type_float;
extern TypeRef type_string;
extern TypeRef type_any;
extern TypeRef type_vector_int;
extern TypeRef type_vector_float;
extern TypeRef type_function_null_void;
extern TypeRef type_function_cocl;
extern TypeRef type_resource;
extern TypeRef type_vector_resource;
extern TypeRef type_typeid;
extern TypeRef type_void;
extern TypeRef type_undefined;

TypeRef WrapKnown(UnTypeRef elem, ValueType with);
TypeRef FixedNumStruct(ValueType num, int flen);

// There must be a single of these per type, since they are compared by pointer.
struct ResourceType {
    string_view name;
    ResourceType *next;
    const Type thistype;
    const Type thistypenil;
    const Type thistypevec;

    ResourceType(string_view n)
        : name(n), next(nullptr), thistype(this),
          thistypenil(V_NIL, &thistype), thistypevec(V_VECTOR, &thistype) {
        next = g_resource_type_list;
        g_resource_type_list = this;
    }
};

inline ResourceType *LookupResourceType(string_view name) {
    for (auto rt = g_resource_type_list; rt; rt = rt->next) {
        if (rt->name == name) return rt;
    }
    return nullptr;
}

struct Named {
    string name;
    int idx = -1;

    Named() = default;
    Named(string_view _name, int _idx = 0) : name(_name), idx(_idx) {}

    [[noreturn]] void Error(const string &msg) {
        THROW_OR_ABORT(cat("INTERNAL ERROR: ", name, ": ", msg));
    }
};

}  // namespace lobster

#endif  // LOBSTER_TYPE
