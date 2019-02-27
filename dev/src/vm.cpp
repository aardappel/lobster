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

#include "lobster/stdafx.h"

#include "lobster/disasm.h"

namespace lobster {

#ifndef NDEBUG
    #define VM_PROFILER              // tiny VM slowdown and memory usage when enabled
#endif

#ifdef VM_COMPILED_CODE_MODE
    #define VM_OP_PASSTHRU ip, fcont
    #pragma warning (disable: 4458)  // ip hides class member, which we rely on
    #pragma warning (disable: 4100)  // ip may not be touched
#else
    #define VM_OP_PASSTHRU
#endif

enum {
    // *8 bytes each
    INITSTACKSIZE   =   4 * 1024,
    // *8 bytes each, modest on smallest handheld we support (iPhone 3GS has 256MB).
    DEFMAXSTACKSIZE = 128 * 1024,
    // *8 bytes each, max by which the stack could possibly grow in a single call.
    STACKMARGIN     =   1 * 1024
};

#define MEASURE_INSTRUCTION_COMBINATIONS 0
#if MEASURE_INSTRUCTION_COMBINATIONS
map<pair<int, int>, size_t> instruction_combinations;
int last_instruction_opc = -1;
#endif

VM::VM(NativeRegistry &nfr, string_view _pn, string &_bytecode_buffer, const void *entry_point,
       const void *static_bytecode, const vector<string> &args)
      : nfr(nfr), maxstacksize(DEFMAXSTACKSIZE),
        bytecode_buffer(std::move(_bytecode_buffer)), programname(_pn),
        compiled_code_ip(entry_point), compiled_code_bc(static_bytecode), program_args(args) {
    auto bcfb = (uchar *)(static_bytecode ? static_bytecode : bytecode_buffer.data());
    bcf = bytecode::GetBytecodeFile(bcfb);
    if (bcf->bytecode_version() != LOBSTER_BYTECODE_FORMAT_VERSION)
        THROW_OR_ABORT(string("bytecode is from a different version of Lobster"));
    codelen = bcf->bytecode()->Length();
    if (FLATBUFFERS_LITTLEENDIAN) {
        // We can use the buffer directly.
        codestart = (const int *)bcf->bytecode()->Data();
        typetable = (const type_elem_t *)bcf->typetable()->Data();
    } else {
        for (uint i = 0; i < codelen; i++)
            codebigendian.push_back(bcf->bytecode()->Get(i));
        codestart = codebigendian.data();

        for (uint i = 0; i < bcf->typetable()->Length(); i++)
            typetablebigendian.push_back((type_elem_t)bcf->typetable()->Get(i));
        typetable = typetablebigendian.data();
    }
    #ifndef VM_COMPILED_CODE_MODE
        ip = codestart;
    #endif
    vars = new Value[bcf->specidents()->size()];
    stack = new Value[stacksize = INITSTACKSIZE];
    #ifdef VM_PROFILER
        byteprofilecounts = new uint64_t[codelen];
        memset(byteprofilecounts, 0, sizeof(uint64_t) * codelen);
    #endif
    vml.LogInit(bcfb);
    InstructionPointerInit();
    constant_strings.resize(bcf->stringtable()->size());
}

VM::~VM() {
    TerminateWorkers();
    if (stack) delete[] stack;
    if (vars)  delete[] vars;
    if (byteprofilecounts) delete[] byteprofilecounts;
}

void VM::OneMoreFrame() {
    // We just landed back into the VM after being suspended inside a gl_frame() call.
    // Emulate the return of gl_frame():
    VM_PUSH(Value(1));  // We're not terminating yet.
    EvalProgram();   // Continue execution as if nothing happened.
}

const TypeInfo &VM::GetVarTypeInfo(int varidx) {
    return GetTypeInfo((type_elem_t)bcf->specidents()->Get(varidx)->typeidx());
}

type_elem_t VM::GetIntVectorType(int which) {
    auto i = bcf->default_int_vector_types()->Get(which);
    return type_elem_t(i < 0 ? -1 : i);
}
type_elem_t VM::GetFloatVectorType(int which) {
    auto i = bcf->default_float_vector_types()->Get(which);
    return type_elem_t(i < 0 ? -1 : i);
}

static bool _LeakSorter(void *va, void *vb) {
    auto a = (RefObj *)va;
    auto b = (RefObj *)vb;
    return a->refc != b->refc
    ? a->refc > b->refc
    : (a->tti != b->tti
        ? a->tti > b->tti
        : false);
}

void VM::DumpVal(RefObj *ro, const char *prefix) {
    ostringstream ss;
    ss << prefix << ": ";
    RefToString(*this, ss, ro, debugpp);
    ss << " (" << ro->refc << "): " << (size_t)ro;
    LOG_DEBUG(ss.str());
}

void VM::DumpFileLine(const int *fip, ostringstream &ss) {
    // error is usually in the byte before the current ip.
    auto li = LookupLine(fip - 1, codestart, bcf);
    ss << bcf->filenames()->Get(li->fileidx())->string_view() << '(' << li->line() << ')';
}

void VM::DumpLeaks() {
    vector<void *> leaks = pool.findleaks();
    auto filename = "leaks.txt";
    if (leaks.empty()) {
        if (FileExists(filename)) FileDelete(filename);
    } else {
        LOG_ERROR("LEAKS FOUND (this indicates cycles in your object graph, or a bug in"
                             " Lobster)");
        ostringstream ss;
        sort(leaks.begin(), leaks.end(), _LeakSorter);
        PrintPrefs leakpp = debugpp;
        leakpp.cycles = 0;
        for (auto p : leaks) {
            auto ro = (RefObj *)p;
            switch(ro->ti(*this).t) {
                case V_VALUEBUF:
                case V_STACKFRAMEBUF:
                    break;
                case V_STRING:
                case V_COROUTINE:
                case V_RESOURCE:
                case V_VECTOR:
                case V_STRUCT_R:
                case V_STRUCT_S:
                case V_CLASS: {
                    ro->CycleStr(ss);
                    ss << " = ";
                    RefToString(*this, ss, ro, leakpp);
                    #if DELETE_DELAY
                    ss << " ";
                    DumpFileLine(ro->alloc_ip, ss);
                    ss << " " << (size_t)ro;
                    #endif
                    ss << "\n";
                    break;
                }
                default: assert(false);
            }
        }
        #ifdef _DEBUG
            LOG_ERROR(ss.str());
        #else
            if (leaks.size() < 50) {
                LOG_ERROR(ss.str());
            } else {
                LOG_ERROR(leaks.size(), " leaks, details in ", filename);
                WriteFile(filename, false, ss.str());
            }
        #endif
    }
    pool.printstats(false);
}

void VM::OnAlloc(RefObj *ro) {
    #if DELETE_DELAY
        LOG_DEBUG("alloc: ", (size_t)ro);
        ro->alloc_ip = ip;
    #else
        (void)ro;
    #endif
}

#undef new
LVector *VM::NewVec(intp initial, intp max, type_elem_t tti) {
    assert(GetTypeInfo(tti).t == V_VECTOR);
    auto v = new (pool.alloc_small(sizeof(LVector))) LVector(*this, initial, max, tti);
    OnAlloc(v);
    return v;
}
LObject *VM::NewObject(intp max, type_elem_t tti) {
    assert(IsUDT(GetTypeInfo(tti).t));
    auto s = new (pool.alloc(sizeof(LObject) + sizeof(Value) * max)) LObject(tti);
    OnAlloc(s);
    return s;
}
LString *VM::NewString(size_t l) {
    auto s = new (pool.alloc(sizeof(LString) + l + 1)) LString((int)l);
    OnAlloc(s);
    return s;
}
LCoRoutine *VM::NewCoRoutine(InsPtr rip, const int *vip, LCoRoutine *p, type_elem_t cti) {
    assert(GetTypeInfo(cti).t == V_COROUTINE);
    auto c = new (pool.alloc(sizeof(LCoRoutine)))
       LCoRoutine(sp + 2 /* top of sp + pushed coro */, (int)stackframes.size(), rip, vip, p, cti);
    OnAlloc(c);
    return c;
}
LResource *VM::NewResource(void *v, const ResourceType *t) {
    auto r = new (pool.alloc(sizeof(LResource))) LResource(v, t);
    OnAlloc(r);
    return r;
}
#ifdef _WIN32
#ifndef NDEBUG
#define new DEBUG_NEW
#endif
#endif

LString *VM::NewString(string_view s) {
    auto r = NewString(s.size());
    auto dest = (char *)r->data();
    memcpy(dest, s.data(), s.size());
    #if DELETE_DELAY
        LOG_DEBUG("string: \"", s, "\" - ", (size_t)r);
    #endif
    return r;
}

LString *VM::NewString(string_view s1, string_view s2) {
    auto s = NewString(s1.size() + s2.size());
    auto dest = (char *)s->data();
    memcpy(dest, s1.data(), s1.size());
    memcpy(dest + s1.size(), s2.data(), s2.size());
    return s;
}

LString *VM::ResizeString(LString *s, intp size, int c, bool back) {
    auto ns = NewString(size);
    auto sdest = (char *)ns->data();
    auto cdest = sdest;
    auto remain = size - s->len;
    if (back) sdest += remain;
    else cdest += s->len;
    memcpy(sdest, s->data(), s->len);
    memset(cdest, c, remain);
    s->Dec(*this);
    return ns;
}

// This function is now way less important than it was when the language was still dynamically
// typed. But ok to leave it as-is for "index out of range" and other errors that are still dynamic.
Value VM::Error(string err, const RefObj *a, const RefObj *b) {
    if (trace_tail && trace_output.size()) {
        string s;
        for (size_t i = trace_ring_idx; i < trace_output.size(); i++) s += trace_output[i].str();
        for (size_t i = 0; i < trace_ring_idx; i++) s += trace_output[i].str();
        s += err;
        THROW_OR_ABORT(s);
    }
    ostringstream ss;
    #ifndef VM_COMPILED_CODE_MODE
        DumpFileLine(ip, ss);
        ss << ": ";
    #endif
    ss << "VM error: " << err;
    if (a) { ss << "\n   arg: "; RefToString(*this, ss, a, debugpp); }
    if (b) { ss << "\n   arg: "; RefToString(*this, ss, b, debugpp); }
    while (sp >= 0 && (!stackframes.size() || sp != stackframes.back().spstart)) {
        // Sadly can't print this properly.
        ss << "\n   stack: ";
        to_string_hex(ss, (size_t)VM_TOP().any());
        if (pool.pointer_is_in_allocator(VM_TOP().any())) {
            ss << ", maybe: ";
            RefToString(*this, ss, VM_TOP().ref(), debugpp);
        }
        VM_POP();  // We don't DEC here, as we can't know what type it is.
                // This is ok, as we ignore leaks in case of an error anyway.
    }
    for (;;) {
        if (!stackframes.size()) break;
        int deffun = *(stackframes.back().funstart);
        if (deffun >= 0) {
            ss << "\nin function: " << bcf->functions()->Get(deffun)->name()->string_view();
        } else {
            ss << "\nin block";
        }
        #ifndef VM_COMPILED_CODE_MODE
        ss << " -> ";
        DumpFileLine(ip, ss);
        #endif
        VarCleanup<1>(ss.tellp() < 10000 ? &ss : nullptr, -2 /* clean up temps always */);
    }
    ss << "\nglobals:";
    for (size_t i = 0; i < bcf->specidents()->size(); i++) {
        DumpVar(ss, vars[i], i, true);
    }
    THROW_OR_ABORT(ss.str());
}

void VM::VMAssert(const char *what)  {
    Error(string("VM internal assertion failure: ") + what);
}
void VM::VMAssert(const char *what, const RefObj *a, const RefObj *b)  {
    Error(string("VM internal assertion failure: ") + what, a, b);
}

#if !defined(NDEBUG) && RTT_ENABLED
    #define STRINGIFY(x) #x
    #define TOSTRING(x) STRINGIFY(x)
    #define VMASSERT(test) { if (!(test)) VMAssert(__FILE__ ": " TOSTRING(__LINE__) ": " #test); }
#else
    #define VMASSERT(test) {}
#endif
#if RTT_ENABLED
    #define VMTYPEEQ(val, vt) VMASSERT((val).type == (vt))
#else
    #define VMTYPEEQ(val, vt) { (void)(val); (void)(vt); }
#endif

void VM::DumpVar(ostringstream &ss, const Value &x, size_t idx, bool dumpglobals) {
    auto sid = bcf->specidents()->Get((uint)idx);
    auto id = bcf->idents()->Get(sid->ididx());
    if (id->readonly() || id->global() != dumpglobals) return;
    auto name = id->name()->string_view();
    auto static_type = GetVarTypeInfo((int)idx).t;
    #if RTT_ENABLED
        if (static_type != x.type) return;  // Likely uninitialized.
    #endif
    ss << "\n   " << name << " = ";
    x.ToString(*this, ss, static_type, debugpp);
}

void VM::EvalMulti(const int *mip, const int *call_arg_types, block_t comp_retip) {
    auto definedfunction = *mip++;
    auto nsubf = *mip++;
    auto nargs = *mip++;
    // FIXME: this is a temp fix to keep MM's working with struct args, but
    // should be completely reworked.
    const int MAX_ARGS = 16;
    int idx[MAX_ARGS];
    assert(nargs <= MAX_ARGS);
    auto so = sp;
    for (int j = nargs - 1; j >= 0; j--) {
        auto giveni = (type_elem_t)call_arg_types[j];
        auto &given = GetTypeInfo(giveni);
        if (IsStruct(given.t)) so -= given.len - 1;
        idx[j] = so;
        so--;
    }
    for (int i = 0; i < nsubf; i++) {
        // TODO: rather than going thru all args, only go thru those that have types
        for (int j = 0; j < nargs; j++) {
            auto desiredi = (type_elem_t)*mip++;
            auto &desired = GetTypeInfo(desiredi);
            if (desired.t != V_ANY) {
                auto giveni = (type_elem_t)call_arg_types[j];
                auto &given = GetTypeInfo(giveni);
                //LOG_ERROR(j, " ", desired.Debug(*this, false), " ",
                //                             given.Debug(*this, false));
                // Have to check the actual value, since given may be a supertype.
                // FIXME: this is slow.
                if (IsRef(given.t)) {
                    giveni = stack[idx[j]].ref()->tti;
                }
                if (giveni != desiredi) {
                    mip += nargs - j;  // Includes the code starting point.
                    goto fail;
                }
            } else {
            }
        } {
            call_arg_types += nargs;
            #ifdef VM_COMPILED_CODE_MODE
                InsPtr retip(comp_retip);
                InsPtr fun(next_mm_table[i]);
            #else
                InsPtr retip(call_arg_types - codestart);
                InsPtr fun(*mip);
                (void)comp_retip;
            #endif
            StartStackFrame(retip);
            return FunIntroPre(fun);
        }
        fail:;
    }
    string argtypes;
    for (int j = 0; j < nargs; j++) {
        auto &ti = GetTypeInfo((type_elem_t)call_arg_types[j]);
        Value &v = stack[idx[j]];  // FIXME: structs
        argtypes += ProperTypeName(IsRef(ti.t) && v.ref() ? v.ref()->ti(*this) : ti);
        if (j < nargs - 1) argtypes += ", ";
    }
    Error("the call " + bcf->functions()->Get(definedfunction)->name()->string_view() + "(" +
          argtypes + ") did not match any function variants");
}

void VM::FinalStackVarsCleanup() {
    VMASSERT(sp < 0 && !stackframes.size());
    #ifndef NDEBUG
        LOG_INFO("stack at its highest was: ", maxsp);
    #endif
}

void VM::JumpTo(InsPtr j) {
    #ifdef VM_COMPILED_CODE_MODE
        next_call_target = j.f;
    #else
        ip = j.f + codestart;
    #endif
}

InsPtr VM::GetIP() {
    #ifdef VM_COMPILED_CODE_MODE
        return InsPtr(next_call_target);
    #else
        return InsPtr(ip - codestart);
    #endif
}

template<int is_error> int VM::VarCleanup(ostringstream *error, int towhere) {
    (void)error;
    auto &stf = stackframes.back();
    if constexpr (is_error) VMASSERT(sp == stf.spstart);
    auto fip = stf.funstart;
    fip++;  // function id.
    auto nargs = *fip++;
    auto freevars = fip + nargs;
    fip += nargs;
    auto ndef = *fip++;
    fip += ndef;
    auto defvars = fip;
    auto nkeepvars = *fip++;
    if constexpr (is_error) {
        // Do this first, since values may get deleted below.
        for (int j = 0; j < ndef; j++) {
            auto i = *(defvars - j - 1);
            DumpVar(*error, vars[i], i, false);
        }
        for (int j = 0; j < nargs; j++) {
            auto i = *(freevars - j - 1);
            DumpVar(*error, vars[i], i, false);
        }
    }
    for (int i = 0; i < nkeepvars; i++) VM_POP().LTDECRTNIL(*this);
    auto ownedvars = *fip++;
    for (int i = 0; i < ownedvars; i++) vars[*fip++].LTDECRTNIL(*this);
    while (ndef--) {
        auto i = *--defvars;
        vars[i] = VM_POP();
    }
    while (nargs--) {
        auto i = *--freevars;
        vars[i] = VM_POP();
    }
    JumpTo(stf.retip);
    bool lastunwind = towhere == *stf.funstart;
    stackframes.pop_back();
    if (!lastunwind) {
        // This kills any temps on the stack. If these are refs these should not be
        // owners, since a var or keepvar owns them instead.
        sp = stackframes.size() ? stackframes.back().spstart : -1;
    }
    return lastunwind;
}

// Initializes only 3 fields of the stack frame, FunIntro must be called right after.
void VM::StartStackFrame(InsPtr retip) {
    stackframes.push_back(StackFrame());
    auto &stf = stackframes.back();
    stf.retip = retip;
}

void VM::FunIntroPre(InsPtr fun) {
    JumpTo(fun);
    #ifdef VM_COMPILED_CODE_MODE
        // We don't call FunIntro() here, instead the compiled code for FUNSTART/FUNMULTI actually
        // does that.
    #else
        VMASSERT(*ip == IL_FUNSTART);
        ip++;
        FunIntro();
    #endif
}

// Only valid to be called right after StartStackFrame, with no bytecode in-between.
void VM::FunIntro(VM_OP_ARGS) {
    #ifdef VM_PROFILER
        vm_count_fcalls++;
    #endif
    auto funstart = ip;
    ip++;  // definedfunction
    if (sp > stacksize - STACKMARGIN) {
        // per function call increment should be small
        // FIXME: not safe for untrusted scripts, could simply add lots of locals
        // could record max number of locals? not allow more than N locals?
        if (stacksize >= maxstacksize) Error("stack overflow! (use set_max_stack_size() if needed)");
        auto nstack = new Value[stacksize *= 2];
        t_memcpy(nstack, stack, sp + 1);
        delete[] stack;
        stack = nstack;

        LOG_DEBUG("stack grew to: ", stacksize);
    }
    auto nargs_fun = *ip++;
    for (int i = 0; i < nargs_fun; i++) swap(vars[ip[i]], stack[sp - nargs_fun + i + 1]);
    ip += nargs_fun;
    auto ndef = *ip++;
    for (int i = 0; i < ndef; i++) {
        // for most locals, this just saves an nil, only in recursive cases it has an actual value.
        auto varidx = *ip++;
        VM_PUSH(vars[varidx]);
        vars[varidx] = Value();
    }
    auto nkeepvars = *ip++;
    for (int i = 0; i < nkeepvars; i++) VM_PUSH(Value());
    auto nownedvars = *ip++;
    ip += nownedvars;
    auto &stf = stackframes.back();
    stf.funstart = funstart;
    stf.spstart = sp;
    #ifndef NDEBUG
        if (sp > maxsp) maxsp = sp;
    #endif
}

void VM::FunOut(int towhere, int nrv) {
    sp -= nrv;
    // Have to store these off the stack, since VarCleanup() may cause stack activity if coroutines
    // are destructed.
    t_memcpy(retvalstemp, VM_TOPPTR(), nrv);
    for(;;) {
        if (!stackframes.size()) {
            Error("\"return from " + bcf->functions()->Get(towhere)->name()->string_view() +
                    "\" outside of function");
        }
        if (VarCleanup<0>(nullptr, towhere)) break;
    }
    t_memcpy(VM_TOPPTR(), retvalstemp, nrv);
    sp += nrv;
}

void VM::CoVarCleanup(LCoRoutine *co) {
    // Convenient way to copy everything back onto the stack.
    InsPtr tip(0);
    auto copylen = co->Resume(sp + 1, stack, stackframes, tip, nullptr);
    auto startsp = sp;
    sp += copylen;
    for (int i = co->stackframecopylen - 1; i >= 0 ; i--) {
        auto &stf = stackframes.back();
        sp = stf.spstart;  // Kill any temps on top of the stack.
        // Save the ip, because VarCleanup will jump to it.
        auto bip = GetIP();
        VarCleanup<0>(nullptr, !i ? *stf.funstart : -2);
        JumpTo(bip);
    }
    assert(sp == startsp);
    (void)startsp;
}

void VM::CoNonRec(const int *varip) {
    // probably could be skipped in a "release" mode
    for (auto co = curcoroutine; co; co = co->parent) if (co->varip == varip) {
        // if allowed, inner coro would save vars of outer, and then possibly restore them outside
        // of scope of parent
        Error("cannot create coroutine recursively");
    }
    // this check guarantees all saved stack vars are undef.
}

void VM::CoNew(VM_OP_ARGS_CALL) {
    #ifdef VM_COMPILED_CODE_MODE
        ip++;
        InsPtr returnip(fcont);
    #else
        InsPtr returnip(*ip++);
    #endif
    auto ctidx = (type_elem_t)*ip++;
    CoNonRec(ip);
    curcoroutine = NewCoRoutine(returnip, ip, curcoroutine, ctidx);
    curcoroutine->BackupParentVars(*this, vars);
    int nvars = *ip++;
    ip += nvars;
    // Always have the active coroutine at top of the stack, retaining 1 refcount. This is
    // because it is not guaranteed that there any other references, and we can't have this drop
    // to 0 while active.
    VM_PUSH(Value(curcoroutine));
}

void VM::CoSuspend(InsPtr retip) {
    int newtop = curcoroutine->Suspend(*this, sp + 1, stack, stackframes, retip, curcoroutine);
    JumpTo(retip);
    sp = newtop - 1; // top of stack is now coro value from create or resume
}

void VM::CoClean() {
    // This function is like yield, except happens implicitly when the coroutine returns.
    // It will jump back to the resume (or create) that invoked it.
    for (int i = 1; i <= *curcoroutine->varip; i++) {
        auto &var = vars[curcoroutine->varip[i]];
        var = curcoroutine->stackcopy[i - 1];
    }
    auto co = curcoroutine;
    CoSuspend(InsPtr(0));
    VMASSERT(co->stackcopylen == 1);
    co->active = false;
}

void VM::CoYield(VM_OP_ARGS_CALL) {
    assert(curcoroutine);  // Should not be possible since yield calls are statically checked.
    #ifdef VM_COMPILED_CODE_MODE
        (void)ip;
        InsPtr retip(fcont);
    #else
        InsPtr retip(ip - codestart);
    #endif
    auto ret = VM_POP();
    for (int i = 1; i <= *curcoroutine->varip; i++) {
        auto &var = vars[curcoroutine->varip[i]];
        VM_PUSH(var);
        //var.type = V_NIL;
        var = curcoroutine->stackcopy[i - 1];
    }
    VM_PUSH(ret);  // current value always top of the stack, saved as part of suspended coroutine.
    CoSuspend(retip);
    // Actual top of stack here is coroutine itself, that we placed here with CoResume.
}

void VM::CoResume(LCoRoutine *co) {
    if (co->stackstart >= 0)
        Error("cannot resume running coroutine");
    if (!co->active)
        Error("cannot resume finished coroutine");
    // This will be the return value for the corresponding yield, and holds the ref for gc.
    VM_PUSH(Value(co));
    CoNonRec(co->varip);
    auto rip = GetIP();
    sp += co->Resume(sp + 1, stack, stackframes, rip, curcoroutine);
    JumpTo(rip);
    curcoroutine = co;
    // must be, since those vars got backed up in it before
    VMASSERT(curcoroutine->stackcopymax >=  *curcoroutine->varip);
    curcoroutine->stackcopylen = *curcoroutine->varip;
    //curcoroutine->BackupParentVars(vars);
    VM_POP().LTDECTYPE(*this, GetTypeInfo(curcoroutine->ti(*this).yieldtype).t);    // previous current value
    for (int i = *curcoroutine->varip; i > 0; i--) {
        auto &var = vars[curcoroutine->varip[i]];
        // No INC, since parent is still on the stack and hold ref for us.
        curcoroutine->stackcopy[i - 1] = var;
        var = VM_POP();
    }
    // the builtin call takes care of the return value
}

void VM::EndEval(const Value &ret, ValueType vt) {
    TerminateWorkers();
    ostringstream ss;
    ret.ToString(*this, ss, vt, programprintprefs);
    evalret = ss.str();
    ret.LTDECTYPE(*this, vt);
    assert(sp == -1);
    FinalStackVarsCleanup();
    vml.LogCleanup();
    for (auto s : constant_strings) {
        if (s) s->Dec(*this);
    }
    while (!delete_delay.empty()) {
        auto ro = delete_delay.back();
        delete_delay.pop_back();
        ro->DECDELETENOW(*this);
    }
    DumpLeaks();
    VMASSERT(!curcoroutine);
    #ifdef VM_PROFILER
        LOG_INFO("Profiler statistics:");
        uint64_t total = 0;
        auto fraction = 200;  // Line needs at least 0.5% to be counted.
        vector<uint64_t> lineprofilecounts(bcf->lineinfo()->size());
        for (size_t i = 0; i < codelen; i++) {
            auto li = LookupLine(codestart + i, codestart, bcf); // FIXME: can do faster
            size_t j = li - bcf->lineinfo()->Get(0);
            lineprofilecounts[j] += byteprofilecounts[i];
            total += byteprofilecounts[i];
        }
        struct LineRange { int line, lastline, fileidx; uint64_t count; };
        vector<LineRange> uniques;
        for (uint i = 0; i < bcf->lineinfo()->size(); i++) {
            uint64_t c = lineprofilecounts[i];
            if (c > total / fraction) {
                auto li = bcf->lineinfo()->Get(i);
                uniques.push_back(LineRange{ li->line(), li->line(), li->fileidx(), c });
            }
        }
        std::sort(uniques.begin(), uniques.end(), [&] (const LineRange &a, const LineRange &b) {
            return a.fileidx != b.fileidx ? a.fileidx < b.fileidx : a.line < b.line;
        });
        for (auto it = uniques.begin(); it != uniques.end();)  {
            if (it != uniques.begin()) {
                auto pit = it - 1;
                if (it->fileidx == pit->fileidx &&
                    ((it->line == pit->lastline) ||
                        (it->line == pit->lastline + 1 && pit->lastline++))) {
                    pit->count += it->count;
                    it = uniques.erase(it);
                    continue;
                }
            }
            ++it;
        }
        for (auto &u : uniques) {
            LOG_INFO(bcf->filenames()->Get(u.fileidx)->string_view(), "(", u.line,
                   u.lastline != u.line ? "-" + to_string(u.lastline) : "",
                   "): ", u.count * 100.0f / total, " %");
        }
        if (vm_count_fcalls)  // remove trivial VM executions from output
            LOG_INFO("ins ", vm_count_ins, ", fcall ", vm_count_fcalls, ", bcall ",
                                vm_count_bcalls, ", decref ", vm_count_decref);
    #endif
    #if MEASURE_INSTRUCTION_COMBINATIONS
        struct trip { size_t freq; int opc1, opc2; };
        vector<trip> combinations;
        for (auto &p : instruction_combinations)
            combinations.push_back({ p.second, p.first.first, p.first.second });
        sort(combinations.begin(), combinations.end(), [](const trip &a, const trip &b) {
            return a.freq > b.freq;
        });
        combinations.resize(50);
        for (auto &c : combinations) {
            LOG_PROGRAM("instruction ", ILNames()[c.opc1], " -> ", ILNames()[c.opc2], " (",
                        c.freq, "x)");
        }
        instruction_combinations.clear();
    #endif
    #ifndef VM_ERROR_RET_EXPERIMENT
    THROW_OR_ABORT(string("end-eval"));
    #endif
}

void VM::EvalProgram() {
    // Keep exception handling code in seperate function from hot loop in EvalProgramInner()
    // just in case it affects the compiler.
    #ifdef USE_EXCEPTION_HANDLING
    try
    #endif
    {
        EvalProgramInner();
    }
    #ifdef USE_EXCEPTION_HANDLING
    catch (string &s) {
        if (s != "end-eval") THROW_OR_ABORT(s);
    }
    #endif
}

void VM::EvalProgramInner() {
    for (;;) {
        #ifdef VM_COMPILED_CODE_MODE
            #if VM_DISPATCH_METHOD == VM_DISPATCH_TRAMPOLINE
                compiled_code_ip = ((block_t)compiled_code_ip)(*this);
            #elif VM_DISPATCH_METHOD == VM_DISPATCH_SWITCH_GOTO
                ((block_base_t)compiled_code_ip)();
                assert(false);  // Should not return here.
            #endif
        #else
            #ifndef NDEBUG
                if (trace) {
                    size_t trace_size = trace_tail ? 50 : 1;
                    if (trace_output.size() < trace_size) trace_output.resize(trace_size);
                    if (trace_ring_idx == trace_size) trace_ring_idx = 0;
                    auto &ss = trace_output[trace_ring_idx++];
                    ss.str(string());
                    DisAsmIns(nfr, ss, ip, codestart, typetable, bcf);
                    ss << " [" << (sp + 1) << "] -";
                    #if RTT_ENABLED
                    #if DELETE_DELAY
                        ss << ' ' << (size_t)VM_TOP().any();
                    #endif
                    for (int i = 0; i < 3 && sp - i >= 0; i++) {
                        auto x = VM_TOPM(i);
                        ss << ' ';
                        x.ToString(*this, ss, x.type, debugpp);
                    }
                    #endif
                    if (trace_tail) {
                        ss << '\n';
                    } else {
                        LOG_DEBUG(ss.str());
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
            #if MEASURE_INSTRUCTION_COMBINATIONS
                instruction_combinations[{ last_instruction_opc, op }]++;
                last_instruction_opc = op;
            #endif
            #ifndef NDEBUG
                if (op < 0 || op >= IL_MAX_OPS)
                    Error(cat("bytecode format problem: ", op));
            #endif
            #ifndef VM_INS_SWITCH
                #ifdef VM_ERROR_RET_EXPERIMENT
                    bool terminate =
                #endif
                ((*this).*(f_ins_pointers[op]))();
                #ifdef VM_ERROR_RET_EXPERIMENT
                    if (terminate) return;
                #endif
            #endif
        #endif

#ifndef VM_INS_SWITCH
    // For loop and function end here.
    }
}
    #define VM_DEF_INS(N) VM_INS_RET VM::F_##N(VM_OP_ARGS)
    #define VM_DEF_CAL(N) VM_INS_RET VM::F_##N(VM_OP_ARGS_CALL)
    #define VM_DEF_JMP(N) VM_JMP_RET VM::F_##N()
#else
    // We start a switch here that contains all instructions below.
        switch (op) {
    #define VM_DEF_INS(N) case IL_##N:
    #define VM_DEF_CAL(N) case IL_##N:
    #define VM_DEF_JMP(N) case IL_##N:
#endif

VM_DEF_INS(PUSHINT) { VM_PUSH(Value(*ip++)); VM_RET; }
VM_DEF_INS(PUSHFLT) { VM_PUSH(Value(*(float *)ip)); ip++; VM_RET; }
VM_DEF_INS(PUSHNIL) { VM_PUSH(Value()); VM_RET; }

VM_DEF_INS(PUSHINT64) {
    #if !VALUE_MODEL_64
        Error("Code containing 64-bit constants cannot run on a 32-bit build.");
    #endif
    auto v = Read64FromIp(ip);
    VM_PUSH(Value(v));
    VM_RET;
}

VM_DEF_INS(PUSHFLT64) {
    int2float64 i2f;
    i2f.i = Read64FromIp(ip);
    VM_PUSH(Value(i2f.f));
    VM_RET;
}

VM_DEF_CAL(PUSHFUN) {
    #ifdef VM_COMPILED_CODE_MODE
        ip++;
    #else
        int start = *ip++;
        auto fcont = start;
    #endif
    VM_PUSH(Value(InsPtr(fcont)));
    VM_RET;
}

VM_DEF_INS(PUSHSTR) {
    // FIXME: have a way that constant strings can stay in the bytecode,
    // or at least preallocate them all
    int i = *ip++;
    auto &s = constant_strings[i];
    if (!s) {
        auto fb_s = bcf->stringtable()->Get(i);
        s = NewString(fb_s->string_view());
    }
    #if STRING_CONSTANTS_KEEP
        s->Inc();
    #endif
    VM_PUSH(Value(s));
    VM_RET;
}

VM_DEF_INS(INCREF) {
    auto off = *ip++;
    VM_TOPM(off).LTINCRTNIL();
    VM_RET;
}

VM_DEF_INS(KEEPREF) {
    auto off = *ip++;
    auto ki = *ip++;
    VM_TOPM(ki).LTDECRTNIL(*this);  // FIXME: this is only here for inlined for bodies!
    VM_TOPM(ki) = VM_TOPM(off);
    VM_RET;
}

VM_DEF_CAL(CALL) {
    #ifdef VM_COMPILED_CODE_MODE
        ip++;
        block_t fun = 0;  // Dynamic calls need this set, but for CALL it is ignored.
    #else
        auto fun = *ip++;
        auto fcont = ip - codestart;
    #endif
    StartStackFrame(InsPtr(fcont));
    FunIntroPre(InsPtr(fun));
    VM_RET;
}

VM_DEF_CAL(CALLMULTI) {
    #ifdef VM_COMPILED_CODE_MODE
        next_mm_call = ip;
        next_call_target = fcont;  // Used just to transfer value here.
    #else
        auto fun = *ip++;
        auto mip = codestart + fun;
        VMASSERT(*mip == IL_FUNMULTI);
        mip++;
        EvalMulti(mip, ip, 0);
    #endif
    VM_RET;
}

VM_DEF_INS(FUNMULTI) {
    #ifdef VM_COMPILED_CODE_MODE
        auto cip = next_mm_call;
        cip++;  // bytecode start of FUNMULTI.
        EvalMulti(ip, cip, next_call_target);
    #else
        VMASSERT(false);
    #endif
    VM_RET;
}

VM_DEF_CAL(CALLVCOND) {
    // FIXME: don't need to check for function value again below if false
    if (!VM_TOP().True()) {
        VM_POP();
        #ifdef VM_COMPILED_CODE_MODE
            next_call_target = 0;
        #endif
    } else {
        #ifdef VM_INS_SWITCH
            goto callv;
        #else
            F_CALLV(VM_OP_PASSTHRU);
        #endif
    }
    VM_RET;
}

VM_DEF_CAL(CALLV) {
    #ifdef VM_INS_SWITCH
    callv:
    #endif
    {
        Value fun = VM_POP();
        VMTYPEEQ(fun, V_FUNCTION);
        #ifndef VM_COMPILED_CODE_MODE
            auto fcont = ip - codestart;
        #endif
        StartStackFrame(InsPtr(fcont));
        FunIntroPre(fun.ip());
        VM_RET;
    }
}

VM_DEF_INS(FUNSTART) {
    #ifdef VM_COMPILED_CODE_MODE
        FunIntro(ip);
    #else
        VMASSERT(false);
    #endif
    VM_RET;
}

VM_DEF_INS(FUNEND) {
    assert(false);
    VM_RET;
}

VM_DEF_INS(RETURN) {
    int df = *ip++;
    int nrv = *ip++;
    FunOut(df, nrv);
    VM_RET;
}

VM_DEF_INS(ENDSTATEMENT) {
    assert(sp == stackframes.back().spstart);
    VM_RET;
}

VM_DEF_INS(EXIT) {
    int tidx = *ip++;
    if (tidx >= 0) EndEval(VM_POP(), GetTypeInfo((type_elem_t)tidx).t);
    else EndEval(Value(), V_NIL);
    VM_TERMINATE;
}

VM_DEF_INS(CONT1) {
    auto nf = nfr.nfuns[*ip++];
    nf->cont1(*this);
    VM_RET;
}

VM_JMP_RET VM::ForLoop(intp len) {
    #ifndef VM_COMPILED_CODE_MODE
        auto cont = *ip++;
    #endif
    auto &i = VM_TOPM(1);
    assert(i.type == V_INT);
    i.setival(i.ival() + 1);
    if (i.ival() < len) {
        #ifdef VM_COMPILED_CODE_MODE
            return true;
        #else
            ip = cont + codestart;
        #endif
    } else {
        (void)VM_POP(); /* iter */
        (void)VM_POP(); /* i */
        #ifdef VM_COMPILED_CODE_MODE
            return false;
        #endif
    }
}

#define FORELEM(L) \
    auto &iter = VM_TOP(); \
    auto i = VM_TOPM(1).ival(); \
    assert(i < L); \

VM_DEF_JMP(IFOR) { return ForLoop(VM_TOP().ival()); VM_RET; }
VM_DEF_JMP(VFOR) { return ForLoop(VM_TOP().vval()->len); VM_RET; }
VM_DEF_JMP(NFOR) { return ForLoop(VM_TOP().oval()->Len(*this)); VM_RET; }
VM_DEF_JMP(SFOR) { return ForLoop(VM_TOP().sval()->len); VM_RET; }

VM_DEF_INS(IFORELEM)    { FORELEM(iter.ival()); (void)iter; VM_PUSH(i); VM_RET; }
VM_DEF_INS(VFORELEM)    { FORELEM(iter.vval()->len); iter.vval()->AtVW(*this, i); VM_RET; }
VM_DEF_INS(VFORELEMREF) { FORELEM(iter.vval()->len); auto el = iter.vval()->At(i); el.LTINCRTNIL(); VM_PUSH(el); VM_RET; }
VM_DEF_INS(NFORELEM)    { FORELEM(iter.oval()->Len(*this)); VM_PUSH(iter.oval()->AtS(i)); VM_RET; }
VM_DEF_INS(SFORELEM)    { FORELEM(iter.sval()->len); VM_PUSH(Value((int)((uchar *)iter.sval()->data())[i])); VM_RET; }

VM_DEF_INS(FORLOOPI) {
    auto &i = VM_TOPM(1);  // This relies on for being inlined, otherwise it would be 2.
    assert(i.type == V_INT);
    VM_PUSH(i);
    VM_RET;
}

VM_DEF_INS(BCALLRETV) {
    BCallProf();
    auto nf = nfr.nfuns[*ip++];
    nf->fun.fV(*this);
    VM_RET;
}
VM_DEF_INS(BCALLREFV) {
    BCallProf();
    auto nf = nfr.nfuns[*ip++];
    nf->fun.fV(*this);
    // This can only pop a single value, not called for structs.
    VM_POP().LTDECRTNIL(*this);
    VM_RET;
}
VM_DEF_INS(BCALLUNBV) {
    BCallProf();
    auto nf = nfr.nfuns[*ip++];
    nf->fun.fV(*this);
    // This can only pop a single value, not called for structs.
    VM_POP();
    VM_RET;
}

#define BCALLOPH(PRE,N,DECLS,ARGS,RETOP) VM_DEF_INS(BCALL##PRE##N) { \
    BCallProf(); \
    auto nf = nfr.nfuns[*ip++]; \
    DECLS; \
    Value v = nf->fun.f##N ARGS; \
    RETOP; \
    VM_RET; \
}

#define BCALLOP(N,DECLS,ARGS) \
    BCALLOPH(RET,N,DECLS,ARGS,VM_PUSH(v);BCallRetCheck(nf)) \
    BCALLOPH(REF,N,DECLS,ARGS,v.LTDECRTNIL(*this)) \
    BCALLOPH(UNB,N,DECLS,ARGS,(void)v)

BCALLOP(0, {}, (*this));
BCALLOP(1, auto a0 = VM_POP(), (*this, a0));
BCALLOP(2, auto a1 = VM_POP();auto a0 = VM_POP(), (*this, a0, a1));
BCALLOP(3, auto a2 = VM_POP();auto a1 = VM_POP();auto a0 = VM_POP(), (*this, a0, a1, a2));
BCALLOP(4, auto a3 = VM_POP();auto a2 = VM_POP();auto a1 = VM_POP();auto a0 = VM_POP(), (*this, a0, a1, a2, a3));
BCALLOP(5, auto a4 = VM_POP();auto a3 = VM_POP();auto a2 = VM_POP();auto a1 = VM_POP();auto a0 = VM_POP(), (*this, a0, a1, a2, a3, a4));
BCALLOP(6, auto a5 = VM_POP();auto a4 = VM_POP();auto a3 = VM_POP();auto a2 = VM_POP();auto a1 = VM_POP();auto a0 = VM_POP(), (*this, a0, a1, a2, a3, a4, a5));
BCALLOP(7, auto a6 = VM_POP();auto a5 = VM_POP();auto a4 = VM_POP();auto a3 = VM_POP();auto a2 = VM_POP();auto a1 = VM_POP();auto a0 = VM_POP(), (*this, a0, a1, a2, a3, a4, a5, a6));

VM_DEF_INS(NEWVEC) {
    auto type = (type_elem_t)*ip++;
    auto len = *ip++;
    auto vec = NewVec(len, len, type);
    if (len) vec->Init(*this, VM_TOPPTR() - len * vec->width, false);
    VM_POPN(len * (int)vec->width);
    VM_PUSH(Value(vec));
    VM_RET;
}

VM_DEF_INS(NEWOBJECT) {
    auto type = (type_elem_t)*ip++;
    auto len = GetTypeInfo(type).len;
    auto vec = NewObject(len, type);
    if (len) vec->Init(*this, VM_TOPPTR() - len, len, false);
    VM_POPN(len);
    VM_PUSH(Value(vec));
    VM_RET;
}

VM_DEF_INS(POP)    { VM_POP(); VM_RET; }
VM_DEF_INS(POPREF) { auto x = VM_POP(); x.LTDECRTNIL(*this); VM_RET; }
VM_DEF_INS(POPV)   { VM_POPN(*ip++); VM_RET; }

VM_DEF_INS(DUP)    { auto x = VM_TOP(); VM_PUSH(x); VM_RET; }

#define GETARGS() Value b = VM_POP(); Value a = VM_POP()
#define TYPEOP(op, extras, field, errstat) Value res; errstat; \
    if (extras & 1 && b.field == 0) Div0(); res = a.field op b.field;

#define _IOP(op, extras) \
    TYPEOP(op, extras, ival(), assert(a.type == V_INT && b.type == V_INT))
#define _FOP(op, extras) \
    TYPEOP(op, extras, fval(), assert(a.type == V_FLOAT && b.type == V_FLOAT))

#define _GETA() VM_TOPPTR() - len
#define _VOP(op, extras, V_T, field, withscalar, geta) { \
    auto len = VM_POP().intval(); \
    if (withscalar) { \
        auto b = VM_POP(); \
        VMTYPEEQ(b, V_T) \
        auto veca = geta; \
        for (int j = 0; j < len; j++) { \
            auto &a = veca[j]; \
            VMTYPEEQ(a, V_T) \
            auto bv = b.field(); \
            if (extras&1 && bv == 0) Div0(); \
            a = Value(a.field() op bv); \
        } \
    } else { \
        VM_POPN(len); \
        auto vecb = VM_TOPPTR(); \
        auto veca = geta; \
        for (int j = 0; j < len; j++) { \
            auto b = vecb[j]; \
            VMTYPEEQ(b, V_T) \
            auto &a = veca[j]; \
            VMTYPEEQ(a, V_T) \
            auto bv = b.field(); \
            if (extras & 1 && bv == 0) Div0(); \
            a = Value(a.field() op bv); \
        } \
    } \
}
#define STCOMPEN(op, init, andor) { \
    auto len = VM_POP().intval(); \
    VM_POPN(len); \
    auto vecb = VM_TOPPTR(); \
    VM_POPN(len); \
    auto veca = VM_TOPPTR(); \
    auto all = init; \
    for (int j = 0; j < len; j++) { \
        all = all andor veca[j].any() op vecb[j].any(); \
    } \
    VM_PUSH(all); \
    VM_RET; \
}

#define _IVOP(op, extras, withscalar, geta) _VOP(op, extras, V_INT, ival, withscalar, geta)
#define _FVOP(op, extras, withscalar, geta) _VOP(op, extras, V_FLOAT, fval, withscalar, geta)

#define _SOP(op) Value res = *a.sval() op *b.sval()
#define _SCAT() Value res = NewString(a.sval()->strv(), b.sval()->strv())

#define ACOMPEN(op)        { GETARGS(); Value res = a.any() op b.any();  VM_PUSH(res); VM_RET; }
#define IOP(op, extras)    { GETARGS(); _IOP(op, extras);                VM_PUSH(res); VM_RET; }
#define FOP(op, extras)    { GETARGS(); _FOP(op, extras);                VM_PUSH(res); VM_RET; }

#define IVVOP(op, extras)  { _IVOP(op, extras, false, _GETA()); VM_RET; }
#define FVVOP(op, extras)  { _FVOP(op, extras, false, _GETA()); VM_RET; }
#define IVSOP(op, extras)  { _IVOP(op, extras, true, _GETA());  VM_RET; }
#define FVSOP(op, extras)  { _FVOP(op, extras, true, _GETA());  VM_RET; }

#define SOP(op)            { GETARGS(); _SOP(op);                        VM_PUSH(res); VM_RET; }
#define SCAT()             { GETARGS(); _SCAT();                         VM_PUSH(res); VM_RET; }

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

VM_DEF_INS(IVVADD) { IVVOP(+,  0);  }
VM_DEF_INS(IVVSUB) { IVVOP(-,  0);  }
VM_DEF_INS(IVVMUL) { IVVOP(*,  0);  }
VM_DEF_INS(IVVDIV) { IVVOP(/,  1);  }
VM_DEF_INS(IVVMOD) { VMASSERT(0); VM_RET; }
VM_DEF_INS(IVVLT)  { IVVOP(<,  0);  }
VM_DEF_INS(IVVGT)  { IVVOP(>,  0);  }
VM_DEF_INS(IVVLE)  { IVVOP(<=, 0);  }
VM_DEF_INS(IVVGE)  { IVVOP(>=, 0);  }
VM_DEF_INS(FVVADD) { FVVOP(+,  0);  }
VM_DEF_INS(FVVSUB) { FVVOP(-,  0);  }
VM_DEF_INS(FVVMUL) { FVVOP(*,  0);  }
VM_DEF_INS(FVVDIV) { FVVOP(/,  1);  }
VM_DEF_INS(FVVMOD) { VMASSERT(0); VM_RET; }
VM_DEF_INS(FVVLT)  { FVVOP(<,  0); }
VM_DEF_INS(FVVGT)  { FVVOP(>,  0); }
VM_DEF_INS(FVVLE)  { FVVOP(<=, 0); }
VM_DEF_INS(FVVGE)  { FVVOP(>=, 0); }

VM_DEF_INS(IVSADD) { IVSOP(+,  0);  }
VM_DEF_INS(IVSSUB) { IVSOP(-,  0);  }
VM_DEF_INS(IVSMUL) { IVSOP(*,  0);  }
VM_DEF_INS(IVSDIV) { IVSOP(/,  1);  }
VM_DEF_INS(IVSMOD) { VMASSERT(0); VM_RET; }
VM_DEF_INS(IVSLT)  { IVSOP(<,  0);  }
VM_DEF_INS(IVSGT)  { IVSOP(>,  0);  }
VM_DEF_INS(IVSLE)  { IVSOP(<=, 0);  }
VM_DEF_INS(IVSGE)  { IVSOP(>=, 0);  }
VM_DEF_INS(FVSADD) { FVSOP(+,  0);  }
VM_DEF_INS(FVSSUB) { FVSOP(-,  0);  }
VM_DEF_INS(FVSMUL) { FVSOP(*,  0);  }
VM_DEF_INS(FVSDIV) { FVSOP(/,  1);  }
VM_DEF_INS(FVSMOD) { VMASSERT(0); VM_RET; }
VM_DEF_INS(FVSLT)  { FVSOP(<,  0); }
VM_DEF_INS(FVSGT)  { FVSOP(>,  0); }
VM_DEF_INS(FVSLE)  { FVSOP(<=, 0); }
VM_DEF_INS(FVSGE)  { FVSOP(>=, 0); }

VM_DEF_INS(AEQ)  { ACOMPEN(==); }
VM_DEF_INS(ANE)  { ACOMPEN(!=); }
VM_DEF_INS(STEQ) { STCOMPEN(==, true, &&); }
VM_DEF_INS(STNE) { STCOMPEN(!=, false, ||); }

VM_DEF_INS(IADD) { IOP(+,  0); }
VM_DEF_INS(ISUB) { IOP(-,  0); }
VM_DEF_INS(IMUL) { IOP(*,  0); }
VM_DEF_INS(IDIV) { IOP(/ , 1); }
VM_DEF_INS(IMOD) { IOP(%,  1); }
VM_DEF_INS(ILT)  { IOP(<,  0); }
VM_DEF_INS(IGT)  { IOP(>,  0); }
VM_DEF_INS(ILE)  { IOP(<=, 0); }
VM_DEF_INS(IGE)  { IOP(>=, 0); }
VM_DEF_INS(IEQ)  { IOP(==, 0); }
VM_DEF_INS(INE)  { IOP(!=, 0); }

VM_DEF_INS(FADD) { FOP(+,  0); }
VM_DEF_INS(FSUB) { FOP(-,  0); }
VM_DEF_INS(FMUL) { FOP(*,  0); }
VM_DEF_INS(FDIV) { FOP(/,  1); }
VM_DEF_INS(FMOD) { VMASSERT(0); VM_RET; }
VM_DEF_INS(FLT)  { FOP(<,  0); }
VM_DEF_INS(FGT)  { FOP(>,  0); }
VM_DEF_INS(FLE)  { FOP(<=, 0); }
VM_DEF_INS(FGE)  { FOP(>=, 0); }
VM_DEF_INS(FEQ)  { FOP(==, 0); }
VM_DEF_INS(FNE)  { FOP(!=, 0); }

VM_DEF_INS(SADD) { SCAT();  }
VM_DEF_INS(SSUB) { VMASSERT(0); VM_RET; }
VM_DEF_INS(SMUL) { VMASSERT(0); VM_RET; }
VM_DEF_INS(SDIV) { VMASSERT(0); VM_RET; }
VM_DEF_INS(SMOD) { VMASSERT(0); VM_RET; }
VM_DEF_INS(SLT)  { SOP(<);  }
VM_DEF_INS(SGT)  { SOP(>);  }
VM_DEF_INS(SLE)  { SOP(<=); }
VM_DEF_INS(SGE)  { SOP(>=); }
VM_DEF_INS(SEQ)  { SOP(==); }
VM_DEF_INS(SNE)  { SOP(!=); }

VM_DEF_INS(IUMINUS) { Value a = VM_POP(); VM_PUSH(Value(-a.ival())); VM_RET; }
VM_DEF_INS(FUMINUS) { Value a = VM_POP(); VM_PUSH(Value(-a.fval())); VM_RET; }

VM_DEF_INS(IVUMINUS) {
    auto len = VM_POP().intval();
    auto vec = VM_TOPPTR() - len;
    for (int i = 0; i < len; i++) {
        auto &a = vec[i];
        VMTYPEEQ(a, V_INT);
        a = -a.ival();
    }
    VM_RET;
}
VM_DEF_INS(FVUMINUS) {
    auto len = VM_POP().intval();
    auto vec = VM_TOPPTR() - len;
    for (int i = 0; i < len; i++) {
        auto &a = vec[i];
        VMTYPEEQ(a, V_FLOAT);
        a = -a.fval();
    }
    VM_RET;
}

VM_DEF_INS(LOGNOT) {
    Value a = VM_POP();
    VM_PUSH(!a.True());
    VM_RET;
}
VM_DEF_INS(LOGNOTREF) {
    Value a = VM_POP();
    bool b = a.True();
    VM_PUSH(!b);
    VM_RET;
}

#define BITOP(op) { GETARGS(); VM_PUSH(a.ival() op b.ival()); VM_RET; }
VM_DEF_INS(BINAND) { BITOP(&);  }
VM_DEF_INS(BINOR)  { BITOP(|);  }
VM_DEF_INS(XOR)    { BITOP(^);  }
VM_DEF_INS(ASL)    { BITOP(<<); }
VM_DEF_INS(ASR)    { BITOP(>>); }
VM_DEF_INS(NEG)    { auto a = VM_POP(); VM_PUSH(~a.ival()); VM_RET; }

VM_DEF_INS(I2F) {
    Value a = VM_POP();
    VMTYPEEQ(a, V_INT);
    VM_PUSH((float)a.ival());
    VM_RET;
}

VM_DEF_INS(A2S) {
    Value a = VM_POP();
    assert(IsRefNil(a.type));
    VM_PUSH(ToString(a, a.ref() ? a.ref()->ti(*this).t : V_NIL));
    VM_RET;
}

VM_DEF_INS(ST2S) {
    auto &ti = GetTypeInfo((type_elem_t)*ip++);
    VM_POPN(ti.len);
    VM_PUSH(StructToString(VM_TOPPTR(), ti));
    VM_RET;
}

VM_DEF_INS(I2S) {
    Value i = VM_POP();
    VMTYPEEQ(i, V_INT);
    VM_PUSH(ToString(i, V_INT));
    VM_RET;
}

VM_DEF_INS(F2S) {
    Value f = VM_POP();
    VMTYPEEQ(f, V_FLOAT);
    VM_PUSH(ToString(f, V_FLOAT));
    VM_RET;
}

VM_DEF_INS(E2B) {
    Value a = VM_POP();
    VM_PUSH(a.True());
    VM_RET;
}

VM_DEF_INS(E2BREF) {
    Value a = VM_POP();
    VM_PUSH(a.True());
    VM_RET;
}

VM_DEF_INS(PUSHVAR) {
    VM_PUSH(vars[*ip++]);
    VM_RET;
}

VM_DEF_INS(PUSHVARV) {
    auto l = VM_POP().intval();
    t_memcpy(VM_TOPPTR(), &vars[*ip++], l);
    VM_PUSHN(l);
    VM_RET;
}

VM_DEF_INS(PUSHFLD) {
    auto i = *ip++;
    Value r = VM_POP();
    VMASSERT(r.ref());
    assert(i < r.oval()->Len(*this));
    VM_PUSH(r.oval()->AtS(i));
    VM_RET;
}
VM_DEF_INS(PUSHFLDMREF) {
    auto i = *ip++;
    Value r = VM_POP();
    if (!r.ref()) {
        VM_PUSH(r);
    } else {
        assert(i < r.oval()->Len(*this));
        VM_PUSH(r.oval()->AtS(i));
    }
    VM_RET;
}
VM_DEF_INS(PUSHFLD2V) {
    auto i = *ip++;
    auto l = VM_POP().intval();
    Value r = VM_POP();
    VMASSERT(r.ref());
    assert(i + l <= r.oval()->Len(*this));
    t_memcpy(VM_TOPPTR(), &r.oval()->AtS(i), l);
    VM_PUSHN(l);
    VM_RET;
}
VM_DEF_INS(PUSHFLDV) {
    auto i = *ip++;
    auto l = VM_POP().intval();
    VM_POPN(l);
    auto val = *(VM_TOPPTR() + i);
    VM_PUSH(val);
    VM_RET;
}
VM_DEF_INS(PUSHFLDV2V) {
    auto i = *ip++;
    auto rl = VM_POP().intval();
    auto l = VM_POP().intval();
    VM_POPN(l);
    t_memmove(VM_TOPPTR(), VM_TOPPTR() + i, rl);
    VM_PUSHN(rl);
    VM_RET;
}

VM_DEF_INS(VPUSHIDXI)    { PushDerefIdxVector(VM_POP().ival()); VM_RET; }
VM_DEF_INS(VPUSHIDXV)    { PushDerefIdxVector(GrabIndex()); VM_RET; }
VM_DEF_INS(NPUSHIDXI)    { PushDerefIdxStruct(VM_POP().ival()); VM_RET; }
VM_DEF_INS(SPUSHIDXI)    { PushDerefIdxString(VM_POP().ival()); VM_RET; }

VM_DEF_INS(PUSHLOC) {
    int i = *ip++;
    auto coro = VM_POP().cval();
    VM_PUSH(coro->GetVar(*this, i));
    VM_RET;
}

VM_DEF_INS(PUSHLOCV) {
    int i = *ip++;
    auto l = VM_POP().intval();
    auto coro = VM_POP().cval();
    t_memcpy(VM_TOPPTR(), &coro->GetVar(*this, i), l);
    VM_PUSHN(l);
    VM_RET;
}

#ifdef VM_COMPILED_CODE_MODE
    #define GJUMP(N, V, C, P) VM_JMP_RET VM::F_##N() \
        { V; if (C) { P; return true; } else { return false; } }
#else
    #define GJUMP(N, V, C, P) VM_DEF_JMP(N) \
        { V; auto nip = *ip++; if (C) { ip = codestart + nip; P; } VM_RET; }
#endif

GJUMP(JUMP          ,               , true     ,              )
GJUMP(JUMPFAIL      , auto x = VM_POP(), !x.True(),              )
GJUMP(JUMPFAILR     , auto x = VM_POP(), !x.True(), VM_PUSH(x)      )
GJUMP(JUMPFAILN     , auto x = VM_POP(), !x.True(), VM_PUSH(Value()))
GJUMP(JUMPNOFAIL    , auto x = VM_POP(),  x.True(),              )
GJUMP(JUMPNOFAILR   , auto x = VM_POP(),  x.True(), VM_PUSH(x)      )

VM_DEF_INS(ISTYPE) {
    auto to = (type_elem_t)*ip++;
    auto v = VM_POP();
    // Optimizer guarantees we don't have to deal with scalars.
    if (v.refnil()) VM_PUSH(v.ref()->tti == to);
    else VM_PUSH(GetTypeInfo(to).t == V_NIL);  // FIXME: can replace by fixed type_elem_t ?
    VM_RET;
}

VM_DEF_CAL(YIELD) { CoYield(VM_OP_PASSTHRU); VM_RET; }

// This value never gets used anywhere, just a placeholder.
VM_DEF_INS(COCL) { VM_PUSH(Value(0, V_YIELD)); VM_RET; }

VM_DEF_CAL(CORO) { CoNew(VM_OP_PASSTHRU); VM_RET; }

VM_DEF_INS(COEND) { CoClean(); VM_RET; }

VM_DEF_INS(LOGREAD) {
    auto val = VM_POP();
    VM_PUSH(vml.LogGet(val, *ip++));
    VM_RET;
}

VM_DEF_INS(LOGWRITE) {
    auto vidx = *ip++;
    auto lidx = *ip++;
    vml.LogWrite(vars[vidx], lidx);
    VM_RET;
}

VM_DEF_INS(ABORT) {
    Error("VM internal error: abort");
    VM_RET;
}

#ifdef VM_INS_SWITCH
        }  // switch
    }  // for
}  // EvalProgramInner()
#endif

