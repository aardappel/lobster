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

namespace lobster
{

#ifdef _DEBUG
    #define VM_PROFILER                     // tiny VM slowdown and memory usage when enabled
#endif

struct VM : VMBase
{
    #include "vmlog.h"

    Value *stack;
    int stacksize;
    int maxstacksize;
    int sp;

    enum
    {
        INITSTACKSIZE   =   4 * 1024, // *8 bytes each
        DEFMAXSTACKSIZE = 128 * 1024, // *8 bytes each, modest on smallest handheld we support (iPhone 3GS has 256MB)
        STACKMARGIN     =   1 * 1024  // *8 bytes each, max by which the stack could possibly grow in a single call
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

    VMLog vml;

    #define PUSH(v) (stack[++sp] = (v))
    #define TOP() (stack[sp])
    #define TOP2() (stack[sp - 1])
    #define TOP3() (stack[sp - 2])
    #define POP() (stack[sp--]) // (sp < 0 ? 0/(sp + 1) : stack[sp--])
    #define TOPPTR() (stack + sp + 1)
    #define OVERWRITE(o, n) TTOverwrite(o, n)

    VM(SymbolTable &_st, int *_code, int _len, const vector<LineInfo> &_lineinfo, const char *_pn)
        : stack(nullptr), stacksize(0), maxstacksize(DEFMAXSTACKSIZE), sp(-1), ip(nullptr),
          curcoroutine(nullptr), vars(nullptr), st(_st), codelen(_len), byteprofilecounts(nullptr), lineprofilecounts(nullptr),
          trace(false), lineinfo(_lineinfo), debugpp(2, 50, true, -1), programname(_pn), vml(*this, st.uses_frame_state)
    {
        // search for "64bit" before trying to make a 64bit build, changes may be required
        assert(sizeof(int) == sizeof(void *));
        assert(vmpool == nullptr);
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

        vml.LogInit();

        // TODO: this isn't great hardcoded in the compiler, would be better if it was declared in lobster code
        static const char *default_vector_type_names[] = { "xy", "xyz", "xyzw", nullptr };
        for (auto name = default_vector_type_names; *name; name++)
        {
            int t = V_VECTOR;
            // linear search because we may not have the map available
            for (auto s : st.structtable) if (s->name == *name) { t = s->idx; break; }
            default_vector_types.push_back(t);
        }

        assert(g_vm == nullptr);
        g_vm = this;
    }

