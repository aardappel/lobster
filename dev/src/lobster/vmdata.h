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

// These are used with VM_COMPILED_CODE_MODE
#define VM_DISPATCH_TRAMPOLINE 1
#define VM_DISPATCH_SWITCH_GOTO 2
#define VM_DISPATCH_METHOD VM_DISPATCH_TRAMPOLINE

#define STRING_CONSTANTS_KEEP 0

#define DELETE_DELAY 0

// Typedefs to make pointers and scalars the same size.
#if _WIN64 || __amd64__ || __x86_64__ || __ppc64__ || __LP64__
    #if !defined(VM_COMPILED_CODE_MODE) || VM_DISPATCH_METHOD != VM_DISPATCH_TRAMPOLINE
        //#define FORCE_32_BIT_MODEL
    #endif
    #ifndef FORCE_32_BIT_MODEL
        #define VALUE_MODEL_64 1
    #else
        #define VALUE_MODEL_64 0
    #endif
#else
    #define VALUE_MODEL_64 0
#endif

#if VALUE_MODEL_64
    typedef int64_t intp;
    typedef uint64_t uintp;
    typedef double floatp;
#else
    typedef int32_t intp;
    typedef uint32_t uintp;
    typedef float floatp;
#endif

typedef vec<floatp, 2> floatp2;
typedef vec<floatp, 3> floatp3;
typedef vec<floatp, 4> floatp4;

typedef vec<intp, 2> intp2;
typedef vec<intp, 3> intp3;
typedef vec<intp, 4> intp4;

const floatp4 floatp4_0 = floatp4(0.0f);
const floatp3 floatp3_0 = floatp3(0.0f);
const floatp2 floatp2_0 = floatp2(0.0f);

const intp2 intp2_0 = intp2((intp)0);
const intp2 intp2_1 = intp2(1);
const intp3 intp3_0 = intp3((intp)0);


enum ValueType : int {
    // refc types are negative
    V_MINVMTYPES = -9,
    V_ANY = -8,         // any other reference type.
    V_STACKFRAMEBUF = -7,
    V_VALUEBUF = -6,    // only used as memory type for vector/coro buffers, not used by Value.
    V_RESOURCE = -5,
    V_COROUTINE = -4,
    V_STRING = -3,
    V_STRUCT = -2,
    V_VECTOR = -1,
    V_NIL = 0,          // VM: null reference, Type checker: nillable.
    V_INT,
    V_FLOAT,
    V_FUNCTION,
    V_YIELD,
    V_VAR,              // [typechecker only] like V_ANY, except idx refers to a type variable
    V_TYPEID,           // [typechecker only] a typetable offset.
    V_VOID,             // [typechecker/codegen only] this exp does not produce a value. 
    V_TUPLE,            // [typechecker/codegen only] this exp produces >1 value.
    V_UNDEFINED,        // [typechecker only] this type should never be accessed.
    V_MAXVMTYPES
};

inline bool IsScalar(ValueType t) { return t == V_INT || t == V_FLOAT; }
inline bool IsUnBoxed(ValueType t) { return t == V_INT || t == V_FLOAT || t == V_FUNCTION; }
inline bool IsRef(ValueType t) { return t <  V_NIL; }
inline bool IsRefNil(ValueType t) { return t <= V_NIL; }
inline bool IsRefNilVar(ValueType t) { return t <= V_NIL || t == V_VAR; }
inline bool IsRuntime(ValueType t) { return t < V_VAR; }
inline bool IsRuntimePrintable(ValueType t) { return t <= V_FLOAT; }

inline string_view BaseTypeName(ValueType t) {
    static const char *typenames[] = {
        "any", "<value_buffer>", "<stackframe_buffer>",
        "resource", "coroutine", "string", "struct", "vector",
        "nil", "int", "float", "function", "yield_function", "variable", "typeid", "void",
        "tuple", "undefined",
        "<logstart>", "<logend>", "<logmarker>"
    };
    if (t <= V_MINVMTYPES || t >= V_MAXVMTYPES) {
        assert(false);
        return "<internal-error-type>";
    }
    return typenames[t - V_MINVMTYPES - 1];
}

enum type_elem_t : int {  // Strongly typed element of typetable.
    // These must correspond to typetable init in Codegen constructor.
    TYPE_ELEM_INT,
    TYPE_ELEM_FLOAT,
    TYPE_ELEM_STRING,
    TYPE_ELEM_RESOURCE,
    TYPE_ELEM_ANY,
    TYPE_ELEM_VALUEBUF,
    TYPE_ELEM_STACKFRAMEBUF,
    TYPE_ELEM_VECTOR_OF_INT = 7,   // 2 each.
    TYPE_ELEM_VECTOR_OF_FLOAT = 9,
    TYPE_ELEM_VECTOR_OF_STRING = 11,
    TYPE_ELEM_VECTOR_OF_VECTOR_OF_INT = 13,
    TYPE_ELEM_VECTOR_OF_VECTOR_OF_FLOAT = 15,

    TYPE_ELEM_FIXED_OFFSET_END = 17
};

struct VM;

