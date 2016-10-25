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

#include "il.h"
#include "bytecode_generated.h"

namespace lobster {

#ifdef _DEBUG
#define RTT_ENABLED 1
#else
#define RTT_ENABLED 0
#endif

enum ValueType : int
{
    // refc types are negative
    V_MINVMTYPES = -10,
    V_ANY = -9,         // any other reference type.
    V_STACKFRAMEBUF = -8,
    V_VALUEBUF = -7,    // only used as memory type for vector/coro buffers, Value not allowed to refer to this
    V_BOXEDFLOAT = -6,
    V_BOXEDINT = -5,
    V_COROUTINE = -4,
    V_STRING = -3,
    V_STRUCT = -2,
    V_VECTOR = -1,
    V_NIL = 0,          // VM: null reference, Type checker: a type that may be nil or a reference type.
    V_INT,
    V_FLOAT,
    V_FUNCTION,
    V_YIELD,
    V_VAR,              // [typechecker only] like V_ANY, except idx refers to a type variable
    V_TYPEID,           // [typechecker only] a typetable offset.
    // used in log, if they appear as a value in a program, that's a bug
    V_LOGSTART, V_LOGEND, V_LOGMARKER,
    V_MAXVMTYPES
};

inline bool IsScalar (ValueType t) { return t == V_INT || t == V_FLOAT; }
inline bool IsRef    (ValueType t) { return t <  V_NIL; }
inline bool IsRefNil (ValueType t) { return t <= V_NIL; }
inline bool IsVector (ValueType t) { return t == V_VECTOR || t == V_STRUCT; }
inline bool IsRuntime(ValueType t) { return t < V_VAR; }

inline const char *BaseTypeName(ValueType t)
{
    static const char *typenames[] =
    {
        "any", "<value_buffer>", "<stackframe_buffer>",
        "boxed_float", "boxed_int", "coroutine", "string", "struct", "vector", 
        "nil", "int", "float", "function", "yield_function", "variable", "typeid",
        "<logstart>", "<logend>", "<logmarker>"
    };
    if (t <= V_MINVMTYPES || t >= V_MAXVMTYPES)
    {
        assert(false);
        return "<internal-error-type>";
    }
    return typenames[t - V_MINVMTYPES - 1];
}

enum type_elem_t : int   // Strongly typed element of typetable.
{
    // These must correspond to typetable init in Codegen constructor.
    TYPE_ELEM_INT,
    TYPE_ELEM_FLOAT,
    TYPE_ELEM_BOXEDINT,
    TYPE_ELEM_BOXEDFLOAT,
    TYPE_ELEM_STRING,
    TYPE_ELEM_ANY,
    TYPE_ELEM_VALUEBUF,
    TYPE_ELEM_STACKFRAMEBUF,
    TYPE_ELEM_VECTOR_OF_INT = 8,   // 2 each.
    TYPE_ELEM_VECTOR_OF_FLOAT = 10,
    TYPE_ELEM_VECTOR_OF_STRING = 12,
    TYPE_ELEM_VECTOR_OF_VECTOR_OF_INT = 14,
    TYPE_ELEM_VECTOR_OF_VECTOR_OF_FLOAT = 16,

    TYPE_ELEM_FIXED_OFFSET_END = 18
};

struct TypeInfo
{
    ValueType t;
    union
    {
        type_elem_t subt;  // V_VECTOR | V_NIL
        struct { int structidx; int len; type_elem_t elems[1]; };    // V_STRUCT
        int sfidx;  // V_FUNCTION;
        struct { int cofunidx; type_elem_t yieldtype; };  // V_COROUTINE
    };
    
    TypeInfo() = delete;
    TypeInfo(const TypeInfo &) = delete;
    TypeInfo &operator=(const TypeInfo &) = delete;

    string Debug(bool rec = true) const;
};

struct Value;
struct LString;
struct ElemObj;
struct LVector;
struct LStruct;
struct CoRoutine;

struct PrintPrefs
{
    int depth;
    int budget;
    bool quoted;
    int decimals;
    int cycles;
    bool anymark;

