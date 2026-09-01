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

const int LOBSTER_METADATA_FORMAT_VERSION = 25;

// Any type specialized ops below must always have this ordering.
enum MathOp {
    MOP_ADD, MOP_SUB, MOP_MUL, MOP_DIV, MOP_MOD, MOP_LT, MOP_GT, MOP_LE, MOP_GE, MOP_EQ, MOP_NE
};

#define ILBASENAMES \
    F(UNUSED, 0) \
    F(PUSHINT, 1) \
    F(PUSHINT64, 2) \
    F(PUSHFLT, 1) \
    F(PUSHFLT64, 2) \
    F(PUSHSTR, 1) \
    F(PUSHNIL, 0) \
    F(PUSHVARF, 1) \
    F(PUSHVARL, 1) \
    F(PUSHVARVF, 2) \
    F(PUSHVARVL, 2) \
    F(VPUSHIDXI, 0) \
    F(VPUSHIDXI2V, 0) \
    F(VPUSHIDXV, 1) \
    F(VPUSHIDXIS, 1) \
    F(VPUSHIDXIS2V, 2) \
    F(VPUSHIDXVS, 3) \
    F(NPUSHIDXI, 1) \
    F(SPUSHIDXI, 0) \
    F(PUSHFLD, 1) \
    F(PUSHFLDMREF, 1) \
    F(PUSHFLDV, 2) \
    F(PUSHFLD2V, 2) \
    F(PUSHFLDV2V, 3) \
    F(BCALLRETV, 2) \
    F(BCALLRET0, 2) \
    F(BCALLRET1, 2) \
    F(BCALLRET2, 2) \
    F(BCALLRET3, 2) \
    F(BCALLRET4, 2) \
    F(BCALLRET5, 2) \
    F(BCALLRET6, 2) \
    F(BCALLRET7, 2) \
    F(ASSERT, 3) \
    F(ASSERTR, 3) \
    F(STATEMENT, 2) \
    F(PROFILE, 1) \
    F(NEWVEC, 2) \
    F(NEWOBJECT, 1) \
    F(POP, 0) \
    F(POPREF, 0) \
    F(POPV, 1) \
    F(DUP, 0) \
    F(EXIT, 1) \
    F(ABORT, 0) \
    F(IADD, 0) F(ISUB, 0) F(IMUL, 0) F(IDIV, 0) F(IMOD, 0) \
    F(ILT, 0) F(IGT, 0) F(ILE, 0) F(IGE, 0) F(IEQ, 0) F(INE, 0) \
    F(FADD, 0) F(FSUB, 0) F(FMUL, 0) F(FDIV, 0) F(FMOD, 0) \
    F(FLT, 0) F(FGT, 0) F(FLE, 0) F(FGE, 0) F(FEQ, 0) F(FNE, 0) \
    F(SADD, 0) F(SSUB, 0) F(SMUL, 0) F(SDIV, 0) F(SMOD, 0) \
    F(SLT, 0) F(SGT, 0) F(SLE, 0) F(SGE, 0) F(SEQ, 0) F(SNE, 0) \
    F(SADDN, 1) \
    F(IVVADD, 1) F(IVVSUB, 1) F(IVVMUL, 1) F(IVVDIV, 1) F(IVVMOD, 1) \
    F(IVVLT, 1) F(IVVGT, 1) F(IVVLE, 1) F(IVVGE, 1) \
    F(FVVADD, 1) F(FVVSUB, 1) F(FVVMUL, 1) F(FVVDIV, 1) F(FVVMOD, 1) \
    F(FVVLT, 1) F(FVVGT, 1) F(FVVLE, 1) F(FVVGE, 1) \
    F(IVSADD, 1) F(IVSSUB, 1) F(IVSMUL, 1) F(IVSDIV, 1) F(IVSMOD, 1) \
    F(IVSLT, 1) F(IVSGT, 1) F(IVSLE, 1) F(IVSGE, 1) \
    F(FVSADD, 1) F(FVSSUB, 1) F(FVSMUL, 1) F(FVSDIV, 1) F(FVSMOD, 1) \
    F(FVSLT, 1) F(FVSGT, 1) F(FVSLE, 1) F(FVSGE, 1) \
    F(SIVADD, 1) F(SIVSUB, 1) F(SIVMUL, 1) F(SIVDIV, 1) F(SIVMOD, 1) \
    F(SIVLT, 1) F(SIVGT, 1) F(SIVLE, 1) F(SIVGE, 1) \
    F(SFVADD, 1) F(SFVSUB, 1) F(SFVMUL, 1) F(SFVDIV, 1) F(SFVMOD, 1) \
    F(SFVLT, 1) F(SFVGT, 1) F(SFVLE, 1) F(SFVGE, 1) \
    F(AEQ, 0) F(ANE, 0) \
    F(SNEQ, 0) F(SNNE, 0) \
    F(STEQ, 1) F(STNE, 1) \
    F(LEQ, 0) F(LNE, 0) \
    F(IUMINUS, 0) F(FUMINUS, 0) \
    F(IVUMINUS, 1) F(FVUMINUS, 1) \
    F(LOGNOT, 0) \
    F(BINAND, 0) F(BINOR, 0) F(XOR, 0) F(ASL, 0) F(ASR, 0) F(NEG, 0) \
    F(I2F, 0) F(A2S, 1) F(E2B, 0) F(E2BREF, 0) F(ST2S, 1) \
    F(RETURNLOCAL, 1) F(RETURNNONLOCAL, 2) F(RETURNANY, 1) \
    F(ISTYPE, 2) F(ISSUBTYPE, 3) \
    F(FORLOOPI, 0) \
    F(IFORELEM, 0) \
    F(SFORELEM, 0) \
    F(VFORELEM, 0) \
    F(VFORELEMREF, 0) \
    F(VFORELEM2S, 0) \
    F(VFORELEMREF2S, 1) \
    F(INCREF, 1) \
    F(KEEPREF, 2) \
    F(KEEPREFLOOP, 2) \
    F(GOTOFUNEXIT, 0) \
    F(CALL, 1) \
    F(CALLV, 0) \
    F(DDCALL, 2) \
    F(LABEL, 1) \
    F(JUMP_TABLE_END, 0) \
    F(JUMP_TABLE_CASE_START, 1) \
    F(ENUM_RANGE_ERR, 0)