struct TypeInfo {
    ValueType t;
    union {
        type_elem_t subt;  // V_VECTOR | V_NIL
        struct { int structidx; int len; type_elem_t elems[1]; };    // V_STRUCT
        int sfidx;  // V_FUNCTION;
        struct { int cofunidx; type_elem_t yieldtype; };  // V_COROUTINE
    };

    TypeInfo() = delete;
    TypeInfo(const TypeInfo &) = delete;
    TypeInfo &operator=(const TypeInfo &) = delete;

    string Debug(VM &vm, bool rec = true) const;
};

struct Value;
struct LString;
struct LVector;
struct LStruct;
struct LCoRoutine;

struct PrintPrefs {
    intp depth;
    intp budget;
    bool quoted;
    intp decimals;
    int cycles;

    PrintPrefs(intp _depth, intp _budget, bool _quoted, intp _decimals)
        : depth(_depth), budget(_budget), quoted(_quoted), decimals(_decimals), cycles(-1) {}
};

typedef void *(*block_base_t)(VM &);
#if VM_DISPATCH_METHOD == VM_DISPATCH_TRAMPOLINE
    typedef block_base_t block_t;
#elif VM_DISPATCH_METHOD == VM_DISPATCH_SWITCH_GOTO
    typedef intp block_t;
#endif

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
        #ifdef _DEBUG
            if (refc <= 0) {  // Should never be "re-vived".
                #if DELETE_DELAY
                    LOG_DEBUG("revive: ", (size_t)this);
                #endif
                assert(false);
            }
        #endif
        #if DELETE_DELAY
            LOG_DEBUG("inc: ", (size_t)this);
        #endif
        refc++;
    }

    void Dec(VM &vm) {
        refc--;
        #ifdef _DEBUG
            DECSTAT(vm);
        #endif
        #if DELETE_DELAY
            LOG_DEBUG("dec: ", (size_t)this);
        #endif
        if (refc <= 0) {
            DECDELETE(vm);
        }
    }

    void CycleDone(int &cycles) {
        refc = -(++cycles);
    }

    void CycleStr(ostringstream &ss) const { ss << "_" << -refc << "_"; }

    void DECDELETE(VM &vm);
    void DECDELETENOW(VM &vm);
    void DECSTAT(VM &vm);

    intp Hash(VM &vm);
};

extern bool RefEqual(VM &vm, const RefObj *a, const RefObj *b, bool structural);
extern void RefToString(VM &vm, ostringstream &ss, const RefObj *ro, PrintPrefs &pp);

struct LString : RefObj {
    intp len;    // has to match the Value integer type, since we allow the length to be obtained
    LString(intp _l);

    const char *data() const { return (char *)(this + 1); }
    string_view strv() const { return string_view(data(), len); }

    void ToString(ostringstream &ss, PrintPrefs &pp);

    void DeleteSelf(VM &vm);

    bool operator==(LString &o) { return strv() == o.strv(); }
    bool operator!=(LString &o) { return strv() != o.strv(); }
    bool operator< (LString &o) { return strv() <  o.strv(); }
    bool operator<=(LString &o) { return strv() <= o.strv(); }
    bool operator> (LString &o) { return strv() >  o.strv(); }
    bool operator>=(LString &o) { return strv() >= o.strv(); }

    intp Hash();
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

    void ToString(ostringstream &ss) {
        ss << "(resource:" << type->name << ")";
    }
};

struct InsPtr {
    #ifdef VM_COMPILED_CODE_MODE
        block_t f;
        explicit InsPtr(block_t _f) : f(_f) {}
        static_assert(sizeof(block_t) == sizeof(intp), "");
    #else
        intp f;
        explicit InsPtr(intp _f) : f(_f) {}
        #ifdef FORCE_32_BIT_MODEL
            explicit InsPtr(ptrdiff_t _f) : f((intp)_f) {}
        #endif
    #endif
    InsPtr() : f(0) {}
    bool operator==(const InsPtr o) const { return f == o.f; }
};

#if RTT_ENABLED
    #define TYPE_INIT(t) type(t),
#else
    #define TYPE_INIT(t)
#endif

// These pointer types are for use inside Value below. In most other parts of the code we
// use naked pointers.
#ifndef FORCE_32_BIT_MODEL
    // We use regular pointers of the current architecture.
    typedef LString *LStringPtr;
    typedef LVector *LVectorPtr;
    typedef LStruct *LStructPtr;
    typedef LCoRoutine *LCoRoutinePtr;
    typedef LResource *LResourcePtr;
    typedef RefObj *RefObjPtr;
