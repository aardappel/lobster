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

#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"
#include "il.h"

#include "bytecode_generated.h"

#include "disasm.h"

namespace lobster
{

VMBase *g_vm = nullptr;                    // set during the lifetime of a VM object
SlabAlloc *vmpool = nullptr;               // set during the lifetime of a VM object

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

    const int *ip;

    CoRoutine *curcoroutine;

    Value *vars;
    
    size_t codelen;
    const int *codestart;
    vector<int> codebigendian;
    uint64_t *byteprofilecounts;
    
    const bytecode::BytecodeFile *bcf;

    int currentline;
    int maxsp;

    PrintPrefs debugpp;

    const char *programname;

    VMLog vml;

    bool trace;
    bool trace_tail;
    string trace_output;

    #define PUSH(v) (stack[++sp] = (v))
    #define TOP() (stack[sp])
    #define TOP2() (stack[sp - 1])
    #define TOP3() (stack[sp - 2])
    #define POP() (stack[sp--]) // (sp < 0 ? 0/(sp + 1) : stack[sp--])
    #define TOPPTR() (stack + sp + 1)

    VM(const char *_pn, const uchar *bytecode_buffer)
        : stack(nullptr), stacksize(0), maxstacksize(DEFMAXSTACKSIZE), sp(-1), ip(nullptr),
          curcoroutine(nullptr), vars(nullptr), codelen(0), codestart(nullptr), byteprofilecounts(nullptr), 
          bcf(bytecode::GetBytecodeFile(bytecode_buffer)),
          currentline(-1), maxsp(-1),
          debugpp(2, 50, true, -1), programname(_pn), vml(*this, bcf->uses_frame_state() != 0),
          trace(false), trace_tail(true)
    {
        assert(vmpool == nullptr);
        vmpool = new SlabAlloc();

        if (bcf->bytecode_version() != LOBSTER_BYTECODE_FORMAT_VERSION)
            throw "bytecode is from a different version of Lobster";

        codelen = bcf->bytecode()->Length();
        if (FLATBUFFERS_LITTLEENDIAN)
        {
            // We can use the buffer directly.
            codestart = (const int *)bcf->bytecode()->Data();
        }
        else
        {
            for (uint i = 0; i < codelen; i++) codebigendian.push_back(bcf->bytecode()->Get(i));
            codestart = codebigendian.data();
        }
        ip = codestart;

        vars = new Value[bcf->idents()->size()];
        stack = new Value[stacksize = INITSTACKSIZE];
        
        #ifdef VM_PROFILER
            byteprofilecounts = new uint64_t[codelen];
            memset(byteprofilecounts, 0, sizeof(uint64_t) * codelen);
        #endif

        vml.LogInit();

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

        if (vmpool)
        {
            delete vmpool;
            vmpool = nullptr;
        }
    }

    void SetMaxStack(int ms) { maxstacksize = ms; }
    const char *GetProgramName() { return programname; }
    int GetVectorType(int which) { return bcf->default_vector_types()->Get(which); }

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
            Output(OUTPUT_WARN,
                "LEAKS FOUND (this indicates cycles in your object graph, or a bug in Lobster, details in leaks.txt)");
                    
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
    
    #undef new
    LVector *NewVector(size_t n, int t)
    {
        return new (vmpool->alloc(sizeof(LVector) + sizeof(Value) * n)) LVector((int)n, t);
    }
    LString *NewString(size_t l)
    {
        return new (vmpool->alloc(sizeof(LString) + l + 1)) LString((int)l); 
    }
    CoRoutine *NewCoRoutine(const int *rip, const int *vip, CoRoutine *p)
    {
        return new (vmpool->alloc(sizeof(CoRoutine))) CoRoutine(sp + 2 /* top of sp + pushed coro */, rip, vip, p);
    }
    #ifdef WIN32
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #endif
    #endif
    
    LString *NewString(const char *c, size_t l)
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

    LString *NewString(const char *c1, size_t l1, const char *c2, size_t l2)
    {
        auto s = NewString(l1 + l2);
        memcpy(s->str(),      c1, l1);
        memcpy(s->str() + l1, c2, l2);
        s->str()[l1 + l2] = 0;
        return s;
    }

