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

#include "disasm.h"

namespace lobster
{

VM *g_vm = nullptr;                    // set during the lifetime of a VM object
SlabAlloc *vmpool = nullptr;               // set during the lifetime of a VM object

#ifdef _DEBUG
    #define VM_PROFILER                     // tiny VM slowdown and memory usage when enabled
    #define VM_PROFILER
#endif

enum
{
    INITSTACKSIZE   =   4 * 1024, // *8 bytes each
    DEFMAXSTACKSIZE = 128 * 1024, // *8 bytes each, modest on smallest handheld we support (iPhone 3GS has 256MB)
    STACKMARGIN     =   1 * 1024  // *8 bytes each, max by which the stack could possibly grow in a single call
}; 

#define PUSH(v) (stack[++sp] = (v))
#define TOP() (stack[sp])
#define TOPM(n) (stack[sp - n])
#define POP() (stack[sp--]) // (sp < 0 ? 0/(sp + 1) : stack[sp--])
#define POPN(n) (sp -= (n))
#define TOPPTR() (stack + sp + 1)

VM::VM(const char *_pn, vector<uchar> &&_bytecode_buffer, const void *entry_point, const void *static_bytecode)
      : stack(nullptr), stacksize(0), maxstacksize(DEFMAXSTACKSIZE), sp(-1), ip(nullptr),
        curcoroutine(nullptr), vars(nullptr), codelen(0), codestart(nullptr), byteprofilecounts(nullptr),
        bytecode_buffer(std::move(_bytecode_buffer)),
        bcf(nullptr),
        programprintprefs(10, 10000, false, -1, false), typetable(nullptr),
        currentline(-1), maxsp(-1),
        debugpp(2, 50, true, -1, true), programname(_pn), vml(*this),
        trace(false), trace_tail(true),
        vm_count_ins(0), vm_count_fcalls(0), vm_count_bcalls(0),
        compiled_code_ip(entry_point)
{
    assert(vmpool == nullptr);
    vmpool = new SlabAlloc();

    bcf = bytecode::GetBytecodeFile(static_bytecode ? static_bytecode : bytecode_buffer.data());
    if (bcf->bytecode_version() != LOBSTER_BYTECODE_FORMAT_VERSION)
        throw string("bytecode is from a different version of Lobster");
    vml.uses_frame_state = bcf->uses_frame_state() != 0;

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

    #define F(N, A) f_ins_pointers[IL_##N] = &VM::F_##N;
        ILNAMES
    #undef F

    assert(g_vm == nullptr);
    g_vm = this;
}

VM::~VM()
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

void VM::OneMoreFrame()
{
    // We just landed back into the VM after being suspended inside a gl_frame() call.
    // Emulate the return of gl_frame():
    PUSH(Value(1));  // We're not terminating yet.
    EvalProgram();   // Continue execution as if nothing happened.
}

const TypeInfo &VM::GetVarTypeInfo(int varidx)
{
    return GetTypeInfo((type_elem_t)bcf->specidents()->Get(varidx)->typeidx());
}

const TypeInfo *VM::GetIntVectorType(int which)
{
    auto i = bcf->default_int_vector_types()->Get(which);
    return i < 0 ? nullptr : &GetTypeInfo((type_elem_t)i);
}
const TypeInfo *VM::GetFloatVectorType(int which)
{
    auto i = bcf->default_float_vector_types()->Get(which);
    return i < 0 ? nullptr : &GetTypeInfo((type_elem_t)i);
}

static bool _LeakSorter(void *va, void *vb)
{
    auto a = (RefObj *)va;
    auto b = (RefObj *)vb;
    return a->refc != b->refc
    ? a->refc > b->refc
    : (&a->ti != &b->ti
        ? &a->ti > &b->ti
        : false);
}

void VM::DumpLeaks()
{
    vector<void *> leaks;
    vmpool->findleaks([&](void *p) { leaks.push_back(p); });

    if (!leaks.empty())
    {
        Output(OUTPUT_ERROR,
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
                auto ro = (RefObj *)p;
                switch(ro->ti.t)
                {
                    case V_VALUEBUF:
                    case V_STACKFRAMEBUF:
                        break;

                    case V_STRING:
                    case V_COROUTINE:
                    case V_BOXEDINT:
                    case V_BOXEDFLOAT:
                    case V_VECTOR:
                    case V_STRUCT:
                    {
                        auto s = RefToString(ro, leakpp);
                        fputs((ro->CycleStr() + " = " + s + "\n").c_str(), leakf);
                        break;
                    }

                    default: assert(false);
                }
            }
                        
            fclose(leakf);
        }
    }
                
    vmpool->printstats(false);
}
    
#undef new
ElemObj *VM::NewVector(int initial, int max, const TypeInfo &ti)
{
    if (ti.t == V_VECTOR)
        return new (vmpool->alloc_small(sizeof(LVector))) LVector(initial, max, ti);
    assert(ti.t == V_STRUCT && max == initial && max == ti.len);
    return new (vmpool->alloc(sizeof(LStruct) + sizeof(Value) * max)) LStruct(ti);
}
LString *VM::NewString(size_t l)
{
    return new (vmpool->alloc(sizeof(LString) + l + 1)) LString((int)l); 
}
CoRoutine *VM::NewCoRoutine(const int *rip, const int *vip, CoRoutine *p, const TypeInfo &cti)
{
    assert(cti.t == V_COROUTINE);
    return new (vmpool->alloc(sizeof(CoRoutine))) CoRoutine(sp + 2 /* top of sp + pushed coro */,
                                                            (int)stackframes.size(), rip, vip, p, cti);
}
BoxedInt *VM::NewInt(int i)
{
    return new (vmpool->alloc(sizeof(BoxedInt))) BoxedInt(i); 
}
BoxedFloat *VM::NewFloat(float f)
{
    return new (vmpool->alloc(sizeof(BoxedFloat))) BoxedFloat(f); 
}
#ifdef _WIN32
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif
    
