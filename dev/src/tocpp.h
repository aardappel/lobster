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
    int ParseOpAndGetArity(int opc, const int *&ip)
    {
        auto arity = ILArity()[opc];

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

        return arity;
    }

    void ToCPP(string &s, const uchar *bytecode_buffer, size_t bytecode_len)
    {
        auto bcf = bytecode::GetBytecodeFile(bytecode_buffer);
        assert(FLATBUFFERS_LITTLEENDIAN);
        auto code = (const int *)bcf->bytecode()->Data();  // Assumes we're on a little-endian machine.
        //auto typetable = (const type_elem_t *)bcf->typetable()->Data();  // Same.

        map<int, const bytecode::Function *> function_lookup;
        for (size_t i = 0; i < bcf->functions()->size(); i++)
        {
            auto f = bcf->functions()->Get(i);
            function_lookup[f->bytecodestart()] = f;
        }

        s += "#ifndef VM_COMPILED_CODE_MODE\n"
             "  #error VM_COMPILED_CODE_MODE must be set for the entire code base.\n"
             "#endif\n"
             "\n"
             "#include \"stdafx.h\"\n"
             "#include \"vmdata.h\"\n"
             "#include \"sdlinterface.h\"\n"
             "\n"
             "using lobster::g_vm;\n"
             "\n"
             "#pragma warning (disable: 4102)  // Unused label.\n"
             "\n";

        auto len = bcf->bytecode()->Length();

        auto ilnames = ILNames();

        const int *ip = code;

        // Skip past 1st jump.
        assert(*ip == IL_JUMP);
        ip++;
        auto starting_point = *ip++;

        while (ip < code + len)
        {
            if (bcf->bytecode_attr()->Get(ip - code) & bytecode::Attr_SPLIT)
            {
                s += "static void *block";
                s += to_string(ip - code);
                s += "();\n";
            }

            int opc = *ip++;

            if (opc < 0 || opc >= IL_MAX_OPS)
            {
                s += "// Corrupt bytecode starts here: ";
                s += to_string(opc);
                s += "\n";
                return;
            }

            ParseOpAndGetArity(opc, ip);
        }

        s += "\n";

        ip = code + 2;

        bool start_block = false;

        while (ip < code + len)
        {
            int opc = *ip++;

            if (opc == IL_FUNSTART)
            {
                s += "\n";
                auto it = function_lookup.find(ip - 1 - code);
                if (it != function_lookup.end()) s += "// " + it->second->name()->str() + "\n";
            }

            auto ilname = ilnames[opc];

            auto args = ip;

            if (bcf->bytecode_attr()->Get(ip - 1 - code) & bytecode::Attr_SPLIT) start_block = true;

            auto arity = ParseOpAndGetArity(opc, ip);

            if (start_block)
            {
                s += "static void *block";
                s += to_string(args - 1 - code);
                s += "() {\n";
            }

            s += "  ";

            start_block = false;

            if (IsJumpOp(opc))
            {
                s += "if (g_vm->F_";
                s += ilname;
                s += "()) return block";
                s += to_string(args[0]);
                s += ";\n";
            }
            else
            {
                s += "{ ";
                if (arity)
                {
                    s += "int args[] = {";

                    for (int i = 0; i < arity; i++)
                    {
                        if (i) s += ", ";
                        s += to_string(args[i]);
                    }

                    s += "}; ";
                }

                s += "g_vm->F_";
                s += ilname;
                s += "(";
                s += arity ? "args" : "nullptr";
                s += ");";
                if (opc == IL_CALL)
                {
                    //s += " g_vm->SetRet"
                }
                s += " }\n";                
            }
            if (start_block || bcf->bytecode_attr()->Get(ip - code) & bytecode::Attr_SPLIT)
            {
                s += "  return ";
                if (opc == IL_EXIT)
                {
                    s += "nullptr";
                }
                else
                {
                    s += "block";
                    s += to_string(ip - code);
                }
                s += ";\n}\n";
            }
        }

        // FIXME: this obviously does NOT need to include the actual bytecode, just the metadata.
        // in fact, it be nice if those were in readable format in the generated code.
        s += "\nstatic const int bytecodefb[] =\n{";
        auto bytecode_ints = (const int *)bytecode_buffer;
        for (size_t i = 0; i < bytecode_len / sizeof(int); i++)
        {
            if ((i & 0xF) == 0) s += "\n  ";
            s += to_string(bytecode_ints[i]);
            s += ", ";
        }
        s += "\n};\n\n";

        s += "int main(int argc, char *argv[])\n{\n  return EngineRunCompiledCodeMain(argc, argv, block" +
             to_string(starting_point) + ", bytecodefb);\n}\n";
    }
}
