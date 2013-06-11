#ifdef _DEBUG
    #define VM_PROFILER                     // tiny VM slowdown and memory usage when enabled
#endif

struct VM : VMBase
{
    Value *stack;
    int stacksize;
    int maxstacksize;
    int sp;

    enum
    {
        INITSTACKSIZE   =   4 * 1024,    // *8 bytes each
        DEFMAXSTACKSIZE = 128 * 1024,    // *8 bytes each, should be modest on smallest handheld we support (iPhone 3GS has 256MB)
        STACKMARGIN     =   1 * 1024     // *8 bytes each, max by which the stack could possibly grow in a single function call
    }; 

    int *ip;

    CoRoutine *curcoroutine;

    Value *vars;
    
    size_t codelen;
    int *codestart;
    size_t *byteprofilecounts;
    size_t *lineprofilecounts;
    
    
    SymbolTable &st;

    bool trace;

    const vector<LineInfo> &lineinfo;
    #ifdef _DEBUG
        int currentline;
        int maxsp;
    #endif

    PrintPrefs debugpp;

    vector<int> default_vector_types;
    
    const char *programname;

    #define PUSH(v) (stack[++sp] = (v))
    #define TOP() (stack[sp])
    #define POP() (stack[sp--]) // (sp < 0 ? 0/(sp + 1) : stack[sp--])
    #define TOPPTR() (stack + sp + 1)
    #define OVERWRITE(o, n) TTOverwrite(o, n)

    VM(SymbolTable &_st, int *_code, int _len, const vector<LineInfo> &_lineinfo, const char *_pn)
        : stack(NULL), stacksize(0), maxstacksize(DEFMAXSTACKSIZE), sp(-1), ip(NULL),
          curcoroutine(NULL), vars(NULL), st(_st), codelen(_len), byteprofilecounts(NULL), lineprofilecounts(NULL),
          trace(false), lineinfo(_lineinfo), debugpp(2, 50, true, -1), programname(_pn)
    {
        assert(sizeof(int) == sizeof(void *));   // search for "64bit" before trying to make a 64bit build, changes may be required
        assert(vmpool == NULL);
        vmpool = new SlabAlloc();
        ip = codestart = _code;
        vars = new Value[st.identtable.size()];
        stack = new Value[stacksize = INITSTACKSIZE];

        #ifdef _DEBUG
            currentline = -1;
            maxsp = -1;
        #endif
        
        #ifdef VM_PROFILER
            byteprofilecounts = new size_t[codelen];
            lineprofilecounts = new size_t[lineinfo.size()];
            memset(byteprofilecounts, 0, sizeof(size_t) * codelen);
            memset(lineprofilecounts, 0, sizeof(size_t) * lineinfo.size());
        #endif

        static const char *default_vector_type_names[] = { "xy", "xyz", "xyzw", NULL }; // TODO: this isn't great hardcoded in the compiler, would be better if it was declared in lobster code
        for (auto name = default_vector_type_names; *name; name++)
        {
            int t = V_VECTOR;
            for (auto s : st.structtable) if (s->name == *name) { t = s->idx; break; } // linear search because we may not have the map available
            default_vector_types.push_back(t);
        }

        assert(g_vm == NULL);
        g_vm = this;
    }

    ~VM()
    {
        assert(g_vm == this);
        g_vm = NULL;

        if (stack) delete[] stack;
        if (vars)  delete[] vars;

        if (vmpool)
        {
            delete vmpool;
            vmpool = NULL;
        }
    }

    void SetMaxStack(int ms) { maxstacksize = ms; }
    const char *GetProgramName() { return programname; }
    int GetVectorType(int which) { return default_vector_types[which - 2]; }

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