    PrintPrefs(int _depth, int _budget, bool _quoted, int _decimals, bool _anymark)
        : depth(_depth), budget(_budget), quoted(_quoted), decimals(_decimals), cycles(-1), anymark(_anymark) {}
};

typedef void *(*block_t)();

struct VM;

// the 2 globals that make up the current VM instance
extern VM *g_vm;
extern SlabAlloc *vmpool;

struct DynAlloc     // ANY memory allocated by the VM must inherit from this, so we can identify leaked memory
{
    const TypeInfo &ti;    // offset into the VM's typetable

    DynAlloc(const TypeInfo &_ti) : ti(_ti) {}
};

struct RefObj : DynAlloc
{
    int refc;

    RefObj(const TypeInfo &_ti) : DynAlloc(_ti), refc(1) {}

    void Inc()
    {
        refc++;
    }

    void Dec()
    {
        refc--;
        if (refc <= 0) DECDELETE(true);
    }

    void CycleDone(int &cycles)
    {
        refc = -(++cycles);
    }

    string CycleStr() const { return "_" + to_string(-refc) + "_"; }

    void DECDELETE(bool deref);
    void Mark();
};

extern bool RefEqual(const RefObj *a, const RefObj *b, bool structural);
extern string RefToString(const RefObj *ro, PrintPrefs &pp);

struct BoxedInt : RefObj
{
    int val;

    BoxedInt(int _v);
};

struct BoxedFloat : RefObj
{
    float val;

    BoxedFloat(float _v);
};

struct LString : RefObj
{
    int len;    // has to match the Value integer type, since we allow the length to be obtained

    LString(int _l);

    char *str() { return (char *)(this + 1); }

    string ToString(PrintPrefs &pp);

    char HexChar(char i) { return i + (i < 10 ? '0' : 'A' - 10); }

    void DeleteSelf() { vmpool->dealloc(this, sizeof(LString) + len + 1); }

    bool operator==(LString &o) { return strcmp(str(), o.str()) == 0; }
    bool operator!=(LString &o) { return strcmp(str(), o.str()) != 0; }
    bool operator< (LString &o) { return strcmp(str(), o.str()) <  0; }
    bool operator<=(LString &o) { return strcmp(str(), o.str()) <= 0; }
    bool operator> (LString &o) { return strcmp(str(), o.str()) >  0; }
    bool operator>=(LString &o) { return strcmp(str(), o.str()) >= 0; }
};

#if RTT_ENABLED
#define TYPE_ASSERT(cond) assert(cond)
#define TYPE_INIT(t) type(t),
#else
#define TYPE_ASSERT(cond) ((void)0)
#define TYPE_INIT(t)
#endif

struct Value
{
    #if RTT_ENABLED
    ValueType type;
    #endif

    private:
    union
    {
        // Unboxed values.
        intp ival_;      // scalars stored as pointer-sized versions.
        floatp fval_;
        const int *ip_;  // Never gets converted to any, so no boxed version available.

        // Reference values (includes NULL if nillable version).
        LString *sval_;
        LVector *vval_;
        LStruct *stval_;
        CoRoutine *cval_;

        // Boxed scalars (never NULL)
        BoxedInt *bival_;
        BoxedFloat *bfval_;

        // Generic reference access.
        RefObj *ref_;
        ElemObj *eval_;
    };
    public:

    // These asserts help track down any invalid code generation issues.
    int         ival  () const { TYPE_ASSERT(type == V_INT);        return (int)ival_;   }
    float       fval  () const { TYPE_ASSERT(type == V_FLOAT);      return (float)fval_; }
    LString    *sval  () const { TYPE_ASSERT(type == V_STRING);     return sval_;        }
    BoxedInt   *bival () const { TYPE_ASSERT(type == V_BOXEDINT);   return bival_;       }
    BoxedFloat *bfval () const { TYPE_ASSERT(type == V_BOXEDFLOAT); return bfval_;       }
    LVector    *vval  () const { TYPE_ASSERT(type == V_VECTOR);     return vval_;        }
    LStruct    *stval () const { TYPE_ASSERT(type == V_STRUCT);     return stval_;       }
    CoRoutine  *cval  () const { TYPE_ASSERT(type == V_COROUTINE);  return cval_;        }
    ElemObj    *eval  () const { TYPE_ASSERT(IsVector(type));       return eval_;        }
    RefObj     *ref   () const { TYPE_ASSERT(IsRef(type));          return ref_;         }
    RefObj     *refnil() const { TYPE_ASSERT(IsRefNil(type));       return ref_;         }
    const int  *ip    () const { TYPE_ASSERT(type >= V_FUNCTION);   return ip_;          }
    int         logip () const { TYPE_ASSERT(type >= V_LOGSTART);   return (int)ival_;   }
    void       *any   () const {                                    return ref_;         }
                                                                       
