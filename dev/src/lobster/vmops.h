#include "lobster/geom.h"
#include "lobster/vmdata.h"
#include "lobster/natreg.h"
#include "lobster/bytecode_generated.h"

namespace lobster {

#define VM_OP_ARGS0
#define VM_OP_ARGS1 int _a
#define VM_OP_ARGS2 int _a, int _b
#define VM_OP_ARGS3 int _a, int _b, int _c
#define VM_OP_ARGS999999 const int *ip  // ILUNKNOWN
#define VM_OP_ARGSN(N) VM_OP_ARGS##N
#define VM_OP_PASS0
#define VM_OP_PASS1 _a
#define VM_OP_PASS2 _a, _b
#define VM_OP_PASS3 _a, _b, _c
#define VM_OP_PASS999999 ip  // ILUNKNOWN
#define VM_OP_PASSN(N) VM_OP_PASS##N
#define VM_COMMA_0
#define VM_COMMA_1 ,
#define VM_COMMA_2 ,
#define VM_COMMA_3 ,
#define VM_COMMA_999999 ,  // ILUNKNOWN
#define VM_COMMA_IF(N) VM_COMMA_##N

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
    Push(sp, v->At(i));
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

VM_INLINE void PushDerefIdxStruct(StackPtr &sp, iint i, int l) {
    PopN(sp, l);
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
    return vec.oval()->AtS(i);
}

VM_INLINE Value &GetFieldILVal(VM &vm, StackPtr &sp, iint i) {
    Value vec = Pop(sp);
    RANGECHECK(vm, i, vec.oval()->Len(vm), vec.oval());
    return vec.oval()->AtS(i);
}

VM_INLINE Value &GetVecLVal(VM &vm, StackPtr &sp, iint i) {
    Value vec = Pop(sp);
    auto v = vec.vval();
    RANGECHECK(vm, i, v->len, v);
    return *v->AtSt(i);
}

VM_INLINE StackPtr U_PUSHINT(VM &, StackPtr sp, int x) {
    Push(sp, Value(x));
    return sp;
}

VM_INLINE StackPtr U_PUSHFLT(VM &, StackPtr sp, int x) {
    int2float i2f;
    i2f.i = x;
    Push(sp, Value(i2f.f));
    return sp;
}

VM_INLINE StackPtr U_PUSHNIL(VM &, StackPtr sp) {
    Push(sp, Value());
    return sp;
}

VM_INLINE StackPtr U_PUSHINT64(VM &, StackPtr sp, int a, int b) {
    auto v = Int64FromInts(a, b);
    Push(sp, Value(v));
    return sp;
}

VM_INLINE StackPtr U_PUSHFLT64(VM &, StackPtr sp, int a, int b) {
    int2float64 i2f;
    i2f.i = Int64FromInts(a, b);
    Push(sp, Value(i2f.f));
    return sp;
}

VM_INLINE StackPtr U_PUSHFUN(VM &, StackPtr sp, int start, fun_base_t fcont) {
    (void)start;
    Push(sp, Value(fcont));
    return sp;
}

VM_INLINE StackPtr U_PUSHSTR(VM &vm, StackPtr sp, int i) {
    // FIXME: have a way that constant strings can stay in the bytecode,
    // or at least preallocate them all
    auto &s = vm.constant_strings[i];
    if (!s) {
        auto fb_s = vm.bcf->stringtable()->Get(i);
        s = vm.NewString(fb_s->string_view());
    }
    #if STRING_CONSTANTS_KEEP
        s->Inc();
    #endif
    Push(sp, Value(s));
    return sp;
}

VM_INLINE StackPtr U_INCREF(VM &, StackPtr sp, int off) {
    TopM(sp, off).LTINCRTNIL();
    return sp;
}

VM_INLINE StackPtr U_KEEPREFLOOP(VM &vm, StackPtr sp, int off, int ki) {
    TopM(sp, ki).LTDECRTNIL(vm);
    TopM(sp, ki) = TopM(sp, off);
    return sp;
}

VM_INLINE StackPtr U_KEEPREF(VM &, StackPtr sp, int off, int ki) {
    TopM(sp, ki) = TopM(sp, off);
    return sp;
}

VM_INLINE StackPtr U_CALL(VM &, StackPtr sp, int) {
    return sp;
}

VM_INLINE StackPtr U_CALLV(VM &vm, StackPtr sp) {
    Value fun = Pop(sp);
    VMTYPEEQ(fun, V_FUNCTION);
    vm.next_call_target = fun.ip();
    return sp;
}

VM_INLINE StackPtr U_CALLVCOND(VM &vm, StackPtr sp) {
    // FIXME: don't need to check for function value again below if false
    if (Top(sp).False()) {
        Pop(sp);
        vm.next_call_target = 0;
    } else {
        sp = U_CALLV(vm, sp);
    }
    return sp;
}

VM_INLINE StackPtr U_DDCALL(VM &vm, StackPtr sp, int vtable_idx, int stack_idx) {
    auto self = TopM(sp, stack_idx);
    VMTYPEEQ(self, V_CLASS);
    auto start = self.oval()->ti(vm).vtable_start;
    vm.next_call_target = vm.native_vtables[start + vtable_idx];
    return sp;
}

VM_INLINE StackPtr U_FUNSTART(VM &vm, StackPtr sp, const int *ip) {
     vm.FunIntro(sp, ip);
     return sp;
}

VM_INLINE StackPtr U_RETURN(VM &vm, StackPtr sp, int df, int nrv) {
    vm.ret_unwind_to = df;  // FIXME: most returns don't need this.
    vm.ret_nrv = nrv;
    vm.FunOut(sp, nrv);
    return sp;
}

VM_INLINE StackPtr U_RETURNANY(VM &vm, StackPtr sp) {
    vm.FunOut(sp, vm.ret_nrv);
    return sp;
}

VM_INLINE StackPtr U_SAVERETS(VM &vm, StackPtr sp) {
    // Only some POP instructions appear between this and RESTORERETS.
    sp -= vm.ret_nrv;
    vm.savedrets = TopPtr(sp);
    return sp;
}

VM_INLINE StackPtr U_RESTORERETS(VM &vm, StackPtr sp) {
    ts_memcpy(TopPtr(sp), vm.savedrets, vm.ret_nrv);
    sp += vm.ret_nrv;
    return sp;
}

VM_INLINE StackPtr U_ENDSTATEMENT(VM &vm, StackPtr sp, int line, int fileidx) {
    #ifdef NDEBUG
        (void)line;
        (void)fileidx;
        (void)vm;
    #else
        if (vm.trace != TraceMode::OFF) {
            auto &sd = vm.TraceStream();
            append(sd, vm.bcf->filenames()->Get(fileidx)->string_view(), "(", line, ")");
            if (vm.trace == TraceMode::TAIL) sd += "\n"; else LOG_PROGRAM(sd);
        }
    #endif
    assert(sp == vm.stackframes.back().spstart + vm.stack);
    return sp;
}

VM_INLINE StackPtr U_EXIT(VM &vm, StackPtr sp, int tidx) {
    if (tidx >= 0) vm.EndEval(sp, Pop(sp), vm.GetTypeInfo((type_elem_t)tidx));
    else vm.EndEval(sp, Value(), vm.GetTypeInfo(TYPE_ELEM_ANY));
    return sp;
}

VM_INLINE StackPtr U_CONT1(VM &vm, StackPtr sp, int nfi) {
    auto nf = vm.nfr.nfuns[nfi];
    nf->cont1(sp, vm);
    return sp;
}

VM_INLINE StackPtr ForLoop(VM &, StackPtr sp, iint len) {
    auto &i = TopM(sp, 1);
    TYPE_ASSERT(i.type == V_INT);
    i.setival(i.ival() + 1);
    if (i.ival() >= len) {
        (void)Pop(sp); /* iter */
        (void)Pop(sp); /* i */
        Push(sp, false);
    } else {
        Push(sp, true);
    }
    return sp;
}

#define FORELEM(L) \
    auto &iter = Top(sp); \
    auto i = TopM(sp, 1).ival(); \
    assert(i < L);

VM_INLINE StackPtr U_IFOR(VM &vm, StackPtr sp) { return ForLoop(vm, sp, Top(sp).ival()); return sp; }
VM_INLINE StackPtr U_VFOR(VM &vm, StackPtr sp) { return ForLoop(vm, sp, Top(sp).vval()->len); return sp; }
VM_INLINE StackPtr U_SFOR(VM &vm, StackPtr sp) { return ForLoop(vm, sp, Top(sp).sval()->len); return sp; }

VM_INLINE StackPtr U_IFORELEM(VM &, StackPtr sp)      { FORELEM(iter.ival()); (void)iter; Push(sp, i); return sp; }
VM_INLINE StackPtr U_SFORELEM(VM &, StackPtr sp)      { FORELEM(iter.sval()->len); Push(sp, Value(((uint8_t *)iter.sval()->data())[i])); return sp; }
VM_INLINE StackPtr U_VFORELEM(VM &, StackPtr sp)      { FORELEM(iter.vval()->len); Push(sp, iter.vval()->At(i)); return sp; }
VM_INLINE StackPtr U_VFORELEM2S(VM &, StackPtr sp)    { FORELEM(iter.vval()->len); iter.vval()->AtVW(sp, i); return sp; }
VM_INLINE StackPtr U_VFORELEMREF(VM &, StackPtr sp)   { FORELEM(iter.vval()->len); auto el = iter.vval()->At(i); el.LTINCRTNIL(); Push(sp, el); return sp; }
VM_INLINE StackPtr U_VFORELEMREF2S(VM &, StackPtr sp) { FORELEM(iter.vval()->len); iter.vval()->AtVWInc(sp, i); return sp; }

VM_INLINE StackPtr U_FORLOOPI(VM &, StackPtr sp) {
    auto &i = TopM(sp, 1);  // This relies on for being inlined, otherwise it would be 2.
    TYPE_ASSERT(i.type == V_INT);
    Push(sp, i);
    return sp;
}

VM_INLINE StackPtr U_BCALLRETV(VM &vm, StackPtr sp, int nfi, int /*has_ret*/) {
    auto nf = vm.nfr.nfuns[nfi];
    nf->fun.fV(sp, vm);
    return sp;
}

#define BCALLOP(N,DECLS,ARGS) \
VM_INLINE StackPtr U_BCALLRET##N(VM &vm, StackPtr sp, int nfi, int has_ret) { \
    auto nf = vm.nfr.nfuns[nfi]; \
    DECLS; \
    Value v = nf->fun.f##N ARGS; \
    if (has_ret) { Push(sp, v); vm.BCallRetCheck(sp, nf); } \
    return sp; \
}

