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

#include "lobster/vmops.h"

#include "lobster/lobsterreader.h"

namespace lobster {

#if LOBSTER_FRAME_PROFILER_GLOBAL
// This is only here to debug difficult crashes on platforms without a stack trace.
thread_local vector<___tracy_source_location_data> g_function_locations;
thread_local vector<___tracy_source_location_data> g_builtin_locations;
#endif

VM::VM(VMArgs &&vmargs)
    : vma(std::move(vmargs)) {

    typetable = vma.meta->type_table.data();
    native_funs = vma.nfr.nfun_ptrs.data();
    // Allocated up front rather than on first use, so that pushing one is a load rather than a
    // load and a branch. They are owned by the VM for its entire lifetime, see EndEval.
    constant_strings.reserve(vma.meta->stringtable.size());
    for (auto s : vma.meta->stringtable) constant_strings.push_back(Value(NewString(s)));
    constant_strings_ptr = constant_strings.data();
    fvars_ptr = fvars;
    assert(vma.native_vtables);

    #if LOBSTER_FRAME_PROFILER
        auto &funs = vma.meta->function_names;
        for (auto f : funs) {
            pre_allocated_function_locations.push_back(
                ___tracy_source_location_data{ f.data(), f.data(), "", 0, 0x008888 });
        }
    #endif
}

VM::~VM() {
    TerminateWorkers();
    if (byteprofilecounts) delete[] byteprofilecounts;

    #if LOBSTER_FRAME_PROFILER == 2
        // FIXME: this is not ideal, because there may be multiple VMs.
        // But the profiler runs its own thread, and may be accessing pre_allocated_function_locations
        // stored in this VM.
        if (tracy::GetProfiler().IsConnected()) {
            tracy::GetProfiler().RequestShutdown();
            //while (!tracy::GetProfiler().HasShutdownFinished()) {
            //    std::this_thread::sleep_for(std::chrono::milliseconds(10));
            //}
            tracy::GetProfiler().~Profiler();
            // FIXME: have to do this to avoid it crashing when destructed twice.
            abort();
        }
    #endif
}

VMAllocator::VMAllocator(VMArgs &&args) {
    // Allocate enough memory to fit the "fvars" array inline.
    auto size = sizeof(VM) + sizeof(Value) * args.meta->specidents.size();
    auto mem = malloc(size);
    assert(mem);
    memset(mem, 0, size);  // FIXME: this shouldn't be necessary.

    #undef new

    vm = new (mem) VM(std::move(args));

    #if defined(_MSC_VER) && !defined(NDEBUG)
        #define new DEBUG_NEW
    #endif
}

VMAllocator::~VMAllocator() {
    if (!vm) return;
    vm->~VM();
    free(vm);
}

const TypeInfo &VM::GetVarTypeInfo(int varidx) {
    return GetTypeInfo((type_elem_t)vma.meta->specidents[varidx].typeidx);
}

type_elem_t VM::GetSubClassFromSerID(type_elem_t super, uint32_t ser_id) {
    auto ser_ids = vma.meta->ser_ids;
    auto size = ser_ids.size();
    if (ser_id >= size) return (type_elem_t)-1;
    auto typeoff = (type_elem_t)ser_ids[ser_id];
    if (typeoff == super) return typeoff;
    auto sup = &GetTypeInfo(typeoff);
    for (;;) {
        auto supoff = sup->superclass;
        if (supoff < 0) return (type_elem_t)-1;
        if (supoff == super) return typeoff;
        sup = &GetTypeInfo(supoff);
    }
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

void VM::DumpLeaks() {
    if (!vma.dump_leaks) return;
    vector<void *> leaks = pool.findleaks();
    auto filename = "memory_leaks.txt";
    if (leaks.empty()) {
        if (FileExists(filename, false)) FileDelete(filename);
    } else {
        LOG_ERROR("LEAKS FOUND (this indicates cycles in your object graph, or a bug in"
                             " Lobster)");
        sort(leaks.begin(), leaks.end(), _LeakSorter);
        PrintPrefs leakpp = debugpp;
        leakpp.cycles = 0;
        string sd;
        for (auto p : leaks) {
            auto ro = (RefObj *)p;
            switch(ro->ti(*this).t) {
                case RTT_VALUEBUF:
                    break;
                case RTT_STRING:
                case RTT_RESOURCE:
                case RTT_VECTOR:
                case RTT_CLASS: {
                    ro->CycleStr(sd);
                    sd += " = ";
                    RefToString(*this, sd, ro, leakpp);
                    #if DELETE_DELAY
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
                WriteFile(filename, false, sd, false);
            }
        #endif
    }
    if (!is_worker) pool.printstats(false);
}


struct Stat {
    size_t num = 0;
    size_t bytes = 0;
    size_t max = 0;
    size_t gpu = 0;
    const ResourceType *rt = nullptr;

    void Add(size_t2 size, const ResourceType *_rt = nullptr) {
        num++;
        bytes += size.x + size.y;
        max = std::max(max, size.x + size.y);
        gpu += size.y;
        rt = _rt;
    }

    void Add(size_t size) {
        Add(size_t2(size, 0));
    }
};

static bool _UsageSorter(const pair<const void *, Stat> &a, const pair<const void *, Stat> &b) {
    return a.second.bytes != b.second.bytes ? a.second.bytes > b.second.bytes : false;
}

string VM::MemoryUsage(size_t show_max) {
    vector<void *> leaks = pool.findleaks();
    string sd;
    map<const void *, Stat> stats;
    for (auto p : leaks) {
        auto ro = (RefObj *)p;
        auto &ti = ro->ti(*this);
        switch(ti.t) {
            case RTT_VALUEBUF:
                break;
            case RTT_STRING:
                stats[&ti].Add(((LString *)ro)->MemoryUsage());
                break;
            case RTT_RESOURCE:
                stats[((LResource *)ro)->type].Add(((LResource *)ro)->MemoryUsage(),
                                                   ((LResource *)ro)->type);
                break;
            case RTT_VECTOR:
                stats[&ti].Add(((LVector *)ro)->MemoryUsage());
                break;
            case RTT_CLASS:
                stats[&ti].Add(((LObject *)ro)->MemoryUsage(*this));
                break;
            default:
                assert(false);
        }
    }
    vector<pair<const void *, Stat>> sorted;
    size_t total = 0;
    size_t totalgpu = 0;
    for (auto &p : stats) {
        sorted.push_back(p);
        total += p.second.bytes;
        totalgpu += p.second.gpu;
    }
    sort(sorted.begin(), sorted.end(), _UsageSorter);
    if (!total) return sd;
    const char *suffixes[] = {
        " B", " KB", " MB", " GB", " TB", " PB", " EB", " ZB", "YB"
    };
    auto show_amount = [&](size_t a) -> string {
        for (size_t i = 0; ; i++) {
            if (a < 10 * 1024)
                return cat(a, suffixes[i]);
            a /= 1024;
        }
    };
    append(sd, "TOTAL: ", show_amount(total), " (", totalgpu * 100 / total, "% on GPU)\n");
    for (auto [i, p] : enumerate(sorted)) {
        if (i >= show_max || p.second.bytes < 1024) break;
        if (p.second.rt) append(sd, "resource<", p.second.rt->name, ">");
        else append(sd, ((const TypeInfo *)p.first)->Debug(*this, false));
        append(sd, ": ", show_amount(p.second.bytes), " in ", p.second.num, " objects");
        if (p.second.max >= 1024 && p.second.max != p.second.bytes / p.second.num) {
            append(sd, " (biggest: ", show_amount(p.second.max), ")");
        }
        if (p.second.gpu) {
            append(sd, " (", p.second.gpu * 100 / p.second.bytes, "% on GPU)");
        }
        append(sd, "\n");
    }
    return sd;
}

void VM::OnAlloc(RefObj *ro) {
    #if DELETE_DELAY
        LOG_DEBUG("alloc: ", (size_t)ro, " - ", ro->refc);
    #else
        (void)ro;
    #endif
}

#undef new

LVector *VM::NewVec(iint initial, iint max, type_elem_t tti) {
    assert(GetTypeInfo(tti).t == RTT_VECTOR);
    auto v = new (pool.alloc_small(sizeof(LVector))) LVector(*this, initial, max, tti);
    OnAlloc(v);
    return v;
}

LObject *VM::NewObject(iint max, type_elem_t tti) {
    assert(RTIsUDT(GetTypeInfo(tti).t));
    auto s = new (pool.alloc(ssizeof<LObject>() + ssizeof<Value>() * max)) LObject(tti);
    OnAlloc(s);
    return s;
}

LString *VM::NewString(iint l) {
    auto s = new (pool.alloc(ssizeof<LString>() + l + 1)) LString(l);
    OnAlloc(s);
    return s;
}

#if defined(_MSC_VER) && !defined(NDEBUG)
    #define new DEBUG_NEW
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

void VM::ErrorBase(const string &err) {
    if (error_has_occured) {
        // We're calling this function recursively, not good. Try to get back to a reasonable
        // state by throwing an exception to be caught by the original error.
        errmsg = err;
        UnwindOnError();
    }
    error_has_occured = true;
    append(errmsg, "VM error (");
    if (last.line >= 0 && last.fileidx >= 0) {
        append(errmsg, vma.meta->file_names[last.fileidx], ":", last.line);
    } else {
        append(errmsg, vma.programname);
    }
    append(errmsg, "): ", err);
}

void VM::DumpVar(Value *locals, int idx, int &j, int &jl, const DumperFun &dump) {
    auto &sid = vma.meta->specidents[(uint32_t)idx];
    auto is_freevar = sid.used_as_freevar;
    auto name = string_view_nt(sid.name);
    auto &ti = GetVarTypeInfo(idx);
    auto width = RTIsStruct(ti.t) ? ti.len : 1;
    auto x = is_freevar ? &fvars[idx] : locals + jl;
    // FIXME: this is not ideal, it filters global "let" declared vars.
    // It should probably instead filter global let vars whose values are entirely
    // constructors, and which are never written to.
    if (!sid.readonly || !sid.global) {
        dump(*this, name, ti, x);
    }
    j += width;
    if (!is_freevar) jl += width;
}

void VM::DumpStackFrame(const int *fip, Value *locals, const DumperFun &dump) {
    auto nargs = *fip++;
    auto args = fip;
    fip += nargs;
    auto ndef = *fip++;
    auto defvars = fip;
    fip += ndef;
    int jla = 0;
    for (int j = 0; j < nargs;) {
        auto i = *(args + j);
        DumpVar(locals, i, j, jla, dump);
    }
    for (int j = 0, jld = 0; j < ndef;) {
        auto i = *(defvars + j);
        DumpVar(locals + jla, i, j, jld, dump);
    }
}

string VM::DumpFileLine(int fileidx, int line) {
    string loc;
    if (line >= 0 && fileidx >= 0) {
        append(loc, "[", vma.meta->file_names[fileidx], ":", line, "]");
    }
    return loc;
}

pair<string, const int *> VM::DumpStackFrameStart(const int *fip, int fileidx, int line) {
    auto sf_idx = *fip++;
    string fname;
    auto nargs = fip[0];
    if (nargs) {
        auto &ti = GetVarTypeInfo(fip[1]);
        ti.Print(*this, fname, nullptr);
        append(fname, ".");
    }
    append(fname, vma.meta->function_names[vma.meta->subfunctions_to_function[sf_idx]],
           DumpFileLine(fileidx, line));
    return { fname, fip };
}

// See also imbind.cpp:DumpStackTrace and VM::DumpStackTraceMemory()
void VM::DumpStackTrace(string &sd, bool python_ordering) {
    if (fun_id_stack.empty()) {
        // We don't have a stack trace, but maybe this minimum information will be better
        // than nothing:
        sd += DumpFileLine(last.fileidx, last.line);
        return;
    }

    #ifdef USE_EXCEPTION_HANDLING
    try {
    #endif

    DumperFun dumper = [&sd](VM &vm, string_view_nt name, const TypeInfo &ti, Value *x) {
        append(sd, "        ", name);
        #if RTT_ENABLED
            auto debug_type = x->type;
        #else
            auto debug_type = ti.t;
        #endif
        if (debug_type == RTT_NIL && !ti.is_nil) {
            // Uninitialized.
            append(sd, ":");
            ti.Print(vm, sd, nullptr);
            append(sd, " (uninitialized)");
        } else if (ti.t != debug_type && !RTIsStruct(ti.t)) {
            // Some runtime type corruption, show the problem rather than crashing.
            append(sd, ":");
            ti.Print(vm, sd, nullptr);
            append(sd, " ERROR != ", BaseTypeName(debug_type));
        } else {
            append(sd, " = ");
            PrintPrefs minipp { 1, 20, true, -1 };
            if (RTIsStruct(ti.t)) {
                vm.StructToString(sd, minipp, ti, x);
            } else {
                x->ToString(vm, sd, ti, minipp);
            }
        }
        append(sd, "\n");
    };

    if (!sd.empty()) append(sd, "\n");
    if (!python_ordering) {
        auto cur_fileidx = last.fileidx;
        auto cur_line = last.line;
        for (auto &funstackelem : reverse(fun_id_stack)) {
            auto [name, fip] =
                DumpStackFrameStart(funstackelem.funstartinfo, cur_fileidx, cur_line);
            append(sd, "in function ", name, "\n");
            DumpStackFrame(fip, funstackelem.locals, dumper);
            cur_fileidx = funstackelem.fileidx;
            cur_line = funstackelem.line;
        }
    } else {
        append(sd, "Traceback (most recent call last):\n");  // Python in-joke?
        for (auto [i, funstackelem] : enumerate(fun_id_stack)) {
            auto fileidx = i < fun_id_stack.size() - 1 ? fun_id_stack[i + 1].fileidx : last.fileidx;
            auto line = i < fun_id_stack.size() - 1 ? fun_id_stack[i + 1].line : last.line;
            auto [name, fip] = DumpStackFrameStart(funstackelem.funstartinfo, fileidx, line);
            append(sd, "in function ", name, "\n");
            DumpStackFrame(fip, funstackelem.locals, dumper);
        }
    }

    #ifdef USE_EXCEPTION_HANDLING
    } catch (string &s) {
        // Error happened while we were building this stack trace.
        // That may happen if the reason we're dumping the stack trace is because something got in an
        // inconsistent state in the first place.
        append(sd, "\nRECURSIVE ERROR:\n", s);
    }
    #endif
}

// See also imbind.cpp:DumpStackTrace and VM::DumpStackTrace()
void VM::DumpStackTraceMemory(const string &err) {
    if (fun_id_stack.empty()) {
        // We don't have a stack trace.
        return;
    }
    // We're going to be dumping as much as possible of the memory of the program along with
    // the stack trace. To this end we're using the FlexBuffers dumper which already can deal with
    // cycles etc, so it will output max 1 copy of each data structure.
    ToFlexBufferContext fbc(*this, 1024 * 1024, flexbuffers::BUILDER_FLAG_SHARE_KEYS_AND_STRINGS);
    fbc.cycle_detect = true;
    fbc.max_depth = 64;
    fbc.ignore_unsupported_types = true;
    fbc.save_default_value_fields = true;  // Confusing to have fields missing when debugging.

    #ifdef USE_EXCEPTION_HANDLING
    try {
    #endif

    DumperFun dumper = [&fbc](VM &vm, string_view_nt name, const TypeInfo &ti, Value *x) {
        #if RTT_ENABLED
            auto debug_type = x->type;
        #else
            auto debug_type = ti.t;
        #endif
        if (debug_type == RTT_NIL && !ti.is_nil) {
            // Just skip, only useful in debug.
        } else if (ti.t != debug_type && !RTIsStruct(ti.t)) {
            // Just skip, only useful in debug.
        } else {
            fbc.builder.Key(name.data(), name.size());
            if (RTIsStruct(ti.t)) {
                vm.StructToFlexBuffer(fbc, ti, x, false);
            } else {
                x->ToFlexBuffer(fbc, ti.t, {}, (type_elem_t)0);
            }
        }
    };

    auto vstart = fbc.builder.StartVector();
    fbc.builder.String(err);
    auto cur_fileidx = last.fileidx;
    auto cur_line = last.line;
    for (auto &funstackelem : reverse(fun_id_stack)) {
        auto vstart2 = fbc.builder.StartVector();
        auto [name, fip] = DumpStackFrameStart(funstackelem.funstartinfo, cur_fileidx, cur_line);
        fbc.builder.String(name);
        auto mstart = fbc.builder.StartMap();
        DumpStackFrame(fip, funstackelem.locals, dumper);
        fbc.builder.EndMap(mstart);
        fbc.builder.EndVector(vstart2, false, false);
        cur_fileidx = funstackelem.fileidx;
        cur_line = funstackelem.line;
    }
    fbc.builder.EndVector(vstart, false, false);
    fbc.builder.Finish();
    auto fn = cat("crash_stack_trace_memory_dump_", GetDateTime(), ".flex");
    auto contents =
        string_view((char *)fbc.builder.GetBuffer().data(), fbc.builder.GetBuffer().size());
    if (WriteFile(fn, true, contents, false)) {
        LOG_PROGRAM(fn + " written succesfully");
    } else {
        LOG_ERROR(fn + " failed to write!")
    }

    #ifdef USE_EXCEPTION_HANDLING
    } catch (string &) {
        // Error happened while we were building this stack trace.
        // That may happen if the reason we're dumping the stack trace is because something got in an
        // inconsistent state in the first place.

        // It would be cool to make a heroic effort to still output the information already in the
        // FlexBuffer for debugging, but we can't really tell how to repair the information there-in.
        return;
    }
    #endif
}

Value VM::Error(string err) {
    if (vma.stack_trace_python_ordering && vma.runtime_checks <= RUNTIME_DEBUG) {
        DumpStackTrace(errmsg, true);
        ErrorBase(err);
    } else {
        ErrorBase(err);
        #if LOBSTER_ENGINE
            if (vma.runtime_checks >= RUNTIME_DEBUGGER) {
                BreakPoint(*this, errmsg, vma.runtime_checks >= RUNTIME_DEBUGGER_DUMP);
            } else if (vma.runtime_checks >= RUNTIME_DEBUG_DUMP) {
                DumpStackTraceMemory(err);
            }
        #endif
        DumpStackTrace(errmsg, false);
    }
    UnwindOnError();
    return NilVal();
}

// Unlike Error above, this one does not attempt any variable dumping since the VM may already be
// in an inconsistent state.
Value VM::SeriousError(string err) {
    ErrorBase(err);
    assert(false);
    UnwindOnError();
    return NilVal();
}

Value VM::NormalExit(string err) {
    errmsg = err;
    UnwindOnError();
    return NilVal();
}

void VM::VMAssert(const char *what)  {
    SeriousError(string("VM internal assertion failure: ") + what);
}

void VM::EndEval(StackPtr &, Value ret, const TypeInfo &ti) {
    TerminateWorkers();
    ret.ToString(*this, evalret.first, ti, programprintprefs);
    ret.LTDECTYPE(*this, ti.t);
    for (auto s : constant_strings) s.LTDECRT(*this);
    while (!delete_delay.empty()) {
        auto ro = delete_delay.back();
        delete_delay.pop_back();
        ro->DECDELETENOW(*this);
    }
    if (engine_shutdown) engine_shutdown();
    DumpLeaks();
}

#if VM_USE_LONGJMP && defined(_MSC_VER)
    // Skipping destructors is the whole point here, see UnwindOnError.
    #pragma warning(disable: 4611)  // interaction between setjmp and C++ object destruction
#endif

// An error inside the VM has to get back out to EvalProgram, and there is jitted code in between
// (this is only ever called in JIT mode). Whether an exception can unwind thru that code depends
// on it having unwind info: libtcc registers some on Windows (RtlAddFunctionTable), MIR registers
// none on any platform, and on Linux etc. neither of them does. Where it can't, we longjmp.
bool VM::JitNeedsLongJmp() {
    #if !VM_USE_LONGJMP
        return false;
    #elif defined(_MSC_VER)
        return vma.jit_options.mir;
    #else
        return true;
    #endif
}

#if VM_USE_LONGJMP
static void PrepareLongJmpBuffer(jmp_buf buf) {
    #ifdef _MSC_VER
        // On Windows longjmp normally unwinds the stack, which needs unwind info for every frame
        // in between, and jitted code has none. A null frame pointer makes it restore the saved
        // context directly instead, which is all we want anyway (see UnwindOnError on why we
        // must not run destructors here).
        ((_JUMP_BUFFER *)buf)->Frame = 0;
    #else
        (void)buf;
    #endif
}
#endif

void VM::UnwindOnError() {
    // This is the single location from which we unwind the execution stack from within the VM.
    // This requires special care, because there may be jitted code on the stack, and depending
    // on the platform we can use exception handling, or not.
    // This code is only needed upon error, the regular execution path uses normal returns.
    #if VM_USE_LONGJMP
    if (JitNeedsLongJmp()) {
        // We are in JIT mode, and the code on the stack cannot be unwound thru by an exception.
        // To retain modularity (allow the VM to be used in an environment where a VM error
        // shouldn't terminate the whole app) we try to work around this with setjmp/longjmp.
        // This does NOT call destructors on the way, so code calling into here should make sure
        // to not require these.
        // Though even if there are some, a small memory leak upon a VM error is probably
        // preferable to aborting when modularity is needed.
        // FIXME: audit calling code for destructors. Can we automatically enforce this?
        longjmp(jump_buffer, 1);
        // The corresponding setjmp is right below here.
    }
    #endif
    // Use the standard error mechanism, which uses exceptions (on Windows, or other platforms
    // when not JIT-ing) or aborts (Wasm).
    THROW_OR_ABORT(errmsg);
}

void VM::EvalProgram() {
    #if VM_USE_LONGJMP
    if (JitNeedsLongJmp()) {
        // See longjmp above for why this is needed.
        if (setjmp(jump_buffer)) {
            // Resume normal error now that we've jumped past the C/JIT-ted code.
            THROW_OR_ABORT(errmsg);
        }
        PrepareLongJmpBuffer(jump_buffer);
    }
    #endif
    #if VM_JIT_MODE
        vma.jit_entry(*this, nullptr);
    #else
        compiled_entry_point(*this, nullptr);
    #endif
}

void VM::CallFunctionValue(Value f) {
    auto fv = f.ip();
    fv(*this, nullptr);
}

string VM::ProperTypeName(const TypeInfo &ti) {
    string r;
    switch (ti.t) {
        case RTT_STRUCT_R:
        case RTT_STRUCT_S:
        case RTT_CLASS:
            r = string(ReverseLookupType(ti.structidx));
            break;
        case RTT_VECTOR:
            r = "[" + ProperTypeName(GetTypeInfo(ti.subt)) + "]";
            break;
        case RTT_INT:
            r = ti.enumidx >= 0 ? string(EnumName(ti.enumidx)) : "int";
            break;
        default:
            r = string(BaseTypeName(ti.t));
            break;
    }
    return ti.is_nil ? r + "?" : r;
}

void VM::BCallRetCheck(StackPtr sp, int nfi) {
    #if RTT_ENABLED
        // See if any builtin function is lying about what type it returns
        // other function types return intermediary values that don't correspond to final return
        // values.
        auto nf = vma.nfr.nfuns[nfi];
        for (size_t i = 0; i < nf->retvals.size(); i++) {
            #ifndef NDEBUG
            auto t = (TopPtr(sp) - nf->retvals.size() + i)->type;
            auto u = nf->retvals[i].rttype;
            assert(t == u || u == RTT_INVALID || u == RTT_NIL || (u == RTT_VECTOR && RTIsUDT(t)));
            #endif
        }
    #else
        (void)nfi;
        (void)sp;
    #endif
}

iint VM::GrabIndex(StackPtr &sp, int len) {
    auto &v = TopMR(sp, len);
    for (len--; ; len--) {
        auto sidx = Pop(sp).ival();
        if (!len) return sidx;
        RANGECHECK((*this), sidx, v.vval()->len, v.vval());
        v = v.vval()->AtS(sidx);
    }
}

void VM::AssertFailed(int line, int fileidx, int stringidx) {
    last.line = line;
    last.fileidx = fileidx;
    Error(cat("assertion failed: ", vma.meta->stringtable[stringidx]));
}

void VM::IDXErr(iint i, iint n, const RefObj *v) {
    string sd;
    append(sd, "index ", i, " out of range ", n, " of: ");
    RefToString(*this, sd, v, debugpp);
    Error(sd);
}

void VM::IDXErrS(iint i, iint n) {
    string sd;
    append(sd, "index ", i, " out of range ", n, " of struct");
    Error(sd);
}

string_view VM::StructName(const TypeInfo &ti) {
    return vma.meta->udts[ti.structidx].name;
}

string_view VM::ReverseLookupType(int v) {
    return vma.meta->udts[v].name;
}

string_view VM::LookupField(int stidx, iint fieldn) const {
    auto &st = vma.meta->udts[stidx];
    return st.fields[fieldn].name;
}

string_view VM::LookupFieldByOffset(int stidx, int offset) const {
    auto &st = vma.meta->udts[stidx];
    auto fieldn = st.fields.size() - 1;
    for (flatbuffers::uoffset_t i = 1; i < st.fields.size(); i++) {
        auto foffset = st.fields[i].offset;
        if (foffset < 0) {
            // Generic type that does not have field offsets.
            return "";
        }
        if (offset < foffset) {
            fieldn = i - 1;
            break;
        }
    }
    return st.fields[fieldn].name;
}

int VM::LookupFieldByName(int stidx, string_view fname) const {
    auto &st = vma.meta->udts[stidx];
    for (flatbuffers::uoffset_t i = 0; i < st.fields.size(); i++) {
        auto &f = st.fields[i];
        if (f.name == fname) {
            return f.offset;
        }
    }
    return -1;
}

// Appends the name(s) the value is made up of to sd, or with sd null merely reports whether it
// has any, which is what a caller that only wants to range check a value needs (and it then
// allocates nothing).
bool VM::EnumLookup(string *sd, iint enum_val, int enumidx) {
    auto &enum_def = vma.meta->enums[enumidx];
    auto &vals = enum_def.vals;
    auto lookup = [&](iint val) -> bool {
        // FIXME: can store a bool that says whether this enum is contiguous, so we just index instead.
        for (auto &ev : vals)
            if (ev.val == val) {
                if (sd) *sd += ev.name;
                return true;
            }
        return false;
    };
    if (!enum_def.flags || !enum_val) return lookup(enum_val);
    auto start = sd ? sd->size() : 0;
    auto upto = 64 - HighZeroBits(enum_val);
    for (int i = 0; i < upto; i++) {
        auto bit = enum_val & (1LL << i);
        if (bit) {
            if (sd && sd->size() != start) *sd += "|";
            if (!lookup(bit)) {
                // enum contains unknown bits, so can't display this properly.
                if (sd) sd->resize(start);
                return false;
            }
        }
    }
    return true;
}

bool VM::EnumName(string &sd, iint enum_val, int enumidx) {
    return EnumLookup(&sd, enum_val, enumidx);
}

bool VM::EnumValueValid(iint enum_val, int enumidx) {
    return EnumLookup(nullptr, enum_val, enumidx);
}

string_view VM::EnumName(int enumidx) {
    return vma.meta->enums[enumidx].name;
}

optional<int64_t> VM::LookupEnum(string_view name, int enumidx) {
    auto &vals = vma.meta->enums[enumidx].vals;
    for (auto &ev : vals)
        if (ev.name == name)
            return ev.val;
    return {};
}

void VM::EnsureUDTLookupPopulated() {
    if (!UDTLookup.empty()) return;
    for (auto &udt : vma.meta->udts) {
        auto &v = UDTLookup[udt.name];
        v.push_back(&udt);
    }
}

string_view VM::BuildInfo() {
    return vma.meta->build_info;
}

void VM::StartWorkers(iint numthreads) {
    if (is_worker) Error("workers can\'t start more worker threads");
    if (tuple_space) Error("workers already running");
    // Stop bad values from locking up the machine :)
    // FIXME: if the caller assumes more threads were started, some patterns won't work.
    numthreads = std::min(numthreads, 1024_L64);
    tuple_space = new TupleSpace(vma.meta->udts.size());
    for (iint i = 0; i < numthreads; i++) {
        // Create a new VM that should own all its own memory and be completely independent
        // from this one.
        // We share nfr, metadata and programname for now since they're fully read-only.
        auto vmargs = vma;
        vmargs.program_args.resize(0);
        auto vmalloc = new VMAllocator(std::move(vmargs));
        vmalloc->vm->is_worker = true;
        vmalloc->vm->tuple_space = tuple_space;
        workers.emplace_back([vmalloc] {
            string err;
            #ifdef USE_EXCEPTION_HANDLING
            try
            #endif
            {
                vmalloc->vm->EvalProgram();
            }
            #ifdef USE_EXCEPTION_HANDLING
            catch (string &s) {
                err = s;
            }
            #endif
            delete vmalloc;
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
    if (ti.t != RTT_CLASS) Error("thread write: must be a class");
    auto st = (LObject *)ref;
    vector<uint8_t> buf;
    st->ToLobsterBinary(*this, buf);
    auto &tt = tuple_space->tupletypes[ti.structidx];
    {
        unique_lock<mutex> lock(tt.mtx);
        tt.tuples.emplace_back(std::move(buf));
    }
    tt.condition.notify_one();
}

Value VM::WorkerRead(type_elem_t tti) {
    if (!tuple_space) return NilVal();
    auto &ti = GetTypeInfo(tti);
    if (ti.t != RTT_CLASS) Error("thread read: must be a class type");
    vector<uint8_t> buf;
    auto &tt = tuple_space->tupletypes[ti.structidx];
    {
        unique_lock<mutex> lock(tt.mtx);
        tt.condition.wait(lock, [&] { return !tuple_space->alive || !tt.tuples.empty(); });
        if (!tt.tuples.empty()) {
            buf = std::move(tt.tuples.front());
            tt.tuples.pop_front();
        }
    }
    if (buf.empty()) return NilVal();
    LobsterBinaryParser parser(*this);
    return parser.Parse(tti, buf.data(), buf.data() + buf.size());
}

Value VM::WorkerCheck(type_elem_t tti) {
    if (!tuple_space) return NilVal();
    if (!!tuple_space->alive) return NilVal();
    auto &ti = GetTypeInfo(tti);
    if (ti.t != RTT_CLASS) Error("thread check: must be a class type");
    vector<uint8_t> buf;
    auto &tt = tuple_space->tupletypes[ti.structidx];
    {
        unique_lock<mutex> lock(tt.mtx);
        if (tt.tuples.empty()) return NilVal();
        buf = std::move(tt.tuples.front());
        tt.tuples.pop_front();
    }
    if (buf.empty()) return NilVal();
    LobsterBinaryParser parser(*this);
    return parser.Parse(tti, buf.data(), buf.data() + buf.size());
}

void VM::WorkerWake(type_elem_t tti) {
    if (!tuple_space) return;
    auto &ti = GetTypeInfo(tti);
    if (ti.t != RTT_CLASS) Error("thread check: must be a class type");
    auto &tt = tuple_space->tupletypes[ti.structidx];
    {
        unique_lock<mutex> lock(tt.mtx);
        tt.tuples.emplace_back(vector<uint8_t>{});
    }
    tt.condition.notify_one();
}

}  // namespace lobster

#if LOBSTER_FRAME_PROFILER == 1

thread_local ProfDB prof_db_thread_local;

___tracy_c_zone_context ___tracy_emit_zone_begin(const struct ___tracy_source_location_data *srcloc, int) {
    prof_db_thread_local.stack.push_back({ srcloc, 0.0 });
    return { SecondsSinceStart() };
}

void ___tracy_emit_zone_end(___tracy_c_zone_context ctx) {
    auto &prof_db = prof_db_thread_local;  // Accessing TLS slow, do it just once.
    auto [srcloc, child_time] = prof_db.stack.back();
    prof_db.stack.pop_back();
    if (prof_db.paused) return;
    auto time = SecondsSinceStart() - ctx.start_time;
    auto it = prof_db.stats.find(srcloc);
    ProfStat *el = nullptr;
    if (it == prof_db.stats.end()) {
        el = &prof_db.stats[srcloc];
    } else {
        el = &it->second;
    }
    auto curtime = time - child_time;
    el->n++;
    el->time += curtime;
    el->highest = std::max(curtime, el->highest);
    el->window[prof_db.window_pos] += (float)curtime;
    if (!prof_db.stack.empty()) {
        prof_db.stack.back().second += time;
    }
}

void ProfDB::Advance() {
    if (paused) return;
    window_pos = (window_pos + 1) % PROF_WINDOW_SIZE;
    for (auto &it : stats) {
        it.second.window[window_pos] = 0.0f;
    }
}

#endif

// Make VM ops available as C functions for linking purposes:

extern "C" {

using namespace lobster;

fun_base_t CVM_GetNextCallTarget(VM *vm) {
    return vm->next_call_target;
}

// The generated code calls this before anything else with what its mirrors of our types came out
// as, see CodeGen::Prologue, which catches the C compiler laying one out differently than the C++
// one did. That the C++ types are still what those mirrors were written against is a static_assert
// next to each of them instead.
void CVM_Entry(int value_size, int vmbase_size, int refobj_size, int lvector_size,
               int lvector_elems_off, int lstring_size) {
    auto check = [](bool ok, const char *what) {
        if (ok) return;
        auto err = cat("INTERNAL ERROR: C <-> C++ ", what, " mismatch!");
        // Also printed, since an exception cannot unwind out of jitted code, which is where this
        // is called from, so the message would otherwise not make it out.
        fprintf(stderr, "%s\n", err.c_str());
        THROW_OR_ABORT(err);
    };
    check(value_size == ssizeof<Value>(), "Value size (Debug vs Release?)");
    check(vmbase_size == ssizeof<VMBase>(), "VMBase size");
    check(refobj_size == ssizeof<RefObj>(), "RefObj size");
    check(lvector_size == ssizeof<LVector>() && lvector_elems_off == lvector_elems_offset,
          "LVector layout");
    check(lstring_size == ssizeof<LString>(), "LString size");
}

void CVM_IDXErr(VM *vm, iint i, iint n, RefObj *v) { vm->IDXErr(i, n, v); }

void CVM_SwapVars(VM *vm, int i, StackPtr psp, int off) { SwapVars(*vm, i, psp, off); }
void CVM_BackupVar(VM *vm, int i) { BackupVar(*vm, i); }
void CVM_DecOwned(VM *vm, int i) { DecOwned(*vm, i); }
void CVM_DecDelete(VM *vm, RefObj *ro) { ro->DECDELETE(*vm); }
void CVM_AssertFailed(VM *vm, int line, int fileidx, int stringidx) {
    vm->AssertFailed(line, fileidx, stringidx);
}
void CVM_DecVal(VM *vm, Value v) { DecVal(*vm, v); }
void CVM_RestoreBackup(VM *vm, int i) { RestoreBackup(*vm, i); }
StackPtr CVM_PopArg(VM *vm, int i, StackPtr psp) { return PopArg(*vm, i, psp); }
int CVM_RetSlots(VM *vm) { return RetSlots(*vm); }
int CVM_GetTypeSwitchID(VM *vm, Value self, int vtable_idx) { return GetTypeSwitchID(*vm, self, vtable_idx); }
void CVM_PushFunId(VM *vm, const int *id, StackPtr locals) { PushFunId(*vm, id, locals); }
void CVM_PopFunId(VM *vm) { PopFunId(*vm); }
#if LOBSTER_FRAME_PROFILER
___tracy_c_zone_context CVM_StartProfile(___tracy_source_location_data *tsld) {
    return StartProfile(tsld);
}
void CVM_EndProfile(___tracy_c_zone_context ctx) {
    EndProfile(ctx);
}
#endif

void CVM_UNUSED(VM *vm, StackPtr sp) { U_UNUSED(*vm, sp); }
void CVM_PUSHINT(VM *vm, StackPtr sp, int _a) { U_PUSHINT(*vm, sp, _a); }
void CVM_PUSHINT64(VM *vm, StackPtr sp, int _a, int _b) { U_PUSHINT64(*vm, sp, _a, _b); }
void CVM_PUSHFLT(VM *vm, StackPtr sp, int _a) { U_PUSHFLT(*vm, sp, _a); }
void CVM_PUSHFLT64(VM *vm, StackPtr sp, int _a, int _b) { U_PUSHFLT64(*vm, sp, _a, _b); }
void CVM_PUSHSTR(VM *vm, StackPtr sp, int _a) { U_PUSHSTR(*vm, sp, _a); }
void CVM_PUSHNIL(VM *vm, StackPtr sp) { U_PUSHNIL(*vm, sp); }
void CVM_PUSHVARF(VM *vm, StackPtr sp, int _a) { U_PUSHVARF(*vm, sp, _a); }
void CVM_PUSHVARL(VM *vm, StackPtr sp, int _a) { U_PUSHVARL(*vm, sp, _a); }
void CVM_PUSHVARVF(VM *vm, StackPtr sp, int _a, int _b) { U_PUSHVARVF(*vm, sp, _a, _b); }
void CVM_PUSHVARVL(VM *vm, StackPtr sp, int _a, int _b) { U_PUSHVARVL(*vm, sp, _a, _b); }
void CVM_VPUSHIDXI(VM *vm, StackPtr sp) { U_VPUSHIDXI(*vm, sp); }
void CVM_VPUSHIDXI2V(VM *vm, StackPtr sp) { U_VPUSHIDXI2V(*vm, sp); }
void CVM_VPUSHIDXV(VM *vm, StackPtr sp, int _a) { U_VPUSHIDXV(*vm, sp, _a); }
void CVM_VPUSHIDXIS(VM *vm, StackPtr sp, int _a) { U_VPUSHIDXIS(*vm, sp, _a); }
void CVM_VPUSHIDXIS2V(VM *vm, StackPtr sp, int _a, int _b) { U_VPUSHIDXIS2V(*vm, sp, _a, _b); }
void CVM_VPUSHIDXVS(VM *vm, StackPtr sp, int _a, int _b, int _c) { U_VPUSHIDXVS(*vm, sp, _a, _b, _c); }
void CVM_NPUSHIDXI(VM *vm, StackPtr sp, int _a) { U_NPUSHIDXI(*vm, sp, _a); }
void CVM_SPUSHIDXI(VM *vm, StackPtr sp) { U_SPUSHIDXI(*vm, sp); }
void CVM_PUSHFLD(VM *vm, StackPtr sp, int _a) { U_PUSHFLD(*vm, sp, _a); }
void CVM_PUSHFLDMREF(VM *vm, StackPtr sp, int _a) { U_PUSHFLDMREF(*vm, sp, _a); }
void CVM_PUSHFLDV(VM *vm, StackPtr sp, int _a, int _b) { U_PUSHFLDV(*vm, sp, _a, _b); }
void CVM_PUSHFLD2V(VM *vm, StackPtr sp, int _a, int _b) { U_PUSHFLD2V(*vm, sp, _a, _b); }
void CVM_PUSHFLDV2V(VM *vm, StackPtr sp, int _a, int _b, int _c) { U_PUSHFLDV2V(*vm, sp, _a, _b, _c); }
void CVM_BCALLRETV(VM *vm, StackPtr sp, int _a, int _b) { U_BCALLRETV(*vm, sp, _a, _b); }
void CVM_BCALLRET0(VM *vm, StackPtr sp, int _a, int _b) { U_BCALLRET0(*vm, sp, _a, _b); }
void CVM_BCALLRET1(VM *vm, StackPtr sp, int _a, int _b) { U_BCALLRET1(*vm, sp, _a, _b); }
void CVM_BCALLRET2(VM *vm, StackPtr sp, int _a, int _b) { U_BCALLRET2(*vm, sp, _a, _b); }
void CVM_BCALLRET3(VM *vm, StackPtr sp, int _a, int _b) { U_BCALLRET3(*vm, sp, _a, _b); }
void CVM_BCALLRET4(VM *vm, StackPtr sp, int _a, int _b) { U_BCALLRET4(*vm, sp, _a, _b); }
void CVM_BCALLRET5(VM *vm, StackPtr sp, int _a, int _b) { U_BCALLRET5(*vm, sp, _a, _b); }
void CVM_BCALLRET6(VM *vm, StackPtr sp, int _a, int _b) { U_BCALLRET6(*vm, sp, _a, _b); }
void CVM_BCALLRET7(VM *vm, StackPtr sp, int _a, int _b) { U_BCALLRET7(*vm, sp, _a, _b); }
void CVM_ASSERT(VM *vm, StackPtr sp, int _a, int _b, int _c) { U_ASSERT(*vm, sp, _a, _b, _c); }
void CVM_ASSERTR(VM *vm, StackPtr sp, int _a, int _b, int _c) { U_ASSERTR(*vm, sp, _a, _b, _c); }
void CVM_STATEMENT(VM *vm, StackPtr sp, int _a, int _b) { U_STATEMENT(*vm, sp, _a, _b); }
void CVM_PROFILE(VM *vm, StackPtr sp, int _a) { U_PROFILE(*vm, sp, _a); }
void CVM_NEWVEC(VM *vm, StackPtr sp, int _a, int _b) { U_NEWVEC(*vm, sp, _a, _b); }
void CVM_NEWOBJECT(VM *vm, StackPtr sp, int _a) { U_NEWOBJECT(*vm, sp, _a); }
void CVM_POP(VM *vm, StackPtr sp) { U_POP(*vm, sp); }
void CVM_POPREF(VM *vm, StackPtr sp) { U_POPREF(*vm, sp); }
void CVM_POPV(VM *vm, StackPtr sp, int _a) { U_POPV(*vm, sp, _a); }
void CVM_DUP(VM *vm, StackPtr sp) { U_DUP(*vm, sp); }
void CVM_EXIT(VM *vm, StackPtr sp, int _a) { U_EXIT(*vm, sp, _a); }
void CVM_ABORT(VM *vm, StackPtr sp) { U_ABORT(*vm, sp); }
void CVM_IADD(VM *vm, StackPtr sp) { U_IADD(*vm, sp); }
void CVM_ISUB(VM *vm, StackPtr sp) { U_ISUB(*vm, sp); }
void CVM_IMUL(VM *vm, StackPtr sp) { U_IMUL(*vm, sp); }
void CVM_IDIV(VM *vm, StackPtr sp) { U_IDIV(*vm, sp); }
void CVM_IMOD(VM *vm, StackPtr sp) { U_IMOD(*vm, sp); }
void CVM_ILT(VM *vm, StackPtr sp) { U_ILT(*vm, sp); }
void CVM_IGT(VM *vm, StackPtr sp) { U_IGT(*vm, sp); }
void CVM_ILE(VM *vm, StackPtr sp) { U_ILE(*vm, sp); }
void CVM_IGE(VM *vm, StackPtr sp) { U_IGE(*vm, sp); }
void CVM_IEQ(VM *vm, StackPtr sp) { U_IEQ(*vm, sp); }
void CVM_INE(VM *vm, StackPtr sp) { U_INE(*vm, sp); }
void CVM_FADD(VM *vm, StackPtr sp) { U_FADD(*vm, sp); }
void CVM_FSUB(VM *vm, StackPtr sp) { U_FSUB(*vm, sp); }
void CVM_FMUL(VM *vm, StackPtr sp) { U_FMUL(*vm, sp); }
void CVM_FDIV(VM *vm, StackPtr sp) { U_FDIV(*vm, sp); }
void CVM_FMOD(VM *vm, StackPtr sp) { U_FMOD(*vm, sp); }
void CVM_FLT(VM *vm, StackPtr sp) { U_FLT(*vm, sp); }
void CVM_FGT(VM *vm, StackPtr sp) { U_FGT(*vm, sp); }
void CVM_FLE(VM *vm, StackPtr sp) { U_FLE(*vm, sp); }
void CVM_FGE(VM *vm, StackPtr sp) { U_FGE(*vm, sp); }
void CVM_FEQ(VM *vm, StackPtr sp) { U_FEQ(*vm, sp); }
void CVM_FNE(VM *vm, StackPtr sp) { U_FNE(*vm, sp); }
void CVM_SADD(VM *vm, StackPtr sp) { U_SADD(*vm, sp); }
void CVM_SSUB(VM *vm, StackPtr sp) { U_SSUB(*vm, sp); }
void CVM_SMUL(VM *vm, StackPtr sp) { U_SMUL(*vm, sp); }
void CVM_SDIV(VM *vm, StackPtr sp) { U_SDIV(*vm, sp); }
void CVM_SMOD(VM *vm, StackPtr sp) { U_SMOD(*vm, sp); }
void CVM_SLT(VM *vm, StackPtr sp) { U_SLT(*vm, sp); }
void CVM_SGT(VM *vm, StackPtr sp) { U_SGT(*vm, sp); }
void CVM_SLE(VM *vm, StackPtr sp) { U_SLE(*vm, sp); }
void CVM_SGE(VM *vm, StackPtr sp) { U_SGE(*vm, sp); }
void CVM_SEQ(VM *vm, StackPtr sp) { U_SEQ(*vm, sp); }
void CVM_SNE(VM *vm, StackPtr sp) { U_SNE(*vm, sp); }
void CVM_SADDN(VM *vm, StackPtr sp, int _a) { U_SADDN(*vm, sp, _a); }
void CVM_IVVADD(VM *vm, StackPtr sp, int _a) { U_IVVADD(*vm, sp, _a); }
void CVM_IVVSUB(VM *vm, StackPtr sp, int _a) { U_IVVSUB(*vm, sp, _a); }
void CVM_IVVMUL(VM *vm, StackPtr sp, int _a) { U_IVVMUL(*vm, sp, _a); }
void CVM_IVVDIV(VM *vm, StackPtr sp, int _a) { U_IVVDIV(*vm, sp, _a); }
void CVM_IVVMOD(VM *vm, StackPtr sp, int _a) { U_IVVMOD(*vm, sp, _a); }
void CVM_IVVLT(VM *vm, StackPtr sp, int _a) { U_IVVLT(*vm, sp, _a); }
void CVM_IVVGT(VM *vm, StackPtr sp, int _a) { U_IVVGT(*vm, sp, _a); }
void CVM_IVVLE(VM *vm, StackPtr sp, int _a) { U_IVVLE(*vm, sp, _a); }
void CVM_IVVGE(VM *vm, StackPtr sp, int _a) { U_IVVGE(*vm, sp, _a); }
void CVM_FVVADD(VM *vm, StackPtr sp, int _a) { U_FVVADD(*vm, sp, _a); }
void CVM_FVVSUB(VM *vm, StackPtr sp, int _a) { U_FVVSUB(*vm, sp, _a); }
void CVM_FVVMUL(VM *vm, StackPtr sp, int _a) { U_FVVMUL(*vm, sp, _a); }
void CVM_FVVDIV(VM *vm, StackPtr sp, int _a) { U_FVVDIV(*vm, sp, _a); }
void CVM_FVVMOD(VM *vm, StackPtr sp, int _a) { U_FVVMOD(*vm, sp, _a); }
void CVM_FVVLT(VM *vm, StackPtr sp, int _a) { U_FVVLT(*vm, sp, _a); }
void CVM_FVVGT(VM *vm, StackPtr sp, int _a) { U_FVVGT(*vm, sp, _a); }
void CVM_FVVLE(VM *vm, StackPtr sp, int _a) { U_FVVLE(*vm, sp, _a); }
void CVM_FVVGE(VM *vm, StackPtr sp, int _a) { U_FVVGE(*vm, sp, _a); }
void CVM_IVSADD(VM *vm, StackPtr sp, int _a) { U_IVSADD(*vm, sp, _a); }
void CVM_IVSSUB(VM *vm, StackPtr sp, int _a) { U_IVSSUB(*vm, sp, _a); }
void CVM_IVSMUL(VM *vm, StackPtr sp, int _a) { U_IVSMUL(*vm, sp, _a); }
void CVM_IVSDIV(VM *vm, StackPtr sp, int _a) { U_IVSDIV(*vm, sp, _a); }
void CVM_IVSMOD(VM *vm, StackPtr sp, int _a) { U_IVSMOD(*vm, sp, _a); }
void CVM_IVSLT(VM *vm, StackPtr sp, int _a) { U_IVSLT(*vm, sp, _a); }
void CVM_IVSGT(VM *vm, StackPtr sp, int _a) { U_IVSGT(*vm, sp, _a); }
void CVM_IVSLE(VM *vm, StackPtr sp, int _a) { U_IVSLE(*vm, sp, _a); }
void CVM_IVSGE(VM *vm, StackPtr sp, int _a) { U_IVSGE(*vm, sp, _a); }
void CVM_FVSADD(VM *vm, StackPtr sp, int _a) { U_FVSADD(*vm, sp, _a); }
void CVM_FVSSUB(VM *vm, StackPtr sp, int _a) { U_FVSSUB(*vm, sp, _a); }
void CVM_FVSMUL(VM *vm, StackPtr sp, int _a) { U_FVSMUL(*vm, sp, _a); }
void CVM_FVSDIV(VM *vm, StackPtr sp, int _a) { U_FVSDIV(*vm, sp, _a); }
void CVM_FVSMOD(VM *vm, StackPtr sp, int _a) { U_FVSMOD(*vm, sp, _a); }
void CVM_FVSLT(VM *vm, StackPtr sp, int _a) { U_FVSLT(*vm, sp, _a); }
void CVM_FVSGT(VM *vm, StackPtr sp, int _a) { U_FVSGT(*vm, sp, _a); }
void CVM_FVSLE(VM *vm, StackPtr sp, int _a) { U_FVSLE(*vm, sp, _a); }
void CVM_FVSGE(VM *vm, StackPtr sp, int _a) { U_FVSGE(*vm, sp, _a); }
void CVM_SIVADD(VM *vm, StackPtr sp, int _a) { U_SIVADD(*vm, sp, _a); }
void CVM_SIVSUB(VM *vm, StackPtr sp, int _a) { U_SIVSUB(*vm, sp, _a); }
void CVM_SIVMUL(VM *vm, StackPtr sp, int _a) { U_SIVMUL(*vm, sp, _a); }
void CVM_SIVDIV(VM *vm, StackPtr sp, int _a) { U_SIVDIV(*vm, sp, _a); }
void CVM_SIVMOD(VM *vm, StackPtr sp, int _a) { U_SIVMOD(*vm, sp, _a); }
void CVM_SIVLT(VM *vm, StackPtr sp, int _a) { U_SIVLT(*vm, sp, _a); }
void CVM_SIVGT(VM *vm, StackPtr sp, int _a) { U_SIVGT(*vm, sp, _a); }
void CVM_SIVLE(VM *vm, StackPtr sp, int _a) { U_SIVLE(*vm, sp, _a); }
void CVM_SIVGE(VM *vm, StackPtr sp, int _a) { U_SIVGE(*vm, sp, _a); }
void CVM_SFVADD(VM *vm, StackPtr sp, int _a) { U_SFVADD(*vm, sp, _a); }
void CVM_SFVSUB(VM *vm, StackPtr sp, int _a) { U_SFVSUB(*vm, sp, _a); }
void CVM_SFVMUL(VM *vm, StackPtr sp, int _a) { U_SFVMUL(*vm, sp, _a); }
void CVM_SFVDIV(VM *vm, StackPtr sp, int _a) { U_SFVDIV(*vm, sp, _a); }
void CVM_SFVMOD(VM *vm, StackPtr sp, int _a) { U_SFVMOD(*vm, sp, _a); }
void CVM_SFVLT(VM *vm, StackPtr sp, int _a) { U_SFVLT(*vm, sp, _a); }
void CVM_SFVGT(VM *vm, StackPtr sp, int _a) { U_SFVGT(*vm, sp, _a); }
void CVM_SFVLE(VM *vm, StackPtr sp, int _a) { U_SFVLE(*vm, sp, _a); }
void CVM_SFVGE(VM *vm, StackPtr sp, int _a) { U_SFVGE(*vm, sp, _a); }
void CVM_AEQ(VM *vm, StackPtr sp) { U_AEQ(*vm, sp); }
void CVM_ANE(VM *vm, StackPtr sp) { U_ANE(*vm, sp); }
void CVM_SNEQ(VM *vm, StackPtr sp) { U_SNEQ(*vm, sp); }
void CVM_SNNE(VM *vm, StackPtr sp) { U_SNNE(*vm, sp); }
void CVM_STEQ(VM *vm, StackPtr sp, int _a) { U_STEQ(*vm, sp, _a); }
void CVM_STNE(VM *vm, StackPtr sp, int _a) { U_STNE(*vm, sp, _a); }
void CVM_LEQ(VM *vm, StackPtr sp) { U_LEQ(*vm, sp); }
void CVM_LNE(VM *vm, StackPtr sp) { U_LNE(*vm, sp); }
void CVM_IUMINUS(VM *vm, StackPtr sp) { U_IUMINUS(*vm, sp); }
void CVM_FUMINUS(VM *vm, StackPtr sp) { U_FUMINUS(*vm, sp); }
void CVM_IVUMINUS(VM *vm, StackPtr sp, int _a) { U_IVUMINUS(*vm, sp, _a); }
void CVM_FVUMINUS(VM *vm, StackPtr sp, int _a) { U_FVUMINUS(*vm, sp, _a); }
void CVM_LOGNOT(VM *vm, StackPtr sp) { U_LOGNOT(*vm, sp); }
void CVM_BINAND(VM *vm, StackPtr sp) { U_BINAND(*vm, sp); }
void CVM_BINOR(VM *vm, StackPtr sp) { U_BINOR(*vm, sp); }
void CVM_XOR(VM *vm, StackPtr sp) { U_XOR(*vm, sp); }
void CVM_ASL(VM *vm, StackPtr sp) { U_ASL(*vm, sp); }
void CVM_ASR(VM *vm, StackPtr sp) { U_ASR(*vm, sp); }
void CVM_NEG(VM *vm, StackPtr sp) { U_NEG(*vm, sp); }
void CVM_I2F(VM *vm, StackPtr sp) { U_I2F(*vm, sp); }
void CVM_A2S(VM *vm, StackPtr sp, int _a) { U_A2S(*vm, sp, _a); }
void CVM_E2B(VM *vm, StackPtr sp) { U_E2B(*vm, sp); }
void CVM_E2BREF(VM *vm, StackPtr sp) { U_E2BREF(*vm, sp); }
void CVM_ST2S(VM *vm, StackPtr sp, int _a) { U_ST2S(*vm, sp, _a); }
void CVM_RETURNLOCAL(VM *vm, StackPtr sp, int _a) { U_RETURNLOCAL(*vm, sp, _a); }
void CVM_RETURNNONLOCAL(VM *vm, StackPtr sp, int _a, int _b) { U_RETURNNONLOCAL(*vm, sp, _a, _b); }
void CVM_RETURNANY(VM *vm, StackPtr sp, int _a) { U_RETURNANY(*vm, sp, _a); }
void CVM_ISTYPE(VM *vm, StackPtr sp, int _a, int _b) { U_ISTYPE(*vm, sp, _a, _b); }
void CVM_ISSUBTYPE(VM *vm, StackPtr sp, int _a, int _b, int _c) { U_ISSUBTYPE(*vm, sp, _a, _b, _c); }
void CVM_FORLOOPI(VM *vm, StackPtr sp) { U_FORLOOPI(*vm, sp); }
void CVM_IFORELEM(VM *vm, StackPtr sp) { U_IFORELEM(*vm, sp); }
void CVM_SFORELEM(VM *vm, StackPtr sp) { U_SFORELEM(*vm, sp); }
void CVM_VFORELEM(VM *vm, StackPtr sp) { U_VFORELEM(*vm, sp); }
void CVM_VFORELEMREF(VM *vm, StackPtr sp) { U_VFORELEMREF(*vm, sp); }
void CVM_VFORELEM2S(VM *vm, StackPtr sp) { U_VFORELEM2S(*vm, sp); }
void CVM_VFORELEMREF2S(VM *vm, StackPtr sp, int _a) { U_VFORELEMREF2S(*vm, sp, _a); }
void CVM_INCREF(VM *vm, StackPtr sp, int _a) { U_INCREF(*vm, sp, _a); }
void CVM_KEEPREF(VM *vm, StackPtr sp, int _a, int _b) { U_KEEPREF(*vm, sp, _a, _b); }
void CVM_KEEPREFLOOP(VM *vm, StackPtr sp, int _a, int _b) { U_KEEPREFLOOP(*vm, sp, _a, _b); }
void CVM_GOTOFUNEXIT(VM *vm, StackPtr sp) { U_GOTOFUNEXIT(*vm, sp); }
void CVM_CALL(VM *vm, StackPtr sp, int _a) { U_CALL(*vm, sp, _a); }
void CVM_CALLV(VM *vm, StackPtr sp) { U_CALLV(*vm, sp); }
void CVM_DDCALL(VM *vm, StackPtr sp, int _a, int _b) { U_DDCALL(*vm, sp, _a, _b); }
void CVM_LABEL(VM *vm, StackPtr sp, int _a) { U_LABEL(*vm, sp, _a); }
void CVM_JUMP_TABLE_END(VM *vm, StackPtr sp) { U_JUMP_TABLE_END(*vm, sp); }
void CVM_JUMP_TABLE_CASE_START(VM *vm, StackPtr sp, int _a) { U_JUMP_TABLE_CASE_START(*vm, sp, _a); }
void CVM_ENUM_RANGE_ERR(VM *vm, StackPtr sp) { U_ENUM_RANGE_ERR(*vm, sp); }
Value *CVM_LVAL_VARF(VM *vm, StackPtr sp, Value *lv, int _a) { return U_LVAL_VARF(*vm, sp, lv, _a); }
Value *CVM_LVAL_VARL(VM *vm, StackPtr sp, Value *lv, int _a) { return U_LVAL_VARL(*vm, sp, lv, _a); }
Value *CVM_LVAL_FLD(VM *vm, StackPtr sp, Value *lv, int _a) { return U_LVAL_FLD(*vm, sp, lv, _a); }
Value *CVM_LVAL_IDXVI(VM *vm, StackPtr sp, Value *lv, int _a) { return U_LVAL_IDXVI(*vm, sp, lv, _a); }
Value *CVM_LVAL_IDXVV(VM *vm, StackPtr sp, Value *lv, int _a, int _b) { return U_LVAL_IDXVV(*vm, sp, lv, _a, _b); }
Value *CVM_LVAL_IDXNI(VM *vm, StackPtr sp, Value *lv, int _a) { return U_LVAL_IDXNI(*vm, sp, lv, _a); }
Value *CVM_LVAL_IDXSI(VM *vm, StackPtr sp, Value *lv, int _a, int _b) { return U_LVAL_IDXSI(*vm, sp, lv, _a, _b); }
void CVM_LV_DUP(VM *vm, StackPtr sp, Value *lv) { U_LV_DUP(*vm, sp, lv); }
void CVM_LV_DUPV(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_DUPV(*vm, sp, lv, _a); }
void CVM_LV_WRITE(VM *vm, StackPtr sp, Value *lv) { U_LV_WRITE(*vm, sp, lv); }
void CVM_LV_WRITEREF(VM *vm, StackPtr sp, Value *lv) { U_LV_WRITEREF(*vm, sp, lv); }
void CVM_LV_WRITEV(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_WRITEV(*vm, sp, lv, _a); }
void CVM_LV_WRITEREFV(VM *vm, StackPtr sp, Value *lv, int _a, int _b) { U_LV_WRITEREFV(*vm, sp, lv, _a, _b); }
void CVM_LV_IADD(VM *vm, StackPtr sp, Value *lv) { U_LV_IADD(*vm, sp, lv); }
void CVM_LV_ISUB(VM *vm, StackPtr sp, Value *lv) { U_LV_ISUB(*vm, sp, lv); }
void CVM_LV_IMUL(VM *vm, StackPtr sp, Value *lv) { U_LV_IMUL(*vm, sp, lv); }
void CVM_LV_IDIV(VM *vm, StackPtr sp, Value *lv) { U_LV_IDIV(*vm, sp, lv); }
void CVM_LV_IMOD(VM *vm, StackPtr sp, Value *lv) { U_LV_IMOD(*vm, sp, lv); }
void CVM_LV_BINAND(VM *vm, StackPtr sp, Value *lv) { U_LV_BINAND(*vm, sp, lv); }
void CVM_LV_BINOR(VM *vm, StackPtr sp, Value *lv) { U_LV_BINOR(*vm, sp, lv); }
void CVM_LV_XOR(VM *vm, StackPtr sp, Value *lv) { U_LV_XOR(*vm, sp, lv); }
void CVM_LV_ASL(VM *vm, StackPtr sp, Value *lv) { U_LV_ASL(*vm, sp, lv); }
void CVM_LV_ASR(VM *vm, StackPtr sp, Value *lv) { U_LV_ASR(*vm, sp, lv); }
void CVM_LV_FADD(VM *vm, StackPtr sp, Value *lv) { U_LV_FADD(*vm, sp, lv); }
void CVM_LV_FSUB(VM *vm, StackPtr sp, Value *lv) { U_LV_FSUB(*vm, sp, lv); }
void CVM_LV_FMUL(VM *vm, StackPtr sp, Value *lv) { U_LV_FMUL(*vm, sp, lv); }
void CVM_LV_FDIV(VM *vm, StackPtr sp, Value *lv) { U_LV_FDIV(*vm, sp, lv); }
void CVM_LV_FMOD(VM *vm, StackPtr sp, Value *lv) { U_LV_FMOD(*vm, sp, lv); }
void CVM_LV_IVVADD(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_IVVADD(*vm, sp, lv, _a); }
void CVM_LV_IVVSUB(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_IVVSUB(*vm, sp, lv, _a); }
void CVM_LV_IVVMUL(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_IVVMUL(*vm, sp, lv, _a); }
void CVM_LV_IVVDIV(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_IVVDIV(*vm, sp, lv, _a); }
void CVM_LV_IVVMOD(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_IVVMOD(*vm, sp, lv, _a); }
void CVM_LV_FVVADD(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_FVVADD(*vm, sp, lv, _a); }
void CVM_LV_FVVSUB(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_FVVSUB(*vm, sp, lv, _a); }
void CVM_LV_FVVMUL(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_FVVMUL(*vm, sp, lv, _a); }
void CVM_LV_FVVDIV(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_FVVDIV(*vm, sp, lv, _a); }
void CVM_LV_FVVMOD(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_FVVMOD(*vm, sp, lv, _a); }
void CVM_LV_IVSADD(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_IVSADD(*vm, sp, lv, _a); }
void CVM_LV_IVSSUB(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_IVSSUB(*vm, sp, lv, _a); }
void CVM_LV_IVSMUL(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_IVSMUL(*vm, sp, lv, _a); }
void CVM_LV_IVSDIV(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_IVSDIV(*vm, sp, lv, _a); }
void CVM_LV_IVSMOD(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_IVSMOD(*vm, sp, lv, _a); }
void CVM_LV_FVSADD(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_FVSADD(*vm, sp, lv, _a); }
void CVM_LV_FVSSUB(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_FVSSUB(*vm, sp, lv, _a); }
void CVM_LV_FVSMUL(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_FVSMUL(*vm, sp, lv, _a); }
void CVM_LV_FVSDIV(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_FVSDIV(*vm, sp, lv, _a); }
void CVM_LV_FVSMOD(VM *vm, StackPtr sp, Value *lv, int _a) { U_LV_FVSMOD(*vm, sp, lv, _a); }
void CVM_LV_SADD(VM *vm, StackPtr sp, Value *lv) { U_LV_SADD(*vm, sp, lv); }
void CVM_LV_IPP(VM *vm, StackPtr sp, Value *lv) { U_LV_IPP(*vm, sp, lv); }
void CVM_LV_IMM(VM *vm, StackPtr sp, Value *lv) { U_LV_IMM(*vm, sp, lv); }
void CVM_LV_FPP(VM *vm, StackPtr sp, Value *lv) { U_LV_FPP(*vm, sp, lv); }
void CVM_LV_FMM(VM *vm, StackPtr sp, Value *lv) { U_LV_FMM(*vm, sp, lv); }
void CVM_PUSHFUN(VM *vm, StackPtr sp, int _a, fun_base_t fcont) { U_PUSHFUN(*vm, sp, _a, fcont); }
void CVM_JUMP_TABLE(VM *vm, StackPtr sp, const int *ip) { U_JUMP_TABLE(*vm, sp, ip); }
void CVM_JUMP_TABLE_DISPATCH(VM *vm, StackPtr sp, const int *ip) { U_JUMP_TABLE_DISPATCH(*vm, sp, ip); }
int CVM_JUMP(VM *vm, StackPtr sp) { return U_JUMP(*vm, sp); }
int CVM_JUMPFAIL(VM *vm, StackPtr sp) { return U_JUMPFAIL(*vm, sp); }
int CVM_JUMPFAILR(VM *vm, StackPtr sp) { return U_JUMPFAILR(*vm, sp); }
int CVM_JUMPNOFAIL(VM *vm, StackPtr sp) { return U_JUMPNOFAIL(*vm, sp); }
int CVM_JUMPNOFAILR(VM *vm, StackPtr sp) { return U_JUMPNOFAILR(*vm, sp); }
int CVM_IFOR(VM *vm, StackPtr sp) { return U_IFOR(*vm, sp); }
int CVM_SFOR(VM *vm, StackPtr sp) { return U_SFOR(*vm, sp); }
int CVM_VFOR(VM *vm, StackPtr sp) { return U_VFOR(*vm, sp); }
int CVM_JUMPIFUNWOUND(VM *vm, StackPtr sp, int df) { return U_JUMPIFUNWOUND(*vm, sp, df); }
int CVM_JUMPIFSTATICLF(VM *vm, StackPtr sp, int df) { return U_JUMPIFSTATICLF(*vm, sp, df); }
int CVM_JUMPIFMEMBERLF(VM *vm, StackPtr sp, int df) { return U_JUMPIFMEMBERLF(*vm, sp, df); }

#if VM_JIT_MODE

#if LOBSTER_ENGINE
extern "C" void GLFrame(StackPtr sp, VM & vm);
#endif

const void *vm_ops_jit_table[] = {
    "U_UNUSED", (void *)&CVM_UNUSED,
    "U_PUSHINT", (void *)&CVM_PUSHINT,
    "U_PUSHINT64", (void *)&CVM_PUSHINT64,
    "U_PUSHFLT", (void *)&CVM_PUSHFLT,
    "U_PUSHFLT64", (void *)&CVM_PUSHFLT64,
    "U_PUSHSTR", (void *)&CVM_PUSHSTR,
    "U_PUSHNIL", (void *)&CVM_PUSHNIL,
    "U_PUSHVARF", (void *)&CVM_PUSHVARF,
    "U_PUSHVARL", (void *)&CVM_PUSHVARL,
    "U_PUSHVARVF", (void *)&CVM_PUSHVARVF,
    "U_PUSHVARVL", (void *)&CVM_PUSHVARVL,
    "U_VPUSHIDXI", (void *)&CVM_VPUSHIDXI,
    "U_VPUSHIDXI2V", (void *)&CVM_VPUSHIDXI2V,
    "U_VPUSHIDXV", (void *)&CVM_VPUSHIDXV,
    "U_VPUSHIDXIS", (void *)&CVM_VPUSHIDXIS,
    "U_VPUSHIDXIS2V", (void *)&CVM_VPUSHIDXIS2V,
    "U_VPUSHIDXVS", (void *)&CVM_VPUSHIDXVS,
    "U_NPUSHIDXI", (void *)&CVM_NPUSHIDXI,
    "U_SPUSHIDXI", (void *)&CVM_SPUSHIDXI,
    "U_PUSHFLD", (void *)&CVM_PUSHFLD,
    "U_PUSHFLDMREF", (void *)&CVM_PUSHFLDMREF,
    "U_PUSHFLDV", (void *)&CVM_PUSHFLDV,
    "U_PUSHFLD2V", (void *)&CVM_PUSHFLD2V,
    "U_PUSHFLDV2V", (void *)&CVM_PUSHFLDV2V,
    "U_BCALLRETV", (void *)&CVM_BCALLRETV,
    "U_BCALLRET0", (void *)&CVM_BCALLRET0,
    "U_BCALLRET1", (void *)&CVM_BCALLRET1,
    "U_BCALLRET2", (void *)&CVM_BCALLRET2,
    "U_BCALLRET3", (void *)&CVM_BCALLRET3,
    "U_BCALLRET4", (void *)&CVM_BCALLRET4,
    "U_BCALLRET5", (void *)&CVM_BCALLRET5,
    "U_BCALLRET6", (void *)&CVM_BCALLRET6,
    "U_BCALLRET7", (void *)&CVM_BCALLRET7,
    "U_ASSERT", (void *)&CVM_ASSERT,
    "U_ASSERTR", (void *)&CVM_ASSERTR,
    "U_STATEMENT", (void *)&CVM_STATEMENT,
    "U_PROFILE", (void *)&CVM_PROFILE,
    "U_NEWVEC", (void *)&CVM_NEWVEC,
    "U_NEWOBJECT", (void *)&CVM_NEWOBJECT,
    "U_POP", (void *)&CVM_POP,
    "U_POPREF", (void *)&CVM_POPREF,
    "U_POPV", (void *)&CVM_POPV,
    "U_DUP", (void *)&CVM_DUP,
    "U_EXIT", (void *)&CVM_EXIT,
    "U_ABORT", (void *)&CVM_ABORT,
    "U_IADD", (void *)&CVM_IADD,
    "U_ISUB", (void *)&CVM_ISUB,
    "U_IMUL", (void *)&CVM_IMUL,
    "U_IDIV", (void *)&CVM_IDIV,
    "U_IMOD", (void *)&CVM_IMOD,
    "U_ILT", (void *)&CVM_ILT,
    "U_IGT", (void *)&CVM_IGT,
    "U_ILE", (void *)&CVM_ILE,
    "U_IGE", (void *)&CVM_IGE,
    "U_IEQ", (void *)&CVM_IEQ,
    "U_INE", (void *)&CVM_INE,
    "U_FADD", (void *)&CVM_FADD,
    "U_FSUB", (void *)&CVM_FSUB,
    "U_FMUL", (void *)&CVM_FMUL,
    "U_FDIV", (void *)&CVM_FDIV,
    "U_FMOD", (void *)&CVM_FMOD,
    "U_FLT", (void *)&CVM_FLT,
    "U_FGT", (void *)&CVM_FGT,
    "U_FLE", (void *)&CVM_FLE,
    "U_FGE", (void *)&CVM_FGE,
    "U_FEQ", (void *)&CVM_FEQ,
    "U_FNE", (void *)&CVM_FNE,
    "U_SADD", (void *)&CVM_SADD,
    "U_SSUB", (void *)&CVM_SSUB,
    "U_SMUL", (void *)&CVM_SMUL,
    "U_SDIV", (void *)&CVM_SDIV,
    "U_SMOD", (void *)&CVM_SMOD,
    "U_SLT", (void *)&CVM_SLT,
    "U_SGT", (void *)&CVM_SGT,
    "U_SLE", (void *)&CVM_SLE,
    "U_SGE", (void *)&CVM_SGE,
    "U_SEQ", (void *)&CVM_SEQ,
    "U_SNE", (void *)&CVM_SNE,
    "U_SADDN", (void *)&CVM_SADDN,
    "U_IVVADD", (void *)&CVM_IVVADD,
    "U_IVVSUB", (void *)&CVM_IVVSUB,
    "U_IVVMUL", (void *)&CVM_IVVMUL,
    "U_IVVDIV", (void *)&CVM_IVVDIV,
    "U_IVVMOD", (void *)&CVM_IVVMOD,
    "U_IVVLT", (void *)&CVM_IVVLT,
    "U_IVVGT", (void *)&CVM_IVVGT,
    "U_IVVLE", (void *)&CVM_IVVLE,
    "U_IVVGE", (void *)&CVM_IVVGE,
    "U_FVVADD", (void *)&CVM_FVVADD,
    "U_FVVSUB", (void *)&CVM_FVVSUB,
    "U_FVVMUL", (void *)&CVM_FVVMUL,
    "U_FVVDIV", (void *)&CVM_FVVDIV,
    "U_FVVMOD", (void *)&CVM_FVVMOD,
    "U_FVVLT", (void *)&CVM_FVVLT,
    "U_FVVGT", (void *)&CVM_FVVGT,
    "U_FVVLE", (void *)&CVM_FVVLE,
    "U_FVVGE", (void *)&CVM_FVVGE,
    "U_IVSADD", (void *)&CVM_IVSADD,
    "U_IVSSUB", (void *)&CVM_IVSSUB,
    "U_IVSMUL", (void *)&CVM_IVSMUL,
    "U_IVSDIV", (void *)&CVM_IVSDIV,
    "U_IVSMOD", (void *)&CVM_IVSMOD,
    "U_IVSLT", (void *)&CVM_IVSLT,
    "U_IVSGT", (void *)&CVM_IVSGT,
    "U_IVSLE", (void *)&CVM_IVSLE,
    "U_IVSGE", (void *)&CVM_IVSGE,
    "U_FVSADD", (void *)&CVM_FVSADD,
    "U_FVSSUB", (void *)&CVM_FVSSUB,
    "U_FVSMUL", (void *)&CVM_FVSMUL,
    "U_FVSDIV", (void *)&CVM_FVSDIV,
    "U_FVSMOD", (void *)&CVM_FVSMOD,
    "U_FVSLT", (void *)&CVM_FVSLT,
    "U_FVSGT", (void *)&CVM_FVSGT,
    "U_FVSLE", (void *)&CVM_FVSLE,
    "U_FVSGE", (void *)&CVM_FVSGE,
    "U_SIVADD", (void *)&CVM_SIVADD,
    "U_SIVSUB", (void *)&CVM_SIVSUB,
    "U_SIVMUL", (void *)&CVM_SIVMUL,
    "U_SIVDIV", (void *)&CVM_SIVDIV,
    "U_SIVMOD", (void *)&CVM_SIVMOD,
    "U_SIVLT", (void *)&CVM_SIVLT,
    "U_SIVGT", (void *)&CVM_SIVGT,
    "U_SIVLE", (void *)&CVM_SIVLE,
    "U_SIVGE", (void *)&CVM_SIVGE,
    "U_SFVADD", (void *)&CVM_SFVADD,
    "U_SFVSUB", (void *)&CVM_SFVSUB,
    "U_SFVMUL", (void *)&CVM_SFVMUL,
    "U_SFVDIV", (void *)&CVM_SFVDIV,
    "U_SFVMOD", (void *)&CVM_SFVMOD,
    "U_SFVLT", (void *)&CVM_SFVLT,
    "U_SFVGT", (void *)&CVM_SFVGT,
    "U_SFVLE", (void *)&CVM_SFVLE,
    "U_SFVGE", (void *)&CVM_SFVGE,
    "U_AEQ", (void *)&CVM_AEQ,
    "U_ANE", (void *)&CVM_ANE,
    "U_SNEQ", (void *)&CVM_SNEQ,
    "U_SNNE", (void *)&CVM_SNNE,
    "U_STEQ", (void *)&CVM_STEQ,
    "U_STNE", (void *)&CVM_STNE,
    "U_LEQ", (void *)&CVM_LEQ,
    "U_LNE", (void *)&CVM_LNE,
    "U_IUMINUS", (void *)&CVM_IUMINUS,
    "U_FUMINUS", (void *)&CVM_FUMINUS,
    "U_IVUMINUS", (void *)&CVM_IVUMINUS,
    "U_FVUMINUS", (void *)&CVM_FVUMINUS,
    "U_LOGNOT", (void *)&CVM_LOGNOT,
    "U_BINAND", (void *)&CVM_BINAND,
    "U_BINOR", (void *)&CVM_BINOR,
    "U_XOR", (void *)&CVM_XOR,
    "U_ASL", (void *)&CVM_ASL,
    "U_ASR", (void *)&CVM_ASR,
    "U_NEG", (void *)&CVM_NEG,
    "U_I2F", (void *)&CVM_I2F,
    "U_A2S", (void *)&CVM_A2S,
    "U_E2B", (void *)&CVM_E2B,
    "U_E2BREF", (void *)&CVM_E2BREF,
    "U_ST2S", (void *)&CVM_ST2S,
    "U_RETURNLOCAL", (void *)&CVM_RETURNLOCAL,
    "U_RETURNNONLOCAL", (void *)&CVM_RETURNNONLOCAL,
    "U_RETURNANY", (void *)&CVM_RETURNANY,
    "U_ISTYPE", (void *)&CVM_ISTYPE,
    "U_ISSUBTYPE", (void *)&CVM_ISSUBTYPE,
    "U_FORLOOPI", (void *)&CVM_FORLOOPI,
    "U_IFORELEM", (void *)&CVM_IFORELEM,
    "U_SFORELEM", (void *)&CVM_SFORELEM,
    "U_VFORELEM", (void *)&CVM_VFORELEM,
    "U_VFORELEMREF", (void *)&CVM_VFORELEMREF,
    "U_VFORELEM2S", (void *)&CVM_VFORELEM2S,
    "U_VFORELEMREF2S", (void *)&CVM_VFORELEMREF2S,
    "U_INCREF", (void *)&CVM_INCREF,
    "U_KEEPREF", (void *)&CVM_KEEPREF,
    "U_KEEPREFLOOP", (void *)&CVM_KEEPREFLOOP,
    "U_GOTOFUNEXIT", (void *)&CVM_GOTOFUNEXIT,
    "U_CALL", (void *)&CVM_CALL,
    "U_CALLV", (void *)&CVM_CALLV,
    "U_DDCALL", (void *)&CVM_DDCALL,
    "U_LABEL", (void *)&CVM_LABEL,
    "U_JUMP_TABLE_END", (void *)&CVM_JUMP_TABLE_END,
    "U_JUMP_TABLE_CASE_START", (void *)&CVM_JUMP_TABLE_CASE_START,
    "U_ENUM_RANGE_ERR", (void *)&CVM_ENUM_RANGE_ERR,
    "U_LVAL_VARF", (void *)&CVM_LVAL_VARF,
    "U_LVAL_VARL", (void *)&CVM_LVAL_VARL,
    "U_LVAL_FLD", (void *)&CVM_LVAL_FLD,
    "U_LVAL_IDXVI", (void *)&CVM_LVAL_IDXVI,
    "U_LVAL_IDXVV", (void *)&CVM_LVAL_IDXVV,
    "U_LVAL_IDXNI", (void *)&CVM_LVAL_IDXNI,
    "U_LVAL_IDXSI", (void *)&CVM_LVAL_IDXSI,
    "U_LV_DUP", (void *)&CVM_LV_DUP,
    "U_LV_DUPV", (void *)&CVM_LV_DUPV,
    "U_LV_WRITE", (void *)&CVM_LV_WRITE,
    "U_LV_WRITEREF", (void *)&CVM_LV_WRITEREF,
    "U_LV_WRITEV", (void *)&CVM_LV_WRITEV,
    "U_LV_WRITEREFV", (void *)&CVM_LV_WRITEREFV,
    "U_LV_IADD", (void *)&CVM_LV_IADD,
    "U_LV_ISUB", (void *)&CVM_LV_ISUB,
    "U_LV_IMUL", (void *)&CVM_LV_IMUL,
    "U_LV_IDIV", (void *)&CVM_LV_IDIV,
    "U_LV_IMOD", (void *)&CVM_LV_IMOD,
    "U_LV_BINAND", (void *)&CVM_LV_BINAND,
    "U_LV_BINOR", (void *)&CVM_LV_BINOR,
    "U_LV_XOR", (void *)&CVM_LV_XOR,
    "U_LV_ASL", (void *)&CVM_LV_ASL,
    "U_LV_ASR", (void *)&CVM_LV_ASR,
    "U_LV_FADD", (void *)&CVM_LV_FADD,
    "U_LV_FSUB", (void *)&CVM_LV_FSUB,
    "U_LV_FMUL", (void *)&CVM_LV_FMUL,
    "U_LV_FDIV", (void *)&CVM_LV_FDIV,
    "U_LV_FMOD", (void *)&CVM_LV_FMOD,
    "U_LV_IVVADD", (void *)&CVM_LV_IVVADD,
    "U_LV_IVVSUB", (void *)&CVM_LV_IVVSUB,
    "U_LV_IVVMUL", (void *)&CVM_LV_IVVMUL,
    "U_LV_IVVDIV", (void *)&CVM_LV_IVVDIV,
    "U_LV_IVVMOD", (void *)&CVM_LV_IVVMOD,
    "U_LV_FVVADD", (void *)&CVM_LV_FVVADD,
    "U_LV_FVVSUB", (void *)&CVM_LV_FVVSUB,
    "U_LV_FVVMUL", (void *)&CVM_LV_FVVMUL,
    "U_LV_FVVDIV", (void *)&CVM_LV_FVVDIV,
    "U_LV_FVVMOD", (void *)&CVM_LV_FVVMOD,
    "U_LV_IVSADD", (void *)&CVM_LV_IVSADD,
    "U_LV_IVSSUB", (void *)&CVM_LV_IVSSUB,
    "U_LV_IVSMUL", (void *)&CVM_LV_IVSMUL,
    "U_LV_IVSDIV", (void *)&CVM_LV_IVSDIV,
    "U_LV_IVSMOD", (void *)&CVM_LV_IVSMOD,
    "U_LV_FVSADD", (void *)&CVM_LV_FVSADD,
    "U_LV_FVSSUB", (void *)&CVM_LV_FVSSUB,
    "U_LV_FVSMUL", (void *)&CVM_LV_FVSMUL,
    "U_LV_FVSDIV", (void *)&CVM_LV_FVSDIV,
    "U_LV_FVSMOD", (void *)&CVM_LV_FVSMOD,
    "U_LV_SADD", (void *)&CVM_LV_SADD,
    "U_LV_IPP", (void *)&CVM_LV_IPP,
    "U_LV_IMM", (void *)&CVM_LV_IMM,
    "U_LV_FPP", (void *)&CVM_LV_FPP,
    "U_LV_FMM", (void *)&CVM_LV_FMM,
    "U_PUSHFUN", (void *)&CVM_PUSHFUN,
    "U_JUMP_TABLE", (void *)&CVM_JUMP_TABLE,
    "U_JUMP_TABLE_DISPATCH", (void *)&CVM_JUMP_TABLE_DISPATCH,
    "U_JUMP", (void *)&CVM_JUMP,
    "U_JUMPFAIL", (void *)&CVM_JUMPFAIL,
    "U_JUMPFAILR", (void *)&CVM_JUMPFAILR,
    "U_JUMPNOFAIL", (void *)&CVM_JUMPNOFAIL,
    "U_JUMPNOFAILR", (void *)&CVM_JUMPNOFAILR,
    "U_IFOR", (void *)&CVM_IFOR,
    "U_SFOR", (void *)&CVM_SFOR,
    "U_VFOR", (void *)&CVM_VFOR,
    "U_JUMPIFUNWOUND", (void *)&CVM_JUMPIFUNWOUND,
    "U_JUMPIFSTATICLF", (void *)&CVM_JUMPIFSTATICLF,
    "U_JUMPIFMEMBERLF", (void *)&CVM_JUMPIFMEMBERLF,
    "GetNextCallTarget", (void *)CVM_GetNextCallTarget,
    "Entry", (void *)CVM_Entry,
    "IDXErr", (void *)CVM_IDXErr,
    "SwapVars", (void *)CVM_SwapVars,
    "BackupVar", (void *)CVM_BackupVar,
    "DecOwned", (void *)CVM_DecOwned,
    "DecDelete", (void *)CVM_DecDelete,
    "AssertFailed", (void *)CVM_AssertFailed,
    "DecVal", (void *)CVM_DecVal,
    "RestoreBackup", (void *)CVM_RestoreBackup,
    "PopArg", (void *)CVM_PopArg,
    "RetSlots", (void *)CVM_RetSlots,
    "GetTypeSwitchID", (void *)CVM_GetTypeSwitchID,
    "PushFunId", (void *)CVM_PushFunId,
    "PopFunId", (void *)CVM_PopFunId,
    #if LOBSTER_ENGINE
    "GLFrame", (void *)GLFrame,
    #endif
    #if LOBSTER_FRAME_PROFILER
    "StartProfile", (void *)CVM_StartProfile,
    "EndProfile", (void *)CVM_EndProfile,
    #endif
    0, 0
};
#endif

}  // extern "C"

