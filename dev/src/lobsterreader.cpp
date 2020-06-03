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

#include "lobster/natreg.h"

#include "lobster/lex.h"

namespace lobster {

struct ValueParser {
    vector<string> filenames;
    vector<RefObj *> allocated;
    Lex lex;
    VM &vm;

    ValueParser(VM &vm, string_view _src) : lex("string", filenames, _src), vm(vm) {}

    void Parse(StackPtr &sp, type_elem_t typeoff) {
        ParseFactor(sp, typeoff, true);
        Gobble(T_LINEFEED);
        Expect(T_ENDOFFILE);
    }

    // Vector or struct.
    void ParseElems(StackPtr &sp, TType end, type_elem_t typeoff, int numelems, bool push) {
        Gobble(T_LINEFEED);
        auto &ti = vm.GetTypeInfo(typeoff);
        auto stack_start = sp;
        auto NumElems = [&]() { return sp - stack_start; };
        if (lex.token == end) lex.Next();
        else {
            for (;;) {
                if (NumElems() == numelems) {
                    ParseFactor(sp, TYPE_ELEM_ANY, false);
                } else {
                    auto eti = ti.t == V_VECTOR ? ti.subt : ti.GetElemOrParent(NumElems());
                    ParseFactor(sp, eti, push);
                }
                bool haslf = lex.token == T_LINEFEED;
                if (haslf) lex.Next();
                if (lex.token == end) break;
                if (!haslf) Expect(T_COMMA);
            }
            lex.Next();
        }
        if (!push) return;
        if (numelems >= 0) {
            while (NumElems() < numelems) {
                switch (vm.GetTypeInfo(ti.elemtypes[NumElems()]).t) {
                    case V_INT:   Push(sp,  Value(0)); break;
                    case V_FLOAT: Push(sp,  Value(0.0f)); break;
                    case V_NIL:   Push(sp,  Value()); break;
                    default:      lex.Error("no default value exists for missing struct elements");
                }
            }
        }
        if (ti.t == V_CLASS) {
            auto vec = vm.NewObject(NumElems(), typeoff);
            if (NumElems()) vec->Init(vm, TopPtr(sp) - NumElems(), NumElems(), false);
            PopN(sp, NumElems());
            allocated.push_back(vec);
            Push(sp,  vec);
        } else if (ti.t == V_VECTOR) {
            auto &sti = vm.GetTypeInfo(ti.subt);
            auto width = IsStruct(sti.t) ? sti.len : 1;
            auto n = NumElems() / width;
            auto vec = vm.NewVec(n, n, typeoff);
            if (NumElems()) vec->Init(vm, TopPtr(sp) - NumElems(), false);
            PopN(sp, NumElems());
            allocated.push_back(vec);
            Push(sp,  vec);
        }
        // else if ti.t == V_STRUCT_* then.. do nothing!
    }

    void ExpectType(ValueType given, ValueType needed) {
        if (given != needed && needed != V_ANY) {
            lex.Error("type " +
                      BaseTypeName(needed) +
                      " required, " +
                      BaseTypeName(given) +
                      " given");
        }
    }

    void ParseFactor(StackPtr &sp, type_elem_t typeoff, bool push) {
        auto &ti = vm.GetTypeInfo(typeoff);
        auto vt = ti.t;
        switch (lex.token) {
            case T_INT: {
                ExpectType(V_INT, vt);
                auto i = lex.IntVal();
                lex.Next();
                if (push) Push(sp,  i);
                break;
            }
            case T_FLOAT: {
                ExpectType(V_FLOAT, vt);
                auto f = strtod(lex.sattr.data(), nullptr);
                lex.Next();
                if (push) Push(sp,  f);
                break;
            }
            case T_STR: {
                ExpectType(V_STRING, vt);
                string s = lex.StringVal();
                lex.Next();
                if (push) {
                    auto str = vm.NewString(s);
                    allocated.push_back(str);
                    Push(sp,  str);
                }
                break;
            }
            case T_NIL: {
                ExpectType(V_NIL, vt);
                lex.Next();
                if (push) Push(sp,  Value());
                break;
            }
            case T_MINUS: {
                lex.Next();
                ParseFactor(sp, typeoff, push);
                if (push) {
                    switch (typeoff) {
                        case TYPE_ELEM_INT:   Push(sp,  Pop(sp).ival() * -1); break;
                        case TYPE_ELEM_FLOAT: Push(sp,  Pop(sp).fval() * -1); break;
                        default: lex.Error("unary minus: numeric value expected");
                    }
                }
                break;
            }
            case T_LEFTBRACKET: {
                ExpectType(V_VECTOR, vt);
                lex.Next();
                ParseElems(sp, T_RIGHTBRACKET, typeoff, -1, push);
                break;
            }
            case T_IDENT: {
                if (vt == V_INT && ti.enumidx >= 0) {
                    auto opt = vm.LookupEnum(lex.sattr, ti.enumidx);
                    if (!opt) lex.Error("unknown enum value " + lex.sattr);
                    lex.Next();
                    if (push) Push(sp,  *opt);
                    break;
                }
                if (!IsUDT(vt) && vt != V_ANY)
                    lex.Error("class/struct type required, " + BaseTypeName(vt) + " given");
                auto sname = lex.sattr;
                lex.Next();
                Expect(T_LEFTCURLY);
                auto name = vm.StructName(ti);
                if (name != sname)
                    lex.Error("class/struct type " + name + " required, " + sname + " given");
                ParseElems(sp, T_RIGHTCURLY, typeoff, ti.len, push);
                break;
            }
            default:
                lex.Error("illegal start of expression: " + lex.TokStr());
                Push(sp,  Value());
                break;
        }
    }

    void Expect(TType t) {
        if (lex.token != t)
            lex.Error(lex.TokStr(t) + " expected, found: " + lex.TokStr());
        lex.Next();
    }

    void Gobble(TType t) {
        if (lex.token == t) lex.Next();
    }
};

static void ParseData(StackPtr &sp, VM &vm, type_elem_t typeoff, string_view inp) {
    auto stack_level = sp;
    ValueParser parser(vm, inp);
    #ifdef USE_EXCEPTION_HANDLING
    try
    #endif
    {
        parser.Parse(sp, typeoff);
        Push(sp,  Value());
    }
    #ifdef USE_EXCEPTION_HANDLING
    catch (string &s) {
        sp = stack_level;
        for (auto a : parser.allocated) a->Dec(vm);
        Push(sp,  Value());
        Push(sp,  vm.NewString(s));
    }
    #endif
}

void AddReader(NativeRegistry &nfr) {

nfr("parse_data", "typeid,stringdata", "TS", "A1?S?",
    "parses a string containing a data structure in lobster syntax (what you get if you convert"
    " an arbitrary data structure to a string) back into a data structure. supports"
    " int/float/string/vector and classes. classes will be forced to be compatible with their "
    " current definitions, i.e. too many elements will be truncated, missing elements will be"
    " set to 0/nil if possible. useful for simple file formats. returns the value and an error"
    " string as second return value (or nil if no error)",
    [](StackPtr &sp, VM &vm) {
        auto ins = Pop(sp).sval();
        auto type = Pop(sp).ival();
        ParseData(sp, vm, (type_elem_t)type, ins->strv());
    });

}

}
