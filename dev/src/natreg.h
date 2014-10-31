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
    ValueType t, t2, t3, t4;  // If t == V_VECTOR|V_NILABLE, t2 is the contained type, etc.
                              // Means we only allow vector types to nest 3 deep.
    int idx;                  // if any t* == V_STRUCT

    Type()                       : t(V_ANY), t2(V_ANY), t3(V_ANY), t4(V_ANY), idx(-1) {}
    Type(ValueType _t)           : t(_t),    t2(V_ANY), t3(V_ANY), t4(V_ANY), idx(-1) {}
    Type(ValueType _t, int _idx) : t(_t),    t2(V_ANY), t3(V_ANY), t4(V_ANY), idx(_idx) {}

    bool operator==(const Type &o) const { return t == o.t && idx == o.idx && t2 == o.t2 && t3 == o.t3 && t4 == o.t4; }
    bool operator!=(const Type &o) const { return !(*this == o); }

    // This one is used to sort types for multi-dispatch.
    bool operator< (const Type &o) const
    {
        return              t  < o.t                                     ||
            (t  == o.t  && (t2 < o.t2 || (t  == V_STRUCT && idx < o.idx) ||
            (t2 == o.t2 && (t3 < o.t3 || (t2 == V_STRUCT && idx < o.idx) ||
            (t3 == o.t3 && (t4 < o.t4 || (t3 == V_STRUCT && idx < o.idx) || 
            (t4 == o.t4 &&               (t4 == V_STRUCT && idx < o.idx))))))));
    }

    Type Element() const
    {
        assert(t == V_VECTOR || t == V_NILABLE);
        Type s = *this;
        s.t = s.t2;
        s.t2 = s.t3;
        s.t3 = s.t4;
        s.t4 = V_ANY;
        return s;
    }

    bool CanWrap() const { return t4 == V_ANY; }

    Type Wrap(ValueType with = V_VECTOR) const
    {
        assert(t4 == V_ANY);
        Type v = *this;
        v.t4 = v.t3;
        v.t3 = v.t2;
        v.t2 = v.t;
        v.t = with;
        return v;
    }

    bool Numeric() const { return t == V_INT || t == V_FLOAT; }
};

enum ArgFlags { AF_NONE, NF_EXPFUNVAL, NF_OPTIONAL, AF_ANYTYPE };

struct Arg
{
    Type type;
    ArgFlags flags;
    string id;

    void Set(char t, const string &name)
    {
        id = name;
        flags = AF_NONE;
        bool optional = false;
        if (t >= 'a') { optional = true; t -= 'a' - 'A'; }
        switch (t)
        {
            case 'I': type.t = V_INT; break;
            case 'F': type.t = V_FLOAT; break;
            case 'S': type.t = V_STRING; break;
            case 'V': type.t = V_VECTOR; break;
            case 'E': type.t = V_FUNCTION; flags = NF_EXPFUNVAL; break;
            case 'C': type.t = V_FUNCTION; break;
            case 'R': type.t = V_COROUTINE; break;
            case 'A': type.t = V_ANY; break;
            default:  assert(0);
        }
        if (optional)
        {
            assert(type.CanWrap());
            type = type.Wrap(V_NILABLE);
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

enum NativeCallMode { NCM_NONE, NCM_CONT_EXIT };

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
        return it != nfunlookup.end() ? it->second : nullptr;
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

#define ENDDECL0(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 0, NCM_NONE, nullptr)
#define ENDDECL1(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 1, NCM_NONE, nullptr)
#define ENDDECL2(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 2, NCM_NONE, nullptr)
#define ENDDECL3(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 3, NCM_NONE, nullptr)
#define ENDDECL4(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 4, NCM_NONE, nullptr)
#define ENDDECL5(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 5, NCM_NONE, nullptr)
#define ENDDECL6(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 6, NCM_NONE, nullptr)

#define ENDDECL2CONTEXIT(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 2, NCM_CONT_EXIT, \
                                                                                                 &___##name::mid_##name)
#define ENDDECL3CONTEXIT(name, ids, types, rets, help) ENDDECL_(name, ids, types, rets, help, 3, NCM_CONT_EXIT, \
                                                                                                 &___##name::mid_##name)
