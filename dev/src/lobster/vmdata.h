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

#ifndef LOBSTER_VMDATA
#define LOBSTER_VMDATA

#include "il.h"

namespace bytecode { struct BytecodeFile; }  // FIXME

namespace lobster {

#ifndef NDEBUG
#define RTT_ENABLED 1
#define RTT_TYPE_ERRORS 1
#else
#define RTT_ENABLED 0
#define RTT_TYPE_ERRORS 0
#endif

#define STRING_CONSTANTS_KEEP 0

#define DELETE_DELAY 0

enum ValueType : int {
    // refc types are negative
    V_MINVMTYPES = -9,
    V_ANY = -8,         // any other reference type.
    V_STACKFRAMEBUF = -7,
    V_VALUEBUF = -6,    // only used as memory type for vector/coro buffers, not used by Value.
    V_STRUCT_R = -5,
    V_RESOURCE = -4,
    V_STRING = -3,
    V_CLASS = -2,
    V_VECTOR = -1,
    V_NIL = 0,          // VM: null reference, Type checker: nillable.
    V_INT,
    V_FLOAT,
    V_FUNCTION,
    V_STRUCT_S,
    V_VAR,              // [typechecker only] like V_ANY, except idx refers to a type variable
    V_TYPEVAR,          // [typechecker only] refers to an explicit type variable in code, e.g. "T".
    V_TYPEID,           // [typechecker only] a typetable offset.
    V_VOID,             // [typechecker/codegen only] this exp does not produce a value.
    V_TUPLE,            // [typechecker/codegen only] this exp produces >1 value.
    V_UUDT,             // [parser/typechecker only] udt with unresolved generics.
    V_UNDEFINED,        // [typechecker only] this type should never be accessed.
    V_MAXVMTYPES
};

inline bool IsScalar(ValueType t) { return t == V_INT || t == V_FLOAT; }
inline bool IsUnBoxed(ValueType t) { return t == V_INT || t == V_FLOAT || t == V_FUNCTION; }
inline bool IsRef(ValueType t) { return t <  V_NIL; }
inline bool IsRefNil(ValueType t) { return t <= V_NIL; }
inline bool IsRefNilVar(ValueType t) { return t <= V_NIL || t == V_VAR; }
inline bool IsRefNilStruct(ValueType t) { return t <= V_NIL || t == V_STRUCT_S; }
inline bool IsRefNilNoStruct(ValueType t) { return t <= V_NIL && t != V_STRUCT_R; }
inline bool IsRuntime(ValueType t) { return t < V_VAR; }
inline bool IsStruct(ValueType t) { return t == V_STRUCT_R || t == V_STRUCT_S; }
inline bool IsUDT(ValueType t) { return t == V_CLASS || IsStruct(t); }

inline string_view BaseTypeName(ValueType t) {
    static const char *typenames[] = {
        "any", "<stackframe_buffer>", "<value_buffer>",
        "struct_ref",
        "resource", "string", "class", "vector",
        "nil", "int", "float", "function", "struct_scalar",
        "unknown", "type_variable", "typeid", "void",
        "tuple", "unresolved_udt", "undefined",
    };
    if (t <= V_MINVMTYPES || t >= V_MAXVMTYPES) {
        assert(false);
        return "<internal-error-type>";
    }
    return typenames[t - V_MINVMTYPES - 1];
}

enum type_elem_t : int {  // Strongly typed element of typetable.
    // These must correspond to typetable init in Codegen constructor.
    TYPE_ELEM_INT = 0,  // This has -1 for its enumidx.
    TYPE_ELEM_FLOAT = 2,
    TYPE_ELEM_STRING = 3,
    TYPE_ELEM_RESOURCE = 4,
    TYPE_ELEM_ANY = 5,
    TYPE_ELEM_VALUEBUF = 6,
    TYPE_ELEM_STACKFRAMEBUF = 7,
    TYPE_ELEM_VECTOR_OF_INT = 8,   // 2 each.
    TYPE_ELEM_VECTOR_OF_FLOAT = 10,
    TYPE_ELEM_VECTOR_OF_STRING = 12,
    TYPE_ELEM_VECTOR_OF_VECTOR_OF_INT = 14,
    TYPE_ELEM_VECTOR_OF_VECTOR_OF_FLOAT = 16,

    TYPE_ELEM_FIXED_OFFSET_END = 18
};

struct VM;

struct TypeInfo {
    ValueType t;
    union {
        type_elem_t subt;  // V_VECTOR | V_NIL
        struct {           // V_CLASS, V_STRUCT_*
            int structidx;
            int len;
            int vtable_start;
            type_elem_t elemtypes[1];  // len elems, followed by len parent types.
        };
        int enumidx;       // V_INT, -1 if not an enum.
        int sfidx;         // V_FUNCTION;
    };

    TypeInfo() = delete;
    TypeInfo(const TypeInfo &) = delete;
    TypeInfo &operator=(const TypeInfo &) = delete;

