
struct VM : VMBase
{
    Value *stack;
    int stacksize;
    int sp;

    int framepointer;

    Value *vars;   
    int *codestart;
    SymbolTable &st;

    bool trace;

    PreciseTimer timer;

    LineInfo *lineinfo;
    int *lineip;    // FIXME: can we merge this with ip if we're writing here anyway?
    #ifdef _DEBUG
        int currentline;
    #endif

    PrintPrefs debugpp;

    #define PUSH(v) (stack[++sp] = (v))
    #define TOP() (stack[sp])
    #define POP() (stack[sp--]) // (sp < 0 ? 0/(sp + 1) : stack[sp--])
    #define TOPPTR() (stack + sp + 1)
    #define OVERWRITE(o, n) TTOverwrite(o, n)

    Value Error(string err, const Value &a = Value(0, V_MAXVMTYPES), const Value &b = Value(0, V_MAXVMTYPES))
    {
        LineInfo &li = lineinfo[lineip - codestart];
        auto s = string(st.filenames[li.fileidx]) + "(" + inttoa(li.line) + "): VM error: " + err;
        if (a.type != V_MAXVMTYPES) s += "\n   arg: " + ValueDBG(a);
        if (b.type != V_MAXVMTYPES) s += "\n   arg: " + ValueDBG(b);
        for (int i = sp; i >= 0 && sp - i < 3; i--) if (stack[i].type != V_UNDEFINED)
        {
            s += "\n   stack: " + ValueDBG(stack[i]);
        }
        throw s;
        return Value();
    }

    Value BuiltinError(string err) { return Error(err); }

    string ValueDBG(const Value &a)
    {
        int found;
        int nfound = 0;
        for (size_t i = 0; i < st.identtable.size(); i++) if (a == vars[i]) { found = i; nfound++; }
        string s = a.ToString(debugpp);
        if (nfound == 1) s += " (" + st.ReverseLookupIdent(found) + " ?)";
        return s;
    }

    void Require(Value &v, int t, const char *op) // FIXME: make this a macro so we don't pass this extra string
    {
        if (v.type != t)
        {
            Error(string("type error: ") + op + " requires value of type " + TypeName(t) + ", instead found " + ProperTypeName(v), v);
        }
    }
    
    VM(SymbolTable &_st) : st(_st), vars(NULL), stack(NULL),
                           stacksize(0), sp(-1), framepointer(-1), trace(false), lineinfo(NULL), debugpp(2, 50, true, -1)
    {}

    ~VM()
    {
        if (stack) delete[] stack;
        if (vars)  delete[] vars;

        if (vmpool)
        {
            delete vmpool;
            vmpool = NULL;
        }
    }

    void Init(int *_code, LineInfo *_lineinfo)
    {
        assert(sizeof(int) == sizeof(void *));   // search for "64bit" before trying to make a 64bit build, changes may be required
        assert(vmpool == NULL);
        vmpool = new SlabAlloc();
        codestart = _code;
        lineinfo = _lineinfo;
        vars = new Value[st.identtable.size()];
        stack = new Value[stacksize = 4096];
        timer.Init();
    }

    void StartEval()
    {
        int rc = EvalE(codestart, -1);
        assert(rc < 0);
    }

    void EndEval(string &evalret)
    {
        evalret = TOP().ToString(programprintprefs);
        POP().DEC();
        for (size_t i = 0; i < st.identtable.size(); i++) vars[i].DEC();
        StackCleanup();
        DumpLeaks();
        assert(framepointer == -1);
    }

    static bool _LeakSorter(void *va, void *vb)
    {
        auto a = (LVector *)va;
        auto b = (LVector *)vb;
        return a->refc != b->refc
        ? a->refc > b->refc
        : (a->type != b->type
           ? a->type > b->type 
           : (a->type == V_VECTOR
              ? a->len > b->len 
              : false));
    }

    void DumpLeaks()
    {
        vector<void *> leaks;
        vmpool->findleaks(leaks);

        if (leaks.size())
        {
            printf("\nLEAKS FOUND (this indicates cycles in your object graph, or a bug in Lobster, details in leaks.txt)\n");
                    
            FILE *leakf = fopen("leaks.txt", "w");
            if (leakf)
            {
                //qsort(&leaks[0], leaks.size(), sizeof(void *), &LeakSorter);
                sort(leaks.begin(), leaks.end(), _LeakSorter);

                PrintPrefs leakpp = debugpp;
                leakpp.cycles = 0;
                        
                for (auto p : leaks)
                {
                    auto vec = (LVector *)p;
                    switch(vec->type)
                    {
                        case V_CYCLEDONE:
                        case V_VECTORBUF:
                            break;
                                    
                        case V_STRING:
                        {
                            auto str = (LString *)vec;
                            fputs((str->CycleStr() + " = " + str->ToString(leakpp) + "\n").c_str(), leakf);
                            break;
                        }
                                    
                        default:
                        {
                            fputs((vec->CycleStr() + " = " + vec->ToString(leakpp) + "\n").c_str(), leakf);
                            break;
                        }
                    }
                }
                        
                fclose(leakf);
            }
        }
                
        vmpool->printstats();
    }
    
