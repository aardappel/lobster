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

#ifndef LOBSTER_IL
#define LOBSTER_IL

// FlatBuffers takes care of backwards compatibility of all metadata, but not the actual bytecode.
// This needs to be bumped each time we make changes to the format.

namespace lobster {

const int LOBSTER_BYTECODE_FORMAT_VERSION = 17;

// Any type specialized ops below must always have this ordering.
enum MathOp {
    MOP_ADD, MOP_SUB, MOP_MUL, MOP_DIV, MOP_MOD, MOP_LT, MOP_GT, MOP_LE, MOP_GE, MOP_EQ, MOP_NE
};

#define ILUNKNOWN 9

#define ILBASENAMES \
    F(PUSHINT,      1, 0, 1) \
    F(PUSHINT64,    2, 0, 1) \
    F(PUSHFLT,      1, 0, 1) \
    F(PUSHFLT64,    2, 0, 1) \
    F(PUSHSTR,      1, 0, 1) \
    F(PUSHNIL,      0, 0, 1) \
    F(PUSHVAR,      1, 0, 1) \
    F(PUSHVARV,     2, 0, 1) \
    F(VPUSHIDXI,    0, 2, 1) \
    F(VPUSHIDXI2V,  0, 2, ILUNKNOWN) \
    F(VPUSHIDXV,    1, ILUNKNOWN, ILUNKNOWN) \
    F(VPUSHIDXIS,   1, 2, 1) \
    F(VPUSHIDXIS2V, 2, 2, ILUNKNOWN) \
    F(VPUSHIDXVS,   3, ILUNKNOWN, ILUNKNOWN) \
    F(NPUSHIDXI,    1, ILUNKNOWN, 1) \
    F(SPUSHIDXI,    0, 2, 1) \
    F(PUSHFLD,      1, 1, 1) \
    F(PUSHFLDMREF,  1, 1, 1) \
    F(PUSHFLDV,     2, ILUNKNOWN, 1) \
    F(PUSHFLD2V,    2, 1, ILUNKNOWN) \
    F(PUSHFLDV2V,   3, ILUNKNOWN, ILUNKNOWN) \
    F(BCALLRETV,    2, ILUNKNOWN, ILUNKNOWN) \
    F(BCALLRET0,    2, 0, 1) \
    F(BCALLRET1,    2, 1, 1) \
    F(BCALLRET2,    2, 2, 1) \
    F(BCALLRET3,    2, 3, 1) \
    F(BCALLRET4,    2, 4, 1) \
    F(BCALLRET5,    2, 5, 1) \
    F(BCALLRET6,    2, 6, 1) \
    F(BCALLRET7,    2, 7, 1) \
    F(ASSERT,       3, 0, 0) \
    F(ASSERTR,      3, 0, 0) \
    F(CONT1,        1, 0, 0) \
    F(FUNSTART,     ILUNKNOWN, 0, 0) \
    F(ENDSTATEMENT, 2, 0, 0) \
    F(NEWVEC,       2, 0, 0) \
    F(NEWOBJECT,    1, 0, 0) \
    F(POP,          0, 0, 0) \
    F(POPREF,       0, 0, 0) \
    F(POPV,         1, 0, 0) \
    F(POPVREF,      1, 0, 0) \
    F(DUP,          0, 0, 0) \
    F(EXIT,         1, 0, 0) \
    F(ABORT,        0, 0, 0) \
    F(IADD, 0, 2, 1) F(ISUB, 0, 2, 1) F(IMUL, 0, 2, 1) F(IDIV, 0, 2, 1) F(IMOD, 0, 2, 1) \
    F(ILT,  0, 2, 1) F(IGT,  0, 2, 1) F(ILE,  0, 2, 1) F(IGE,  0, 2, 1) F(IEQ,  0, 2, 1) F(INE, 0, 2, 1) \
    F(FADD, 0, 2, 1) F(FSUB, 0, 2, 1) F(FMUL, 0, 2, 1) F(FDIV, 0, 2, 1) F(FMOD, 0, 2, 1) \
    F(FLT,  0, 2, 1) F(FGT,  0, 2, 1) F(FLE,  0, 2, 1) F(FGE,  0, 2, 1) F(FEQ,  0, 2, 1) F(FNE, 0, 2, 1) \
    F(SADD, 0, 2, 1) F(SSUB, 0, 2, 1) F(SMUL, 0, 2, 1) F(SDIV, 0, 2, 1) F(SMOD, 0, 2, 1) \
    F(SLT,  0, 2, 1) F(SGT,  0, 2, 1) F(SLE,  0, 2, 1) F(SGE,  0, 2, 1) F(SEQ,  0, 2, 1) F(SNE, 0, 2, 1) \
    F(IVVADD, 1, 0, 0) F(IVVSUB, 1, 0, 0) F(IVVMUL, 1, 0, 0) F(IVVDIV, 1, 0, 0) F(IVVMOD, 1, 0, 0) \
    F(IVVLT,  1, 0, 0) F(IVVGT,  1, 0, 0) F(IVVLE,  1, 0, 0) F(IVVGE,  1, 0, 0) \
    F(FVVADD, 1, 0, 0) F(FVVSUB, 1, 0, 0) F(FVVMUL, 1, 0, 0) F(FVVDIV, 1, 0, 0) F(FVVMOD, 1, 0, 0) \
    F(FVVLT,  1, 0, 0) F(FVVGT,  1, 0, 0) F(FVVLE,  1, 0, 0) F(FVVGE,  1, 0, 0) \
    F(IVSADD, 1, 0, 0) F(IVSSUB, 1, 0, 0) F(IVSMUL, 1, 0, 0) F(IVSDIV, 1, 0, 0) F(IVSMOD, 1, 0, 0) \
    F(IVSLT,  1, 0, 0) F(IVSGT,  1, 0, 0) F(IVSLE,  1, 0, 0) F(IVSGE,  1, 0, 0) \
    F(FVSADD, 1, 0, 0) F(FVSSUB, 1, 0, 0) F(FVSMUL, 1, 0, 0) F(FVSDIV, 1, 0, 0) F(FVSMOD, 1, 0, 0) \
    F(FVSLT,  1, 0, 0) F(FVSGT,  1, 0, 0) F(FVSLE,  1, 0, 0) F(FVSGE,  1, 0, 0) \
    F(AEQ,  0, 0, 0) F(ANE,  0, 0, 0) \
    F(STEQ, 1, 0, 0) F(STNE, 1, 0, 0) \
    F(LEQ,  0, 0, 0) F(LNE,  0, 0, 0) \
    F(IUMINUS, 0, 0, 0) F(FUMINUS, 0, 0, 0) F(IVUMINUS, 1, 0, 0) F(FVUMINUS, 1, 0, 0) \
    F(LOGNOT, 0, 0, 0) F(LOGNOTREF, 0, 0, 0) \
    F(BINAND, 0, 0, 0) F(BINOR, 0, 0, 0) F(XOR, 0, 0, 0) F(ASL, 0, 0, 0) F(ASR, 0, 0, 0) F(NEG, 0, 0, 0) \
    F(I2F, 0, 0, 0) F(A2S, 1, 0, 0) F(E2B, 0, 0, 0) F(E2BREF, 0, 0, 0) F(ST2S, 1, 0, 0) \
    F(RETURN, 2, 0, 0) F(RETURNANY, 0, 0, 0) \
    F(ISTYPE, 1, 0, 0) \
    F(FORLOOPI, 0, 0, 0) F(IFORELEM, 0, 0, 0) F(SFORELEM, 0, 0, 0) F(VFORELEM, 0, 0, 0) F(VFORELEMREF, 0, 0, 0) \
    F(VFORELEM2S, 0, 0, 0) F(VFORELEMREF2S, 0, 0, 0) \
    F(INCREF, 1, 0, 0) F(KEEPREF, 2, 0, 0) F(KEEPREFLOOP, 2, 0, 0) \
    F(JUMP_TABLE, ILUNKNOWN, 0, 0) \
    F(SAVERETS, 0, 0, 0) F(RESTORERETS, 0, 0, 0) \
    F(CALL, 1, 0, 0) F(CALLV, 0, 0, 0) F(CALLVCOND, 0, 0, 0) F(DDCALL, 2, 0, 0) \
    F(NATIVEHINT, 1, 0, 0)
#define ILCALLNAMES \
    F(PUSHFUN, 1, 0, 0)

#define ILJUMPNAMES1 \
    F(JUMP, 1, 0, 0) \
    F(JUMPFAIL, 1, 0, 0) F(JUMPFAILR, 1, 0, 0) \
    F(JUMPNOFAIL, 1, 0, 0) F(JUMPNOFAILR, 1, 0, 0) \
    F(IFOR, 1, 0, 0) F(SFOR, 1, 0, 0) F(VFOR, 1, 0, 0)
#define ILJUMPNAMES2 \
    F(JUMPIFUNWOUND, 2, 0, 0)

#define LVALOPNAMES \
    LVAL(WRITE, 0)  LVAL(WRITER, 0)  LVAL(WRITEREF, 0)  LVAL(WRITERREF, 0) \
    LVAL(WRITEV, 1) LVAL(WRITERV, 1) LVAL(WRITEREFV, 1) LVAL(WRITERREFV, 1) \
    LVAL(IADD, 0)   LVAL(IADDR, 0)   LVAL(ISUB, 0)   LVAL(ISUBR, 0)   LVAL(IMUL, 0)   LVAL(IMULR, 0)   LVAL(IDIV, 0)   LVAL(IDIVR, 0) \
    LVAL(IMOD, 0)   LVAL(IMODR, 0) \
    LVAL(BINAND, 0) LVAL(BINANDR, 0) LVAL(BINOR, 0)  LVAL(BINORR, 0)  LVAL(XOR, 0)    LVAL(XORR, 0) \
    LVAL(ASL, 0)    LVAL(ASLR, 0)    LVAL(ASR, 0)    LVAL(ASRR, 0) \
    LVAL(FADD, 0)   LVAL(FADDR, 0)   LVAL(FSUB, 0)   LVAL(FSUBR, 0)   LVAL(FMUL, 0)   LVAL(FMULR, 0)   LVAL(FDIV, 0)   LVAL(FDIVR, 0) \
    LVAL(IVVADD, 1) LVAL(IVVADDR, 1) LVAL(IVVSUB, 1) LVAL(IVVSUBR, 1) LVAL(IVVMUL, 1) LVAL(IVVMULR, 1) LVAL(IVVDIV, 1) LVAL(IVVDIVR, 1) \
    LVAL(IVVMOD, 1) LVAL(IVVMODR, 1) \
    LVAL(FVVADD, 1) LVAL(FVVADDR, 1) LVAL(FVVSUB, 1) LVAL(FVVSUBR, 1) LVAL(FVVMUL, 1) LVAL(FVVMULR, 1) LVAL(FVVDIV, 1) LVAL(FVVDIVR, 1) \
    LVAL(IVSADD, 1) LVAL(IVSADDR, 1) LVAL(IVSSUB, 1) LVAL(IVSSUBR, 1) LVAL(IVSMUL, 1) LVAL(IVSMULR, 1) LVAL(IVSDIV, 1) LVAL(IVSDIVR, 1) \
    LVAL(IVSMOD, 1) LVAL(IVSMODR, 1) \
    LVAL(FVSADD, 1) LVAL(FVSADDR, 1) LVAL(FVSSUB, 1) LVAL(FVSSUBR, 1) LVAL(FVSMUL, 1) LVAL(FVSMULR, 1) LVAL(FVSDIV, 1) LVAL(FVSDIVR, 1) \
    LVAL(SADD, 0)   LVAL(SADDR, 0) \
    LVAL(IPP, 0) LVAL(IPPR, 0) LVAL(IMM, 0) LVAL(IMMR, 0) LVAL(IPPP, 0) LVAL(IPPPR, 0) LVAL(IMMP, 0) LVAL(IMMPR, 0) \
    LVAL(FPP, 0) LVAL(FPPR, 0) LVAL(FMM, 0) LVAL(FMMR, 0) LVAL(FPPP, 0) LVAL(FPPPR, 0) LVAL(FMMP, 0) LVAL(FMMPR, 0)

enum LVALOP {
    #define LVAL(N, V) LVO_##N,
        LVALOPNAMES
    #undef LVAL
};

#define NUMBASELVALOPS 5  // HAS to match LVAL below!
#define GENLVALOP(LV, OP) ((ILOP)(IL_##LV##_WRITE + (OP) * NUMBASELVALOPS))  // WRITE assumed to be first!
#define GENOP(OP) ((ILOP)(OP))
#define ILADD00 0
#define ILADD01 1
#define ILADD10 1
#define ILADD11 2
#define ILADD(X, Y) ILADD##X##Y
#define LVAL(N, V) F(VAR_##N, ILADD(1, V), 0, 0) \
                   F(FLD_##N, ILADD(1, V), 0, 0) \
                   F(IDXVI_##N, ILADD(0, V), 0, 0) \
                   F(IDXVV_##N, ILADD(1, V), 0, 0) \
                   F(IDXNI_##N, ILADD(0, V), 0, 0)
// This assumes VAR is first!
#define ISLVALVARINS(O) O >= IL_VAR_WRITE && O <= IL_VAR_FMMPR && (O % NUMBASELVALOPS) == 0

#define ISBCALL(O) (O >= IL_BCALLRETV && O <= IL_BCALLRET7)

#define ILNAMES LVALOPNAMES ILBASENAMES ILCALLNAMES ILJUMPNAMES1 ILJUMPNAMES2

enum ILOP {
    #define F(N, A, USE, DEF) IL_##N,
        ILNAMES
    #undef F
    IL_MAX_OPS
};

inline const char **ILNames() {
    #define F(N, A, USE, DEF) #N,
        static const char *ilnames[] = { ILNAMES };
    #undef F
    return ilnames;
}

inline const int *ILArity() {
    #define F(N, A, USE, DEF) A,
        static const int ilarity[] = { ILNAMES };
    #undef F
    return ilarity;
}

inline const int *ILUses() {
    #define F(N, A, USE, DEF) USE,
        static const int iluses[] = { ILNAMES };
    #undef F
    return iluses;
}

inline const int *ILDefs() {
    #define F(N, A, USE, DEF) DEF,
        static const int ildefs[] = { ILNAMES };
    #undef F
    return ildefs;
}


#define NATIVEHINTS \
    NH(NONE) \
    NH(BLOCK_START) \
    NH(JUMPTABLE_END) \
    NH(JUMPTABLE_TO_CASE) \
    NH(JUMPTABLE_CASE_START) \
    NH(LOOP_BACK) \
    NH(LOOP_REMOVE) \
    NH(JUMPOUT_START) \
    NH(JUMPOUT_END) \
    NH(COND_JUMP) \
    NH(SWITCH_RANGE_BLOCK) \
    NH(SWITCH_RANGE_JUMP) \
    NH(SWITCH_RANGE_END) \
    NH(SWITCH_THISCASE_BLOCK) \
    NH(SWITCH_THISCASE_JUMP) \
    NH(SWITCH_THISCASE_END) \
    NH(SWITCH_NEXTCASE_BLOCK) \
    NH(SWITCH_NEXTCASE_JUMP) \
    NH(SWITCH_NEXTCASE_END)

enum NativeHint {
    #define NH(N) NH_##N,
        NATIVEHINTS
    #undef NH
};

inline const char **NHNames() {
    #define NH(N) #N,
        static const char *nhnames[] = { NATIVEHINTS };
    #undef F
    return nhnames;
}

}

#endif  // LOBSTER_IL