    string Debug(VM &vm, bool rec = true) const;

    type_elem_t GetElemOrParent(iint i) const {
        auto pti = elemtypes[len + i];
        return pti >= 0 ? pti : elemtypes[i];
    }
};

struct Value;
struct LString;
struct LVector;
struct LObject;

struct PrintPrefs {
    iint depth;
    iint budget;
    bool quoted;
    iint decimals;
    int cycles = -1;
    int indent = 0;
    int cur_indent = 0;

    PrintPrefs(iint _depth, iint _budget, bool _quoted, iint _decimals)
        : depth(_depth), budget(_budget), quoted(_quoted), decimals(_decimals) {}
};

// ANY memory allocated by the VM must inherit from this, so we can identify leaked memory
struct DynAlloc {
    type_elem_t tti;  // offset into the VM's typetable
    const TypeInfo &ti(VM &vm) const;

    DynAlloc(type_elem_t _tti) : tti(_tti) {}
};

struct RefObj : DynAlloc {
    int refc = 1;

    #if DELETE_DELAY
    const int *alloc_ip;
    #endif

    RefObj(type_elem_t _tti) : DynAlloc(_tti)
        #if DELETE_DELAY
            , alloc_ip(nullptr)
        #endif
    {}

    void Inc() {
        #ifndef NDEBUG
            if (refc <= 0) {  // Should never be "re-vived".
                #if DELETE_DELAY
                    LOG_DEBUG("revive: ", (size_t)this, " - ", refc);
                #endif
                assert(false);
            }
        #endif
        refc++;
        #if DELETE_DELAY
            LOG_DEBUG("inc: ", (size_t)this, " - ", refc);
        #endif
    }

    void Dec(VM &vm) {
        refc--;
        #ifndef NDEBUG
            DECSTAT(vm);
        #endif
        #if DELETE_DELAY
            LOG_DEBUG("dec: ", (size_t)this, " - ", refc);
        #endif
        if (refc <= 0) {
            DECDELETE(vm);
        }
    }

    void CycleStr(string &sd) const { append(sd, "_", -refc, "_"); }

    bool CycleCheck(string &sd, PrintPrefs &pp) {
        if (pp.cycles >= 0) {
            if (refc < 0) { CycleStr(sd); return true; }
            refc = -(++pp.cycles);
        }
        return false;
    }

    void DECDELETE(VM &vm);
    void DECDELETENOW(VM &vm);
    void DECSTAT(VM &vm);

    iint Hash(VM &vm);
};

extern bool RefEqual(VM &vm, const RefObj *a, const RefObj *b, bool structural);
extern void RefToString(VM &vm, string &sd, const RefObj *ro, PrintPrefs &pp);

struct LString : RefObj {
    iint len;    // has to match the Value integer type, since we allow the length to be obtained
    LString(iint _l);

    const char *data() const { return (char *)(this + 1); }
    string_view strv() const { return string_view(data(), (size_t)len); }

    void ToString(string &sd, PrintPrefs &pp);

    void DeleteSelf(VM &vm);

    bool operator==(LString &o) { return strv() == o.strv(); }
    bool operator!=(LString &o) { return strv() != o.strv(); }
    bool operator< (LString &o) { return strv() <  o.strv(); }
    bool operator<=(LString &o) { return strv() <= o.strv(); }
    bool operator> (LString &o) { return strv() >  o.strv(); }
    bool operator>=(LString &o) { return strv() >= o.strv(); }

    iint Hash();
};

// There must be a single of these per type, since they are compared by pointer.
struct ResourceType {
    const char *name;
    void (* deletefun)(void *);
};

struct LResource : RefObj {
    void *val;
    const ResourceType *type;

    LResource(void *v, const ResourceType *t);

    void DeleteSelf(VM &vm);