BCALLOP(0, {}, (sp, vm));
BCALLOP(1, auto a0 = Pop(sp), (sp, vm, a0));
BCALLOP(2, auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1));
BCALLOP(3, auto a2 = Pop(sp);auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1, a2));
BCALLOP(4, auto a3 = Pop(sp);auto a2 = Pop(sp);auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1, a2, a3));
BCALLOP(5, auto a4 = Pop(sp);auto a3 = Pop(sp);auto a2 = Pop(sp);auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1, a2, a3, a4));
BCALLOP(6, auto a5 = Pop(sp);auto a4 = Pop(sp);auto a3 = Pop(sp);auto a2 = Pop(sp);auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1, a2, a3, a4, a5));
BCALLOP(7, auto a6 = Pop(sp);auto a5 = Pop(sp);auto a4 = Pop(sp);auto a3 = Pop(sp);auto a2 = Pop(sp);auto a1 = Pop(sp);auto a0 = Pop(sp), (sp, vm, a0, a1, a2, a3, a4, a5, a6));

VM_INLINE StackPtr U_ASSERTR(VM &vm, StackPtr sp, int line, int fileidx, int stringidx) {
    (void)line;
    (void)fileidx;
    if (Top(sp).False()) {
        vm.Error(sp, cat(
            #if !VM_JIT_MODE
                vm.bcf->filenames()->Get(fileidx)->string_view(), "(", line, "): ",
            #endif
            "assertion failed: ", vm.bcf->stringtable()->Get(stringidx)->string_view()));
    }
    return sp;
}