    void setival(int i)   { TYPE_ASSERT(type == V_INT);   ival_ = i; }
    void setfval(float f) { TYPE_ASSERT(type == V_FLOAT); fval_ = f; }

    inline Value()                          : TYPE_INIT(V_NIL)         ref_(nullptr) {}
    inline Value(int i)                     : TYPE_INIT(V_INT)         ival_(i)      {}
    inline Value(int i, ValueType t)        : TYPE_INIT(t)             ival_(i)      { (void)t; }
    inline Value(bool b)                    : TYPE_INIT(V_INT)         ival_(b)      {}
    inline Value(float f)                   : TYPE_INIT(V_FLOAT)       fval_(f)      {}
    inline Value(const int *i)              : TYPE_INIT(V_FUNCTION)    ip_(i)        {}
    inline Value(const int *i, ValueType t) : TYPE_INIT(t)             ip_(i)        { (void)t; }
    inline Value(RefObj *r)                 : TYPE_INIT(r->ti.t)       ref_(r)       {}

    inline bool True() const { return ival_ != 0; }

    inline Value &INCRT()
    {
        TYPE_ASSERT(IsRef(type) && ref_);
        ref_->Inc();
        return *this;
    }

    inline Value &INCRTNIL() { if (ref_) INCRT(); return *this; }
    inline Value &INCTYPE(ValueType t) { return IsRefNil(t) ? INCRTNIL() : *this; }

    inline void DECRT() const   // we already know its a ref type
    {
        TYPE_ASSERT(IsRef(type) && ref_);
        ref_->Dec();
    }

    inline void DECRTNIL() const { if (ref_) DECRT(); }
    inline void DECTYPE(ValueType t) const { if (IsRefNil(t)) DECRTNIL(); }

    int Nargs()
    {
        TYPE_ASSERT(type == V_FUNCTION);
        //assert(*ip == IL_FUNSTART);
        return ip_[1];
    }

    string ToString(ValueType vtype, PrintPrefs &pp) const;
    bool Equal(ValueType vtype, const Value &o, ValueType otype, bool structural) const;
    void Mark(ValueType vtype);
};

template<typename T> inline T *AllocSubBuf(size_t size, const TypeInfo &ti)
{
    auto mem = (const TypeInfo **)vmpool->alloc(size * sizeof(T) + sizeof(TypeInfo *));
    *mem = &ti;  // DynAlloc header.
    mem++;
    return (T *)mem;
}

template<typename T> inline void DeallocSubBuf(T *v, size_t size)
{
    auto mem = (TypeInfo **)v;
    mem--;
    vmpool->dealloc(mem, size * sizeof(T) + sizeof(TypeInfo *));
}

struct ElemObj : RefObj
{
    ElemObj(const TypeInfo &_ti) : RefObj(_ti) {}

    int Len() const;

    Value &At(int i) const;

    void Init(Value *from, int len, bool inc)
    {
        assert(len == Len());
        if (len)
        {
            memcpy(&At(0), from, len * sizeof(Value));
            if (inc) IncAll();
        }
    }

    ValueType ElemType(int i) const;

    // TODO: If any of the methods below ever become performance critical, we can duplicate them over LVector/LStruct,
    // such that the check for which type it is only is made once.

    Value &AtInc(int i) const
    {
        return At(i).INCTYPE(ElemType(i));
    }

    void Dec(int i) const
    {
        At(i).DECTYPE(ElemType(i));
    }

    void DecAll() const
    {
        for (int i = 0; i < Len(); i++) Dec(i);
    }

    void IncAll() const
    {
        for (int i = 0; i < Len(); i++) AtInc(i);
    }