    void ToString(string &sd) {
        append(sd, "(resource:", type->name, ")");
    }
};

#if RTT_ENABLED
    #define TYPE_INIT(t) ,type(t)
    #define TYPE_ASSERT(c) assert(c)
#else
    #define TYPE_INIT(t)
    #define TYPE_ASSERT(c)
#endif

typedef Value *StackPtr;

typedef StackPtr(*block_base_t)(VM &, StackPtr);
#ifdef VM_COMPILED_CODE_MODE
    extern "C" StackPtr compiled_entry_point(VM & vm, StackPtr sp);
    #ifdef VM_JIT_MODE
        extern "C" const void *vm_ops_jit_table[];
    #endif
#endif

// These pointer types are for use inside Value below. In most other parts of the code we
// use naked pointers.
#if _WIN64 || __amd64__ || __x86_64__ || __ppc64__ || __LP64__
    // We use regular pointers.
    typedef LString *LStringPtr;
    typedef LVector *LVectorPtr;
    typedef LObject *LObjectPtr;
    typedef LResource *LResourcePtr;
    typedef RefObj *RefObjPtr;
    typedef TypeInfo *TypeInfoPtr;
    typedef block_base_t BlockPtr;
#else
    // We use this special pointer type to represent a 32-bit pointer inside a
    // 64-bit value.
    // This is necessary because we want all values to be exactly the same size,
    // to be able to test for 0, etc, so we can't have unused bits in the union.
    template<typename T> class ExpandedPtr {
        uint64_t c;
      public:
        ExpandedPtr(const T p) : c((uint64_t)p) {}
        T get() const { return (T)c; }
        operator T () const { return (T)c; }
        T operator->() const { return (T)c; }
    };
    typedef ExpandedPtr<LString *> LStringPtr;
    typedef ExpandedPtr<LVector *> LVectorPtr;
    typedef ExpandedPtr<LObject *> LObjectPtr;
    typedef ExpandedPtr<LResource *> LResourcePtr;
    typedef ExpandedPtr<RefObj *> RefObjPtr;
    typedef ExpandedPtr<TypeInfo *> TypeInfoPtr;
    typedef ExpandedPtr<block_base_t> BlockPtr;
#endif

static_assert(sizeof(iint) == sizeof(double) && sizeof(iint) == sizeof(RefObjPtr),
              "typedefs need fixing");

#ifdef VM_COMPILED_CODE_MODE
    typedef BlockPtr block_t;
#else
    typedef iint block_t;
#endif

struct InsPtr {
    block_t f;
    InsPtr() : f(0) {}
    explicit InsPtr(block_t _f) : f(_f) {}
    bool operator==(const InsPtr o) const { return f == o.f; }
    bool operator!=(const InsPtr o) const { return f != o.f; }
};

struct Value {
    private:
    union {
        // All these types must all be exactly 64-bits, even in 32-bit builds.

        // Non-reference values.
        iint ival_;      // scalars stored as pointer-sized versions.
        double fval_;
        InsPtr ip_;

        // Reference values (includes NULL if nillable version).
        LStringPtr sval_;
        LVectorPtr vval_;
        LObjectPtr oval_;
        LResourcePtr xval_;

        // Generic reference access.
        RefObjPtr ref_;

        // Temp: for inline structs.
        TypeInfoPtr ti_;
    };
    public:
    #if RTT_ENABLED
        // This one comes second, since that allows e.g. the Wasm codegen to access the above
        // data without knowing if we're in debug mode.
        ValueType type;
    #endif

    // These asserts help track down any invalid code generation issues.
    iint        ival  () const { TYPE_ASSERT(type == V_INT);        return ival_;        }
    double      fval  () const { TYPE_ASSERT(type == V_FLOAT);      return fval_;        }
    int         intval() const { TYPE_ASSERT(type == V_INT);        return (int)ival_;   }
    float       fltval() const { TYPE_ASSERT(type == V_FLOAT);      return (float)fval_; }
    LString    *sval  () const { TYPE_ASSERT(type == V_STRING);     return sval_;        }
    LVector    *vval  () const { TYPE_ASSERT(type == V_VECTOR);     return vval_;        }
    LObject    *oval  () const { TYPE_ASSERT(type == V_CLASS);      return oval_;        }
    LResource  *xval  () const { TYPE_ASSERT(type == V_RESOURCE);   return xval_;        }
    RefObj     *ref   () const { TYPE_ASSERT(IsRef(type));          return ref_;         }
    RefObj     *refnil() const { TYPE_ASSERT(IsRefNil(type));       return ref_;         }
    InsPtr      ip    () const { TYPE_ASSERT(type >= V_FUNCTION);   return ip_;          }
    void       *any   () const {                                    return ref_;         }
    TypeInfo   *tival () const { TYPE_ASSERT(type == V_STRUCT_S);   return ti_;          }

    template<typename T> T ifval() const {
        if constexpr (is_floating_point<T>()) { TYPE_ASSERT(type == V_FLOAT); return (T)fval_; }
        else                                  { TYPE_ASSERT(type == V_INT);   return (T)ival_; }
    }

    void setival(iint i)   { TYPE_ASSERT(type == V_INT);   ival_ = i; }
    void setfval(double f) { TYPE_ASSERT(type == V_FLOAT); fval_ = f; }