LString *VM::NewString(const char *c, size_t l)
{
    auto s = NewString(l);
    memcpy(s->str(), c, l);
    s->str()[l] = 0;
    return s;
}

LString *VM::NewString(const string &s)
{
    return NewString(s.c_str(), s.size());
}

LString *VM::NewString(const char *c1, size_t l1, const char *c2, size_t l2)
{
    auto s = NewString(l1 + l2);
    memcpy(s->str(),      c1, l1);
    memcpy(s->str() + l1, c2, l2);
    s->str()[l1 + l2] = 0;
    return s;
}

// This function is now way less important than it was when the language was still dynamically typed.
// But ok to leave it as-is for "index out of range" and other errors that are still dynamic.
Value VM::Error(string err, const RefObj *a, const RefObj *b)
{
    if (trace_tail && trace_output.length()) throw trace_output + err;

    auto li = LookupLine(ip - 1, codestart, bcf);  // error is usually in the byte before the current ip
    auto s = string(bcf->filenames()->Get(li->fileidx())->c_str()) + "(" + to_string(li->line()) + "): VM error: " + err;
    if (a) s += "\n   arg: " + ValueDBG(a);
    if (b) s += "\n   arg: " + ValueDBG(b);
    while (sp >= 0 && (!stackframes.size() || sp != stackframes.back().spstart))
    {
        s += "\n   stack: " + to_string_hex((size_t)TOP().any());  // Sadly can't print this properly.
        if (vmpool->pointer_is_in_allocator(TOP().any())) s += ", maybe: " + RefToString(TOP().ref(), debugpp);
        POP();  // We don't DEC here, as we can't know what type it is.
                // This is ok, as we ignore leaks in case of an error anyway.
    }

    for (;;)
    {
        if (!stackframes.size()) break;
        
        string locals;
        int deffun = stackframes.back().definedfunction;
        VarCleanup(s.length() < 10000 ? &locals : nullptr, -2 /* clean up temps always */);

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

    throw s;
}

void VM::VMAssert(bool ok, const char *what) 
{
    if (!ok)
        Error(string("VM internal assertion failure: ") + what); 
}
void VM::VMAssert(bool ok, const char *what, const RefObj *a, const RefObj *b) 
{
    if (!ok)
        Error(string("VM internal assertion failure: ") + what, a, b); 
}

#if defined(_DEBUG) && RTT_ENABLED
    #define STRINGIFY(x) #x
    #define TOSTRING(x) STRINGIFY(x)
    #define VMASSERT(test)             { VMAssert(test, __FILE__ ": " TOSTRING(__LINE__) ": " #test); }
#else
    #define VMASSERT(test)             {}
#endif
#if RTT_ENABLED
    #define VMTYPEEQ(val, vt) VMASSERT((val).type == (vt))
#else
    #define VMTYPEEQ(val, vt) { (void)(val); (void)(vt); }
#endif

string VM::ValueDBG(const RefObj *a)
{
    return RefToString(a, debugpp);
}

string VM::DumpVar(const Value &x, size_t idx, bool dumpglobals)
{
    auto sid = bcf->specidents()->Get((uint)idx);
    auto id = bcf->idents()->Get(sid->ididx());
    if (id->readonly() || id->global() != dumpglobals) return "";
    string name = id->name()->c_str();
    auto static_type = GetVarTypeInfo((int)idx).t;
    #if RTT_ENABLED
        if (static_type != x.type) return "";  // Likely uninitialized.
    #endif
    return "\n   " + name + " = " + x.ToString(static_type, debugpp);
}

void VM::EvalMulti(int nargs, const int *mip, int definedfunction, const int *retip, int tempmask)
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
            auto &desired = GetTypeInfo((type_elem_t)*mip++);
            if (desired.t != V_ANY)
            {
                auto &given = GetTypeInfo((type_elem_t)retip[j]);
                // Have to check the actual value, since given may be a supertype.
                // FIXME: this is slow.
                if ((given.t != desired.t && given.t != V_ANY) ||
                    (IsRef(given.t) && &stack[sp - nargs + j + 1].ref()->ti != &desired))
                {
                    mip += nargs - j;  // Includes the code starting point.
                    goto fail;
                }
            }
            else
            {

            }
        }

        retip += nargs;

        return FunIntro(nargs, codestart + *mip, definedfunction, retip, tempmask);

        fail:;
    }

    string argtypes;
    for (int j = 0; j < nargs; j++)
    {
        auto &ti = GetTypeInfo((type_elem_t)retip[j]);
        Value &v = stack[sp - nargs + j + 1];
        argtypes += ProperTypeName(IsRef(ti.t) && v.ref() ? v.ref()->ti : ti);
        if (j < nargs - 1) argtypes += ", ";
    }
    Error(string("the call ") + bcf->functions()->Get(definedfunction)->name()->c_str() + "(" + argtypes +
            ") did not match any function variants");
}

void VM::FinalStackVarsCleanup()
{
    VMASSERT(sp < 0 && !stackframes.size());

    for (size_t i = 0; i < bcf->specidents()->size(); i++)
    {
        auto sid = bcf->specidents()->Get((uint)i);
        //Output(OUTPUT_INFO, "destructing: %s", bcf->idents()->Get(sid->ididx())->name()->c_str());
        vars[i].DECTYPE(GetTypeInfo((type_elem_t)sid->typeidx()).t);
    }

    #ifdef _DEBUG
        Output(OUTPUT_INFO, "stack at its highest was: %d", maxsp);
    #endif
}
    