void VM::IDXErr(intp i, intp n, const RefObj *v) {
    Error(cat("index ", i, " out of range ", n), v);
}
#define RANGECHECK(I, BOUND, VEC) if ((uintp)I >= (uintp)BOUND) IDXErr(I, BOUND, VEC);

void VM::PushDerefIdxVector(intp i) {
    Value r = VM_POP();
    VMASSERT(r.ref());
    auto v = r.vval();
    RANGECHECK(i, v->len, v);
    v->AtVW(*this, i);
}

void VM::PushDerefIdxStruct(intp i) {
    auto l = VM_POP().intval();
    VM_POPN(l);
    auto val = *(VM_TOPPTR() + i);
    VM_PUSH(val);
}

void VM::PushDerefIdxString(intp i) {
    Value r = VM_POP();
    VMASSERT(r.ref());
    // Allow access of the terminating 0-byte.
    RANGECHECK(i, r.sval()->len + 1, r.sval());
    VM_PUSH(Value((int)((uchar *)r.sval()->data())[i]));
}

Value &VM::GetFieldLVal(intp i) {
    Value vec = VM_POP();
    #ifdef _DEBUG
        RANGECHECK(i, vec.oval()->Len(*this), vec.oval());
    #endif
    return vec.oval()->AtS(i);
}

