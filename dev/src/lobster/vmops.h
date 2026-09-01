#include "lobster/geom.h"
#include "lobster/vmdata.h"
#include "lobster/natreg.h"
#include "lobster/bytecode_generated.h"

extern string BreakPoint(lobster::VM &vm, string_view reason, bool also_flex_dump);

namespace lobster {

#if RTT_ENABLED
    #define VMTYPEEQ(val, vt) VMASSERT(vm, (val).type == (vt))
#else
    #define VMTYPEEQ(val, vt) { (void)(val); (void)(vt); (void)vm; }
#endif

VM_INLINE void PushDerefIdxVector1(VM &vm, StackPtr &sp, iint i) {
    Value r = Pop(sp);
    VMASSERT(vm, r.ref());
    auto v = r.vval();
    RANGECHECK(vm, i, v->len, v);
    Push(sp, v->AtS(i));
}

VM_INLINE void PushDerefIdxVector2V(VM &vm, StackPtr &sp, iint i) {
    Value r = Pop(sp);
    VMASSERT(vm, r.ref());
    auto v = r.vval();
    RANGECHECK(vm, i, v->len, v);
    v->AtVW(sp, i);
}

VM_INLINE void PushDerefIdxVectorSub1(VM &vm, StackPtr &sp, iint i, int offset) {
    Value r = Pop(sp);
    VMASSERT(vm, r.ref());
    auto v = r.vval();
    RANGECHECK(vm, i, v->len, v);
    Push(sp, v->AtSub(i, offset));
}

VM_INLINE void PushDerefIdxVectorSub2V(VM &vm, StackPtr &sp, iint i, int width, int offset) {
    Value r = Pop(sp);
    VMASSERT(vm, r.ref());
    auto v = r.vval();
    RANGECHECK(vm, i, v->len, v);
    v->AtVWSub(sp, i, width, offset);
}

VM_INLINE void PushDerefIdxStruct(VM &vm, StackPtr &sp, iint i, int l) {
    PopN(sp, l);
    RANGECHECKS(vm, i, l);
    auto val = *(TopPtr(sp) + i);
    Push(sp, val);
}

VM_INLINE void PushDerefIdxString(VM &vm, StackPtr &sp, iint i) {
    Value r = Pop(sp);
    VMASSERT(vm, r.ref());
    // Allow access of the terminating 0-byte.
    RANGECHECK(vm, i, r.sval()->len + 1, r.sval());
    Push(sp, Value(((uint8_t *)r.sval()->data())[i]));
}

VM_INLINE Value &GetFieldLVal(VM &vm, StackPtr &sp, iint i) {
    Value vec = Pop(sp);
    #ifndef NDEBUG
        RANGECHECK(vm, i, vec.oval()->Len(vm), vec.oval());
    #else
        (void)vm;
    #endif
    return vec.oval()->AtR(i);
}

VM_INLINE Value &GetFieldILVal(VM &vm, StackPtr &sp, iint i) {
    Value vec = Pop(sp);
    RANGECHECK(vm, i, vec.oval()->Len(vm), vec.oval());
    return vec.oval()->AtR(i);
}

// This is the only time we turn one lval into another!
VM_INLINE Value &GetFieldISLVal(VM &vm, Value *lv, iint i, int maxfields) {
    RANGECHECKS(vm, i, maxfields);
    return lv[i];
}

VM_INLINE Value &GetVecLVal(VM &vm, StackPtr &sp, iint i) {
    Value vec = Pop(sp);
    auto v = vec.vval();
    RANGECHECK(vm, i, v->len, v);
    return *v->AtSt(i);
}

VM_INLINE void RtPushFloat(StackPtr sp, int64_t bits) {
    Push(sp, Value(int2float64(bits).f));
}

// Only still called when the constants are kept rather than borrowed; otherwise the code
// generator emits the copy out of the VM itself, see EmitPUSHSTR.
// FIXME: have a way that constant strings can stay in the bytecode, so this doesn't need the
// indirection thru the VM at all.
VM_INLINE void RtPushStr(VM &vm, StackPtr sp, int i) {
    auto s = vm.constant_strings[i];
    #if STRING_CONSTANTS_KEEP
        s.LTINCRT();
    #endif
    Push(sp, s);
}

VM_INLINE void RtCallValue(VM &vm, StackPtr sp) {
    Value fun = Pop(sp);
    VMTYPEEQ(fun, RTT_FUNCTION);
    vm.next_call_target = fun.ip();
}

VM_INLINE void RtDynDispatch(VM &vm, StackPtr sp, int vtable_idx, int stack_idx) {
    auto self = TopM(sp, stack_idx);
    VMTYPEEQ(self, RTT_CLASS);
    auto start = self.oval()->ti(vm).vtable_start_or_bitmask;
    vm.next_call_target = vm.vma.native_vtables[start + vtable_idx];
    assert(vm.next_call_target);
}

VM_INLINE void RtExit(VM &vm, StackPtr sp, type_elem_t ti) {
    if (ti >= 0) vm.EndEval(sp, Pop(sp), vm.GetTypeInfo(ti));
    else vm.EndEval(sp, NilVal(), vm.GetTypeInfo(TYPE_ELEM_ANY));
}

VM_INLINE bool ForLoop(VM &, StackPtr sp, iint len) {
    auto &i = TopMR(sp, 1);
    TYPE_ASSERT(i.type == RTT_INT);
    i.setival(i.ival() + 1);
    return i.ival() < len;
}

#define FORELEM(L) \
    auto iter = Top(sp); \
    auto i = TopM(sp, 1).ival(); \
    assert(i < L);

#if LOBSTER_FRAME_PROFILER_BUILTINS
    #define BPROF_START(NFI) auto ctx = ___tracy_emit_zone_begin(&vm.vma.nfr.pre_allocated_function_locations[NFI], true)
    #define BPROF_END() ___tracy_emit_zone_end(ctx)
#else
    #define BPROF_START(NFI)
    #define BPROF_END() 
#endif

#if LOBSTER_FRAME_PROFILER_GLOBAL
    #define GPROF_START(NFI) g_builtin_locations.push_back(vm.vma.nfr.pre_allocated_function_locations[NFI]);
    #define GPROF_END() g_builtin_locations.pop_back();
#else
    #define GPROF_START(NFI) 
    #define GPROF_END() 
#endif


VM_INLINE void RtNativeCallV(VM &vm, StackPtr sp, int nfi, int /*has_ret*/) {
    BPROF_START(nfi);
    GPROF_START(nfi);
    vm.native_funs[nfi].fV(sp, vm);
    GPROF_END();
    BPROF_END();
}

#define BCALLOP(N,DECLS,ARGS) \
VM_INLINE void RtNativeCall##N(VM &vm, StackPtr sp, int nfi, int has_ret) { \
    BPROF_START(nfi); \
    GPROF_START(nfi); \
    DECLS; \
    Value v = vm.native_funs[nfi].f##N ARGS; \
    GPROF_END(); \
    BPROF_END(); \
    if (has_ret) { Push(sp, v); vm.BCallRetCheck(sp, nfi); } \
}