VM_INLINE StackPtr U_ASSERT(VM &vm, StackPtr sp, int line, int fileidx, int stringidx) {
    sp = U_ASSERTR(vm, sp, line, fileidx, stringidx);
    Pop(sp);
    return sp;
}

VM_INLINE StackPtr U_NEWVEC(VM &vm, StackPtr sp, int ty, int len) {
    auto type = (type_elem_t)ty;
    auto vec = vm.NewVec(len, len, type);
    if (len) vec->Init(vm, TopPtr(sp) - len * vec->width, false);
    PopN(sp, len * (int)vec->width);
    Push(sp, Value(vec));
    return sp;
}

VM_INLINE StackPtr U_NEWOBJECT(VM &vm, StackPtr sp, int ty) {
    auto type = (type_elem_t)ty;
    auto len = vm.GetTypeInfo(type).len;
    auto vec = vm.NewObject(len, type);
    if (len) vec->Init(vm, TopPtr(sp) - len, len, false);
    PopN(sp, len);
    Push(sp, Value(vec));
    return sp;
}

VM_INLINE StackPtr U_POP(VM &, StackPtr sp)     { Pop(sp); return sp; }
VM_INLINE StackPtr U_POPREF(VM &vm, StackPtr sp)  { auto x = Pop(sp); x.LTDECRTNIL(vm); return sp; }

VM_INLINE StackPtr U_POPV(VM &, StackPtr sp, int len)    { PopN(sp, len); return sp; }
VM_INLINE StackPtr U_POPVREF(VM &vm, StackPtr sp, int len) { while (len--) Pop(sp).LTDECRTNIL(vm); return sp; }

VM_INLINE StackPtr U_DUP(VM &, StackPtr sp)    { auto x = Top(sp); Push(sp, x); return sp; }


#define GETARGS() Value b = Pop(sp); Value a = Pop(sp)
#define TYPEOP(op, extras, av, bv, res) \
    if constexpr ((extras & 1) != 0) if (bv == 0) vm.Div0(sp); \
    if constexpr ((extras & 2) != 0) res = fmod((double)av, (double)bv); else res = av op bv;

#define _IOP(op, extras) \
    Value res; \
    TYPE_ASSERT(a.type == V_INT && b.type == V_INT); \
    TYPEOP(op, extras, a.ival(), b.ival(), res)
#define _FOP(op, extras) \
    Value res; \
    TYPE_ASSERT(a.type == V_FLOAT && b.type == V_FLOAT); \
    TYPEOP(op, extras, a.fval(), b.fval(), res)

#define _VOPS(op, extras, V_T, field, geta) { \
    auto b = Pop(sp); \
    VMTYPEEQ(b, V_T) \
    auto veca = geta; \
    for (int j = 0; j < len; j++) { \
        auto &a = veca[j]; \
        VMTYPEEQ(a, V_T) \
        auto bv = b.field(); \
        TYPEOP(op, extras, a.field(), bv, a) \
    } \
}
#define _VOPV(op, extras, V_T, field, geta) { \
    PopN(sp, len); \
    auto vecb = TopPtr(sp); \
    auto veca = geta; \
    for (int j = 0; j < len; j++) { \
        auto b = vecb[j]; \
        VMTYPEEQ(b, V_T) \
        auto &a = veca[j]; \
        VMTYPEEQ(a, V_T) \
        auto bv = b.field(); \
        TYPEOP(op, extras, a.field(), bv, a) \
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
    return sp; \
}

#define _IVOPS(op, extras, geta) _VOPS(op, extras, V_INT,   ival, geta)
#define _IVOPV(op, extras, geta) _VOPV(op, extras, V_INT,   ival, geta)
#define _FVOPS(op, extras, geta) _VOPS(op, extras, V_FLOAT, fval, geta)
#define _FVOPV(op, extras, geta) _VOPV(op, extras, V_FLOAT, fval, geta)