Value &VM::GetFieldILVal(intp i) {
    Value vec = VM_POP();
    RANGECHECK(i, vec.oval()->Len(*this), vec.oval());
    return vec.oval()->AtS(i);
}

Value &VM::GetVecLVal(intp i) {
    Value vec = VM_POP();
    auto v = vec.vval();
    RANGECHECK(i, v->len, v);
    return *v->AtSt(i);
}

Value &VM::GetLocLVal(int i) {
    Value coro = VM_POP();
    VMTYPEEQ(coro, V_COROUTINE);
    return coro.cval()->GetVar(*this, i);
}

#undef LVAL

#define LVAL(N) VM_DEF_INS(VAR_##N) { LV_##N(vars[*ip++]); VM_RET; }
    LVALOPNAMES
#undef LVAL

#define LVAL(N) VM_DEF_INS(FLD_##N) { LV_##N(GetFieldLVal(*ip++)); VM_RET; }
    LVALOPNAMES
#undef LVAL

#define LVAL(N) VM_DEF_INS(LOC_##N) { LV_##N(GetLocLVal(*ip++)); VM_RET; }
    LVALOPNAMES
#undef LVAL

#define LVAL(N) VM_DEF_INS(IDXVI_##N) { LV_##N(GetVecLVal(VM_POP().ival())); VM_RET; }
    LVALOPNAMES