// These produce the address an assignment writes to, and return it rather than leaving it in the
// VM, so that the generated code can hold on to it in a local. Only LVAL_IDXSI indexes an address
// it is handed; the others start a fresh one and ignore that argument.
#define ILLVALNAMES \
    F(LVAL_VARF, 1) \
    F(LVAL_VARL, 1) \
    F(LVAL_FLD, 1) \
    F(LVAL_IDXVI, 1) \
    F(LVAL_IDXVV, 2) \
    F(LVAL_IDXNI, 1) \
    F(LVAL_IDXSI, 2)

// These consume an address produced by one of the above.
#define ILLVNAMES \
    F(LV_DUP, 0) \
    F(LV_DUPV, 1) \
    F(LV_WRITE, 0)  F(LV_WRITEREF, 0)  \
    F(LV_WRITEV, 1) F(LV_WRITEREFV, 2) \
    F(LV_IADD, 0)   F(LV_ISUB, 0)   F(LV_IMUL, 0)   F(LV_IDIV, 0)   \
    F(LV_IMOD, 0)   \
    F(LV_BINAND, 0) F(LV_BINOR, 0)  F(LV_XOR, 0)    \
    F(LV_ASL, 0)    F(LV_ASR, 0)    \
    F(LV_FADD, 0)   F(LV_FSUB, 0)   F(LV_FMUL, 0)   F(LV_FDIV, 0)   \
    F(LV_FMOD, 0)   \
    F(LV_IVVADD, 1) F(LV_IVVSUB, 1) F(LV_IVVMUL, 1) F(LV_IVVDIV, 1) \
    F(LV_IVVMOD, 1) \
    F(LV_FVVADD, 1) F(LV_FVVSUB, 1) F(LV_FVVMUL, 1) F(LV_FVVDIV, 1) \
    F(LV_FVVMOD, 1) \
    F(LV_IVSADD, 1) F(LV_IVSSUB, 1) F(LV_IVSMUL, 1) F(LV_IVSDIV, 1) \
    F(LV_IVSMOD, 1) \
    F(LV_FVSADD, 1) F(LV_FVSSUB, 1) F(LV_FVSMUL, 1) F(LV_FVSDIV, 1) \
    F(LV_FVSMOD, 1) \
    F(LV_SADD, 0) \
    F(LV_IPP, 0) F(LV_IMM, 0) \
    F(LV_FPP, 0) F(LV_FMM, 0)

#define ILCALLNAMES \
    F(PUSHFUN, 1)

// These take a pointer to a list of cases rather than a fixed number of int arguments, which is
// what the V in the argument count column says.
#define ILVARARGNAMES \
    F(JUMP_TABLE, V) \
    F(JUMP_TABLE_DISPATCH, V)

#define ILJUMPNAMES1 \
    F(JUMP, 1) \
    F(JUMPFAIL, 1) \
    F(JUMPFAILR, 1) \
    F(JUMPNOFAIL, 1) \
    F(JUMPNOFAILR, 1) \
    F(IFOR, 1) \
    F(SFOR, 1) \
    F(VFOR, 1)
#define ILJUMPNAMES2 \
    F(JUMPIFUNWOUND, 2) \
    F(JUMPIFSTATICLF, 2) \
    F(JUMPIFMEMBERLF, 2)

#define GENOP(OP) ((ILOP)(OP))

#define ILNAMES ILBASENAMES ILLVALNAMES ILLVNAMES ILCALLNAMES ILVARARGNAMES ILJUMPNAMES1 \
                ILJUMPNAMES2

enum ILOP {
    #define F(N, A) IL_##N,
        ILNAMES
    #undef F
    IL_MAX_OPS
};

inline const char **ILNames() {
    #define F(N, A) #N,
        static const char *ilnames[] = { ILNAMES };
    #undef F
    return ilnames;
}

}

#endif  // LOBSTER_IL