#define _SCAT() Value res = vm.NewString(a.sval()->strv(), b.sval()->strv())

#define ACOMPEN(op)     { GETARGS(); Value res = a.any() op b.any(); Push(sp, res); return sp; }
#define IOP(op, extras) { GETARGS(); _IOP(op, extras);               Push(sp, res); return sp; }
#define FOP(op, extras) { GETARGS(); _FOP(op, extras);               Push(sp, res); return sp; }
#define LOP(op)         { GETARGS(); auto res = a.ip() op b.ip();    Push(sp, res); return sp; }

#define IVVOP(op, extras) { _IVOPV(op, extras, TopPtr(sp) - len); return sp; }
#define FVVOP(op, extras) { _FVOPV(op, extras, TopPtr(sp) - len); return sp; }
#define IVSOP(op, extras) { _IVOPS(op, extras, TopPtr(sp) - len); return sp; }
#define FVSOP(op, extras) { _FVOPS(op, extras, TopPtr(sp) - len); return sp; }

#define SOP(op) { GETARGS(); Value res = *a.sval() op *b.sval(); Push(sp, res); return sp; }
#define SCAT()  { GETARGS(); _SCAT();                            Push(sp, res); return sp; }

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

VM_INLINE StackPtr U_IVVADD(VM &vm, StackPtr sp, int len) { IVVOP(+,  0);  }
VM_INLINE StackPtr U_IVVSUB(VM &vm, StackPtr sp, int len) { IVVOP(-,  0);  }
VM_INLINE StackPtr U_IVVMUL(VM &vm, StackPtr sp, int len) { IVVOP(*,  0);  }
VM_INLINE StackPtr U_IVVDIV(VM &vm, StackPtr sp, int len) { IVVOP(/,  1);  }
VM_INLINE StackPtr U_IVVMOD(VM &vm, StackPtr sp, int len) { IVVOP(% , 1); }
VM_INLINE StackPtr U_IVVLT(VM &vm, StackPtr sp, int len)  { IVVOP(<,  0);  }
VM_INLINE StackPtr U_IVVGT(VM &vm, StackPtr sp, int len)  { IVVOP(>,  0);  }
VM_INLINE StackPtr U_IVVLE(VM &vm, StackPtr sp, int len)  { IVVOP(<=, 0);  }
VM_INLINE StackPtr U_IVVGE(VM &vm, StackPtr sp, int len)  { IVVOP(>=, 0);  }
VM_INLINE StackPtr U_FVVADD(VM &vm, StackPtr sp, int len) { FVVOP(+,  0);  }
VM_INLINE StackPtr U_FVVSUB(VM &vm, StackPtr sp, int len) { FVVOP(-,  0);  }
VM_INLINE StackPtr U_FVVMUL(VM &vm, StackPtr sp, int len) { FVVOP(*,  0);  }
VM_INLINE StackPtr U_FVVDIV(VM &vm, StackPtr sp, int len) { FVVOP(/,  1);  }
VM_INLINE StackPtr U_FVVMOD(VM &vm, StackPtr sp, int len) { FVVOP(/ , 3); }
VM_INLINE StackPtr U_FVVLT(VM &vm, StackPtr sp, int len)  { FVVOP(<,  0); }
VM_INLINE StackPtr U_FVVGT(VM &vm, StackPtr sp, int len)  { FVVOP(>,  0); }
VM_INLINE StackPtr U_FVVLE(VM &vm, StackPtr sp, int len)  { FVVOP(<=, 0); }
VM_INLINE StackPtr U_FVVGE(VM &vm, StackPtr sp, int len)  { FVVOP(>=, 0); }

VM_INLINE StackPtr U_IVSADD(VM &vm, StackPtr sp, int len) { IVSOP(+,  0);  }
VM_INLINE StackPtr U_IVSSUB(VM &vm, StackPtr sp, int len) { IVSOP(-,  0);  }
VM_INLINE StackPtr U_IVSMUL(VM &vm, StackPtr sp, int len) { IVSOP(*,  0);  }
VM_INLINE StackPtr U_IVSDIV(VM &vm, StackPtr sp, int len) { IVSOP(/,  1);  }
VM_INLINE StackPtr U_IVSMOD(VM &vm, StackPtr sp, int len) { IVSOP(% , 1); }
VM_INLINE StackPtr U_IVSLT(VM &vm, StackPtr sp, int len)  { IVSOP(<,  0);  }
VM_INLINE StackPtr U_IVSGT(VM &vm, StackPtr sp, int len)  { IVSOP(>,  0);  }
VM_INLINE StackPtr U_IVSLE(VM &vm, StackPtr sp, int len)  { IVSOP(<=, 0);  }
VM_INLINE StackPtr U_IVSGE(VM &vm, StackPtr sp, int len)  { IVSOP(>=, 0);  }
VM_INLINE StackPtr U_FVSADD(VM &vm, StackPtr sp, int len) { FVSOP(+,  0);  }
VM_INLINE StackPtr U_FVSSUB(VM &vm, StackPtr sp, int len) { FVSOP(-,  0);  }
VM_INLINE StackPtr U_FVSMUL(VM &vm, StackPtr sp, int len) { FVSOP(*,  0);  }
VM_INLINE StackPtr U_FVSDIV(VM &vm, StackPtr sp, int len) { FVSOP(/,  1);  }
VM_INLINE StackPtr U_FVSMOD(VM &vm, StackPtr sp, int len) { FVSOP(/ , 3); }
VM_INLINE StackPtr U_FVSLT(VM &vm, StackPtr sp, int len)  { FVSOP(<,  0); }
VM_INLINE StackPtr U_FVSGT(VM &vm, StackPtr sp, int len)  { FVSOP(>,  0); }
VM_INLINE StackPtr U_FVSLE(VM &vm, StackPtr sp, int len)  { FVSOP(<=, 0); }
VM_INLINE StackPtr U_FVSGE(VM &vm, StackPtr sp, int len)  { FVSOP(>=, 0); }

