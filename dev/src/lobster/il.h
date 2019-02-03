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

const int LOBSTER_BYTECODE_FORMAT_VERSION = 13;
const int MAX_RETURN_VALUES = 16;

// Any type specialized ops below must always have this ordering.
enum MathOp {
    MOP_ADD, MOP_SUB, MOP_MUL, MOP_DIV, MOP_MOD, MOP_LT, MOP_GT, MOP_LE, MOP_GE, MOP_EQ, MOP_NE
};

#define ILBASENAMES \
    F(PUSHINT, 1) \
    F(PUSHINT64, 2) \
    F(PUSHFLT, 1) \
    F(PUSHFLT64, 2) \
    F(PUSHSTR, 1) \
    F(PUSHNIL, 0) \
    F(PUSHVAR, 1) \
    F(VPUSHIDXI, 0) F(VPUSHIDXV, 0) F(NPUSHIDXI, 0) F(SPUSHIDXI, 0) \
    F(PUSHFLD, 1) F(PUSHFLDMREF, 1) \
    F(PUSHLOC, 1) \
    F(BCALLRET0, 1) F(BCALLREF0, 1) F(BCALLUNB0, 1) \
    F(BCALLRET1, 1) F(BCALLREF1, 1) F(BCALLUNB1, 1) \
    F(BCALLRET2, 1) F(BCALLREF2, 1) F(BCALLUNB2, 1) \
    F(BCALLRET3, 1) F(BCALLREF3, 1) F(BCALLUNB3, 1) \
    F(BCALLRET4, 1) F(BCALLREF4, 1) F(BCALLUNB4, 1) \
    F(BCALLRET5, 1) F(BCALLREF5, 1) F(BCALLUNB5, 1) \
    F(BCALLRET6, 1) F(BCALLREF6, 1) F(BCALLUNB6, 1) \
    F(BCALLRET7, 1) F(BCALLREF7, 1) F(BCALLUNB7, 1) \
    F(CONT1, 1) \
    F(FUNSTART, -1) F(FUNEND, 0) F(FUNMULTI, -1) \
    F(ENDSTATEMENT, 0) \
    F(NEWVEC, 2) F(NEWSTRUCT, 1) \
    F(POP, 0) F(POPREF, 0) \
    F(DUP, 0) \
    F(EXIT, 1) F(ABORT, 0) \
    F(IADD, 0)  F(ISUB, 0)  F(IMUL, 0)  F(IDIV, 0)  F(IMOD, 0) \
    F(ILT, 0)  F(IGT, 0)  F(ILE, 0)  F(IGE, 0)  F(IEQ, 0) F(INE, 0) \
    F(FADD, 0)  F(FSUB, 0)  F(FMUL, 0)  F(FDIV, 0)  F(FMOD, 0) \
    F(FLT, 0)  F(FGT, 0)  F(FLE, 0)  F(FGE, 0)  F(FEQ, 0) F(FNE, 0) \
    F(SADD, 0)  F(SSUB, 0)  F(SMUL, 0)  F(SDIV, 0)  F(SMOD, 0) \
    F(SLT, 0)  F(SGT, 0)  F(SLE, 0)  F(SGE, 0)  F(SEQ, 0) F(SNE, 0) \
    F(IVVADD, 0) F(IVVSUB, 0) F(IVVMUL, 0) F(IVVDIV, 0) F(IVVMOD, 0) \
    F(IVVLT, 0) F(IVVGT, 0) F(IVVLE, 0) F(IVVGE, 0) \
    F(FVVADD, 0) F(FVVSUB, 0) F(FVVMUL, 0) F(FVVDIV, 0) F(FVVMOD, 0) \
    F(FVVLT, 0) F(FVVGT, 0) F(FVVLE, 0) F(FVVGE, 0) \
    F(IVSADD, 0) F(IVSSUB, 0) F(IVSMUL, 0) F(IVSDIV, 0) F(IVSMOD, 0) \
    F(IVSLT, 0) F(IVSGT, 0) F(IVSLE, 0) F(IVSGE, 0) \
    F(FVSADD, 0) F(FVSSUB, 0) F(FVSMUL, 0) F(FVSDIV, 0) F(FVSMOD, 0) \
    F(FVSLT, 0) F(FVSGT, 0) F(FVSLE, 0) F(FVSGE, 0) \
    F(AEQ, 0) F(ANE, 0) \
    F(IUMINUS, 0) F(FUMINUS, 0) F(IVUMINUS, 0) F(FVUMINUS, 0) \
    F(LOGNOT, 0) F(LOGNOTREF, 0) \
    F(BINAND, 0) F(BINOR, 0) F(XOR, 0) F(ASL, 0) F(ASR, 0) F(NEG, 0) \
    F(I2F, 0) F(A2S, 0) F(I2S, 0) F(F2S, 0) F(E2B, 0) F(E2BREF, 0) \
    F(RETURN, 2) \
    F(ISTYPE, 1) F(COCL, 0) F(COEND, 0) \
    F(LOGREAD, 1) F(LOGWRITE, 2) \
    F(FORLOOPI, 0) F(IFORELEM, 0) F(SFORELEM, 0) F(VFORELEM, 0) F(VFORELEMREF, 0) F(NFORELEM, 0) \
    F(INCREF, 1) F(KEEPREF, 2)