        if (!leaks.empty())
        {
            printf("\nLEAKS FOUND (this indicates cycles in your object graph, or a bug in Lobster, details in leaks.txt)\n");
                    
            FILE *leakf = OpenForWriting("leaks.txt", false);
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
                        case V_VALUEBUF:
                            break;
                                    
                        case V_STRING:
                        {
                            auto str = (LString *)vec;
                            fputs((str->CycleStr() + " = " + str->ToString(leakpp) + "\n").c_str(), leakf);
                            break;
                        }

                        case V_COROUTINE:
                        {
                            auto co = (CoRoutine *)vec;
                            fputs((co->CycleStr() + " = coroutine\n").c_str(), leakf);
                            break;
                        }
                                    
                        default:
                        {
                            assert(vec->type >= V_VECTOR);
                            fputs((vec->CycleStr() + " = " + vec->ToString(leakpp) + "\n").c_str(), leakf);
                            break;
                        }
                    }
                }
                        
                fclose(leakf);
            }
        }
                
        vmpool->printstats(false);
    }

    const LineInfo &LookupLine(int *ip) { return ::LookupLine(ip - codestart, lineinfo); }
    
    #undef new
    LVector   *NewVector(int n, int t)                        { return new (vmpool->alloc(sizeof(LVector) + sizeof(Value) * n)) LVector(n, t); }
    LString   *NewString(int l)                               { return new (vmpool->alloc(sizeof(LString) + l + 1)) LString(l); }
    CoRoutine *NewCoRoutine(int *rip, int *vip, CoRoutine *p) { return new (vmpool->alloc(sizeof(CoRoutine))) CoRoutine(sp + 2 /* top of sp + pushed coro */, rip, vip, p); }
    #ifdef WIN32
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #endif
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

    LString *NewString(const char *c1, int l1, const char *c2, int l2)
    {
        auto s = NewString(l1 + l2);
        memcpy(s->str(),      c1, l1);
        memcpy(s->str() + l1, c2, l2);
        s->str()[l1 + l2] = 0;
        return s;
    }

    Value Error(string err, const Value &a = Value(0, V_MAXVMTYPES), const Value &b = Value(0, V_MAXVMTYPES))
    {
        const LineInfo &li = LookupLine(ip - 1);  // error is usually in the byte before the current ip
        auto s = string(st.filenames[li.fileidx]) + "(" + inttoa(li.line) + "): VM error: " + err;
        if (a.type != V_MAXVMTYPES) s += "\n   arg: " + ValueDBG(a);
        if (b.type != V_MAXVMTYPES) s += "\n   arg: " + ValueDBG(b);
        while (sp >= 0 && TOP().type != V_DEFFUN)
        {
            if (TOP().type != V_UNKNOWN)
            {
                s += "\n   stack: " + ValueDBG(TOP());
            }
            POP().DEC();
        }

        for (;;)
        {
            TempCleanup();

            if (sp < 0) break;
        
            string locals;
            int deffun = varcleanup(s.length() < 10000 ? &locals : NULL);

            const LineInfo &li = LookupLine(ip - 1);
            if (deffun >= 0)
            {
                s += "\nin function: " + st.ReverseLookupFunction(deffun);
            }
            else
            {
                s += "\nin block";
            }
            s += " -> " + st.filenames[li.fileidx] + "(" + inttoa(li.line) + ")";

            s += locals;
        }

        s += "\nglobals:";

        for (size_t i = 0; i < st.identtable.size(); i++)
        {
            s += DumpVar(vars[i], st, i);
        }

        FinalStackVarsCleanup();

        throw s;
        return Value();
    }

    Value BuiltinError(string err) { return Error(err); }

    string ValueDBG(const Value &a)
    {
        int found;
        int nfound = 0;
        for (size_t i = 0; i < st.identtable.size(); i++) if (a.Equal(vars[i], false)) { found = i; nfound++; }
        string s = a.ToString(debugpp);
        if (nfound == 1) s += " (" + st.ReverseLookupIdent(found) + " ?)";
        return s;
    }

    string DumpVar(const Value &x, SymbolTable &st, int idx)
    {
        if (x.type == V_UNKNOWN) return "";
        if (st.ReadOnlyIdent(idx)) return "";
        return "\n   " + st.ReverseLookupIdent(idx) + " = " + x.ToString(debugpp);
    }

    void EvalMulti(int nargs, int *ip, int definedfunction, int *oldip)
    {
        assert(*ip == IL_FUNMULTI);
        ip++;

        auto nsubf = *ip++;
        for (int i = 0; i < nsubf; i++)
        {
            for (int j = 0; j < nargs; j++)  // TODO: rather than going thru all args, only go thru those that have types
            {
                int desired = *ip++;
                if (desired != V_UNKNOWN)
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

            return FunIntro(nargs, codestart + *ip, definedfunction, oldip);

            fail:;
        }

        string argtypes;
        for (int j = 0; j < nargs; j++)
        {
            argtypes += ProperTypeName(stack[sp - nargs + j + 1]);
            if (j < nargs - 1) argtypes += ", ";
        }
        Error("the call " + st.ReverseLookupFunction(definedfunction) + "(" + argtypes + ") did not match any function variants");
    }

    void FinalStackVarsCleanup()
    {
        assert(sp < 0);

        for (size_t i = 0; i < st.identtable.size(); i++) vars[i].DEC();

        #ifdef _DEBUG
            DebugLog(0, (string("stack at its highest was: ") + inttoa(maxsp)).c_str());
        #endif
    }
    
    int CallerId()
    {
        for (int _sp = sp; _sp >= 0; _sp--)
            if (stack[_sp].type == V_RETIP)
                return stack[_sp].ip - codestart;
        return -1;
    }

    void TempCleanup()
    {
        while (sp >= 0 && TOP().type != V_DEFFUN) POP().DEC();  // only if from a return or error thats has tempories above it, and if returning thru a control structure
    }
    
    int varcleanup(string *error)
    {
        auto dfv = POP(); assert(dfv.type == V_DEFFUN);       auto deffun = dfv.ival;
        auto ipv = POP(); assert(ipv.type == V_FUNSTART);     ip = ipv.ip; 
        auto nav = POP(); assert(nav.type == V_NARGS);        auto nargs = nav.ival;
        auto riv = POP(); assert(riv.type == V_RETIP);        auto oldip = riv.ip;
        auto nfree = *ip++;
        auto freevars = ip + nargs;
        ip += nfree;
        auto ndef = *ip++;
        auto defvars = ip + ndef;
        while (ndef--)  { auto i = *--defvars;  if (error) (*error) += DumpVar(vars[i], st, i); vars[i].DEC(); vars[i] = POP(); }
        while (nargs--) { auto i = *--freevars; if (error) (*error) += DumpVar(vars[i], st, i); vars[i].DEC(); vars[i] = POP(); } 

        ip = oldip;

        return deffun;
    }

    void FunIntro(int nargs, int *newip, int definedfunction, int *oldip)
    {
        ip = newip;

        assert(*ip == IL_FUNSTART);
        ip++;

        auto funstart = ip;

        if (sp > stacksize - STACKMARGIN)   // per function call increment should be small
        {                                   // FIXME: not safe for untrusted scripts, could simply add lots of locals
            if (stacksize >= maxstacksize) Error("stack overflow! (use set_max_stack_size() if needed)");
            auto nstack = new Value[stacksize *= 2];
            memcpy(nstack, stack, sizeof(Value) * (sp + 1));
            delete[] stack;
            stack = nstack;

            DebugLog(0, (string("stack grew to: ") + inttoa(stacksize)).c_str());
        }

        auto nfree = *ip++;
        if (nargs > nfree)      // nargs < nfree currently only happens with _ DS vars, see todo.txt
        {
            string nas = inttoa(nargs);
            Error(string("function value called with ") + nas + " arguments, but declared with only " + inttoa(nfree));
        }
        //auto freevars = ip + nargs;
        
        for (int i = 0; i < nargs; i++) swap(vars[ip[i]], stack[sp - nargs + i + 1]);
        ip += nfree;
        auto ndef = *ip++;
        //auto defvars = ip + ndef;
        for (int i = 0; i < ndef; i++) PUSH(vars[*ip++].INC()); // for most locals, this just saves an undefined, only in recursive cases it has an actual value. The reason we don't clear the var after backing it up is that in the DS case, you want to be able to use the old value until a new one gets defined, as in a <- a + 1. clearing it would save the INC and a DEC when it eventually gets overwritten, so maybe we can at some point distinguish between vars that are used with DS and those that are not. for recursive functions it can be problematic with TTOVERWRITE check, but we fixed this temp by using a separate instruction for assign + def

        // FIXME: can we reduce the amount of this?
        PUSH(Value(oldip, V_RETIP)); 
        PUSH(Value(nargs, V_NARGS)); 
        PUSH(Value(funstart, V_FUNSTART));
        PUSH(Value(definedfunction, V_DEFFUN)); // we assume that V_DEFFUN marks the top of the stackframe elsewhere

        #ifdef _DEBUG
            if (sp > maxsp) maxsp = sp;
        #endif                        
    }

    bool FunOut(int towhere, int nrv)
    {
        bool bottom = false;
        //Value ret = POP();
        sp -= nrv;
        auto rvs = TOPPTR();
        for(;;)
        {
            TempCleanup();
            if (sp < 0)
            {
                if (towhere >= 0) Error("\"return from " + st.ReverseLookupFunction(towhere) + "\" outside of function");
                bottom = true;
                break;
            }
            int deffun = varcleanup(NULL);
            if(towhere == -1 || towhere == deffun) break;
        }
        //PUSH(ret);
        memcpy(TOPPTR(), rvs, nrv * sizeof(Value));
        sp += nrv;
        return bottom;
    }

    void CoNonRec(int *varip)
    {
        // probably could be skipped in a "release" mode
        for (auto co = curcoroutine; co; co = co->parent) if (co->varip == varip)
            Error("cannot create coroutine recursively");   // if allowed, inner coro would save vars of outer, and then possibly restore them outside of scope of parent
        // TODO: this check guarantees all saved stack vars are undef, except for DS vars, which could still cause problems
    }

    void CoNew()
    {
        int *returnip = codestart + *ip++;
        CoNonRec(ip);
        curcoroutine = NewCoRoutine(returnip, ip, curcoroutine);
        curcoroutine->BackupParentVars(vars);
        int nvars = *ip++;
        ip += nvars;
        PUSH(Value(curcoroutine));
    }

    void CoDone()
    {
        int newtop = curcoroutine->Suspend(sp + 1, stack, ip, curcoroutine);
        sp = newtop - 1; // top of stack is now coro value from create or resume
    }

    void CoClean()
    {
        for (int i = 1; i <= *curcoroutine->varip; i++)
        {
            auto &var = vars[curcoroutine->varip[i]];
            var = curcoroutine->stackcopy[i - 1];
        }

        auto co = curcoroutine;
        CoDone();
        assert(co->stackcopylen == 1);
        co->active = false;
    }

    void CoYield(int nargs)
    {
        if (nargs > 1)
            Error("more than 1 argument supplied to coroutine yield function");
        if (!curcoroutine)
            Error("coroutine yield function called outside of context");  // can theoretically happen if programmer caches yield value somewhere

        Value ret(0, V_NIL);  
        if (nargs) ret = POP();

        for (int i = 1; i <= *curcoroutine->varip; i++)
        {
            auto &var = vars[curcoroutine->varip[i]];
            PUSH(var);
            //var.type = V_UNKNOWN;
            var = curcoroutine->stackcopy[i - 1];
        }

        PUSH(ret);  // current value always top of the stack
        CoDone();
    }

    void CoResume(CoRoutine *co)
    {
        if (co->stackstart >= 0)
            Error("cannot resume running coroutine");

        if (!co->active)
            Error("cannot resume finished coroutine");

        PUSH(Value(co));    // this will be the return value for the corresponding yield, and holds the ref for gc

        CoNonRec(co->varip);
        sp += co->Resume(sp + 1, stack, ip, curcoroutine);

        curcoroutine = co;

        assert(curcoroutine->stackcopymax >= (size_t)*curcoroutine->varip); // must be, since those vars got backed up in it before
        curcoroutine->stackcopylen = *curcoroutine->varip;
        //curcoroutine->BackupParentVars(vars);

        POP().DEC();    // previous current value

        for (int i = *curcoroutine->varip; i > 0; i--)
        {
            auto &var = vars[curcoroutine->varip[i]];
            curcoroutine->stackcopy[i - 1] = var; // no INC, since parent is still on the stack and hold ref for us
            var = POP();
        }

        // the builtin call takes care of the return value
    }
    
    void Require(const Value &v, int t, const char *op) // FIXME: make this a macro so we don't pass this extra string
    {
        if (v.type != t)
        {
            Error(string("type error: ") + op + " requires value of type " + TypeName(t) + ", instead found " + ProperTypeName(v), v);
        }
    }

    void EndEval(string &evalret)
    {
        evalret = TOP().ToString(programprintprefs);
        POP().DEC();
        TempCleanup();
        FinalStackVarsCleanup();
        DumpLeaks();
        assert(!curcoroutine);
        
        #ifdef VM_PROFILER
            size_t total = 0;
            for (size_t i = 0; i < codelen; i++)
            {
                auto &li = LookupLine(codestart + i); // FIXME: can do faster
                size_t j = &li - &lineinfo[0];
                lineprofilecounts[j] += byteprofilecounts[i];
                total += byteprofilecounts[i];  // FIXME: will overflow
            }
            for (auto &li : lineinfo)
            {
                size_t c = lineprofilecounts[&li - &lineinfo[0]];
                if(c > total / 100)
                    printf("%s(%d): %.1f %%\n", st.filenames[li.fileidx].c_str(), li.line, c * 100.0f / total);
            }
        #endif
    }

    void EvalProgram(string &evalret)
    {
        for (;;)
        {
            #ifdef _DEBUG
                if (trace)
                {
                    DisAsmIns(stdout, st, ip, codestart, LookupLine(ip));
                    if (sp >= 0) printf(" [%d] - %s", sp + 1, TOP().ToString(debugpp).c_str());
                    if (sp >= 1) printf(" / %s", stack[sp - 1].ToString(debugpp).c_str());
                    printf("\n");
                }

                //currentline = LookupLine(ip).line;
            #endif
            
            #ifdef VM_PROFILER
                byteprofilecounts[ip - codestart]++;
            #endif

            int opc = *ip++;

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
                    FunIntro(nargs, codestart + fun, fvar, ip);
                    break;
                }

                case IL_CALLMULTI:
                {
                    auto nargs = *ip++;
                    auto fvar = *ip++;
                    auto fun = st.functiontable[fvar]->bytecodestart;
                    EvalMulti(nargs, codestart + fun, fvar, ip);
                    break;
                }

                case IL_CALLVCOND:
                    if (TOP().type != V_FUNCTION) { ip++; break; } // FIXME: don't need to check for function value again below if false
                case IL_CALLV:
                {
                    Value fun = POP();
                    Require(fun, V_FUNCTION, "function call");
                    auto nargs = *ip++;
                    if (fun.ip) FunIntro(nargs, fun.ip, -1, ip);
                    else CoYield(nargs);
                    break;
                }

                case IL_STORELOOPVAR:
                    loopval[*ip++] = POP();
                    break;

                case IL_DUP:
                {
                    int from = sp - *ip++;
                    PUSH(stack[from].INC());
                    break;
                }

                case IL_FUNSTART:
                    assert(0);

                case IL_FUNEND:
                    FunOut(-1, 1);
                    break;

                case IL_RETURN:
                {
                    int df = *ip++;
                    int nrv = 1;
                    if (df >= 0) nrv = st.functiontable[df]->retvals;   // could encode this in the instruction
                    if(FunOut(df, nrv)) return EndEval(evalret); 
                    break;
                }

                case IL_EXIT:
                    return EndEval(evalret);

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
                    if (n > (int)nf->nargs) Error("native function \"" + nf->name + "\" called with too many arguments");
                    Value v;
                    switch (nf->nargs)
                    {
                        #define ARG(N) Value a##N = POP(); NFCheck(a##N, *nf, N);
                        case 0: {                                                 v = nf->fun.f0();                       break; }
                        case 1: { ARG(0);                                         v = nf->fun.f1(a0);                     break; }
                        case 2: { ARG(1); ARG(0);                                 v = nf->fun.f2(a0, a1);                 break; }
                        case 3: { ARG(2); ARG(1); ARG(0);                         v = nf->fun.f3(a0, a1, a2);             break; }
                        case 4: { ARG(3); ARG(2); ARG(1); ARG(0);                 v = nf->fun.f4(a0, a1, a2, a3);         break; }
                        case 5: { ARG(4); ARG(3); ARG(2); ARG(1); ARG(0);         v = nf->fun.f5(a0, a1, a2, a3, a4);     break; }
                        case 6: { ARG(5); ARG(4); ARG(3); ARG(2); ARG(1); ARG(0); v = nf->fun.f6(a0, a1, a2, a3, a4, a5); break; }
                        default: assert(0); break;
                        #undef ARG
                    }
                    PUSH(v);
                    #ifdef _DEBUG   // see if any builtin function is lying about what type it returns
                        if (nf->ncm == NCM_NONE) // other function types return intermediary values that don't correspond to final return values
                        { 
                            for (int i = 0; i < nf->nretvalues; i++)
                            {
                                auto t = (TOPPTR() - nf->nretvalues + i)->type;
                                auto u = nf->retvals[i].type.t;
                                assert(t == u || u == V_UNKNOWN);   
                            }
                        }
                    #endif
                    break;
                }
                
                case IL_FIELDTABLES:
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

                #define REFOP(exp) { res = exp; a.DEC(); b.DEC(); }
                #define BOP(op, l, r, extras) { if (extras & 1 && r == 0) Div0(); res = l op r; break; }
                #define COP(t, op, l, r, extras) if (b.type == t) BOP(op, l, r, extras)
                #define GETARGS() Value b = POP(); Value a = POP()

                #define _IOP(op, extras) \
                    Value res; \
                    if (a.type != V_INT || b.type != V_INT) BError(#op, a, b); \
                    for (;;) BOP(op, a.ival, b.ival, extras);

                #define _OP(op, extras, opts) Value res; for (;;) { \
                    if (a.type == V_INT) \
                    { \
                        COP(V_INT, op, a.ival, b.ival, extras) \
                        else COP(V_FLOAT, op, float(a.ival), b.fval, extras) \
                    } \
                    else if (a.type == V_FLOAT) \
                    { \
                        COP(V_INT, op, a.fval, float(b.ival), extras) \
                        else COP(V_FLOAT, op, a.fval, b.fval, extras) \
                    } \
                    if ((extras & (8 + 16)) == 0) { \
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
                    opts; \
                    BError(#op, a, b); \
                } 

                #define COMPOPTS(op, extras) \
                    if ((extras & 4) && a.type == V_STRING && b.type == V_STRING) \
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
                    }

                #define IOP(op, extras)      { GETARGS(); _IOP(op, extras);      PUSH(res); break; }
                #define OP(op, extras, opts) { GETARGS(); _OP(op, extras, opts); PUSH(res); break; }

                #define COMPOP(op, extras) OP(op, extras, COMPOPTS(op, extras))
                #define MATHOP(op, extras) OP(op, extras, {})

                case IL_ADD: MATHOP(+, 2);
                case IL_SUB: MATHOP(-, 0);
                case IL_MUL: MATHOP(*, 0);
                case IL_DIV: MATHOP(/, 1);

                case IL_MOD: IOP(%, 1);
                    
                case IL_LT: COMPOP(<,  4);
                case IL_GT: COMPOP(>,  4);
                case IL_LE: COMPOP(<=, 4);
                case IL_GE: COMPOP(>=, 4);
                case IL_EQ: COMPOP(==, (4 + 8));
                case IL_NE: COMPOP(!=, (4 + 16));

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

                #define GETOFFSET(i, vec, mode) \
                    if (mode == 1) { int o1 = *ip++; int o2 = *ip++; i = (i == vec.vval->type) ? o1 : o2; } \
                    if (mode == 2) { i = codestart[i + vec.vval->type]; }
                    
                #define PUSHDEREF(i, dyn, mode) \
                { \
                    Value r = POP(); \
                    switch (r.type) \
                    { \
                        case V_VECTOR: \
                            if (!dyn) { VecType(r); GETOFFSET(i, r, mode); } \
                            IDXErr(i, (int)r.vval->len, r); PUSH(r.vval->at(i).INC()); break; \
                        case V_STRING: if (dyn) { IDXErr(i, r.sval->len, r); PUSH(Value((int)r.sval->str()[i])); break; } /* else fall thru */ \
                        default: Error(string("cannot index into type ") + TypeName(r.type), r); \
                    } \
                    r.DECRT(); \
                    break; \
                }
                case IL_PUSHFLDO: { int i = *ip++; PUSHDEREF(i, false, 0); }
                case IL_PUSHFLDC: { int i = *ip++; PUSHDEREF(i, false, 1); }
                case IL_PUSHFLDT: { int i = *ip++; PUSHDEREF(i, false, 2); }

                case IL_PUSHIDX:
                {
                    Value idx = POP();
                    int i = GrabIndex(idx);
                    PUSHDEREF(i, true, -1);
                }

                case IL_PUSHLOC:
                {
                    int i = *ip++;
                    Value coro = POP();
                    Require(coro, V_COROUTINE, "scoped local variable");
                    PUSH(coro.cval->GetVar(i).INC());
                    coro.DECRT();
                    break;
                }

                case IL_LVALLOC:
                {
                    int lvalop = *ip++;
                    int i = *ip++;
                    Value coro = POP();
                    Require(coro, V_COROUTINE, "scoped local variable");
                    Value &a = coro.cval->GetVar(i);
                    LvalueOp(lvalop, a);
                    coro.DECRT();
                    break;
                }

                #define WRITEDEREFOP(dyn, mode) { \
                    int lvalop = *ip++; \
                    int i; \
                    if (dyn) { \
                        Value idx = POP(); \
                        i = GrabIndex(idx); \
                    } \
                    else { \
                        i = *ip++; \
                    } \
                    Value vec = POP(); \
                    Require(vec, V_VECTOR, "vector indexed assign"); \
                    if (!dyn) { VecType(vec); GETOFFSET(i, vec, mode); } \
                    CheckWritable(vec.vval); \
                    IDXErr(i, (int)vec.vval->len, vec); \
                    Value &a = vec.vval->at(i); \
                    LvalueOp(lvalop, a); \
                    vec.DECRT(); \
                    break; \
                }
                #define PPOP(ret, op, pre) { \
                    if (ret && !pre) PUSH(a.INC()); \
                    if (a.type == V_INT) a.ival = a.ival op 1; \
                    else if (a.type == V_FLOAT) a.fval = a.fval op 1; \
                    else UError(#op, a); \
                    if (ret && pre) PUSH(a.INC()); \
                }
                
                case IL_LVALVAR:   
                {
                    int lvalop = *ip++; 
                    LvalueOp(lvalop, vars[*ip++]);
                    break; \
                }

                case IL_LVALIDX:  WRITEDEREFOP(true, -1);
                case IL_LVALFLDO: WRITEDEREFOP(false, 0);
                case IL_LVALFLDC: WRITEDEREFOP(false, 1);
                case IL_LVALFLDT: WRITEDEREFOP(false, 2);

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

                case IL_TT:       { auto &v = TOP(); int t = *ip++; if (v.type != t) TTError(TypeName(t), v); break; }
                case IL_TTFLT:    { auto &v = TOP(); if (!Coerce(v, V_FLOAT))  TTError("float",  v); break; }
                case IL_TTSTR:    { auto &v = TOP(); if (!Coerce(v, V_STRING)) TTError("string", v); break; }
                case IL_TTSTRUCT: { auto &v = TOP();
                    auto udtid = *ip++;
                    if (v.type == V_VECTOR)
                    {
                        for (int t = v.vval->type; t != -1; t = st.structtable[t]->superclassidx)   // only expensive if long inheritance chain and if often passing subtype values to supertype functions
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

                case IL_COCL:
                    PUSH(Value(NULL, V_FUNCTION));
                    break;

                case IL_CORO:
                    CoNew();
                    break;

                case IL_COEND:
                    CoClean();
                    break;

                default:
                    Error(string("bytecode format problem: ") + inttoa(opc));
            }
        }
    }

    void LvalueOp(int op, Value &a)
    {
        switch(op)
        {
            case LVO_PLUS:  { Value b = POP();  _OP(+, 2, {}); a = res;                  break; }
            case LVO_PLUSR: { Value b = POP();  _OP(+, 2, {}); a = res; PUSH(res.INC()); break; }
            case LVO_MUL:   { Value b = POP();  _OP(*, 0, {}); a = res;                  break; }
            case LVO_MULR:  { Value b = POP();  _OP(*, 0, {}); a = res; PUSH(res.INC()); break; }
            case LVO_SUB:   { Value b = POP();  _OP(-, 0, {}); a = res;                  break; }
            case LVO_SUBR:  { Value b = POP();  _OP(-, 0, {}); a = res; PUSH(res.INC()); break; }
            case LVO_DIV:   { Value b = POP();  _OP(/, 1, {}); a = res;                  break; }
            case LVO_DIVR:  { Value b = POP();  _OP(/, 1, {}); a = res; PUSH(res.INC()); break; }
            case LVO_MOD:   { Value b = POP(); _IOP(%, 1);     a = res;                  break; }
            case LVO_MODR:  { Value b = POP(); _IOP(%, 1);     a = res; PUSH(res.INC()); break; }

            case LVO_WRITE:   { Value  b = POP();       OVERWRITE(a, b); a.DEC(); a = b; break; }
            case LVO_WRITER:  { Value &b = TOP().INC(); OVERWRITE(a, b); a.DEC(); a = b; break; }
            case LVO_WRITED:  { Value  b = POP();                        a.DEC(); a = b; break; } // this last one is only there because OVERWRITE causes problems with rec functions, and its not needed for defines anyway
                    
            case LVO_PP: 
            case LVO_PPR:  { PPOP(op == LVO_PPR,  +, true);  break; }
            case LVO_MM: 
            case LVO_MMR:  { PPOP(op == LVO_MMR,  -, true);  break; }
            case LVO_PPP: 
            case LVO_PPPR: { PPOP(op == LVO_PPPR, +, false); break; }
            case LVO_MMP:
            case LVO_MMPR: { PPOP(op == LVO_MMPR, -, false); break; }

            default:
                Error(string("bytecode format problem (lvalue): ") + inttoa(op));
        }
    }

    //FIXME:
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

    const char *ProperTypeName(const Value &v)
    {
        return v.type == V_VECTOR && v.vval->type >= 0
            ? st.ReverseLookupType(v.vval->type).c_str()
            : TypeName(v.type);
    }

    void TTError(const string &tname, const Value &v)
    {
        Error("function/constructor requires argument of type " + tname + ", instead got " + ProperTypeName(v), v);
    }

    void TTOverwrite(const Value &o, Value &n)
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
            || ot == V_UNKNOWN)    
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

    void CheckWritable(LVector *v)
    {
        if (v->type >= 0 && st.ReadOnlyType(v->type))
            Error(string("can't write to object of value type ") + ProperTypeName(Value(v)));
    }

    bool Coerce(Value &v, int desired)
    {
        if (v.type == desired) return true;
        switch (desired)
        {
            case V_UNKNOWN: return true;  // this means "any" for builtin functions, not used by other callers
            case V_FLOAT:   if (v.type == V_INT)    { v = Value((float)v.ival); return true; } break;
            case V_STRING:  if (v.type != V_STRING) { auto s = v.ToString(programprintprefs); v.DEC(); v = Value(NewString(s)); return true; } break;
        }
        return false;
    }

    void BuiltinCheck(Value &v, int desired, const char *name)
    {       
        if (!Coerce(v, desired))
        {
            Error(string(ProperTypeName(v)) + "is not a valid argument type for native function \"" + name + "\"", v);
        }
    }

    void NFCheck(Value &v, const NativeFun &nf, int i)
    {       
        if (!Coerce(v, nf.args[i].type.t))
        {
            if ((nf.args[i].flags == NF_OPTIONAL) && v.type == V_NIL) return;
            Error(string("argument ") + inttoa(i + 1) + " of native function \"" + nf.name + "\" needs to have type " + nf.args[i].type.Name() + ", not " + ProperTypeName(v), v);
        }
    }

    bool StrOps(const Value &a, const Value &b, Value &res)
    {
        if      (a.type == V_STRING) { string s = b.ToString(programprintprefs); res = NewString(a.sval->str(), a.sval->len, (char *)s.c_str(), (int)s.size()); a.DEC(); b.DEC(); return true; }
        else if (b.type == V_STRING) { string s = a.ToString(programprintprefs); res = NewString((char *)s.c_str(), (int)s.size(), b.sval->str(), b.sval->len); a.DEC(); b.DEC(); return true; }
        return false;
    }

    void BError(const char *op, const Value &a, const Value &b) { Error(string("binary operator ") + op + " cannot operate on " + ProperTypeName(a) + " and " + ProperTypeName(b), a, b); }
    void UError(const char *op, const Value &a)                 { Error(string("unary operator ")  + op + " cannot operate on " + ProperTypeName(a), a); }
    void Div0()                                                 { Error("division by zero"); }
    
    void IDXErr(int i, int n, const Value &v)                   { if (i < 0 || i >= n) Error(string("index ") + string(inttoa(i)) + " out of range " + string(inttoa(n)), v); }
    void VecType(const Value &vec)                              { if (vec.vval->type < 0) Error("cannot use field dereferencing on untyped vector", vec); }

    bool AllInt(const LVector *v)
    {
        for (int i = 0; i < v->len; i++)
            if (v->at(i).type != V_INT)
                return false;
        return true;
    }

    int GrabIndex(const Value &idx)
    {
        if (idx.type == V_INT) return idx.ival;

        if (idx.type == V_VECTOR)
        {
            auto &v = TOP();
            for (int i = idx.vval->len - 1; ; i--)
            {
                auto &sidx = idx.vval->at(i);
                if (sidx.type != V_INT) Error(string("illegal vector index element of type ") + ProperTypeName(sidx), idx);
                if (!i)
                {
                    idx.DECRT();
                    return sidx.ival;
                }
                if (v.type != V_VECTOR) Error(string("vector index of length ") + inttoa(idx.vval->len) + " used on nested vector of depth " + inttoa(i), idx, v); 
                IDXErr(sidx.ival, v.vval->len, v);
                auto nv = v.vval->at(sidx.ival).INC();
                v.DECRT();
                v = nv;
            }
        }

        Error(string("index of type int or vector of int required, instead found: ") + ProperTypeName(idx), idx);
        return 0;
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
                if(a.vval->len < b.vval->len || (a.vval->len == b.vval->len && a.vval->type >= 0))
                {
                    if (a.vval->refc == 1) { res = a; return len; } else type = a.vval->type;
                }
                else
                {
                    if (b.vval->refc == 1) { res = b; return len; } else type = b.vval->type;
                }
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

    int VectorTrim(const Value &a, int len)
    {
        while(a.vval->len > len) a.vval->pop().DEC();
        return len;
    }

    void VectorDec(const Value &a, const Value &res)
    {
        if (a.type == V_VECTOR && a.vval != res.vval) a.DEC();
    }

    template<typename T> T VectorElem(const Value &a, int i)
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
                        Error(string("can't do vector operation with vector element type ") + ProperTypeName(v), v);
                        return 0;
                }
            }
            default:
                assert(0);
                return 0;
        }
    }

    Value loopval[2]; // FIXME: hack
    Value LoopVal(int i) { return loopval[i]; }

    void Push(const Value &v) { PUSH(v); }
    Value Pop() { return POP(); }

    int StructIdx(const string &name, size_t &nargs) { return st.StructIdx(name, nargs); }
    virtual string &ReverseLookupType(uint v) { return st.ReverseLookupType(v); }

    #undef PUSH
    #undef TOP
    #undef POP
    #undef TOPPTR

    void Trace(bool on) { trace = on; }
    float Time() { return (float)SecondsSinceStart(); }

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
            if (r->type == V_VALUEBUF) continue;
            if (r->refc > 0) leaks.push_back(r);
            r->refc = -r->refc;
        }

        for (auto p : leaks)
        {
            auto ro = (RefObj *)p;
            ro->refc = 0;
            Value v(ro);
            switch (ro->type)
            {
                case V_VECTOR:    v.vval->len = 0; v.vval->deleteself(); break;
                case V_STRING:                     v.sval->deleteself(); break;
                case V_COROUTINE:                  v.cval->deleteself(false); break;
                default: assert(0);
            }
        }

        return leaks.size();
    }
};
