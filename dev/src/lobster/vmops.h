#include "lobster/geom.h"
#include "lobster/vmdata.h"
#include "lobster/natreg.h"
#include "lobster/bytecode_generated.h"

namespace lobster {

#define VM_OP_ARGS0
#define VM_OP_ARGS1 int _a
#define VM_OP_ARGS2 int _a, int _b
#define VM_OP_ARGS3 int _a, int _b, int _c
#define VM_OP_ARGS9 VM_OP_ARGS  // ILUNKNOWNARITY
#define VM_OP_ARGSN(N) VM_OP_ARGS##N
#define VM_OP_DEFS0
#define VM_OP_DEFS1 int _a = *vm.ip++;
#define VM_OP_DEFS2 int _a = *vm.ip++; int _b = *vm.ip++;
#define VM_OP_DEFS3 int _a = *vm.ip++; int _b = *vm.ip++; int _c = *vm.ip++;
#define VM_OP_DEFS9  // ILUNKNOWNARITY
#define VM_OP_DEFSN(N) VM_OP_DEFS##N (void)vm.ip;
#define VM_OP_PASS0
#define VM_OP_PASS1 _a
#define VM_OP_PASS2 _a, _b
#define VM_OP_PASS3 _a, _b, _c
#define VM_OP_PASS9 VM_IP_PASS_THRU  // ILUNKNOWNARITY
#define VM_OP_PASSN(N) VM_OP_PASS##N
#define VM_COMMA_0
#define VM_COMMA_1 ,
#define VM_COMMA_2 ,
#define VM_COMMA_3 ,
#define VM_COMMA_9 VM_COMMA
#define VM_COMMA_IF(N) VM_COMMA_##N
#define VM_CCOMMA_0
#define VM_CCOMMA_1 VM_COMMA
#define VM_CCOMMA_2 VM_COMMA
#define VM_CCOMMA_3 VM_COMMA
#define VM_CCOMMA_9 VM_COMMA
#define VM_CCOMMA_IF(N) VM_CCOMMA_##N

#if RTT_ENABLED
    #define VMTYPEEQ(val, vt) VMASSERT(vm, (val).type == (vt))
#else
    #define VMTYPEEQ(val, vt) { (void)(val); (void)(vt); (void)vm; }
#endif

VM_INLINE void PushDerefIdxVector(VM &vm, StackPtr &sp, iint i) {
    Value r = Pop(sp);
    VMASSERT(vm, r.ref());
    auto v = r.vval();
    RANGECHECK(vm, i, v->len, v);
    v->AtVW(sp, i);
}

VM_INLINE void PushDerefIdxVectorSub(VM &vm, StackPtr &sp, iint i, int width, int offset) {
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

VM_INLINE Value &GetLocLVal(VM &vm, StackPtr &sp, int i) {
    Value coro = Pop(sp);
    VMTYPEEQ(coro, V_COROUTINE);
    return coro.cval()->GetVar(vm, sp, i);
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

VM_INLINE StackPtr U_PUSHFUN(VM &, StackPtr sp, int start VM_COMMA VM_OP_ARGS_CALL) {
    #ifdef VM_COMPILED_CODE_MODE
        (void)start;
    #else
        auto fcont = start;
    #endif
    Push(sp, Value(InsPtr(fcont)));
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

VM_INLINE StackPtr U_CALL(VM &vm, StackPtr sp, int f VM_COMMA VM_OP_ARGS_CALL) {
    #ifdef VM_COMPILED_CODE_MODE
        (void)f;
        block_base_t fun = 0;  // Dynamic calls need this set, but for CALL it is ignored.
    #else
        auto fun = f;
        auto fcont = vm.ip - vm.codestart;
    #endif
    vm.StartStackFrame(InsPtr(fcont));
    vm.FunIntroPre(sp, InsPtr(fun));
    return sp;
}

VM_INLINE StackPtr U_CALLV(VM &vm, StackPtr sp VM_COMMA VM_OP_ARGS_CALL) {
    Value fun = Pop(sp);
    VMTYPEEQ(fun, V_FUNCTION);
    #ifndef VM_COMPILED_CODE_MODE
        auto fcont = vm.ip - vm.codestart;
    #endif
    vm.StartStackFrame(InsPtr(fcont));
    vm.FunIntroPre(sp, fun.ip());
    return sp;
}

VM_INLINE StackPtr U_CALLVCOND(VM &vm, StackPtr sp VM_COMMA VM_OP_ARGS_CALL) {
    // FIXME: don't need to check for function value again below if false
    if (Top(sp).False()) {
        Pop(sp);
        #ifdef VM_COMPILED_CODE_MODE
            vm.next_call_target = 0;
        #endif
    } else {
        sp = U_CALLV(vm, sp VM_COMMA VM_FC_PASS_THRU);
    }
    return sp;
}

VM_INLINE StackPtr U_DDCALL(VM &vm, StackPtr sp, int vtable_idx, int stack_idx VM_COMMA VM_OP_ARGS_CALL) {
    auto self = TopM(sp, stack_idx);
    VMTYPEEQ(self, V_CLASS);
    auto start = self.oval()->ti(vm).vtable_start;
    auto fun = vm.vtables[start + vtable_idx];
    #ifdef VM_COMPILED_CODE_MODE
    #else
        auto fcont = vm.ip - vm.codestart;
        assert(fun.f >= 0);
    #endif
    vm.StartStackFrame(InsPtr(fcont));
    vm.FunIntroPre(sp, fun);
    return sp;
}

VM_INLINE StackPtr U_FUNSTART(VM &vm, StackPtr sp VM_COMMA VM_OP_ARGS) {
    #ifdef VM_COMPILED_CODE_MODE
        vm.FunIntro(sp, ip);
    #else
        VMASSERT(vm, false);
    #endif
    return sp;
}

VM_INLINE StackPtr U_RETURN(VM &vm, StackPtr sp, int df, int nrv) {
    vm.FunOut(sp, df, nrv);
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

VM_INLINE StackPtr ForLoop(VM &vm, StackPtr sp, iint len) {
    #ifndef VM_COMPILED_CODE_MODE
        auto cont = *vm.ip++;
    #endif
    auto &i = TopM(sp, 1);
    TYPE_ASSERT(i.type == V_INT);
    i.setival(i.ival() + 1);
    if (i.ival() < len) {
        #ifdef VM_COMPILED_CODE_MODE
            Push(sp, false);
            (void)vm;
        #else
            vm.ip = cont + vm.codestart;
        #endif
    } else {
        (void)Pop(sp); /* iter */
        (void)Pop(sp); /* i */
        #ifdef VM_COMPILED_CODE_MODE
            Push(sp, true);
        #endif
    }
    return sp;
}

#define FORELEM(L) \
    auto &iter = Top(sp); \
    auto i = TopM(sp, 1).ival(); \
    assert(i < L); \

VM_INLINE StackPtr U_IFOR(VM &vm, StackPtr sp) { return ForLoop(vm, sp, Top(sp).ival()); return sp; }
VM_INLINE StackPtr U_VFOR(VM &vm, StackPtr sp) { return ForLoop(vm, sp, Top(sp).vval()->len); return sp; }
VM_INLINE StackPtr U_SFOR(VM &vm, StackPtr sp) { return ForLoop(vm, sp, Top(sp).sval()->len); return sp; }

VM_INLINE StackPtr U_IFORELEM(VM &, StackPtr sp)    { FORELEM(iter.ival()); (void)iter; Push(sp, i); return sp; }
VM_INLINE StackPtr U_VFORELEM(VM &, StackPtr sp)    { FORELEM(iter.vval()->len); iter.vval()->AtVW(sp, i); return sp; }
VM_INLINE StackPtr U_VFORELEMREF(VM &, StackPtr sp) { FORELEM(iter.vval()->len); auto el = iter.vval()->At(i); el.LTINCRTNIL(); Push(sp, el); return sp; }
VM_INLINE StackPtr U_SFORELEM(VM &, StackPtr sp)    { FORELEM(iter.sval()->len); Push(sp, Value(((uint8_t *)iter.sval()->data())[i])); return sp; }

VM_INLINE StackPtr U_FORLOOPI(VM &, StackPtr sp) {
    auto &i = TopM(sp, 1);  // This relies on for being inlined, otherwise it would be 2.
    TYPE_ASSERT(i.type == V_INT);
    Push(sp, i);
    return sp;
}

VM_INLINE StackPtr U_BCALLRETV(VM &vm, StackPtr sp, int nfi) {
    vm.BCallProf();
    auto nf = vm.nfr.nfuns[nfi];
    nf->fun.fV(sp, vm);
    return sp;
}
VM_INLINE StackPtr U_BCALLREFV(VM &vm, StackPtr sp, int nfi) {
    vm.BCallProf();
    auto nf = vm.nfr.nfuns[nfi];
    nf->fun.fV(sp, vm);
    // This can only pop a single value, not called for structs.
    Pop(sp).LTDECRTNIL(vm);
    return sp;
}
VM_INLINE StackPtr U_BCALLUNBV(VM &vm, StackPtr sp, int nfi) {
    vm.BCallProf();
    auto nf = vm.nfr.nfuns[nfi];
    nf->fun.fV(sp, vm);
    // This can only pop a single value, not called for structs.
    Pop(sp);
    return sp;
}

#define BCALLOPH(PRE,N,DECLS,ARGS,RETOP) VM_INLINE StackPtr U_BCALL##PRE##N(VM &vm, StackPtr sp, int nfi) { \
    vm.BCallProf(); \
    auto nf = vm.nfr.nfuns[nfi]; \
    DECLS; \
    Value v = nf->fun.f##N ARGS; \
    RETOP; \
    return sp; \
}

#define BCALLOP(N,DECLS,ARGS) \
    BCALLOPH(RET,N,DECLS,ARGS,Push(sp, v);vm.BCallRetCheck(sp, nf)) \
    BCALLOPH(REF,N,DECLS,ARGS,v.LTDECRTNIL(vm)) \
    BCALLOPH(UNB,N,DECLS,ARGS,(void)v)

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
            #ifdef VM_COMPILED_CODE_MODE
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

#define _VOP(op, extras, V_T, field, withscalar, geta) { \
    if (withscalar) { \
        auto b = Pop(sp); \
        VMTYPEEQ(b, V_T) \
        auto veca = geta; \
        for (int j = 0; j < len; j++) { \
            auto &a = veca[j]; \
            VMTYPEEQ(a, V_T) \
            auto bv = b.field(); \
            TYPEOP(op, extras, a.field(), bv, a) \
        } \
    } else { \
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

#define _IVOP(op, extras, withscalar, geta) _VOP(op, extras, V_INT, ival, withscalar, geta)
#define _FVOP(op, extras, withscalar, geta) _VOP(op, extras, V_FLOAT, fval, withscalar, geta)

#define _SCAT() Value res = vm.NewString(a.sval()->strv(), b.sval()->strv())

#define ACOMPEN(op)        { GETARGS(); Value res = a.any() op b.any();  Push(sp, res); return sp; }
#define IOP(op, extras)    { GETARGS(); _IOP(op, extras);                Push(sp, res); return sp; }
#define FOP(op, extras)    { GETARGS(); _FOP(op, extras);                Push(sp, res); return sp; }

#define LOP(op)            { GETARGS(); auto res = a.ip() op b.ip();     Push(sp, res); return sp; }

#define IVVOP(op, extras)  { _IVOP(op, extras, false, TopPtr(sp) - len); return sp; }
#define FVVOP(op, extras)  { _FVOP(op, extras, false, TopPtr(sp) - len); return sp; }
#define IVSOP(op, extras)  { _IVOP(op, extras, true, TopPtr(sp) - len);  return sp; }
#define FVSOP(op, extras)  { _FVOP(op, extras, true, TopPtr(sp) - len);  return sp; }

#define SOP(op)            { GETARGS(); Value res = *a.sval() op *b.sval(); Push(sp, res); return sp; }
#define SCAT()             { GETARGS(); _SCAT();                            Push(sp, res); return sp; }

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
    PushDerefIdxVector(vm, sp, x);
    return sp;
}

VM_INLINE StackPtr U_VPUSHIDXV(VM &vm, StackPtr sp, int l) {
    auto x = vm.GrabIndex(sp, l);
    PushDerefIdxVector(vm, sp, x);
    return sp;
}

VM_INLINE StackPtr U_VPUSHIDXIS(VM &vm, StackPtr sp, int w, int o) {
    auto x = Pop(sp).ival();
    PushDerefIdxVectorSub(vm, sp, x, w, o);
    return sp;
}

VM_INLINE StackPtr U_VPUSHIDXVS(VM &vm, StackPtr sp, int l, int w, int o) {
    auto x = vm.GrabIndex(sp, l);
    PushDerefIdxVectorSub(vm, sp, x, w, o);
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

VM_INLINE StackPtr U_PUSHLOC(VM &vm, StackPtr sp, int i) {
    auto coro = Pop(sp).cval();
    auto v = coro->GetVar(vm, sp, i);
    Push(sp, v);
    return sp;
}

VM_INLINE StackPtr U_PUSHLOCV(VM &vm, StackPtr sp, int i, int l) {
    auto coro = Pop(sp).cval();
    auto v = &coro->GetVar(vm, sp, i);
    tsnz_memcpy(TopPtr(sp), v, l);
    PushN(sp, l);
    return sp;
}

#ifdef VM_COMPILED_CODE_MODE
VM_INLINE StackPtr U_JUMP(VM &, StackPtr sp) { Push(sp, false); return sp; }
VM_INLINE StackPtr U_JUMPFAIL(VM &, StackPtr sp) { return sp; }
VM_INLINE StackPtr U_JUMPFAILR(VM &, StackPtr sp) { auto x = Top(sp); if (x.False()) Push(sp, x); return sp; }
VM_INLINE StackPtr U_JUMPNOFAIL(VM &, StackPtr sp) { auto x = Pop(sp); Push(sp, x.False()); return sp; }
VM_INLINE StackPtr U_JUMPNOFAILR(VM &, StackPtr sp) { auto x = Pop(sp); if (x.True()) Push(sp, x); Push(sp, x.False()); return sp; }
#else
VM_INLINE StackPtr U_JUMP(VM &vm, StackPtr sp) { auto nip = *vm.ip++; vm.ip = vm.codestart + nip; return sp; }
VM_INLINE StackPtr U_JUMPFAIL(VM &vm, StackPtr sp) { auto x = Pop(sp); auto nip = *vm.ip++; if (x.False()) { vm.ip = vm.codestart + nip; } return sp; }
VM_INLINE StackPtr U_JUMPFAILR(VM &vm, StackPtr sp) { auto x = Pop(sp); auto nip = *vm.ip++; if (x.False()) { vm.ip = vm.codestart + nip; Push(sp, x); } return sp; }
VM_INLINE StackPtr U_JUMPNOFAIL(VM &vm, StackPtr sp) { auto x = Pop(sp); auto nip = *vm.ip++; if (x.True()) { vm.ip = vm.codestart + nip; } return sp; }
VM_INLINE StackPtr U_JUMPNOFAILR(VM &vm, StackPtr sp) { auto x = Pop(sp); auto nip = *vm.ip++; if (x.True()) { vm.ip = vm.codestart + nip; Push(sp, x); } return sp; }
#endif

VM_INLINE StackPtr U_JUMP_TABLE(VM &vm, StackPtr sp, int mini, int maxi, int table_start) {
    auto val = Pop(sp).ival();
    if (val < mini || val > maxi) val = maxi + 1;
    auto target = vm.vtables[(ssize_t)(table_start + val - mini)];
    vm.JumpTo(target);
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

VM_INLINE StackPtr U_YIELD(VM &vm, StackPtr sp VM_COMMA VM_OP_ARGS_CALL) {
    vm.CoYield(sp VM_COMMA VM_FC_PASS_THRU);
    return sp;
}

// This value never gets used anywhere, just a placeholder.
VM_INLINE StackPtr U_COCL(VM &, StackPtr sp) {
    Push(sp, Value(0, V_YIELD));
    return sp;
}

VM_INLINE StackPtr U_CORO(VM &vm, StackPtr sp VM_COMMA VM_OP_ARGS VM_COMMA VM_OP_ARGS_CALL) {
    vm.CoNew(sp VM_COMMA VM_IP_PASS_THRU VM_COMMA VM_FC_PASS_THRU);
    return sp;
}

VM_INLINE StackPtr U_COEND(VM &vm, StackPtr sp) {
    vm.CoClean(sp);
    return sp;
}

VM_INLINE StackPtr U_LOGREAD(VM &vm, StackPtr sp, int vidx) {
    auto val = Pop(sp);
    Push(sp, vm.vml.LogGet(val, vidx));
    return sp;
}

VM_INLINE StackPtr U_LOGWRITE(VM &vm, StackPtr sp, int vidx, int lidx) {
    vm.vml.LogWrite(vm.vars[vidx], lidx);
    return sp;
}

VM_INLINE StackPtr U_ABORT(VM &vm, StackPtr sp) {
    vm.SeriousError("VM internal error: abort");
    return sp;
}

#define LVALCASES(N, B) VM_INLINE void LV_##N(VM &vm, StackPtr &sp, Value &a) { Value b = Pop(sp); B; }
#define LVALCASER(N, B) VM_INLINE void LV_##N(VM &vm, StackPtr &sp, Value &fa, int len) { B; }
#define LVALCASESTR(N, B, B2) VM_INLINE void LV_##N(VM &vm, StackPtr &sp, Value &a) { Value b = Pop(sp); B; a.LTDECRTNIL(vm); B2; }

LVALCASER(IVVADD , _IVOP(+, 0, false, &fa))
LVALCASER(IVVADDR, _IVOP(+, 0, false, &fa))
LVALCASER(IVVSUB , _IVOP(-, 0, false, &fa))
LVALCASER(IVVSUBR, _IVOP(-, 0, false, &fa))
LVALCASER(IVVMUL , _IVOP(*, 0, false, &fa))
LVALCASER(IVVMULR, _IVOP(*, 0, false, &fa))
LVALCASER(IVVDIV , _IVOP(/, 1, false, &fa))
LVALCASER(IVVDIVR, _IVOP(/, 1, false, &fa))
LVALCASER(IVVMOD,  VMASSERT(vm, 0); (void)fa; (void)len; (void)sp)
LVALCASER(IVVMODR, VMASSERT(vm, 0); (void)fa; (void)len; (void)sp)

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
LVALCASER(IVSMOD , VMASSERT(vm, 0); (void)fa; (void)len; (void)sp)
LVALCASER(IVSMODR, VMASSERT(vm, 0); (void)fa; (void)len; (void)sp)

LVALCASER(FVSADD , _FVOP(+, 0, true,  &fa))
LVALCASER(FVSADDR, _FVOP(+, 0, true,  &fa))
LVALCASER(FVSSUB , _FVOP(-, 0, true,  &fa))
LVALCASER(FVSSUBR, _FVOP(-, 0, true,  &fa))
LVALCASER(FVSMUL , _FVOP(*, 0, true,  &fa))
LVALCASER(FVSMULR, _FVOP(*, 0, true,  &fa))
LVALCASER(FVSDIV , _FVOP(/, 1, true,  &fa))
LVALCASER(FVSDIVR, _FVOP(/, 1, true,  &fa))

LVALCASES(IADD   , _IOP(+, 0); a = res;          )
LVALCASES(IADDR  , _IOP(+, 0); a = res; Push(sp, res))
LVALCASES(ISUB   , _IOP(-, 0); a = res;          )
LVALCASES(ISUBR  , _IOP(-, 0); a = res; Push(sp, res))
LVALCASES(IMUL   , _IOP(*, 0); a = res;          )
LVALCASES(IMULR  , _IOP(*, 0); a = res; Push(sp, res))
LVALCASES(IDIV   , _IOP(/, 1); a = res;          )
LVALCASES(IDIVR  , _IOP(/, 1); a = res; Push(sp, res))
LVALCASES(IMOD   , _IOP(%, 1); a = res;          )
LVALCASES(IMODR  , _IOP(%, 1); a = res; Push(sp, res))

LVALCASES(BINAND , _IOP(&,  0); a = res;          )
LVALCASES(BINANDR, _IOP(&,  0); a = res; Push(sp, res))
LVALCASES(BINOR  , _IOP(|,  0); a = res;          )
LVALCASES(BINORR , _IOP(|,  0); a = res; Push(sp, res))
LVALCASES(XOR    , _IOP(^,  0); a = res;          )
LVALCASES(XORR   , _IOP(^,  0); a = res; Push(sp, res))
LVALCASES(ASL    , _IOP(<<, 0); a = res;          )
LVALCASES(ASLR   , _IOP(<<, 0); a = res; Push(sp, res))
LVALCASES(ASR    , _IOP(>>, 0); a = res;          )
LVALCASES(ASRR   , _IOP(>>, 0); a = res; Push(sp, res))

LVALCASES(FADD   , _FOP(+, 0); a = res;          )
LVALCASES(FADDR  , _FOP(+, 0); a = res; Push(sp, res))
LVALCASES(FSUB   , _FOP(-, 0); a = res;          )
LVALCASES(FSUBR  , _FOP(-, 0); a = res; Push(sp, res))
LVALCASES(FMUL   , _FOP(*, 0); a = res;          )
LVALCASES(FMULR  , _FOP(*, 0); a = res; Push(sp, res))
LVALCASES(FDIV   , _FOP(/, 1); a = res;          )
LVALCASES(FDIVR  , _FOP(/, 1); a = res; Push(sp, res))

LVALCASESTR(SADD , _SCAT(),    a = res;          )
LVALCASESTR(SADDR, _SCAT(),    a = res; Push(sp, res))

#define OVERWRITE_VAR(a, b) { TYPE_ASSERT(a.type == b.type || a.type == V_NIL || b.type == V_NIL); a = b; }

VM_INLINE void LV_WRITE    (VM &,   StackPtr &sp, Value &a) { auto  b = Pop(sp);                   OVERWRITE_VAR(a, b); }
VM_INLINE void LV_WRITER   (VM &,   StackPtr &sp, Value &a) { auto &b = Top(sp);                   OVERWRITE_VAR(a, b); }
VM_INLINE void LV_WRITEREF (VM &vm, StackPtr &sp, Value &a) { auto  b = Pop(sp); a.LTDECRTNIL(vm); OVERWRITE_VAR(a, b); }
VM_INLINE void LV_WRITERREF(VM &vm, StackPtr &sp, Value &a) { auto &b = Top(sp); a.LTDECRTNIL(vm); OVERWRITE_VAR(a, b); }

#define WRITESTRUCT(DECS) \
    auto b = TopPtr(sp) - l; \
    if (DECS) for (int i = 0; i < l; i++) (&a)[i].LTDECRTNIL(vm); \
    tsnz_memcpy(&a, b, l);

VM_INLINE void LV_WRITEV    (VM &vm, StackPtr &sp, Value &a, int l) { WRITESTRUCT(false); PopN(sp, l); }
VM_INLINE void LV_WRITERV   (VM &vm, StackPtr &sp, Value &a, int l) { WRITESTRUCT(false); }
VM_INLINE void LV_WRITEREFV (VM &vm, StackPtr &sp, Value &a, int l) { WRITESTRUCT(true); PopN(sp, l); }
VM_INLINE void LV_WRITERREFV(VM &vm, StackPtr &sp, Value &a, int l) { WRITESTRUCT(true); }

#define PPOP(name, ret, op, pre, accessor) VM_INLINE void LV_##name(VM &, StackPtr &sp, Value &a) { \
    if (ret && !pre) Push(sp, a); \
    a.set##accessor(a.accessor() op 1); \
    if (ret && pre) Push(sp, a); \
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

#pragma push_macro("LVAL")
#undef LVAL

#define LVAL(N, V) VM_INLINE StackPtr U_VAR_##N(VM &vm, StackPtr sp, int vidx VM_COMMA_IF(V) VM_OP_ARGSN(V)) \
    { LV_##N(vm, sp, vm.vars[vidx] VM_COMMA_IF(V) VM_OP_PASSN(V)); return sp; }
    LVALOPNAMES
#undef LVAL

#define LVAL(N, V) VM_INLINE StackPtr U_FLD_##N(VM &vm, StackPtr sp, int i VM_COMMA_IF(V) VM_OP_ARGSN(V)) \
    { auto &x = GetFieldLVal(vm, sp, i); LV_##N(vm, sp, x VM_COMMA_IF(V) VM_OP_PASSN(V)); return sp; }
    LVALOPNAMES
#undef LVAL

#define LVAL(N, V) VM_INLINE StackPtr U_LOC_##N(VM &vm, StackPtr sp, int i VM_COMMA_IF(V) VM_OP_ARGSN(V)) \
    { auto &x = GetLocLVal(vm, sp, i); LV_##N(vm, sp, x VM_COMMA_IF(V) VM_OP_PASSN(V)); return sp; }
    LVALOPNAMES
#undef LVAL

#define LVAL(N, V) VM_INLINE StackPtr U_IDXVI_##N(VM &vm, StackPtr sp VM_COMMA_IF(V) VM_OP_ARGSN(V)) \
    { auto x = Pop(sp).ival(); auto &y = GetVecLVal(vm, sp, x); LV_##N(vm, sp, y VM_COMMA_IF(V) VM_OP_PASSN(V)); return sp; }
    LVALOPNAMES
#undef LVAL

#define LVAL(N, V) VM_INLINE StackPtr U_IDXVV_##N(VM &vm, StackPtr sp, int l VM_COMMA_IF(V) VM_OP_ARGSN(V)) \
    { auto x = vm.GrabIndex(sp, l); auto &y = GetVecLVal(vm, sp, x); LV_##N(vm, sp, y VM_COMMA_IF(V) VM_OP_PASSN(V)); return sp; }
    LVALOPNAMES
#undef LVAL

#define LVAL(N, V) VM_INLINE StackPtr U_IDXNI_##N(VM &vm, StackPtr sp VM_COMMA_IF(V) VM_OP_ARGSN(V)) \
    { auto x = Pop(sp).ival(); auto &y = GetFieldILVal(vm, sp, x); LV_##N(vm, sp, y VM_COMMA_IF(V) VM_OP_PASSN(V)); return sp; }
    LVALOPNAMES
#undef LVAL

#pragma pop_macro("LVAL")

#ifndef VM_COMPILED_CODE_MODE
    #define F(N, A) StackPtr F_##N(VM &vm, StackPtr sp VM_COMMA VM_OP_ARGS) { \
                        VM_OP_DEFSN(A); \
                        return U_##N(vm, sp VM_COMMA_IF(A) VM_OP_PASSN(A)); \
                    }
        LVALOPNAMES
    #undef F
    #define F(N, A) StackPtr F_##N(VM &vm, StackPtr sp VM_COMMA VM_OP_ARGS) { \
                        VM_OP_DEFSN(A); \
                        return U_##N(vm, sp VM_COMMA_IF(A) VM_OP_PASSN(A)); \
                    }
        ILBASENAMES
    #undef F
    #define F(N, A) StackPtr F_##N(VM &vm, StackPtr sp VM_COMMA VM_OP_ARGS VM_COMMA VM_OP_ARGS_CALL) { \
                        VM_OP_DEFSN(A); \
                        return U_##N(vm, sp VM_COMMA_IF(A) VM_OP_PASSN(A) VM_CCOMMA_IF(A) VM_FC_PASS_THRU); \
                    }
                ILCALLNAMES
    #undef F
    #define F(N, A) StackPtr F_##N(VM &vm, StackPtr sp) { return U_##N(vm, sp); }
        ILJUMPNAMES
    #undef F
#endif


}  // namespace lobster