BCALLOP(0, {}, (sp, vm));
BCALLOP(1, auto a0 = Pop(sp), (sp, vm, a0));
BCALLOP(2, auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1));
BCALLOP(3, auto a2 = Pop(sp);auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1, a2));
BCALLOP(4, auto a3 = Pop(sp);auto a2 = Pop(sp);auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1, a2, a3));
BCALLOP(5, auto a4 = Pop(sp);auto a3 = Pop(sp);auto a2 = Pop(sp);auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1, a2, a3, a4));
BCALLOP(6, auto a5 = Pop(sp);auto a4 = Pop(sp);auto a3 = Pop(sp);auto a2 = Pop(sp);auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1, a2, a3, a4, a5));
BCALLOP(7, auto a6 = Pop(sp);auto a5 = Pop(sp);auto a4 = Pop(sp);auto a3 = Pop(sp);auto a2 = Pop(sp);auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1, a2, a3, a4, a5, a6));

VM_INLINE void RtNewVec(VM &vm, StackPtr sp, type_elem_t ti, int len) {
    auto vec = vm.NewVec(len, len, ti);
    if (len) vec->CopyElemsShallow(TopPtr(sp) - len * vec->width);
    PopN(sp, len * (int)vec->width);
    Push(sp, Value(vec));
}

VM_INLINE void RtNewObject(VM &vm, StackPtr sp, type_elem_t ti) {
    auto len = vm.GetTypeInfo(ti).len;
    auto vec = vm.NewObject(len, ti);
    if (len) vec->CopyElemsShallow(TopPtr(sp) - len, len);
    PopN(sp, len);
    Push(sp, Value(vec));
}

VM_INLINE void RtPopV(StackPtr sp, int len) { PopN(sp, len); }