#define ILCALLNAMES \
    F(CALL, 1) F(CALLMULTI, -1) F(CALLV, 0) F(CALLVCOND, 0) \
    F(PUSHFUN, 1) F(CORO, -1) F(YIELD, 0)

#define ILJUMPNAMES \
    F(JUMP, 1) \
    F(JUMPFAIL, 1) F(JUMPFAILR, 1) F(JUMPFAILN, 1) \
    F(JUMPNOFAIL, 1) F(JUMPNOFAILR, 1) \
    F(IFOR, 1) F(SFOR, 1) F(VFOR, 1) F(NFOR, 1)

#define LVALOPNAMES \
    LVAL(WRITE)  LVAL(WRITER)  LVAL(WRITEREF) LVAL(WRITERREF) \
    LVAL(IADD)   LVAL(IADDR)   LVAL(ISUB)   LVAL(ISUBR)   LVAL(IMUL)   LVAL(IMULR)   LVAL(IDIV)   LVAL(IDIVR) \
    LVAL(IMOD)   LVAL(IMODR) \
    LVAL(FADD)   LVAL(FADDR)   LVAL(FSUB)   LVAL(FSUBR)   LVAL(FMUL)   LVAL(FMULR)   LVAL(FDIV)   LVAL(FDIVR) \
    LVAL(IVVADD) LVAL(IVVADDR) LVAL(IVVSUB) LVAL(IVVSUBR) LVAL(IVVMUL) LVAL(IVVMULR) LVAL(IVVDIV) LVAL(IVVDIVR) \
    LVAL(IVVMOD) LVAL(IVVMODR) \
    LVAL(FVVADD) LVAL(FVVADDR) LVAL(FVVSUB) LVAL(FVVSUBR) LVAL(FVVMUL) LVAL(FVVMULR) LVAL(FVVDIV) LVAL(FVVDIVR) \
    LVAL(IVSADD) LVAL(IVSADDR) LVAL(IVSSUB) LVAL(IVSSUBR) LVAL(IVSMUL) LVAL(IVSMULR) LVAL(IVSDIV) LVAL(IVSDIVR) \
    LVAL(IVSMOD) LVAL(IVSMODR) \
    LVAL(FVSADD) LVAL(FVSADDR) LVAL(FVSSUB) LVAL(FVSSUBR) LVAL(FVSMUL) LVAL(FVSMULR) LVAL(FVSDIV) LVAL(FVSDIVR) \
    LVAL(SADD)   LVAL(SADDR) \
    LVAL(IPP) LVAL(IPPR) LVAL(IMM) LVAL(IMMR) LVAL(IPPP) LVAL(IPPPR) LVAL(IMMP) LVAL(IMMPR) \
    LVAL(FPP) LVAL(FPPR) LVAL(FMM) LVAL(FMMR) LVAL(FPPP) LVAL(FPPPR) LVAL(FMMP) LVAL(FMMPR)

enum LVALOP {
    #define LVAL(N) LVO_##N,
        LVALOPNAMES
    #undef LVAL
};

#define NUMBASELVALOPS 6  // HAS to match LVAL below!
#define GENLVALOP(LV, OP) (IL_##LV##_WRITE + (OP) * NUMBASELVALOPS)  // WRITE assumed to be first!
#define LVAL(N) F(VAR_##N, 1) F(FLD_##N, 1) F(LOC_##N, 1) \
                F(IDXVI_##N, 0) F(IDXVV_##N, 0) F(IDXNI_##N, 0)
// This assumes VAR is first!
#define ISLVALVARINS(O) O >= IL_VAR_WRITE && O <= IL_VAR_FMMPR && (O % NUMBASELVALOPS) == 0

#define ILNAMES LVALOPNAMES ILBASENAMES ILCALLNAMES ILJUMPNAMES 

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

inline const int *ILArity() {
    #define F(N, A) A,
        static const int ilarity[] = { ILNAMES };
    #undef F
    return ilarity;
}

}

#endif  // LOBSTER_IL