    inline Value()                     : ref_(nullptr)    TYPE_INIT(V_NIL)      {}
    // We underlying types here, because types like int64_t etc can be defined as different types
    // on different platforms, causing ambiguities between multiple types that are long or long long
    inline Value(int i)                : ival_((iint)i)   TYPE_INIT(V_INT)      {}
    inline Value(unsigned int i)       : ival_((iint)i)   TYPE_INIT(V_INT)      {}
    inline Value(long i)               : ival_((iint)i)   TYPE_INIT(V_INT)      {}
    inline Value(unsigned long i)      : ival_((iint)i)   TYPE_INIT(V_INT)      {}
    inline Value(long long i)          : ival_((iint)i)   TYPE_INIT(V_INT)      {}
    inline Value(unsigned long long i) : ival_((iint)i)   TYPE_INIT(V_INT)      {}
    inline Value(int i, ValueType t)   : ival_(i)         TYPE_INIT(t)          { (void)t; }
    inline Value(bool b)               : ival_(b)         TYPE_INIT(V_INT)      {}
    inline Value(float f)              : fval_(f)         TYPE_INIT(V_FLOAT)    {}
    inline Value(double f)             : fval_((double)f) TYPE_INIT(V_FLOAT)    {}
    inline Value(InsPtr i)             : ip_(i)           TYPE_INIT(V_FUNCTION) {}

    inline Value(LString *s)         : sval_(s)         TYPE_INIT(V_STRING)     {}
    inline Value(LVector *v)         : vval_(v)         TYPE_INIT(V_VECTOR)     {}
    inline Value(LObject *s)         : oval_(s)         TYPE_INIT(V_CLASS)      {}
    inline Value(LResource *r)       : xval_(r)         TYPE_INIT(V_RESOURCE)   {}
    inline Value(RefObj *r)          : ref_(r)          TYPE_INIT(V_NIL)        { assert(false); }

    inline Value(TypeInfo *ti)       : ti_(ti)          TYPE_INIT(V_STRUCT_S)   {}

    inline bool True() const { return ival_ != 0; }
    inline bool False() const { return ival_ == 0; }

    inline Value &LTINCRT() {
        TYPE_ASSERT(IsRef(type) && ref_);
        ref_->Inc();
        return *this;
    }
    inline Value &LTINCRTNIL() {
        // Can't assert IsRefNil here, since scalar 0 are valid NIL values due to e.g. and/or.
        if (ref_) LTINCRT();
        return *this;
    }
    inline Value &LTINCTYPE(ValueType t) {
        return IsRefNil(t) ? LTINCRTNIL() : *this;
    }

    inline void LTDECRT(VM &vm) const {  // we already know its a ref type
        TYPE_ASSERT(IsRef(type) && ref_);
        ref_->Dec(vm);
    }
    inline void LTDECRTNIL(VM &vm) const {
        // Can't assert IsRefNil here, since scalar 0 are valid NIL values due to e.g. and/or.
        if (ref_) LTDECRT(vm);
    }
    inline void LTDECTYPE(VM &vm, ValueType t) const {
        if (IsRefNil(t)) LTDECRTNIL(vm);
    }

    void ToString(VM &vm, string &sd, const TypeInfo &ti, PrintPrefs &pp) const;
    void ToStringBase(VM &vm, string &sd, ValueType t, PrintPrefs &pp) const;

    bool Equal(VM &vm, ValueType vtype, const Value &o, ValueType otype, bool structural) const;
    iint Hash(VM &vm, ValueType vtype);
    Value Copy(VM &vm);  // Shallow.
};

template<typename T> inline T *AllocSubBuf(VM &vm, iint size, type_elem_t tti);
template<typename T> inline void DeallocSubBuf(VM &vm, T *v, iint size);

struct LObject : RefObj {
    LObject(type_elem_t _tti) : RefObj(_tti) {}

    // FIXME: reduce the use of these.
    iint Len(VM &vm) const { return ti(vm).len; }

    Value *Elems() const { return (Value *)(this + 1); }

    // This may only be called from a context where i < len has already been ensured/asserted.
    Value &AtS(iint i) const {
        return Elems()[i];
    }

    void DeleteSelf(VM &vm);

    // This may only be called from a context where i < len has already been ensured/asserted.
    const TypeInfo &ElemTypeS(VM &vm, iint i) const;
    const TypeInfo &ElemTypeSP(VM &vm, iint i) const;

    void ToString(VM &vm, string &sd, PrintPrefs &pp);

    bool Equal(VM &vm, const LObject &o) {
        // RefObj::Equal has already guaranteed the typeoff's are the same.
        auto len = Len(vm);
        assert(len == o.Len(vm));
        for (iint i = 0; i < len; i++) {
            auto et = ElemTypeS(vm, i).t;
            if (!AtS(i).Equal(vm, et, o.AtS(i), et, true))
                return false;
        }
        return true;
    }

    iint Hash(VM &vm) {
        iint hash = 0;
        for (int i = 0; i < Len(vm); i++) hash ^= AtS(i).Hash(vm, ElemTypeS(vm, i).t);
        return hash;
    }

    void Init(VM &vm, Value *from, iint len, bool inc) {
        assert(len && len == Len(vm));
        t_memcpy(Elems(), from, len);
        if (inc) for (iint i = 0; i < len; i++) {
            AtS(i).LTINCTYPE(ElemTypeS(vm, i).t);
        }
    }
};

struct LVector : RefObj {
    iint len;    // has to match the Value integer type, since we allow the length to be obtained
    iint maxl;
    iint width;  // TODO: would be great to not have to store this.