    bool Equal(const ElemObj &o)
    {
        // RefObj::Equal has already guaranteed the typeoff's are the same.
        int len = Len();
        if (len != o.Len()) return false;
        for (int i = 0; i < len; i++)
            if (!At(i).Equal(ElemType(i), o.At(i), o.ElemType(i), true))
                return false;
        return true;
    }

    void Mark()
    {
        for (int i = 0; i < Len(); i++)
            At(i).Mark(ElemType(i));
    }

    string ToString(PrintPrefs &pp);
};

struct LStruct : ElemObj
{
    LStruct(const TypeInfo &_ti) : ElemObj(_ti) {}

    int Len() const { return ti.len; }
    Value *Elems() const { return (Value *)(this + 1); }

    Value &At(int i) const
    {
        assert(i < Len());
        return Elems()[i];
    }

    void DeleteSelf(bool deref)
    {
        if (deref) DecAll();
        vmpool->dealloc(this, sizeof(LStruct) + sizeof(Value) * Len());
    }
};

struct LVector : ElemObj
{
    int len;    // has to match the Value integer type, since we allow the length to be obtained

    private:
    Value *v;   // use At()
    
    public:
    int maxl;

    LVector(int _initial, int _max, const TypeInfo &_ti);

    ~LVector() { assert(0); }   // destructed by DECREF

    void DeallocBuf()
    {
        if (v) DeallocSubBuf(v, maxl);
    }

    void DeleteSelf(bool deref)
    {
        if (deref) DecAll();
        DeallocBuf();
        vmpool->dealloc_small(this);
    }

    const TypeInfo &ElemTypeInfo() const;

    void Resize(int newmax);

    void Push(const Value &val)
    {
        if (len == maxl) Resize(maxl ? maxl * 2 : 4);
        v[len++] = val;
    }

    Value Pop()
    {
        return v[--len];
    }

    Value &Top() const
    {
        return v[len - 1].INCTYPE(ElemTypeInfo().t);
    }
    
    void Insert(Value &val, int i)
    {
        assert(i >= 0 && i <= len); // note: insertion right at the end is legal, hence <= 
        if (len + 1 > maxl) Resize(max(len + 1, maxl ? maxl * 2 : 4));   
        memmove(v + i + 1, v + i, sizeof(Value) * (len - i));
        len++;
        v[i] = val;
    }

    Value Remove(int i, int n, int decfrom)
    { 
        assert(n >= 0 && n <= len && i >= 0 && i <= len - n);
        auto x = v[i];
        for (int j = decfrom; j < n; j++) Dec(i + j);
        memmove(v + i, v + i + n, sizeof(Value) * (len - i - n));
        len -= n;
        return x;
    }

    Value &At(int i) const
    {
        assert(i < len);
        return v[i];
    }

    void Append(LVector *from, int start, int amount);
};

struct VMLog
{
    bool uses_frame_state;
    struct LogValue  // FIXME: there's probably a less lazy way to store this.
    {
        Value v;
        ValueType t;
        LogValue(const Value &_v, ValueType _t) : v(_v), t(_t) {}
    };
    vector<LogValue> logread, logwrite;
    size_t logi;
    const int *lognew;

    VM &vm;
    VMLog(VM &_vm);

    void LogInit();
    void LogFrame();
    void LogSkipNestedFuns();
    size_t LogFunctionEntry(const int *funstart, int nlogvars);
    void LogFunctionExit(const int *funstart, const int *logvars, size_t logfunwritestart);
    Value LogGet(Value def, int idx, bool isref);
    void LogCleanup();
    void LogMark();
};

struct StackFrame
{
    const int *retip;
    const int *funstart;
    int definedfunction;
    int spstart;
    int tempmask;
    size_t logfunwritestart;
    size_t logfunreadstart;
};

struct VM
{
    Value *stack;
    int stacksize;
    int maxstacksize;
    int sp;

    const int *ip;

    vector<StackFrame> stackframes;

    CoRoutine *curcoroutine;

    Value *vars;
    
    size_t codelen;
    const int *codestart;
    vector<int> codebigendian;
    vector<type_elem_t> typetablebigendian;
    uint64_t *byteprofilecounts;
    
