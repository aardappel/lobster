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

#include "lobster/il.h"

#include "lobster/vmops.h"

#include "lobster/lobsterreader.h"

namespace lobster {

#if LOBSTER_FRAME_PROFILER_GLOBAL
// This is a global, so doesn't work well with multithreading :)
// It's only here to debug difficult crashes on platforms without a stack trace.
vector<___tracy_source_location_data> g_function_locations;
vector<tracy::SourceLocationData> g_builtin_locations;
#endif

VM::VM(VMArgs &&vmargs)
    : vma(std::move(vmargs)) {

    typetable = vma.meta->type_table.data();
    constant_strings.resize(vma.meta->stringtable.size());
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
                case V_VALUEBUF:
                    break;
                case V_STRING:
                case V_RESOURCE:
                case V_VECTOR:
                case V_CLASS: {
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
    pool.printstats(false);
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
            case V_VALUEBUF:
                break;
            case V_STRING:
                stats[&ti].Add(((LString *)ro)->MemoryUsage());
                break;
            case V_RESOURCE:
                stats[((LResource *)ro)->type].Add(((LResource *)ro)->MemoryUsage(),
                                                   ((LResource *)ro)->type);
                break;
            case V_VECTOR:
                stats[&ti].Add(((LVector *)ro)->MemoryUsage());
                break;
            case V_CLASS:
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
    append(sd, "TOTAL: ", total / 1024, " K (", totalgpu * 100 / total, "% on GPU)\n");
    for (auto [i, p] : enumerate(sorted)) {
        if (i >= show_max || p.second.bytes < 1024) break;
        if (p.second.rt) append(sd, "resource<", p.second.rt->name, ">");
        else append(sd, ((const TypeInfo *)p.first)->Debug(*this, false));
        append(sd, ": ", p.second.bytes / 1024, " K in ", p.second.num, " objects");
        if (p.second.max >= 1024 && p.second.max != p.second.bytes / p.second.num) {
            append(sd, " (biggest: ", p.second.max / 1024, " K)");
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
    auto width = IsStruct(ti.t) ? ti.len : 1;
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
        if (debug_type == V_NIL && ti.t != V_NIL) {
            // Uninitialized.
            append(sd, ":");
            ti.Print(vm, sd, nullptr);
            append(sd, " (uninitialized)");
        } else if (ti.t != debug_type && !IsStruct(ti.t)) {
            // Some runtime type corruption, show the problem rather than crashing.
            append(sd, ":");
            ti.Print(vm, sd, nullptr);
            append(sd, " ERROR != ", BaseTypeName(debug_type));
        } else {
            append(sd, " = ");
            PrintPrefs minipp { 1, 20, true, -1 };
            if (IsStruct(ti.t)) {
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

    #ifdef USE_EXCEPTION_HANDLING
    try {
    #endif

    DumperFun dumper = [&fbc](VM &vm, string_view_nt name, const TypeInfo &ti, Value *x) {
        #if RTT_ENABLED
            auto debug_type = x->type;
        #else
            auto debug_type = ti.t;
        #endif
        if (debug_type == V_NIL && ti.t != V_NIL) {
            // Just skip, only useful in debug.
        } else if (ti.t != debug_type && !IsStruct(ti.t)) {
            // Just skip, only useful in debug.
        } else {
            fbc.builder.Key(name.data(), name.size());
            if (IsStruct(ti.t)) {
                vm.StructToFlexBuffer(fbc, ti, x, false);
            } else {
                x->ToFlexBuffer(fbc, ti.t, {}, -1);
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
                BreakPoint(*this, errmsg);
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
    for (auto s : constant_strings) {
        if (s) s->Dec(*this);
    }
    while (!delete_delay.empty()) {
        auto ro = delete_delay.back();
        delete_delay.pop_back();
        ro->DECDELETENOW(*this);
    }
    DumpLeaks();
}

void VM::UnwindOnError() {
    // This is the single location from which we unwind the execution stack from within the VM.
    // This requires special care, because there may be jitted code on the stack, and depending
    // on the platform we can use exception handling, or not.
    // This code is only needed upon error, the regular execution path uses normal returns.
    #if VM_USE_LONGJMP
        // We are in JIT mode, and on a platform that cannot throw exceptions "thru" C code,
        // e.g. Linux.
        // To retain modularity (allow the VM to be used in an environment where a VM error
        // shouldn't terminate the whole app) we try to work around this with setjmp/longjmp.
        // This does NOT call destructors on the way, so code calling into here should make sure
        // to not require these.
        // Though even if there are some, a small memory leak upon a VM error is probably
        // preferable to aborting when modularity is needed.
        // FIXME: audit calling code for destructors. Can we automatically enforce this?
        longjmp(jump_buffer, 1);
        // The corresponding setjmp is right below here.
    #else
        // Use the standard error mechanism, which uses exceptions (on Windows, or other platforms
        // when not JIT-ing) or aborts (Wasm).
        THROW_OR_ABORT(errmsg);
    #endif
}

void VM::EvalProgram() {
    #if VM_USE_LONGJMP
        // See longjmp above for why this is needed.
        if (setjmp(jump_buffer)) {
            // Resume normal error now that we've jumped past the C/JIT-ted code.
            THROW_OR_ABORT(errmsg);
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

void VM::BCallRetCheck(StackPtr sp, const NativeFun *nf) {
    #if RTT_ENABLED
        // See if any builtin function is lying about what type it returns
        // other function types return intermediary values that don't correspond to final return
        // values.
        for (size_t i = 0; i < nf->retvals.size(); i++) {
            #ifndef NDEBUG
            auto t = (TopPtr(sp) - nf->retvals.size() + i)->type;
            auto u = nf->retvals[i].type->t;
            assert(t == u || u == V_ANY || u == V_NIL || (u == V_VECTOR && IsUDT(t)));
            #endif
        }
    #else
        (void)nf;
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

void VM::IDXErr(iint i, iint n, const RefObj *v) {
    string sd;
    append(sd, "index ", i, " out of range ", n, " of: ");
    RefToString(*this, sd, v, debugpp);
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

bool VM::EnumName(string &sd, iint enum_val, int enumidx) {
    auto &enum_def = vma.meta->enums[enumidx];
    auto &vals = enum_def.vals;
    auto lookup = [&](iint val) -> bool {
        // FIXME: can store a bool that says whether this enum is contiguous, so we just index instead.
        for (auto &ev : vals)
            if (ev.val == val) {
                sd += ev.name;
                return true;
            }
        return false;
    };
    if (!enum_def.flags || !enum_val) return lookup(enum_val);
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
    numthreads = std::min(numthreads, 256_L64);
    tuple_space = new TupleSpace(vma.meta->udts.size());
    for (iint i = 0; i < numthreads; i++) {
        // Create a new VM that should own all its own memory and be completely independent
        // from this one.
        // We share nfr, metadata and programname for now since they're fully read-only.
        auto vmargs = *(VMArgs *)this;
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
    if (ti.t != V_CLASS) Error("thread write: must be a class");
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
    if (ti.t != V_CLASS) Error("thread read: must be a class type");
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
    auto &ti = GetTypeInfo(tti);
    if (ti.t != V_CLASS) Error("thread check: must be a class type");
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

}  // namespace lobster

#if LOBSTER_FRAME_PROFILER == 1

vector<pair<const struct ___tracy_source_location_data *, double>> prof_stack;
unordered_map<const struct ___tracy_source_location_data *, ProfStat> prof_stats;

___tracy_c_zone_context ___tracy_emit_zone_begin(const struct ___tracy_source_location_data *srcloc, int) {
    prof_stack.push_back({ srcloc, 0.0 });
    return { SecondsSinceStart() };
}

void ___tracy_emit_zone_end(___tracy_c_zone_context ctx) {
    auto [srcloc, child_time] = prof_stack.back();
    prof_stack.pop_back();
    auto time = SecondsSinceStart() - ctx.start_time;
    auto it = prof_stats.find(srcloc);
    if (it == prof_stats.end()) {
        prof_stats[srcloc] = ProfStat{ time - child_time };
    } else {
        it->second.time += time - child_time;
    }
    if (!prof_stack.empty()) {
        prof_stack.back().second += time;
    }
}

#endif

// Make VM ops available as C functions for linking purposes:

extern "C" {

using namespace lobster;

fun_base_t CVM_GetNextCallTarget(VM *vm) {
    return vm->next_call_target;
}

void CVM_Entry(int value_size, int vmbase_size) {
    if (value_size != sizeof(Value)) {
        THROW_OR_ABORT("INTERNAL ERROR: C <-> C++ Value size mismatch! (Debug vs Release?)");
    }
    if (vmbase_size != sizeof(VMBase)) {
        THROW_OR_ABORT("INTERNAL ERROR: C <-> C++ VMBase size mismatch!");
    }
}

void CVM_SwapVars(VM *vm, int i, StackPtr psp, int off) { SwapVars(*vm, i, psp, off); }
void CVM_BackupVar(VM *vm, int i) { BackupVar(*vm, i); }
void CVM_DecOwned(VM *vm, int i) { DecOwned(*vm, i); }
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

#define F(N, A, USE, DEF) \
    void CVM_##N(VM *vm, StackPtr sp VM_COMMA_IF(A) VM_OP_ARGSN(A)) { \
        return U_##N(*vm, sp VM_COMMA_IF(A) VM_OP_PASSN(A));              \
    }
ILBASENAMES
#undef F
#define F(N, A, USE, DEF) \
    void CVM_##N(VM *vm, StackPtr sp VM_COMMA_IF(A) VM_OP_ARGSN(A), fun_base_t fcont) { \
        return U_##N(*vm, sp, VM_OP_PASSN(A) VM_COMMA_IF(A) fcont);                         \
    }
ILCALLNAMES
#undef F
#define F(N, A, USE, DEF) \
    void CVM_##N(VM *vm, StackPtr sp VM_COMMA_IF(A) VM_OP_ARGSN(A)) { \
        return U_##N(*vm, sp VM_COMMA_IF(A) VM_OP_PASSN(A));              \
    }
ILVARARGNAMES
#undef F
#define F(N, A, USE, DEF) \
    int CVM_##N(VM *vm, StackPtr sp) {                     \
        return U_##N(*vm, sp);                             \
    }
ILJUMPNAMES1
#undef F
#define F(N, A, USE, DEF) \
    int CVM_##N(VM *vm, StackPtr sp, int df) {                           \
        return U_##N(*vm, sp, df);                                       \
    }
ILJUMPNAMES2
#undef F

#if VM_JIT_MODE

#if LOBSTER_ENGINE
extern "C" void GLFrame(StackPtr sp, VM & vm);
#endif

const void *vm_ops_jit_table[] = {
    #define F(N, A, USE, DEF) "U_" #N, (void *)&CVM_##N,
        ILNAMES
    #undef F
    "GetNextCallTarget", (void *)CVM_GetNextCallTarget,
    "Entry", (void *)CVM_Entry,
    "SwapVars", (void *)CVM_SwapVars,
    "BackupVar", (void *)CVM_BackupVar,
    "DecOwned", (void *)CVM_DecOwned,
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

