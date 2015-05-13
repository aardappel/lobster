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

#define LOG_ENABLED 1

enum ValueType
{
    V_MINVMTYPES = -7,
    V_STRUCT = -6,      // [typechecker only] an alias for V_VECTOR
    V_CYCLEDONE = -5,
    V_VALUEBUF = -4,    // only used as memory type for vector/coro buffers, Value not allowed to refer to this
    V_COROUTINE = -3,
    V_STRING = -2,      // refc types are negative
    V_VECTOR = -1,
    V_INT = 0,          // quickest check for most common type
    V_FLOAT = 1, 
    V_FUNCTION,
    V_YIELD,
    V_NIL,
    V_UNDEFINED,        // used for unitialized values or functions returning "void".
    V_NILABLE,          // [typechecker only] a value that may be nil or a reference type.
    V_ANY,              // [typechecker only] any other type.
    V_VAR,              // [typechecker only] like V_ANY, except idx refers to a type variable
    // used in function calling, if they appear as a value in a program, that's a bug
    V_RETIP, V_FUNSTART, V_NARGS, V_DEFFUN,
    V_LOGSTART, V_LOGEND, V_LOGMARKER, V_LOGFUNWRITESTART, V_LOGFUNREADSTART,
    V_MAXVMTYPES
};

inline const char *BaseTypeName(ValueType t)
{
    static const char *typenames[] =
    {
        "struct", "<cycle>", "<value_buffer>", "coroutine", "string", "vector", 
        "int", "float", "function", "yield_function", "nil", "undefined", "nilable", "any", "variable",
        "<retip>", "<funstart>", "<nargs>", "<deffun>", 
        "<logstart>", "<logend>", "<logmarker>", "<logfunwritestart>", "<logfunreadstart>"
    };
    if (t <= V_MINVMTYPES || t >= V_MAXVMTYPES)
        return "<internal-error-type>";
    return typenames[t - V_MINVMTYPES - 1];
}

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

    PrintPrefs(int _depth, int _budget, bool _quoted, int _decimals)
        : depth(_depth), budget(_budget), quoted(_quoted), decimals(_decimals), cycles(-1) {}
};

struct VMBase
{
    PrintPrefs programprintprefs;

    VMBase() : programprintprefs(10, 10000, false, -1) {}

    //virtual Value EvalC(Value &cl, int nargs) = 0;
    virtual Value BuiltinError(string err) = 0;
    virtual void Push(const Value &v) = 0;
    virtual Value Pop() = 0;
    virtual LString *NewString(const string &s) = 0;
    virtual LString *NewString(const char *c, size_t l) = 0;
    virtual LVector *NewVector(size_t n, int t) = 0;
    virtual int GetVectorType(int which) = 0;
    virtual void Trace(bool on) = 0;
    virtual float Time() = 0;
    virtual int GC() = 0;
    virtual const char *ProperTypeName(const Value &v) = 0;
    virtual int StructIdx(const string &name, size_t &nargs) = 0;
    virtual const string &ReverseLookupType(uint v) = 0;
    virtual void SetMaxStack(int ms) = 0;
    virtual void CoResume(CoRoutine *co) = 0;
    virtual int CallerId() = 0;
    virtual const char *GetProgramName() = 0;
    virtual void LogFrame() = 0;
};

// the 2 globals that make up the current VM instance
extern VMBase *g_vm;
extern SlabAlloc *vmpool;

struct DynAlloc     // ANY memory allocated by the VM must inherit from this, so we can identify leaked memory
{
    int type;       // 0.. for typed vectors (can't grow), ValueType if negative

    DynAlloc(int _t) : type(_t) {}
};

struct RefObj : DynAlloc
{
    int refc;

    RefObj(int _t) : DynAlloc(_t), refc(1) {}

    void CycleDone(int &cycles)
    {
        type = V_CYCLEDONE;
        refc = cycles++;
    }

