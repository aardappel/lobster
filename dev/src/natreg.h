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

struct Type
{
    ValueType t;
    int idx;        // if t == V_VECTOR

    Type()             : t(V_UNKNOWN), idx(-1) {}
    Type(ValueType _t) : t(_t),        idx(-1) {}

    const char *Name()
    {
        return idx >= 0 ? g_vm->ReverseLookupType(idx).c_str() : TypeName(t);
    }
    
    bool operator==(const Type &o) { return t == o.t && idx == o.idx; }
    bool operator!=(const Type &o) { return t != o.t || idx != o.idx; }
    bool operator< (const Type &o) { return t <  o.t || (t == o.t && idx < o.idx); }
};

enum ArgFlags { AF_NONE, NF_EXPFUNVAL, NF_OPTIONAL };

struct Arg
{
    Type type;
    ArgFlags flags;
    string id;

    void Set(char t, const string &name)
    {
        id = name;
        flags = AF_NONE;
        if (t >= 'a') { flags = NF_OPTIONAL; t -= 'a' - 'A'; }
        switch (t)
        {
            case 'I': type.t = V_INT; break;
            case 'F': type.t = V_FLOAT; break;
            case 'S': type.t = V_STRING; break;
            case 'V': type.t = V_VECTOR; break;
            case 'E': type.t = V_FUNCTION; flags = NF_EXPFUNVAL; break;
            case 'C': type.t = V_FUNCTION; break;
            case 'R': type.t = V_COROUTINE; break;
            case 'A': type.t = V_UNKNOWN; break;
            default:  assert(0);
        }
    }
};

struct Name : Serializable
{
    string name;
    int idx;
    bool isprivate;

    Name()                                  :              idx(-1),   isprivate(false) {}
    Name(const string &_name, int _idx = 0) : name(_name), idx(_idx), isprivate(false) {}

    void Serialize(Serializer &ser)
    {
        ser(name);
        ser(idx);
    }
};

struct BuiltinPtr
{
    union 
    {
        Value (*f0)();
        Value (*f1)(Value &);
        Value (*f2)(Value &, Value &);
        Value (*f3)(Value &, Value &, Value &);
        Value (*f4)(Value &, Value &, Value &, Value &);
        Value (*f5)(Value &, Value &, Value &, Value &, Value &);
        Value (*f6)(Value &, Value &, Value &, Value &, Value &, Value &);
    };
};

enum NativeCallMode { NCM_NONE, NCM_CONTINUATION, NCM_CONT_EXIT, NCM_LOOP, NCM_WHILE };

struct NativeFun : Name
{
    BuiltinPtr fun;

    int nargs, nretvalues;
    Arg *args, *retvals;

    NativeCallMode ncm;
    Value (*cont1)(Value &);

    const char *help;

    int subsystemid;

    ~NativeFun() { delete[] args; delete[] retvals; }

    NativeFun(const char *_name, BuiltinPtr f, const char *ids, const char *typeids, const char *rets, int _nargs,
              const char *_help, NativeCallMode _ncm, Value (*_cont1)(Value &))
        : Name(string(_name), 0), fun(f), nargs(_nargs), nretvalues(strlen(rets)), ncm(_ncm), cont1(_cont1),
               help(_help), subsystemid(-1)
    {
        assert(strlen(typeids) == nargs);
        args = new Arg[nargs];

        for (int i = 0; i < nargs; i++)
        {
            const char *idend = strchr(ids, ',');
            if (!idend)
            {
                // if this fails, you're not specifying enough arg names in the comma separated list
                assert(i == nargs - 1);
                idend = ids + strlen(ids);
            }
            args[i].Set(typeids[i], string(ids, idend)); 
            ids = idend + 1;
        }

        retvals = new Arg[nretvalues];
        for (int i = 0; i < nretvalues; i++)
        {
            retvals[i].Set(rets[i], string());
        }
    }
};

struct NativeRegistry
{
    vector<NativeFun *> nfuns;
    map<string, NativeFun *> nfunlookup;
    vector<string> subsystems;

    ~NativeRegistry()
    {
        for (auto f : nfuns) delete f;
    }

    void NativeSubSystemStart(const char *name) { subsystems.push_back(name); }

    void Register(NativeFun *nf)
    {
        nf->idx = (int)nfuns.size();
        nf->subsystemid = subsystems.size() - 1;

        if (nfunlookup[nf->name])
        {
            assert(0);
            throw "native library name clash: " + nf->name;
        }

        nfunlookup[nf->name] = nf;

        nfuns.push_back(nf);
    }

    NativeFun *FindNative(const string &name)
    {
        auto it = nfunlookup.find(name);
        return it != nfunlookup.end() ? it->second : NULL;
    }
};

extern NativeRegistry natreg;

struct AutoRegister;

extern AutoRegister *autoreglist;

struct AutoRegister
{
    AutoRegister *next;
    const char *name;
    void (* regfun)();
    AutoRegister(const char *_name, void (* _rf)())
        : next(autoreglist), name(_name), regfun(_rf) { autoreglist = this; }
};

#define STARTDECL(name) struct ___##name { static Value s_##name

#define MIDDECL(name) static Value mid_##name

#define ENDDECL_(name, ids, types, rets, help, field, ncm, cont1) }; { \
    BuiltinPtr bp; bp.f##field = &___##name::s_##name; \
    natreg.Register(new NativeFun(#name, bp, ids, types, rets, field, help, ncm, cont1)); }

#define ENDDECL0(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 0, NCM_NONE, NULL)
#define ENDDECL1(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 1, NCM_NONE, NULL)
#define ENDDECL2(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 2, NCM_NONE, NULL)
#define ENDDECL3(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 3, NCM_NONE, NULL)
#define ENDDECL4(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 4, NCM_NONE, NULL)
#define ENDDECL5(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 5, NCM_NONE, NULL)
#define ENDDECL6(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 6, NCM_NONE, NULL)

#define ENDDECL3CONT(    name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 3, NCM_CONTINUATION, NULL)
#define ENDDECL2CONTEXIT(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 2, NCM_CONT_EXIT, \
                                                                                                 &___##name::mid_##name)
#define ENDDECL3CONTEXIT(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 3, NCM_CONT_EXIT, \
                                                                                                 &___##name::mid_##name)
#define ENDDECL2LOOP(    name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 2, NCM_LOOP, NULL)
#define ENDDECL2WHILE(   name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 2, NCM_WHILE, NULL)