    string DumpVar(Value &x, SymbolTable &st, int idx)
    {
        return x.type == V_UNDEFINED ? "" : "\n   " + st.ReverseLookupIdent(idx) + " = " + x.ToString(debugpp);
    }

    string DumpGlobals()
    {
        string s;
        for (size_t i = 0; i < st.identtable.size(); i++)
        {
            s += DumpVar(vars[i], st, i);
        }
        return s;
    }

    #undef new
    LVector *NewVector(int n, int t) { return new (vmpool->alloc(sizeof(LVector) + sizeof(Value) * n)) LVector(n, t); }
    LString *NewString(int l)        { auto s = (char *)vmpool->alloc(sizeof(LString) + l + 1); return new (s) LString(l); }
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #endif
    
    LString *NewString(const char *c, int l)
    {
        auto s = NewString(l);
        memcpy(s->str(), c, l);
        s->str()[l] = 0;
        return s;
    }

    LString *NewString(const string &s)
    {
        return NewString(s.c_str(), s.size());
    }

    LString *NewString(char *c1, int l1, char *c2, int l2)
    {
        auto s = NewString(l1 + l2);
        memcpy(s->str(),      c1, l1);
        memcpy(s->str() + l1, c2, l2);
        s->str()[l1 + l2] = 0;
        return s;
    }

    int EvalMulti(int nargs, int *ip, int definedfunction)
    {
        assert(*ip == IL_FUNMULTI);
        ip++;

        auto nsubf = *ip++;
        for (int i = 0; i < nsubf; i++)
        {
            for (int j = 0; j < nargs; j++)  // TODO: rather than going thru all args, only go thru those that have types
            {
                int desired = *ip++;
                if (desired != V_UNDEFINED)
                {
                    Value &v = stack[sp - nargs + j + 1];
                    if (v.type != desired || (v.type == V_VECTOR && v.vval->type != *ip))
                    {
                        ip += (nargs - j) * 2;
                        goto fail;
                    }
                }
                ip++;
            }

            return EvalF(nargs, codestart + *ip, definedfunction);

            fail:;
        }

        string argtypes;
        for (int j = 0; j < nargs; j++)
        {
            argtypes += ProperTypeName(stack[sp - nargs + j + 1]);
            if (j < nargs - 1) argtypes += ", ";
        }
        Error("the call " + st.ReverseLookupFunction(definedfunction) + "(" + argtypes + ") did not match any function variants");
        return -1; // FIXME
    }

    void StackCleanup()
    {
        while (sp > framepointer) POP().DEC();  // only if from a return or error thats has tempories above it, and if returning thru a control structure
    }
    
    void varcleanup(string *error = NULL)
    {
        StackCleanup();
        // FIXME: types & assert
        framepointer = POP().ival;
        auto ip = POP().ip; 
        auto nargs = POP().ival;
        auto nfree = *ip++;
        auto freevars = ip + nargs;
        ip += nfree;
        auto ndef = *ip++;
        auto defvars = ip + ndef;
        while (ndef--)  { auto i = *--defvars;  if (error && error->size() < 10000) (*error) += DumpVar(vars[i], st, i); vars[i].DEC(); vars[i] = POP(); }
        while (nargs--) { auto i = *--freevars; if (error && error->size() < 10000) (*error) += DumpVar(vars[i], st, i); vars[i].DEC(); vars[i] = POP(); } 
    }

    int *FunIntro(int nargs, int *ip)
    {
        assert(*ip == IL_FUNSTART);
        ip++;

        auto funstart = ip;

        if (sp > stacksize - 1024)   // no vars on the stack, so per function call increment should be small
        {                            // FIXME: not safe for untrusted scripts
            if (stacksize >= 256 * 1024) Error("stack overflow!");   // FIXME: make user configurable
            auto nstack = new Value[stacksize *= 2];
            memcpy(nstack, stack, sizeof(Value) * (sp + 1));
            delete[] stack;
            stack = nstack;
        }

        auto nfree = *ip++;
        if (nargs > nfree)      // nargs < nfree currently only happens with _ DS vars, see todo.txt
        {
            string nas = inttoa(nargs);
            Error(string("function called with ") + nas + " arguments, max possible: " + inttoa(nfree));
        }
        auto freevars = ip + nargs;
        
        for (int i = 0; i < nargs; i++) swap(vars[ip[i]], stack[sp - nargs + i + 1]);
        ip += nfree;
        auto ndef = *ip++;
        auto defvars = ip + ndef;
        for (int i = 0; i < ndef; i++) PUSH(vars[*ip++].INC()); // for most locals, this just saves an undefined, only in recursive cases it has an actual value. The reason we don't clear the var after backing it up is that in the DS case, you want to be able to use the old value until a new one gets defined, as in a <- a + 1. clearing it would save the INC and a DEC when it eventually gets overwritten, so maybe we can at some point distinguish between vars that are used with DS and those that are not.

        PUSH(Value(nargs));
        PUSH(Value(funstart));
        PUSH(Value(framepointer));
        framepointer = sp;

        return ip;

        // stackframe: parbackups, localbackups, nargs, funstart, oldfp ...temps                           
    }