    private:
    Value *v;   // use At()

    public:
    LVector(VM &vm, iint _initial, iint _max, type_elem_t _tti);

    ~LVector() { assert(0); }   // destructed by DECREF

    ssize_t SLen() { return (ssize_t)len; }

    void DeallocBuf(VM &vm) {
        if (v) DeallocSubBuf(vm, v, maxl * width);
    }

    void DecSlot(VM &vm, iint i, ValueType et) const {
        AtSlot(i).LTDECTYPE(vm, et);
    }

    void DeleteSelf(VM &vm);

    const TypeInfo &ElemType(VM &vm) const;

    void Resize(VM &vm, iint newmax);

    void Push(VM &vm, const Value &val) {
        assert(width == 1);
        if (len == maxl) Resize(vm, maxl ? maxl * 2 : 4);
        v[len++] = val;
    }

    void PushVW(VM &vm, const Value *vals) {
        if (len == maxl) Resize(vm, maxl ? maxl * 2 : 4);
        tsnz_memcpy(v + len * width, vals, width);
        len++;
    }

    Value Pop() {
        assert(width == 1);
        return v[--len];
    }

    void PopVW(Value *dest) {
        len--;
        tsnz_memcpy(dest, v + len * width, width);
    }

    Value &Top() const {
        assert(width == 1);
        return v[len - 1];
    }

    void TopVW(Value *dest) {
        tsnz_memcpy(dest, v + (len - 1) * width, width);
    }

    void Insert(VM &vm, const Value *vals, iint i) {
        assert(i >= 0 && i <= len); // note: insertion right at the end is legal, hence <=
        if (len + 1 > maxl) Resize(vm, max(len + 1, maxl ? maxl * 2 : 4));
        t_memmove(v + (i + 1) * width, v + i * width, (len - i) * width);
        len++;
        tsnz_memcpy(v + i * width, vals, width);
    }

    void Remove(StackPtr &sp, VM &vm, iint i, iint n, iint decfrom, bool stack_ret);

    Value *Elems() { return v; }

    Value &At(iint i) const {
        assert(i < len && width == 1);
        return v[i];
    }

    Value &AtSub(iint i, int off) const {
        assert(i < len);
        return v[i * width + off];
    }

    Value *AtSt(iint i) const {
        assert(i < len);
        return v + i * width;
    }

    Value &AtSlot(iint i) const {
        assert(i < len * width);
        return v[i];
    }

    void AtVW(StackPtr &sp, iint i) const;
    void AtVWInc(StackPtr &sp, iint i) const;
    void AtVWSub(StackPtr &sp, iint i, int w, int off) const;

    void Append(VM &vm, LVector *from, iint start, iint amount);

    void ToString(VM &vm, string &sd, PrintPrefs &pp);

    bool Equal(VM &vm, const LVector &o) {
        // RefObj::Equal has already guaranteed the typeoff's are the same.
        assert(width == 1);
        if (len != o.len) return false;
        auto et = ElemType(vm).t;
        for (iint i = 0; i < len; i++) {
            if (!At(i).Equal(vm, et, o.At(i), et, true))
                return false;
        }
        return true;
    }

    iint Hash(VM &vm) {
        iint hash = 0;
        assert(width == 1);
        auto et = ElemType(vm).t;
        for (int i = 0; i < len; i++) hash ^= At(i).Hash(vm, et);
        return hash;
    }

    void Init(VM &vm, Value *from, bool inc) {
        assert(len);
        t_memcpy(v, from, len * width);
        auto et = ElemType(vm).t;
        if (inc && IsRefNil(et)) {
            for (iint i = 0; i < len; i++) {
                At(i).LTINCRTNIL();
            }
        }
    }
};

struct StackFrame {
    #ifndef VM_COMPILED_CODE_MODE
        InsPtr retip;
    #endif
    const int *funstart;
    iint spstart;
};

struct NativeFun;
struct NativeRegistry;

// This contains all data shared between threads.
struct TupleSpace {
    struct TupleType {
        // We have an independent list of tuples and synchronization per type, for minimum
        // contention.
        list<Value *> tuples;
        mutex mtx;
        condition_variable condition;
    };
    vector<TupleType> tupletypes;

    atomic<bool> alive;

    TupleSpace(size_t numstructs) : tupletypes(numstructs), alive(true) {}

    ~TupleSpace() {
        for (auto &tt : tupletypes) for (auto p : tt.tuples) delete[] p;
    }
};

enum class TraceMode { OFF, ON, TAIL };

struct VMArgs {
    NativeRegistry &nfr;
    string_view programname;
    string bytecode_buffer;
    const void *static_bytecode = nullptr;
    size_t static_size = 0;
    vector<string> program_args;
    const block_base_t *native_vtables = nullptr;
    block_base_t jit_entry = nullptr;
    TraceMode trace = TraceMode::OFF;
};

struct VM : VMArgs {
    SlabAlloc pool;

