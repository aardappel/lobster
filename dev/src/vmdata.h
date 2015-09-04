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

namespace lobster {

#define RTT_ENABLED 1

enum ValueType
{
    // refc types are negative
    V_MINVMTYPES = -10,
    V_ANY = -9,         // [typechecker only] any other reference type.
    V_CYCLEDONE = -8,
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
    // used in function calling, if they appear as a value in a program, that's a bug
    V_RETIP, V_FUNSTART, V_NARGS, V_DEFFUN,
    V_LOGSTART, V_LOGEND, V_LOGMARKER, V_LOGFUNWRITESTART, V_LOGFUNREADSTART,
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
        "any", "<cycle>", "<value_buffer>", "boxed_float", "boxed_int", "coroutine", "string", "struct", "vector", 
        "nil", "int", "float", "function", "yield_function", "variable", "typeid",
        "<retip>", "<funstart>", "<nargs>", "<deffun>", 
        "<logstart>", "<logend>", "<logmarker>", "<logfunwritestart>", "<logfunreadstart>"
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
    TYPE_ELEM_COROUTINE,
    TYPE_ELEM_ANY,
    TYPE_ELEM_CYCLEDONE,
    TYPE_ELEM_VALUEBUF,
    TYPE_ELEM_VECTOR_OF_INT = 9,   // 2 each.
    TYPE_ELEM_VECTOR_OF_FLOAT = 11,
    TYPE_ELEM_VECTOR_OF_STRING = 13,

    TYPE_ELEM_FIXED_OFFSET_END = 15
};

struct TypeInfo
{
    type_elem_t t;
    type_elem_t sub;
    type_elem_t elems[1];

    ValueType vt() const { return (ValueType)t; }

    TypeInfo() = delete;
    TypeInfo(const TypeInfo &) = delete;

    string Debug(bool rec = true) const;
};

struct Value;
struct LString;
struct LVector;
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

struct VMBase
{
    PrintPrefs programprintprefs;

    VMBase() : programprintprefs(10, 10000, false, -1, false) {}

    //virtual Value EvalC(Value &cl, int nargs) = 0;
    virtual Value BuiltinError(string err) = 0;
    virtual void Push(const Value &v) = 0;
    virtual Value Pop() = 0;
    virtual LString *NewString(const string &s) = 0;
    virtual LString *NewString(const char *c, size_t l) = 0;
    virtual LVector *NewVector(size_t n, type_elem_t t) = 0;
    virtual type_elem_t GetIntVectorType(int which) = 0;
    virtual type_elem_t GetFloatVectorType(int which) = 0;
    virtual void Trace(bool on) = 0;
    virtual double Time() = 0;
    virtual int GC() = 0;
    virtual const char *ProperTypeName(const Value &v) = 0;
    virtual int StructTypeInfo(type_elem_t idx, string &name) = 0;
    virtual const char *ReverseLookupType(uint v) = 0;
    virtual void SetMaxStack(int ms) = 0;
    virtual void CoResume(CoRoutine *co) = 0;
    virtual int CallerId() = 0;
    virtual const char *GetProgramName() = 0;
    virtual void LogFrame() = 0;
    virtual const TypeInfo &GetTypeInfo(type_elem_t offset) = 0;
    virtual const TypeInfo &GetVarTypeInfo(int varidx) = 0;
};

// the 2 globals that make up the current VM instance
extern VMBase *g_vm;
extern SlabAlloc *vmpool;

struct DynAlloc     // ANY memory allocated by the VM must inherit from this, so we can identify leaked memory
{
    type_elem_t typeoff;    // offset into the VM's typetable

    DynAlloc(type_elem_t _t) : typeoff(_t) { assert(_t >= 0); }

    const TypeInfo &GetTypeInfo() const { return g_vm->GetTypeInfo(typeoff); }
    ValueType BaseType() const { return g_vm->GetTypeInfo(typeoff).vt(); }
};

struct RefObj : DynAlloc
{
    int refc;

    RefObj(type_elem_t _t) : DynAlloc(_t), refc(1) {}

