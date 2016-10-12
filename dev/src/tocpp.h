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

namespace lobster
{
    void ToCPP(string &s, const uchar *bytecode_buffer, size_t bytecode_len)
    {
        auto bcf = bytecode::GetBytecodeFile(bytecode_buffer);
        assert(FLATBUFFERS_LITTLEENDIAN);
        auto code = (const int *)bcf->bytecode()->Data();  // Assumes we're on a little-endian machine.
        //auto typetable = (const type_elem_t *)bcf->typetable()->Data();  // Same.

        s += "#define VM_COMPILED_CODE_MODE\n\n"
             "#include \"stdafx.h\"\n"
             "#include \"vmdata.h\"\n"
             "#include \"natreg.h\"\n"
             "#include \"il.h\"\n"
             "#include \"bytecode_generated.h\"\n"
             "\n";

        auto len = bcf->bytecode()->Length();

        auto ilnames = ILNames();
        auto ilarities = ILArity();

        const int *ip = code;
        while (ip < code + len)
        {
            int opc = *ip++;

            if (opc < 0 || opc >= IL_MAX_OPS)
            {
                s += to_string(opc);
                s += " ?";
                continue;
            }

            auto ilname = ilnames[opc];
            auto arity = ilarities[opc];

            auto args = ip;

            switch(opc)
            {
                default:
                {
                    assert(arity >= 0);
                    ip += arity;
                    break;
                }

                case IL_CORO:
                {
                    ip += 2;
                    int n = *ip++;
                    ip += n;
                    arity = n + 3;
                    break;
                }

                case IL_CALLMULTI:
                {
                    auto nargs = *ip++;
                    ip += 3;
                    ip += nargs;
                    arity = nargs + 4;
                    break;
                }

                case IL_FUNSTART:
                {
                    int n = *ip++;
                    ip += n;
                    int m = *ip++; 
                    ip += m;
                    ip++;
                    arity = n + m + 3;
                    break;
                }

                case IL_FUNMULTI:
                {
                    auto n = *ip++;
                    auto nargs = *ip++;
                    auto tablesize = (nargs * 2 + 1) * n;
                    ip += tablesize;
                    arity = tablesize + 2;
                    break;
                }
            }

            s += "void *block";
            s += to_string(args - 1 - code);
            s += "() { ";
            if (IsJumpOp(opc))
            {
                s += "return g_vm->F_";
                s += ilname;
                s += "(block";
                s += to_string(args[0]);
                s += ", block";
                s += to_string(ip - code);
                s += ")";
            }
            else
            {
                s += "int args[] = {";

                for (int i = 0; i < arity; i++)
                {
                    if (i) s += ", ";
                    s += to_string(args[i]);
                }

                s += "}; g_vm->F_";
                s += ilname;
                s += "(args); return ";
                if (opc == IL_EXIT)
                {
                    s += "nullptr";
                }
                else
                {
                    s += "block";
                    s += to_string(ip - code);
                }
            }
            s += "; }\n";
        }

        // FIXME: this obviously does NOT need to include the actual bytecode, just the metadata.
        // in fact, it be nice if those were in readable format in the generated code.
        s += "\nstatic const int bytecode[] =\n{";
        auto bytecode_ints = (const int *)bytecode_buffer;
        for (size_t i = 0; i < bytecode_len / sizeof(int); i++)
        {
            if ((i & 0xF) == 0) s += "\n  ";
            s += to_string(bytecode_ints[i]);
            s += ", ";
        }
        s += "\n}\n\n";

        s += "int main(int argc, char *argv[])\n{\n  return RunCompiledCode(argc, argv, block0, bytecode);\n}\n";
    }
}