int VM::CallerId()
{
    return stackframes.size() ? (int)(stackframes.back().retip - codestart) : -1;
}

int VM::VarCleanup(string *error, int towhere)
{
    auto &stf = stackframes.back();

    assert(sp == stf.spstart);

    ip = stf.funstart;

    auto nargs = *ip++;
    auto freevars = ip + nargs;
    ip += nargs;
    auto ndef = *ip++;
    auto defvars = ip + ndef;

    if (vml.uses_frame_state)
    {
        vml.LogFunctionExit(stf.funstart, defvars, stf.logfunwritestart);
    }

    while (ndef--)
    {
        auto i = *--defvars; 
        if (error) (*error) += DumpVar(vars[i], i, false);
        else vars[i].DECTYPE(GetVarTypeInfo(i).t);
        vars[i] = POP();
    }
    while (nargs--)
    {
        auto i = *--freevars;
        if (error) (*error) += DumpVar(vars[i], i, false);
        else vars[i].DECTYPE(GetVarTypeInfo(i).t);
        vars[i] = POP();
    } 

    ip = stf.retip;

    bool lastunwind = towhere == -1 || towhere == stf.definedfunction;
    auto tempmask = stf.tempmask;

    stackframes.pop_back();

    if (!lastunwind)
    {
        auto untilsp = stackframes.size() ? stackframes.back().spstart : -1;
        if (tempmask && !error)
        {
            for (uint i = 0; i < (uint)min(32, sp - untilsp); i++)
                if (((uint)tempmask) & (1u << i))
                    stack[untilsp + 1 + i].DECRTNIL();
        }
        sp = untilsp;
    }

    return lastunwind;
}

void VM::FunIntro(int nargs_given, const int *newip, int definedfunction, const int *retip, int tempmask)
{
    #ifdef VM_PROFILER
        vm_count_fcalls++;
    #endif

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
        // for most locals, this just saves an nil, only in recursive cases it has an actual value.
        // The reason we don't clear the var after backing it up is that in the DS case,
        // you want to be able to use the old value until a new one gets defined, as in a <- a + 1.
        // clearing it would save the INC and a DEC when it eventually gets overwritten,
        // so maybe we can at some point distinguish between vars that are used with DS and those that are not.
        auto varidx = *ip++;
        PUSH(vars[varidx].INCTYPE(GetVarTypeInfo(varidx).t));
    }
    auto nlogvars = *ip++;

    stackframes.push_back(StackFrame());
    auto &stf = stackframes.back();
    if (vml.uses_frame_state)
    {
        stf.logfunwritestart = vml.LogFunctionEntry(funstart, nlogvars);
        stf.logfunreadstart = vml.logi - nlogvars;
    }
    else
    {
        stf.logfunwritestart = stf.logfunreadstart = 0;
    }
    stf.retip = retip; 
    stf.funstart = funstart;
    stf.definedfunction = definedfunction;
    stf.spstart = sp;
    stf.tempmask = tempmask;

    #ifdef _DEBUG
        if (sp > maxsp) maxsp = sp;
    #endif                        
}

bool VM::FunOut(int towhere, int nrv)
{
    bool bottom = false;

    sp -= nrv;
    auto rvs = TOPPTR();

    for(;;)
    {
        if (!stackframes.size())
        {
            if (towhere >= 0)
                Error(string("\"return from ") + bcf->functions()->Get(towhere)->name()->c_str() + "\" outside of function");
            bottom = true;
            break;
        }
        if(VarCleanup(nullptr, towhere)) break;
    }

    memcpy(TOPPTR(), rvs, nrv * sizeof(Value));
    sp += nrv;

    return bottom;
}

void VM::CoVarCleanup(CoRoutine *co)
{
    // Convenient way to copy everything back onto the stack.
    const int *tip = nullptr;
    auto copylen = co->Resume(sp + 1, stack, stackframes, tip, nullptr);
    auto startsp = sp;
    sp += copylen;

    for (int i = co->stackframecopylen - 1; i >= 0 ; i--)
    {
        auto &stf = stackframes.back();

        // FIXME: guarantee this statically.
        if (stf.spstart != sp)
            g_vm->BuiltinError("internal: can\'t have tempories above a yield.");

        VarCleanup(nullptr, !i ? stf.definedfunction : -2);
    }

    assert(sp == startsp);
    (void)startsp;
}

void VM::CoNonRec(const int *varip)
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

void VM::CoNew()
{
    const int *returnip = codestart + *ip++;
    auto ctidx = (type_elem_t)*ip++;
    CoNonRec(ip);
    curcoroutine = NewCoRoutine(returnip, ip, curcoroutine, GetTypeInfo(ctidx));
    curcoroutine->BackupParentVars(vars);
    int nvars = *ip++;
    ip += nvars;
    PUSH(Value(curcoroutine));
}

void VM::CoDone(const int *retip)
{
    int newtop = curcoroutine->Suspend(sp + 1, stack, stackframes, retip, curcoroutine);
    ip = retip;
    sp = newtop - 1; // top of stack is now coro value from create or resume
}

void VM::CoClean()
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

void VM::CoYield(const int *retip)
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