VM_INLINE void RtStrConcatN(VM &vm, StackPtr sp, int len) {
    iint blen = 0;
    // Find total len.
    for (int i = 0; i < len; i++) blen += TopM(sp, i).sval()->len;
    // Just one alloc.
    auto ds = vm.NewString(blen);
    // Copy them all in, backwards.
    for (int i = 0; i < len; i++) {
        auto s = Pop(sp).sval();
        blen -= s->len;
        memcpy((char *)ds->data() + blen, s->data(), s->len);
    }
    Push(sp, Value(ds));
}

// While float div by zero is generally undefined in C++, if it promises to adhere to IEEE754
// we get the desirable result of Inf values instead, and we don't have to check for 0.
// This behavior is similar to what Java/C#/JS already do.
// https://en.cppreference.com/w/cpp/language/operator_arithmetic#Multiplicative_operators
// We do the same for https://en.cppreference.com/w/c/numeric/math/fmod
// Integer div by zero is still a language level runtime error, as is INT_MIN / -1.
static_assert(std::numeric_limits<double>::is_iec559, "IEEE754 floats required");

#define GETARGS() Value b = Pop(sp); Value a = Pop(sp)
#define TYPEOP(op, extras, av, bv) \
    if constexpr ((extras & 1) != 0) if (bv <= 0 && bv >= -1 && (!bv || av == LLONG_MIN)) vm.DivErr(bv); \
    Value res = av op bv; \
    if constexpr ((extras & 2) != 0) res = (decltype(res))fmod((double)av, (double)bv);

#define _IOP(op, extras) \
    TYPE_ASSERT(a.type == RTT_INT && b.type == RTT_INT); \
    TYPEOP(op, extras, a.ival(), b.ival())
// Shifts don't go thru TYPEOP, since the count needs masking, see MaskedShiftLeft.
#define _ISHIFTOP(f) \
    TYPE_ASSERT(a.type == RTT_INT && b.type == RTT_INT); \
    Value res = f(a.ival(), b.ival())
#define _FOP(op, extras) \
    TYPE_ASSERT(a.type == RTT_FLOAT && b.type == RTT_FLOAT); \
    TYPEOP(op, extras, a.fval(), b.fval())

#define _VOPS(op, extras, RT_T, field, geta) { \
    auto b = Pop(sp); \
    VMTYPEEQ(b, RT_T) \
    auto veca = geta; \
    for (int j = 0; j < len; j++) { \
        auto &a = veca[j]; \
        VMTYPEEQ(a, RT_T) \
        auto bv = b.field(); \
        TYPEOP(op, extras, a.field(), bv) \
        a = res; \
    } \
}
#define _SOPV(op, extras, RT_T, field, geta) { \
    PopN(sp, len); \
    auto vecb = TopPtr(sp); \
    auto a = geta; \
    VMTYPEEQ(a, RT_T) \
    for (int j = 0; j < len; j++) { \
        auto &b = vecb[j]; \
        VMTYPEEQ(b, RT_T) \
        auto av = a.field(); \
        TYPEOP(op, extras, av, b.field()) \
        Push(sp, res); \
    } \
}
#define _VOPV(op, extras, RT_T, field, geta) { \
    PopN(sp, len); \
    auto vecb = TopPtr(sp); \
    auto veca = geta; \
    for (int j = 0; j < len; j++) { \
        auto b = vecb[j]; \
        VMTYPEEQ(b, RT_T) \
        auto &a = veca[j]; \
        VMTYPEEQ(a, RT_T) \
        auto bv = b.field(); \
        TYPEOP(op, extras, a.field(), bv) \
        a = res; \
    } \
}
#define STCOMPEN(op, init, andor) { \
    PopN(sp, len); \
    auto vecb = TopPtr(sp); \
    PopN(sp, len); \
    auto veca = TopPtr(sp); \
    auto all = init; \
    for (int j = 0; j < len; j++) { \
        all = all andor veca[j].any() op vecb[j].any(); \
    } \
    Push(sp, all); \
}

#define _IVOPS(op, extras, geta) _VOPS(op, extras, RTT_INT,   ival, geta)
#define _IVOPV(op, extras, geta) _VOPV(op, extras, RTT_INT,   ival, geta)
#define _SOPIV(op, extras, geta) _SOPV(op, extras, RTT_INT,   ival, geta)
#define _FVOPS(op, extras, geta) _VOPS(op, extras, RTT_FLOAT, fval, geta)
#define _FVOPV(op, extras, geta) _VOPV(op, extras, RTT_FLOAT, fval, geta)
#define _SOPFV(op, extras, geta) _SOPV(op, extras, RTT_FLOAT, fval, geta)