    Value *stack = nullptr;
    int stacksize = 0;
    int maxstacksize;
    StackPtr sp_suspended = nullptr;
    StackPtr savedrets = nullptr;

    #ifdef VM_COMPILED_CODE_MODE
        block_base_t next_call_target = 0;
    #else
        const int *ip = nullptr;
    #endif
    int ret_unwind_to = -1;
    int ret_nrv = -1;

    vector<StackFrame> stackframes;

    size_t codelen = 0;
    const int *codestart = nullptr;
    vector<int> codebigendian;
    vector<type_elem_t> typetablebigendian;
    uint64_t *byteprofilecounts = nullptr;

    const bytecode::BytecodeFile *bcf;

    PrintPrefs programprintprefs { 10, 100000, false, -1 };
    const type_elem_t *typetable = nullptr;
    string evalret;

    int currentline = -1;
    iint maxsp = -1;

    PrintPrefs debugpp { 2, 50, true, -1 };

    string s_reuse;

    vector<string> trace_output;
    size_t trace_ring_idx = 0;

    vector<RefObj *> delete_delay;

    vector<LString *> constant_strings;

    vector<InsPtr> vtables;

    int64_t vm_count_ins = 0;
    int64_t vm_count_fcalls = 0;
    int64_t vm_count_bcalls = 0;
    int64_t vm_count_decref = 0;

    #if defined(VM_COMPILED_CODE_MODE) && defined(NDEBUG)
        // Inlining the base VM ops allows for a great deal of optimisation,
        // collapsing a lot of code.
        #ifdef _WIN32
            #define VM_INLINE __forceinline
        #else
            #define VM_INLINE inline __attribute__((always_inline))
        #endif
    #else
        // Inlining things causes a code explosion in debug, so use static instead.
        #define VM_INLINE static inline
    #endif

    typedef StackPtr (* f_ins_pointer)(VM &, StackPtr);
    f_ins_pointer f_ins_pointers[IL_MAX_OPS];

    bool is_worker = false;
    vector<thread> workers;
    TupleSpace *tuple_space = nullptr;

    // A runtime error triggers code that does extensive stack trace & variable dumping, which
    // for certain errors could trigger yet more errors. This vars ensures that we don't.
    bool error_has_occured = false;  // Don't error again.

    // We stick this in here directly, since the constant offsets into this array in
    // compiled mode a big win.
    Value vars[1];

    // NOTE: NO MORE VAR DECLS AFTER "vars"

    VM(VMArgs &&args, const bytecode::BytecodeFile *bcf);
    ~VM();

    void SuspendSP(StackPtr &sp) {
        assert(!sp_suspended);
        sp_suspended = sp;
        sp = nullptr;
    }

    StackPtr ResumeSP() {
        assert(sp_suspended);
        auto sp = sp_suspended;
        sp_suspended = nullptr;
        return sp;
    }

    const TypeInfo &GetTypeInfo(type_elem_t offset) {
        return *(TypeInfo *)(typetable + offset);
    }
    const TypeInfo &GetVarTypeInfo(int varidx);

    void SetMaxStack(int ms) { maxstacksize = ms; }
    string_view GetProgramName() { return programname; }

    type_elem_t GetIntVectorType(int which);
    type_elem_t GetFloatVectorType(int which);

    void DumpVal(RefObj *ro, const char *prefix);
    void DumpFileLine(const int *fip, string &sd);
    void DumpLeaks();

    string &TraceStream();

    void OnAlloc(RefObj *ro);
    LVector *NewVec(iint initial, iint max, type_elem_t tti);
    LObject *NewObject(iint max, type_elem_t tti);
    LResource *NewResource(void *v, const ResourceType *t);
    LString *NewString(iint l);
    LString *NewString(string_view s);
    LString *NewString(string_view s1, string_view s2);
    LString *ResizeString(LString *s, iint size, int c, bool back);

    Value Error(StackPtr sp, string err);
    Value BuiltinError(StackPtr sp, string err) { return Error(sp, err); }
    Value SeriousError(string err);
    void ErrorBase(string &sd, const string &err);
    void VMAssert(const char *what);

    int DumpVar(string &sd, const Value &x, int idx);

    void FinalStackVarsCleanup(StackPtr &sp);

    void StartWorkers(StackPtr &sp, iint numthreads);
    void TerminateWorkers();
    void WorkerWrite(StackPtr &sp, RefObj *ref);
    LObject *WorkerRead(StackPtr &sp, type_elem_t tti);

    #ifdef VM_COMPILED_CODE_MODE
        #define VM_COMMA ,
        #define VM_OP_ARGS const int *ip
        #define VM_OP_ARGS_CALL block_base_t fcont
        #define VM_IP_PASS_THRU ip
        #define VM_FC_PASS_THRU fcont
    #else
        #define VM_COMMA
        #define VM_OP_ARGS
        #define VM_OP_ARGS_CALL
        #define VM_IP_PASS_THRU
        #define VM_FC_PASS_THRU
    #endif