#undef LVAL

#define LVAL(N) VM_DEF_INS(IDXVV_##N) { LV_##N(GetVecLVal(GrabIndex())); VM_RET; }
    LVALOPNAMES
#undef LVAL

#define LVAL(N) VM_DEF_INS(IDXNI_##N) { LV_##N(GetFieldILVal(VM_POP().ival())); VM_RET; }
    LVALOPNAMES
#undef LVAL

#define LVALCASES(N, B) void VM::LV_##N(Value &a) { Value b = VM_POP(); B; }
#define LVALCASER(N, B) void VM::LV_##N(Value &fa) { B; }
#define LVALCASESTR(N, B, B2) void VM::LV_##N(Value &a) { Value b = VM_POP(); B; a.LTDECRTNIL(*this); B2; }

LVALCASER(IVVADD , _IVOP(+, 0, false, &fa))
LVALCASER(IVVADDR, _IVOP(+, 0, false, &fa))
LVALCASER(IVVSUB , _IVOP(-, 0, false, &fa))
LVALCASER(IVVSUBR, _IVOP(-, 0, false, &fa))
LVALCASER(IVVMUL , _IVOP(*, 0, false, &fa))
LVALCASER(IVVMULR, _IVOP(*, 0, false, &fa))
LVALCASER(IVVDIV , _IVOP(/, 1, false, &fa))
LVALCASER(IVVDIVR, _IVOP(/, 1, false, &fa))
LVALCASER(IVVMOD , VMASSERT(0); (void)fa)
LVALCASER(IVVMODR, VMASSERT(0); (void)fa)