#define _SCAT() Value res = vm.NewString(a.sval()->strv(), b.sval()->strv())

#define SNCOMPEN(op) { GETARGS(); \
   if (!a.any()) { Push(sp, b.any() op nullptr); } \
   else if (!b.any()) { Push(sp, a.any() op nullptr); } \
   else { Value res = *a.sval() op *b.sval(); Push(sp, res); } }

#define ACOMPEN(op)     { GETARGS(); Value res = a.any() op b.any(); Push(sp, res); }
#define IOP(op, extras) { GETARGS(); _IOP(op, extras);               Push(sp, res); }
#define FOP(op, extras) { GETARGS(); _FOP(op, extras);               Push(sp, res); }
#define LOP(op)         { GETARGS(); auto res = a.ip() op b.ip();    Push(sp, res); }

#define IVVOP(op, extras) { _IVOPV(op, extras, TopPtr(sp) - len); }
#define FVVOP(op, extras) { _FVOPV(op, extras, TopPtr(sp) - len); }
#define IVSOP(op, extras) { _IVOPS(op, extras, TopPtr(sp) - len); }
#define FVSOP(op, extras) { _FVOPS(op, extras, TopPtr(sp) - len); }
#define SIVOP(op, extras) { _SOPIV(op, extras, Pop(sp)); }
#define SFVOP(op, extras) { _SOPFV(op, extras, Pop(sp)); }

#define SOP(op) { GETARGS(); Value res = *a.sval() op *b.sval(); Push(sp, res); }
#define SCAT()  { GETARGS(); _SCAT();                            Push(sp, res); }

// +  += I F Vif S
// -  -= I F Vif
// *  *= I F Vif
// /  /= I F Vif
// %  %= I F Vif

// <     I F Vif S
// >     I F Vif S
// <=    I F Vif S
// >=    I F Vif S
// ==    I F V   S   // FIXME differentiate struct / value / vector
// !=    I F V   S

// U-    I F Vif
// U!    A

VM_INLINE void RtIvvAdd(VM &vm, StackPtr sp, int len) { IVVOP(+,  0); }
VM_INLINE void RtIvvSub(VM &vm, StackPtr sp, int len) { IVVOP(-,  0); }
VM_INLINE void RtIvvMul(VM &vm, StackPtr sp, int len) { IVVOP(*,  0); }
VM_INLINE void RtIvvDiv(VM &vm, StackPtr sp, int len) { IVVOP(/,  1); }
VM_INLINE void RtIvvMod(VM &vm, StackPtr sp, int len) { IVVOP(%,  1); }
VM_INLINE void RtIvvLt(VM &vm, StackPtr sp, int len)  { IVVOP(<,  0); }
VM_INLINE void RtIvvGt(VM &vm, StackPtr sp, int len)  { IVVOP(>,  0); }
VM_INLINE void RtIvvLe(VM &vm, StackPtr sp, int len)  { IVVOP(<=, 0); }
VM_INLINE void RtIvvGe(VM &vm, StackPtr sp, int len)  { IVVOP(>=, 0); }
VM_INLINE void RtFvvAdd(VM &vm, StackPtr sp, int len) { FVVOP(+,  0); }
VM_INLINE void RtFvvSub(VM &vm, StackPtr sp, int len) { FVVOP(-,  0); }
VM_INLINE void RtFvvMul(VM &vm, StackPtr sp, int len) { FVVOP(*,  0); }
VM_INLINE void RtFvvDiv(VM &vm, StackPtr sp, int len) { FVVOP(/,  0); }
VM_INLINE void RtFvvMod(VM &vm, StackPtr sp, int len) { FVVOP(/,  2); }
VM_INLINE void RtFvvLt(VM &vm, StackPtr sp, int len)  { FVVOP(<,  0); }
VM_INLINE void RtFvvGt(VM &vm, StackPtr sp, int len)  { FVVOP(>,  0); }
VM_INLINE void RtFvvLe(VM &vm, StackPtr sp, int len)  { FVVOP(<=, 0); }
VM_INLINE void RtFvvGe(VM &vm, StackPtr sp, int len)  { FVVOP(>=, 0); }

