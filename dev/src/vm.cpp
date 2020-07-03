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

#include "lobster/vmops.h"

namespace lobster {

#ifndef NDEBUG
    #define VM_PROFILER              // tiny VM slowdown and memory usage when enabled
#endif

#ifdef VM_COMPILED_CODE_MODE
    #ifdef _WIN32
        #pragma warning (disable: 4458)  // ip hides class member, which we rely on
        #pragma warning (disable: 4100)  // ip may not be touched
    #endif
#endif

enum {
    // *8 bytes each
    INITSTACKSIZE   =  32 * 1024,
    // *8 bytes each, modest on smallest handheld we support (iPhone 3GS has 256MB).
    DEFMAXSTACKSIZE = 512 * 1024,
    // *8 bytes each, max by which the stack could possibly grow in a single call.
    STACKMARGIN     =   8 * 1024
};

#define MEASURE_INSTRUCTION_COMBINATIONS 0
#if MEASURE_INSTRUCTION_COMBINATIONS
map<pair<int, int>, size_t> instruction_combinations;
int last_instruction_opc = -1;
#endif

VM::VM(VMArgs &&vmargs, const bytecode::BytecodeFile *bcf)
    : VMArgs(std::move(vmargs)), maxstacksize(DEFMAXSTACKSIZE), bcf(bcf) {

    codelen = bcf->bytecode()->Length();
    if (FLATBUFFERS_LITTLEENDIAN) {
        // We can use the buffer directly.
        codestart = (const int *)bcf->bytecode()->Data();
        typetable = (const type_elem_t *)bcf->typetable()->Data();
    } else {
        for (uint32_t i = 0; i < codelen; i++)
            codebigendian.push_back(bcf->bytecode()->Get(i));
        codestart = codebigendian.data();

        for (uint32_t i = 0; i < bcf->typetable()->Length(); i++)
            typetablebigendian.push_back((type_elem_t)bcf->typetable()->Get(i));
        typetable = typetablebigendian.data();
    }
    #ifdef VM_COMPILED_CODE_MODE
        compiled_code_ip = entry_point;
    #else
        ip = codestart;
    #endif
    stack = new Value[stacksize = INITSTACKSIZE];
    #ifdef VM_PROFILER
        byteprofilecounts = new uint64_t[codelen];
        memset(byteprofilecounts, 0, sizeof(uint64_t) * codelen);
    #endif

    #ifdef VM_COMPILED_CODE_MODE
        #define F(N, A) f_ins_pointers[IL_##N] = nullptr;
    #else
        #define F(N, A) f_ins_pointers[IL_##N] = &F_##N;
    #endif
    ILNAMES
    #undef F

    constant_strings.resize(bcf->stringtable()->size());
    #ifdef VM_COMPILED_CODE_MODE
        assert(native_vtables);
        for (size_t i = 0; i < bcf->vtables()->size(); i++) {
            vtables.push_back(InsPtr(native_vtables[i]));
        }
    #else
        assert(!native_vtables);
        for (auto bcs : *bcf->vtables()) {
            vtables.push_back(InsPtr(bcs));
        }
    #endif
}

VM::~VM() {
    TerminateWorkers();
    if (stack) delete[] stack;
    if (byteprofilecounts) delete[] byteprofilecounts;
}

VMAllocator::VMAllocator(VMArgs &&args) {
    // Verify the bytecode.
    auto bcfb = (uint8_t *)(args.static_bytecode ? args.static_bytecode : args.bytecode_buffer.data());
    auto bcs = args.static_bytecode ? args.static_size : args.bytecode_buffer.size();
    flatbuffers::Verifier verifier(bcfb, bcs);
    auto ok = bytecode::VerifyBytecodeFileBuffer(verifier);
    if (!ok) THROW_OR_ABORT(string("bytecode file failed to verify"));
    auto bcf = bytecode::GetBytecodeFile(bcfb);
    if (bcf->bytecode_version() != LOBSTER_BYTECODE_FORMAT_VERSION)
        THROW_OR_ABORT(string("bytecode is from a different version of Lobster"));

    // Allocate enough memory to fit the "vars" array inline.
    auto size = sizeof(VM) + sizeof(Value) * bcf->specidents()->size();
    auto mem = malloc(size);
    assert(mem);
    memset(mem, 0, size);  // FIXME: this shouldn't be necessary.

    #undef new

    vm = new (mem) VM(std::move(args), bcf);

    #ifdef _WIN32
    #ifndef NDEBUG
    #define new DEBUG_NEW
    #endif
    #endif
}

VMAllocator::~VMAllocator() {
    if (!vm) return;
    vm->~VM();
    free(vm);
}


void VM::OneMoreFrame(StackPtr sp) {
    // We just landed back into the VM after being suspended inside a gl_frame() call.
    // Emulate the return of gl_frame():
    Push(sp, Value(1));  // We're not terminating yet.
    #ifdef VM_COMPILED_CODE_MODE
        // Native code generators ensure that next_call_target is set before
        // a native function call, and that it is returned to the trampoline
        // after, so do the same thing here.
        compiled_code_ip = (const void *)next_call_target;
    #endif
    EvalProgram(sp);   // Continue execution as if nothing happened.
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
    string sd;
    append(sd, prefix, ": ");
    RefToString(*this, sd, ro, debugpp);
    append(sd, " (", ro->refc, "): ", (size_t)ro);
    LOG_DEBUG(sd);
}

void VM::DumpFileLine(const int *fip, string &sd) {
    // error is usually in the byte before the current ip.
    auto li = LookupLine(fip - 1, codestart, bcf);
    append(sd, bcf->filenames()->Get(li->fileidx())->string_view(), "(", li->line(), ")");
}

void VM::DumpLeaks() {
    vector<void *> leaks = pool.findleaks();
    auto filename = "leaks.txt";
    if (leaks.empty()) {
        if (FileExists(filename)) FileDelete(filename);
    } else {
        LOG_ERROR("LEAKS FOUND (this indicates cycles in your object graph, or a bug in"
                             " Lobster)");
        string sd;
        #ifndef VM_COMPILED_CODE_MODE
            sd += "in: ";
            DumpFileLine(ip, sd);
            sd += "\n";
        #endif
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
                case V_CLASS: {
                    ro->CycleStr(sd);
                    sd += " = ";
                    RefToString(*this, sd, ro, leakpp);
                    #if DELETE_DELAY
                    sd += " ";
                    DumpFileLine(ro->alloc_ip, sd);
                    append(sd, " ", (size_t)ro);
                    #endif
                    sd += "\n";
                    break;
                }
                default: assert(false);
            }
        }
        #ifndef NDEBUG
            LOG_ERROR(sd);
        #else
            if (leaks.size() < 50) {
                LOG_ERROR(sd);
            } else {
                LOG_ERROR(leaks.size(), " leaks, details in ", filename);
                WriteFile(filename, false, sd);
            }
        #endif
    }
    pool.printstats(false);
}