    void Inc()
    {
        refc++;
        //Output(OUTPUT_INFO, "INC to %d for %s", refc, ToString(g_vm->programprintprefs).c_str());
    }

    void Dec()
    {
        refc--;
        //Output(OUTPUT_INFO, "DEC to %d for %s", refc, ToString(g_vm->programprintprefs).c_str());
        if (refc <= 0) DECDELETE(true);
    }

    void CycleDone(int &cycles)
    {
        typeoff = TYPE_ELEM_CYCLEDONE;
        refc = cycles++;
    }

    string CycleStr() const { return "_" + to_string(refc) + "_"; }

    void DECDELETE(bool deref);
    bool Equal(const RefObj *o, bool structural) const;
    string ToString(PrintPrefs &pp) const;
    void Mark();
};

struct BoxedInt : RefObj
{
    int val;

    BoxedInt(int _v) : RefObj(TYPE_ELEM_BOXEDINT), val(_v) {}
};

struct BoxedFloat : RefObj
{
    float val;

    BoxedFloat(float _v) : RefObj(TYPE_ELEM_BOXEDFLOAT), val(_v) {}
};

struct LenObj : RefObj
{
    int len;    // has to match the Value integer type, since we allow the length to be obtained

    LenObj(type_elem_t _t, int _l) : RefObj(_t), len(_l) {}
};

struct LString : LenObj
{
    LString(int _l) : LenObj(TYPE_ELEM_STRING, _l) {}

    char *str() { return (char *)(this + 1); }

    string ToString(PrintPrefs &pp)
    {
        if (pp.cycles >= 0)
        {
            if (typeoff == TYPE_ELEM_CYCLEDONE) return CycleStr(); 
            CycleDone(pp.cycles);
        }
        string s = len > pp.budget ? string(str()).substr(0, pp.budget) + ".." : str();
        if (pp.quoted)
        {
            string r = "\"";
            for (size_t i = 0; i < s.length(); i++) switch(s[i])
            {
                case '\n': r += "\\n"; break;
                case '\t': r += "\\t"; break;
                case '\r': r += "\\r"; break;
                case '\\': r += "\\\\"; break;
                case '\"': r += "\\\""; break;
                case '\'': r += "\\\'"; break;
                default:
                    if (s[i] >= ' ' && s[i] <= '~') r += s[i];
                    else { 
                        r += "\\x"; r += HexChar(((uchar)s[i]) >> 4); r += HexChar(s[i] & 0xF); }
                    break;
            }

            r += "\"";
            return r; 
        }
        else
        {
            return s;
        }
    }

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
#define TYPE_ASSERT(cond) if(!(cond)) { g_vm->BuiltinError("type verification failed: " #cond); }
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
        int ival_;       // keep this 32bit even on 64bit for predictable results
        float fval_;     // idem, also the type that most graphics hardware works with natively
        const int *ip_;  // Never gets converted to any, so no boxed version available.

        // Reference values (includes NULL if nillable version).
        LString *sval_;
        LVector *vval_;
        CoRoutine *cval_;

        // Boxed scalars (never NULL)
        BoxedInt *bival_;
        BoxedFloat *bfval_;

        // Generic reference access.
        LenObj *lobj_;
        RefObj *ref_;
    };
    public:

    // These asserts help track down any invalid code generation issues.
    int         ival  () const { TYPE_ASSERT(type == V_INT);                return ival_;  }
    int        &ival  ()       { TYPE_ASSERT(type == V_INT);                return ival_;  }
    float       fval  () const { TYPE_ASSERT(type == V_FLOAT);              return fval_;  }
    float      &fval  ()       { TYPE_ASSERT(type == V_FLOAT);              return fval_;  }
    LString    *sval  () const { TYPE_ASSERT(type == V_STRING);             return sval_;  }
    BoxedInt   *bival () const { TYPE_ASSERT(type == V_BOXEDINT);           return bival_; }
    BoxedFloat *bfval () const { TYPE_ASSERT(type == V_BOXEDFLOAT);         return bfval_; }
    LVector    *vval  () const { TYPE_ASSERT(IsVector(type));               return vval_;  }
    CoRoutine  *cval  () const { TYPE_ASSERT(type == V_COROUTINE);          return cval_;  }
    LenObj     *lobj  () const { TYPE_ASSERT(IsRef(type));                  return lobj_;  }
    RefObj     *ref   () const { TYPE_ASSERT(IsRef(type));                  return ref_;   }
    RefObj     *refnil() const { TYPE_ASSERT(IsRef(type) || type == V_NIL); return ref_;   }
    const int  *ip    () const { TYPE_ASSERT(type >= V_FUNCTION);           return ip_;    }
    int         info  () const { TYPE_ASSERT(type >= V_NARGS);              return ival_;  }
    void       *any   () const { return ref_; }
                                                                       
