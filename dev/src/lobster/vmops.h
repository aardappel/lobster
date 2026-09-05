#include "lobster/geom.h"
#include "lobster/vmdata.h"
#include "lobster/natreg.h"
#include "lobster/bytecode_generated.h"

extern string BreakPoint(lobster::VM &vm, string_view reason, bool also_flex_dump);

namespace lobster {

// The helpers the generated code calls. The C++ backend calls them directly, the C one thru the
// wrappers in vm.cpp, see vm_ops_jit_table there and CodeGen::Prologue for how the C side
// declares them. They take their operands by value and return their result, except for the
// ones that work on a run of values, like a constructor, which take a pointer to those, and
// the calls to functions and natives that leave more than one value, which work on a stack.

// Only still called when the constants are kept rather than borrowed; otherwise the code
// generator emits the copy out of the VM itself, see EmitPushStr.
// FIXME: have a way that constant strings can stay in the bytecode, so this doesn't need the
// indirection thru the VM at all.
VM_INLINE LString *RtPushStr(VM &vm, int i) {
    auto s = vm.constant_strings[i];
    #if STRING_CONSTANTS_KEEP
        s.LTINCRT();
    #endif
    return s.sval();
}

// The function a dynamic dispatch on the class of `self` lands in, which the generated code
// then calls with the signature it knows the dispatch has.
VM_INLINE fun_base_t RtDynDispatch(VM &vm, LObject *self, int vtable_idx) {
    auto start = self->ti(vm).vtable_start_or_bitmask;
    auto target = vm.vma.native_vtables[start + vtable_idx];
    assert(target);
    return target;
}

VM_INLINE void RtExit(VM &vm, Value ret, type_elem_t ti) {
    vm.EndEval(ret, vm.GetTypeInfo(ti));
}

VM_INLINE void RtExitVoid(VM &vm) {
    vm.EndEval(NilVal(), vm.GetTypeInfo(TYPE_ELEM_ANY));
}

// The generated code calls builtins directly by their symbol, see CodeGen::EmitNativeCall,
// which puts these around the call when they are compiled in, see LOBSTER_NATIVE_PROFILE.
#if LOBSTER_NATIVE_PROFILE
VM_INLINE ___tracy_c_zone_context RtNativeProfileStart(VM &vm, int nfi) {
    #if LOBSTER_FRAME_PROFILER_GLOBAL
        g_builtin_locations.push_back(vm.vma.nfr.pre_allocated_function_locations[nfi]);
    #endif
    #if LOBSTER_FRAME_PROFILER_BUILTINS
        return ___tracy_emit_zone_begin(&vm.vma.nfr.pre_allocated_function_locations[nfi], true);
    #else
        return ___tracy_c_zone_context();
    #endif
}

VM_INLINE void RtNativeProfileEnd(___tracy_c_zone_context ctx) {
    #if LOBSTER_FRAME_PROFILER_BUILTINS
        ___tracy_emit_zone_end(ctx);
    #else
        (void)ctx;
    #endif
    #if LOBSTER_FRAME_PROFILER_GLOBAL
        g_builtin_locations.pop_back();
    #endif
}
#endif

// A vector or object with room for its elements, which the generated code writes itself.
VM_INLINE LVector *RtNewVec(VM &vm, type_elem_t ti, int len) {
    return vm.NewVec(len, len, ti);
}

VM_INLINE LObject *RtNewObject(VM &vm, type_elem_t ti) {
    return vm.NewObject(vm.GetTypeInfo(ti).len, ti);
}

// Room for one more element in a vector that has none left, which is all a push the generated
// code writes out itself cannot do on its own, see CodeGen::EmitVectorPush.
VM_INLINE void RtVectorGrow(VM &vm, LVector *v) {
    v->Resize(vm, v->maxl ? v->maxl * 2 : 4);
}

// The same for the ones that take an element out, see CodeGen::EmitCodegenBuiltin: the errors,
// named after the builtin that ran into them, and closing the gap the element leaves behind.
VM_INLINE void RtVectorEmptyErr(VM &vm, int nfi) {
    vm.BuiltinError(cat(vm.vma.nfr.nfuns[nfi]->name, ": empty vector"));
}

VM_INLINE void RtVectorIdxErr(VM &vm, int nfi, iint i, iint len) {
    vm.BuiltinError(cat(vm.vma.nfr.nfuns[nfi]->name, ": index (", i, ") out of range (", len,
                        ")"));
}

VM_INLINE void RtVectorErase(LVector *v, iint i) {
    v->Erase(i);
}

VM_INLINE LString *RtStrConcatN(VM &vm, Value *strs, int len) {
    iint blen = 0;
    // Find total len.
    for (int i = 0; i < len; i++) blen += strs[i].sval()->len;
    // Just one alloc.
    auto ds = vm.NewString(blen);
    auto dest = (char *)ds->data();
    for (int i = 0; i < len; i++) {
        auto s = strs[i].sval();
        memcpy(dest, s->data(), s->len);
        dest += s->len;
    }
    return ds;
}

// While float div by zero is generally undefined in C++, if it promises to adhere to IEEE754
// we get the desirable result of Inf values instead, and we don't have to check for 0.
// This behavior is similar to what Java/C#/JS already do.
// https://en.cppreference.com/w/cpp/language/operator_arithmetic#Multiplicative_operators
// We do the same for https://en.cppreference.com/w/c/numeric/math/fmod
// Integer div by zero is still a language level runtime error, as is INT_MIN / -1.
static_assert(std::numeric_limits<double>::is_iec559, "IEEE754 floats required");

// Division and modulo are the only arithmetic that stays a call, for the check on the divisor.
// Everything else the code generator emits as the operator it is, see BinExpr.
VM_INLINE void DivCheck(VM &vm, iint a, iint b) {
    if (b <= 0 && b >= -1 && (!b || a == LLONG_MIN)) vm.DivErr(b);
}

VM_INLINE iint RtIDiv(VM &vm, iint a, iint b) {
    DivCheck(vm, a, b);
    return a / b;
}

VM_INLINE iint RtIMod(VM &vm, iint a, iint b) {
    DivCheck(vm, a, b);
    return a % b;
}

VM_INLINE double RtFMod(double a, double b) {
    return fmod(a, b);
}

VM_INLINE LString *RtSAdd(VM &vm, LString *a, LString *b) {
    return vm.NewString(a->strv(), b->strv());
}

VM_INLINE iint RtSLt(LString *a, LString *b) { return *a <  *b; }
VM_INLINE iint RtSGt(LString *a, LString *b) { return *a >  *b; }
VM_INLINE iint RtSLe(LString *a, LString *b) { return *a <= *b; }
VM_INLINE iint RtSGe(LString *a, LString *b) { return *a >= *b; }
VM_INLINE iint RtSEq(LString *a, LString *b) { return *a == *b; }
VM_INLINE iint RtSNe(LString *a, LString *b) { return *a != *b; }

// The nillable string comparisons, where nil is equal to nil and to nothing else.
VM_INLINE iint RtSnEq(LString *a, LString *b) {
    if (!a || !b) return a == b;
    return *a == *b;
}

VM_INLINE iint RtSnNe(LString *a, LString *b) {
    if (!a || !b) return a != b;
    return *a != *b;
}

VM_INLINE LString *RtToString(VM &vm, Value a, type_elem_t ti) {
    return vm.ToString(a, vm.GetTypeInfo(ti));
}

VM_INLINE LString *RtStructToString(VM &vm, Value *vals, type_elem_t ti) {
    return vm.StructToString(vals, vm.GetTypeInfo(ti));
}

VM_INLINE bool RtStaticSetThisFrame(VM &vm, int vidx) {
    auto &v = vm.fvars[vidx];
    auto jump = v.ival() < vm.frame_count;
    v = vm.frame_count + 1;
    return jump;
}

VM_INLINE bool RtMemberSetThisFrame(VM &vm, LObject *self, int slot) {
    auto jump = self->At(slot).ival() < vm.frame_count;
    self->SetAt(slot, vm.frame_count + 1);
    return jump;
}

// Only emitted when the tested type is a class with subclasses, otherwise the test is against
// a single type id, which the code generator emits inline, see EmitIsType.
VM_INLINE iint RtIsSubType(VM &vm, LObject *v, int start, int end, int nilres) {
    // The typechecker guarantees the value is statically a class (or nil), so
    // its type info always has a subtype_dfs.
    if (!v) return nilres;
    auto dfs = v->ti(vm).subtype_dfs;
    return start <= dfs && dfs <= end;
}

VM_INLINE void RtAbort(VM &vm) {
    vm.SeriousError("VM internal error: abort");
}

[[noreturn]] VM_INLINE void RtEnumRangeErr(VM &vm) {
    vm.Error("Enum out of range of possible values in switch");
}

// A class indexed at runtime as an lvalue, whose range check needs the type info.
VM_INLINE Value *RtLvalIndexClass(VM &vm, LObject *o, iint i, int offset) {
    RANGECHECK(vm, i, o->Len(vm), o);
    return (Value *)o->FieldSlots() + i + offset;
}

// Appending to a string in place, which can free the old one.
VM_INLINE void RtLvSAdd(VM &vm, Value *lv, LString *b) {
    auto res = vm.NewString(lv->sval()->strv(), b->strv());
    lv->LTDECRTNIL(vm);
    *lv = Value(res);
}

}  // namespace lobster