    string CycleStr() { return "_" + to_string(refc) + "_"; }
};

struct LenObj : RefObj
{
    int len;    // has to match the Value integer type, since we allow the length to be obtained

    LenObj(int _t, int _l) : RefObj(_t), len(_l) {}
};

struct LString : LenObj
{
    LString(int _l) : LenObj(V_STRING, _l) {}

    char *str() { return (char *)(this + 1); }

    string ToString(PrintPrefs &pp)
    {
        if (pp.cycles >= 0)
        {
            if (type == V_CYCLEDONE) return CycleStr(); 
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

    void Mark() { if (refc > 0) refc = -refc; }

    void deleteself() { vmpool->dealloc(this, sizeof(LString) + len + 1); }

    bool operator==(LString &o) { return strcmp(str(), o.str()) == 0; }
    bool operator!=(LString &o) { return strcmp(str(), o.str()) != 0; }
    bool operator< (LString &o) { return strcmp(str(), o.str()) <  0; }
    bool operator<=(LString &o) { return strcmp(str(), o.str()) <= 0; }
    bool operator> (LString &o) { return strcmp(str(), o.str()) >  0; }
    bool operator>=(LString &o) { return strcmp(str(), o.str()) >= 0; }
};

struct Value
{
    ValueType type;     // FIXME: on 64bit system this should really sit in a separate array

    private:
    union
    {
        int ival_;       // keep this 32bit even on 64bit for predictable results
        float fval_;     // idem, also the type that most graphics hardware works with natively
        LString *sval_;
        LVector *vval_;
        CoRoutine *cval_;
        LenObj *lobj_;
        RefObj *ref_;
        int *ip_;
    };
    public:

    // These asserts help track down any invalid code generation issues.
    int        ival() const { assert(type == V_INT);        return ival_; }
    int       &ival()       { assert(type == V_INT);        return ival_; }
    float      fval() const { assert(type == V_FLOAT);      return fval_; }
    float     &fval()       { assert(type == V_FLOAT);      return fval_; }
    LString   *sval() const { assert(type == V_STRING);     return sval_; }
    LVector   *vval() const { assert(type == V_VECTOR);     return vval_; }
    CoRoutine *cval() const { assert(type == V_COROUTINE);  return cval_; }
    LenObj    *lobj() const { assert(type < 0);             return lobj_; }
    RefObj    *ref () const { assert(type < 0);             return ref_;  }
    int       *ip()   const { assert(type >= V_FUNCTION);   return ip_;   }
    int        info() const { assert(type >= V_NARGS);      return ival_; }
    void      *any()  const { return ref_; }

    inline Value()                    : type(V_UNDEFINED), ival_(0) {}
    inline Value(int i)               : type(V_INT),       ival_(i) {}
    inline Value(int i, ValueType t)  : type(t),           ival_(i) {}
    inline Value(bool b)              : type(V_INT),       ival_(b) {}
    inline Value(float f)             : type(V_FLOAT),     fval_(f) {}
    inline Value(LString *s)          : type(V_STRING),    sval_(s) {}
    inline Value(int *i)              : type(V_FUNCTION),  ip_(i)   {}
    inline Value(int *i, ValueType t) : type(t),           ip_(i)   {}
    inline Value(LVector *v)          : type(V_VECTOR),    vval_(v) {}
    inline Value(CoRoutine *c)        : type(V_COROUTINE), cval_(c) {}
    inline Value(RefObj *r)           : type(r->type >= 0 ? V_VECTOR : (ValueType)r->type), ref_(r) {}

    inline bool True() const { return ival_ != 0; } // FIXME: not safe on 64bit systems unless we make ival 64bit also
                                                    // esp big endian.

    inline Value &INC()
    {
        if (type < 0)
        {
            #ifdef _DEBUG
            if (ref_->refc > 0)  // force too many dec bugs to become apparent
            #endif
            ref_->refc++;
        }
        return *this;
    }

    inline void INCN(int n) { if (type < 0) ref_->refc += n; }
    
    inline const Value &DEC() const
    {
        if (type < 0) DECRT();
        return *this;
    }

    inline void DECRT() const   // we already know its a ref type
    {
        ref_->refc--;
        if (ref_->refc <= 0) DECDELETE();
    }

    int Nargs()
    {
        assert(type == V_FUNCTION);
        //assert(*ip == IL_FUNSTART);
        return ip_[1];
    }


    void DECDELETE() const;

    bool Equal(const Value &o, bool structural) const;

    string ToString(PrintPrefs &pp) const;
    void Mark();
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
    *((int *)mem) = V_VALUEBUF;    // DynAlloc header, padded to pointer size if needed
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

    LVector(int _size, int _t) : LenObj(_t, 0), maxl(_size), initiallen(_size)
    {
        v = (Value *)(this + 1);
    }

    ~LVector() { assert(0); }   // destructed by DECREF

    void deallocbuf()
    {
        if (v == (Value *)(this + 1)) return;
        DeallocSubBuf(v, maxl);
    }

    void deleteself()
    {
        DeRef();
        deallocbuf();
        vmpool->dealloc(this, sizeof(LVector) + sizeof(Value) * initiallen);
    }

    void resize(int newmax)
    {
        // FIXME: check overflow
        auto mem = AllocSubBuf(newmax);
        if (len) memcpy(mem, v, sizeof(Value) * len);
        deallocbuf();
        maxl = newmax;
        v = (Value *)mem;
    }

    void push(const Value &val)
    {
        if (len == maxl) resize(maxl ? maxl * 2 : 4);
        v[len++] = val;
    }

    Value pop()
    {
        return v[--len];
    }

    Value &top() const
    {
        return v[len - 1];
    }
    
    void insert(Value &val, int i, int n)
    {
        assert(n > 0 && i >= 0 && i <= len); // note: insertion right at the end is legal, hence <= 
        if (len + n > maxl) resize(max(len + n, maxl ? maxl * 2 : 4));   
        memmove(v + i + n, v + i, sizeof(Value) * (len - i));
        len++;
        for (int j = 0; j < n; j++) v[i + j] = val;
        val.INCN(n - 1);
    }

    Value remove(int i, int n)
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

    void append(LVector *from, int start, int amount)
    {
        if (len + amount > maxl) resize(len + amount);  // FIXME: check overflow
        memcpy(v + len, from->v + start, sizeof(Value) * amount);
        for (int i = 0; i < amount; i++) v[len + i].INC();
        len += amount;
    }

    string ToString(PrintPrefs &pp)
    {
        if (pp.cycles >= 0)
        {
            if (type == V_CYCLEDONE) return CycleStr(); 
            CycleDone(pp.cycles);
        }

        string s = type >= 0 ? g_vm->ReverseLookupType(type) + "{" : "[";
        for (int i = 0; i < len; i++)
        {
            if (i) s += ", ";
            if ((int)s.size() > pp.budget) { s += "...."; break; }
            PrintPrefs subpp(pp.depth - 1, pp.budget - (int)s.size(), true, pp.decimals);
            s += pp.depth || v[i].type >= 0 ? v[i].ToString(subpp) : "..";
        }
        s += type >= 0 ? "}" : "]";
        return s;
    }

    bool Equal(LVector &o)
    {
        for (int i = 0; i < len; i++)
            if (!v[i].Equal(o.v[i], true))
                return false;
        return true;
    }

    void DeRef()
    {
        for (int i = 0; i < len; i++) v[i].DEC();
    }

    void Mark()
    {
        if (refc < 0) return;
        refc = -refc;
        for (int i = 0; i < len; i++) v[i].Mark();
    }
};

struct CoRoutine : RefObj
{
    bool active;        // goes to false when it has hit the end of the coroutine instead of a yield
    int stackstart;     // when currently running, otherwise -1
    Value *stackcopy;
    size_t stackcopylen, stackcopymax;
    int *returnip;
    int *varip;
    CoRoutine *parent;

    CoRoutine(int _ss, int *_rip, int *_vip, CoRoutine *_p)
        : RefObj(V_COROUTINE), active(true), stackstart(_ss), stackcopy(nullptr), stackcopylen(0), stackcopymax(0),
          returnip(_rip), varip(_vip), parent(_p) {}

    Value &Current()
    {
        if (stackstart >= 0) g_vm->BuiltinError("cannot get value of active coroutine");
        return stackcopy[stackcopylen - 1];
    }

    void Resize(size_t newlen)
    {
        if (newlen > stackcopymax)
        {
            if (stackcopy) DeallocSubBuf(stackcopy, stackcopymax);
            stackcopy = AllocSubBuf(stackcopymax = newlen);
        }
        stackcopylen = newlen;
    }

    int Suspend(int top, Value *stack, int *&rip, CoRoutine *&curco)
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

    int Resume(int top, Value *stack, int *&rip, CoRoutine *p)
    {
        assert(stackstart < 0);

        swap(rip, returnip);

        assert(!parent);
        parent = p;

        stackstart = top;
        // FIXME: assume that it fits, which is not guaranteed with recursive coros
        memcpy(stack + top, stackcopy, stackcopylen * sizeof(Value));
        return (int)stackcopylen;
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

    Value &GetVar(int ididx)
    {
        if (stackstart >= 0)
            g_vm->BuiltinError("cannot access locals of running coroutine");

        // FIXME: we can probably make it work without this search, but for now no big deal
        for (int i = 1; i <= *varip; i++)
        {
            if (varip[i] == ididx)
            {
                // -1 because of i's base, -1 because of retval on top of stack
                return stackcopy[stackcopylen - *varip + i - 1 - 1];
            }
        }
        // this one should be really rare, since parser already only allows lexically contained vars for that function,
        // could happen when accessing var that's not in the callchain of yields
        g_vm->BuiltinError("local variable being accessed is not part of coroutine state");
        return *stackcopy;
    }

    void deleteself(bool deref)
    {
        assert(stackstart < 0);
        if (stackcopy)
        {
            if (deref) for (size_t i = 0; i < stackcopylen; i++) stackcopy[i].DEC();
            DeallocSubBuf(stackcopy, stackcopymax);
        }
        vmpool->dealloc(this, sizeof(CoRoutine));
    }

    void Mark()
    {
        if (refc < 0) return;
        refc = -refc;
        if (stackstart < 0) for (size_t i = 0; i < stackcopylen; i++) stackcopy[i].Mark();
    }
};

template<typename T> inline T ValueTo(const Value &v, float def = 0)
{
    if (v.type == V_VECTOR)
    {
        T t;
        for (int i = 0; i < T::NUM_ELEMENTS; i++)
        {
            float e = def;
            if (v.vval()->len > i)
            {
                Value &c = v.vval()->at(i);
                if      (c.type == V_FLOAT) e = c.fval();
                else if (c.type == V_INT)   e = (float)c.ival();
                else g_vm->BuiltinError(string("non-numeric component in vector: ") + g_vm->ProperTypeName(c));
            }
            t.set(i, e);
        }
        return t;
    }
    else if (v.type == V_FLOAT)
    {
        return T(v.fval());
    }
    else if (v.type == V_INT)
    {
        return T((float)v.ival());
    }
    else
    {
        return T(def);
    }
}

template<typename T> inline T ValueDecTo(const Value &v, float def = 0)
{
    auto r = ValueTo<T>(v, def);
    v.DEC();
    return r;
}

template <typename T> inline Value ToValue(const T &vec)
{
    auto v = g_vm->NewVector(T::NUM_ELEMENTS, g_vm->GetVectorType(T::NUM_ELEMENTS));
    for (auto a : vec) v->push(Value(a));
    return Value(v);
}

}  // namespace lobster
