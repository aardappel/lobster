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

// FlatBuffers takes care of backwards compatibility of all metadata, but not the actual bytecode.
// This needs to be bumped each time we make changes to the format.
const int LOBSTER_BYTECODE_FORMAT_VERSION = 1;

#define ILNAMES \
    F(PUSHINT) \
    F(PUSHFLT) \
    F(PUSHSTR) \
    F(PUSHUNDEF) \
    F(PUSHNIL) \
    F(PUSHFUN) \
    F(PUSHVAR) F(LVALVAR) \
    F(PUSHIDX) F(LVALIDX) \
    F(PUSHFLD) F(PUSHFLDM) F(LVALFLD) \
    F(PUSHLOC) F(LVALLOC) \
    F(BCALL) \
    F(CALL) F(CALLV) F(CALLVCOND) F(YIELD) F(DUP) F(CONT1) \
    F(FUNSTART) F(FUNEND) F(FUNMULTI) F(CALLMULTI) \
    F(JUMP) \
    F(NEWVEC) \
    F(POP) \
    F(EXIT) \
    F(IADD)  F(ISUB)  F(IMUL)  F(IDIV)  F(IMOD)  F(ILT)  F(IGT)  F(ILE)  F(IGE)  F(IEQ) F(INE) \
    F(FADD)  F(FSUB)  F(FMUL)  F(FDIV)  F(FMOD)  F(FLT)  F(FGT)  F(FLE)  F(FGE)  F(FEQ) F(FNE) \
    F(SADD)  F(SSUB)  F(SMUL)  F(SDIV)  F(SMOD)  F(SLT)  F(SGT)  F(SLE)  F(SGE)  F(SEQ) F(SNE) \
    F(IVADD) F(IVSUB) F(IVMUL) F(IVDIV) F(IVMOD) F(IVLT) F(IVGT) F(IVLE) F(IVGE) \
    F(FVADD) F(FVSUB) F(FVMUL) F(FVDIV) F(FVMOD) F(FVLT) F(FVGT) F(FVLE) F(FVGE) \
                                                                                 F(AEQ) F(ANE) \
    F(UMINUS) F(LOGNOT) F(I2F) F(A2S) F(A2A) F(JUMPFAIL) F(JUMPFAILR) F(JUMPNOFAIL) F(JUMPNOFAILR) F(RETURN) F(FOR) \
    F(PUSHONCE) \
    F(ISTYPE) F(CORO) F(COCL) F(COEND) \
    F(LOGREAD)

#define LVALOPNAMES \
    F(WRITE) F(WRITER) F(WRITED) \
    F(IADD)  F(IADDR)  F(ISUB)  F(ISUBR)  F(IMUL)  F(IMULR)  F(IDIV)  F(IDIVR)  F(IMOD) F(IMODR) \
    F(FADD)  F(FADDR)  F(FSUB)  F(FSUBR)  F(FMUL)  F(FMULR)  F(FDIV)  F(FDIVR) \
    F(IVADD) F(IVADDR) F(IVSUB) F(IVSUBR) F(IVMUL) F(IVMULR) F(IVDIV) F(IVDIVR) F(IVMOD) F(IVMODR) \
    F(FVADD) F(FVADDR) F(FVSUB) F(FVSUBR) F(FVMUL) F(FVMULR) F(FVDIV) F(FVDIVR) \
    F(SADD)  F(SADDR) \
    F(PP) F(PPR) F(MM) F(MMR) F(PPP) F(PPPR) F(MMP) F(MMPR) 

namespace lobster
{

#define F(N) IL_##N,
    enum { ILNAMES };
#undef F

#define F(N) LVO_##N,
    enum { LVALOPNAMES };
#undef F

}