#else
    // We use a compressed pointer to fit in 32-bit on a 64-bit build.
    // These are shifted by COMPRESS_BITS, so for 3 we can address the bottom 32GB of the
    // address space. The memory allocator for these values must guarantee we only allocate
    // from that region, by using mmap or similar.
    template<typename T> class CompressedPtr {
        uint32_t c;
        enum { COMPRESS_BITS = 3, COMPRESS_MASK = (1 << COMPRESS_BITS) - 1 };
      public:
        CompressedPtr(const T *p) {
            auto bits = (size_t)p;
            assert(!(bits & COMPRESS_MASK));  // Must not have low bits set.
            bits >>= COMPRESS_BITS;
            assert(!(bits >> 32));  // Must not have high bits set.
            c = (uint32_t)bits;
        }
        T *get() const { return (T *)(((size_t)c) << COMPRESS_BITS); }
        operator T *() const { return get(); }
        T *operator->() const { return get(); }
    };
    typedef CompressedPtr<LString> LStringPtr;
    typedef CompressedPtr<LVector> LVectorPtr;
    typedef CompressedPtr<LStruct> LStructPtr;
    typedef CompressedPtr<LCoRoutine> LCoRoutinePtr;
    typedef CompressedPtr<LResource> LResourcePtr;
    typedef CompressedPtr<BoxedInt> BoxedIntPtr;
    typedef CompressedPtr<BoxedFloat> BoxedFloatPtr;
    typedef CompressedPtr<RefObj> RefObjPtr;
#endif

static_assert(sizeof(intp) == sizeof(floatp) && sizeof(intp) == sizeof(RefObjPtr),
              "typedefs need fixing");

struct Value {
    #if RTT_ENABLED
    ValueType type;
    #endif

    private:
    union {
        // All these types can be defined to be either all 32 or 64-bit, depending on the
        // compilation mode.

        // Unboxed values.
        intp ival_;      // scalars stored as pointer-sized versions.
        floatp fval_;
        InsPtr ip_;  // Never gets converted to any, so no boxed version available.

        // Reference values (includes NULL if nillable version).
        LStringPtr sval_;
        LVectorPtr vval_;
        LStructPtr stval_;
        LCoRoutinePtr cval_;
        LResourcePtr xval_;

        // Generic reference access.
        RefObjPtr ref_;
    };
    public:

    // These asserts help track down any invalid code generation issues.
    intp        ival  () const { assert(type == V_INT);        return ival_;        }
    floatp      fval  () const { assert(type == V_FLOAT);      return fval_;        }
    int         intval() const { assert(type == V_INT);        return (int)ival_;   }
    float       fltval() const { assert(type == V_FLOAT);      return (float)fval_; }
    LString    *sval  () const { assert(type == V_STRING);     return sval_;        }
    LVector    *vval  () const { assert(type == V_VECTOR);     return vval_;        }
    LStruct    *stval () const { assert(type == V_STRUCT);     return stval_;       }
    LCoRoutine *cval  () const { assert(type == V_COROUTINE);  return cval_;        }
    LResource  *xval  () const { assert(type == V_RESOURCE);   return xval_;        }
    RefObj     *ref   () const { assert(IsRef(type));          return ref_;         }
    RefObj     *refnil() const { assert(IsRefNil(type));       return ref_;         }
    InsPtr      ip    () const { assert(type >= V_FUNCTION);   return ip_;          }
    void       *any   () const {                               return ref_;         }

    template<typename T, bool IF> T ifval() const {
        if constexpr (IF) { assert(type == V_FLOAT); return (T)fval_; }
        else              { assert(type == V_INT);   return (T)ival_; }
    }

    void setival(intp i)   { assert(type == V_INT);   ival_ = i; }
    void setfval(floatp f) { assert(type == V_FLOAT); fval_ = f; }

    inline Value()                   : TYPE_INIT(V_NIL)        ref_(nullptr)    {}
    inline Value(int i)              : TYPE_INIT(V_INT)        ival_(i)         {}
    inline Value(uint i)             : TYPE_INIT(V_INT)        ival_((intp)i)   {}
    inline Value(int64_t i)          : TYPE_INIT(V_INT)        ival_((intp)i)   {}
    inline Value(uint64_t i)         : TYPE_INIT(V_INT)        ival_((intp)i)   {}
    inline Value(int i, ValueType t) : TYPE_INIT(t)            ival_(i)         { (void)t; }
    inline Value(bool b)             : TYPE_INIT(V_INT)        ival_(b)         {}
    inline Value(float f)            : TYPE_INIT(V_FLOAT)      fval_(f)         {}
    inline Value(double f)           : TYPE_INIT(V_FLOAT)      fval_((floatp)f) {}
    inline Value(InsPtr i)           : TYPE_INIT(V_FUNCTION)   ip_(i)           {}

    inline Value(LString *s)         : TYPE_INIT(V_STRING)     sval_(s)         {}
    inline Value(LVector *v)         : TYPE_INIT(V_VECTOR)     vval_(v)         {}
    inline Value(LStruct *s)         : TYPE_INIT(V_STRUCT)     stval_(s)        {}
    inline Value(LCoRoutine *c)      : TYPE_INIT(V_COROUTINE)  cval_(c)         {}
    inline Value(LResource *r)       : TYPE_INIT(V_RESOURCE)   xval_(r)         {}
    inline Value(RefObj *r)          : TYPE_INIT(V_NIL)        ref_(r)          { assert(false); }

    inline bool True() const { return ival_ != 0; }

    inline Value &LTINCRT() {
        assert(IsRef(type) && ref_);
        ref_->Inc();
        return *this;
    }
    inline Value &LTINCRTNIL() { if (ref_) LTINCRT(); return *this; }
    inline Value &LTINCTYPE(ValueType t) { return IsRefNil(t) ? LTINCRTNIL() : *this; }