    void JumpTo(InsPtr j);
    InsPtr GetIP();
    template<int is_error> int VarCleanup(StackPtr &sp, string *error, int towhere);
    void StartStackFrame(InsPtr retip);
    void FunIntroPre(StackPtr &sp, InsPtr fun);
    void FunIntro(StackPtr &sp VM_COMMA VM_OP_ARGS);
    void FunOut(StackPtr &sp, int nrv);

    void EndEval(StackPtr &sp, const Value &ret, const TypeInfo &ti);

    void EvalProgram(StackPtr = nullptr);
    void EvalProgramInner(StackPtr);

    void LvalueIdxVector(int lvalop, iint i);
    void LvalueIdxStruct(int lvalop, iint i);
    void LvalueField(int lvalop, iint i);
    void LvalueOp(int op, Value &a);

    string ProperTypeName(const TypeInfo &ti);

    void Div0(StackPtr sp) { Error(sp, "division by zero"); }
    void IDXErr(StackPtr sp, iint i, iint n, const RefObj *v);
    void BCallProf();
    void BCallRetCheck(StackPtr sp, const NativeFun *nf);
    iint GrabIndex(StackPtr &sp, int len);

    string_view StructName(const TypeInfo &ti);
    string_view ReverseLookupType(int v);
    void Trace(TraceMode m) { trace = m; }
    double Time() { return SecondsSinceStart(); }

    Value ToString(const Value &a, const TypeInfo &ti) {
        s_reuse.clear();
        a.ToString(*this, s_reuse, ti, programprintprefs);
        return NewString(s_reuse);
    }
    Value StructToString(const Value *elems, const TypeInfo &ti) {
        s_reuse.clear();
        StructToString(s_reuse, programprintprefs, ti, elems);
        return NewString(s_reuse);
    }
    void StructToString(string &sd, PrintPrefs &pp, const TypeInfo &ti, const Value *elems);