    vector<uchar> bytecode_buffer;
    const bytecode::BytecodeFile *bcf;

    PrintPrefs programprintprefs;
    const type_elem_t *typetable;
    string evalret;

    int currentline;
    int maxsp;

    PrintPrefs debugpp;

    string programname;

    VMLog vml;

    bool trace;
    bool trace_tail;
    string trace_output;

    int64_t vm_count_ins;
    int64_t vm_count_fcalls;
    int64_t vm_count_bcalls;

    typedef void (VM::* f_ins_pointer)();
    f_ins_pointer f_ins_pointers[IL_MAX_OPS];

    const void *compiled_code_ip;

    VM(const char *_pn, vector<uchar> &&_bytecode_buffer, const void *entry_point, const void *static_bytecode);
    ~VM();

    void OneMoreFrame();

    const TypeInfo &GetTypeInfo(type_elem_t offset) { return *(TypeInfo *)(typetable + offset); }
    const TypeInfo &GetVarTypeInfo(int varidx);

    void SetMaxStack(int ms) { maxstacksize = ms; }
    const char *GetProgramName() { return programname.c_str(); }

    const TypeInfo *GetIntVectorType(int which);
    const TypeInfo *GetFloatVectorType(int which);

    void DumpLeaks();
    
    ElemObj *NewVector(int initial, int max, const TypeInfo &ti);
    CoRoutine *NewCoRoutine(const int *rip, const int *vip, CoRoutine *p, const TypeInfo &cti);
    BoxedInt *NewInt(int i);
    BoxedFloat *NewFloat(float f);
    LString *NewString(size_t l);
    LString *NewString(const char *c, size_t l);
    LString *NewString(const string &s);
    LString *NewString(const char *c1, size_t l1, const char *c2, size_t l2);

    Value Error(string err, const RefObj *a = nullptr, const RefObj *b = nullptr);
    Value BuiltinError(string err) { return Error(err); }
    void VMAssert(bool ok, const char *what);
    void VMAssert(bool ok, const char *what, const RefObj *a, const RefObj *b);

    string ValueDBG(const RefObj *a);
    string DumpVar(const Value &x, size_t idx, bool dumpglobals);

    void EvalMulti(int nargs, const int *mip, int definedfunction, const int *retip, int tempmask);

    void FinalStackVarsCleanup();
    
    int CallerId();

    int VarCleanup(string *error, int towhere);
    void FunIntro(int nargs_given, const int *newip, int definedfunction, const int *retip, int tempmask);
    bool FunOut(int towhere, int nrv);

    void CoVarCleanup(CoRoutine *co);
    void CoNonRec(const int *varip);
    void CoNew();
    void CoDone(const int *retip);
    void CoClean();
    void CoYield(const int *retip);
    void CoResume(CoRoutine *co);

    void EndEval(Value &ret, ValueType vt);

    #ifdef VM_COMPILED_CODE_MODE
        #define VM_OP_ARGS const int *ip
        #define VM_JUMP_RET bool
    #else
        #define VM_OP_ARGS
        #define VM_JUMP_RET void
    #endif

    #define F(N, A) void F_##N(VM_OP_ARGS);
        ILBASENAMES
    #undef F
    #define F(N, A) VM_JUMP_RET F_##N();
        ILJUMPNAMES
    #undef F

    void EvalProgram();

    void PushDerefField(int i);
    void PushDerefIdx(int i);
    void LvalueObj(int lvalop, int i);
    void LvalueOp(int op, Value &a);

    string ProperTypeName(const TypeInfo &ti);

    void Div0() { Error("division by zero"); } 
    void IDXErr(int i, int n, const RefObj *v);
    void BCallRetCheck(const void *nf);
    int GrabIndex(const Value &idx);
    int VectorLoop(const Value &a, const Value &b, Value &res, bool withscalar, const TypeInfo &desttype);

    void Push(const Value &v);
    Value Pop();

    string StructName(const TypeInfo &ti);
    const char *ReverseLookupType(uint v);
    void Trace(bool on) { trace = on; }
    double Time() { return SecondsSinceStart(); }

