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

#include "flatbuffers/idl.h"

#define FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
#include "lobster/bytecode_generated.h"

#include "lobster/lobsterreader.h"

namespace lobster {

struct ValueParser : Deserializer {
    vector<pair<string, string>> filenames;
    Lex lex;

    ValueParser(VM &vm, string_view _src) : Deserializer(vm), lex("string", filenames, {}, _src) {
        lex.do_string_interpolation = false;
    }

    Value Parse(type_elem_t typeoff) {
        ParseFactor(typeoff, true);
        Gobble(T_LINEFEED);
        Expect(T_ENDOFFILE);
        assert(stack.size() == 1);
        return PopV();
    }

    // A vector: `[ .. ]` with its elements.
    void ParseElems(TType end, type_elem_t typeoff, bool push) {
        Gobble(T_LINEFEED);
        auto &ti = vm.GetTypeInfo(typeoff);
        auto stack_start = stack.size();
        if (lex.token == end) lex.Next();
        else {
            for (;;) {
                ParseFactor(ti.subt, push);
                bool haslf = lex.token == T_LINEFEED;
                if (haslf) lex.Next();
                if (lex.token == end) break;
                if (!haslf) Expect(T_COMMA);
            }
            lex.Next();
        }
        if (!push) return;
        auto &sti = vm.GetTypeInfo(ti.subt);
        auto width = RTIsStruct(sti.t) ? sti.len : 1;
        auto len = iint(stack.size() - stack_start);
        auto n = len / width;
        auto vec = vm.NewVec(n, n, typeoff);
        if (len) vec->CopyElemsShallow(stack.size() - len + stack.data());
        PopVN(len);
        PushV(vec, true);
    }

    // A struct or class: `Name { .. }` with the values of its fields in order, `ti` being the
    // type named (for a struct in an abstract struct family, the member it is), which is what
    // it prints as (see VM::StructToString). The fields not given get their defaults, and any
    // values beyond them are skipped.
    void ParseStructElems(const TypeInfo &ti, type_elem_t typeoff, bool push) {
        Gobble(T_LINEFEED);
        if (ti.IsAbstractFamilyStruct())
            lex.Error(cat("cannot construct abstract struct ", vm.StructName(ti)));
        auto more = lex.token != T_RIGHTCURLY;
        // After an element: whether another follows.
        auto next = [&]() {
            bool haslf = lex.token == T_LINEFEED;
            if (haslf) lex.Next();
            if (lex.token == T_RIGHTCURLY) return false;
            if (!haslf) Expect(T_COMMA);
            return true;
        };
        auto base = push ? ReserveSlots(ti) : 0;
        vm.ForEachField(ti, [&](const FieldInfo &f) {
            if (more) {
                ParseFactor(f.type, push);
                more = next();
            } else if (push) {
                if (!PushDefault(f, ti.elemtypes))
                    lex.Error("no default value exists for missing struct elements");
            } else {
                return;
            }
            if (push) StoreFieldFromTop(base, f);
        });
        while (more) {
            ParseFactor(TYPE_ELEM_ANY, false);
            more = next();
        }
        Expect(T_RIGHTCURLY);
        if (push && ti.t == RTT_CLASS) PushObject(ti, typeoff);
    }
    void ExpectType(RTType given, RTType needed) {
        if (given != needed && needed != RTT_INVALID) {
            lex.Error("type " +
                      BaseTypeName(needed) +
                      " required, " +
                      BaseTypeName(given) +
                      " given");
        }
    }

