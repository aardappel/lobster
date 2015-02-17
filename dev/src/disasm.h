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

static const LineInfo &LookupLine(int pos, const vector<LineInfo> &_lineinfo)
{
    const LineInfo *lineinfo = _lineinfo.data();
    int size = _lineinfo.size();
    assert(size);

    for (;;)    // quick hardcoded binary search
    {
        if (size == 1) return *lineinfo;

        int nsize = size / 2;
        if (lineinfo[nsize].bytecodestart <= pos) { lineinfo += nsize; size -= nsize; }
        else size = nsize;
    }
}

static void LvalDisAsm(string &s, int *&ip)
{
    #define F(N) #N,
    static const char *lvonames[] = { LVALOPNAMES };
    #undef F

    s += lvonames[*ip++];
    s += " ";
}

static int *DisAsmIns(string &s, SymbolTable &st, int *ip, int *code, const LineInfo &li)
{
    #define F(N) #N,
    static const char *ilnames[] = { ILNAMES };
    #undef F

    // FIXME: some indication of the filename, maybe with a table index?
    s += "I ";
    s += inttoa(ip - code);
    s += " \tL ";
    s += inttoa(li.line);
    s += " \t";
    s += ilnames[*ip];
    s += " ";

    switch(*ip++)
    {
        case IL_PUSHINT:
        case IL_PUSHFUN:
        case IL_CALLV:
        case IL_CALLVCOND:
        case IL_DUP:
        case IL_CONT1:
        case IL_JUMP:
        case IL_JUMPFAIL:
        case IL_JUMPFAILR:
        case IL_JUMPNOFAIL:
        case IL_JUMPNOFAILR:
        case IL_TT:
        case IL_TTSTRUCT:
        case IL_LOGREAD:
            s += inttoa(*ip++);
            break;

        case IL_RETURN:
        {
            auto id = *ip++;
            s += id >= 0 ? st.functiontable[id]->name.c_str() : inttoa(id);
            break;
        }

        case IL_CALL:
        case IL_CALLMULTI:
        {
            auto nargs = *ip++;
            auto id = *ip++;
            auto bc = *ip++;
            s += inttoa(nargs);
            s += " ";
            s += st.functiontable[id]->name;
            s += " ";
            s += inttoa(bc);
            break;
        }

        case IL_NEWVEC:
        {
            auto t = *ip++;
            auto nargs = *ip++;
            s += t >= 0 ? st.ReverseLookupType(t).c_str() : "vector";
            s += " ";
            s += inttoa(nargs);
            break;
        }

        case IL_BCALL:
        {
            int a = *ip++;
            s += natreg.nfuns[a]->name;
            s += " ";
            s += inttoa(*ip++);
            break;
        }

        case IL_LVALVAR:
            LvalDisAsm(s, ip);
        case IL_PUSHVAR:
            s += st.ReverseLookupIdent(*ip++);
            break;

        case IL_LVALFLDO:
        case IL_LVALFLDT:
        case IL_LVALLOC:
           LvalDisAsm(s, ip);
        case IL_PUSHFLDT:
        case IL_PUSHFLDO:
        case IL_PUSHFLDMT:
        case IL_PUSHFLDMO:
        case IL_PUSHLOC:
            s += inttoa(*ip++);
            break;

        case IL_LVALFLDC:
           LvalDisAsm(s, ip);
        case IL_PUSHFLDC:
        case IL_PUSHFLDMC:
            s += inttoa(*ip++);
            s += " ";
            s += inttoa(*ip++);
            s += " ";
            s += inttoa(*ip++);
            break;

        case IL_LVALIDX:
            LvalDisAsm(s, ip);
            break;

        case IL_PUSHFLT:
            s += flttoa(*(float *)ip);
            ip++;
            break;

        case IL_PUSHSTR:
            s += "\"";
            while(*ip) s += (char)*ip++;
            ip++;
            s += "\"";
            break;

        case IL_FUNSTART:
        {
            int n = *ip++;
            while (n--) { s += st.ReverseLookupIdent(*ip++); s += " "; }
            n = *ip++; 
            s += "=> ";
            while (n--) { s += st.ReverseLookupIdent(*ip++); s += " "; }
            n = *ip++;
            if (n) { s += "(log = "; s += inttoa(n); s += ")"; }
            break;
        }

        case IL_ISTYPE:
            s += inttoa(*ip++);
            s += " ";
            s += inttoa(*ip++);
            break;

        case IL_CORO:
        {
            s += inttoa(*ip++);
            int n = *ip++;
            for (int i = 0; i < n; i++) { s += " v"; s += inttoa(*ip++); }
            break;
        }

        case IL_FIELDTABLES:
            s += inttoa(*ip);
            ip = code + *ip;
            break;

        case IL_FUNMULTI:
        {
            auto n = *ip++;
            auto nargs = *ip++;
            s += inttoa(n);
            s += " ";
            s += inttoa(nargs);
            ip += (nargs * 2 + 1) * n;
        }
    }

    return ip;
}

static void DisAsm(string &s, SymbolTable &st, int *code, const vector<LineInfo> &lineinfo, size_t len)
{
    int *ip = code;
    while (ip < code + len)
    {
        ip = DisAsmIns(s, st, ip, code, LookupLine(ip - code, lineinfo));
        s += "\n";
    }
}

}  // namespace lobster