LVALCASER(FVVADD , _FVOP(+, 0, false, &fa))
LVALCASER(FVVADDR, _FVOP(+, 0, false, &fa))
LVALCASER(FVVSUB , _FVOP(-, 0, false, &fa))
LVALCASER(FVVSUBR, _FVOP(-, 0, false, &fa))
LVALCASER(FVVMUL , _FVOP(*, 0, false, &fa))
LVALCASER(FVVMULR, _FVOP(*, 0, false, &fa))
LVALCASER(FVVDIV , _FVOP(/, 1, false, &fa))
LVALCASER(FVVDIVR, _FVOP(/, 1, false, &fa))

LVALCASER(IVSADD , _IVOP(+, 0, true,  &fa))
LVALCASER(IVSADDR, _IVOP(+, 0, true,  &fa))
LVALCASER(IVSSUB , _IVOP(-, 0, true,  &fa))
LVALCASER(IVSSUBR, _IVOP(-, 0, true,  &fa))
LVALCASER(IVSMUL , _IVOP(*, 0, true,  &fa))
LVALCASER(IVSMULR, _IVOP(*, 0, true,  &fa))
LVALCASER(IVSDIV , _IVOP(/, 1, true,  &fa))
LVALCASER(IVSDIVR, _IVOP(/, 1, true,  &fa))
LVALCASER(IVSMOD , VMASSERT(0); (void)fa)
LVALCASER(IVSMODR, VMASSERT(0); (void)fa)