    bool EnumName(string &sd, iint val, int enumidx);
    string_view EnumName(int enumidx);
    optional<int64_t> LookupEnum(string_view name, int enumidx);
};

// This is like a smart-pointer for VM above that dynamically allocates the size of "vars".
struct VMAllocator {
    VM *vm = nullptr;
    VMAllocator(VMArgs &&args);
    ~VMAllocator();
};

VM_INLINE void Push(StackPtr &sp, Value v) { *++sp = v; }
VM_INLINE Value Pop(StackPtr &sp) { return *sp--; }
VM_INLINE Value &Top(StackPtr sp) { return *sp; }
VM_INLINE Value &TopM(StackPtr sp, iint n) { return *(sp - n); }
VM_INLINE Value *TopPtr(StackPtr sp) { return sp + 1; }
VM_INLINE void PushN(StackPtr &sp, iint n) { sp += n; }
VM_INLINE void PopN(StackPtr &sp, iint n) { sp -= n; }

VM_INLINE pair<Value *, iint> PopVecPtr(StackPtr &sp) {
    auto width = Pop(sp).ival();
    PopN(sp, width);
    return { TopPtr(sp), width };
}

template<typename T, int N> void PushVec(StackPtr &sp, const vec<T, N> &v, int truncate = 4) {
    auto l = min(N, truncate);
    for (int i = 0; i < l; i++) Push(sp, v[i]);
}
template<typename T> T PopVec(StackPtr &sp, typename T::CTYPE def = 0) {
    T v;
    auto l = Pop(sp).intval();
    if (l > T::NUM_ELEMENTS) PopN(sp, l - T::NUM_ELEMENTS);
    for (int i = T::NUM_ELEMENTS - 1; i >= 0; i--) {
        v[i] = i < l ? Pop(sp).ifval<typename T::CTYPE>() : def;
    }
    return v;
}
template<typename T> void PushAnyAsString(StackPtr &sp, VM &vm, const T &t) {
    Push(sp, vm.NewString(string_view((char *)&t, sizeof(T))));
}

template<typename T> void PopAnyFromString(StackPtr &sp, T &t) {
    auto s = Pop(sp);
    assert(s.type == V_STRING);
    assert(s.sval()->len == sizeof(T));
    t = *(T *)s.sval()->strv().data();
    // No rc dec, these are stored in a keepvar.
}

inline int64_t Int64FromInts(int a, int b) {
    int64_t v = (uint32_t)a;
    v |= ((int64_t)b) << 32;
    return v;
}

inline const TypeInfo &DynAlloc::ti(VM &vm) const { return vm.GetTypeInfo(tti); }

template<typename T> inline T *AllocSubBuf(VM &vm, iint size, type_elem_t tti) {
    auto header_sz = max(salignof<T>(), ssizeof<DynAlloc>());
    auto mem = (uint8_t *)vm.pool.alloc(size * ssizeof<T>() + header_sz);
    ((DynAlloc *)mem)->tti = tti;
    mem += header_sz;
    return (T *)mem;
}

template<typename T> inline void DeallocSubBuf(VM &vm, T *v, iint size) {
    auto header_sz = max(salignof<T>(), ssizeof<DynAlloc>());
    auto mem = ((uint8_t *)v) - header_sz;
    vm.pool.dealloc(mem, size * ssizeof<T>() + header_sz);
}

template<bool back> LString *WriteMem(VM &vm, LString *s, iint i, const void *data, iint size) {
    auto minsize = i + size;
    if (s->len < minsize) s = vm.ResizeString(s, minsize * 2, 0, back);
    memcpy((void *)(s->data() + (back ? s->len - i - size : i)), data, (size_t)size);
    return s;
}

template<typename T, bool back> LString *WriteValLE(VM &vm, LString *s, iint i, T val) {
    T t = flatbuffers::EndianScalar(val);
    return WriteMem<back>(vm, s, i, &t, ssizeof<T>());
}

template<typename T, bool back> T ReadValLE(const LString *s, iint i) {
    T val;
    memcpy(&val, (void *)(s->data() + (back ? s->len - i - sizeof(T) : i)), sizeof(T));
    return flatbuffers::EndianScalar(val);
}


// FIXME: turn check for len into an assert and make caller guarantee lengths match.
template<int N> inline vec<double, N> ValueToF(const Value *v, iint width, double def = 0) {
    vec<double, N> t;
    for (int i = 0; i < N; i++) t[i] = width > i ? (v + i)->fval() : def;
    return t;
}
template<int N> inline vec<iint, N> ValueToI(const Value *v, iint width, iint def = 0) {
    vec<iint, N> t;
    for (int i = 0; i < N; i++) t[i] = width > i ? (v + i)->ival() : def;
    return t;
}
template<int N> inline vec<float, N> ValueToFLT(const Value *v, iint width, float def = 0) {
    vec<float, N> t;
    for (int i = 0; i < N; i++) t[i] = width > i ? (v + i)->fltval() : def;
    return t;
}
template<int N> inline vec<int, N> ValueToINT(const Value *v, iint width, int def = 0) {
    vec<int, N> t;
    for (int i = 0; i < N; i++) t[i] = width > i ? (v + i)->intval() : def;
    return t;
}

template <typename T, int N> inline void ToValue(Value *dest, iint width, const vec<T, N> &v) {
    for (iint i = 0; i < width; i++) dest[i] = i < N ? v[i] : 0;
}

inline iint RangeCheck(StackPtr sp, VM &vm, const Value &idx, iint range, iint bias = 0) {
    auto i = idx.ival();
    if (i < bias || i >= bias + range)
        vm.BuiltinError(sp, cat("index out of range [", bias, "..", bias + range, "): ", i));
    return i;
}

template<typename T> inline T GetResourceDec(StackPtr sp, VM &vm, const Value &val, const ResourceType *type) {
    if (val.False())
        return nullptr;
    auto x = val.xval();
    if (x->type != type)
        vm.BuiltinError(sp, string_view("needed resource type: ") + type->name + ", got: " +
            x->type->name);
    return (T)x->val;
}

inline vector<string> ValueToVectorOfStrings(Value &v) {
    vector<string> r;
    for (int i = 0; i < v.vval()->len; i++) r.push_back(string(v.vval()->At(i).sval()->strv()));
    return r;
}

inline Value ToValueOfVectorOfStrings(VM &vm, const vector<string> &in) {
    auto v = vm.NewVec(0, ssize(in), TYPE_ELEM_VECTOR_OF_STRING);
    for (auto &a : in) v->Push(vm, vm.NewString(a));
    return Value(v);
}

inline Value ToValueOfVectorOfStringsEmpty(VM &vm, const int2 &size, char init) {
    auto v = vm.NewVec(0, size.y, TYPE_ELEM_VECTOR_OF_STRING);
    for (int i = 0; i < size.y; i++) {
        auto s = vm.NewString(size.x);
        memset((char *)s->data(), init, size.x);
        v->Push(vm, s);
    }
    return Value(v);
}

void EscapeAndQuote(string_view s, string &sd, bool strip_comments);

#if !defined(NDEBUG) && RTT_ENABLED
    #define STRINGIFY(x) #x
    #define TOSTRING(x) STRINGIFY(x)
    #define VMASSERT(vm, test) { if (!(test)) vm.VMAssert(__FILE__ ": " TOSTRING(__LINE__) ": " #test); }
#else
    #define VMASSERT(vm, test) { (void)vm; }
#endif

#define RANGECHECK(vm, I, BOUND, VEC) \
    if ((uint64_t)I >= (uint64_t)BOUND) vm.IDXErr(sp, I, BOUND, VEC);

}  // namespace lobster

#endif  // LOBSTER_VMDATA