    void ParseFactor(type_elem_t typeoff, bool push) {
        auto ti = &vm.GetTypeInfo(typeoff);
        if (ti->is_nil && lex.token != T_NIL) {
            ti = &vm.GetTypeInfo(typeoff = ti->is_nil);
        }
        auto vt = ti->t;
        switch (lex.token) {
            case T_INT: {
                ExpectType(RTT_INT, vt);
                auto i = lex.ival;
                lex.Next();
                if (push) PushV(i);
                break;
            }
            case T_FLOAT: {
                ExpectType(RTT_FLOAT, vt);
                auto f = lex.fval;
                lex.Next();
                if (push) PushV(f);
                break;
            }
            case T_STR: {
                ExpectType(RTT_STRING, vt);
                string s = std::move(lex.sval);
                lex.Next();
                if (push) {
                    auto str = vm.NewString(s);
                    PushV(str, true);
                }
                break;
            }
            case T_NIL: {
                if (!ti->is_nil)
                    lex.Error("nillable type required, " + BaseTypeName(vt) + " given");
                lex.Next();
                if (push) PushV(NilVal());
                break;
            }
            case T_MINUS: {
                lex.Next();
                ParseFactor(typeoff, push);
                if (push) {
                    switch (typeoff) {
                        case TYPE_ELEM_INT:   stack.back().setival(stack.back().ival() * -1); break;
                        case TYPE_ELEM_FLOAT: stack.back().setfval(stack.back().fval() * -1); break;
                        default: lex.Error("unary minus: numeric value expected");
                    }
                }
                break;
            }
            case T_LEFTBRACKET: {
                ExpectType(RTT_VECTOR, vt);
                lex.Next();
                ParseElems(T_RIGHTBRACKET, typeoff, push);
                break;
            }
            case T_IDENT: {
                auto sname = string(lex.sattr);
                lex.Next();
                while (lex.token == T_DOT) {
                    // We're going to have to assume this is a namespaced name, since we don't know what namespaces there are.
                    lex.Next();
                    if (lex.token != T_IDENT) lex.Error("identifier expected after .");
                    sname += ".";
                    sname += lex.sattr;
                    lex.Next();
                }
                if (vt == RTT_INT && ti->enumidx >= 0) {
                    auto opt = vm.LookupEnum(sname, ti->enumidx);
                    if (!opt) lex.Error("unknown enum value " + sname);
                    if (push) PushV(*opt);
                    break;
                }
                if (!RTIsUDT(vt) && vt != RTT_INVALID)
                    lex.Error("class/struct type required, " + BaseTypeName(vt) + " given");
                Expect(T_LEFTCURLY);
                auto name = vm.StructName(*ti);
                if (name != sname) {
                    auto p = LookupSubClass(sname, ti, typeoff);
                    if (!p.first)
                        lex.Error("class/struct type " + name + " required, " + sname + " given");
                    ti = p.first;
                    typeoff = p.second;
                }
                ParseStructElems(*ti, typeoff, push);
                break;
            }
            default:
                lex.Error("illegal start of expression: " + lex.TokStr());
                PushV(NilVal());
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

struct FlexBufferParser : Deserializer {

    FlexBufferParser(VM &vm) : Deserializer(vm) {}

    Value Parse(type_elem_t typeoff, flexbuffers::Reference r) {
        ParseFactor(r, typeoff, "root");
        assert(stack.size() == 1);
        return PopV();
    }

    void Error(const string &s) {
        // FIXME: not great on non-exception platforms, this should not abort.
        THROW_OR_ABORT(cat("flexbuffers_binary_to_value: ", s));
    }

    void ExpectType(RTType given, RTType needed, string_view parent_field_name) {
        if (given != needed && needed != RTT_INVALID) {
            Error(cat(parent_field_name, ": type ", BaseTypeName(needed), " required, ",
                      BaseTypeName(given),
                               " given"));
        }
    }

    void ParseFactor(flexbuffers::Reference r, type_elem_t typeoff, string_view parent_field_name) {
        auto ti = &vm.GetTypeInfo(typeoff);
        auto ft = r.GetType();
        if (ti->is_nil && ft != flexbuffers::FBT_NULL) {
            ti = &vm.GetTypeInfo(typeoff = ti->is_nil);
        }
        auto vt = ti->t;
        switch (ft) {
            case flexbuffers::FBT_INT:
            case flexbuffers::FBT_BOOL: {
                if (vt == RTT_FLOAT) {
                    PushV((double)r.AsInt64());
                } else {
                    ExpectType(RTT_INT, vt, parent_field_name);
                    PushV(r.AsInt64());
                }
                break;
            }
            case flexbuffers::FBT_FLOAT: {
                ExpectType(RTT_FLOAT, vt, parent_field_name);
                PushV(r.AsDouble());
                break;
            }
            case flexbuffers::FBT_STRING: {
                ExpectType(RTT_STRING, vt, parent_field_name);
                auto s = r.AsString();
                auto str = vm.NewString(string_view(s.c_str(), s.size()));
                PushV(str, true);
                break;
            }
            case flexbuffers::FBT_NULL: {
                if (!ti->is_nil)
                    Error(cat(parent_field_name, ": nillable type required, ",
                        BaseTypeName(vt), " given"));
                PushV(NilVal());
                break;
            }
            case flexbuffers::FBT_VECTOR: {
                ExpectType(RTT_VECTOR, vt, parent_field_name);
                auto v = r.AsVector();
                auto stack_start = stack.size();
                for (size_t i = 0; i < v.size(); i++) {
                    ParseFactor(v[i], ti->subt, parent_field_name);
                }
                auto &sti = vm.GetTypeInfo(ti->subt);
                auto width = RTIsStruct(sti.t) ? sti.len : 1;
                auto len = iint(stack.size() - stack_start);
                auto n = len / width;
                auto vec = vm.NewVec(n, n, typeoff);
                if (len) vec->CopyElemsShallow(stack.size() - len + stack.data());
                PopVN(len);
                PushV(vec, true);
                break;
            }
            case flexbuffers::FBT_MAP: {
                if (!RTIsUDT(vt) && vt != RTT_INVALID)
                    Error(cat(parent_field_name, ": class/struct type required, ", BaseTypeName(vt),
                              " given"));
                auto m = r.AsMap();
                auto name = vm.StructName(*ti);
                auto sname = m["_type"];
                if (sname.IsString() && sname.AsString().c_str() != name) {
                    auto p = LookupSubClass(sname.AsString().c_str(), ti, typeoff);
                    if (!p.first)
                        Error(cat(parent_field_name, ": class/struct type ", name, " required, ",
                                  sname.AsString().str(),
                                  " given"));
                    ti = p.first;
                    typeoff = p.second;
                }
                if (ti->IsAbstractFamilyStruct())
                    Error(cat(parent_field_name, ": cannot construct abstract struct ", name,
                              " (missing _type)"));
                // The fields of the type the map's _type named (see LObject::ToFlexBuffer /
                // VM::StructToFlexBuffer), by name, the ones not present getting their
                // defaults.
                auto base = ReserveSlots(*ti);
                int fi = 0;
                vm.ForEachField(*ti, [&](const FieldInfo &f) {
                    auto fname = vm.LookupField(ti->structidx, fi++);
                    auto e = m[fname.data()];
                    if (e.IsNull()) {
                        if (!PushDefault(f, ti->elemtypes))
                            Error("no default value exists for missing field " + fname);
                    } else {
                        ParseFactor(e, f.type, fname);
                    }
                    StoreFieldFromTop(base, f);
                });
                if (vt == RTT_CLASS) PushObject(*ti, typeoff);
                break;
            }
            default:
                Error(cat(parent_field_name, ": can\'t convert to value: ", r.ToString()));
                PushV(NilVal());
                break;
        }
    }
};

static void ParseData(RefObj **val, LString **err, VM &vm, type_elem_t typeoff,
                      string_view inp) {
    ValueParser parser(vm, inp);
    #ifdef USE_EXCEPTION_HANDLING
    try
    #endif
    {
        *val = parser.Parse(typeoff).refnil();
        *err = nullptr;
    }
    #ifdef USE_EXCEPTION_HANDLING
    catch (string &s) {
        *val = nullptr;
        *err = vm.NewString(s);
    }
    #endif
}

static void ParseFlexData(RefObj **val, LString **err, VM &vm, type_elem_t typeoff,
                          flexbuffers::Reference r) {
    FlexBufferParser parser(vm);
    #ifdef USE_EXCEPTION_HANDLING
    try
    #endif
    {
        *val = parser.Parse(typeoff, r).refnil();
        *err = nullptr;
    }
    #ifdef USE_EXCEPTION_HANDLING
    catch (string &s) {
        *val = nullptr;
        *err = vm.NewString(s);
    }
    #endif
}

static void ParseLobsterBinaryData(RefObj **val, LString **err, VM &vm, type_elem_t typeoff,
                                   const uint8_t *data, size_t size) {
    LobsterBinaryParser parser(vm);
    #ifdef USE_EXCEPTION_HANDLING
    try
    #endif
    {
        *val = parser.Parse(typeoff, data, data + size).refnil();
        *err = nullptr;
    }
    #ifdef USE_EXCEPTION_HANDLING
    catch (string &s) {
        *val = nullptr;
        *err = vm.NewString(s);
    }
    #endif
}

BuiltinGroup parsedata_builtins;
#define BUILTIN_GROUP parsedata_builtins
#define BUILTIN_SYM(name) builtin_##name

BUILTIN_OUTS(parse_data, "typeid,stringdata", "TS", "A1?S?",
    "parses a string containing a data structure in lobster syntax (what you get if you convert"
    " an arbitrary data structure to a string) back into a data structure. supports"
    " int/float/string/vector and classes. classes will be forced to be compatible with their "
    " current definitions, i.e. too many elements will be truncated, missing elements will be"
    " set to 0/nil if possible. useful for simple file formats. returns the value and an error"
    " string as second return value (or nil if no error)")
(VM &vm, RefObj **val, LString **err, iint type, LString *ins) {
    ParseData(val, err, vm, (type_elem_t)type, ins->strv());
}

BUILTIN(flexbuffers_value_to_binary, "val,max_nesting,cycle_detection", "AI?B?", "S",
    "turns any reference value into a flexbuffer. max_nesting defaults to 100. "
    "cycle_detection is by default off (expensive)")
(VM &vm, RefObj *ref, iint mn, iint cycle_detect) {
    auto val = Value(ref);
    ToFlexBufferContext fbc(vm, 1024, flexbuffers::BUILDER_FLAG_SHARE_KEYS);
    if (mn > 0) fbc.max_depth = mn;
    fbc.cycle_detect = (cycle_detect != 0);
    val.ToFlexBuffer(fbc, ref ? ref->ti(vm).t : RTT_NIL, {}, (type_elem_t)0);
    fbc.builder.Finish();
    if (!fbc.cycle_hit.empty())
        vm.BuiltinError("flexbuffers_value_to_binary: data structure contains a cycle: " +
                        fbc.cycle_hit);
    if (!fbc.max_depth_hit.empty())
        vm.BuiltinError(
            "flexbuffers_value_to_binary: data structure exceeds max nesting depth: " +
            fbc.max_depth_hit);
    auto s = vm.NewString(
        string_view((const char *)fbc.builder.GetBuffer().data(), fbc.builder.GetSize()));
    return s;
}

BUILTIN_OUTS(flexbuffers_binary_to_value, "typeid,flex", "TS", "A1?S?",
    "turns a flexbuffer into a value")
(VM &vm, RefObj **val, LString **err, iint id, LString *flex) {
    auto fsv = flex->strv();
    vector<uint8_t> reuse_buffer;
    if (flexbuffers::VerifyBuffer((const uint8_t *)fsv.data(), fsv.size(), &reuse_buffer)) {
        auto root = flexbuffers::GetRoot((const uint8_t *)fsv.data(), fsv.size());
        ParseFlexData(val, err, vm, (type_elem_t)id, root);
    } else {
        *val = nullptr;
        *err = vm.NewString("flexbuffer binary does not verify!");
    }
}

BUILTIN_OUTS(flexbuffers_binary_to_json, "flex,field_quotes,indent_string", "SBS", "S?S?",
    "turns a flexbuffer into a JSON string. If indent_string is empty, will be a single line string")
(VM &vm, LString **json_, LString **err, LString *flex, iint quoted, LString *indent_string_) {
    auto indent_string = indent_string_->strvnt();
    auto fsv = flex->strv();
    vector<uint8_t> reuse_buffer;
    if (flexbuffers::VerifyBuffer((const uint8_t *)fsv.data(), fsv.size(), &reuse_buffer)) {
        auto root = flexbuffers::GetRoot((const uint8_t *)fsv.data(), fsv.size());
        string json;
        root.ToString(true, quoted, json, indent_string.size() != 0, 0, indent_string.c_str(), true);
        *json_ = vm.NewString(json);
        *err = nullptr;
    } else {
        *json_ = nullptr;
        *err = vm.NewString("flexbuffer binary does not verify!");
    }
}

BUILTIN(flexbuffers_json_to_binary, "json,filename_for_errors", "SS?", "SS?",
    "turns a JSON string into a flexbuffer, second value is error, if any")
(VM &vm, LString **binary, LString *json, LString *filename) {
    flexbuffers::Builder builder;
    flatbuffers::Parser parser;
    LString *err = nullptr;
    auto fn = (filename != nullptr)
        ? filename->strvnt()
        : string_view_nt("{flexbuffers_json_to_binary}");
    if (!parser.ParseFlexBuffer(json->strv().data(), fn.c_str(),
                                &builder)) {
        err = vm.NewString(parser.error_);
        *binary = vm.NewString("");
    } else {
        *binary = vm.NewString(
            string_view((const char *)builder.GetBuffer().data(), builder.GetSize()));
    }
    return err;
}

BUILTIN(lobster_value_to_binary, "val", "A", "S",
    "turns any reference value into a binary using a fast & compact Lobster native serialization format. "
    "this is intended for threads/networking, not for storage (since it is not readable by other languages). "
    "data structures participating must have been marked by attribute serializable. "
    "does not provide protection against cycles, use flexbuffers if that is a concern. ")
(VM &vm, RefObj *ref) {
    vector<uint8_t> buf;
    auto val = Value(ref);
    val.ToLobsterBinary(vm, buf, ref ? ref->ti(vm).t : RTT_NIL);
    // FIXME: since this is meant to be fast, worth seeing if this can be made 0-copy?
    auto s = vm.NewString(
        string_view((const char *)buf.data(), buf.size()));
    return s;
}

BUILTIN_OUTS(lobster_binary_to_value, "typeid,bin", "TS", "A1?S?",
    "turns binary created by lobster_value_to_binary back into a value")
(VM &vm, RefObj **val, LString **err, iint id, LString *bin) {
    auto fsv = bin->strv();
    ParseLobsterBinaryData(val, err, vm, (type_elem_t)id, (const uint8_t *)fsv.data(),
                           fsv.size());
}

}