VM_INLINE StackPtr U_AEQ(VM &, StackPtr sp)  { ACOMPEN(==); }
VM_INLINE StackPtr U_ANE(VM &, StackPtr sp)  { ACOMPEN(!=); }
VM_INLINE StackPtr U_STEQ(VM &, StackPtr sp, int len) { STCOMPEN(==, true, &&); }
VM_INLINE StackPtr U_STNE(VM &, StackPtr sp, int len) { STCOMPEN(!=, false, ||); }
VM_INLINE StackPtr U_LEQ(VM &, StackPtr sp) { LOP(==); }
VM_INLINE StackPtr U_LNE(VM &, StackPtr sp) { LOP(!=); }

VM_INLINE StackPtr U_IADD(VM &vm, StackPtr sp) { IOP(+,  0); }
VM_INLINE StackPtr U_ISUB(VM &vm, StackPtr sp) { IOP(-,  0); }
VM_INLINE StackPtr U_IMUL(VM &vm, StackPtr sp) { IOP(*,  0); }
VM_INLINE StackPtr U_IDIV(VM &vm, StackPtr sp) { IOP(/ , 1); }
VM_INLINE StackPtr U_IMOD(VM &vm, StackPtr sp) { IOP(%,  1); }
VM_INLINE StackPtr U_ILT(VM &vm, StackPtr sp)  { IOP(<,  0); }
VM_INLINE StackPtr U_IGT(VM &vm, StackPtr sp)  { IOP(>,  0); }
VM_INLINE StackPtr U_ILE(VM &vm, StackPtr sp)  { IOP(<=, 0); }
VM_INLINE StackPtr U_IGE(VM &vm, StackPtr sp)  { IOP(>=, 0); }
VM_INLINE StackPtr U_IEQ(VM &vm, StackPtr sp)  { IOP(==, 0); }
VM_INLINE StackPtr U_INE(VM &vm, StackPtr sp)  { IOP(!=, 0); }

VM_INLINE StackPtr U_FADD(VM &vm, StackPtr sp) { FOP(+,  0); }
VM_INLINE StackPtr U_FSUB(VM &vm, StackPtr sp) { FOP(-,  0); }
VM_INLINE StackPtr U_FMUL(VM &vm, StackPtr sp) { FOP(*,  0); }
VM_INLINE StackPtr U_FDIV(VM &vm, StackPtr sp) { FOP(/,  1); }
VM_INLINE StackPtr U_FMOD(VM &vm, StackPtr sp) { FOP(/,  3); }
VM_INLINE StackPtr U_FLT(VM &vm, StackPtr sp)  { FOP(<,  0); }
VM_INLINE StackPtr U_FGT(VM &vm, StackPtr sp)  { FOP(>,  0); }
VM_INLINE StackPtr U_FLE(VM &vm, StackPtr sp)  { FOP(<=, 0); }
VM_INLINE StackPtr U_FGE(VM &vm, StackPtr sp)  { FOP(>=, 0); }
VM_INLINE StackPtr U_FEQ(VM &vm, StackPtr sp)  { FOP(==, 0); }
VM_INLINE StackPtr U_FNE(VM &vm, StackPtr sp)  { FOP(!=, 0); }

VM_INLINE StackPtr U_SADD(VM &vm, StackPtr sp) { SCAT();  }
VM_INLINE StackPtr U_SSUB(VM &vm, StackPtr sp) { VMASSERT(vm, 0); return sp; }
VM_INLINE StackPtr U_SMUL(VM &vm, StackPtr sp) { VMASSERT(vm, 0); return sp; }
VM_INLINE StackPtr U_SDIV(VM &vm, StackPtr sp) { VMASSERT(vm, 0); return sp; }
VM_INLINE StackPtr U_SMOD(VM &vm, StackPtr sp) { VMASSERT(vm, 0); return sp; }
VM_INLINE StackPtr U_SLT(VM &, StackPtr sp)  { SOP(<);  }
VM_INLINE StackPtr U_SGT(VM &, StackPtr sp)  { SOP(>);  }
VM_INLINE StackPtr U_SLE(VM &, StackPtr sp)  { SOP(<=); }
VM_INLINE StackPtr U_SGE(VM &, StackPtr sp)  { SOP(>=); }
VM_INLINE StackPtr U_SEQ(VM &, StackPtr sp)  { SOP(==); }
VM_INLINE StackPtr U_SNE(VM &, StackPtr sp)  { SOP(!=); }

VM_INLINE StackPtr U_IUMINUS(VM &, StackPtr sp) { Value a = Pop(sp); Push(sp, Value(-a.ival())); return sp; }
VM_INLINE StackPtr U_FUMINUS(VM &, StackPtr sp) { Value a = Pop(sp); Push(sp, Value(-a.fval())); return sp; }