VM_INLINE void RtIvsAdd(VM &vm, StackPtr sp, int len) { IVSOP(+,  0); }
VM_INLINE void RtIvsSub(VM &vm, StackPtr sp, int len) { IVSOP(-,  0); }
VM_INLINE void RtIvsMul(VM &vm, StackPtr sp, int len) { IVSOP(*,  0); }
VM_INLINE void RtIvsDiv(VM &vm, StackPtr sp, int len) { IVSOP(/,  1); }
VM_INLINE void RtIvsMod(VM &vm, StackPtr sp, int len) { IVSOP(%,  1); }
VM_INLINE void RtIvsLt(VM &vm, StackPtr sp, int len)  { IVSOP(<,  0); }
VM_INLINE void RtIvsGt(VM &vm, StackPtr sp, int len)  { IVSOP(>,  0); }
VM_INLINE void RtIvsLe(VM &vm, StackPtr sp, int len)  { IVSOP(<=, 0); }
VM_INLINE void RtIvsGe(VM &vm, StackPtr sp, int len)  { IVSOP(>=, 0); }
VM_INLINE void RtFvsAdd(VM &vm, StackPtr sp, int len) { FVSOP(+,  0); }
VM_INLINE void RtFvsSub(VM &vm, StackPtr sp, int len) { FVSOP(-,  0); }
VM_INLINE void RtFvsMul(VM &vm, StackPtr sp, int len) { FVSOP(*,  0); }
VM_INLINE void RtFvsDiv(VM &vm, StackPtr sp, int len) { FVSOP(/,  0); }
VM_INLINE void RtFvsMod(VM &vm, StackPtr sp, int len) { FVSOP(/,  2); }
VM_INLINE void RtFvsLt(VM &vm, StackPtr sp, int len)  { FVSOP(<,  0); }
VM_INLINE void RtFvsGt(VM &vm, StackPtr sp, int len)  { FVSOP(>,  0); }
VM_INLINE void RtFvsLe(VM &vm, StackPtr sp, int len)  { FVSOP(<=, 0); }
VM_INLINE void RtFvsGe(VM &vm, StackPtr sp, int len)  { FVSOP(>=, 0); }

VM_INLINE void RtSivAdd(VM &vm, StackPtr sp, int len) { SIVOP(+,  0); }
VM_INLINE void RtSivSub(VM &vm, StackPtr sp, int len) { SIVOP(-,  0); }
VM_INLINE void RtSivMul(VM &vm, StackPtr sp, int len) { SIVOP(*,  0); }
VM_INLINE void RtSivDiv(VM &vm, StackPtr sp, int len) { SIVOP(/,  1); }
VM_INLINE void RtSivMod(VM &vm, StackPtr sp, int len) { SIVOP(%,  1); }
VM_INLINE void RtSivLt(VM &vm, StackPtr sp, int len)  { SIVOP(<,  0); }
VM_INLINE void RtSivGt(VM &vm, StackPtr sp, int len)  { SIVOP(>,  0); }
VM_INLINE void RtSivLe(VM &vm, StackPtr sp, int len)  { SIVOP(<=, 0); }
VM_INLINE void RtSivGe(VM &vm, StackPtr sp, int len)  { SIVOP(>=, 0); }
VM_INLINE void RtSfvAdd(VM &vm, StackPtr sp, int len) { SFVOP(+,  0); }
VM_INLINE void RtSfvSub(VM &vm, StackPtr sp, int len) { SFVOP(-,  0); }
VM_INLINE void RtSfvMul(VM &vm, StackPtr sp, int len) { SFVOP(*,  0); }
VM_INLINE void RtSfvDiv(VM &vm, StackPtr sp, int len) { SFVOP(/,  0); }
VM_INLINE void RtSfvMod(VM &vm, StackPtr sp, int len) { SFVOP(/,  2); }
VM_INLINE void RtSfvLt(VM &vm, StackPtr sp, int len)  { SFVOP(<,  0); }
VM_INLINE void RtSfvGt(VM &vm, StackPtr sp, int len)  { SFVOP(>,  0); }
VM_INLINE void RtSfvLe(VM &vm, StackPtr sp, int len)  { SFVOP(<=, 0); }
VM_INLINE void RtSfvGe(VM &vm, StackPtr sp, int len)  { SFVOP(>=, 0); }

VM_INLINE void RtAEq(StackPtr sp)  { ACOMPEN(==); }
VM_INLINE void RtANe(StackPtr sp)  { ACOMPEN(!=); }