LVALCASER(FVSADD , _FVOP(+, 0, true,  &fa))
LVALCASER(FVSADDR, _FVOP(+, 0, true,  &fa))
LVALCASER(FVSSUB , _FVOP(-, 0, true,  &fa))
LVALCASER(FVSSUBR, _FVOP(-, 0, true,  &fa))
LVALCASER(FVSMUL , _FVOP(*, 0, true,  &fa))
LVALCASER(FVSMULR, _FVOP(*, 0, true,  &fa))
LVALCASER(FVSDIV , _FVOP(/, 1, true,  &fa))
LVALCASER(FVSDIVR, _FVOP(/, 1, true,  &fa))

LVALCASES(IADD   , _IOP(+, 0); a = res;          )
LVALCASES(IADDR  , _IOP(+, 0); a = res; VM_PUSH(res))
LVALCASES(ISUB   , _IOP(-, 0); a = res;          )
LVALCASES(ISUBR  , _IOP(-, 0); a = res; VM_PUSH(res))
LVALCASES(IMUL   , _IOP(*, 0); a = res;          )
LVALCASES(IMULR  , _IOP(*, 0); a = res; VM_PUSH(res))
LVALCASES(IDIV   , _IOP(/, 1); a = res;          )
LVALCASES(IDIVR  , _IOP(/, 1); a = res; VM_PUSH(res))
LVALCASES(IMOD   , _IOP(%, 1); a = res;          )
LVALCASES(IMODR  , _IOP(%, 1); a = res; VM_PUSH(res))