    inline Value()                          : TYPE_INIT(V_NIL)         ref_(nullptr) {}
    inline Value(int i)                     : TYPE_INIT(V_INT)         ival_(i)      {}
    inline Value(int i, ValueType t)        : TYPE_INIT(t)             ival_(i)      { (void)t; }
    inline Value(bool b)                    : TYPE_INIT(V_INT)         ival_(b)      {}
    inline Value(float f)                   : TYPE_INIT(V_FLOAT)       fval_(f)      {}
    inline Value(const int *i)              : TYPE_INIT(V_FUNCTION)    ip_(i)        {}
    inline Value(const int *i, ValueType t) : TYPE_INIT(t)             ip_(i)        { (void)t; }
    inline Value(RefObj *r)                 : TYPE_INIT(r->BaseType()) ref_(r)       {}


    inline bool True() const { return ival_ != 0; } // FIXME: not safe on 64bit systems unless we make ival 64bit also
                                                    // esp big endian.

    inline Value &INCRT()
    {
        TYPE_ASSERT(IsRef(type));
        ref_->Inc();
        return *this;
    }

    inline Value &INCRTNIL() { if (ref_) INCRT(); return *this; }
        
    inline Value &INC()
    {
        return IsRef(type) ? INCRT() : *this;
    }

    inline void DECRT() const   // we already know its a ref type
    {
        TYPE_ASSERT(IsRef(type));
        ref_->Dec();
    }

    inline void DECRTNIL() const { if (ref_) DECRT(); }

    inline const Value &DEC() const
    {
        if (IsRef(type)) DECRT();
        return *this;
    }


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

struct ValueRef
{
    const Value &v;

    ValueRef(const Value &_v) : v(_v) {}
    ~ValueRef() { v.DEC(); }
};

inline Value *AllocSubBuf(size_t size)
{
    auto mem = (void **)vmpool->alloc(size * sizeof(Value) + sizeof(void *));
    *((type_elem_t *)mem) = TYPE_ELEM_VALUEBUF;    // DynAlloc header, padded to pointer size if needed
    mem++;
    return (Value *)mem;
}

inline void DeallocSubBuf(Value *v, size_t size)
{
    auto mem = (void **)v;
    mem--;
    vmpool->dealloc(mem, size * sizeof(Value) + sizeof(void *));
}

struct LVector : LenObj
{
    private:
    Value *v;   // use at()
    
    public:
    int maxl;
    int initiallen;

    LVector(int _size, type_elem_t _t) : LenObj(_t, 0), maxl(_size), initiallen(_size)
    {
        v = (Value *)(this + 1);
    }

    ~LVector() { assert(0); }   // destructed by DECREF

    void DeallocBuf()
    {
        if (v == (Value *)(this + 1)) return;
        DeallocSubBuf(v, maxl);
    }

    void DeleteSelf(bool deref)
    {
        if (deref) DeRef();
        DeallocBuf();
        vmpool->dealloc(this, sizeof(LVector) + sizeof(Value) * initiallen);
    }

    void Resize(int newmax)
    {
        // FIXME: check overflow
        auto mem = AllocSubBuf(newmax);
        if (len) memcpy(mem, v, sizeof(Value) * len);
        DeallocBuf();
        maxl = newmax;
        v = (Value *)mem;
    }

    void Push(const Value &val)
    {
        if (len == maxl) Resize(maxl ? maxl * 2 : 4);
        v[len++] = val;
        ElemType(len - 1);  // FIXME: just for testing, triggers type check
    }