VM_INLINE void RtSnEq(StackPtr sp)  { SNCOMPEN(==); }
VM_INLINE void RtSnNe(StackPtr sp)  { SNCOMPEN(!=); }

VM_INLINE void RtStEq(StackPtr sp, int len) { STCOMPEN(==, true,  &&); }
VM_INLINE void RtStNe(StackPtr sp, int len) { STCOMPEN(!=, false, ||); }

VM_INLINE void RtLEq(StackPtr sp) { LOP(==); }
VM_INLINE void RtLNe(StackPtr sp) { LOP(!=); }

// The arithmetic and comparison ops below are emitted as the C operator they are, see
// GenSimpleBinOp, so only a C backend build that has a runtime type field to keep correct still
// calls them. Division and modulo are always called, for their division by zero check.
VM_INLINE void RtIAdd(VM &vm, StackPtr sp) { IOP(+,  0); }
VM_INLINE void RtISub(VM &vm, StackPtr sp) { IOP(-,  0); }
VM_INLINE void RtIMul(VM &vm, StackPtr sp) { IOP(*,  0); }
VM_INLINE void RtIDiv(VM &vm, StackPtr sp) { IOP(/,  1); }
VM_INLINE void RtIMod(VM &vm, StackPtr sp) { IOP(%,  1); }
VM_INLINE void RtILt(VM &vm, StackPtr sp)  { IOP(<,  0); }
VM_INLINE void RtIGt(VM &vm, StackPtr sp)  { IOP(>,  0); }
VM_INLINE void RtILe(VM &vm, StackPtr sp)  { IOP(<=, 0); }
VM_INLINE void RtIGe(VM &vm, StackPtr sp)  { IOP(>=, 0); }
VM_INLINE void RtIEq(VM &vm, StackPtr sp)  { IOP(==, 0); }
VM_INLINE void RtINe(VM &vm, StackPtr sp)  { IOP(!=, 0); }

VM_INLINE void RtFAdd(VM &vm, StackPtr sp) { FOP(+,  0); }
VM_INLINE void RtFSub(VM &vm, StackPtr sp) { FOP(-,  0); }
VM_INLINE void RtFMul(VM &vm, StackPtr sp) { FOP(*,  0); }
VM_INLINE void RtFDiv(VM &vm, StackPtr sp) { FOP(/,  0); }
VM_INLINE void RtFMod(VM &vm, StackPtr sp) { FOP(/,  2); }
VM_INLINE void RtFLt(VM &vm, StackPtr sp)  { FOP(<,  0); }
VM_INLINE void RtFGt(VM &vm, StackPtr sp)  { FOP(>,  0); }
VM_INLINE void RtFLe(VM &vm, StackPtr sp)  { FOP(<=, 0); }
VM_INLINE void RtFGe(VM &vm, StackPtr sp)  { FOP(>=, 0); }
VM_INLINE void RtFEq(VM &vm, StackPtr sp)  { FOP(==, 0); }
VM_INLINE void RtFNe(VM &vm, StackPtr sp)  { FOP(!=, 0); }

VM_INLINE void RtSAdd(VM &vm, StackPtr sp) { SCAT(); }
VM_INLINE void RtSSub(VM &vm, StackPtr)             { VMASSERT(vm, 0); }
VM_INLINE void RtSMul(VM &vm, StackPtr)             { VMASSERT(vm, 0); }
VM_INLINE void RtSDiv(VM &vm, StackPtr)             { VMASSERT(vm, 0); }
VM_INLINE void RtSMod(VM &vm, StackPtr)             { VMASSERT(vm, 0); }
VM_INLINE void RtSLt(VM &, StackPtr sp)    { SOP(<);  }
VM_INLINE void RtSGt(VM &, StackPtr sp)    { SOP(>);  }
VM_INLINE void RtSLe(VM &, StackPtr sp)    { SOP(<=); }
VM_INLINE void RtSGe(VM &, StackPtr sp)    { SOP(>=); }
VM_INLINE void RtSEq(VM &, StackPtr sp)    { SOP(==); }
VM_INLINE void RtSNe(VM &, StackPtr sp)    { SOP(!=); }

VM_INLINE void RtIUMinus(StackPtr sp) { Value a = Pop(sp); Push(sp, Value(-a.ival())); }
VM_INLINE void RtFUMinus(StackPtr sp) { Value a = Pop(sp); Push(sp, Value(-a.fval())); }