void VM::CoResume(CoRoutine *co)
{
    if (co->stackstart >= 0)
        Error("cannot resume running coroutine");

    if (!co->active)
        Error("cannot resume finished coroutine");

    PUSH(Value(co));    // this will be the return value for the corresponding yield, and holds the ref for gc

    CoNonRec(co->varip);
    sp += co->Resume(sp + 1, stack, stackframes, ip, curcoroutine);

    curcoroutine = co;

    // must be, since those vars got backed up in it before
    VMASSERT(curcoroutine->stackcopymax >=  *curcoroutine->varip);
    curcoroutine->stackcopylen = *curcoroutine->varip;
    //curcoroutine->BackupParentVars(vars);

    POP().DECTYPE(GetTypeInfo(curcoroutine->ti.yieldtype).t);    // previous current value

    for (int i = *curcoroutine->varip; i > 0; i--)
    {
        auto &var = vars[curcoroutine->varip[i]];
        curcoroutine->stackcopy[i - 1] = var; // no INC, since parent is still on the stack and hold ref for us
        var = POP();
    }

    // the builtin call takes care of the return value
}

void VM::EndEval(Value &ret, ValueType vt)
{
    evalret = ret.ToString(vt, programprintprefs);
    ret.DECTYPE(vt);
    assert(sp == -1);
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

        if (vm_count_fcalls)  // remove trivial VM executions from output
            Output(OUTPUT_INFO, "ins %lld, fcall %lld, bcall %lld", vm_count_ins, vm_count_fcalls, vm_count_bcalls);
    #endif

    throw string("end-eval");
}

void VM::F_PUSHINT() { PUSH(Value(*ip++)); }
void VM::F_PUSHFLT() { PUSH(Value(*(float *)ip)); ip++; }
void VM::F_PUSHNIL() { PUSH(Value()); }

void VM::F_PUSHFUN()
{
    int start = *ip++;
    PUSH(Value(codestart + start));
}

void VM::F_PUSHSTR()
{
    // FIXME: have a way that constant strings can stay in the bytecode,
    // or at least preallocate them all
    auto fb_s = bcf->stringtable()->Get(*ip++);
    auto s = NewString(fb_s->c_str(), fb_s->Length());
    PUSH(Value(s));
}

void VM::F_CALL()
{
    auto nargs = *ip++;
    auto fvar = *ip++;
    auto fun = *ip++;
    auto tm = *ip++;
    FunIntro(nargs, codestart + fun, fvar, ip, tm);
}

void VM::F_CALLMULTI()
{
    auto nargs = *ip++;
    auto fvar = *ip++;
    auto fun = *ip++;
    auto tm = *ip++;
    EvalMulti(nargs, codestart + fun, fvar, ip, tm);
}

void VM::F_FUNMULTI() { VMASSERT(0); }

void VM::F_CALLVCOND()
{
    // FIXME: don't need to check for function value again below if false
    if (!TOP().True()) { ip += 2; } else F_CALLV();
}

void VM::F_CALLV()
{
    Value fun = POP();
    VMTYPEEQ(fun, V_FUNCTION);
    auto nargs = *ip++;
    auto tm = *ip++;
    FunIntro(nargs, fun.ip(), -1, ip, tm);
}

void VM::F_YIELD() { CoYield(ip); }

void VM::F_FUNSTART() { VMASSERT(0); }
void VM::F_FUNEND() { FunOut(-1, 1); }

void VM::F_RETURN()
{
    int df = *ip++;
    int nrv = *ip++;
    int tidx = *ip++;
    if(FunOut(df, nrv))
    {
        assert(nrv == 1);
        EndEval(POP(), GetTypeInfo((type_elem_t)tidx).t); 
    }
}

void VM::F_EXIT()
{
    int tidx = *ip++;
    EndEval(POP(), GetTypeInfo((type_elem_t)tidx).t);
}

void VM::F_CONT1()
{
    auto nf = natreg.nfuns[*ip++];
    POP();  // return value from body.
    nf->cont1();
    PUSH(Value());
}

void VM::F_CONT1REF()
{
    auto nf = natreg.nfuns[*ip++];
    POP().DECRTNIL();  // return value from body.
    nf->cont1();
    PUSH(Value());
}

#define FORLOOP(L, V, iterref, bodyref) { \
    auto forstart = ip - 1; \
    auto tm = *ip++; \
    auto bodyret = POP(); \
    if (bodyref) bodyret.DECRTNIL(); \
    auto &body = TOP(); \
    auto &iter = TOPM(1); \
    auto &i = TOPM(2); \
    TYPE_ASSERT(i.type == V_INT); \
    i.setival(i.ival() + 1); \
    int len = 0; \
    if (i.ival() < (len = (L))) { \
        int nargs = body.ip()[1]; \
        if (nargs) { PUSH(V); if (nargs > 1) PUSH(i); } /* FIXME: make this static? */ \
        FunIntro(nargs, body.ip(), -1, forstart, tm); \
        return; \
    } \
    (void)POP(); /* body */ \
    if (iterref) TOP().DECRT(); \
    (void)POP(); /* iter */ \
    (void)POP(); /* i */ \
}

void VM::F_IFOR()    { FORLOOP(iter.ival(), i, false, false); }
void VM::F_IFORREF() { FORLOOP(iter.ival(), i, false, true); }
void VM::F_VFOR()    { FORLOOP(iter.eval()->Len(), iter.eval()->AtInc(i.ival()), true, false); }
void VM::F_VFORREF() { FORLOOP(iter.eval()->Len(), iter.eval()->AtInc(i.ival()), true, true); }
void VM::F_SFOR()    { FORLOOP(iter.sval()->len, Value((int)((uchar *)iter.sval()->str())[i.ival()]), true, false); }
void VM::F_SFORREF() { FORLOOP(iter.sval()->len, Value((int)((uchar *)iter.sval()->str())[i.ival()]), true, true); }