    Value Pop()
    {
        return v[--len];
    }

    Value &Top() const
    {
        if (IsRef(ElemType(len - 1))) v[len - 1].INCRT();
        return v[len - 1];
    }
    
    void Insert(Value &val, int i, int n)
    {
        assert(n > 0 && i >= 0 && i <= len); // note: insertion right at the end is legal, hence <= 
        if (len + n > maxl) Resize(max(len + n, maxl ? maxl * 2 : 4));   
        memmove(v + i + n, v + i, sizeof(Value) * (len - i));
        len++;
        for (int j = 0; j < n; j++) { v[i + j] = val; val.INC(); }
        val.DEC();
    }

    Value Remove(int i, int n)
    { 
        assert(n >= 0 && n <= len && i >= 0 && i <= len - n);
        auto x = v[i];
        for (int j = 1; j < n; j++) v[i + j].DEC();
        memmove(v + i, v + i + n, sizeof(Value) * (len - i - n));
        len -= n;
        return x;
    }

    Value &at(int i) const
    {
        assert(i < len);
        return v[i];
    }

    void Append(LVector *from, int start, int amount)
    {
        if (len + amount > maxl) Resize(len + amount);  // FIXME: check overflow
        memcpy(v + len, from->v + start, sizeof(Value) * amount);
        for (int i = 0; i < amount; i++) v[len + i].INC();
        len += amount;
    }

    ValueType ElemType(int i) const
    {
        auto &ti = GetTypeInfo();
        auto &sti = g_vm->GetTypeInfo(ti.vt() == V_VECTOR ? ti.sub : ti.elems[i]);
        auto vt = sti.vt();
        if (vt == V_NIL) vt = g_vm->GetTypeInfo(sti.sub).vt();
        #if RTT_ENABLED
        if(vt != v[i].type && v[i].type != V_NIL && !(vt == V_VECTOR && v[i].type == V_STRUCT))  // FIXME: for testing
        {
            Output(OUTPUT_INFO, "elemtype of %s != %s", ti.Debug().c_str(), BaseTypeName(v[i].type));
            assert(false);
        }
        #endif
        return vt;
    }

    string ToString(PrintPrefs &pp)
    {
        if (pp.cycles >= 0)
        {
            if (typeoff == TYPE_ELEM_CYCLEDONE) return CycleStr(); 
            CycleDone(pp.cycles);
        }

        auto &ti = GetTypeInfo();
        string s = ti.vt() == V_STRUCT ? g_vm->ReverseLookupType(ti.sub) + string("{") : "[";
        for (int i = 0; i < len; i++)
        {
            if (i) s += ", ";
            if ((int)s.size() > pp.budget) { s += "...."; break; }
            PrintPrefs subpp(pp.depth - 1, pp.budget - (int)s.size(), true, pp.decimals, pp.anymark);
            s += pp.depth || !IsRef(ElemType(i)) ? v[i].ToString(ElemType(i), subpp) : "..";
        }
        s += ti.vt() == V_STRUCT ? "}" : "]";
        return s;
    }

    bool Equal(const LVector &o)
    {
        if (len != o.len) return false;
        for (int i = 0; i < len; i++)
            if (!v[i].Equal(ElemType(i), o.v[i], o.ElemType(i), true))
                return false;
        return true;
    }

    void DeRef()
    {
        for (int i = 0; i < len; i++)
            if (IsRef(ElemType(i)))
                v[i].DECRTNIL();
    }

    void Mark()
    {
        for (int i = 0; i < len; i++)
            v[i].Mark(ElemType(i));
    }
};

struct CoRoutine : RefObj
{
    bool active;        // goes to false when it has hit the end of the coroutine instead of a yield
    int stackstart;     // when currently running, otherwise -1
    Value *stackcopy;
    int stackcopylen, stackcopymax;
    const int *returnip;
    const int *varip;
    CoRoutine *parent;

    CoRoutine(int _ss, const int *_rip, const int *_vip, CoRoutine *_p)
        : RefObj(TYPE_ELEM_COROUTINE), active(true), stackstart(_ss), stackcopy(nullptr), stackcopylen(0), stackcopymax(0),
          returnip(_rip), varip(_vip), parent(_p) {}