VM_INLINE void RtIvUMinus(VM &vm, StackPtr sp, int len) {
    auto vec = TopPtr(sp) - len;
    for (int i = 0; i < len; i++) {
        auto &a = vec[i];
        VMTYPEEQ(a, RTT_INT);
        a = -a.ival();
    }
}

VM_INLINE void RtFvUMinus(VM &vm, StackPtr sp, int len) {
    auto vec = TopPtr(sp) - len;
    for (int i = 0; i < len; i++) {
        auto &a = vec[i];
        VMTYPEEQ(a, RTT_FLOAT);
        a = -a.fval();
    }
}

#define BITOP(op) { GETARGS(); Push(sp, a.ival() op b.ival()); }
#define SHIFTOP(f) { GETARGS(); Push(sp, f(a.ival(), b.ival())); }
VM_INLINE void RtBinAnd(StackPtr sp) { BITOP(&);  }
VM_INLINE void RtBinOr(StackPtr sp)  { BITOP(|);  }
VM_INLINE void RtXor(StackPtr sp)    { BITOP(^);  }
VM_INLINE void RtAsl(StackPtr sp)    { SHIFTOP(MaskedShiftLeft);  }
VM_INLINE void RtAsr(StackPtr sp)    { SHIFTOP(MaskedShiftRight); }
VM_INLINE void RtNeg(StackPtr sp)    { auto a = Pop(sp); Push(sp, ~a.ival()); }

VM_INLINE void RtToString(VM &vm, StackPtr sp, type_elem_t ti) {
    Value a = Pop(sp);
    Push(sp, vm.ToString(a, vm.GetTypeInfo(ti)));
}

VM_INLINE void RtStructToString(VM &vm, StackPtr sp, type_elem_t ti) {
    auto &info = vm.GetTypeInfo(ti);
    PopN(sp, info.len);
    auto top = TopPtr(sp);
    Push(sp, vm.StructToString(top, info));
}

VM_INLINE void RtPushFieldMRef(VM &vm, StackPtr sp, int i) {
    Value r = Pop(sp);
    if (!r.ref()) {
        Push(sp, r);
    } else {
        assert(i < r.oval()->Len(vm));
        (void)vm;
        Push(sp, r.oval()->At(i));
    }
}
VM_INLINE void RtPushFieldV(StackPtr sp, int i, int l) {
    PopN(sp, l);
    auto val = *(TopPtr(sp) + i);
    Push(sp, val);
}
VM_INLINE void RtPushFieldV2V(StackPtr sp, int i, int rl, int l) {
    PopN(sp, l);
    t_memmove(TopPtr(sp), TopPtr(sp) + i, rl);
    PushN(sp, rl);
}

VM_INLINE void RtIndexVecSub(VM &vm, StackPtr sp, int o) {
    auto x = Pop(sp).ival();
    PushDerefIdxVectorSub1(vm, sp, x, o);
}

VM_INLINE void RtIndexVecSubV(VM &vm, StackPtr sp, int w, int o) {
    auto x = Pop(sp).ival();
    PushDerefIdxVectorSub2V(vm, sp, x, w, o);
}

VM_INLINE void RtIndexVecNestSubV(VM &vm, StackPtr sp, int l, int w, int o) {
    auto x = vm.GrabIndex(sp, l);
    PushDerefIdxVectorSub2V(vm, sp, x, w, o);
}

VM_INLINE void RtIndexStruct(VM &vm, StackPtr sp, int l) {
    auto x = Pop(sp).ival();
    PushDerefIdxStruct(vm, sp, x, l);
}

VM_INLINE bool RtStaticSetThisFrame(VM &vm, int vidx) {
    auto &v = vm.fvars[vidx];
    auto jump = v.ival() < vm.frame_count;
    v = vm.frame_count + 1;
    return jump;
}

VM_INLINE bool RtMemberSetThisFrame(VM &vm, StackPtr sp, int slot) {
    auto self = Pop(sp).oval();
    auto &v = self->AtR(slot);
    auto jump = v.ival() < vm.frame_count;
    v = vm.frame_count + 1;
    return jump;
}

VM_INLINE void RtIsType(StackPtr sp, type_elem_t ti, int nilres) {
    auto v = Pop(sp);
    // Optimizer guarantees we don't have to deal with scalars.
    if (v.refnil()) Push(sp, v.ref()->tti == ti);
    else Push(sp, nilres);
}