LVALCASES(FADD   , _FOP(+, 0); a = res;          )
LVALCASES(FADDR  , _FOP(+, 0); a = res; VM_PUSH(res))
LVALCASES(FSUB   , _FOP(-, 0); a = res;          )
LVALCASES(FSUBR  , _FOP(-, 0); a = res; VM_PUSH(res))
LVALCASES(FMUL   , _FOP(*, 0); a = res;          )
LVALCASES(FMULR  , _FOP(*, 0); a = res; VM_PUSH(res))
LVALCASES(FDIV   , _FOP(/, 1); a = res;          )
LVALCASES(FDIVR  , _FOP(/, 1); a = res; VM_PUSH(res))

LVALCASESTR(SADD , _SCAT(),    a = res;          )
LVALCASESTR(SADDR, _SCAT(),    a = res; VM_PUSH(res))

void VM::LV_WRITE    (Value &a) { auto  b = VM_POP();                      a = b; }
void VM::LV_WRITER   (Value &a) { auto &b = VM_TOP();                      a = b; }
void VM::LV_WRITEREF (Value &a) { auto  b = VM_POP(); a.LTDECRTNIL(*this); a = b; }
void VM::LV_WRITERREF(Value &a) { auto &b = VM_TOP(); a.LTDECRTNIL(*this); a = b; }