void VM::F_BCALL()
{
    #ifdef VM_PROFILER
        vm_count_bcalls++;
    #endif
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
    #if RTT_ENABLED
        // see if any builtin function is lying about what type it returns
        // other function types return intermediary values that don't correspond to final return values
        if (nf->ncm == NCM_NONE)
        { 
            for (size_t i = 0; i < nf->retvals.v.size(); i++)
            {
                auto t = (TOPPTR() - nf->retvals.v.size() + i)->type;
                auto u = nf->retvals.v[i].type->t;
                TYPE_ASSERT(t == u || u == V_ANY || u == V_NIL || (u == V_VECTOR && t == V_STRUCT));   
            }
            TYPE_ASSERT(nf->retvals.v.size() || TOP().type == V_NIL);
        }
    #endif
}
                
void VM::F_NEWVEC()
{
    auto type = (type_elem_t)*ip++;
    auto len = *ip++;
    auto vec = NewVector(len, len, GetTypeInfo(type));
    if (len) vec->Init(TOPPTR() - len, len, false);
    POPN(len);
    PUSH(Value(vec));
}

void VM::F_POP()    { POP(); }
void VM::F_POPREF() { POP().DECRTNIL(); }

void VM::F_DUP()    { auto x = TOP();            PUSH(x); }
void VM::F_DUPREF() { auto x = TOP().INCRTNIL(); PUSH(x); }

#define REFOP(exp) { res = exp; a.DECRTNIL(); b.DECRTNIL(); }
#define GETARGS() Value b = POP(); Value a = POP()
#define TYPEOP(op, extras, field, errstat) Value res; errstat; \
    if (extras & 1 && b.field == 0) Div0(); res = a.field op b.field;

#define _IOP(op, extras)  TYPEOP(op, extras, ival(), VMASSERT(a.type == V_INT && b.type == V_INT))
#define _FOP(op, extras)  TYPEOP(op, extras, fval(), VMASSERT(a.type == V_FLOAT && b.type == V_FLOAT))

#define _VELEM(a, i, isfloat, T) (isfloat ? (T)a.eval()->At(i).fval() : (T)a.eval()->At(i).ival())
#define _VOP(op, extras, T, isfloat, withscalar, comp) Value res; { \
    int len = VectorLoop(a, b, res, withscalar, comp ? GetTypeInfo(TYPE_ELEM_VECTOR_OF_INT) : a.eval()->ti); \
    for (int j = 0; j < len; j++) \
    { \
        if (withscalar) VMTYPEEQ(b, isfloat ? V_FLOAT : V_INT) else VMTYPEEQ(b.eval()->At(j), isfloat ? V_FLOAT : V_INT); \
        auto bv = withscalar ? (isfloat ? (T)b.fval() : (T)b.ival()) : _VELEM(b, j, isfloat, T); \
        if (extras&1 && bv == 0) Div0(); \
        VMTYPEEQ(a.eval()->At(j), isfloat ? V_FLOAT : V_INT); \
        res.eval()->At(j) = Value(_VELEM(a, j, isfloat, T) op bv); \
    } \
    a.DECRT(); \
    if (!withscalar) b.DECRT(); \
} 
#define _IVOP(op, extras, withscalar, icomp) _VOP(op, extras, int, false, withscalar, icomp)
#define _FVOP(op, extras, withscalar, fcomp) _VOP(op, extras, float, true, withscalar, fcomp)
                    
#define _SCAT()  Value res; REFOP(NewString(a.sval()->str(), a.sval()->len, b.sval()->str(), b.sval()->len))
#define _SOP(op) Value res; REFOP((*a.sval()) op (*b.sval()))

#define ACOMPEN(op) { GETARGS(); Value res; REFOP(a.any() op b.any()); PUSH(res); }
                                           
#define IOP(op, extras)    { GETARGS(); _IOP(op, extras);                PUSH(res); }
#define FOP(op, extras)    { GETARGS(); _FOP(op, extras);                PUSH(res); }
#define IVVOP(op, extras)  { GETARGS(); _IVOP(op, extras, false, false); PUSH(res); }
#define IVVOPC(op, extras) { GETARGS(); _IVOP(op, extras, false, true);  PUSH(res); }
#define FVVOP(op, extras)  { GETARGS(); _FVOP(op, extras, false, false); PUSH(res); }
#define FVVOPC(op, extras) { GETARGS(); _FVOP(op, extras, false, true);  PUSH(res); }
#define IVSOP(op, extras)  { GETARGS(); _IVOP(op, extras, true, false);  PUSH(res); }
#define IVSOPC(op, extras) { GETARGS(); _IVOP(op, extras, true, true);   PUSH(res); }
#define FVSOP(op, extras)  { GETARGS(); _FVOP(op, extras, true, false);  PUSH(res); }
#define FVSOPC(op, extras) { GETARGS(); _FVOP(op, extras, true, true);   PUSH(res); }
#define SOP(op)            { GETARGS(); _SOP(op);                        PUSH(res); }
#define SCAT()             { GETARGS(); _SCAT();                         PUSH(res); }

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