    inline void LTDECRT(VM &vm) const {  // we already know its a ref type
        assert(IsRef(type) && ref_);
        ref_->Dec(vm);
    }
    inline void LTDECRTNIL(VM &vm) const { if (ref_) LTDECRT(vm); }
    inline void LTDECTYPE(VM &vm, ValueType t) const { if (IsRefNil(t)) LTDECRTNIL(vm); }


    void ToString(VM &vm, ostringstream &ss, ValueType vtype, PrintPrefs &pp) const;
    bool Equal(VM &vm, ValueType vtype, const Value &o, ValueType otype, bool structural) const;
    intp Hash(VM &vm, ValueType vtype);
    Value Copy(VM &vm);  // Shallow.
};

template<typename T> inline T *AllocSubBuf(VM &vm, size_t size, type_elem_t tti);
template<typename T> inline void DeallocSubBuf(VM &vm, T *v, size_t size);

struct LStruct : RefObj {
    LStruct(type_elem_t _tti) : RefObj(_tti) {}

    // FIXME: reduce the use of these.
    intp Len(VM &vm) const { return ti(vm).len; }

    Value *Elems() const { return (Value *)(this + 1); }

    // This may only be called from a context where i < len has already been ensured/asserted.
    Value &AtS(intp i) const {
        return Elems()[i];
    }

    // This may only be called from a context where i < len has already been ensured/asserted.
    void DecS(VM &vm, intp i) const {
        AtS(i).LTDECTYPE(vm, ElemTypeS(vm, i));
    }

    void DeleteSelf(VM &vm);

    // This may only be called from a context where i < len has already been ensured/asserted.
    ValueType ElemTypeS(VM &vm, intp i) const;

    void ToString(VM &vm, ostringstream &ss, PrintPrefs &pp);

    bool Equal(VM &vm, const LStruct &o) {
        // RefObj::Equal has already guaranteed the typeoff's are the same.
        auto len = Len(vm);
        assert(len == o.Len(vm));
        for (intp i = 0; i < len; i++) {
            auto et = ElemTypeS(vm, i);
            if (!AtS(i).Equal(vm, et, o.AtS(i), et, true))
                return false;
        }
        return true;
    }

    intp Hash(VM &vm) {
        intp hash = 0;
        for (int i = 0; i < Len(vm); i++) hash ^= AtS(i).Hash(vm, ElemTypeS(vm, i));
        return hash;
    }

    void Init(VM &vm, Value *from, intp len, bool inc) {
        assert(len && len == Len(vm));
        memcpy(Elems(), from, len * sizeof(Value));
        if (inc) for (intp i = 0; i < len; i++) {
            AtS(i).LTINCTYPE(ElemTypeS(vm, i));
        }
    }
};

struct LVector : RefObj {
    intp len;    // has to match the Value integer type, since we allow the length to be obtained
    intp maxl;

    private:
    Value *v;   // use At()

    public:
    LVector(VM &vm, intp _initial, intp _max, type_elem_t _tti);

    ~LVector() { assert(0); }   // destructed by DECREF

    void DeallocBuf(VM &vm) {
        if (v) DeallocSubBuf(vm, v, maxl);
    }

    void Dec(VM &vm, intp i, ValueType et) const {
        At(i).LTDECTYPE(vm, et);
    }

    void DeleteSelf(VM &vm);

    ValueType ElemType(VM &vm) const;

    void Resize(VM &vm, intp newmax);

    void Push(VM &vm, const Value &val) {
        if (len == maxl) Resize(vm, maxl ? maxl * 2 : 4);
        v[len++] = val;
    }

    Value Pop() {
        return v[--len];
    }

    Value &Top() const {
        return v[len - 1];
    }

    void Insert(VM &vm, Value &val, intp i) {
        assert(i >= 0 && i <= len); // note: insertion right at the end is legal, hence <=
        if (len + 1 > maxl) Resize(vm, max(len + 1, maxl ? maxl * 2 : 4));
        memmove(v + i + 1, v + i, sizeof(Value) * (len - i));
        len++;
        v[i] = val;
    }

    Value Remove(VM &vm, intp i, intp n, intp decfrom) {
        assert(n >= 0 && n <= len && i >= 0 && i <= len - n);
        auto x = v[i];
        auto et = ElemType(vm);
        for (intp j = decfrom; j < n; j++) Dec(vm, i + j, et);
        memmove(v + i, v + i + n, sizeof(Value) * (len - i - n));
        len -= n;
        return x;
    }

    Value *Elems() { return v; }

    Value &At(intp i) const {
        assert(i < len);
        return v[i];
    }

    void Append(VM &vm, LVector *from, intp start, intp amount);

    void ToString(VM &vm, ostringstream &ss, PrintPrefs &pp);

    bool Equal(VM &vm, const LVector &o) {
        // RefObj::Equal has already guaranteed the typeoff's are the same.
        if (len != o.len) return false;
        auto et = ElemType(vm);
        for (intp i = 0; i < len; i++) {
            if (!At(i).Equal(vm, et, o.At(i), et, true))
                return false;
        }
        return true;
    }

    intp Hash(VM &vm) {
        intp hash = 0;
        auto et = ElemType(vm);
        for (int i = 0; i < len; i++) hash ^= At(i).Hash(vm, et);
        return hash;
    }