#define WRITESTRUCT(DECS) \
    auto l = VM_POP().intval(); \
    auto b = VM_TOPPTR() - l; \
    DECS; \
    t_memcpy(&a, b, l);

void VM::LV_WRITEV    (Value &a) { WRITESTRUCT({}); VM_POPN(l); }
void VM::LV_WRITERV   (Value &a) { WRITESTRUCT({}); }
void VM::LV_WRITEREFV (Value &a) { WRITESTRUCT(assert(0)); VM_POPN(l); }
void VM::LV_WRITERREFV(Value &a) { WRITESTRUCT(assert(0)); }


#define PPOP(name, ret, op, pre, accessor) void VM::LV_##name(Value &a) { \
    if (ret && !pre) VM_PUSH(a); \
    a.set##accessor(a.accessor() op 1); \
    if (ret && pre) VM_PUSH(a); \
}

PPOP(IPP  , false, +, true , ival)
PPOP(IPPR , true , +, true , ival)
PPOP(IMM  , false, -, true , ival)
PPOP(IMMR , true , -, true , ival)
PPOP(IPPP , false, +, false, ival)
PPOP(IPPPR, true , +, false, ival)
PPOP(IMMP , false, -, false, ival)
PPOP(IMMPR, true , -, false, ival)
PPOP(FPP  , false, +, true , fval)
PPOP(FPPR , true , +, true , fval)
PPOP(FMM  , false, -, true , fval)
PPOP(FMMR , true , -, true , fval)
PPOP(FPPP , false, +, false, fval)
PPOP(FPPPR, true , +, false, fval)
PPOP(FMMP , false, -, false, fval)
PPOP(FMMPR, true , -, false, fval)

string VM::ProperTypeName(const TypeInfo &ti) {
    switch (ti.t) {
        case V_STRUCT_R:
        case V_STRUCT_S:
        case V_CLASS: return string(ReverseLookupType(ti.structidx));
        case V_NIL: return ProperTypeName(GetTypeInfo(ti.subt)) + "?";
        case V_VECTOR: return "[" + ProperTypeName(GetTypeInfo(ti.subt)) + "]";
        default: return string(BaseTypeName(ti.t));
    }
}

void VM::BCallProf() {
    #ifdef VM_PROFILER
        vm_count_bcalls++;
    #endif
}

void VM::BCallRetCheck(const NativeFun *nf) {
    #if RTT_ENABLED
        // See if any builtin function is lying about what type it returns
        // other function types return intermediary values that don't correspond to final return
        // values.
        if (!nf->cont1) {
            for (size_t i = 0; i < nf->retvals.v.size(); i++) {
                auto t = (VM_TOPPTR() - nf->retvals.v.size() + i)->type;
                auto u = nf->retvals.v[i].type->t;
                assert(t == u || u == V_ANY || u == V_NIL || (u == V_VECTOR && IsUDT(t)));
            }
            assert(nf->retvals.v.size() || VM_TOP().type == V_NIL);
        }
    #else
        (void)nf;
    #endif
}

intp VM::GrabIndex() {
    auto len = VM_POP().ival();
    auto &v = VM_TOPM(len);
    for (len--; ; len--) {
        auto sidx = VM_POP().ival();
        if (!len) return sidx;
        RANGECHECK(sidx, v.vval()->len, v.vval());
        v = v.vval()->At(sidx);
    }
}

string_view VM::StructName(const TypeInfo &ti) {
    return bcf->udts()->Get(ti.structidx)->name()->string_view();
}

string_view VM::ReverseLookupType(uint v) {
    return bcf->udts()->Get(v)->name()->string_view();
}

void VM::StartWorkers(size_t numthreads) {
    if (is_worker) Error("workers can\'t start more worker threads");
    if (tuple_space) Error("workers already running");
    // Stop bad values from locking up the machine :)
    numthreads = min(numthreads, (size_t)256);
    tuple_space = new TupleSpace(bcf->udts()->size());
    for (size_t i = 0; i < numthreads; i++) {
        // Create a new VM that should own all its own memory and be completely independent
        // from this one.
        // We share nfr and programname for now since they're fully read-only.
        // FIXME: have to copy this even though it is read-only.
        auto bc = bytecode_buffer;
        auto wvm = new VM(nfr, programname, bc, compiled_code_ip, compiled_code_bc,
                          vector<string>());
        wvm->is_worker = true;
        wvm->tuple_space = tuple_space;
        workers.emplace_back([wvm] {
            string err;
            #ifdef USE_EXCEPTION_HANDLING
            try
            #endif
            {
                wvm->EvalProgram();
            }
            #ifdef USE_EXCEPTION_HANDLING
            catch (string &s) {
                if (s != "end-eval") err = s;
            }
            #endif
            delete wvm;
            // FIXME: instead return err to main thread?
            if (!err.empty()) LOG_ERROR("worker error: ", err);
        });
    }
}

void VM::TerminateWorkers() {
    if (is_worker || !tuple_space) return;
    tuple_space->alive = false;
    for (auto &tt : tuple_space->tupletypes) tt.condition.notify_all();
    for (auto &worker : workers) worker.join();
    workers.clear();
    delete tuple_space;
    tuple_space = nullptr;
}

void VM::WorkerWrite(RefObj *ref) {
    if (!tuple_space) return;
    if (!ref) Error("thread write: nil reference");
    auto &ti = ref->ti(*this);
    if (ti.t != V_CLASS) Error("thread write: must be a class");
    auto st = (LObject *)ref;
    auto buf = new Value[ti.len];
    for (int i = 0; i < ti.len; i++) {
        // FIXME: lift this restriction.
        if (IsRefNil(GetTypeInfo(ti.elemtypes[i]).t))
            Error("thread write: only scalar class members supported for now");
        buf[i] = st->AtS(i);
    }
    auto &tt = tuple_space->tupletypes[ti.structidx];
    {
        unique_lock<mutex> lock(tt.mtx);
        tt.tuples.push_back(buf);
    }
    tt.condition.notify_one();
}

LObject *VM::WorkerRead(type_elem_t tti) {
    auto &ti = GetTypeInfo(tti);
    if (ti.t != V_CLASS) Error("thread read: must be a class type");
    Value *buf = nullptr;
    auto &tt = tuple_space->tupletypes[ti.structidx];
    {
        unique_lock<mutex> lock(tt.mtx);
        tt.condition.wait(lock, [&] { return !tuple_space->alive || !tt.tuples.empty(); });
        if (!tt.tuples.empty()) {
            buf = tt.tuples.front();
            tt.tuples.pop_front();
        }
    }
    if (!buf) return nullptr;
    auto ns = NewObject(ti.len, tti);
    ns->Init(*this, buf, ti.len, false);
    delete[] buf;
    return ns;
}

}  // namespace lobster