void VM::F_IVVADD() { IVVOP(+,  0);  }
void VM::F_IVVSUB() { IVVOP(-,  0);  }
void VM::F_IVVMUL() { IVVOP(*,  0);  }
void VM::F_IVVDIV() { IVVOP(/,  1);  }
void VM::F_IVVMOD() { VMASSERT(0);   }
void VM::F_IVVLT()  { IVVOP(<,  0);  }
void VM::F_IVVGT()  { IVVOP(>,  0);  }
void VM::F_IVVLE()  { IVVOP(<=, 0);  }
void VM::F_IVVGE()  { IVVOP(>=, 0);  }
void VM::F_FVVADD() { FVVOP(+,  0);  }
void VM::F_FVVSUB() { FVVOP(-,  0);  }
void VM::F_FVVMUL() { FVVOP(*,  0);  }
void VM::F_FVVDIV() { FVVOP(/,  1);  }
void VM::F_FVVMOD() { VMASSERT(0);   }
void VM::F_FVVLT()  { FVVOPC(<,  0); }
void VM::F_FVVGT()  { FVVOPC(>,  0); }
void VM::F_FVVLE()  { FVVOPC(<=, 0); }
void VM::F_FVVGE()  { FVVOPC(>=, 0); }

void VM::F_IVSADD() { IVSOP(+,  0);  }
void VM::F_IVSSUB() { IVSOP(-,  0);  }
void VM::F_IVSMUL() { IVSOP(*,  0);  }
void VM::F_IVSDIV() { IVSOP(/,  1);  }
void VM::F_IVSMOD() { VMASSERT(0);   }
void VM::F_IVSLT()  { IVSOP(<,  0);  }
void VM::F_IVSGT()  { IVSOP(>,  0);  }
void VM::F_IVSLE()  { IVSOP(<=, 0);  }
void VM::F_IVSGE()  { IVSOP(>=, 0);  }
void VM::F_FVSADD() { FVSOP(+,  0);  }
void VM::F_FVSSUB() { FVSOP(-,  0);  }
void VM::F_FVSMUL() { FVSOP(*,  0);  }
void VM::F_FVSDIV() { FVSOP(/,  1);  }
void VM::F_FVSMOD() { VMASSERT(0);   }
void VM::F_FVSLT()  { FVSOPC(<,  0); }
void VM::F_FVSGT()  { FVSOPC(>,  0); }
void VM::F_FVSLE()  { FVSOPC(<=, 0); }
void VM::F_FVSGE()  { FVSOPC(>=, 0); }

void VM::F_AEQ() { ACOMPEN(==); }
void VM::F_ANE() { ACOMPEN(!=); }
                    
void VM::F_IADD() { IOP(+,  0); }
void VM::F_ISUB() { IOP(-,  0); }
void VM::F_IMUL() { IOP(*,  0); }
void VM::F_IDIV() { IOP(/ , 1); }
void VM::F_IMOD() { IOP(%,  1); }
void VM::F_ILT()  { IOP(<,  0); }
void VM::F_IGT()  { IOP(>,  0); }
void VM::F_ILE()  { IOP(<=, 0); }
void VM::F_IGE()  { IOP(>=, 0); }
void VM::F_IEQ()  { IOP(==, 0); }
void VM::F_INE()  { IOP(!=, 0); }

void VM::F_FADD() { FOP(+,  0); }
void VM::F_FSUB() { FOP(-,  0); }
void VM::F_FMUL() { FOP(*,  0); }
void VM::F_FDIV() { FOP(/,  1); }
void VM::F_FMOD() { VMASSERT(0); }
void VM::F_FLT()  { FOP(<,  0); }
void VM::F_FGT()  { FOP(>,  0); }
void VM::F_FLE()  { FOP(<=, 0); }
void VM::F_FGE()  { FOP(>=, 0); }
void VM::F_FEQ()  { FOP(==, 0); }
void VM::F_FNE()  { FOP(!=, 0); }

void VM::F_SADD() { SCAT();  }
void VM::F_SSUB() { VMASSERT(0); }
void VM::F_SMUL() { VMASSERT(0); }
void VM::F_SDIV() { VMASSERT(0); }
void VM::F_SMOD() { VMASSERT(0); }
void VM::F_SLT()  { SOP(<);  }
void VM::F_SGT()  { SOP(>);  }
void VM::F_SLE()  { SOP(<=); }
void VM::F_SGE()  { SOP(>=); }
void VM::F_SEQ()  { SOP(==); }
void VM::F_SNE()  { SOP(!=); }

void VM::F_IUMINUS() { Value a = POP(); PUSH(Value(-a.ival())); }
void VM::F_FUMINUS() { Value a = POP(); PUSH(Value(-a.fval())); }

#define VUMINUS(isfloat, type) { \
    Value a = POP(); \
    Value res; \
    int len = VectorLoop(a, Value((type)1), res, true, a.eval()->ti); \
    if (len >= 0) \
    { \
        for (int i = 0; i < len; i++) \
        { \
            VMTYPEEQ(a.eval()->At(i), isfloat ? V_FLOAT : V_INT); \
            res.eval()->At(i) = Value(-_VELEM(a, i, isfloat, type)); \
        } \
        a.DECRT(); \
        PUSH(res); \
        return; \
    } \
    VMASSERT(false); \
    }
void VM::F_IVUMINUS() { VUMINUS(false, int) }
void VM::F_FVUMINUS() { VUMINUS(true, float) }

void VM::F_LOGNOT()
{
    Value a = POP();
    PUSH(!a.True());
}
void VM::F_LOGNOTREF()
{
    Value a = POP();
    bool b = a.True();
    PUSH(!b);
    if (b) a.DECRT();
}

#define BITOP(op) { GETARGS(); PUSH(a.ival() op b.ival()); }
void VM::F_BINAND() { BITOP(&);  }
void VM::F_BINOR()  { BITOP(|);  }
void VM::F_XOR()    { BITOP(^);  }
void VM::F_ASL()    { BITOP(<<); }
void VM::F_ASR()    { BITOP(>>); }
void VM::F_NEG()    { auto a = POP(); PUSH(~a.ival()); }