    void Init(VM &vm, Value *from, bool inc) {
        assert(len);
        memcpy(&At(0), from, len * sizeof(Value));
        auto et = ElemType(vm);
        if (inc && IsRefNil(et))
            for (intp i = 0; i < len; i++) {
                At(i).LTINCRTNIL();
            }
    }
};

struct VMLog {
    struct LogVar {
        vector<Value> values;
        size_t read;
        const TypeInfo *type;
    };
    vector<LogVar> logvars;

    VM &vm;
    VMLog(VM &_vm);

    void LogInit(const uchar *bcf);
    void LogPurge();
    void LogFrame();
    Value LogGet(Value def, int idx);
    void LogWrite(Value newval, int idx);
    void LogCleanup();
};

struct StackFrame {
    InsPtr retip;
    const int *funstart;
    int spstart;
};

struct NativeFun;
struct NativeRegistry;

struct VM {
    SlabAlloc pool;

    NativeRegistry &natreg;

    Value *stack = nullptr;
    int stacksize = 0;
    int maxstacksize;
    int sp = -1;

    Value retvalstemp[MAX_RETURN_VALUES];

    #ifdef VM_COMPILED_CODE_MODE
        block_t next_call_target = 0;
        block_t *next_mm_table = nullptr;
        const int *next_mm_call = nullptr;
    #else
        const int *ip = nullptr;
    #endif

    vector<StackFrame> stackframes;

    LCoRoutine *curcoroutine = nullptr;

    Value *vars = nullptr;

    size_t codelen = 0;
    const int *codestart = nullptr;
    vector<int> codebigendian;
    vector<type_elem_t> typetablebigendian;
    uint64_t *byteprofilecounts = nullptr;

    string bytecode_buffer;
    const bytecode::BytecodeFile *bcf = nullptr;

    PrintPrefs programprintprefs { 10, 10000, false, -1 };
    const type_elem_t *typetable = nullptr;
    string evalret;

    int currentline = -1;
    int maxsp = -1;

    PrintPrefs debugpp { 2, 50, true, -1 };

    string programname;

    VMLog vml { *this };

    ostringstream ss_reuse;

    bool trace = false;
    bool trace_tail = false;
    vector<ostringstream> trace_output;
    size_t trace_ring_idx = 0;

    vector<RefObj *> delete_delay;

    vector<LString *> constant_strings;

    int64_t vm_count_ins = 0;
    int64_t vm_count_fcalls = 0;
    int64_t vm_count_bcalls = 0;
    int64_t vm_count_decref = 0;

    #ifndef VM_COMPILED_CODE_MODE
        //#define VM_INS_SWITCH
    #endif

    //#define VM_ERROR_RET_EXPERIMENT
    #if defined(VM_ERROR_RET_EXPERIMENT) && !defined(VM_COMPILED_CODE_MODE)
        #define VM_INS_RET bool
        #ifdef VM_INS_SWITCH
            #define VM_RET break
            #define VM_TERMINATE return
        #else
            #define VM_RET return false
            #define VM_TERMINATE return true
        #endif
    #else
        #define VM_INS_RET void
        #ifdef VM_INS_SWITCH
            #define VM_RET break
            #define VM_TERMINATE return
        #else
            #define VM_RET
            #define VM_TERMINATE
        #endif
    #endif

    #ifndef VM_INS_SWITCH
    typedef VM_INS_RET (VM::* f_ins_pointer)();
    f_ins_pointer f_ins_pointers[IL_MAX_OPS];
    #endif

    const void *compiled_code_ip;

    const vector<string> &program_args;

    VM(NativeRegistry &natreg, string_view _pn, string &_bytecode_buffer, const void *entry_point,
       const void *static_bytecode, const vector<string> &args);
    ~VM();

    void OneMoreFrame();

    const TypeInfo &GetTypeInfo(type_elem_t offset) { return *(TypeInfo *)(typetable + offset); }
    const TypeInfo &GetVarTypeInfo(int varidx);

    void SetMaxStack(int ms) { maxstacksize = ms; }
    string_view GetProgramName() { return programname; }

    type_elem_t GetIntVectorType(int which);
    type_elem_t GetFloatVectorType(int which);

    void DumpVal(RefObj *ro, const char *prefix);
    void DumpFileLine(const int *fip, ostringstream &ss);
    void DumpLeaks();
    
    void OnAlloc(RefObj *ro);
    LVector *NewVec(intp initial, intp max, type_elem_t tti);
    LStruct *NewStruct(intp max, type_elem_t tti);
    LCoRoutine *NewCoRoutine(InsPtr rip, const int *vip, LCoRoutine *p, type_elem_t tti);
    LResource *NewResource(void *v, const ResourceType *t);
    LString *NewString(size_t l);
    LString *NewString(string_view s);
    LString *NewString(string_view s1, string_view s2);
    LString *ResizeString(LString *s, intp size, int c, bool back);

    Value Error(string err, const RefObj *a = nullptr, const RefObj *b = nullptr);
    Value BuiltinError(string err) { return Error(err); }
    void VMAssert(const char *what);
    void VMAssert(const char *what, const RefObj *a, const RefObj *b);

