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
    const type_elem_t *typetable;
    vector<type_elem_t> typetablebigendian;
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
          debugpp(2, 50, true, -1, true), programname(_pn), vml(*this, bcf->uses_frame_state() != 0),
          trace(false), trace_tail(false)
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
            typetable = (const type_elem_t *)bcf->typetable()->Data();
        }
        else
        {
            for (uint i = 0; i < codelen; i++)
                codebigendian.push_back(bcf->bytecode()->Get(i));
            codestart = codebigendian.data();

            for (uint i = 0; i < bcf->typetable()->Length(); i++)
                typetablebigendian.push_back((type_elem_t)bcf->typetable()->Get(i));
            typetable = typetablebigendian.data();
        }
        ip = codestart;

        vars = new Value[bcf->specidents()->size()];
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

    const TypeInfo &GetTypeInfo(type_elem_t offset) { return *(TypeInfo *)(typetable + offset); }
    const TypeInfo &GetVarTypeInfo(int varidx)
    {
        return  *(TypeInfo *)(typetable + bcf->specidents()->Get(varidx)->typeidx());
    }

    void SetMaxStack(int ms) { maxstacksize = ms; }
    const char *GetProgramName() { return programname; }
    type_elem_t GetIntVectorType(int which) { return (type_elem_t)bcf->default_int_vector_types()->Get(which); }
    type_elem_t GetFloatVectorType(int which) { return (type_elem_t)bcf->default_float_vector_types()->Get(which); }

    static bool _LeakSorter(void *va, void *vb)
    {
        auto a = (LVector *)va;
        auto b = (LVector *)vb;
        return a->refc != b->refc
        ? a->refc > b->refc
        : (a->typeoff != b->typeoff
           ? a->typeoff > b->typeoff
           : false);
    }

    void DumpLeaks()
    {
        vector<void *> leaks;
        vmpool->findleaks([&](void *p) { leaks.push_back(p); });

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
                    switch(vec->typeoff)
                    {
                        case TYPE_ELEM_CYCLEDONE:
                        case TYPE_ELEM_VALUEBUF:
                            break;
                                    
                        case TYPE_ELEM_STRING:
                        {
                            auto str = (LString *)vec;
                            fputs((str->CycleStr() + " = " + str->ToString(leakpp) + "\n").c_str(), leakf);
                            break;
                        }

                        case TYPE_ELEM_COROUTINE:
                        {
                            auto co = (CoRoutine *)vec;
                            fputs((co->CycleStr() + " = coroutine\n").c_str(), leakf);
                            break;
                        }

                        case TYPE_ELEM_BOXEDINT:
                        {
                            auto bi = (BoxedInt *)vec;
                            fputs((bi->CycleStr() + " = " + to_string(bi->val) + "\n").c_str(), leakf);
                            break;
                        }
                                    
                        case TYPE_ELEM_BOXEDFLOAT:
                        {
                            auto bf = (BoxedFloat *)vec;
                            fputs((bf->CycleStr() + " = " + to_string_float(bf->val) + "\n").c_str(), leakf);
                            break;
                        }

                        default:
                        {
                            assert(IsVector(vec->BaseType()));
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
    LVector *NewVector(size_t n, type_elem_t t)
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
    BoxedInt *NewInt(int i)
    {
        return new (vmpool->alloc(sizeof(BoxedInt))) BoxedInt(i); 
    }
    BoxedFloat *NewFloat(float f)
    {
        return new (vmpool->alloc(sizeof(BoxedFloat))) BoxedFloat(f); 
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
            s += "\n   stack: " + ValueDBG(TOP());
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

        for (size_t i = 0; i < bcf->specidents()->size(); i++)
        {
            s += DumpVar(vars[i], i, true);
        }

        FinalStackVarsCleanup();

        throw s;
    }

    Value BuiltinError(string err) { return Error(err); }

    void VMAssert(bool ok, const char *what) 
    {
        if (!ok)
            Error(string("VM internal assertion failure: ") + what); 
    }
    void VMAssert(bool ok, const char *what, const Value &a, const Value &b) 
    {
        if (!ok)
            Error(string("VM internal assertion failure: ") + what, a, b); 
    }

    #ifdef _DEBUG
        #define STRINGIFY(x) #x
        #define TOSTRING(x) STRINGIFY(x)
        #define VMASSERT(test)             VMAssert(test, __FILE__ ": " TOSTRING(__LINE__) ": " #test)
        #define VMASSERTVALUES(test, a, b) VMAssert(test, __FILE__ ": " TOSTRING(__LINE__) ": " #test, a, b)
    #else
        #define VMASSERT(test)             (void)(test)
        #define VMASSERTVALUES(test, a, b) (void)(test); (void)(a); (void)(b)
    #endif
    #define VMTYPEEQ(val, vt) VMASSERT((val).type == (vt))

    string ValueDBG(const Value &a)
    {
        return a.ToString(a.type, debugpp);
    }

    string DumpVar(const Value &x, size_t idx, uchar dumpglobals)
    {
        auto sid = bcf->specidents()->Get(idx);
        auto id = bcf->idents()->Get(sid->ididx());
        if (id->readonly() || id->global() != dumpglobals) return "";
        //auto &ti = GetTypeInfo(sid->typeidx());
        string name = id->name()->c_str();
        return "\n   " + name + " = " + x.ToString(x.type, debugpp);
    }

    void EvalMulti(int nargs, const int *mip, int definedfunction, const int *retip)
    {
        VMASSERT(*mip == IL_FUNMULTI);
        mip++;

        auto nsubf = *mip++;
        auto table_nargs = *mip++;
        VMASSERT(nargs == table_nargs);
        (void)table_nargs;
        for (int i = 0; i < nsubf; i++)
        {
            // TODO: rather than going thru all args, only go thru those that have types
            for (int j = 0; j < nargs; j++)
            {
                auto desired = (type_elem_t)*mip++;
                if (desired != TYPE_ELEM_ANY)
                {
                    Value &v = stack[sp - nargs + j + 1];
                    auto &ti = GetTypeInfo(desired);
                    if (v.type != ti.vt() || (IsRef(v.type) && v.ref()->typeoff != desired))
                    {
                        mip += nargs - j;  // Includes the code starting point.
                        goto fail;
                    }
                }
                else
                {

                }
            }

            return FunIntro(nargs, codestart + *mip, definedfunction, retip);

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

        for (size_t i = 0; i < bcf->specidents()->size(); i++) vars[i].DEC();

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
        auto dfv = POP(); VMTYPEEQ(dfv, V_DEFFUN);       auto deffun = dfv.info();
        auto ipv = POP(); VMTYPEEQ(ipv, V_FUNSTART);     ip = ipv.ip(); 
        auto nav = POP(); VMTYPEEQ(nav, V_NARGS);        auto nargs_given = nav.info();
        auto riv = POP(); VMTYPEEQ(riv, V_RETIP);        auto retip = riv.ip();

        auto nargs_fun = *ip++;
        auto freevars = ip + nargs_given;
        ip += nargs_fun;
        auto ndef = *ip++;
        auto defvars = ip + ndef;

        if (vml.uses_frame_state)
        {
            auto lfr = POP(); VMTYPEEQ(lfr, V_LOGFUNREADSTART); (void)lfr;
            auto lfw = POP(); VMTYPEEQ(lfw, V_LOGFUNWRITESTART);
            vml.LogFunctionExit(ipv.ip(), defvars, lfw.info());
        }

        while (ndef--)
        {
            auto i = *--defvars; 
            if (error) (*error) += DumpVar(vars[i], i, false);
            vars[i].DEC();
            vars[i] = POP();
        }
        while (nargs_given--)
        {
            auto i = *--freevars;
            if (error) (*error) += DumpVar(vars[i], i, false);
            vars[i].DEC();
            vars[i] = POP();
        } 

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
        VMASSERT(nargs_given == nargs_fun);
        
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
            //var.type = V_NIL;
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
        VMASSERT(curcoroutine->stackcopymax >=  *curcoroutine->varip);
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
        auto last = TOP();
        evalret = last.ToString(last.type, programprintprefs);
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
                    DisAsmIns(trace_output, ip, codestart, typetable, bcf);
                    trace_output += " [";
                    trace_output += to_string(sp + 1);
                    trace_output += "] - ";
                    if (sp >= 0) { auto x = TOP();  trace_output += x.ToString(x.type, debugpp); }
                    if (sp >= 1) { auto x = TOP2(); trace_output += " "; trace_output += x.ToString(x.type, debugpp); }
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
                case IL_PUSHINT:   PUSH(Value(*ip++)); break;
                case IL_PUSHFLT:   PUSH(Value(*(float *)ip)); ip++; break;
                case IL_PUSHNIL:   PUSH(Value()); break;

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
                    VMTYPEEQ(fun, V_FUNCTION);
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

                #define FORLOOP(L, V, D) { \
                    auto forstart = ip - 1; \
                    POP().DEC(); /* body retval */ \
                    auto &body = TOP(); \
                    auto &iter = TOP2(); \
                    auto &i = TOP3(); \
                    assert(i.type == V_INT); \
                    i.ival()++; \
                    int len = 0; \
                    if (i.ival() >= (len = (L))) goto D; \
                    int nargs = body.ip()[1]; \
                    if (nargs) { PUSH(V); if (nargs > 1) PUSH(i); } /* FIXME: make this static? */ \
                    FunIntro(nargs, body.ip(), -1, forstart); \
                    break; \
                    D: \
                    (void)POP(); /* body */ \
                    (void)POP().DEC(); /* iter */ \
                    (void)POP(); /* i */ \
                    break; \
                }

                case IL_IFOR: FORLOOP(iter.ival(), i, donei);
                case IL_VFOR: FORLOOP(iter.vval()->len, iter.vval()->at(i.ival()).INC(), donev);
                case IL_SFOR: FORLOOP(iter.sval()->len, Value((int)((uchar *)iter.sval()->str())[i.ival()]), dones);

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
                                VMASSERT(t == u || u == V_ANY || u == V_NIL || (u == V_VECTOR && t == V_STRUCT));   
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
                    auto type = (type_elem_t)*ip++;
                    VMASSERT(IsVector((ValueType)*(typetable + type)));
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

                #define _VELEM(a, i, isfloat, T) (isfloat ? (T)a.vval()->at(i).fval() : (T)a.vval()->at(i).ival())
                #define _VOP(op, extras, T, isfloat, withscalar, comp) Value res; { \
                    int len = VectorLoop(a, b, res, withscalar, comp ? TYPE_ELEM_VECTOR_OF_INT : a.vval()->typeoff); \
                    for (int j = 0; j < len; j++) \
                    { \
                        if (withscalar) VMTYPEEQ(b, isfloat ? V_FLOAT : V_INT); else VMTYPEEQ(b.vval()->at(j), isfloat ? V_FLOAT : V_INT); \
                        auto bv = withscalar ? (isfloat ? (T)b.fval() : (T)b.ival()) : _VELEM(b, j, isfloat, T); \
                        if (extras&1 && bv == 0) Div0(); \
                        VMTYPEEQ(a.vval()->at(j), isfloat ? V_FLOAT : V_INT); \
                        res.vval()->at(j) = Value(_VELEM(a, j, isfloat, T) op bv); \
                    } \
                    VectorDec(a, res); \
                    if (!withscalar) VectorDec(b, res); \
                } 
                #define _IVOP(op, extras, withscalar, icomp) _VOP(op, extras, int, false, withscalar, icomp)
                #define _FVOP(op, extras, withscalar, fcomp) _VOP(op, extras, float, true, withscalar, fcomp)
                    
                #define _SCAT()  Value res; REFOP(NewString(a.sval()->str(), a.sval()->len, b.sval()->str(), b.sval()->len))
                #define _SOP(op) Value res; REFOP((*a.sval()) op (*b.sval()))

                #define ACOMPEN(op, andor) { GETARGS(); Value res; REFOP(a.type op b.type andor a.any() op b.any()); PUSH(res); break; }

                                           
                #define IOP(op, extras)    { GETARGS(); _IOP(op, extras);                PUSH(res); break; }
                #define FOP(op, extras)    { GETARGS(); _FOP(op, extras);                PUSH(res); break; }
                #define IVVOP(op, extras)  { GETARGS(); _IVOP(op, extras, false, false); PUSH(res); break; }
                #define IVVOPC(op, extras) { GETARGS(); _IVOP(op, extras, false, true);  PUSH(res); break; }
                #define FVVOP(op, extras)  { GETARGS(); _FVOP(op, extras, false, false); PUSH(res); break; }
                #define FVVOPC(op, extras) { GETARGS(); _FVOP(op, extras, false, true);  PUSH(res); break; }
                #define IVSOP(op, extras)  { GETARGS(); _IVOP(op, extras, true, false);  PUSH(res); break; }
                #define IVSOPC(op, extras) { GETARGS(); _IVOP(op, extras, true, true);   PUSH(res); break; }
                #define FVSOP(op, extras)  { GETARGS(); _FVOP(op, extras, true, false);  PUSH(res); break; }
                #define FVSOPC(op, extras) { GETARGS(); _FVOP(op, extras, true, true);   PUSH(res); break; }
                #define SOP(op)            { GETARGS(); _SOP(op);                        PUSH(res); break; }
                #define SCAT()             { GETARGS(); _SCAT();                         PUSH(res); break; }

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

                case IL_IVVADD: IVVOP(+,  0);
                case IL_IVVSUB: IVVOP(-,  0);
                case IL_IVVMUL: IVVOP(*,  0);
                case IL_IVVDIV: IVVOP(/,  1);
                case IL_IVVLT:  IVVOP(<,  0);
                case IL_IVVGT:  IVVOP(>,  0);
                case IL_IVVLE:  IVVOP(<=, 0);
                case IL_IVVGE:  IVVOP(>=, 0);
                case IL_FVVADD: FVVOP(+,  0);
                case IL_FVVSUB: FVVOP(-,  0);
                case IL_FVVMUL: FVVOP(*,  0);
                case IL_FVVDIV: FVVOP(/,  1);
                case IL_FVVLT:  FVVOPC(<,  0);
                case IL_FVVGT:  FVVOPC(>,  0);
                case IL_FVVLE:  FVVOPC(<=, 0);
                case IL_FVVGE:  FVVOPC(>=, 0);

                case IL_IVSADD: IVSOP(+,  0);
                case IL_IVSSUB: IVSOP(-,  0);
                case IL_IVSMUL: IVSOP(*,  0);
                case IL_IVSDIV: IVSOP(/,  1);
                case IL_IVSLT:  IVSOP(<,  0);
                case IL_IVSGT:  IVSOP(>,  0);
                case IL_IVSLE:  IVSOP(<=, 0);
                case IL_IVSGE:  IVSOP(>=, 0);
                case IL_FVSADD: FVSOP(+,  0);
                case IL_FVSSUB: FVSOP(-,  0);
                case IL_FVSMUL: FVSOP(*,  0);
                case IL_FVSDIV: FVSOP(/,  1);
                case IL_FVSLT:  FVSOPC(<,  0);
                case IL_FVSGT:  FVSOPC(>,  0);
                case IL_FVSLE:  FVSOPC(<=, 0);
                case IL_FVSGE:  FVSOPC(>=, 0);

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

                case IL_IUMINUS: { Value a = POP(); PUSH(Value(-a.ival())); break; }
                case IL_FUMINUS: { Value a = POP(); PUSH(Value(-a.fval())); break; }

                #define VUMINUS(isfloat, type) { \
                    Value a = POP(); \
                    Value res; \
                    int len = VectorLoop(a, Value((type)1), res, true, a.vval()->typeoff); \
                    if (len >= 0) \
                    { \
                        for (int i = 0; i < len; i++) \
                        { \
                            VMTYPEEQ(a.vval()->at(i), isfloat ? V_FLOAT : V_INT); \
                            res.vval()->at(i) = Value(-_VELEM(a, i, isfloat, type)); \
                        } \
                        VectorDec(a, res); \
                        PUSH(res); \
                        break; \
                    } \
                    VMASSERT(false); \
                    break; }
                case IL_IVUMINUS: VUMINUS(false, int)
                case IL_FVUMINUS: VUMINUS(true, float)

                case IL_LOGNOT:
                {
                    Value a = POP();
                    PUSH(!a.DEC().True());    
                    break;
                }

                case IL_I2F:
                {
                    Value a = POP();
                    VMTYPEEQ(a, V_INT);
                    PUSH((float)a.ival());    
                    break;
                }                
                
                case IL_A2S:
                {
                    Value a = POP();
                    TYPE_ASSERT(IsRefNil(a.type));
                    PUSH(NewString(a.ToString(a.type, programprintprefs)));
                    a.DECRTNIL();
                    break;
                }
                    
                case IL_I2A:
                {
                    Value i = POP();
                    VMTYPEEQ(i, V_INT);
                    PUSH(NewInt(i.ival()));
                    break;
                }

                case IL_F2A:
                {
                    Value f = POP();
                    VMTYPEEQ(f, V_FLOAT);
                    PUSH(NewFloat(f.fval()));
                    break;
                }

                case IL_PUSHVAR:   PUSH(vars[*ip++].INC()); break;

                case IL_PUSHFLD:
                case IL_PUSHFLDM: PushDerefField(*ip++); break;
                case IL_PUSHIDXI: PushDerefIdx(POP().ival()); break;
                case IL_PUSHIDXV: PushDerefIdx(GrabIndex(POP())); break;

                case IL_PUSHLOC:
                {
                    int i = *ip++;
                    Value coro = POP();
                    VMTYPEEQ(coro, V_COROUTINE);
                    PUSH(coro.cval()->GetVar(i).INC());
                    coro.DECRT();
                    break;
                }

                case IL_LVALLOC:
                {
                    int lvalop = *ip++;
                    int i = *ip++;
                    Value coro = POP();
                    VMTYPEEQ(coro, V_COROUTINE);
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

                case IL_LVALIDXI: { int lvalop = *ip++; LvalueObj(lvalop, POP().ival()); break; }
                case IL_LVALIDXV: { int lvalop = *ip++; LvalueObj(lvalop, GrabIndex(POP())); break; }
                case IL_LVALFLD:  { int lvalop = *ip++; LvalueObj(lvalop, *ip++); break; }

                case IL_PUSHONCE:
                {
                    auto x = POP();
                    auto &v = TOP();
                    TYPE_ASSERT(IsVector(v.type));
                    v.vval()->Push(x);
                    break;
                }

                case IL_JUMPFAIL:    { auto x = POP(); auto nip = *ip++; if (!x.DEC().True()) { ip = codestart + nip;                }               break; }
                case IL_JUMPFAILR:   { auto x = POP(); auto nip = *ip++; if (!x      .True()) { ip = codestart + nip; PUSH(x);       } else x.DEC(); break; }
                case IL_JUMPFAILN:   { auto x = POP(); auto nip = *ip++; if (!x.DEC().True()) { ip = codestart + nip; PUSH(Value()); }               break; }
                case IL_JUMPNOFAIL:  { auto x = POP(); auto nip = *ip++; if ( x.DEC().True()) { ip = codestart + nip;                }               break; }
                case IL_JUMPNOFAILR: { auto x = POP(); auto nip = *ip++; if ( x      .True()) { ip = codestart + nip; PUSH(x);       } else x.DEC(); break; }

                case IL_ISTYPE:
                {
                    auto to = (type_elem_t)*ip++;
                    auto &v = POP();
                    TYPE_ASSERT(IsRef(v.type));  // Optimizer guarantees we don't have to deal with scalars.
                    v.DECRT();
                    PUSH(v.ref()->typeoff == to);
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

    void PushDerefField(int i) 
    { 
        Value r = POP(); 
        if (!r.ref()) { PUSH(r); return; }  // ?.
        PUSH(r.vval()->at(i).INC());
        r.DECRT(); 
    }

    void PushDerefIdx(int i) 
    { 
        Value r = POP();
        if (!r.ref()) { PUSH(r); return; }  // ?.
        switch (r.ref()->BaseType()) 
        {
            case V_STRUCT:  // Struct::vectortype
            case V_VECTOR:
                IDXErr(i, r.vval()->len, r);
                PUSH(r.vval()->at(i).INC());
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
        TYPE_ASSERT(IsVector(vec.type));
        IDXErr(i, (int)vec.vval()->len, vec);
        Value &a = vec.vval()->at(i);
        LvalueOp(lvalop, a);
        vec.DECRT();
    }

    void LvalueOp(int op, Value &a)
    {
        switch(op)
        {
            case LVO_IVVADD:  { Value b = POP();  _IVOP(+, 0, false, false); a = res;                    break; }
            case LVO_IVVADDR: { Value b = POP();  _IVOP(+, 0, false, false); a = res; PUSH(res.INCRT()); break; }
            case LVO_IVVSUB:  { Value b = POP();  _IVOP(-, 0, false, false); a = res;                    break; }
            case LVO_IVVSUBR: { Value b = POP();  _IVOP(-, 0, false, false); a = res; PUSH(res.INCRT()); break; }
            case LVO_IVVMUL:  { Value b = POP();  _IVOP(*, 0, false, false); a = res;                    break; }
            case LVO_IVVMULR: { Value b = POP();  _IVOP(*, 0, false, false); a = res; PUSH(res.INCRT()); break; }
            case LVO_IVVDIV:  { Value b = POP();  _IVOP(/, 1, false, false); a = res;                    break; }
            case LVO_IVVDIVR: { Value b = POP();  _IVOP(/, 1, false, false); a = res; PUSH(res.INCRT()); break; }

            case LVO_FVVADD:  { Value b = POP();  _FVOP(+, 0, false, false); a = res;                    break; }
            case LVO_FVVADDR: { Value b = POP();  _FVOP(+, 0, false, false); a = res; PUSH(res.INCRT()); break; }
            case LVO_FVVSUB:  { Value b = POP();  _FVOP(-, 0, false, false); a = res;                    break; }
            case LVO_FVVSUBR: { Value b = POP();  _FVOP(-, 0, false, false); a = res; PUSH(res.INCRT()); break; }
            case LVO_FVVMUL:  { Value b = POP();  _FVOP(*, 0, false, false); a = res;                    break; }
            case LVO_FVVMULR: { Value b = POP();  _FVOP(*, 0, false, false); a = res; PUSH(res.INCRT()); break; }
            case LVO_FVVDIV:  { Value b = POP();  _FVOP(/, 1, false, false); a = res;                    break; }
            case LVO_FVVDIVR: { Value b = POP();  _FVOP(/, 1, false, false); a = res; PUSH(res.INCRT()); break; }

            case LVO_IVSADD:  { Value b = POP();  _IVOP(+, 0, true, false); a = res;                    break; }
            case LVO_IVSADDR: { Value b = POP();  _IVOP(+, 0, true, false); a = res; PUSH(res.INCRT()); break; }
            case LVO_IVSSUB:  { Value b = POP();  _IVOP(-, 0, true, false); a = res;                    break; }
            case LVO_IVSSUBR: { Value b = POP();  _IVOP(-, 0, true, false); a = res; PUSH(res.INCRT()); break; }
            case LVO_IVSMUL:  { Value b = POP();  _IVOP(*, 0, true, false); a = res;                    break; }
            case LVO_IVSMULR: { Value b = POP();  _IVOP(*, 0, true, false); a = res; PUSH(res.INCRT()); break; }
            case LVO_IVSDIV:  { Value b = POP();  _IVOP(/, 1, true, false); a = res;                    break; }
            case LVO_IVSDIVR: { Value b = POP();  _IVOP(/, 1, true, false); a = res; PUSH(res.INCRT()); break; }
                                                                      
            case LVO_FVSADD:  { Value b = POP();  _FVOP(+, 0, true, false);  a = res;                    break; }
            case LVO_FVSADDR: { Value b = POP();  _FVOP(+, 0, true, false);  a = res; PUSH(res.INCRT()); break; }
            case LVO_FVSSUB:  { Value b = POP();  _FVOP(-, 0, true, false);  a = res;                    break; }
            case LVO_FVSSUBR: { Value b = POP();  _FVOP(-, 0, true, false);  a = res; PUSH(res.INCRT()); break; }
            case LVO_FVSMUL:  { Value b = POP();  _FVOP(*, 0, true, false);  a = res;                    break; }
            case LVO_FVSMULR: { Value b = POP();  _FVOP(*, 0, true, false);  a = res; PUSH(res.INCRT()); break; }
            case LVO_FVSDIV:  { Value b = POP();  _FVOP(/, 1, true, false);  a = res;                    break; }
            case LVO_FVSDIVR: { Value b = POP();  _FVOP(/, 1, true, false);  a = res; PUSH(res.INCRT()); break; }

            case LVO_IADD:    { Value b = POP();  _IOP(+, 0);                a = res;                    break; }
            case LVO_IADDR:   { Value b = POP();  _IOP(+, 0);                a = res; PUSH(res);         break; }
            case LVO_ISUB:    { Value b = POP();  _IOP(-, 0);                a = res;                    break; }
            case LVO_ISUBR:   { Value b = POP();  _IOP(-, 0);                a = res; PUSH(res);         break; }
            case LVO_IMUL:    { Value b = POP();  _IOP(*, 0);                a = res;                    break; }
            case LVO_IMULR:   { Value b = POP();  _IOP(*, 0);                a = res; PUSH(res);         break; }
            case LVO_IDIV:    { Value b = POP();  _IOP(/, 1);                a = res;                    break; }
            case LVO_IDIVR:   { Value b = POP();  _IOP(/, 1);                a = res; PUSH(res);         break; }
            case LVO_IMOD:    { Value b = POP();  _IOP(%, 1);                a = res;                    break; }
            case LVO_IMODR:   { Value b = POP();  _IOP(%, 1);                a = res; PUSH(res);         break; }
                                                                                                         
            case LVO_FADD:    { Value b = POP();  _FOP(+, 0);                a = res;                    break; }
            case LVO_FADDR:   { Value b = POP();  _FOP(+, 0);                a = res; PUSH(res);         break; }
            case LVO_FSUB:    { Value b = POP();  _FOP(-, 0);                a = res;                    break; }
            case LVO_FSUBR:   { Value b = POP();  _FOP(-, 0);                a = res; PUSH(res);         break; }
            case LVO_FMUL:    { Value b = POP();  _FOP(*, 0);                a = res;                    break; }
            case LVO_FMULR:   { Value b = POP();  _FOP(*, 0);                a = res; PUSH(res);         break; }
            case LVO_FDIV:    { Value b = POP();  _FOP(/, 1);                a = res;                    break; }
            case LVO_FDIVR:   { Value b = POP();  _FOP(/, 1);                a = res; PUSH(res);         break; }
                                                                            
            case LVO_SADD:    { Value b = POP();  _SCAT();                   a = res;                    break; }
            case LVO_SADDR:   { Value b = POP();  _SCAT();                   a = res; PUSH(res.INCRT()); break; }

            case LVO_WRITE:     { Value  b = POP();                          a = b; break; }
            case LVO_WRITER:    { Value &b = TOP();                          a = b; break; }
            case LVO_WRITEREF:  { Value  b = POP();            a.DECRTNIL(); a = b; break; }
            case LVO_WRITERREF: { Value &b = TOP().INCRTNIL(); a.DECRTNIL(); a = b; break; }

            #define PPOP(ret, op, pre, accessor) { \
                if (ret && !pre) PUSH(a); \
                a.accessor() = a.accessor() op 1; \
                if (ret && pre) PUSH(a); \
            }
                
            case LVO_IPP: 
            case LVO_IPPR:  { PPOP(op == LVO_IPPR,  +, true,  ival);  break; }
            case LVO_IMM: 
            case LVO_IMMR:  { PPOP(op == LVO_IMMR,  -, true,  ival);  break; }
            case LVO_IPPP: 
            case LVO_IPPPR: { PPOP(op == LVO_IPPPR, +, false, ival); break; }
            case LVO_IMMP:
            case LVO_IMMPR: { PPOP(op == LVO_IMMPR, -, false, ival); break; }
            case LVO_FPP: 
            case LVO_FPPR:  { PPOP(op == LVO_FPPR,  +, true,  fval);  break; }
            case LVO_FMM: 
            case LVO_FMMR:  { PPOP(op == LVO_FMMR,  -, true,  fval);  break; }
            case LVO_FPPP: 
            case LVO_FPPPR: { PPOP(op == LVO_FPPPR, +, false, fval); break; }
            case LVO_FMMP:
            case LVO_FMMPR: { PPOP(op == LVO_FMMPR, -, false, fval); break; }

            default:
                Error("bytecode format problem (lvalue): " + to_string(op));
        }
    }

    const char *ProperTypeName(const Value &v)
    {
        if (IsRef(v.type))
        {
            auto &ti = GetTypeInfo(v.ref()->typeoff);
            if (ti.vt() == V_STRUCT) return ReverseLookupType(ti.sub);
        }
        return BaseTypeName(v.type);
    }

    void Div0() { Error("division by zero"); } 

    void IDXErr(int i, int n, const Value &v)
    {
        if (i < 0 || i >= n) Error("index " + to_string(i) + " out of range " + to_string(n), v);
    }

    int GrabIndex(const Value &idx)
    {
        auto &v = TOP();
        for (int i = idx.vval()->len - 1; ; i--)
        {
            auto sidx = idx.vval()->at(i);
            VMTYPEEQ(sidx, V_INT);
            if (!i)
            {
                idx.DECRT();
                return sidx.ival();
            }
            TYPE_ASSERT(IsVector(v.type));
            IDXErr(sidx.ival(), v.vval()->len, v);
            auto nv = v.vval()->at(sidx.ival()).INCRT();
            v.DECRT();
            v = nv;
        }
    }

    int VectorLoop(const Value &a, const Value &b, Value &res, bool withscalar, type_elem_t desttype)
    {
        TYPE_ASSERT(IsVector(a.type));
        int len = a.vval()->len;
        if (!withscalar)
        {
            TYPE_ASSERT(IsVector(b.type));
            if (b.vval()->len != len) Error("vectors operation: vector must be same length", a, b);

            if (a.vval()->refc == 1) { res = a; a.vval()->typeoff = desttype; return len; }
            if (b.vval()->refc == 1) { res = b; b.vval()->typeoff = desttype; return len; }
        }
        else
        {
            if (a.vval()->refc == 1) { res = a; return len; }
        }

        res = Value(NewVector(len, desttype));
        res.vval()->len = len;
        return len;
    }

    void VectorDec(const Value &a, const Value &res)
    {
        if (a.vval() != res.vval()) a.DECRT();
    }

    void Push(const Value &v) { PUSH(v); }

    Value Pop() { return POP(); }

    int StructTypeInfo(type_elem_t idx, string &name)
    {
        auto s = bcf->structs()->Get(idx);
        name = s->name()->c_str();
        return s->nfields();
    }

    virtual const char *ReverseLookupType(uint v)
    {
        return bcf->structs()->Get(v)->name()->c_str();
    }

    void Trace(bool on) { trace = on; }

    double Time() { return SecondsSinceStart(); }

    int GC()    // shouldn't really be used, but just in case
    {
        for (int i = 0; i <= sp; i++) stack[i].Mark(stack[i].type);
        for (size_t i = 0; i < bcf->specidents()->size(); i++) vars[i].Mark(vars[i].type);
        vml.LogMark();

        vector<RefObj *> leaks;
        int total = 0;
        vmpool->findleaks([&](void *p)
        {
            total++;
            auto r = (RefObj *)p;
            if (r->typeoff == TYPE_ELEM_VALUEBUF) return;
            if (r->refc > 0) leaks.push_back(r);
            r->refc = -r->refc;
        });

        for (auto p : leaks)
        {
            auto ro = (RefObj *)p;
            ro->refc = 0;
            ro->DECDELETE(false);
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