VM_INLINE StackPtr U_IVUMINUS(VM &vm, StackPtr sp, int len) {
    auto vec = TopPtr(sp) - len;
    for (int i = 0; i < len; i++) {
        auto &a = vec[i];
        VMTYPEEQ(a, V_INT);
        a = -a.ival();
    }
    return sp;
}
VM_INLINE StackPtr U_FVUMINUS(VM &vm, StackPtr sp, int len) {
    auto vec = TopPtr(sp) - len;
    for (int i = 0; i < len; i++) {
        auto &a = vec[i];
        VMTYPEEQ(a, V_FLOAT);
        a = -a.fval();
    }
    return sp;
}

VM_INLINE StackPtr U_LOGNOT(VM &, StackPtr sp) {
    Value a = Pop(sp);
    Push(sp, a.False());
    return sp;
}
VM_INLINE StackPtr U_LOGNOTREF(VM &, StackPtr sp) {
    Value a = Pop(sp);
    bool b = a.True();
    Push(sp, !b);
    return sp;
}

#define BITOP(op) { GETARGS(); Push(sp, a.ival() op b.ival()); return sp; }
VM_INLINE StackPtr U_BINAND(VM &, StackPtr sp) { BITOP(&);  }
VM_INLINE StackPtr U_BINOR(VM &, StackPtr sp)  { BITOP(|);  }
VM_INLINE StackPtr U_XOR(VM &, StackPtr sp)    { BITOP(^);  }
VM_INLINE StackPtr U_ASL(VM &, StackPtr sp)    { BITOP(<<); }
VM_INLINE StackPtr U_ASR(VM &, StackPtr sp)    { BITOP(>>); }
VM_INLINE StackPtr U_NEG(VM &, StackPtr sp)    { auto a = Pop(sp); Push(sp, ~a.ival()); return sp; }

VM_INLINE StackPtr U_I2F(VM &vm, StackPtr sp) {
    Value a = Pop(sp);
    VMTYPEEQ(a, V_INT);
    Push(sp, (float)a.ival());
    return sp;
}

VM_INLINE StackPtr U_A2S(VM &vm, StackPtr sp, int ty) {
    Value a = Pop(sp);
    Push(sp, vm.ToString(a, vm.GetTypeInfo((type_elem_t)ty)));
    return sp;
}

VM_INLINE StackPtr U_ST2S(VM &vm, StackPtr sp, int ty) {
    auto &ti = vm.GetTypeInfo((type_elem_t)ty);
    PopN(sp, ti.len);
    auto top = TopPtr(sp);
    Push(sp, vm.StructToString(top, ti));
    return sp;
}

VM_INLINE StackPtr U_E2B(VM &, StackPtr sp) {
    Value a = Pop(sp);
    Push(sp, a.True());
    return sp;
}

VM_INLINE StackPtr U_E2BREF(VM &, StackPtr sp) {
    Value a = Pop(sp);
    Push(sp, a.True());
    return sp;
}

VM_INLINE StackPtr U_PUSHVAR(VM &vm, StackPtr sp, int vidx) {
    Push(sp, vm.vars[vidx]);
    return sp;
}

VM_INLINE StackPtr U_PUSHVARV(VM &vm, StackPtr sp, int vidx, int l) {
    tsnz_memcpy(TopPtr(sp), &vm.vars[vidx], l);
    PushN(sp, l);
    return sp;
}

VM_INLINE StackPtr U_PUSHFLD(VM &vm, StackPtr sp, int i) {
    Value r = Pop(sp);
    VMASSERT(vm, r.ref());
    assert(i < r.oval()->Len(vm));
    Push(sp, r.oval()->AtS(i));
    return sp;
}
VM_INLINE StackPtr U_PUSHFLDMREF(VM &vm, StackPtr sp, int i) {
    Value r = Pop(sp);
    if (!r.ref()) {
        Push(sp, r);
    } else {
        assert(i < r.oval()->Len(vm));
        (void)vm;
        Push(sp, r.oval()->AtS(i));
    }
    return sp;
}
VM_INLINE StackPtr U_PUSHFLD2V(VM &vm, StackPtr sp, int i, int l) {
    Value r = Pop(sp);
    VMASSERT(vm, r.ref());
    assert(i + l <= r.oval()->Len(vm));
    tsnz_memcpy(TopPtr(sp), &r.oval()->AtS(i), l);
    PushN(sp, l);
    return sp;
}
VM_INLINE StackPtr U_PUSHFLDV(VM &, StackPtr sp, int i, int l) {
    PopN(sp, l);
    auto val = *(TopPtr(sp) + i);
    Push(sp, val);
    return sp;
}
VM_INLINE StackPtr U_PUSHFLDV2V(VM &, StackPtr sp, int i, int rl, int l) {
    PopN(sp, l);
    t_memmove(TopPtr(sp), TopPtr(sp) + i, rl);
    PushN(sp, rl);
    return sp;
}

VM_INLINE StackPtr U_VPUSHIDXI(VM &vm, StackPtr sp) {
    auto x = Pop(sp).ival();
    PushDerefIdxVector1(vm, sp, x);
    return sp;
}

VM_INLINE StackPtr U_VPUSHIDXI2V(VM &vm, StackPtr sp) {
    auto x = Pop(sp).ival();
    PushDerefIdxVector2V(vm, sp, x);
    return sp;
}