    void DumpVar(ostringstream &ss, const Value &x, size_t idx, bool dumpglobals);

    void EvalMulti(const int *mip, const int *call_arg_types, block_t comp_retip);

    void FinalStackVarsCleanup();

    #ifdef VM_COMPILED_CODE_MODE
        #define VM_OP_ARGS const int *ip
        #define VM_OP_ARGS_CALL const int *ip, block_t fcont
        #define VM_JMP_RET bool
    #else
        #define VM_OP_ARGS
        #define VM_OP_ARGS_CALL
        #define VM_JMP_RET VM_INS_RET
    #endif

    void JumpTo(InsPtr j);
    InsPtr GetIP();
    template<int is_error> int VarCleanup(ostringstream *error, int towhere);
    void StartStackFrame(InsPtr retip);
    void FunIntroPre(InsPtr fun);
    void FunIntro(VM_OP_ARGS);
    void FunOut(int towhere, int nrv);

    void CoVarCleanup(LCoRoutine *co);
    void CoNonRec(const int *varip);
    void CoNew(VM_OP_ARGS_CALL);
    void CoSuspend(InsPtr retip);
    void CoClean();
    void CoYield(VM_OP_ARGS_CALL);
    void CoResume(LCoRoutine *co);

    void EndEval(const Value &ret, ValueType vt);

    #ifndef VM_INS_SWITCH
        #define F(N, A) VM_INS_RET F_##N(VM_OP_ARGS);
            ILBASENAMES
        #undef F
        #define F(N, A) VM_INS_RET F_##N(VM_OP_ARGS_CALL);
            ILCALLNAMES
        #undef F
        #define F(N, A) VM_JMP_RET F_##N();
            ILJUMPNAMES
        #undef F
    #endif

    void EvalProgram();
    void EvalProgramInner();

    void PushDerefIdxVector(intp i);
    void PushDerefIdxStruct(intp i);
    void PushDerefIdxString(intp i);
    void LvalueIdxVector(int lvalop, intp i);
    void LvalueIdxStruct(int lvalop, intp i);
    void LvalueField(int lvalop, intp i);
    void LvalueOp(int op, Value &a);

    string ProperTypeName(const TypeInfo &ti);

    void Div0() { Error("division by zero"); }
    void IDXErr(intp i, intp n, const RefObj *v);
    void BCallProf();
    void BCallRetCheck(const NativeFun *nf);
    intp GrabIndex(const Value &idx);

    void Push(const Value &v);
    Value Pop();

    string_view StructName(const TypeInfo &ti);
    string_view ReverseLookupType(uint v);
    void Trace(bool on, bool tail) { trace = on; trace_tail = tail; }
    double Time() { return SecondsSinceStart(); }

    Value ToString(const Value &a, ValueType vt) {
        ss_reuse.str(string());
        ss_reuse.clear();
        a.ToString(*this, ss_reuse, vt, programprintprefs);
        return NewString(ss_reuse.str());
    }
};

inline int64_t Read64FromIp(const int *&ip) {
    int64_t v = (uint)*ip++;
    v |= ((int64_t)*ip++) << 32;
    return v;
}

inline const TypeInfo &DynAlloc::ti(VM &vm) const { return vm.GetTypeInfo(tti); }

template<typename T> inline T *AllocSubBuf(VM &vm, size_t size, type_elem_t tti) {
    auto header_sz = max(alignof(T), sizeof(DynAlloc));
    auto mem = (uchar *)vm.pool.alloc(size * sizeof(T) + header_sz);
    ((DynAlloc *)mem)->tti = tti;
    mem += header_sz;
    return (T *)mem;
}

template<typename T> inline void DeallocSubBuf(VM &vm, T *v, size_t size) {
    auto header_sz = max(alignof(T), sizeof(DynAlloc));
    auto mem = ((uchar *)v) - header_sz;
    vm.pool.dealloc(mem, size * sizeof(T) + header_sz);
}

template<bool back> LString *WriteMem(VM &vm, LString *s, intp i, const void *data, size_t size) {
    auto minsize = i + (intp)size;
    if (s->len < minsize) s = vm.ResizeString(s, minsize * 2, 0, back);
    memcpy((void *)(s->data() + (back ? s->len - i - size : i)), data, size);
    return s;
}

template<typename T, bool back> LString *WriteValLE(VM &vm, LString *s, intp i, T val) {
    T t = flatbuffers::EndianScalar(val);
    return WriteMem<back>(vm, s, i, &t, sizeof(T));
}

template<typename T, bool back> T ReadValLE(const LString *s, intp i) {
    T val;
    memcpy(&val, (void *)(s->data() + (back ? s->len - i - sizeof(T) : i)), sizeof(T));
    return flatbuffers::EndianScalar(val);
}

// FIXME: turn check for len into an assert and make caller guarantee lengths match.
template<int N> inline vec<floatp, N> ValueToF(VM &vm, const Value &v, floatp def = 0) {
    vec<floatp, N> t;
    for (int i = 0; i < N; i++) t[i] = v.stval()->Len(vm) > i ? v.stval()->AtS(i).fval() : def;
    return t;
}