    int GC();
};

template<int N> inline vec<float,N> ValueToF(const Value &v, float def = 0)
{
    vec<float,N> t;
    for (int i = 0; i < N; i++) t.set(i, v.eval()->Len() > i ? v.eval()->At(i).fval() : def);
    return t;
}

template<int N> inline vec<int, N> ValueToI(const Value &v, int def = 0)
{
    vec<int, N> t;
    for (int i = 0; i < N; i++) t.set(i, v.eval()->Len() > i ? v.eval()->At(i).ival() : def);
    return t;
}

template<int N> inline vec<float,N> ValueDecToF(const Value &v, float def = 0)
{
    auto r = ValueToF<N>(v, def);
    v.DECRT();
    return r;
}

template<int N> inline vec<int, N> ValueDecToI(const Value &v, int def = 0)
{
    auto r = ValueToI<N>(v, def);
    v.DECRT();
    return r;
}

template <int N> inline Value ToValueI(const vec<int, N> &vec, int maxelems = 4)
{
    auto numelems = min(maxelems, N);
    auto v = g_vm->NewVector(numelems, numelems, *g_vm->GetIntVectorType(numelems));
    for (int i = 0; i < numelems; i++) v->At(i) = Value(vec[i]);
    return Value(v);
}

template <int N> inline Value ToValueF(const vec<float, N> &vec, int maxelems = 4)
{
    auto numelems = min(maxelems, N);
    auto v = g_vm->NewVector(numelems, numelems, *g_vm->GetFloatVectorType(numelems));
    for (int i = 0; i < numelems; i++) v->At(i) = Value(vec[i]);
    return Value(v);
}

inline int RangeCheck(const Value &idx, int range, int bias = 0)
{
    int i = idx.ival();
    if (i < bias || i >= bias + range)
        g_vm->BuiltinError("index out of range [" + to_string(bias) + ".." + to_string(bias + range) + "): " + 
            to_string(i));
    return i;
}

struct CoRoutine : RefObj
{
    bool active;        // goes to false when it has hit the end of the coroutine instead of a yield

    int stackstart;     // when currently running, otherwise -1
    Value *stackcopy;
    int stackcopylen, stackcopymax;

    int stackframestart;  // when currently running, otherwise -1
    StackFrame *stackframescopy;
    int stackframecopylen, stackframecopymax;
    int top_at_suspend;

    const int *returnip;
    const int *varip;
    CoRoutine *parent;

    CoRoutine(int _ss, int _sfs, const int *_rip, const int *_vip, CoRoutine *_p, const TypeInfo &cti)
        : RefObj(cti), active(true),
          stackstart(_ss), stackcopy(nullptr), stackcopylen(0), stackcopymax(0),
          stackframestart(_sfs), stackframescopy(nullptr), stackframecopylen(0), stackframecopymax(0),
          top_at_suspend(-1),
          returnip(_rip), varip(_vip), parent(_p) {}

    Value &Current()
    {
        if (stackstart >= 0) g_vm->BuiltinError("cannot get value of active coroutine");
        return stackcopy[stackcopylen - 1].INCTYPE(g_vm->GetTypeInfo(ti.yieldtype).t);
    }

    void Resize(int newlen)
    {
        if (newlen > stackcopymax)
        {
            if (stackcopy) DeallocSubBuf(stackcopy, stackcopymax);
            stackcopy = AllocSubBuf<Value>(stackcopymax = newlen, g_vm->GetTypeInfo(TYPE_ELEM_VALUEBUF));
        }
        stackcopylen = newlen;
    }

    void ResizeFrames(int newlen)
    {
        if (newlen > stackframecopymax)
        {
            if (stackframescopy) DeallocSubBuf(stackframescopy, stackframecopymax);
            stackframescopy = AllocSubBuf<StackFrame>(stackframecopymax = newlen,
                                                      g_vm->GetTypeInfo(TYPE_ELEM_STACKFRAMEBUF));
        }
        stackframecopylen = newlen;
    }