void VM::F_I2F()
{
    Value a = POP();
    VMTYPEEQ(a, V_INT);
    PUSH((float)a.ival()); 
}                
                
void VM::F_A2S()
{
    Value a = POP();
    TYPE_ASSERT(IsRefNil(a.type));
    PUSH(NewString(a.ToString(a.ref() ? a.ref()->ti.t : V_NIL, programprintprefs)));
    a.DECRTNIL();
}
                    
void VM::F_I2A()
{
    Value i = POP();
    VMTYPEEQ(i, V_INT);
    PUSH(NewInt(i.ival()));
}

void VM::F_F2A()
{
    Value f = POP();
    VMTYPEEQ(f, V_FLOAT);
    PUSH(NewFloat(f.fval()));
}

void VM::F_E2B()
{
    Value a = POP();
    PUSH(a.True());
}   

void VM::F_E2BREF()
{
    Value a = POP();
    PUSH(a.True());
    a.DECRTNIL();
}  

void VM::F_PUSHVAR()    { PUSH(vars[*ip++]); }
void VM::F_PUSHVARREF() { PUSH(vars[*ip++].INCRTNIL()); }

void VM::F_PUSHFLD()  { PushDerefField(*ip++); }
void VM::F_PUSHFLDM() { PushDerefField(*ip++); }
void VM::F_PUSHIDXI() { PushDerefIdx(POP().ival()); }
void VM::F_PUSHIDXV() { PushDerefIdx(GrabIndex(POP())); }

void VM::F_PUSHLOC()
{
    int i = *ip++;
    Value coro = POP();
    VMTYPEEQ(coro, V_COROUTINE);
    PUSH(coro.cval()->GetVar(i));
    TOP().INCTYPE(GetVarTypeInfo(i).t);
    coro.DECRT();
}

void VM::F_LVALLOC()
{
    int lvalop = *ip++;
    int i = *ip++;
    Value coro = POP();
    VMTYPEEQ(coro, V_COROUTINE);
    Value &a = coro.cval()->GetVar(i);
    LvalueOp(lvalop, a);
    coro.DECRT();
}

void VM::F_LVALVAR()   
{
    int lvalop = *ip++; 
    LvalueOp(lvalop, vars[*ip++]);
}

void VM::F_LVALIDXI() { int lvalop = *ip++; LvalueObj(lvalop, POP().ival()); }
void VM::F_LVALIDXV() { int lvalop = *ip++; LvalueObj(lvalop, GrabIndex(POP())); }
void VM::F_LVALFLD()  { int lvalop = *ip++; LvalueObj(lvalop, *ip++); }

void VM::F_JUMP() { ip = codestart + *ip; }

void VM::F_JUMPFAIL()       { auto x = POP(); auto nip = *ip++;               if (!x.True()) { ip = codestart + nip;                }                    }
void VM::F_JUMPFAILR()      { auto x = POP(); auto nip = *ip++;               if (!x.True()) { ip = codestart + nip; PUSH(x);       }                    }
void VM::F_JUMPFAILN()      { auto x = POP(); auto nip = *ip++;               if (!x.True()) { ip = codestart + nip; PUSH(Value()); }                    }
void VM::F_JUMPNOFAIL()     { auto x = POP(); auto nip = *ip++;               if ( x.True()) { ip = codestart + nip;                }                    }
void VM::F_JUMPNOFAILR()    { auto x = POP(); auto nip = *ip++;               if ( x.True()) { ip = codestart + nip; PUSH(x);       }                    }
void VM::F_JUMPFAILREF()    { auto x = POP(); auto nip = *ip++; x.DECRTNIL(); if (!x.True()) { ip = codestart + nip;                }                    }
void VM::F_JUMPFAILRREF()   { auto x = POP(); auto nip = *ip++;               if (!x.True()) { ip = codestart + nip; PUSH(x);       } else x.DECRTNIL(); }
void VM::F_JUMPFAILNREF()   { auto x = POP(); auto nip = *ip++; x.DECRTNIL(); if (!x.True()) { ip = codestart + nip; PUSH(Value()); }                    }
void VM::F_JUMPNOFAILREF()  { auto x = POP(); auto nip = *ip++; x.DECRTNIL(); if ( x.True()) { ip = codestart + nip;                }                    }
void VM::F_JUMPNOFAILRREF() { auto x = POP(); auto nip = *ip++;               if ( x.True()) { ip = codestart + nip; PUSH(x);       } else x.DECRTNIL(); }

void VM::F_ISTYPE()
{
    auto to = (type_elem_t)*ip++;
    auto v = POP(); 
    auto &ti = GetTypeInfo(to);
    // Optimizer guarantees we don't have to deal with scalars.
    if (v.refnil()) PUSH(&v.ref()->ti == &ti);
    else PUSH(ti.t == V_NIL);
    v.DECRTNIL();
}

void VM::F_COCL()  { PUSH(Value(0, V_YIELD)); }  // This value never gets used anywhere, just a placeholder.
void VM::F_CORO()  { CoNew(); }
void VM::F_COEND() { CoClean(); }

void VM::F_LOGREAD()
{
    auto val = POP();
    PUSH(vml.LogGet(val, *ip++, false));
}
void VM::F_LOGREADREF()
{
    auto val = POP();
    PUSH(vml.LogGet(val, *ip++, true));
}