template<int N> inline vec<intp, N> ValueToI(VM &vm, const Value &v, intp def = 0) {
    vec<intp, N> t;
    for (int i = 0; i < N; i++) t[i] = v.stval()->Len(vm) > i ? v.stval()->AtS(i).ival() : def;
    return t;
}

template<int N> inline vec<floatp, N> ValueDecToF(VM &vm, const Value &v, floatp def = 0) {
    auto r = ValueToF<N>(vm, v, def);
    return r;
}

template<int N> inline vec<intp, N> ValueDecToI(VM &vm, const Value &v, intp def = 0) {
    auto r = ValueToI<N>(vm, v, def);
    return r;
}

// Versions that cast to int/float regardless of the size of intp/floatp.
template<int N> inline vec<float, N> ValueToFLT(VM &vm, const Value &v, floatp def = 0) {
    return vec<float, N>(ValueToF<N>(vm, v, def));
}
template<int N> inline vec<int, N> ValueToINT(VM &vm, const Value &v, intp def = 0) {
    return vec<int, N>(ValueToI<N>(vm, v, def));
}
template<int N> inline vec<float, N> ValueDecToFLT(VM &vm, const Value &v, floatp def = 0) {
    return vec<float, N>(ValueDecToF<N>(vm, v, def));
}
template<int N> inline vec<int, N> ValueDecToINT(VM &vm, const Value &v, intp def = 0) {
    return vec<int, N>(ValueDecToI<N>(vm, v, def));
}


template <int N> inline Value ToValueI(VM &vm, const vec<intp, N> &v, intp maxelems = 4) {
    auto numelems = min(maxelems, (intp)N);
    auto tti = vm.GetIntVectorType((int)numelems);
    assert(tti >= 0);
    auto nv = vm.NewStruct(numelems, tti);
    for (intp i = 0; i < numelems; i++) nv->AtS(i) = Value(v[i]);
    return Value(nv);
}

template <int N> inline Value ToValueF(VM &vm, const vec<floatp, N> &v, intp maxelems = 4) {
    auto numelems = min(maxelems, (intp)N);
    auto tti = vm.GetFloatVectorType((int)numelems);
    assert(tti >= 0);
    auto nv = vm.NewStruct(numelems, tti);
    for (intp i = 0; i < numelems; i++) nv->AtS(i) = Value(v[i]);
    return Value(nv);
}

template <int N> inline Value ToValueINT(VM &vm, const vec<int, N> &v, int maxelems = 4) {
    return ToValueI<N>(vm, vec<intp, N>(v), maxelems);
}
template <int N> inline Value ToValueFLT(VM &vm, const vec<float, N> &v, int maxelems = 4) {
    return ToValueF<N>(vm, vec<floatp, N>(v), maxelems);
}

inline intp RangeCheck(VM &vm, const Value &idx, intp range, intp bias = 0) {
    auto i = idx.ival();
    if (i < bias || i >= bias + range)
        vm.BuiltinError(cat("index out of range [", bias, "..", bias + range, "): ", i));
    return i;
}

template<typename T> inline T GetResourceDec(VM &vm, Value &val, const ResourceType *type) {
    if (!val.True())
        return nullptr;
    auto x = val.xval();
    if (x->type != type)
        vm.BuiltinError(string_view("needed resource type: ") + type->name + ", got: " +
            x->type->name);
    return (T)x->val;
}

inline vector<string> ValueToVectorOfStrings(Value &v) {
    vector<string> r;
    for (int i = 0; i < v.vval()->len; i++) r.push_back(string(v.vval()->At(i).sval()->strv()));
    return r;
}