    Value &Current()
    {
        if (stackstart >= 0) g_vm->BuiltinError("cannot get value of active coroutine");
        return stackcopy[stackcopylen - 1];
    }

    void Resize(int newlen)
    {
        if (newlen > stackcopymax)
        {
            if (stackcopy) DeallocSubBuf(stackcopy, stackcopymax);
            stackcopy = AllocSubBuf(stackcopymax = newlen);
        }
        stackcopylen = newlen;
    }

    int Suspend(int top, Value *stack, const int *&rip, CoRoutine *&curco)
    {
        assert(stackstart >= 0);

        swap(rip, returnip);

        assert(curco == this);
        curco = parent;
        parent = nullptr;

        int newlen = top - stackstart;
        Resize(newlen);
        memcpy(stackcopy, stack + stackstart, stackcopylen * sizeof(Value));

        int ss = stackstart;
        stackstart = -1;
        return ss;
    }

    int Resume(int top, Value *stack, const int *&rip, CoRoutine *p)
    {
        assert(stackstart < 0);

        swap(rip, returnip);

        assert(!parent);
        parent = p;

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
            if (deref) for (int i = 0; i < stackcopylen; i++) stackcopy[i].DEC();
            DeallocSubBuf(stackcopy, stackcopymax);
        }
        vmpool->dealloc(this, sizeof(CoRoutine));
    }

    ValueType ElemType(int i)
    {
        assert(i < *varip);
        auto varidx = varip[i + 1];
        auto &ti = g_vm->GetVarTypeInfo(varidx);
        auto vt = ti.vt();
        if (vt == V_NIL) vt = g_vm->GetTypeInfo(ti.sub).vt();
        #if RTT_ENABLED
        auto &var = AccessVar(i);
        if(vt != var.type && var.type != V_NIL && !(vt == V_VECTOR && var.type == V_STRUCT))  // FIXME: for testing
        {
            Output(OUTPUT_INFO, "coro elem %s != %s", ti.Debug().c_str(), BaseTypeName(var.type));
            assert(false);
        }
        #endif
        return vt;
    }

    void Mark()
    {
        #if RTT_ENABLED
        if (stackstart < 0)
            for (int i = 0; i < stackcopylen; i++)
                stackcopy[i].Mark(stackcopy[i].type);
        #else
        // FIXME!
        // ElemType(i) refers to the ith variable, not the ith stackcopy element.
        g_vm->BuiltinError("internal: can\'t GC coroutines");
        #endif
    }
};

template<int N> inline vec<float,N> ValueToF(const Value &v, float def = 0)
{
    vec<float,N> t;
    for (int i = 0; i < N; i++) t.set(i, v.vval()->len > i ? v.vval()->at(i).fval() : def);
    return t;
}

template<int N> inline vec<int, N> ValueToI(const Value &v, int def = 0)
{
    vec<int, N> t;
    for (int i = 0; i < N; i++) t.set(i, v.vval()->len > i ? v.vval()->at(i).ival() : def);
    return t;
}

template<int N> inline vec<float,N> ValueDecToF(const Value &v, float def = 0)
{
    auto r = ValueToF<N>(v, def);
    v.DEC();
    return r;
}

template<int N> inline vec<int, N> ValueDecToI(const Value &v, int def = 0)
{
    auto r = ValueToI<N>(v, def);
    v.DEC();
    return r;
}

template <int N> inline Value ToValueI(const vec<int, N> &vec, int maxelems = 4)
{
    auto numelems = min(maxelems, N);
    auto v = g_vm->NewVector(numelems, g_vm->GetIntVectorType(numelems));
    for (int i = 0; i < numelems; i++) v->Push(Value(vec[i]));
    return Value(v);
}

template <int N> inline Value ToValueF(const vec<float, N> &vec, int maxelems = 4)
{
    auto numelems = min(maxelems, N);
    auto v = g_vm->NewVector(numelems, g_vm->GetFloatVectorType(numelems));
    for (int i = 0; i < numelems; i++) v->Push(Value(vec[i]));
    return Value(v);
}

}  // namespace lobster
