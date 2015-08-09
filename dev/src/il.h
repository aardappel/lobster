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
    F(IVVADD) F(IVVSUB) F(IVVMUL) F(IVVDIV) F(IVVMOD) F(IVVLT) F(IVVGT) F(IVVLE) F(IVVGE) \
    F(FVVADD) F(FVVSUB) F(FVVMUL) F(FVVDIV) F(FVVMOD) F(FVVLT) F(FVVGT) F(FVVLE) F(FVVGE) \
    F(IVSADD) F(IVSSUB) F(IVSMUL) F(IVSDIV) F(IVSMOD) F(IVSLT) F(IVSGT) F(IVSLE) F(IVSGE) \
    F(FVSADD) F(FVSSUB) F(FVSMUL) F(FVSDIV) F(FVSMOD) F(FVSLT) F(FVSGT) F(FVSLE) F(FVSGE) \
    F(AEQ) F(ANE) \
    F(IUMINUS) F(FUMINUS) F(IVUMINUS) F(FVUMINUS) \
    F(LOGNOT) F(I2F) F(A2S) F(I2A) F(F2A) F(JUMPFAIL) F(JUMPFAILR) F(JUMPNOFAIL) F(JUMPNOFAILR) F(RETURN) F(FOR) \
    F(PUSHONCE) \
    F(ISTYPE) F(CORO) F(COCL) F(COEND) \
    F(LOGREAD)

#define LVALOPNAMES \
    F(WRITE)  F(WRITER)  F(WRITED) \
    F(IADD)   F(IADDR)   F(ISUB)   F(ISUBR)   F(IMUL)   F(IMULR)   F(IDIV)   F(IDIVR)   F(IMOD)  F(IMODR) \
    F(FADD)   F(FADDR)   F(FSUB)   F(FSUBR)   F(FMUL)   F(FMULR)   F(FDIV)   F(FDIVR) \
    F(IVVADD) F(IVVADDR) F(IVVSUB) F(IVVSUBR) F(IVVMUL) F(IVVMULR) F(IVVDIV) F(IVVDIVR) F(IVVMOD) F(IVVMODR) \
    F(FVVADD) F(FVVADDR) F(FVVSUB) F(FVVSUBR) F(FVVMUL) F(FVVMULR) F(FVVDIV) F(FVVDIVR) \
    F(IVSADD) F(IVSADDR) F(IVSSUB) F(IVSSUBR) F(IVSMUL) F(IVSMULR) F(IVSDIV) F(IVSDIVR) F(IVSMOD) F(IVSMODR) \
    F(FVSADD) F(FVSADDR) F(FVSSUB) F(FVSSUBR) F(FVSMUL) F(FVSMULR) F(FVSDIV) F(FVSDIVR) \
    F(SADD)   F(SADDR) \
    F(IPP) F(IPPR) F(IMM) F(IMMR) F(IPPP) F(IPPPR) F(IMMP) F(IMMPR) \
    F(FPP) F(FPPR) F(FMM) F(FMMR) F(FPPP) F(FPPPR) F(FMMP) F(FMMPR)

namespace lobster
{

#define F(N) IL_##N,
    enum { ILNAMES };
#undef F

#define F(N) LVO_##N,
    enum { LVALOPNAMES };
#undef F

}