inline Value ToValueOfVectorOfStrings(VM &vm, const vector<string> &in) {
    auto v = vm.NewVec(0, (intp)in.size(), TYPE_ELEM_VECTOR_OF_STRING);
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

void EscapeAndQuote(string_view s, ostringstream &ss);

struct LCoRoutine : RefObj {
    bool active = true;  // Goes to false when it has hit the end of the coroutine instead of a yield.

    int stackstart;    // When currently running, otherwise -1
    Value *stackcopy = nullptr;
    int stackcopylen = 0;
    int stackcopymax = 0;

    int stackframestart;  // When currently running, otherwise -1
    StackFrame *stackframescopy = nullptr;
    int stackframecopylen = 0;
    int stackframecopymax = 0;
    int top_at_suspend = -1;

    InsPtr returnip;
    const int *varip;
    LCoRoutine *parent;

    LCoRoutine(int _ss, int _sfs, InsPtr _rip, const int *_vip, LCoRoutine *_p, type_elem_t cti)
        : RefObj(cti), stackstart(_ss), stackframestart(_sfs), returnip(_rip), varip(_vip),
          parent(_p) {}

    Value &Current(VM &vm) {
        if (stackstart >= 0) vm.BuiltinError("cannot get value of active coroutine");
        return stackcopy[stackcopylen - 1].LTINCTYPE(vm.GetTypeInfo(ti(vm).yieldtype).t);
    }

    void Resize(VM &vm, int newlen) {
        if (newlen > stackcopymax) {
            if (stackcopy) DeallocSubBuf(vm, stackcopy, stackcopymax);
            stackcopy = AllocSubBuf<Value>(vm, stackcopymax = newlen, TYPE_ELEM_VALUEBUF);
        }
        stackcopylen = newlen;
    }

    void ResizeFrames(VM &vm, int newlen) {
        if (newlen > stackframecopymax) {
            if (stackframescopy) DeallocSubBuf(vm, stackframescopy, stackframecopymax);
            stackframescopy = AllocSubBuf<StackFrame>(vm, stackframecopymax = newlen,
                                                      TYPE_ELEM_STACKFRAMEBUF);
        }
        stackframecopylen = newlen;
    }

    int Suspend(VM &vm, int top, Value *stack, vector<StackFrame> &stackframes, InsPtr &rip,
                LCoRoutine *&curco) {
        assert(stackstart >= 0);
        swap(rip, returnip);
        assert(curco == this);
        curco = parent;
        parent = nullptr;
        ResizeFrames(vm, (int)stackframes.size() - stackframestart);
        memcpy(stackframescopy, stackframes.data() + stackframestart,
               stackframecopylen * sizeof(StackFrame));
        stackframes.erase(stackframes.begin() + stackframestart, stackframes.end());
        stackframestart = -1;
        top_at_suspend = top;
        Resize(vm, top - stackstart);
        memcpy(stackcopy, stack + stackstart, stackcopylen * sizeof(Value));
        int ss = stackstart;
        stackstart = -1;
        return ss;
    }

    void AdjustStackFrames(int top) {
        int topdelta = (top + stackcopylen) - top_at_suspend;
        if (topdelta) {
            for (int i = 0; i < stackframecopylen; i++) {
                stackframescopy[i].spstart += topdelta;
            }
        }
    }

    int Resume(int top, Value *stack, vector<StackFrame> &stackframes, InsPtr &rip, LCoRoutine *p) {
        assert(stackstart < 0);
        swap(rip, returnip);
        assert(!parent);
        parent = p;
        stackframestart = (int)stackframes.size();
        AdjustStackFrames(top);
        stackframes.insert(stackframes.end(), stackframescopy, stackframescopy + stackframecopylen);
        stackstart = top;
        // FIXME: assume that it fits, which is not guaranteed with recursive coros
        memcpy(stack + top, stackcopy, stackcopylen * sizeof(Value));
        return stackcopylen;
    }

    void BackupParentVars(VM &vm, Value *vars) {
        // stored here while coro is active
        Resize(vm, *varip);
        for (int i = 1; i <= *varip; i++) {
            auto &var = vars[varip[i]];
            // we don't INC, since parent var is still on the stack and will hold ref
            stackcopy[i - 1] = var;
        }
    }

    Value &AccessVar(int savedvaridx) {
        assert(stackstart < 0);
        // Variables are always saved on top of the stack before the stackcopy gets made, so they
        // are last, followed by the retval (thus -1).
        return stackcopy[stackcopylen - *varip + savedvaridx - 1];
    }

    Value &GetVar(VM &vm, int ididx) {
        if (stackstart >= 0)
            vm.BuiltinError("cannot access locals of running coroutine");
        // FIXME: we can probably make it work without this search, but for now no big deal
        for (int i = 1; i <= *varip; i++) {
            if (varip[i] == ididx) {
                return AccessVar(i - 1);
            }
        }
        // This one should be really rare, since parser already only allows lexically contained vars
        // for that function, could happen when accessing var that's not in the callchain of yields.
        vm.BuiltinError("local variable being accessed is not part of coroutine state");
        return *stackcopy;
    }

    void DeleteSelf(VM &vm) {
        assert(stackstart < 0);
        if (stackcopy) {
            auto curvaltype = vm.GetTypeInfo(ti(vm).yieldtype).t;
            auto &ts = stackcopy[--stackcopylen];
            ts.LTDECTYPE(vm, curvaltype);
            if (active) {
                for (int i = *varip; i > 0; i--) {
                    auto &vti = vm.GetVarTypeInfo(varip[i]);
                    stackcopy[--stackcopylen].LTDECTYPE(vm, vti.t);
                }
                top_at_suspend -= *varip + 1;
                // This calls Resume() to get the rest back onto the stack, then unwinds it.
                vm.CoVarCleanup(this);
            } else {
               assert(!stackcopylen);
            }
            DeallocSubBuf(vm, stackcopy, stackcopymax);
        }
        if (stackframescopy) DeallocSubBuf(vm, stackframescopy, stackframecopymax);
        vm.pool.dealloc(this, sizeof(LCoRoutine));
    }

    ValueType ElemType(VM &vm, int i) {
        assert(i < *varip);
        auto varidx = varip[i + 1];
        auto &vti = vm.GetVarTypeInfo(varidx);
        auto vt = vti.t;
        if (vt == V_NIL) vt = vm.GetTypeInfo(vti.subt).t;
        #if RTT_ENABLED
        auto &var = AccessVar(i);
        // FIXME: For testing.
        if(vt != var.type && var.type != V_NIL && !(vt == V_VECTOR && var.type == V_STRUCT)) {
            LOG_INFO("coro elem ", vti.Debug(vm), " != ", BaseTypeName(var.type));
            assert(false);
        }
        #endif
        return vt;
    }
};

}  // namespace lobster

#endif  // LOBSTER_VMDATA