    ~VM()
    {
        assert(g_vm == this);
        g_vm = nullptr;

        if (stack) delete[] stack;
        if (vars)  delete[] vars;

        if (byteprofilecounts) delete[] byteprofilecounts;
        if (lineprofilecounts) delete[] lineprofilecounts;

        if (vmpool)
        {
            delete vmpool;
            vmpool = nullptr;
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
            printf("\nLEAKS FOUND (this indicates cycles in your object graph, or a bug in Lobster,"
                   " details in leaks.txt)\n");
                    
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

    const LineInfo &LookupLine(int *ip) { return lobster::LookupLine(ip - codestart, lineinfo); }
    
    #undef new
    LVector *NewVector(int n, int t) { return new (vmpool->alloc(sizeof(LVector) + sizeof(Value) * n)) LVector(n, t); }
    LString *NewString(int l) { return new (vmpool->alloc(sizeof(LString) + l + 1)) LString(l); }
    CoRoutine *NewCoRoutine(int *rip, int *vip, CoRoutine *p)
    {
        return new (vmpool->alloc(sizeof(CoRoutine))) CoRoutine(sp + 2 /* top of sp + pushed coro */, rip, vip, p);
    }
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
            if (TOP().type != V_UNDEFINED)
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
            int deffun = varcleanup(s.length() < 10000 ? &locals : nullptr);

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

        DebugLog(1, "%s", s.c_str());
        throw s;
    }

    Value BuiltinError(string err) { return Error(err); }

    void VMAssert(bool ok, const char *what) 
    {
        if (!ok) Error(string("VM internal assertion failure: ") + what); 
    }
    void VMAssert(bool ok, const char *what, const Value &a, const Value &b) 
    {
        if (!ok) Error(string("VM internal assertion failure: ") + what, a, b); 
    }

    #ifdef _DEBUG
        #define STRINGIFY(x) #x
        #define TOSTRING(x) STRINGIFY(x)
        #define VMASSERT(test)             VMAssert(test, __FILE__ ": " TOSTRING(__LINE__) ": " #test)
        #define VMASSERTVALUES(test, a, b) VMAssert(test, __FILE__ ": " TOSTRING(__LINE__) ": " #test, a, b)
    #else
        #define VMASSERT(test)
        #define VMASSERTVALUES(test, a, b)
    #endif

    string ValueDBG(const Value &a)
    {
        int found = 0;
        int nfound = 0;
        for (size_t i = 0; i < st.identtable.size(); i++) if (a.Equal(vars[i], false)) { found = i; nfound++; }
        string s = a.ToString(debugpp);
        if (nfound == 1) s += " (" + st.ReverseLookupIdent(found) + " ?)";
        return s;
    }

    string DumpVar(const Value &x, SymbolTable &st, int idx)
    {
        if (x.type == V_UNDEFINED) return "";
        if (st.ReadOnlyIdent(idx)) return "";
        return "\n   " + st.ReverseLookupIdent(idx) + " = " + x.ToString(debugpp);
    }

    void EvalMulti(int nargs, int *ip, int definedfunction, int *retip)
    {
        VMASSERT(*ip == IL_FUNMULTI);
        ip++;

        auto nsubf = *ip++;
        auto table_nargs = *ip++;
        VMASSERT(nargs == table_nargs);
        (void)table_nargs;
        for (int i = 0; i < nsubf; i++)
        {
            // TODO: rather than going thru all args, only go thru those that have types
            for (int j = 0; j < nargs; j++)
            {
                int desired = *ip++;
                if (desired != V_ANY)
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

            return FunIntro(nargs, codestart + *ip, definedfunction, retip);

            fail:;
        }

        string argtypes;
        for (int j = 0; j < nargs; j++)
        {
            argtypes += ProperTypeName(stack[sp - nargs + j + 1]);
            if (j < nargs - 1) argtypes += ", ";
        }
        Error("the call " + st.ReverseLookupFunction(definedfunction) + "(" + argtypes +
              ") did not match any function variants");
    }

    void FinalStackVarsCleanup()
    {
        VMASSERT(sp < 0);

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

    void LogFrame() { vml.LogFrame(); }

    void TempCleanup()
    {
        while (sp >= 0 && TOP().type != V_DEFFUN) {
            // only if from a return or error thats has tempories above it, and if returning thru a control structure
            POP().DEC();
        }
    }
    
    int varcleanup(string *error)
    {
        auto dfv = POP(); VMASSERT(dfv.type == V_DEFFUN);       auto deffun = dfv.ival;
        auto ipv = POP(); VMASSERT(ipv.type == V_FUNSTART);     ip = ipv.ip; 
        auto nav = POP(); VMASSERT(nav.type == V_NARGS);        auto nargs_given = nav.ival;
        auto riv = POP(); VMASSERT(riv.type == V_RETIP);        auto retip = riv.ip;

        auto nargs_fun = *ip++;
        auto freevars = ip + nargs_given;
        ip += nargs_fun;
        auto ndef = *ip++;
        auto defvars = ip + ndef;

        if (vml.uses_frame_state)
        {
            auto lfr = POP(); VMASSERT(lfr.type == V_LOGFUNREADSTART);
            auto lfw = POP(); VMASSERT(lfw.type == V_LOGFUNWRITESTART);
            vml.LogFunctionExit(ipv.ip, defvars, lfw.ival);
        }

        while (ndef--)  { auto i = *--defvars;  if (error) (*error) += DumpVar(vars[i], st, i); vars[i].DEC();
                                                                                                vars[i] = POP(); }
        while (nargs_given--) { auto i = *--freevars; if (error) (*error) += DumpVar(vars[i], st, i); vars[i].DEC();
                                                                                                vars[i] = POP(); } 

        ip = retip;

        return deffun;
    }

    void FunIntroOrYield(int nargs_given, int *newip, int definedfunction, int *retip)
    {
        if (newip != (int *)Value::FAKE_COCLOSURE_ADDRESS) FunIntro(nargs_given, newip, definedfunction, retip);
        else CoYield(nargs_given, retip);
    }

    void FunIntro(int nargs_given, int *newip, int definedfunction, int *retip)
    {
        ip = newip;

        VMASSERT(*ip == IL_FUNSTART);
        ip++;

        auto funstart = ip;

        if (sp > stacksize - STACKMARGIN)   // per function call increment should be small
        {                                   // FIXME: not safe for untrusted scripts, could simply add lots of locals
                                            // could record max number of locals? not allow more than N locals?
            if (stacksize >= maxstacksize) Error("stack overflow! (use set_max_stack_size() if needed)");
            auto nstack = new Value[stacksize *= 2];
            memcpy(nstack, stack, sizeof(Value) * (sp + 1));
            delete[] stack;
            stack = nstack;

            DebugLog(0, (string("stack grew to: ") + inttoa(stacksize)).c_str());
        }

        auto nargs_fun = *ip++;
        if (nargs_given != nargs_fun)
        {
            // This only ever happens when called from IL_CALLV, for IL_CALL, these two are are guaranteed to be equal by the parser.
            if (nargs_given > nargs_fun)
            {
                // This used to be an error, since the only uses of this were in builtin functions like for/map,
                // but since map is now not a builtin anymore, we support superfluous args on all dynamic calls.
                // This is actually nice functionality, since very similarly to multiple return values, we should be
                // able to choose what values we find useful.
                // In the future, this code can become an assert, when the type system specializes every HOF call.

                // string nas = inttoa(nargs_given);
                // Error(string("function value called with ") + nas + " arguments, but declared with only " + inttoa(nargs_fun));

                // Instead, simply discard superfluous args:
                for (; nargs_given > nargs_fun; nargs_given--) POP().DEC();
            }
            else
            {
                // nargs_given < nargs_fun currently may still happen when a body is declared with more parameters than it needs:
                // hof_supplies_no_args() x: x + 1
                // Here, x would resolve to undefined and lead to a runtime error.
                // more typically, this happens with the _ var:
                // hof_supplies_one_arg(): _ + hof_supplies_no_args(): _
                // This actually works as expected. This usuage used to work for if/while/for and will likely be disabled
                // for all other hofs as well with typechecking, so this will become an assert instead.

                // assert(0);
            }
        }
        
        for (int i = 0; i < nargs_given; i++) swap(vars[ip[i]], stack[sp - nargs_given + i + 1]);
        ip += nargs_fun;

        auto ndef = *ip++;
        for (int i = 0; i < ndef; i++)
        {
            // for most locals, this just saves an undefined, only in recursive cases it has an actual value.
            // The reason we don't clear the var after backing it up is that in the DS case,
            // you want to be able to use the old value until a new one gets defined, as in a <- a + 1.
            // clearing it would save the INC and a DEC when it eventually gets overwritten,
            // so maybe we can at some point distinguish between vars that are used with DS and those that are not.
            // for recursive functions it can be problematic with TTOVERWRITE check, but we fixed this temp by using
            // a separate instruction for assign + def
            PUSH(vars[*ip++].INC());
        }
        auto nlogvars = *ip++;

        if (vml.uses_frame_state)
        {
            PUSH(Value((int)vml.LogFunctionEntry(funstart, nlogvars), V_LOGFUNWRITESTART));
            PUSH(Value((int)vml.logi - nlogvars, V_LOGFUNREADSTART));
        }

        // FIXME: can we reduce the amount of this? -> stick it in a struct on a seperate stack?
        PUSH(Value(retip, V_RETIP)); 
        PUSH(Value(nargs_given, V_NARGS)); 
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
                if (towhere >= 0)
                    Error("\"return from " + st.ReverseLookupFunction(towhere) + "\" outside of function");
                bottom = true;
                break;
            }
            int deffun = varcleanup(nullptr);
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
        {
            // if allowed, inner coro would save vars of outer, and then possibly restore them outside of scope
            // of parent
            Error("cannot create coroutine recursively");
        }
        // TODO: this check guarantees all saved stack vars are undef, except for DS vars,
        // which could still cause problems
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

    void CoDone(int *retip)
    {
        int newtop = curcoroutine->Suspend(sp + 1, stack, retip, curcoroutine);
        ip = retip;
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
        CoDone(ip);
        VMASSERT(co->stackcopylen == 1);
        co->active = false;
    }

    void CoYield(int nargs_given, int *retip)
    {
        if (nargs_given > 1)
        {
            // Error("more than 1 argument supplied to coroutine yield function");

            // Similar to FunIntro, we support an excess in nargs_given for the moment:
            for (; nargs_given > 1; nargs_given--) POP().DEC();
        }
        if (!curcoroutine)
        {
            // can theoretically happen if programmer caches yield value somewhere
            Error("coroutine yield function called outside of context");
        }

        Value ret(0, V_NIL);  
        if (nargs_given) ret = POP();

        for (int i = 1; i <= *curcoroutine->varip; i++)
        {
            auto &var = vars[curcoroutine->varip[i]];
            PUSH(var);
            //var.type = V_UNDEFINED;
            var = curcoroutine->stackcopy[i - 1];
        }

        PUSH(ret);  // current value always top of the stack
        CoDone(retip);
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

        // must be, since those vars got backed up in it before
        VMASSERT(curcoroutine->stackcopymax >= (size_t)*curcoroutine->varip);
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

    void Require(const Value &v, ValueType t, const char *op) // FIXME: make this a macro so we don't pass this extra string
    {
        if (v.type != t)
        {
            Error(string("type error: ") + op + " requires value of type " + BaseTypeName(t) + 
                  ", instead found " + ProperTypeName(v), v);
        }
    }

    void EndEval(string &evalret)
    {
        evalret = TOP().ToString(programprintprefs);
        POP().DEC();
        TempCleanup();
        FinalStackVarsCleanup();
        vml.LogCleanup();
        DumpLeaks();
        VMASSERT(!curcoroutine);
        
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
                    // FIXME: have a way that constant strings can stay in the bytecode,
                    // or at least preallocate them all
                    auto s = NewString(len - 1);
                    for (int i = 0; i < len; i++) s->str()[i] = (char)start[i]; 
                    PUSH(Value(s));
                    break;
                }

                case IL_CALL:
                {
                    auto nargs = *ip++;
                    auto fvar = *ip++;
                    auto fun = *ip++;
                    FunIntro(nargs, codestart + fun, fvar, ip);
                    break;
                }

                case IL_CALLMULTI:
                {
                    auto nargs = *ip++;
                    auto fvar = *ip++;
                    auto fun = *ip++;
                    EvalMulti(nargs, codestart + fun, fvar, ip);
                    break;
                }

                case IL_CALLVCOND:
                    // FIXME: don't need to check for function value again below if false
                    if (TOP().type != V_FUNCTION) { ip++; break; }
                case IL_CALLV:
                {
                    Value fun = POP();
                    Require(fun, V_FUNCTION, "function call");
                    auto nargs = *ip++;
                    FunIntroOrYield(nargs, fun.ip, -1, ip);
                    break;
                }

                case IL_DUP:
                {
                    int from = sp - *ip++;
                    PUSH(stack[from].INC());
                    break;
                }

                case IL_FUNSTART:
                    VMASSERT(0);

                case IL_FUNEND:
                    FunOut(-1, 1);
                    break;

                case IL_RETURN:
                {
                    int df = *ip++;
                    int nrv = 1;
                    if (df >= 0) nrv = st.functiontable[df]->retvals;   // TODO: could encode this in the instruction
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

                case IL_FOR:
                {
                    auto forstart = ip - 1;
                    POP().DEC();  // body retval
                    auto &body = TOP();
                    auto &iter = TOP2();
                    auto &i = TOP3();
                    assert(i.type == V_INT); 
                    i.ival++;
                    int len = 0;
                    switch (iter.type)
                    {
                        #define PUSHITER(L, V) if (i.ival >= (len = L)) goto done; PUSH(V); break;
                        case V_INT:    PUSHITER(iter.ival     , i);
                        case V_VECTOR: PUSHITER(iter.vval->len, iter.vval->at(i.ival).INC());
                        case V_STRING: PUSHITER(iter.sval->len, Value((int)((uchar *)iter.sval->str())[i.ival]));
                        #undef PUSHITER
                        default:       Error("for: cannot iterate over argument", iter);
                    }
                    PUSH(i);
                    FunIntroOrYield(2, body.ip, -1, forstart);
                    break;

                    done:
                    POP();        // body
                    POP().DEC();  // iter
                    POP();        // i
                    break;
                }

                case IL_BCALL:
                {
                    auto nf = natreg.nfuns[*ip++];
                    int n = *ip++;
                    if (n > (int)nf->args.v.size())
                        Error("native function \"" + nf->name + "\" called with too many arguments");
                    Value v;
                    switch (nf->args.v.size())
                    {
                        #define ARG(N) Value a##N = POP(); NFCheck(a##N, *nf, N);
                        case 0: {                                           v = nf->fun.f0(); break; }
                        case 1: { ARG(0)                                    v = nf->fun.f1(a0); break; }
                        case 2: { ARG(1) ARG(0)                             v = nf->fun.f2(a0, a1); break; }
                        case 3: { ARG(2) ARG(1) ARG(0)                      v = nf->fun.f3(a0, a1, a2); break; }
                        case 4: { ARG(3) ARG(2) ARG(1) ARG(0)               v = nf->fun.f4(a0, a1, a2, a3); break; }
                        case 5: { ARG(4) ARG(3) ARG(2) ARG(1) ARG(0)        v = nf->fun.f5(a0, a1, a2, a3, a4); break; }
                        case 6: { ARG(5) ARG(4) ARG(3) ARG(2) ARG(1) ARG(0) v = nf->fun.f6(a0, a1, a2, a3, a4, a5);
                                                                                                                break; }
                        default: VMASSERT(0); break;
                        #undef ARG
                    }
                    PUSH(v);
                    #ifdef _DEBUG   // see if any builtin function is lying about what type it returns
                        // other function types return intermediary values that don't correspond to final return values
                        if (nf->ncm == NCM_NONE)
                        { 
                            for (size_t i = 0; i < nf->retvals.v.size(); i++)
                            {
                                auto t = (TOPPTR() - nf->retvals.v.size() + i)->type;
                                auto u = nf->retvals.v[i].type.t;
                                VMASSERT(t == u || u == V_ANY);   
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
                #define BOP(op, l, r, extras) { if (extras & 1 && r == 0) Div0(); res = l op r; }
                #define COP(t, op, l, r, extras) if (b.type == t) { BOP(op, l, r, extras); break; }
                #define GETARGS() Value b = POP(); Value a = POP()
                #define TYPEOP(op, extras, field, errstat) Value res; errstat; BOP(op, a.field, b.field, extras);

                #define _IOP(op, extras) TYPEOP(op, extras, ival, VMASSERTVALUES(a.type == V_INT && b.type == V_INT, a, b))
                #define _FOP(op, extras) TYPEOP(op, extras, fval, VMASSERTVALUES(a.type == V_FLOAT && b.type == V_FLOAT, a, b))
                #define _AIOP(op, extras) TYPEOP(op, extras, ival, if (a.type != V_INT || b.type != V_INT) BError(#op, a, b))

                #define _AOP(op, extras, opts) Value res; for (;;) { \
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
                            if (isfloat) { auto bv = VectorElem<float>(b, j); if (extras&1 && bv == 0) Div0(); \
                                           res.vval->at(j) = Value(VectorElem<float>(a, j) op bv); }\
                            else         { auto bv = VectorElem<int>  (b, j); if (extras&1 && bv == 0) Div0(); \
                                           res.vval->at(j) = Value(VectorElem<int>  (a, j) op bv); }\
                            VectorDec(a, res); VectorDec(b, res); \
                            break; } \
                    } \
                    if (extras & 2) if (StrOps(a, b, res)) break; \
                    opts; \
                    BError(#op, a, b); \
                } 

                #define ACOMPOPTS(op, extras) \
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

                #define AIOP(op, extras)      { GETARGS(); _AIOP(op, extras);      PUSH(res); break; }
                #define IOP(op, extras)       { GETARGS(); _IOP(op, extras);       PUSH(res); break; }
                #define FOP(op, extras)       { GETARGS(); _FOP(op, extras);       PUSH(res); break; }
                #define AOP(op, extras, opts) { GETARGS(); _AOP(op, extras, opts); PUSH(res); break; }

                #define ACOMPOP(op, extras) AOP(op, extras, ACOMPOPTS(op, extras))
                #define AMATHOP(op, extras) AOP(op, extras, {})

                case IL_AADD: AMATHOP(+, 2);
                case IL_ASUB: AMATHOP(-, 0);
                case IL_AMUL: AMATHOP(*, 0);
                case IL_ADIV: AMATHOP(/, 1);
                case IL_AMOD: AIOP(%, 1);
                case IL_ALT:  ACOMPOP(<,  4);
                case IL_AGT:  ACOMPOP(>,  4);
                case IL_ALE:  ACOMPOP(<=, 4);
                case IL_AGE:  ACOMPOP(>=, 4);
                case IL_AEQ:  ACOMPOP(==, (4 + 8));
                case IL_ANE:  ACOMPOP(!=, (4 + 16));
                    
                case IL_IADD: IOP(+, 0);
                case IL_ISUB: IOP(-, 0);
                case IL_IMUL: IOP(*, 0);
                case IL_IDIV: IOP(/ , 1);
                case IL_IMOD: IOP(%, 1);
                case IL_ILT:  IOP(<, 0);
                case IL_IGT:  IOP(>, 0);
                case IL_ILE:  IOP(<=, 0);
                case IL_IGE:  IOP(>=, 0);
                case IL_IEQ:  IOP(==, 0);
                case IL_INE:  IOP(!=, 0);
                
                case IL_FADD: FOP(+, 0);
                case IL_FSUB: FOP(-, 0);
                case IL_FMUL: FOP(*, 0);
                case IL_FDIV: FOP(/, 1);
                case IL_FLT:  FOP(<, 0);
                case IL_FGT:  FOP(>, 0);
                case IL_FLE:  FOP(<=, 0);
                case IL_FGE:  FOP(>=, 0);
                case IL_FEQ:  FOP(==, 0);
                case IL_FNE:  FOP(!=, 0);

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

                case IL_I2F:
                {
                    Value a = POP();
                    VMASSERT(a.type == V_INT);
                    PUSH((float)a.ival);    
                    break;
                }                
                
                case IL_A2S:
                {
                    Value a = POP();
                    PUSH(NewString(a.ToString(programprintprefs)));   
                    a.DEC();
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
                        case V_STRING: if (dyn) { IDXErr(i, r.sval->len, r); \
                                                  PUSH(Value((int)r.sval->str()[i])); break; } /* else fall thru */ \
                        default: Error(string("cannot index into type ") + BaseTypeName(r.type), r); \
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
                    VMASSERT(v.type == V_VECTOR);
                    v.vval->push(x);
                    break;
                }

                case IL_PUSHPARENT:
                {
                    auto x = POP();
                    auto &v = TOP();
                    VMASSERT(v.type == V_VECTOR);
                    if (x.type != V_VECTOR || *ip++ != x.vval->type)
                        Error("super class constructor is of the wrong type", x);
                    v.vval->append(x.vval, 0, x.vval->len);
                    //for (int i = 0; i < x.vval->len; i++) v.vval->push(x.vval->at(i));
                    x.DECRT();
                    break;
                }

                case IL_JUMPFAIL:    { auto x = POP(); auto nip = *ip++; if (!x.DEC().True()) { ip = codestart + nip;          }               break; }
                case IL_JUMPFAILR:   { auto x = POP(); auto nip = *ip++; if (!x      .True()) { ip = codestart + nip; PUSH(x); } else x.DEC(); break; }
                case IL_JUMPNOFAIL:  { auto x = POP(); auto nip = *ip++; if ( x.DEC().True()) { ip = codestart + nip;          }               break; }
                case IL_JUMPNOFAILR: { auto x = POP(); auto nip = *ip++; if ( x      .True()) { ip = codestart + nip; PUSH(x); } else x.DEC(); break; }

                case IL_TT:       { auto &v = TOP(); auto t = (ValueType)*ip++; if (v.type != t) TTError(BaseTypeName(t), v); break; }
                case IL_TTFLT:    { auto &v = TOP(); if (!Coerce(v, V_FLOAT))  TTError("float",  v); break; }
                case IL_TTSTR:    { auto &v = TOP(); if (!Coerce(v, V_STRING)) TTError("string", v); break; }
                case IL_TTSTRUCT: { auto &v = TOP();
                    auto udtid = *ip++;
                    if (v.type == V_VECTOR)
                    {
                        // only expensive if long inheritance chain and if often passing subtype values to
                        // supertype functions
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

                case IL_COCL:
                    PUSH(Value((int *)Value::FAKE_COCLOSURE_ADDRESS, V_FUNCTION));
                    break;

                case IL_CORO:
                    CoNew();
                    break;

                case IL_COEND:
                    CoClean();
                    break;

                case IL_LOGREAD:
                {
                    auto val = POP();
                    PUSH(vml.LogGet(val, *ip++));
                    break;
                }

                default:
                    Error(string("bytecode format problem: ") + inttoa(opc));
            }
        }
    }

    void LvalueOp(int op, Value &a)
    {
        switch(op)
        {
            case LVO_PLUS:  { Value b = POP();  _AOP(+, 2, {}); a = res;                  break; }
            case LVO_PLUSR: { Value b = POP();  _AOP(+, 2, {}); a = res; PUSH(res.INC()); break; }
            case LVO_MUL:   { Value b = POP();  _AOP(*, 0, {}); a = res;                  break; }
            case LVO_MULR:  { Value b = POP();  _AOP(*, 0, {}); a = res; PUSH(res.INC()); break; }
            case LVO_SUB:   { Value b = POP();  _AOP(-, 0, {}); a = res;                  break; }
            case LVO_SUBR:  { Value b = POP();  _AOP(-, 0, {}); a = res; PUSH(res.INC()); break; }
            case LVO_DIV:   { Value b = POP();  _AOP(/, 1, {}); a = res;                  break; }
            case LVO_DIVR:  { Value b = POP();  _AOP(/, 1, {}); a = res; PUSH(res.INC()); break; }
            case LVO_MOD:   { Value b = POP(); _AIOP(%, 1);     a = res;                  break; }
            case LVO_MODR:  { Value b = POP(); _AIOP(%, 1);     a = res; PUSH(res.INC()); break; }

            case LVO_WRITE:   { Value  b = POP();       OVERWRITE(a, b); a.DEC(); a = b; break; }
            case LVO_WRITER:  { Value &b = TOP().INC(); OVERWRITE(a, b); a.DEC(); a = b; break; }
            case LVO_WRITED:  { Value  b = POP();                        a.DEC(); a = b; break; }
            // LVO_WRITED is only there because OVERWRITE causes problems with rec functions,
            // and its not needed for defines anyway
                    
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
        return v.type == V_VECTOR && v.vval->type >= 0 ? ReverseLookupType(v.vval->type).c_str() : BaseTypeName(v.type);
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
                // the code below only does super->sub and sub->super,
                // but can't do common supertype which is too expensive at runtime
                // overwrite with a subclass:
                // if (st.IsSuperTypeOrSame(ot, nt)) return;
                // overwrite with a superclass:
                // if (st.IsSuperTypeOrSame(nt, ot)) return;
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

    void CheckWritable(LVector *v)
    {
        if (v->type >= 0 && st.ReadOnlyType(v->type))
            Error(string("can't write to object of value type ") + ProperTypeName(Value(v)));
    }

    bool Coerce(Value &v, ValueType desired)
    {
        if (v.type == desired) return true;
        switch (desired)
        {
            case V_ANY: return true;  // only used by native functions, not used by other callers
            case V_FLOAT:   if (v.type == V_INT) { v = Value((float)v.ival); return true; } break;
            case V_STRING:  if (v.type != V_STRING)
                            {
                                auto s = v.ToString(programprintprefs);
                                v.DEC();
                                v = Value(NewString(s));
                                return true;
                            }
                            break;
        }
        return false;
    }

    void BuiltinCheck(Value &v, ValueType desired, const char *name)
    {       
        if (!Coerce(v, desired))
        {
            Error(string(ProperTypeName(v)) + "is not a valid argument type for native function \"" + name + "\"", v);
        }
    }

    void NFCheck(Value &v, const NativeFun &nf, int i)
    {       
        if (!Coerce(v, nf.args.v[i].type.t))
        {
            if (nf.args.v[i].type.t == V_NILABLE)
            {
                if (v.type == V_NIL || Coerce(v, nf.args.v[i].type.t2)) return;
            }
            Error(string("argument ") + inttoa(i + 1) + " of native function \"" + nf.name + 
                  "\" needs to have type " + st.TypeName(nf.args.v[i].type) + ", not " + ProperTypeName(v), v);
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
                auto sidx = idx.vval->at(i);
                if (sidx.type != V_INT)
                    Error(string("illegal vector index element of type ") + ProperTypeName(sidx), idx);
                if (!i)
                {
                    idx.DECRT();
                    return sidx.ival;
                }
                if (v.type != V_VECTOR)
                    Error(string("vector index of length ") + inttoa(idx.vval->len) + 
                          " used on nested vector of depth " + inttoa(i), idx, v); 
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
                VMASSERT(0);
                return 0;
        }
    }

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
        vml.LogMark();

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
                default: VMASSERT(0);
            }
        }

        return leaks.size();
    }
};

}  // namespace lobster