void VM::EvalProgram()
{
    try
    {
        for (;;)
        {
            #ifdef VM_COMPILED_CODE_MODE
                compiled_code_ip = ((block_t)compiled_code_ip)();
            #else
                #ifdef _DEBUG
                    if (trace)
                    {
                        if (!trace_tail) trace_output.clear();
                        DisAsmIns(trace_output, ip, codestart, typetable, bcf);
                        trace_output += " [";
                        trace_output += to_string(sp + 1);
                        trace_output += "] - ";
                        #if RTT_ENABLED
                        if (sp >= 0) { auto x = TOP();   trace_output += x.ToString(x.type, debugpp); }
                        if (sp >= 1) { auto x = TOPM(1); trace_output += " "; trace_output += x.ToString(x.type, debugpp); }
                        #endif
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
                    auto code_idx = size_t(ip - codestart);
                    assert(code_idx < codelen);
                    byteprofilecounts[code_idx]++;
                    vm_count_ins++;
                #endif
            
                auto op = *ip++;

                #ifdef _DEBUG
                    if (op < 0 || op >= IL_MAX_OPS)
                        Error("bytecode format problem: " + to_string(op));
                #endif

                ((*this).*(f_ins_pointers[op]))();
            #endif
        }
    }
    catch (string &s)
    {
        if (s != "end-eval") throw s;
    }
}

void VM::PushDerefField(int i) 
{ 
    Value r = POP(); 
    if (!r.ref()) { PUSH(r); return; }  // ?.
    PUSH(r.eval()->AtInc(i));
    r.DECRT(); 
}

void VM::PushDerefIdx(int i) 
{ 
    Value r = POP();
    if (!r.ref()) { PUSH(r); return; }  // ?.
    switch (r.ref()->ti.t) 
    {
        case V_STRUCT:  // Struct::vectortype
        case V_VECTOR:
            IDXErr(i, r.eval()->Len(), r.eval());
            PUSH(r.eval()->AtInc(i));
            break;
        case V_STRING:
            IDXErr(i, r.sval()->len, r.sval()); 
            PUSH(Value((int)((uchar *)r.sval()->str())[i]));
            break;
        default:
            VMASSERT(false); 
    } 
    r.DECRT(); 
}

void VM::LvalueObj(int lvalop, int i)
{
    Value vec = POP();
    TYPE_ASSERT(IsVector(vec.type));
    IDXErr(i, (int)vec.eval()->Len(), vec.eval());
    Value &a = vec.eval()->At(i);
    LvalueOp(lvalop, a);
    vec.DECRT();
}

void VM::LvalueOp(int op, Value &a)
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
            a.set##accessor(a.accessor() op 1); \
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

string VM::ProperTypeName(const TypeInfo &ti)
{
    switch (ti.t)
    {
        case V_STRUCT: return ReverseLookupType(ti.structidx);
        case V_NIL: return ProperTypeName(GetTypeInfo(ti.subt)) + "?";
        case V_VECTOR: return "[" + ProperTypeName(GetTypeInfo(ti.subt)) + "]";
    }
    return BaseTypeName(ti.t);
}

void VM::IDXErr(int i, int n, const RefObj *v)
{
    if (i < 0 || i >= n) Error("index " + to_string(i) + " out of range " + to_string(n), v);
}

int VM::GrabIndex(const Value &idx)
{
    auto &v = TOP();
    for (int i = idx.eval()->Len() - 1; ; i--)
    {
        auto sidx = idx.eval()->At(i);
        VMTYPEEQ(sidx, V_INT);
        if (!i)
        {
            idx.DECRT();
            return sidx.ival();
        }
        TYPE_ASSERT(IsVector(v.type));
        IDXErr(sidx.ival(), v.eval()->Len(), v.eval());
        auto nv = v.eval()->At(sidx.ival()).INCRT();
        v.DECRT();
        v = nv;
    }
}

int VM::VectorLoop(const Value &a, const Value &b, Value &res, bool withscalar, const TypeInfo &desttype)
{
    TYPE_ASSERT(IsVector(a.type));
    int len = a.eval()->Len();
    if (!withscalar)
    {
        TYPE_ASSERT(IsVector(b.type));
        if (b.eval()->Len() != len) Error("vectors operation: vector must be same length", a.eval(), b.eval());
    }

    res = Value(NewVector(len, len, desttype));
    return len;
}

void VM::Push(const Value &v) { PUSH(v); }

Value VM::Pop() { return POP(); }

string VM::StructName(const TypeInfo &ti)
{
    return bcf->structs()->Get(ti.structidx)->name()->c_str();
}

const char *VM::ReverseLookupType(uint v)
{
    return bcf->structs()->Get(v)->name()->c_str();
}

int VM::GC()    // shouldn't really be used, but just in case
{
    for (int i = 0; i <= sp; i++)
    {
        //stack[i].Mark(?);

        // TODO: we could actually walk the stack here and recover correct types, but it is so easy to avoid
        // this error that that may not be worth it.
        if (stack[i].True())  // Typically all nil
            Error("collect_garbage() must be called from a top level function");
    }
    for (uint i = 0; i < bcf->specidents()->size(); i++) vars[i].Mark(GetVarTypeInfo(i).t);
    vml.LogMark();

    vector<RefObj *> leaks;
    int total = 0;
    vmpool->findleaks([&](void *p)
    {
        total++;
        auto r = (RefObj *)p;
        if (&r->ti == &GetTypeInfo(TYPE_ELEM_VALUEBUF) || 
            &r->ti == &GetTypeInfo(TYPE_ELEM_STACKFRAMEBUF)) return;
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

void RunBytecode(const char *programname, vector<uchar> &&bytecode, const void *entry_point,
                 const void *static_bytecode)
{
    new VM(programname, std::move(bytecode), entry_point, static_bytecode);  // Sets up g_vm
    g_vm->EvalProgram();
}

}  // namespace lobster