VM_INLINE StackPtr U_VPUSHIDXV(VM &vm, StackPtr sp, int l) {
    auto x = vm.GrabIndex(sp, l);
    PushDerefIdxVector2V(vm, sp, x);
    return sp;
}

VM_INLINE StackPtr U_VPUSHIDXIS(VM &vm, StackPtr sp, int o) {
    auto x = Pop(sp).ival();
    PushDerefIdxVectorSub1(vm, sp, x, o);
    return sp;
}

VM_INLINE StackPtr U_VPUSHIDXIS2V(VM &vm, StackPtr sp, int w, int o) {
    auto x = Pop(sp).ival();
    PushDerefIdxVectorSub2V(vm, sp, x, w, o);
    return sp;
}

VM_INLINE StackPtr U_VPUSHIDXVS(VM &vm, StackPtr sp, int l, int w, int o) {
    auto x = vm.GrabIndex(sp, l);
    PushDerefIdxVectorSub2V(vm, sp, x, w, o);
    return sp;
}

VM_INLINE StackPtr U_NPUSHIDXI(VM &, StackPtr sp, int l) {
    auto x = Pop(sp).ival();
    PushDerefIdxStruct(sp, x, l);
    return sp;
}

VM_INLINE StackPtr U_SPUSHIDXI(VM &vm, StackPtr sp) {
    auto x = Pop(sp).ival();
    PushDerefIdxString(vm, sp, x);
    return sp;
}

VM_INLINE StackPtr U_NATIVEHINT(VM &, StackPtr sp, int) {
    assert(false);
    return sp;
}

VM_INLINE StackPtr U_JUMP(VM &, StackPtr sp) {
    assert(false);
    return sp;
}

VM_INLINE StackPtr U_JUMPFAIL(VM &, StackPtr sp) {
    return sp;
}

VM_INLINE StackPtr U_JUMPFAILR(VM &, StackPtr sp) {
    auto x = Top(sp);
    Push(sp, x);
    return sp;
}

VM_INLINE StackPtr U_JUMPNOFAIL(VM &, StackPtr sp) {
    auto x = Pop(sp);
    Push(sp, x.False());
    return sp;
}

VM_INLINE StackPtr U_JUMPNOFAILR(VM &, StackPtr sp) {
    auto x = Top(sp);
    Push(sp, x.False());
    return sp;
}

VM_INLINE StackPtr U_JUMPIFUNWOUND(VM &vm, StackPtr sp, int df) {
    Push(sp, vm.ret_unwind_to != df);
    return sp;
}

VM_INLINE StackPtr U_JUMP_TABLE(VM &, StackPtr sp, const int *) {
    assert(false);
    return sp;
}

VM_INLINE StackPtr U_ISTYPE(VM &vm, StackPtr sp, int ty) {
    auto to = (type_elem_t)ty;
    auto v = Pop(sp);
    // Optimizer guarantees we don't have to deal with scalars.
    if (v.refnil()) Push(sp, v.ref()->tti == to);
    else Push(sp, vm.GetTypeInfo(to).t == V_NIL);  // FIXME: can replace by fixed type_elem_t ?
    return sp;
}

VM_INLINE StackPtr U_ABORT(VM &vm, StackPtr sp) {
    vm.SeriousError("VM internal error: abort");
    return sp;
}

VM_INLINE StackPtr U_LVAL_VAR(VM &vm, StackPtr sp, int vidx) {
    vm.temp_lval = &vm.vars[vidx];
    return sp;
}


VM_INLINE StackPtr U_LVAL_FLD(VM &vm, StackPtr sp, int i) {
    vm.temp_lval = &GetFieldLVal(vm, sp, i);
    return sp;
}

VM_INLINE StackPtr U_LVAL_IDXVI(VM &vm, StackPtr sp) {
    auto x = Pop(sp).ival();
    vm.temp_lval = &GetVecLVal(vm, sp, x);
    return sp;
}

VM_INLINE StackPtr U_LVAL_IDXVV(VM &vm, StackPtr sp, int l) {
    auto x = vm.GrabIndex(sp, l);
    vm.temp_lval = &GetVecLVal(vm, sp, x);
    return sp;
}

// Class accessed by index.
VM_INLINE StackPtr U_LVAL_IDXNI(VM &vm, StackPtr sp) {
    auto x = Pop(sp).ival();
    vm.temp_lval = &GetFieldILVal(vm, sp, x);
    return sp;
}

VM_INLINE StackPtr U_LV_DUP(VM &vm, StackPtr sp) {
    Push(sp, *vm.temp_lval);
    return sp;
}

VM_INLINE StackPtr U_LV_DUPV(VM &vm, StackPtr sp, int l) {
    tsnz_memcpy(TopPtr(sp), vm.temp_lval, l);
    PushN(sp, l);
    return sp;
}

/*
VM_INLINE StackPtr U_LV_DUPREF(VM &vm, StackPtr sp) {
    vm.temp_lval->LTINCRTNIL();
    Push(sp, *vm.temp_lval);
    return sp;
}

VM_INLINE StackPtr U_LV_DUPREFV(VM &vm, StackPtr sp, int l) {
    tsnz_memcpy(TopPtr(sp), vm.temp_lval, l);
    for (int i = 0; i < l; i++) {
        sp++;
        sp->LTINCRTNIL();
    }
    return sp;
}
*/