    // This function is now way less important than it was when the language was still dynamically typed.
    // But ok to leave it as-is for "index out of range" and other errors that are still dynamic.
    Value Error(string err, const Value &a = Value(0, V_MAXVMTYPES), const Value &b = Value(0, V_MAXVMTYPES))
    {
        if (trace_tail) err = trace_output + err;

        auto li = LookupLine(ip - 1, codestart, bcf);  // error is usually in the byte before the current ip
        auto s = string(bcf->filenames()->Get(li->fileidx())->c_str()) + "(" + to_string(li->line()) + "): VM error: " + err;
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

            auto li = LookupLine(ip - 1, codestart, bcf);
            if (deffun >= 0)
            {
                s += string("\nin function: ") + bcf->functions()->Get(deffun)->name()->c_str();
            }
            else
            {
                s += "\nin block";
            }
            s += string(" -> ") + bcf->filenames()->Get(li->fileidx())->c_str() + "(" + to_string(li->line()) + ")";

            s += locals;
        }

        s += "\nglobals:";

        for (size_t i = 0; i < bcf->idents()->size(); i++)
        {
            s += DumpVar(vars[i], i);
        }

        FinalStackVarsCleanup();

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
        size_t found = 0;
        size_t nfound = 0;
        for (size_t i = 0; i < bcf->idents()->size(); i++) if (a.Equal(vars[i], false)) { found = i; nfound++; }
        string s = a.ToString(debugpp);
        if (nfound == 1) s += string(" (") + bcf->idents()->Get(found)->name()->c_str() + " ?)";
        return s;
    }

    string DumpVar(const Value &x, size_t idx)
    {
        if (x.type == V_UNDEFINED) return "";
        if (bcf->idents()->Get(idx)->readonly()) return "";
        return string("\n   ") + bcf->idents()->Get(idx)->name()->c_str() + " = " + x.ToString(debugpp);
    }

    void EvalMulti(int nargs, const int *ip, int definedfunction, const int *retip)
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
                    if (v.type != desired || (v.type == V_VECTOR && v.vval()->type != *ip))
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
        Error(string("the call ") + bcf->functions()->Get(definedfunction)->name()->c_str() + "(" + argtypes +
              ") did not match any function variants");
    }

    void FinalStackVarsCleanup()
    {
        VMASSERT(sp < 0);

        for (size_t i = 0; i < bcf->idents()->size(); i++) vars[i].DEC();

        #ifdef _DEBUG
            Output(OUTPUT_INFO, "stack at its highest was: %d", maxsp);
        #endif
    }
    
    int CallerId()
    {
        for (int _sp = sp; _sp >= 0; _sp--)
            if (stack[_sp].type == V_RETIP)
                return int(stack[_sp].ip() - codestart);
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
        auto dfv = POP(); VMASSERT(dfv.type == V_DEFFUN);       auto deffun = dfv.info();
        auto ipv = POP(); VMASSERT(ipv.type == V_FUNSTART);     ip = ipv.ip(); 
        auto nav = POP(); VMASSERT(nav.type == V_NARGS);        auto nargs_given = nav.info();
        auto riv = POP(); VMASSERT(riv.type == V_RETIP);        auto retip = riv.ip();

        auto nargs_fun = *ip++;
        auto freevars = ip + nargs_given;
        ip += nargs_fun;
        auto ndef = *ip++;
        auto defvars = ip + ndef;

        if (vml.uses_frame_state)
        {
            auto lfr = POP(); VMASSERT(lfr.type == V_LOGFUNREADSTART); (void)lfr;
            auto lfw = POP(); VMASSERT(lfw.type == V_LOGFUNWRITESTART);
            vml.LogFunctionExit(ipv.ip(), defvars, lfw.info());
        }

        while (ndef--)  { auto i = *--defvars;  if (error) (*error) += DumpVar(vars[i], i); vars[i].DEC();
                                                                                                vars[i] = POP(); }
        while (nargs_given--) { auto i = *--freevars; if (error) (*error) += DumpVar(vars[i], i); vars[i].DEC();
                                                                                                vars[i] = POP(); } 

        ip = retip;

        return deffun;
    }

    void FunIntro(int nargs_given, const int *newip, int definedfunction, const int *retip)
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

            Output(OUTPUT_DEBUG, "stack grew to: %d", stacksize);
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

                // Error("function value called with " + to_string(nargs_given) + " arguments, but declared with only " + to_string(nargs_fun));

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
                    Error(string("\"return from ") + bcf->functions()->Get(towhere)->name()->c_str() + "\" outside of function");
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

    void CoNonRec(const int *varip)
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
        const int *returnip = codestart + *ip++;
        CoNonRec(ip);
        curcoroutine = NewCoRoutine(returnip, ip, curcoroutine);
        curcoroutine->BackupParentVars(vars);
        int nvars = *ip++;
        ip += nvars;
        PUSH(Value(curcoroutine));
    }

    void CoDone(const int *retip)
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

    void CoYield(const int *retip)
    {
        assert(curcoroutine);  // Should not be possible since yield calls are statically checked.

        auto ret = POP();

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
            Output(OUTPUT_INFO, "Profiler statistics:");
            uint64_t total = 0;
            auto fraction = 200;  // Line needs at least 0.5% to be counted.
            vector<uint64_t> lineprofilecounts(bcf->lineinfo()->size());
            for (size_t i = 0; i < codelen; i++)
            {
                auto li = LookupLine(codestart + i, codestart, bcf); // FIXME: can do faster
                size_t j = li - bcf->lineinfo()->Get(0);
                lineprofilecounts[j] += byteprofilecounts[i];
                total += byteprofilecounts[i];
            }
            struct LineRange { int line, lastline, fileidx; uint64_t count; };
            vector<LineRange> uniques;
            for (uint i = 0; i < bcf->lineinfo()->size(); i++)
            {
                uint64_t c = lineprofilecounts[i];
                if (c > total / fraction)
                {
                    auto li = bcf->lineinfo()->Get(i);
                    uniques.push_back(LineRange{ li->line(), li->line(), li->fileidx(), c });
                }
            }
            std::sort(uniques.begin(), uniques.end(), [&] (const LineRange &a, const LineRange &b) {
                return a.fileidx != b.fileidx ? a.fileidx < b.fileidx : a.line < b.line;
            });
            for (auto it = uniques.begin(); it != uniques.end();) 
            {
                if (it != uniques.begin())
                {
                    auto pit = it - 1;
                    if (it->fileidx == pit->fileidx &&
                        ((it->line == pit->lastline) ||
                         (it->line == pit->lastline + 1 && pit->lastline++)))
                    {
                        pit->count += it->count;
                        it = uniques.erase(it);
                        continue;
                    }
                }
                ++it;
            }
            for (auto &u : uniques)
            {
                Output(OUTPUT_INFO, "%s(%d%s): %.1f %%", bcf->filenames()->Get(u.fileidx)->c_str(), u.line,
                       u.lastline != u.line ? ("-" + to_string(u.lastline)).c_str() : "",
                       u.count * 100.0f / total);
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
                    if (!trace_tail) trace_output.clear();
                    DisAsmIns(trace_output, ip, codestart, bcf);
                    trace_output += " [";
                    trace_output += to_string(sp + 1);
                    trace_output += "] - ";
                    if (sp >= 0) trace_output += TOP().ToString(debugpp);
                    if (sp >= 1) { trace_output += " "; trace_output += TOP2().ToString(debugpp); }
                    if (trace_tail)
                    {
                        trace_output += "\n";
                        const int trace_max = 10000;
                        if (trace_output.length() > trace_max) trace_output.erase(0, trace_max / 2);
                    }
                    else
                    {
                        Output(OUTPUT_INFO, "%s", trace_output.c_str());
                    }
                }

                //currentline = LookupLine(ip).line;
            #endif
            
            #ifdef VM_PROFILER
                byteprofilecounts[ip - codestart]++;
            #endif

            switch (*ip++)
            {
                case IL_PUSHUNDEF: PUSH(Value()); break;
                case IL_PUSHINT:   PUSH(Value(*ip++)); break;
                case IL_PUSHFLT:   PUSH(Value(*(float *)ip)); ip++; break;
                case IL_PUSHNIL:   PUSH(Value(0, V_NIL)); break;

                case IL_PUSHFUN:
                {
                    int start = *ip++;
                    PUSH(Value(codestart + start));
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
                    VMASSERT(fun.type == V_FUNCTION);
                    auto nargs = *ip++;
                    FunIntro(nargs, fun.ip(), -1, ip);
                    break;
                }

                case IL_YIELD:
                    CoYield(ip);
                    break;

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
                    int nrv = *ip++;
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
                    i.ival()++;
                    int len = 0;
                    switch (iter.type)
                    {
                        #define PUSHITER(L, V) if (i.ival() >= (len = L)) goto done; PUSH(V); break;
                        case V_INT:    PUSHITER(iter.ival()     , i);
                        case V_VECTOR: PUSHITER(iter.vval()->len, iter.vval()->at(i.ival()).INC());
                        case V_STRING: PUSHITER(iter.sval()->len, Value((int)((uchar *)iter.sval()->str())[i.ival()]));
                        #undef PUSHITER
                        default: VMASSERT(0);
                    }
                    PUSH(i);
                    FunIntro(2, body.ip(), -1, forstart);
                    break;

                    done:
                    (void)POP();        // body
                    (void)POP().DEC();  // iter
                    (void)POP();        // i
                    break;
                }

                case IL_BCALL:
                {
                    auto nf = natreg.nfuns[*ip++];
                    Value v;
                    switch (nf->args.v.size())
                    {
                        #define ARG(N) Value a##N = POP();
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
                    #ifdef _DEBUG
                        // see if any builtin function is lying about what type it returns
                        // other function types return intermediary values that don't correspond to final return values
                        if (nf->ncm == NCM_NONE)
                        { 
                            for (size_t i = 0; i < nf->retvals.v.size(); i++)
                            {
                                auto t = (TOPPTR() - nf->retvals.v.size() + i)->type;
                                auto u = nf->retvals.v[i].type->t;
                                VMASSERT(t == u || u == V_ANY || u == V_NILABLE);   
                            }
                        }
                    #endif
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

                #define REFOP(exp) { res = exp; a.DEC(); b.DEC(); }
                #define COP(t) if (b.type == t) { VMASSERTVALUES(false, a, b); }
                #define GETARGS() Value b = POP(); Value a = POP()
                #define TYPEOP(op, extras, field, errstat) Value res; errstat; \
                    if (extras & 1 && b.field == 0) Div0(); res = a.field op b.field;

                #define _IOP(op, extras)  TYPEOP(op, extras, ival(), VMASSERTVALUES(a.type == V_INT && b.type == V_INT, a, b))
                #define _FOP(op, extras)  TYPEOP(op, extras, fval(), VMASSERTVALUES(a.type == V_FLOAT && b.type == V_FLOAT, a, b))

                #define _VOP(op, extras, type, isfloat) Value res; for (;;) { \
                    int len = VectorLoop(a, b, res, isfloat); \
                    if (len >= 0) { \
                        for (int j = 0; j < len; j++) \
                        { auto bv = VectorElem<type>(b, j); if (extras&1 && bv == 0) Div0(); \
                          res.vval()->at(j) = Value(VectorElem<type>(a, j) op bv); }\
                        VectorDec(a, res); VectorDec(b, res); \
                        break; } \
                    VMASSERTVALUES(false, a, b); \
                } 
                #define _IVOP(op, extras) _VOP(op, extras, int, false)
                #define _FVOP(op, extras) _VOP(op, extras, float, true)
                    
                #define _SCAT()  Value res; REFOP(NewString(a.sval()->str(), a.sval()->len, b.sval()->str(), b.sval()->len))
                #define _SOP(op) Value res; REFOP((*a.sval()) op (*b.sval()))

                #define ACOMPEN(op, andor) { GETARGS(); Value res; REFOP(a.type op b.type andor a.any() op b.any()); PUSH(res); break; }


                #define IOP(op, extras)  { GETARGS(); _IOP(op, extras);  PUSH(res); break; }
                #define FOP(op, extras)  { GETARGS(); _FOP(op, extras);  PUSH(res); break; }
                #define IVOP(op, extras) { GETARGS(); _IVOP(op, extras); PUSH(res); break; }
                #define FVOP(op, extras) { GETARGS(); _FVOP(op, extras); PUSH(res); break; }
                #define SOP(op)          { GETARGS(); _SOP(op);          PUSH(res); break; }
                #define SCAT()           { GETARGS(); _SCAT();           PUSH(res); break; }

                // +  += I F Vif S
                // -  -= I F Vif 
                // *  *= I F Vif 
                // /  /= I F Vif 
                // %  %= I   Vi 

                // <     I F Vif S
                // >     I F Vif S
                // <=    I F Vif S
                // >=    I F Vif S
                // ==    I F V   S   // FIXME differentiate struct / value / vector
                // !=    I F V   S 

                // U-    I F Vif
                // U!    A

                case IL_IVADD: IVOP(+,  0);
                case IL_IVSUB: IVOP(-,  0);
                case IL_IVMUL: IVOP(*,  0);
                case IL_IVDIV: IVOP(/,  1);
                case IL_IVLT:  IVOP(<,  0);
                case IL_IVGT:  IVOP(>,  0);
                case IL_IVLE:  IVOP(<=, 0);
                case IL_IVGE:  IVOP(>=, 0);
                case IL_FVADD: FVOP(+,  0);
                case IL_FVSUB: FVOP(-,  0);
                case IL_FVMUL: FVOP(*,  0);
                case IL_FVDIV: FVOP(/,  1);
                case IL_FVLT:  FVOP(<,  0);
                case IL_FVGT:  FVOP(>,  0);
                case IL_FVLE:  FVOP(<=, 0);
                case IL_FVGE:  FVOP(>=, 0);
                case IL_AEQ:   ACOMPEN(==, &&);
                case IL_ANE:   ACOMPEN(!=, ||);
                    
                case IL_IADD: IOP(+,  0);
                case IL_ISUB: IOP(-,  0);
                case IL_IMUL: IOP(*,  0);
                case IL_IDIV: IOP(/ , 1);
                case IL_IMOD: IOP(%,  1);
                case IL_ILT:  IOP(<,  0);
                case IL_IGT:  IOP(>,  0);
                case IL_ILE:  IOP(<=, 0);
                case IL_IGE:  IOP(>=, 0);
                case IL_IEQ:  IOP(==, 0);
                case IL_INE:  IOP(!=, 0);

                case IL_FADD: FOP(+,  0);
                case IL_FSUB: FOP(-,  0);
                case IL_FMUL: FOP(*,  0);
                case IL_FDIV: FOP(/,  1);
                case IL_FLT:  FOP(<,  0);
                case IL_FGT:  FOP(>,  0);
                case IL_FLE:  FOP(<=, 0);
                case IL_FGE:  FOP(>=, 0);
                case IL_FEQ:  FOP(==, 0);
                case IL_FNE:  FOP(!=, 0);

                case IL_SADD: SCAT();
                case IL_SLT:  SOP(<);
                case IL_SGT:  SOP(>);
                case IL_SLE:  SOP(<=);
                case IL_SGE:  SOP(>=);
                case IL_SEQ:  SOP(==);
                case IL_SNE:  SOP(!=);

                case IL_UMINUS:
                {
                    Value a = POP();
                    switch (a.type)
                    {
                        case V_INT: PUSH(Value(-a.ival())); break;
                        case V_FLOAT: PUSH(Value(-a.fval())); break;
                        case V_VECTOR:
                        {
                            bool isfloat = true;
                            Value res;
                            int len = VectorLoop(a, Value(1), res, isfloat);
                            if (len >= 0)
                            {
                                for (int i = 0; i < len; i++)
                                    res.vval()->at(i) = isfloat ? Value(-VectorElem<float>(a, i))
                                                                : Value(-VectorElem<int>  (a, i));
                                VectorDec(a, res);
                                PUSH(res);
                                break;
                            }
                            // fall tru
                        }

                        default: VMASSERT(false);
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
                    PUSH((float)a.ival());    
                    break;
                }                
                
                case IL_A2S:
                {
                    Value a = POP();
                    PUSH(NewString(a.ToString(programprintprefs)));   
                    a.DEC();
                    break;
                }
                    
                case IL_A2A:
                    // For now, this is a no-op. In the future, this will box the given value.
                    break;

                case IL_PUSHVAR:   PUSH(vars[*ip++].INC()); break;

                case IL_PUSHFLD:
                case IL_PUSHFLDM: PushDeref(*ip++); break;
                case IL_PUSHIDX:  PushDeref(GrabIndex(POP())); break;

                case IL_PUSHLOC:
                {
                    int i = *ip++;
                    Value coro = POP();
                    VMASSERT(coro.type == V_COROUTINE);
                    PUSH(coro.cval()->GetVar(i).INC());
                    coro.DECRT();
                    break;
                }

                case IL_LVALLOC:
                {
                    int lvalop = *ip++;
                    int i = *ip++;
                    Value coro = POP();
                    VMASSERT(coro.type == V_COROUTINE);
                    Value &a = coro.cval()->GetVar(i);
                    LvalueOp(lvalop, a);
                    coro.DECRT();
                    break;
                }

                case IL_LVALVAR:   
                {
                    int lvalop = *ip++; 
                    LvalueOp(lvalop, vars[*ip++]);
                    break;
                }

                case IL_LVALIDX: { int lvalop = *ip++; LvalueObj(lvalop, GrabIndex(POP())); break; }
                case IL_LVALFLD: { int lvalop = *ip++; LvalueObj(lvalop, *ip++); break; }

                case IL_PUSHONCE:
                {
                    auto x = POP();
                    auto &v = TOP();
                    VMASSERT(v.type == V_VECTOR);
                    v.vval()->push(x);
                    break;
                }

                case IL_JUMPFAIL:    { auto x = POP(); auto nip = *ip++; if (!x.DEC().True()) { ip = codestart + nip;          }               break; }
                case IL_JUMPFAILR:   { auto x = POP(); auto nip = *ip++; if (!x      .True()) { ip = codestart + nip; PUSH(x); } else x.DEC(); break; }
                case IL_JUMPNOFAIL:  { auto x = POP(); auto nip = *ip++; if ( x.DEC().True()) { ip = codestart + nip;          }               break; }
                case IL_JUMPNOFAILR: { auto x = POP(); auto nip = *ip++; if ( x      .True()) { ip = codestart + nip; PUSH(x); } else x.DEC(); break; }

                case IL_ISTYPE:
                {
                    auto t = *ip++;
                    auto idx = *ip++;
                    auto &v = POP().DEC();
                    PUSH(Value(v.type == t && (t != V_VECTOR || v.vval()->type == idx)));
                    break;
                }

                case IL_COCL:
                    PUSH(Value(0, V_YIELD));  // This value never gets used anywhere, just a placeholder.
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
                    Error("bytecode format problem: " + to_string(*--ip));
            }
        }
    }

    void PushDeref(int i) 
    { 
        Value r = POP(); 
        switch (r.type) 
        { 
            case V_VECTOR:
                IDXErr(i, r.vval()->len, r);
                PUSH(r.vval()->at(i).INC());
                break;
            case V_NIL:  // only used with ?.
                PUSH(r);
                break;
            case V_STRING:
                IDXErr(i, r.sval()->len, r); 
                PUSH(Value((int)r.sval()->str()[i]));
                break;
            default:
                VMASSERT(false); 
        } 
        r.DECRT(); 
    }

    void LvalueObj(int lvalop, int i)
    {
        Value vec = POP();
        VMASSERT(vec.type == V_VECTOR);
        IDXErr(i, (int)vec.vval()->len, vec);
        Value &a = vec.vval()->at(i);
        LvalueOp(lvalop, a);
        vec.DECRT();
    }

    void LvalueOp(int op, Value &a)
    {
        switch(op)
        {
            case LVO_IVADD:   { Value b = POP();  _IVOP(+, 0); a = res;                  break; }
            case LVO_IVADDR:  { Value b = POP();  _IVOP(+, 0); a = res; PUSH(res.INC()); break; }
            case LVO_IVSUB:   { Value b = POP();  _IVOP(-, 0); a = res;                  break; }
            case LVO_IVSUBR:  { Value b = POP();  _IVOP(-, 0); a = res; PUSH(res.INC()); break; }
            case LVO_IVMUL:   { Value b = POP();  _IVOP(*, 0); a = res;                  break; }
            case LVO_IVMULR:  { Value b = POP();  _IVOP(*, 0); a = res; PUSH(res.INC()); break; }
            case LVO_IVDIV:   { Value b = POP();  _IVOP(/, 1); a = res;                  break; }
            case LVO_IVDIVR:  { Value b = POP();  _IVOP(/, 1); a = res; PUSH(res.INC()); break; }

            case LVO_FVADD:   { Value b = POP();  _FVOP(+, 0); a = res;                  break; }
            case LVO_FVADDR:  { Value b = POP();  _FVOP(+, 0); a = res; PUSH(res.INC()); break; }
            case LVO_FVSUB:   { Value b = POP();  _FVOP(-, 0); a = res;                  break; }
            case LVO_FVSUBR:  { Value b = POP();  _FVOP(-, 0); a = res; PUSH(res.INC()); break; }
            case LVO_FVMUL:   { Value b = POP();  _FVOP(*, 0); a = res;                  break; }
            case LVO_FVMULR:  { Value b = POP();  _FVOP(*, 0); a = res; PUSH(res.INC()); break; }
            case LVO_FVDIV:   { Value b = POP();  _FVOP(/, 1); a = res;                  break; }
            case LVO_FVDIVR:  { Value b = POP();  _FVOP(/, 1); a = res; PUSH(res.INC()); break; }

            case LVO_IADD:    { Value b = POP();  _IOP(+, 0);  a = res;                  break; }
            case LVO_IADDR:   { Value b = POP();  _IOP(+, 0);  a = res; PUSH(res.INC()); break; }
            case LVO_ISUB:    { Value b = POP();  _IOP(-, 0);  a = res;                  break; }
            case LVO_ISUBR:   { Value b = POP();  _IOP(-, 0);  a = res; PUSH(res.INC()); break; }
            case LVO_IMUL:    { Value b = POP();  _IOP(*, 0);  a = res;                  break; }
            case LVO_IMULR:   { Value b = POP();  _IOP(*, 0);  a = res; PUSH(res.INC()); break; }
            case LVO_IDIV:    { Value b = POP();  _IOP(/, 1);  a = res;                  break; }
            case LVO_IDIVR:   { Value b = POP();  _IOP(/, 1);  a = res; PUSH(res.INC()); break; }
            case LVO_IMOD:    { Value b = POP();  _IOP(%, 1);  a = res;                  break; }
            case LVO_IMODR:   { Value b = POP();  _IOP(%, 1);  a = res; PUSH(res.INC()); break; }

            case LVO_FADD:    { Value b = POP();  _FOP(+, 0);  a = res;                  break; }
            case LVO_FADDR:   { Value b = POP();  _FOP(+, 0);  a = res; PUSH(res.INC()); break; }
            case LVO_FSUB:    { Value b = POP();  _FOP(-, 0);  a = res;                  break; }
            case LVO_FSUBR:   { Value b = POP();  _FOP(-, 0);  a = res; PUSH(res.INC()); break; }
            case LVO_FMUL:    { Value b = POP();  _FOP(*, 0);  a = res;                  break; }
            case LVO_FMULR:   { Value b = POP();  _FOP(*, 0);  a = res; PUSH(res.INC()); break; }
            case LVO_FDIV:    { Value b = POP();  _FOP(/, 1);  a = res;                  break; }
            case LVO_FDIVR:   { Value b = POP();  _FOP(/, 1);  a = res; PUSH(res.INC()); break; }

            case LVO_SADD:    { Value b = POP();  _SCAT();     a = res;                  break; }
            case LVO_SADDR:   { Value b = POP();  _SCAT();     a = res; PUSH(res.INC()); break; }

            case LVO_WRITE:   { Value  b = POP();       a.DEC(); a = b; break; }
            case LVO_WRITER:  { Value &b = TOP().INC(); a.DEC(); a = b; break; }
            case LVO_WRITED:  { Value  b = POP();       a.DEC(); a = b; break; }
            // LVO_WRITED is only there because OVERWRITE causes problems with rec functions,
            // and its not needed for defines anyway
                    
            #define PPOP(ret, op, pre) { \
                if (ret && !pre) PUSH(a.INC()); \
                if (a.type == V_INT) a.ival() = a.ival() op 1; \
                else if (a.type == V_FLOAT) a.fval() = a.fval() op 1; \
                else VMASSERT(false); \
                if (ret && pre) PUSH(a.INC()); \
            }
                
            case LVO_PP: 
            case LVO_PPR:  { PPOP(op == LVO_PPR,  +, true);  break; }
            case LVO_MM: 
            case LVO_MMR:  { PPOP(op == LVO_MMR,  -, true);  break; }
            case LVO_PPP: 
            case LVO_PPPR: { PPOP(op == LVO_PPPR, +, false); break; }
            case LVO_MMP:
            case LVO_MMPR: { PPOP(op == LVO_MMPR, -, false); break; }

            default:
                Error("bytecode format problem (lvalue): " + to_string(op));
        }
    }

    const char *ProperTypeName(const Value &v)
    {
        return v.type == V_VECTOR && v.vval()->type >= 0 ? ReverseLookupType(v.vval()->type) : BaseTypeName(v.type);
    }

    void Div0() { Error("division by zero"); } 

    void IDXErr(int i, int n, const Value &v)
    {
        if (i < 0 || i >= n) Error("index " + to_string(i) + " out of range " + to_string(n), v);
    }

    int GrabIndex(const Value &idx)
    {
        if (idx.type == V_INT) return idx.ival();

        if (idx.type == V_VECTOR)
        {
            auto &v = TOP();
            for (int i = idx.vval()->len - 1; ; i--)
            {
                auto sidx = idx.vval()->at(i);
                VMASSERT(sidx.type == V_INT);
                if (!i)
                {
                    idx.DECRT();
                    return sidx.ival();
                }
                VMASSERT(v.type == V_VECTOR);
                IDXErr(sidx.ival(), v.vval()->len, v);
                auto nv = v.vval()->at(sidx.ival()).INC();
                v.DECRT();
                v = nv;
            }
        }

        VMASSERT(0);
        return 0;
    }

    int VectorLoop(const Value &a, const Value &b, Value &res, bool isfloat)
    {
        // note: not doing DEC() on the reused vectors is ok because VectorElem will error on not float/int
        int len;
        int type = V_VECTOR;
        if (a.type == V_VECTOR)
        {
            len = a.vval()->len;
            if (b.type == V_VECTOR)
            {
                len = min(len, b.vval()->len);
                if(a.vval()->len < b.vval()->len || (a.vval()->len == b.vval()->len && a.vval()->type >= 0))
                {
                    if (a.vval()->refc == 1) { res = a; return len; } else type = a.vval()->type;
                }
                else
                {
                    if (b.vval()->refc == 1) { res = b; return len; } else type = b.vval()->type;
                }
            }
            else
            {
                VMASSERT(isfloat ? b.type == V_FLOAT : b.type == V_INT);
                if (a.vval()->refc == 1) { res = a; return len; }
                type = a.vval()->type;
            }
        }
        else if (b.type == V_VECTOR)
        {
            len = b.vval()->len;
            VMASSERT(isfloat ? a.type == V_FLOAT : a.type == V_INT);
            if (b.vval()->refc == 1) { res = b; return len; }
            type = b.vval()->type;
        }
        else
        {
            VMASSERT(false);
            len = -1;
        }
        res = Value(NewVector(len, type));
        res.vval()->len = len;    // so we can overwrite, needed for reuse
        return len;
    }

    int VectorTrim(const Value &a, int len)
    {
        while(a.vval()->len > len) a.vval()->pop().DEC();
        return len;
    }

    void VectorDec(const Value &a, const Value &res)
    {
        if (a.type == V_VECTOR && a.vval() != res.vval()) a.DEC();
    }

    template<typename T> T VectorElem(const Value &a, int i)
    {
        switch (a.type)
        {
            case V_FLOAT: return (T)a.fval();
            case V_INT:   return (T)a.ival();
            case V_VECTOR:
            {
                auto v = a.vval()->at(i);
                switch (v.type)
                {
                    case V_FLOAT: return (T)v.fval();
                    case V_INT:   return (T)v.ival();
                    default:
                        VMASSERT(0);
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

    int StructIdx(const string &name, int &nargs)  // FIXME: this is inefficient, used by parse_data()
    {
        for (uint i = 0; i < bcf->structs()->size(); i++)
        {
            auto s = bcf->structs()->Get(i);
            if (s->name()->c_str() == name)
            {
                nargs = s->nfields();
                return s->idx();
            }
        }
        return -1;
    }

    virtual const char *ReverseLookupType(uint v)
    {
        return bcf->structs()->Get(v)->name()->c_str();
    }

    void Trace(bool on) { trace = on; }

    double Time() { return SecondsSinceStart(); }

    int GC()    // shouldn't really be used, but just in case
    {
        for (int i = 0; i <= sp; i++) stack[i].Mark();
        for (size_t i = 0; i < bcf->idents()->size(); i++) vars[i].Mark();
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
                default: VMASSERT(ro->type >= 0);  // fall thru: a struct type
                case V_VECTOR:    v.vval()->len = 0; v.vval()->deleteself(); break;
                case V_STRING:                     v.sval()->deleteself(); break;
                case V_COROUTINE:                  v.cval()->deleteself(false); break;
            }
        }

        return (int)leaks.size();
    }
};

void RunBytecode(string &evalret, const char *programname, const uchar *bytecode)
{
    VM vm(programname, bytecode);
    vm.EvalProgram(evalret);
}

}  // namespace lobster
