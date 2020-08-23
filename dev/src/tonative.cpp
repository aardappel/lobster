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
#include "lobster/disasm.h"  // Some shared bytecode utilities.
#include "lobster/compiler.h"
#include "lobster/tonative.h"

namespace lobster {

int ParseOpAndGetArity(int opc, const int *&ip) {
    auto arity = ILArity()[opc];
    auto ips = ip;
    switch(opc) {
        default: {
            assert(arity != ILUNKNOWNARITY);
            ip += arity;
            break;
        }
        case IL_JUMP_TABLE: {
            auto mini = *ip++;
            auto maxi = *ip++;
            auto n = maxi - mini + 2;
            ip += n;
            arity = int(ip - ips);
            break;
        }
        case IL_FUNSTART: {
            ip++;  // function idx.
            int n = *ip++;
            ip += n;
            int m = *ip++;
            ip += m;
            ip++;  // keepvar
            int o = *ip++;  // ownedvar
            ip += o;
            arity = int(ip - ips);
            break;
        }
    }
    return arity;
}

string ToNative(NativeRegistry &natreg, NativeGenerator &ng,
                string_view bytecode_buffer) {
    auto bcf = bytecode::GetBytecodeFile(bytecode_buffer.data());
    if (!FLATBUFFERS_LITTLEENDIAN) return "native code gen requires little endian";
    auto code = (const int *)bcf->bytecode()->Data();  // Assumes we're on a little-endian machine.
    auto typetable = (const type_elem_t *)bcf->typetable()->Data();  // Same.
    map<int, const bytecode::Function *> function_lookup;
    for (flatbuffers::uoffset_t i = 0; i < bcf->functions()->size(); i++) {
        auto f = bcf->functions()->Get(i);
        function_lookup[f->bytecodestart()] = f;
    }
    ng.FileStart();
    auto len = bcf->bytecode()->Length();
    const int *ip = code;
    auto cur_id = [&]() { return (int)(ip - code); };
    // Skip past 1st jump.
    assert(*ip == IL_JUMP);
    ip++;
    auto starting_ip = code + *ip++;
    int starting_point = -1;
    while (ip < code + len) {
        int id = cur_id();
        if (*ip == IL_FUNSTART || ip == starting_ip) {
            ng.DeclareFun(id);
            starting_point = id;
        }
        if ((false)) {  // Debug corrupt bytecode.
            string sd;
            DisAsmIns(natreg, sd, ip, code, typetable, bcf);
            LOG_DEBUG(sd);
        }
        int opc = *ip++;
        if (opc < 0 || opc >= IL_MAX_OPS) {
            return cat("Corrupt bytecode: ", opc, " at: ", id);
        }
        ParseOpAndGetArity(opc, ip);
    }
    ng.BeforeBlocks(starting_point, bytecode_buffer);
    ip = code + 2;  // Past first IL_JUMP.
    while (ip < code + len) {
        int id = cur_id();
        int opc = *ip++;
        if (opc == IL_FUNSTART || ip - 1 == starting_ip) {
            auto it = function_lookup.find(id);
            ng.FunStart(it != function_lookup.end() ? it->second : nullptr, id);
        }
        auto args = ip;
        auto arity = ParseOpAndGetArity(opc, ip);
        auto is_vararg = ILArity()[opc] == ILUNKNOWNARITY;
        ng.InstStart();
        if (opc == IL_JUMP) {
            ng.EmitJump(args[0]);
        } else if (opc == IL_JUMPIFUNWOUND) {
            auto id = args[1];
            assert(id >= 0);
            ng.EmitConditionalJump(opc, id, args[0]);
        } else if ((opc >= IL_JUMPFAIL && opc <= IL_JUMPNOFAILR) ||
                   (opc >= IL_IFOR && opc <= IL_VFOR)) {
            auto id = args[0];
            assert(id >= 0);
            ng.EmitConditionalJump(opc, id, -1);
        } else if (opc == IL_JUMP_TABLE) {
            ng.EmitJumpTable(args);
        } else if (opc == IL_NATIVEHINT) {
            ng.EmitHint((NativeHint)args[0], id);
        } else if (ISBCALL(opc)) {
            auto nf = natreg.nfuns[args[0]];
            if (nf->IsGLFrame()) {
                ng.EmitExternCall("GLFrame");
            } else {
                ng.EmitOperands(bytecode_buffer.data(), args, arity, is_vararg);
                ng.EmitGenericInst(opc, args, arity, is_vararg, -1);
            }
            ng.Annotate(nf->name);
        } else {
            ng.EmitOperands(bytecode_buffer.data(), args, arity, is_vararg);
            int target = -1;
            if (opc == IL_PUSHFUN) {
                target = args[0];
            }
            ng.EmitGenericInst(opc, args, arity, is_vararg, target);
            if (opc == IL_PUSHVAR) {
                ng.Annotate(IdName(bcf, args[0], typetable, false));
            } else if (ISLVALVARINS(opc)) {
                ng.Annotate(IdName(bcf, args[0], typetable, false));
            } else if (opc == IL_PUSHSTR) {
                string cs;
                auto sv = bcf->stringtable()->Get(args[0])->string_view();
                sv = sv.substr(0, 50);
                EscapeAndQuote(sv, cs, true);
                ng.Annotate(cs);
            } else if (opc == IL_CALL) {
                auto fs = code + args[0];
                assert(*fs == IL_FUNSTART);
                fs++;
                ng.Annotate(bcf->functions()->Get(*fs)->name()->string_view());
            }
            if (opc == IL_CALL) {
                ng.EmitCall(args[0]);
            } else if (opc == IL_CALLV || opc == IL_DDCALL) {
                ng.EmitCallIndirect();
            } else if (opc == IL_RETURN || opc == IL_RETURNANY) {
                ng.EmitReturn();
            } else if (opc == IL_CALLVCOND) {
                ng.EmitCallIndirectNull();
            }
        }
        ng.InstEnd();
        if (opc == IL_EXIT || opc == IL_ABORT) ng.Exit();
        if (ip == code + len || *ip == IL_FUNSTART || ip == starting_ip) ng.FunEnd();
    }
    ng.CodeEnd();
    vector<int> vtables;
    for (auto bcs : *bcf->vtables()) {
        int id = -1;
        if (bcs >= 0) {
            id = bcs;
            assert(id >= 0);
        }
        vtables.push_back(id);
    }
    ng.VTables(vtables);
    ng.FileEnd(starting_point, bytecode_buffer);
    return "";
}

}