#define LVALCASES(N, B) VM_INLINE StackPtr U_LV_##N(VM &vm, StackPtr &sp) { \
    auto &a = *vm.temp_lval; Value b = Pop(sp); B; return sp; }

#define LVALCASER(N, B) VM_INLINE StackPtr U_LV_##N(VM &vm, StackPtr &sp, int len) { \
    auto &fa = *vm.temp_lval; B; return sp; }

LVALCASER(IVVADD , _IVOPV(+, 0, &fa))
LVALCASER(IVVSUB , _IVOPV(-, 0, &fa))
LVALCASER(IVVMUL , _IVOPV(*, 0, &fa))
LVALCASER(IVVDIV , _IVOPV(/, 1, &fa))
LVALCASER(IVVMOD,  VMASSERT(vm, 0); (void)fa; (void)len; (void)sp)

LVALCASER(FVVADD , _FVOPV(+, 0, &fa))
LVALCASER(FVVSUB , _FVOPV(-, 0, &fa))
LVALCASER(FVVMUL , _FVOPV(*, 0, &fa))
LVALCASER(FVVDIV , _FVOPV(/, 1, &fa))

LVALCASER(IVSADD , _IVOPS(+, 0, &fa))
LVALCASER(IVSSUB , _IVOPS(-, 0, &fa))
LVALCASER(IVSMUL , _IVOPS(*, 0, &fa))
LVALCASER(IVSDIV , _IVOPS(/, 1, &fa))
LVALCASER(IVSMOD , VMASSERT(vm, 0); (void)fa; (void)len; (void)sp)

LVALCASER(FVSADD , _FVOPS(+, 0, &fa))
LVALCASER(FVSSUB , _FVOPS(-, 0, &fa))
LVALCASER(FVSMUL , _FVOPS(*, 0, &fa))
LVALCASER(FVSDIV , _FVOPS(/, 1, &fa))

LVALCASES(IADD   , _IOP(+, 0); a = res;)
LVALCASES(ISUB   , _IOP(-, 0); a = res;)
LVALCASES(IMUL   , _IOP(*, 0); a = res;)
LVALCASES(IDIV   , _IOP(/, 1); a = res;)
LVALCASES(IMOD   , _IOP(%, 1); a = res;)

LVALCASES(BINAND , _IOP(&,  0); a = res;)
LVALCASES(BINOR  , _IOP(|,  0); a = res;)
LVALCASES(XOR    , _IOP(^,  0); a = res;)
LVALCASES(ASL    , _IOP(<<, 0); a = res;)
LVALCASES(ASR    , _IOP(>>, 0); a = res;)

LVALCASES(FADD   , _FOP(+, 0); a = res;)
LVALCASES(FSUB   , _FOP(-, 0); a = res;)
LVALCASES(FMUL   , _FOP(*, 0); a = res;)
LVALCASES(FDIV   , _FOP(/, 1); a = res;)

VM_INLINE StackPtr U_LV_SADD(VM &vm, StackPtr &sp) {
    auto &a = *vm.temp_lval;
    Value b = Pop(sp);
    _SCAT();
    a.LTDECRTNIL(vm);
    a = res;
    return sp;
}

VM_INLINE StackPtr U_LV_WRITE(VM &vm, StackPtr &sp) {
    auto &a = *vm.temp_lval;
    auto  b = Pop(sp);
    TYPE_ASSERT(a.type == b.type || a.type == V_NIL || b.type == V_NIL);
    a = b;
    return sp;
}

VM_INLINE StackPtr U_LV_WRITEREF(VM &vm, StackPtr &sp) {
    auto &a = *vm.temp_lval;
    auto  b = Pop(sp);
    a.LTDECRTNIL(vm);
    TYPE_ASSERT(a.type == b.type || a.type == V_NIL || b.type == V_NIL);
    a = b;
    return sp;
}

VM_INLINE StackPtr U_LV_WRITEV(VM &vm, StackPtr &sp, int l) {
    auto &a = *vm.temp_lval;
    auto b = TopPtr(sp) - l;
    tsnz_memcpy(&a, b, l);
    PopN(sp, l);
    return sp;
}

VM_INLINE StackPtr U_LV_WRITEREFV(VM &vm, StackPtr &sp, int l) {
    auto &a = *vm.temp_lval;
    for (int i = 0; i < l; i++) (&a)[i].LTDECRTNIL(vm);
    auto b = TopPtr(sp) - l;
    tsnz_memcpy(&a, b, l);
    PopN(sp, l);
    return sp;
}

VM_INLINE StackPtr U_LV_IPP(VM &vm, StackPtr &sp) {
    auto &a = *vm.temp_lval;
    a.setival(a.ival() + 1);
    return sp;
}

VM_INLINE StackPtr U_LV_IMM(VM & vm, StackPtr & sp) {
    auto &a = *vm.temp_lval;
    a.setival(a.ival() - 1);
    return sp;
}

VM_INLINE StackPtr U_LV_FPP(VM & vm, StackPtr & sp) {
    auto &a = *vm.temp_lval;
    a.setfval(a.fval() + 1);
    return sp;
}

VM_INLINE StackPtr U_LV_FMM(VM & vm, StackPtr & sp) {
    auto &a = *vm.temp_lval;
    a.setfval(a.fval() - 1);
    return sp;
}

}  // namespace lobster