    int EvalF(int nargs, int *ip, int definedfunction)
    {
        ip = FunIntro(nargs, ip);

        int towhere = EvalE(ip, definedfunction);
        Value ret = POP();
        varcleanup();
        PUSH(ret);

        return towhere != -1 && towhere != definedfunction ? towhere : -1;
    }

    int EvalE(int *ip, int definedfunction)
    {
        try
        {
            return Eval(ip);
        }
        catch(string error)
        {
            LineInfo &li = lineinfo[ip - codestart];
            if (definedfunction >= 0)
            {
                error += "\nin function: " + st.ReverseLookupFunction(definedfunction);
            }
            else
            {
                error += "\nin block";
            }
            error += " -> " + st.filenames[li.fileidx] + "(" + inttoa(li.line) + ")";

            if (framepointer >= 0) varcleanup(&error);

            throw error;
        }        
    }

    #define CHECKE(ec) { int rc = ec; if (rc != -1) return rc; }
    
    int Eval(int *ip)
    {
        for (;;)
        {
            #ifdef _HEAVY_DEBUG
                if (trace)
                {
                    DisAsmIns(stdout, st, ip, codestart, lineinfo);
                    if (sp >= 0) printf(" [%d] - %s", sp + 1, TOP().ToString(debugpp).c_str());
                    if (sp >= 1) printf(" / %s", stack[sp - 1].ToString(debugpp).c_str());
                    //printf(" [%d/%s]", vars[6].top, vars[6].TOP().ToString(true, 2, 10).c_str());
                    printf("\n");
                }
            #endif

            lineip = ip;
            int opc = *ip++;

            #ifdef _DEBUG
                currentline = lineinfo[lineip - codestart].line;
            #endif

            switch (opc)
            {
                case IL_PUSHUNDEF: PUSH(Value()); break;
                case IL_PUSHINT:   PUSH(Value(*ip++)); break;
                case IL_PUSHFLT:   PUSH(Value(*(float *)ip)); ip++; break;
                case IL_PUSHNIL:   PUSH(Value(0, V_NIL)); break;

                case IL_PUSHFUN:
                {
                    int after = *ip++;
                    PUSH(Value(ip));
                    ip = codestart + after;
                    break;
                }

                case IL_PUSHSTR:
                {
                    auto start = ip;
                    while (*ip++) ;
                    auto len = (int)(ip - start);
                    auto s = NewString(len - 1);   // FIXME: have a way that constant strings can stay in the bytecode, or at least preallocate them all
                    for (int i = 0; i < len; i++) s->str()[i] = start[i]; 
                    PUSH(Value(s));
                    break;
                }

                case IL_CALL:
                {
                    auto nargs = *ip++;
                    auto fvar = *ip++;
                    auto fun = st.functiontable[fvar]->bytecodestart;
                    CHECKE(EvalF(nargs, codestart + fun, fvar));
                    break;
                }

                case IL_CALLMULTI:
                {
                    auto nargs = *ip++;
                    auto fvar = *ip++;
                    auto fun = st.functiontable[fvar]->bytecodestart;
                    CHECKE(EvalMulti(nargs, codestart + fun, fvar));
                    break;
                }

                case IL_CALLVCOND:
                    if (TOP().type != V_CLOSURE) { ip++; break; } // FIXME: don't need to check for closure again below if false
                case IL_CALLV:
                {
                    Value fun = POP();
                    Require(fun, V_CLOSURE, "function call");
                    auto nargs = *ip++;
                    CHECKE(EvalF(nargs, fun.ip, -1));
                    break;
                }

                case IL_CALLVLOOP1:      // FIXME refactor
                {
                    Value fun = POP();
                    Require(fun, V_CLOSURE, "function call");
                    auto nargs = *ip++;
                    CHECKE(EvalF(nargs, fun.ip, -1));
                    loopval[0] = POP();
                    break;
                }

                case IL_CALLVLOOP1DUP2:      // FIXME refactor
                {
                    Value fun = stack[sp - 1];  //urgh!
                    Require(fun, V_CLOSURE, "function call");
                    auto nargs = *ip++;
                    CHECKE(EvalF(nargs, fun.ip, -1));
                    loopval[0] = POP();
                    break;
                }

                case IL_CALLVLOOP2DUP1:      // FIXME refactor
                {
                    Value fun = stack[sp];  //urgh!
                    Require(fun, V_CLOSURE, "function call");
                    auto nargs = *ip++;
                    CHECKE(EvalF(nargs, fun.ip, -1));
                    loopval[1] = POP();
                    break;
                }

                case IL_FUNSTART:
                    assert(0);

                case IL_FUNEND:
                    return -1;

                case IL_RETURN:
                    return *ip++;

                case IL_CONT1:
                {
                    auto nf = natreg.nfuns[*ip++];
                    auto ret = nf->cont1(POP());
                    PUSH(ret);
                    break;
                }

                case IL_CONT2:
                    assert(0);
                    break;

                case IL_BCALL:
                {
                    auto nf = natreg.nfuns[*ip++];
                    int n = *ip++;
                    //printf("%s %d\n", nf->name.c_str(), nf->nargs);
                    if (n > (int)nf->nargs) Error("native function \"" + nf->name + "\" called with too many arguments");
                    Value v;
                    switch (nf->nargs)
                    {
                        #define ARG(N) Value a##N = POP(); NFCheck(a##N, *nf, N);
                        #define RET() break
                        case 0: {                                                 v = nf->fun.f0();                       RET(); }
                        case 1: { ARG(0);                                         v = nf->fun.f1(a0);                     RET(); }
                        case 2: { ARG(1); ARG(0);                                 v = nf->fun.f2(a0, a1);                 RET(); }
                        case 3: { ARG(2); ARG(1); ARG(0);                         v = nf->fun.f3(a0, a1, a2);             RET(); }
                        case 4: { ARG(3); ARG(2); ARG(1); ARG(0);                 v = nf->fun.f4(a0, a1, a2, a3);         RET(); }
                        case 5: { ARG(4); ARG(3); ARG(2); ARG(1); ARG(0);         v = nf->fun.f5(a0, a1, a2, a3, a4);     RET(); }
                        case 6: { ARG(5); ARG(4); ARG(3); ARG(2); ARG(1); ARG(0); v = nf->fun.f6(a0, a1, a2, a3, a4, a5); RET(); }
                        default: assert(0); break;
                        #undef RET
                        #undef ARG
                    }
                    /*
                    if (v.type == V_EXCEPTION)
                    {
                        PUSH(exceptionval);
                        return v.ival;
                    };
                    */
                    PUSH(v);
                    break;
                }
                
                case IL_JUMP:
                    ip = codestart + *ip;
                    break;
                
                case IL_NEWVEC:
                {
                    int type = *ip++;
                    auto vec = NewVector(*ip++, type);
                    PUSH(Value(vec));
                    break;
                }

                case IL_POP:
                    POP().DEC();
                    break;

                case IL_EXIT:
                    return -1;

                #define REFOP(exp) { \
                    res = exp;  \
                    a.DEC(); b.DEC(); \
                }
                #define BOP(op, l, r, extras) { \
                    if (extras & 1 && r == 0) Div0(); \
                    res = l op r;  \
                    break; \
                }
                #define COP(t, op, l, r, extras) if (b.type == t) BOP(op, l, r, extras)
                #define GETARGS() Value b = POP(); Value a = POP()
                #define IOP(op, extras) { \
                    GETARGS(); Value res; \
                    if (a.type != V_INT || b.type != V_INT) BError(#op, a, b); \
                    for (;;) BOP(op, a.ival, b.ival, extras); PUSH(res); break; \
                }
                #define OP(op, extras) { \
                    GETARGS(); \
                    _OP(op, extras); PUSH(res); break; \
                }
                #define _OP(op, extras) Value res; for (;;) { \
                    IFOP(op, extras); \
                    { \
                        bool isfloat = true; \
                        int len = VectorLoop(a, b, res, isfloat); \
                        if (len >= 0) { \
                            for (int j = 0; j < len; j++) \
                            if (isfloat) { auto bv = VectorElem<float>(b, j); if (extras&1 && bv == 0) Div0(); res.vval->at(j) = Value(VectorElem<float>(a, j) op bv); }\
                            else         { auto bv = VectorElem<int>  (b, j); if (extras&1 && bv == 0) Div0(); res.vval->at(j) = Value(VectorElem<int>  (a, j) op bv); }\
                            VectorDec(a, res); VectorDec(b, res); \
                            break; } \
                    } \
                    if (extras & 2) if (StrOps(a, b, res)) break; \
                    BError(#op, a, b); \
                } 
                #define IFOP(op, extras) \
                    if (a.type == V_INT) \
                    { \
                        COP(V_INT, op, a.ival, b.ival, extras) \
                        else COP(V_FLOAT, op, float(a.ival), b.fval, extras) \
                    } \
                    else if (a.type == V_FLOAT) \
                    { \
                        COP(V_INT, op, a.fval, float(b.ival), extras) \
                        else COP(V_FLOAT, op, a.fval, b.fval, extras) \
                    }
                #define COMPOP(op, extras) { Value res; for (;;) { \
                    GETARGS(); \
                    IFOP(op, extras); \
                    if (a.type == V_STRING && b.type == V_STRING) \
                    { \
                        REFOP((*a.sval) op (*b.sval)); break; \
                    } \
                    if (extras & 8) \
                    { \
                        REFOP(a.type == b.type && a.ref == b.ref); break; \
                    } \
                    if (extras & 16) \
                    { \
                        REFOP(a.type != b.type || a.ref != b.ref); break; \
                    } \
                    BError(#op, a, b); \
                } PUSH(res); break; }

                case IL_ADD: OP(+, 2);
                case IL_SUB: OP(-, 0);
                case IL_MUL: OP(*, 0);
                case IL_DIV: OP(/, 1);

                case IL_MOD: IOP(%, 1);
                    
                case IL_LT: COMPOP(<,  0);
                case IL_GT: COMPOP(>,  0);
                case IL_LE: COMPOP(<=, 0);
                case IL_GE: COMPOP(>=, 0);
                case IL_EQ: COMPOP(==, 8);
                case IL_NE: COMPOP(!=, 16);

                case IL_UMINUS:
                {
                    Value a = POP();
                    switch (a.type)
                    {
                        case V_INT: PUSH(Value(-a.ival)); break;
                        case V_FLOAT: PUSH(Value(-a.fval)); break;
                        case V_VECTOR:
                        {
                            bool isfloat = true;
                            Value res;
                            int len = VectorLoop(a, Value(1), res, isfloat);
                            if (len >= 0)
                            {
                                for (int i = 0; i < len; i++) \
                                    res.vval->at(i) = isfloat ? Value(-VectorElem<float>(a, i))
                                                              : Value(-VectorElem<int>  (a, i));
                                VectorDec(a, res);
                                PUSH(res);
                                break;
                            }
                            // fall tru
                        }

                        default: UError("-", a);
                    }
                    break;
                }

                case IL_LOGNOT:
                {
                    Value a = POP();
                    PUSH(!a.DEC().True());    
                    break;
                }

                case IL_PUSHVAR:   PUSH(vars[*ip++].INC()); break;
                case IL_WRITEVAR:  { Value &v = POP();       auto &d = vars[*ip++]; OVERWRITE(d, v); d.DEC(); d = v; break; }
                case IL_WRITEVARR: { Value &v = TOP().INC(); auto &d = vars[*ip++]; OVERWRITE(d, v); d.DEC(); d = v; break; }

                #define PUSHDEREF(i, dyn) \
                { \
                    Value r = POP(); \
                    switch (r.type) \
                    { \
                        case V_VECTOR: \
                            if (!dyn) VecType(r); \
                            IDXErr(i, (int)r.vval->len, r); PUSH(r.vval->at(i).INC()); break; \
                        case V_STRING: IDXErr(i, r.sval->len, r);      PUSH(Value((int)r.sval->str()[i])); break; \
                        default: Error(string("cannot index into type ") + TypeName(r.type), r); \
                    } \
                    r.DECRT(); \
                    break; \
                }
                case IL_PUSHFLD:
                {
                    int i = *ip++;
                    PUSHDEREF(i, false);
                }
                case IL_PUSHIDX:
                {
                    Value idx = POP();
                    Require(idx, V_INT, "vector indexed read");
                    int i = idx.ival;
                    PUSHDEREF(i, true);
                }

                #define WRITEDEREF(ret, dyn, binary) \
                    int i; \
                    if (dyn) { \
                        Value idx = POP(); \
                        Require(idx, V_INT, "vector indexed assign"); \
                        i = idx.ival; \
                    } \
                    else { \
                        i = *ip++; \
                    } \
                    Value vec = POP(); \
                    Require(vec, V_VECTOR, "vector indexed assign"); \
                    if (!dyn) VecType(vec); \
                    Value b = binary ? POP() : Value(); \
                    IDXErr(i, (int)vec.vval->len, vec); \
                    Value &a = vec.vval->at(i);
                #define WRITEDEREFASSIGN(ret, dyn) { \
                    WRITEDEREF(ret, dyn, true); \
                    OVERWRITE(a, b); \
                    a.DEC(); a = b; \
                    if (ret) PUSH(b.INC()); \
                    vec.DECRT(); \
                    break; \
                }
                #define WRITEDEREFOP(ret, dyn, op, extras) { \
                    WRITEDEREF(ret, dyn, true); \
                    OVERWRITE(a, b); \
                    _OP(op, extras); \
                    a = res; \
                    if (ret) PUSH(res.INC()); \
                    vec.DECRT(); \
                    break; \
                }
                #define WRITEOP(ret, op, extras) { \
                    Value b = POP(); \
                    Value &a = vars[*ip++]; \
                    _OP(op, extras); \
                    a = res; \
                    if (ret) PUSH(res.INC()); \
                    break; \
                }
                #define WRITEDEREFOPPP(ret, dyn, op, pre) { \
                    WRITEDEREF(ret, dyn, false); \
                    PPOP(ret, op, pre); \
                    vec.DECRT(); \
                    break; \
                }
                #define WRITEOPPP(ret, op, pre) { \
                    Value &a = vars[*ip++]; \
                    PPOP(ret, op, pre); \
                    break; \
                }
                #define PPOP(ret, op, pre) { \
                    if (ret && !pre) PUSH(a.INC()); \
                    if (a.type == V_INT) a.ival = a.ival op 1; \
                    else if (a.type == V_FLOAT) a.fval = a.fval op 1; \
                    else UError(#op, a); \
                    if (ret && pre) PUSH(a.INC()); \
                }
                
                case IL_WRITEIDX:
                case IL_WRITEIDXR: WRITEDEREFASSIGN(opc == IL_WRITEIDXR,  true);
                case IL_WRITEFLD:  
                case IL_WRITEFLDR: WRITEDEREFASSIGN(opc == IL_WRITEFLDR,  false);

                case IL_PLUSVAR:   
                case IL_PLUSVARR:  WRITEOP(     opc == IL_PLUSVARR,        +, 2);
                case IL_PLUSIDX:  
                case IL_PLUSIDXR:  WRITEDEREFOP(opc == IL_PLUSIDXR, true,  +, 2);
                case IL_PLUSFLD:  
                case IL_PLUSFLDR:  WRITEDEREFOP(opc == IL_PLUSFLDR, false, +, 2);

                case IL_MULVAR:   
                case IL_MULVARR:   WRITEOP(     opc == IL_MULVARR,         *, 0);
                case IL_MULIDX:   
                case IL_MULIDXR:   WRITEDEREFOP(opc == IL_MULIDXR,  true,  *, 0);
                case IL_MULFLD:  
                case IL_MULFLDR:   WRITEDEREFOP(opc == IL_MULFLDR,  false, *, 0);

                case IL_SUBVAR:   
                case IL_SUBVARR:   WRITEOP(     opc == IL_SUBVARR,         -, 0);
                case IL_SUBIDX:   
                case IL_SUBIDXR:   WRITEDEREFOP(opc == IL_SUBIDXR,  true,  -, 0);
                case IL_SUBFLD: 
                case IL_SUBFLDR:   WRITEDEREFOP(opc == IL_SUBFLDR,  false, -, 0);

                case IL_DIVVAR:   
                case IL_DIVVARR:   WRITEOP(     opc == IL_DIVVARR,         /, 1);
                case IL_DIVIDX:  
                case IL_DIVIDXR:   WRITEDEREFOP(opc == IL_DIVIDXR,  true,  /, 1);
                case IL_DIVFLD:   
                case IL_DIVFLDR:   WRITEDEREFOP(opc == IL_DIVFLDR,  false, /, 1);

                case IL_PPVAR:   
                case IL_PPVARR:   WRITEOPPP(     opc == IL_PPVARR,         +, true);
                case IL_PPIDX:                         
                case IL_PPIDXR:   WRITEDEREFOPPP(opc == IL_PPIDXR,  true,  +, true);
                case IL_PPFLD:                          
                case IL_PPFLDR:   WRITEDEREFOPPP(opc == IL_PPFLDR,  false, +, true);

                case IL_MMVAR:   
                case IL_MMVARR:   WRITEOPPP(     opc == IL_MMVARR,         -, true);
                case IL_MMIDX:                                          
                case IL_MMIDXR:   WRITEDEREFOPPP(opc == IL_MMIDXR,  true,  -, true);
                case IL_MMFLD:                                           
                case IL_MMFLDR:   WRITEDEREFOPPP(opc == IL_MMFLDR,  false, -, true);

                case IL_PPPVAR:   
                case IL_PPPVARR:   WRITEOPPP(     opc == IL_PPPVARR,         +, false);
                case IL_PPPIDX:                                                
                case IL_PPPIDXR:   WRITEDEREFOPPP(opc == IL_PPPIDXR,  true,  +, false);
                case IL_PPPFLD:                                               
                case IL_PPPFLDR:   WRITEDEREFOPPP(opc == IL_PPPFLDR,  false, +, false);
                                                                               
                case IL_MMPVAR:                                                
                case IL_MMPVARR:   WRITEOPPP(     opc == IL_MMPVARR,         -, false);
                case IL_MMPIDX:                                                
                case IL_MMPIDXR:   WRITEDEREFOPPP(opc == IL_MMPIDXR,  true,  -, false);
                case IL_MMPFLD:                                                 
                case IL_MMPFLDR:   WRITEDEREFOPPP(opc == IL_MMPFLDR,  false, -, false);

                #undef WRITEDEREFASSIGN
                #undef WRITEDEREFOP
                #undef PUSHDEREF
                #undef WRITEDEREF
                #undef CMPFAILRES
                #undef BOP
                #undef COP
                #undef GETARGS
                #undef BERROR
                #undef UERROR
                #undef IOP
                #undef OP

                case IL_PUSHONCE:
                {
                    auto x = POP();
                    auto &v = TOP();
                    assert(v.type == V_VECTOR);
                    v.vval->push(x);
                    break;
                }

                case IL_PUSHPARENT:
                {
                    auto x = POP();
                    auto &v = TOP();
                    assert(v.type == V_VECTOR);
                    if (x.type != V_VECTOR || *ip++ != x.vval->type) Error("super class constructor is of the wrong type", x);
                    v.vval->append(x.vval, 0, x.vval->len);
                    //for (int i = 0; i < x.vval->len; i++) v.vval->push(x.vval->at(i));
                    x.DECRT();
                    break;
                }

                case IL_JUMPFAIL:    { auto x = POP(); auto nip = *ip++; if (!x.DEC().True()) { ip = codestart + nip;          }               break; }
                case IL_JUMPFAILR:   { auto x = POP(); auto nip = *ip++; if (!x      .True()) { ip = codestart + nip; PUSH(x); } else x.DEC(); break; }
                case IL_JUMPNOFAIL:  { auto x = POP(); auto nip = *ip++; if ( x.DEC().True()) { ip = codestart + nip;          }               break; }
                case IL_JUMPNOFAILR: { auto x = POP(); auto nip = *ip++; if ( x      .True()) { ip = codestart + nip; PUSH(x); } else x.DEC(); break; }

                case IL_TTINT:    { auto &v = vars[*ip++]; if (v.type != V_INT)    TTError("int",    v); break; }
                case IL_TTFLT:    { auto &v = vars[*ip++]; if (v.type != V_FLOAT)  TTError("float",  v); break; }
                case IL_TTSTR:    { auto &v = vars[*ip++]; if (v.type != V_STRING) TTError("string", v); break; }
                case IL_TTSTRUCT: { auto &v = vars[*ip++];
                    auto udtid = *ip++;
                    if (v.type == V_VECTOR)
                    {
                        for (int t = v.vval->type; t != -1; t = st.structtable[t]->superclassidx)
                        {
                            if (t == udtid) goto found;
                        }
                    }
                    TTError(st.ReverseLookupType(udtid), v);
                    found:
                    break;
                }

                case IL_ISTYPE:
                {
                    auto t = *ip++;
                    auto idx = *ip++;
                    auto &v = POP().DEC();
                    PUSH(Value(v.type == t && (t != V_VECTOR || v.vval->type == idx)));
                    break;
                }

                default:
                    Error(string("bytecode format problem: ") + inttoa(opc));
            }
        }
    }

    const char *ProperTypeName(Value &v)
    {
        return v.type == V_VECTOR && v.vval->type >= 0
            ? st.ReverseLookupType(v.vval->type).c_str()
            : TypeName(v.type);
    }

    void TTError(const string &tname, Value &v)
    {
        Error("function requires argument of type " + tname + ", instead got " + ProperTypeName(v), v);
    }

    void TTOverwrite(Value &o, Value &n)
    {
        int ot = o.type;
        int nt = n.type;
        if (ot == V_VECTOR)
        {
            if (nt == V_VECTOR)
            {
                ot = o.vval->type;
                nt = n.vval->type;
                if (ot == nt) return;   // 2nd most common path
                if (ot >= 0 && nt >= 0) return; // for now, any struct types can be exchanged
                // the code below only does super->sub and sub->super, but can't do common supertype which is too expensive at runtime
                //for (int t = nt; t != -1; t = st.structtable[t]->superclassidx) if (t == ot) return;    // overwrite with a subclass
                //for (int t = ot; t != -1; t = st.structtable[t]->superclassidx) if (t == nt) return;    // overwrite with a superclass
            }
            else if (nt == V_NIL)
            {
                return;
            }
        }
        else if (ot == nt    // 1st most common path
                // all of these less common
            || ot == V_NIL
            || nt == V_NIL
            || ot == V_UNDEFINED)    
        {
            return;
        }
        else if (ot == V_FLOAT && nt == V_INT)   // medium common
        {
            n.type = V_FLOAT;
            n.fval = (float)n.ival;
            return;
        }
        Error(string("can't overwrite variable of type ") + ProperTypeName(o) + " with " + ProperTypeName(n), n);
    }

    void NFCheck(Value &v, NativeFun &nf, int i)
    {
        int desired = nf.args[i].type.t;

        switch (desired)
        {
            case V_UNDEFINED: return;
            //case V_INT:       if (v.type == V_FLOAT)  { v = Value((int)  v.fval);                          return; } break;
            case V_FLOAT:     if (v.type == V_INT)    { v = Value((float)v.ival);                                return; } break;
            case V_STRING:    if (v.type != V_STRING) { v = Value(NewString(v.ToString(programprintprefs))); return; } break;
            default:          break;
        }
                
        if (desired != v.type)
        {
            if ((nf.args[i].flags == NF_OPTIONAL) && v.type == V_NIL) return;
            Error(string("argument ") + inttoa(i + 1) + " of native function \"" + nf.name + "\" needs to have type " + nf.args[i].type.Name() + ", not " + ProperTypeName(v), v);
        }
    }

    bool StrOps(Value &a, Value &b, Value &res)
    {
        if      (a.type == V_STRING) { string s = b.ToString(programprintprefs); res = NewString(a.sval->str(), a.sval->len, (char *)s.c_str(), (int)s.size()); a.DEC(); b.DEC(); return true; }
        else if (b.type == V_STRING) { string s = a.ToString(programprintprefs); res = NewString((char *)s.c_str(), (int)s.size(), b.sval->str(), b.sval->len); a.DEC(); b.DEC(); return true; }
        return false;
    }

    void BError(const char *op, Value &a, Value &b) { Error(string("binary operator ") + op + " cannot operate on " + ProperTypeName(a) + " and " + ProperTypeName(b), a, b); }
    void UError(const char *op, Value &a)           { Error(string("unary operator ")  + op + " cannot operate on " + ProperTypeName(a), a); }
    void Div0()                                     { Error("division by zero"); }
    
    void IDXErr(int i, int n, Value &v) { if (i < 0 || i >= n) Error(string("index ") + string(inttoa(i)) + " out of range " + string(inttoa(n)), v); }
    void VecType(Value &vec) { if (vec.vval->type < 0) Error("cannot use field dereferencing on untyped vector", vec); }

    bool AllInt(LVector *v)
    {
        for (int i = 0; i < v->len; i++)
            if (v->at(i).type != V_INT)
                return false;
        return true;
    }

    int VectorLoop(const Value &a, const Value &b, Value &res, bool &isfloat)
    {
        // note: not doing DEC() on the reused vectors is ok because VectorElem will error on not float/int
        int len;
        int type = V_VECTOR;
        if (a.type == V_VECTOR)
        {
            len = a.vval->len;
            if (b.type == V_VECTOR)
            {
                len = min(len, b.vval->len);
                if (len && AllInt(a.vval) && AllInt(b.vval)) isfloat = false;
                bool aseq = a.vval->len <= b.vval->len;
                if ( aseq && a.vval->refc == 1) { res = a; return len; }
                if (!aseq && b.vval->refc == 1) { res = b; return len; }
                type = (aseq ? a : b).vval->type;
            }
            else
            {
                if (b.type == V_INT) { if (len && AllInt(a.vval)) isfloat = false; }
                else if (b.type != V_FLOAT) return -1;
                if (a.vval->refc == 1) { res = a; return len; }
                type = a.vval->type;
            }
        }
        else if (b.type == V_VECTOR)
        {
            len = b.vval->len;
            if (a.type == V_INT) { if (len && AllInt(b.vval)) isfloat = false; }
            else if (b.type != V_FLOAT) return -1;
            if (b.vval->refc == 1) { res = b; return len; }
            type = b.vval->type;
        }
        else
        {
            return -1;
        }
        res.vval = NewVector(len, type);
        res.type = V_VECTOR;
        res.vval->len = len;    // so we can overwrite, needed for reuse
        return len;
    }

    int VectorTrim(Value &a, int len)
    {
        while(a.vval->len > len) a.vval->pop().DEC();
        return len;
    }

    void VectorDec(Value &a, Value &res)
    {
        if (a.type == V_VECTOR && a.vval != res.vval) a.DEC();
    }

    template<typename T> T VectorElem(Value &a, int i)
    {
        switch (a.type)
        {
            case V_FLOAT: return (T)a.fval;
            case V_INT:   return (T)a.ival;
            case V_VECTOR:
            {
                auto v = a.vval->at(i);
                switch (v.type)
                {
                    case V_FLOAT: return (T)v.fval;
                    case V_INT:   return (T)v.ival;
                    default:
                        Error(string("can't do vector operation with vector element type ") + ProperTypeName(a), a);
                        return 0;
                }
            }
            default:
                assert(0);
                return 0;
        }
    }

    /*
    Value exceptionval; // FIXME: bit of a hack

    Value EvalC(Value &cl, int nargs)
    {
        assert(cl.type == V_CLOSURE);
        int rc = EvalF(nargs, cl.ip, -1);
        if (rc >= 0) { exceptionval = POP(); return Value(rc, V_EXCEPTION); }
        return POP();
    }
    */

    Value loopval[2]; // FIXME: hack
    Value LoopVal(int i) { return loopval[i]; }

    void Push(const Value &v) { PUSH(v); }
    Value Pop() { return POP(); }

    int StructIdx(string &name, size_t &nargs) { return st.StructIdx(name, nargs); }
    virtual string &ReverseLookupType(unsigned int v) { return st.ReverseLookupType(v); }

    #undef PUSH
    #undef TOP
    #undef POP
    #undef TOPPTR

    void Trace(bool on) { trace = on; }
    float Time() { return (float)timer.SecsSinceStart(); }

    int GC()    // shouldn't really be used, but just in case
    {
        for (int i = 0; i <= sp; i++) stack[i].Mark();
        for (size_t i = 0; i < st.identtable.size(); i++) vars[i].Mark();

        vector<void *> objs;
        vector<void *> leaks;
        vmpool->findleaks(objs); // FIXME: inefficient sticking them in a vector first

        for (auto p : objs)
        {
            auto r = (RefObj *)p;
            if (r->type == V_VECTORBUF) continue;
            if (r->refc > 0) leaks.push_back(r);
            r->refc = -r->refc;
        }

        for (auto p : leaks)
        {
            auto v = (LVector *)p;
            v->refc = 0;
            if (v->type == V_STRING) Value((LString *)v).DECDELETE();
            else { v->len = 0; Value(v).DECDELETE(); }
        }

        return leaks.size();
    }
};