// Only emitted when the tested type is a class with subclasses, otherwise the
// cheaper RtIsType is used.
VM_INLINE void RtIsSubType(VM &vm, StackPtr sp, int start, int end, int nilres) {
    auto v = Pop(sp);
    // The typechecker guarantees the value is statically a class (or nil), so
    // its type info always has a subtype_dfs.
    if (v.refnil()) {
        auto dfs = v.ref()->ti(vm).subtype_dfs;
        Push(sp, start <= dfs && dfs <= end);
    } else {
        Push(sp, nilres);
    }
}

VM_INLINE void RtAbort(VM &vm) {
    vm.SeriousError("VM internal error: abort");
}

VM_INLINE void RtEnumRangeErr(VM &vm) {
    vm.Error("Enum out of range of possible values in switch");
}

VM_INLINE Value *RtLvalIndexVecV(VM &vm, StackPtr sp, int offset, int l) {
    auto x = vm.GrabIndex(sp, l);
    return &GetVecLVal(vm, sp, x) + offset;
}

// Class accessed by index.
VM_INLINE Value *RtLvalIndexClass(VM &vm, StackPtr sp, int offset) {
    auto x = Pop(sp).ival();
    return &GetFieldILVal(vm, sp, x) + offset;
}

// Struct accessed by index, the one case that indexes an lvalue it was handed.
VM_INLINE Value *RtLvalIndexStruct(VM &vm, StackPtr sp, Value *lv, int offset, int maxfields) {
    auto x = Pop(sp).ival();
    return &GetFieldISLVal(vm, lv, x, maxfields) + offset;
}

VM_INLINE void RtLvDupV(StackPtr sp, Value *lv, int l) {
    tsnz_memcpy(TopPtr(sp), lv, l);
    PushN(sp, l);
}

#define LVALCASES(N, B) VM_INLINE void RtLv##N(VM &vm, StackPtr sp, Value *lv) { \
    auto &a = *lv; Value b = Pop(sp); B; }

#define LVALCASER(N, B) VM_INLINE void RtLv##N(VM &vm, StackPtr sp, Value *lv, int len) { \
    auto &fa = *lv; B; }

LVALCASER(IvvAdd, _IVOPV(+, 0, &fa))
LVALCASER(IvvSub, _IVOPV(-, 0, &fa))
LVALCASER(IvvMul, _IVOPV(*, 0, &fa))
LVALCASER(IvvDiv, _IVOPV(/, 1, &fa))
LVALCASER(IvvMod, _IVOPV(%, 1, &fa))

LVALCASER(FvvAdd, _FVOPV(+, 0, &fa))
LVALCASER(FvvSub, _FVOPV(-, 0, &fa))
LVALCASER(FvvMul, _FVOPV(*, 0, &fa))
LVALCASER(FvvDiv, _FVOPV(/, 0, &fa))
LVALCASER(FvvMod, _FVOPV(/, 2, &fa))

LVALCASER(IvsAdd, _IVOPS(+, 0, &fa))
LVALCASER(IvsSub, _IVOPS(-, 0, &fa))
LVALCASER(IvsMul, _IVOPS(*, 0, &fa))
LVALCASER(IvsDiv, _IVOPS(/, 1, &fa))
LVALCASER(IvsMod, _IVOPS(%, 1, &fa))

LVALCASER(FvsAdd, _FVOPS(+, 0, &fa))
LVALCASER(FvsSub, _FVOPS(-, 0, &fa))
LVALCASER(FvsMul, _FVOPS(*, 0, &fa))
LVALCASER(FvsDiv, _FVOPS(/, 0, &fa))
LVALCASER(FvsMod, _FVOPS(/, 2, &fa))

// What is left here either checks for division by zero or masks a shift count; the ones that
// are a single operator the code generator emits itself, see GenLvalModifierOpWithStructInfo.

LVALCASES(IDiv  , _IOP(/, 1); a = res;)
LVALCASES(IMod  , _IOP(%, 1); a = res;)

LVALCASES(Asl   , _ISHIFTOP(MaskedShiftLeft);  a = res;)
LVALCASES(Asr   , _ISHIFTOP(MaskedShiftRight); a = res;)

LVALCASES(FMod  , _FOP(/, 2); a = res;)

VM_INLINE void RtLvSAdd(VM &vm, StackPtr sp, Value *lv) {
    auto &a = *lv;
    Value b = Pop(sp);
    _SCAT();
    a.LTDECRTNIL(vm);
    a = res;
}

}  // namespace lobster