void VM::OnAlloc(RefObj *ro) {
    #if DELETE_DELAY
        LOG_DEBUG("alloc: ", (size_t)ro, " - ", ro->refc);
        ro->alloc_ip = ip;
    #else
        (void)ro;
    #endif
}

#undef new

LVector *VM::NewVec(iint initial, iint max, type_elem_t tti) {
    assert(GetTypeInfo(tti).t == V_VECTOR);
    auto v = new (pool.alloc_small(sizeof(LVector))) LVector(*this, initial, max, tti);
    OnAlloc(v);
    return v;
}

LObject *VM::NewObject(iint max, type_elem_t tti) {
    assert(IsUDT(GetTypeInfo(tti).t));
    auto s = new (pool.alloc(ssizeof<LObject>() + ssizeof<Value>() * max)) LObject(tti);
    OnAlloc(s);
    return s;
}

LString *VM::NewString(iint l) {
    auto s = new (pool.alloc(ssizeof<LString>() + l + 1)) LString(l);
    OnAlloc(s);
    return s;\
}

LCoRoutine *VM::NewCoRoutine(StackPtr &sp, InsPtr rip, const int *vip, LCoRoutine *p, type_elem_t cti) {
    assert(GetTypeInfo(cti).t == V_COROUTINE);
    auto c = new (pool.alloc(sizeof(LCoRoutine)))
       LCoRoutine((int)(sp - stack + 2) /* top of sp + pushed coro */, (int)stackframes.size(), rip, vip, p, cti);
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

LString *VM::ResizeString(LString *s, iint size, int c, bool back) {
    auto ns = NewString(size);
    auto sdest = (char *)ns->data();
    auto cdest = sdest;
    auto remain = size - s->len;
    if (back) sdest += remain;
    else cdest += s->len;
    memcpy(sdest, s->data(), (size_t)s->len);
    memset(cdest, c, (size_t)remain);
    s->Dec(*this);
    return ns;
}

void VM::ErrorBase(string &sd, const string &err) {
    if (error_has_occured) {
        // We're calling this function recursively, not good. Try to get back to a reasonable
        // state by throwing an exception to be caught by the original error.
        THROW_OR_ABORT(err);
    }
    error_has_occured = true;
    if (trace == TraceMode::TAIL && trace_output.size()) {
        for (size_t i = trace_ring_idx; i < trace_output.size(); i++) sd += trace_output[i];
        for (size_t i = 0; i < trace_ring_idx; i++) sd += trace_output[i];
        sd += err;
        THROW_OR_ABORT(sd);
    }
    #ifndef VM_COMPILED_CODE_MODE
        DumpFileLine(ip, sd);
        sd += ": ";
    #endif
    append(sd, "VM error: ", err);
}

// This function is now way less important than it was when the language was still dynamically
// typed. But ok to leave it as-is for "index out of range" and other errors that are still dynamic.
Value VM::Error(StackPtr sp, string err) {
    string sd;
    ErrorBase(sd, err);
    try {
        while (sp >= stack && (!stackframes.size() || sp - stack != stackframes.back().spstart)) {
            // Sadly can't print this properly.
            sd += "\n   stack: ";
            to_string_hex(sd, (size_t)Top(sp).any());
            if (pool.pointer_is_in_allocator(Top(sp).any())) {
                sd += ", maybe: ";
                RefToString(*this, sd, Top(sp).ref(), debugpp);
            }
            Pop(sp);  // We don't DEC here, as we can't know what type it is.
                    // This is ok, as we ignore leaks in case of an error anyway.
        }
        for (;;) {
            if (!stackframes.size()) break;
            int deffun = *(stackframes.back().funstart);
            if (deffun >= 0) {
                append(sd, "\nin function: ", bcf->functions()->Get(deffun)->name()->string_view());
            } else {
                sd += "\nin block";
            }
            #ifndef VM_COMPILED_CODE_MODE
            sd += " -> ";
            DumpFileLine(ip, sd);
            #endif
            VarCleanup<1>(sp, sd.size() < 10000 ? &sd : nullptr, -2 /* clean up temps always */);
        }
    } catch (string &s) {
        // Error happened while we were building this stack trace.
        append(sd, "\nRECURSIVE ERROR:\n", s);
    }
    THROW_OR_ABORT(sd);
}

// Unlike Error above, this one does not attempt any variable dumping since the VM may already be
// in an inconsistent state.
Value VM::SeriousError(string err) {
    string sd;
    ErrorBase(sd, err);
    THROW_OR_ABORT(sd);
}

void VM::VMAssert(const char *what)  {
    SeriousError(string("VM internal assertion failure: ") + what);
}

int VM::DumpVar(string &sd, const Value &x, int idx) {
    auto sid = bcf->specidents()->Get((uint32_t)idx);
    auto id = bcf->idents()->Get(sid->ididx());
    // FIXME: this is not ideal, it filters global "let" declared vars.
    // It should probably instead filter global let vars whose values are entirely
    // constructors, and which are never written to.
    if (id->readonly() && id->global()) return 1;
    auto name = id->name()->string_view();
    auto &ti = GetVarTypeInfo(idx);
    #if RTT_ENABLED
        if (ti.t != x.type) return 1;  // Likely uninitialized.
    #endif
    append(sd, "\n   ", name, " = ");
    if (IsStruct(ti.t)) {
        StructToString(sd, debugpp, ti, &x);
        return ti.len;
    } else {
        x.ToString(*this, sd, ti, debugpp);
        return 1;
    }
}

void VM::FinalStackVarsCleanup(StackPtr &sp) {
    VMASSERT((*this), sp == stack - 1 && !stackframes.size());
    #ifndef NDEBUG
        LOG_INFO("stack at its highest was: ", maxsp);
    #else
        (void)sp;
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

template<int is_error> int VM::VarCleanup(StackPtr &sp, string *error, int towhere) {
    (void)error;
    auto &stf = stackframes.back();
    if constexpr (!is_error) {
        auto depth = sp - stack;
        if (depth != stf.spstart) {
            VMASSERT((*this), false);
        }
        (void)depth;
    }
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
        for (int j = 0; j < ndef; ) {
            auto i = *(defvars - j - 1);
            j += DumpVar(*error, vars[i], i);
        }
        for (int j = 0; j < nargs; ) {
            auto i = *(freevars - j - 1);
            j += DumpVar(*error, vars[i], i);
        }
    }
    for (int i = 0; i < nkeepvars; i++) Pop(sp).LTDECRTNIL(*this);
    auto ownedvars = *fip++;
    for (int i = 0; i < ownedvars; i++) vars[*fip++].LTDECRTNIL(*this);
    while (ndef--) {
        auto i = *--defvars;
        vars[i] = Pop(sp);
    }
    while (nargs--) {
        auto i = *--freevars;
        vars[i] = Pop(sp);
    }
    JumpTo(stf.retip);
    bool lastunwind = towhere == *stf.funstart;
    stackframes.pop_back();
    if (!lastunwind) {
        // This kills any temps on the stack. If these are refs these should not be
        // owners, since a var or keepvar owns them instead.
        sp = (stackframes.size() ? stackframes.back().spstart : -1) + stack;
    }
    return lastunwind;
}

// Initializes only 3 fields of the stack frame, FunIntro must be called right after.
void VM::StartStackFrame(InsPtr retip) {
    stackframes.push_back(StackFrame());
    auto &stf = stackframes.back();
    stf.retip = retip;
}

void VM::FunIntroPre(StackPtr &sp, InsPtr fun) {
    JumpTo(fun);
    #ifdef VM_COMPILED_CODE_MODE
        // We don't call FunIntro() here, instead the compiled code for FUNSTART/FUNMULTI actually
        // does that.
    #else
        VMASSERT((*this), *ip == IL_FUNSTART);
        ip++;
        FunIntro(sp);
    #endif
}

// Only valid to be called right after StartStackFrame, with no bytecode in-between.
void VM::FunIntro(StackPtr &sp VM_COMMA VM_OP_ARGS) {
    #ifdef VM_PROFILER
        vm_count_fcalls++;
    #endif
    auto funstart = ip;
    ip++;  // definedfunction
    if (sp - stack > stacksize - STACKMARGIN) {
        // per function call increment should be small
        // FIXME: not safe for untrusted scripts, could simply add lots of locals
        // could record max number of locals? not allow more than N locals?
        if (stacksize >= maxstacksize)
            SeriousError("stack overflow! (use set_max_stack_size() if needed)");
        auto nstack = new Value[stacksize *= 2];
        t_memcpy(nstack, stack, sp - stack + 1);
        sp = sp - stack + nstack;
        delete[] stack;
        stack = nstack;


        LOG_DEBUG("stack grew to: ", stacksize);
    }
    auto nargs_fun = *ip++;
    for (int i = 0; i < nargs_fun; i++) swap(vars[ip[i]], *(sp - nargs_fun + i + 1));
    ip += nargs_fun;
    auto ndef = *ip++;
    for (int i = 0; i < ndef; i++) {
        // for most locals, this just saves an nil, only in recursive cases it has an actual value.
        auto varidx = *ip++;
        Push(sp, vars[varidx]);
        vars[varidx] = Value();
    }
    auto nkeepvars = *ip++;
    for (int i = 0; i < nkeepvars; i++) Push(sp, Value());
    auto nownedvars = *ip++;
    ip += nownedvars;
    auto &stf = stackframes.back();
    stf.funstart = funstart;
    stf.spstart = sp - stack;
    #ifndef NDEBUG
        if (sp - stack > maxsp) maxsp = sp - stack;
    #endif
}

void VM::FunOut(StackPtr &sp, int towhere, int nrv) {
    sp -= nrv;
    // Have to store these off the stack, since VarCleanup() may cause stack activity if coroutines
    // are destructed.
    ts_memcpy(retvalstemp, TopPtr(sp), nrv);
    for(;;) {
        if (!stackframes.size()) {
            SeriousError("\"return from " + bcf->functions()->Get(towhere)->name()->string_view() +
                         "\" outside of function");
        }
        if (VarCleanup<0>(sp, nullptr, towhere)) break;
    }
    ts_memcpy(TopPtr(sp), retvalstemp, nrv);
    sp += nrv;
}

void VM::CoVarCleanup(StackPtr &sp, LCoRoutine *co) {
    // Convenient way to copy everything back onto the stack.
    InsPtr tip(0);
    auto copylen = co->Resume(sp - stack + 1, stack, stackframes, tip, nullptr);
    auto startsp = sp;
    sp += copylen;
    for (auto i = co->stackframecopylen - 1; i >= 0 ; i--) {
        auto &stf = stackframes.back();
        sp = stf.spstart + stack;  // Kill any temps on top of the stack.
        // Save the ip, because VarCleanup will jump to it.
        auto bip = GetIP();
        VarCleanup<0>(sp, nullptr, !i ? *stf.funstart : -2);
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
        SeriousError("cannot create coroutine recursively");
    }
    // this check guarantees all saved stack vars are undef.
}

void VM::CoNew(StackPtr &sp VM_COMMA VM_OP_ARGS VM_COMMA VM_OP_ARGS_CALL) {
    CleanupDelayDeleteCoroutines(sp);  // As good a place as any.
    #ifdef VM_COMPILED_CODE_MODE
        ip++;
        InsPtr returnip(fcont);
    #else
        InsPtr returnip(*ip++);
    #endif
    auto ctidx = (type_elem_t)*ip++;
    CoNonRec(ip);
    curcoroutine = NewCoRoutine(sp, returnip, ip, curcoroutine, ctidx);
    curcoroutine->BackupParentVars(*this, vars);
    int nvars = *ip++;
    ip += nvars;
    // Always have the active coroutine at top of the stack, retaining 1 refcount. This is
    // because it is not guaranteed that there any other references, and we can't have this drop
    // to 0 while active.
    Push(sp, Value(curcoroutine));
}

void VM::CoSuspend(StackPtr &sp, InsPtr retip) {
    auto newtop = curcoroutine->Suspend(*this, sp - stack + 1, stack, stackframes, retip, curcoroutine);
    JumpTo(retip);
    sp = newtop - 1 + stack; // top of stack is now coro value from create or resume
}

void VM::CoClean(StackPtr &sp) {
    // This function is like yield, except happens implicitly when the coroutine returns.
    // It will jump back to the resume (or create) that invoked it.
    for (int i = 1; i <= *curcoroutine->varip; i++) {
        auto &var = vars[curcoroutine->varip[i]];
        var = curcoroutine->stackcopy[i - 1];
    }
    auto co = curcoroutine;
    CoSuspend(sp, InsPtr(0));
    VMASSERT((*this), co->stackcopylen == 1);
    co->active = false;
}

void VM::CoYield(StackPtr &sp VM_COMMA VM_OP_ARGS_CALL) {
    assert(curcoroutine);  // Should not be possible since yield calls are statically checked.
    #ifdef VM_COMPILED_CODE_MODE
        InsPtr retip(fcont);
    #else
        InsPtr retip(ip - codestart);
    #endif
    auto ret = Pop(sp);
    for (int i = 1; i <= *curcoroutine->varip; i++) {
        auto &var = vars[curcoroutine->varip[i]];
        Push(sp, var);
        //var.type = V_NIL;
        var = curcoroutine->stackcopy[i - 1];
    }
    Push(sp, ret);  // current value always top of the stack, saved as part of suspended coroutine.
    CoSuspend(sp, retip);
    // Actual top of stack here is coroutine itself, that we placed here with CoResume.
}

void VM::CoResume(StackPtr &sp, LCoRoutine *co) {
    if (co->stackstart >= 0)
        Error(sp, "cannot resume running coroutine");
    if (!co->active)
        Error(sp, "cannot resume finished coroutine");
    // This will be the return value for the corresponding yield, and holds the ref for gc.
    Push(sp, Value(co));
    CoNonRec(co->varip);
    auto rip = GetIP();
    sp += co->Resume(sp - stack + 1, stack, stackframes, rip, curcoroutine);
    JumpTo(rip);
    curcoroutine = co;
    // must be, since those vars got backed up in it before
    VMASSERT((*this), curcoroutine->stackcopymax >=  *curcoroutine->varip);
    curcoroutine->stackcopylen = *curcoroutine->varip;
    //curcoroutine->BackupParentVars(vars);
    Pop(sp).LTDECTYPE(*this, GetTypeInfo(curcoroutine->ti(*this).yieldtype).t);    // previous current value
    for (int i = *curcoroutine->varip; i > 0; i--) {
        auto &var = vars[curcoroutine->varip[i]];
        // No INC, since parent is still on the stack and hold ref for us.
        curcoroutine->stackcopy[i - 1] = var;
        var = Pop(sp);
    }
    // the builtin call takes care of the return value
}

void VM::CleanupDelayDeleteCoroutines(StackPtr &sp) {
    for (auto c : delete_delay_coroutine) c->DelayedDelete(sp, *this);
    delete_delay_coroutine.clear();
}

void VM::EndEval(StackPtr &sp, const Value &ret, const TypeInfo &ti) {
    TerminateWorkers();
    ret.ToString(*this, evalret, ti, programprintprefs);
    ret.LTDECTYPE(*this, ti.t);
    assert(sp == stack - 1);
    FinalStackVarsCleanup(sp);
    for (auto s : constant_strings) {
        if (s) s->Dec(*this);
    }
    while (!delete_delay.empty()) {
        auto ro = delete_delay.back();
        delete_delay.pop_back();
        ro->DECDELETENOW(*this);
    }
    // This one is last, since the above cleanup may result in new delayed coroutines.
    CleanupDelayDeleteCoroutines(sp);
    DumpLeaks();
    VMASSERT((*this), !curcoroutine);
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
        for (uint32_t i = 0; i < bcf->lineinfo()->size(); i++) {
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
    THROW_OR_ABORT(string("end-eval"));
}

void VM::EvalProgram(StackPtr sp) {
    if (!sp) sp = stack - 1;
    // Keep exception handling code in seperate function from hot loop in EvalProgramInner()
    // just in case it affects the compiler.
    #ifdef USE_EXCEPTION_HANDLING
    try
    #endif
    {
        EvalProgramInner(sp);
    }
    #ifdef USE_EXCEPTION_HANDLING
    catch (string &s) {
        if (s != "end-eval") THROW_OR_ABORT(s);
    }
    #endif
}

string &VM::TraceStream() {
  size_t trace_size = trace == TraceMode::TAIL ? 50 : 1;
  if (trace_output.size() < trace_size) trace_output.resize(trace_size);
  if (trace_ring_idx == trace_size) trace_ring_idx = 0;
  auto &sd = trace_output[trace_ring_idx++];
  sd.clear();
  return sd;
}

void VM::EvalProgramInner(StackPtr sp) {
    for (;;) {
        #ifdef VM_COMPILED_CODE_MODE
            compiled_code_ip = ((block_base_t)compiled_code_ip)(*this, sp);
        #else
            #ifndef NDEBUG
                if (trace != TraceMode::OFF) {
                    auto &sd = TraceStream();
                    DisAsmIns(nfr, sd, ip, codestart, typetable, bcf);
                    append(sd, " [", sp - stack + 1, "] -");
                    #if RTT_ENABLED
                    #if DELETE_DELAY
                        append(sd, " ", (size_t)Top(sp).any());
                    #endif
                    for (int i = 0; i < 3 && sp - i >= stack; i++) {
                        auto x = TopM(sp, i);
                        sd += ' ';
                        x.ToStringBase(*this, sd, x.type, debugpp);
                    }
                    #endif
                    if (trace == TraceMode::TAIL) sd += '\n'; else LOG_PROGRAM(sd);
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
                    SeriousError(cat("bytecode format problem: ", op));
            #endif
            sp = f_ins_pointers[op](*this, sp);
        #endif
    }
}

string VM::ProperTypeName(const TypeInfo &ti) {
    switch (ti.t) {
        case V_STRUCT_R:
        case V_STRUCT_S:
        case V_CLASS: return string(ReverseLookupType(ti.structidx));
        case V_NIL: return ProperTypeName(GetTypeInfo(ti.subt)) + "?";
        case V_VECTOR: return "[" + ProperTypeName(GetTypeInfo(ti.subt)) + "]";
        case V_INT: return ti.enumidx >= 0 ? string(EnumName(ti.enumidx)) : "int";
        default: return string(BaseTypeName(ti.t));
    }
}

void VM::BCallProf() {
    #ifdef VM_PROFILER
        vm_count_bcalls++;
    #endif
}

void VM::BCallRetCheck(StackPtr sp, const NativeFun *nf) {
    #if RTT_ENABLED
        // See if any builtin function is lying about what type it returns
        // other function types return intermediary values that don't correspond to final return
        // values.
        if (!nf->cont1) {
            for (size_t i = 0; i < nf->retvals.size(); i++) {
                #ifndef NDEBUG
                auto t = (TopPtr(sp) - nf->retvals.size() + i)->type;
                auto u = nf->retvals[i].type->t;
                assert(t == u || u == V_ANY || u == V_NIL || (u == V_VECTOR && IsUDT(t)));
                #endif
            }
            assert(nf->retvals.size() || Top(sp).type == V_NIL);
        }
    #else
        (void)nf;
    #endif
    (void)sp;
}

iint VM::GrabIndex(StackPtr &sp, int len) {
    auto &v = TopM(sp, len);
    for (len--; ; len--) {
        auto sidx = Pop(sp).ival();
        if (!len) return sidx;
        RANGECHECK((*this), sidx, v.vval()->len, v.vval());
        v = v.vval()->At(sidx);
    }
}

void VM::IDXErr(StackPtr sp, iint i, iint n, const RefObj *v) {
    string sd;
    append(sd, "index ", i, " out of range ", n, " of: ");
    RefToString(*this, sd, v, debugpp);
    Error(sp, sd);
}

string_view VM::StructName(const TypeInfo &ti) {
    return bcf->udts()->Get(ti.structidx)->name()->string_view();
}

string_view VM::ReverseLookupType(int v) {
    return bcf->udts()->Get((flatbuffers::uoffset_t)v)->name()->string_view();
}

bool VM::EnumName(string &sd, iint enum_val, int enumidx) {
    auto enum_def = bcf->enums()->Get(enumidx);
    auto &vals = *enum_def->vals();
    auto lookup = [&](iint val) -> bool {
        // FIXME: can store a bool that says wether this enum is contiguous, so we just index instead.
        for (auto v : vals)
            if (v->val() == val) {
                sd += v->name()->string_view();
                return true;
            }
        return false;
    };
    if (!enum_def->flags() || !enum_val) return lookup(enum_val);
    auto start = sd.size();
    auto upto = 64 - HighZeroBits(enum_val);
    for (int i = 0; i < upto; i++) {
        auto bit = enum_val & (1LL << i);
        if (bit) {
            if (sd.size() != start) sd += "|";
            if (!lookup(bit)) {
                // enum contains unknown bits, so can't display this properly.
                sd.resize(start);
                return false;
            }
        }
    }
    return true;
}

string_view VM::EnumName(int enumidx) {
    return bcf->enums()->Get(enumidx)->name()->string_view();
}

optional<int64_t> VM::LookupEnum(string_view name, int enumidx) {
    auto &vals = *bcf->enums()->Get(enumidx)->vals();
    for (auto v : vals)
        if (v->name()->string_view() == name)
            return v->val();
    return {};
}

void VM::StartWorkers(StackPtr &sp, iint numthreads) {
    if (is_worker) Error(sp, "workers can\'t start more worker threads");
    if (tuple_space) Error(sp, "workers already running");
    // Stop bad values from locking up the machine :)
    numthreads = min(numthreads, 256_L);
    tuple_space = new TupleSpace(bcf->udts()->size());
    for (iint i = 0; i < numthreads; i++) {
        // Create a new VM that should own all its own memory and be completely independent
        // from this one.
        // We share nfr and programname for now since they're fully read-only.
        // FIXME: have to copy bytecode buffer even though it is read-only.
        auto vmargs = *(VMArgs *)this;
        vmargs.program_args.resize(0);
        vmargs.trace = TraceMode::OFF;
        auto vma = new VMAllocator(std::move(vmargs));
        vma->vm->is_worker = true;
        vma->vm->tuple_space = tuple_space;
        workers.emplace_back([vma] {
            string err;
            #ifdef USE_EXCEPTION_HANDLING
            try
            #endif
            {
                vma->vm->EvalProgram();
            }
            #ifdef USE_EXCEPTION_HANDLING
            catch (string &s) {
                if (s != "end-eval") err = s;
            }
            #endif
            delete vma;
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

void VM::WorkerWrite(StackPtr &sp, RefObj *ref) {
    if (!tuple_space) return;
    if (!ref) Error(sp, "thread write: nil reference");
    auto &ti = ref->ti(*this);
    if (ti.t != V_CLASS) Error(sp, "thread write: must be a class");
    auto st = (LObject *)ref;
    auto buf = new Value[ti.len];
    for (int i = 0; i < ti.len; i++) {
        // FIXME: lift this restriction.
        if (IsRefNil(GetTypeInfo(ti.elemtypes[i]).t))
            Error(sp, "thread write: only scalar class members supported for now");
        buf[i] = st->AtS(i);
    }
    auto &tt = tuple_space->tupletypes[ti.structidx];
    {
        unique_lock<mutex> lock(tt.mtx);
        tt.tuples.push_back(buf);
    }
    tt.condition.notify_one();
}

LObject *VM::WorkerRead(StackPtr &sp, type_elem_t tti) {
    auto &ti = GetTypeInfo(tti);
    if (ti.t != V_CLASS) Error(sp, "thread read: must be a class type");
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


// Make VM ops available as C functions for linking purposes:

#ifdef VM_COMPILED_CODE_MODE

extern "C" {

using namespace lobster;

void CVM_Trace(VM *vm, StackPtr sp, string op) {
    auto &sd = vm->TraceStream();
    sd += op;
    #if RTT_ENABLED
        if (sp >= vm->stack) {
            sd += " - ";
            Top(sp).ToStringBase(*vm, sd, Top(sp).type, vm->debugpp);
            if (sp > vm->stack) {
                sd += " - ";
                TopM(sp, 1).ToStringBase(*vm, sd, TopM(sp, 1).type, vm->debugpp);
            }
        }
    #else
        (void)sp;
    #endif
    // append(sd, " / ", (size_t)Top(sp).any());
    // for (int _i = 0; _i < 7; _i++) { append(sd, " #", (size_t)vm->vars[_i].any()); }
    if (vm->trace == TraceMode::TAIL) sd += "\n"; else LOG_PROGRAM(sd);
}

#ifndef NDEBUG
    #define CHECKI(B) if (vm->trace != TraceMode::OFF) CVM_Trace(vm, sp, B);
    #define CHECK(N, A) CHECKI(cat(#N, cat_parens A))
    #define CHECKJ(N) CHECKI(#N)
#else
    #define CHECK(N, A)
    #define CHECKJ(N)
#endif

void CVM_SetNextCallTarget(VM *vm, block_base_t fcont) {
    vm->next_call_target = fcont;
}

block_base_t CVM_GetNextCallTarget(VM *vm) {
    return vm->next_call_target;
}

// Only here because in compiled code we don't know sizeof(Value) (!)
StackPtr CVM_Drop(StackPtr sp) { return --sp; }

#define F(N, A) \
    StackPtr CVM_##N(VM *vm, StackPtr sp VM_COMMA_IF(A) VM_OP_ARGSN(A)) { \
        CHECK(N, (VM_OP_PASSN(A))); return U_##N(*vm, sp VM_COMMA_IF(A) VM_OP_PASSN(A)); }
    LVALOPNAMES
#undef F
#define F(N, A) \
    StackPtr CVM_##N(VM *vm, StackPtr sp VM_COMMA_IF(A) VM_OP_ARGSN(A)) { \
        CHECK(N, (VM_OP_PASSN(A))); return U_##N(*vm, sp VM_COMMA_IF(A) VM_OP_PASSN(A)); }
    ILBASENAMES
#undef F
#define F(N, A) \
    StackPtr CVM_##N(VM *vm, StackPtr sp VM_COMMA_IF(A) VM_OP_ARGSN(A), block_base_t fcont) { \
        CHECK(N, (VM_OP_PASSN(A))); return U_##N(*vm, sp VM_COMMA VM_OP_PASSN(A) VM_COMMA_IF(A) fcont); }
    ILCALLNAMES
#undef F
#define F(N, A) \
    StackPtr CVM_##N(VM *vm, StackPtr sp) { \
        CHECKJ(N); return U_##N(*vm, sp); }
    ILJUMPNAMES
#undef F

}  // extern "C"

#endif  // VM_COMPILED_CODE_MODE
