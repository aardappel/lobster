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
#include "lobster/il.h"
#include "lobster/disasm.h"

namespace lobster {

string IdName(VM &vm, int i, const type_elem_t *typetable, bool is_whole_struct) {
    auto &sid = vm.meta->specidents[i];
    auto ti = (TypeInfo *)(typetable + sid.typeidx);
    if (is_whole_struct || !IsStruct(ti->t)) {
        return string(sid.name);
    } else {
        int j = i;
        while (j && vm.meta->specidents[j - 1].idx == sid.idx) j--;
        return cat(sid.name, "+", i - j);
    }
}

const int *DisAsmIns(NativeRegistry &nfr, string &sd, const int *ip,
                     const type_elem_t *typetable, int line, VM &vm) {
    auto ilnames = ILNames();
    auto ilarity = ILArity();
    if (line >= 0) {
        append(sd, "L ", line, " \t");
    }
    auto ins_start = ip;
    int opc = *ip++;
    if (opc < 0 || opc >= IL_MAX_OPS) {
        append(sd, "ILLEGAL INSTRUCTION: ", opc);
        return nullptr;
    }
    if (opc < 0 || opc >= IL_MAX_OPS) {
        append(sd, opc, " ?");
        return ip;
    }
    auto arity = ilarity[opc];
    int regs = *ip++;
    append(sd, "R ", regs, "\t");
    append(sd, ilnames[opc], " ");
    switch(opc) {
        case IL_PUSHINT64:
        case IL_PUSHFLT64: {
            auto a = *ip++;
            auto v = Int64FromInts(a, *ip++);
            if (opc == IL_PUSHINT64) append(sd, v);
            else sd += to_string_float(int2float64(v).f);
            break;
        }

        case IL_KEEPREF:
        case IL_KEEPREFLOOP:
            append(sd, *ip++, " ");
            append(sd, *ip++);
            break;

        case IL_RETURNLOCAL: {
            auto nrets = *ip++;
            append(sd, nrets);
            break;
        }
        case IL_RETURNNONLOCAL: {
            auto nrets = *ip++;
            auto id = *ip++;
            append(sd, vm.meta->function_names[id], " ", nrets);
            break;
        }

        case IL_CALL: {
            auto bc = *ip++;
            append(sd, " ", bc);
            break;
        }

        case IL_NEWVEC: {
            ip++;  // ti
            auto nargs = *ip++;
            append(sd, "vector ", nargs);
            break;
        }
        case IL_ST2S:
        case IL_NEWOBJECT: {
            auto ti = (TypeInfo *)(typetable + *ip++);
            sd += vm.meta->udts[ti->structidx].name;
            break;
        }

        case IL_BCALLRETV:
        case IL_BCALLRET0:
        case IL_BCALLRET1:
        case IL_BCALLRET2:
        case IL_BCALLRET3:
        case IL_BCALLRET4:
        case IL_BCALLRET5:
        case IL_BCALLRET6: {
            int a = *ip++;
            ip++;  // has_ret
            sd += nfr.nfuns[a]->name;
            break;
        }

        case IL_PUSHVARVL:
        case IL_PUSHVARVF:
            sd += IdName(vm, *ip++, typetable, true);
            append(sd, " ", *ip++);
            break;

        case IL_LVAL_VARL:
        case IL_LVAL_VARF:
        case IL_PUSHVARL:
        case IL_PUSHVARF:
            sd += IdName(vm, *ip++, typetable, false);
            break;

        case IL_PUSHFLT:
            sd += to_string_float(*(float *)ip);
            ip++;
            break;

        case IL_PUSHSTR:
            EscapeAndQuote(vm.meta->stringtable[*ip++], sd);
            break;

        case IL_JUMP_TABLE: {
            auto mini = *ip++;
            auto maxi = *ip++;
            auto n = maxi - mini + 2;
            append(sd, mini, "..", maxi, " [ ");
            while (n--) append(sd, *ip++, " ");
            sd += "]";
            break;
        }

        case IL_JUMP_TABLE_DISPATCH: {
            auto vtable_idx = *ip++;
            auto mini = *ip++;
            auto maxi = *ip++;
            auto n = maxi - mini + 2;
            append(sd, vtable_idx, "/", n - 1, " [ ");
            while (n--)
                append(sd, *ip++, " ");
            sd += "]";
            break;
        }

        default:
            for (int i = 0; i < arity; i++) {
                if (i) sd += ' ';
                append(sd, *ip++);
            }
            break;
    }
    assert(arity == ILUNKNOWN || ip - ins_start == arity + 2);
    (void)ins_start;
    return ip;
}

}  // namespace lobster