    int Suspend(int top, Value *stack, vector<StackFrame> &stackframes, const int *&rip, CoRoutine *&curco)
    {
        assert(stackstart >= 0);

        swap(rip, returnip);

        assert(curco == this);
        curco = parent;
        parent = nullptr;

        ResizeFrames((int)stackframes.size() - stackframestart);
        memcpy(stackframescopy, stackframes.data() + stackframestart, stackframecopylen * sizeof(StackFrame));
        stackframes.erase(stackframes.begin() + stackframestart, stackframes.end());
        stackframestart = -1;

        top_at_suspend = top;

        Resize(top - stackstart);
        memcpy(stackcopy, stack + stackstart, stackcopylen * sizeof(Value));
        int ss = stackstart;
        stackstart = -1;
        return ss;
    }

    void AdjustStackFrames(int top)
    {
        int topdelta = (top + stackcopylen) - top_at_suspend;
        if (topdelta)
        {
            for (int i = 0; i < stackframecopylen; i++)
            {
                stackframescopy[i].spstart += topdelta;
            }
        }
    }

    int Resume(int top, Value *stack, vector<StackFrame> &stackframes, const int *&rip, CoRoutine *p)
    {
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

    void BackupParentVars(Value *vars)
    {
        // stored here while coro is active
        Resize(*varip);
        for (int i = 1; i <= *varip; i++)
        {
            auto &var = vars[varip[i]];
            // we don't INC, since parent var is still on the stack and will hold ref
            stackcopy[i - 1] = var;
        }
    }

    Value &AccessVar(int savedvaridx)
    {
        assert(stackstart < 0);
        // variables are always saved on top of the stack before the stackcopy gets made, so they are
        // last, followed by the retval (thus -1).
        return stackcopy[stackcopylen - *varip + savedvaridx - 1];
    }

    Value &GetVar(int ididx)
    {
        if (stackstart >= 0)
            g_vm->BuiltinError("cannot access locals of running coroutine");

        // FIXME: we can probably make it work without this search, but for now no big deal
        for (int i = 1; i <= *varip; i++)
        {
            if (varip[i] == ididx)
            {
                return AccessVar(i - 1);
            }
        }
        // this one should be really rare, since parser already only allows lexically contained vars for that function,
        // could happen when accessing var that's not in the callchain of yields
        g_vm->BuiltinError("local variable being accessed is not part of coroutine state");
        return *stackcopy;
    }

    void DeleteSelf(bool deref)
    {
        assert(stackstart < 0);
        if (stackcopy)
        {
            stackcopy[--stackcopylen].DECTYPE(g_vm->GetTypeInfo(ti.yieldtype).t);
            if (active)
            {
                if (deref)
                {
                    for (int i = *varip; i > 0; i--)
                    {
                        auto &vti = g_vm->GetVarTypeInfo(varip[i]);
                        stackcopy[--stackcopylen].DECTYPE(vti.t);
                    }

                    top_at_suspend -= *varip + 1;

                    // This calls Resume() to get the rest back onto the stack, then unwinds it.
                    g_vm->CoVarCleanup(this);
                }
            }
            else
            {
               assert(!stackcopylen);
            }
            DeallocSubBuf(stackcopy, stackcopymax);
        }

        if (stackframescopy) DeallocSubBuf(stackframescopy, stackframecopymax);
        vmpool->dealloc(this, sizeof(CoRoutine));
    }

    ValueType ElemType(int i)
    {
        assert(i < *varip);
        auto varidx = varip[i + 1];
        auto &vti = g_vm->GetVarTypeInfo(varidx);
        auto vt = vti.t;
        if (vt == V_NIL) vt = g_vm->GetTypeInfo(vti.subt).t;
        #if RTT_ENABLED
        auto &var = AccessVar(i);
        if(vt != var.type && var.type != V_NIL && !(vt == V_VECTOR && var.type == V_STRUCT))  // FIXME: for testing
        {
            Output(OUTPUT_INFO, "coro elem %s != %s", vti.Debug().c_str(), BaseTypeName(var.type));
            assert(false);
        }
        #endif
        return vt;
    }

    void Mark()
    {
        // FIXME!
        // ElemType(i) refers to the ith variable, not the ith stackcopy element.
        /*
        if (stackstart < 0)
            for (int i = 0; i < stackcopylen; i++)
                stackcopy[i].Mark(?);
        */
        g_vm->BuiltinError("internal: can\'t GC coroutines");
    }
};



}  // namespace lobster
