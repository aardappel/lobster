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

struct NativeFun;
struct SymbolTable;
struct Node;

struct LineInfo : Serializable
{
    int line;
    int fileidx;
    int bytecodestart;

    LineInfo(int l, int i, int b) : line(l),  fileidx(i),  bytecodestart(b)  {}
    LineInfo() : LineInfo(-1, -1, -1) {}

    void Serialize(Serializer &ser)
    {
        ser(line);
        ser(fileidx);
        ser(bytecodestart);
    }
};

struct SubFunction;

struct Ident : Named
{
    int line;
    size_t scope;
    
    Ident *prev;

    SubFunction *sf_def;    // Where it is defined, including anonymous functions.
    
    bool single_assignment;
    bool constant;
    bool static_constant;
    bool anonymous_arg;

    int logvaridx;

    TypeRef type;

    Ident(const string &_name, int _l, int _idx, size_t _sc)
        : Named(_name, _idx), line(_l), 
          scope(_sc), prev(nullptr), sf_def(nullptr),
          single_assignment(true), constant(false), static_constant(false), anonymous_arg(false), logvaridx(-1) {}
    Ident() : Ident("", -1, 0, SIZE_MAX) {}

    void Serialize(Serializer &ser)
    {
        Named::Serialize(ser);
        ser(line);
        ser(static_constant);
    }
    
    void Assign(Lex &lex)
    {
        single_assignment = false;
        if (constant)
            lex.Error("variable " + name + " is constant");
    }
};

struct SharedField : Named  // Only still needed because we have no idea which struct it refers to at parsing time.
{
    SharedField(const string &_name, int _idx) : Named(_name, _idx) {}
    SharedField() : SharedField("", 0) {}
};

struct Field : Typed<SharedField>
{
    int fieldref;
    Node *defaultval;  // Not deleted, possibly referred to by multiple structs.

    Field(SharedField *_id, TypeRef _type, bool _generic, int _fieldref, Node *_defaultval)
        : Typed(_id, _type, _generic),
          fieldref(_fieldref), defaultval(_defaultval) {}
};

struct Struct : Named
{
    vector<Field> fields; 

    Struct *next, *first;

    Struct *superclass;
    
    Struct *firstsubclass, *nextsubclass;  // Used in codegen.

    bool readonly;
    bool generic;
    bool predeclaration;

    Type thistype;       // convenient place to store the type corresponding to this
    TypeRef vectortype;  // What kind of vector this can be demoted to.

    Struct(const string &_name, int _idx)
        : Named(_name, _idx), next(nullptr), first(this), superclass(nullptr),
          firstsubclass(nullptr), nextsubclass(nullptr),
          readonly(false), generic(false), predeclaration(false),
          thistype(V_STRUCT, this),
          vectortype(type_vector_any) {}
    Struct() : Struct("", 0) {}

    void Serialize(Serializer &ser)
    {
        Named::Serialize(ser);
        ser(readonly);
    }

    int Has(SharedField *fld)
    {
        for (auto &uf : fields) if (uf.id == fld) return int(&uf - &fields[0]);
        return -1;
    }

    Struct *CloneInto(Struct *st)
    {
        *st = *this;
        st->thistype = Type(V_STRUCT, st);
        st->next = next;
        st->first = first;
        next = st;
        return st;
    }

    bool IsSpecialization(Struct *other)
    {
        if (generic)
        {
            for (auto struc = first->next; struc; struc = struc->next) if (struc == other) return true;
            return false;
        }
        else
        {
            return this == other;
        }
    }

    void Resolve(Field &field)
    {
        if (field.fieldref >= 0) field.type = fields[field.fieldref].type;
    }
};

struct Function;

struct SubFunction
{
    int idx;
    
    ArgVector args;
    ArgVector locals;
    ArgVector dynscoperedefs;  // any lhs of <-
    ArgVector freevars;        // any used from outside this scope, could overlap with dynscoperedefs
    vector<TypeRef> returntypes;

    ArgVector coyieldsave;
    TypeRef coresumetype;

    Node *body;

    SubFunction *next;
    Function *parent;

    int subbytecodestart;

    bool typechecked, freevarchecked, iscoroutine;

    Type thistype;       // convenient place to store the type corresponding to this

    SubFunction(int _idx)
        : idx(_idx),
          parent(nullptr), args(0, nullptr), locals(0, nullptr), dynscoperedefs(0, nullptr), freevars(0, nullptr),
          coyieldsave(0, nullptr),
          body(nullptr), next(nullptr), subbytecodestart(0),
          typechecked(false), freevarchecked(false), iscoroutine(false),
          thistype(V_FUNCTION, this)
    {
        returntypes.push_back(type_any);  // functions always have at least 1 return value.
    }

    void SetParent(Function &f, SubFunction *&link)
    {
        parent = &f;
        next = link;
        link = this;
    }

    void CloneIds(SubFunction &o)
    {
        args = o.args;
        locals = o.locals;
        dynscoperedefs = o.dynscoperedefs;
        freevars = o.freevars;
    }

    ~SubFunction()
    {
        if (next) delete next;
    }
};

struct Function : Named
{
    int bytecodestart;

    SubFunction *subf; // functions with the same name and args, but different types (dynamic dispatch | specialization) 
    Function *sibf;    // functions with the same name but different number of args (overloaded)

    bool multimethod;  // if false, subfunctions can be generated by type specialization as opposed to programmer
                       // implemented dynamic dispatch

    bool anonymous;    // does not have a programmer specified name
    bool istype;       // its merely a function type, has no body, but does have a set return type.

    ArgVector orig_args; // Store the original types the function was declared with, before specialization.

    int scopelevel;
    int retvals;

    Function(const string &_name, int _idx, int _sl)
     : Named(_name, _idx), bytecodestart(0),  subf(nullptr), sibf(nullptr),
       multimethod(false), anonymous(false), istype(false), orig_args(0, nullptr),
       scopelevel(_sl), retvals(0)
    {
    }
    Function() : Function("", 0, -1) {}
    ~Function() { if (subf) delete subf; }

    int nargs() { return (int)subf->args.v.size(); }

    void Serialize(Serializer &ser)
    {
        Named::Serialize(ser);
        ser(bytecodestart);
        ser(retvals);
    }
};

inline string TypeName(TypeRef type)
{
    switch (type->t)
    {
        case V_STRUCT: return type->struc->name;
        case V_VECTOR: return type->Element()->t == V_VAR 
            ? "[]"
            : "[" + TypeName(type->Element()) + "]";
        case V_FUNCTION: return type->sf // || type->sf->anonymous
            ? type->sf->parent->name
            : "function";
        case V_NILABLE: return type->Element()->t == V_VAR
            ? "nil"
            : TypeName(type->Element()) + "?";
        case V_VAR: return type->sub
            ? TypeName(type->sub) + "*"
            : BaseTypeName(type->t);
        case V_COROUTINE: return type->sf
            ? "coroutine(" + type->sf->parent->name + ")"
            : "coroutine";
        default: return BaseTypeName(type->t);
    }
}

struct SymbolTable
{
    map<string, Ident *> idents;
    vector<Ident *> identtable;
    vector<Ident *> identstack;

    map<string, Struct *> structs;
    vector<Struct *> structtable;

    map<string, SharedField *> fields;
    vector<SharedField *> fieldtable;

    map<string, Function *> functions;
    vector<Function *> functiontable;
    vector<SubFunction *> subfunctiontable;

    vector<string> filenames;
    
    vector<size_t> scopelevels;

    vector<pair<TypeRef, Ident *>> withstack;
    vector<size_t> withstacklevels;

    vector<Struct *> default_vector_types;

    bool uses_frame_state;

    // Used during parsing.
    vector<SubFunction *> defsubfunctionstack;

    SymbolTable() : uses_frame_state(false) {}

    ~SymbolTable()
    {
        for (auto id : identtable)    delete id;
        for (auto st : structtable)   delete st;
        for (auto f  : functiontable) delete f;
        for (auto f  : fieldtable)    delete f;
    }
    
    Ident *Lookup(const string &name)
    {
        auto it = idents.find(name);
        return it == idents.end() ? nullptr : it->second;
    }
        
    Ident *LookupAny(const string &name)
    {
        for (auto id : identtable) if (id->name == name) return id;
        return nullptr;
    }

    Ident *LookupDef(const string &name, int line, Lex &lex, bool anonymous_arg, bool islocal)
    {
        auto sf = defsubfunctionstack.empty() ? nullptr : defsubfunctionstack.back();

        auto existing_ident = Lookup(name);
        if (anonymous_arg && existing_ident && existing_ident->sf_def == sf) return existing_ident;

        Ident *ident = nullptr;
        if (LookupWithStruct(name, lex, ident))
            lex.Error("cannot define variable with same name as field in this scope: " + name);

        ident = new Ident(name, line, (int)identtable.size(), (int)scopelevels.back());
        ident->anonymous_arg = anonymous_arg;

        ident->sf_def = sf;
        if (sf) (islocal ? sf->locals : sf->args).v.push_back(Arg(ident, type_any, true));

        if (existing_ident)
        {
            if (scopelevels.back() == existing_ident->scope) lex.Error("identifier redefinition: " + ident->name);
            ident->prev = existing_ident;
        }
        idents[name] = ident;
        identstack.push_back(ident);
        identtable.push_back(ident);
        return ident;
    }

    Ident *LookupDynScopeRedef(const string &name, Lex &lex)
    {
        auto ident = Lookup(name);
        if (!ident) lex.Error("lhs of <- must refer to existing variable: " + name);
        if (defsubfunctionstack.size()) defsubfunctionstack.back()->dynscoperedefs.Add(Arg(ident, type_any, true));
        return ident;
    }
        
    Ident *LookupMaybe(const string &name)
    {
        auto ident = Lookup(name);
        if (!ident) return nullptr;
        
        if (defsubfunctionstack.size() && ident->sf_def != defsubfunctionstack.back())
        {
            // This is a free variable, record it in all parents up to the definition point.
            for (int i = (int)defsubfunctionstack.size() - 1; i >= 0; i--)
            {
                auto sf = defsubfunctionstack[i];
                if (ident->sf_def == sf) break;  // Found the definition.
                sf->freevars.Add(Arg(ident, type_any, true));
            }
        }
        return ident;  
    }

    Ident *LookupUse(const string &name, Lex &lex)
    {
        auto id = LookupMaybe(name);
        if (!id)
            lex.Error("unknown identifier: " + name);
        return id;  
    }

    void AddWithStruct(TypeRef t, Ident *id, Lex &lex)
    {
        if (t->t != V_STRUCT) lex.Error(":: can only be used with struct/value types");
        for (auto &wp : withstack)
            if (wp.first->struc == t->struc)
                lex.Error("type used twice in the same scope with ::");
        // FIXME: should also check if variables have already been defined in this scope that clash with the struct,
        // or do so in LookupUse
        assert(t->struc);
        withstack.push_back(make_pair(t, id));
    }

    SharedField *LookupWithStruct(const string &name, Lex &lex, Ident *&id)
    {
        auto fld = FieldUse(name);
        if (!fld) return nullptr;

        assert(!id);
        for (auto &wp : withstack)
        {
            if (wp.first->struc->Has(fld) >= 0)
            {
                if (id) lex.Error("access to ambiguous field: " + fld->name);
                id = wp.second;
            }
        }

        return id ? fld : nullptr;
    }
    
    void ScopeStart()
    {
        scopelevels.push_back(identstack.size());
        withstacklevels.push_back(withstack.size());
    }

    void ScopeCleanup()
    {
        while (identstack.size() > scopelevels.back())
        {
            auto ident = identstack.back();
            auto it = idents.find(ident->name);
            if (it != idents.end())   // can already have been removed by private var cleanup
            {
                if (ident->prev) it->second = ident->prev;
                else idents.erase(it);
            }
            
            identstack.pop_back();
        }
        scopelevels.pop_back();

        while (withstack.size() > withstacklevels.back()) withstack.pop_back();
        withstacklevels.pop_back();
    }

    void UnregisterStruct(const Struct *st, Lex &lex)
    {
        if (st->predeclaration) lex.Error("pre-declared struct never defined: " + st->name);
        auto it = structs.find(st->name);
        if (it != structs.end()) structs.erase(it);
    }

    void UnregisterFun(Function *f)
    {
        auto it = functions.find(f->name);
        if (it != functions.end())  // it can already have been remove by another variation
            functions.erase(it);
    }
    
    void EndOfInclude()
    {
        auto it = idents.begin();
        while (it != idents.end())
        {
            if (it->second->isprivate)
            {
                assert(!it->second->prev);
                idents.erase(it++);
            }
            else
                it++;
        }
    }

    Struct &StructDecl(const string &name, Lex &lex)
    {
        Struct *st = structs[name];
        if (st)
        {
            if (!st->predeclaration) lex.Error("double declaration of type: " + name);
            st->predeclaration = false;
        }
        else
        {
            st = new Struct(name, (int)structtable.size());
            structs[name] = st;
            structtable.push_back(st);
        }
        return *st;
    }

    Struct &StructUse(const string &name, Lex &lex)
    {
        Struct *st = structs[name];
        if (!st) lex.Error("unknown type: " + name);
        return *st;
    }

    int StructIdx(const string &name, size_t &nargs) // FIXME: this is inefficient, used by parse_data()
    {
        for (auto s : structtable) if (s->name == name) 
        {
            nargs = s->fields.size();
            return s->idx;
        }
        return -1;
    }

    bool IsSuperTypeOrSame(const Struct *sup, const Struct *sub)
    {
        for (auto t = sub; t; t = t->superclass)
            if (t == sup)
                return true;
        return false;
    }

    const Struct *CommonSuperType(const Struct *a, const Struct *b)
    {
        if (a != b) for (;;)
        {
            a = a->superclass;
            if (!a) return nullptr;
            if (IsSuperTypeOrSame(a, b)) break;
        }
        return a;
    }

    SharedField &FieldDecl(const string &name)
    {
        SharedField *fld = fields[name];
        if (!fld)
        {
            fld = new SharedField(name, (int)fieldtable.size());
            fields[name] = fld;
            fieldtable.push_back(fld);
        }
        return *fld;
    }

    SharedField *FieldUse(const string &name)
    {
        auto it = fields.find(name);
        return it != fields.end() ? it->second : nullptr;
    }
    
    SubFunction *CreateSubFunction()
    {
        auto sf = new SubFunction((int)subfunctiontable.size());
        subfunctiontable.push_back(sf);
        return sf;
    }

    Function &CreateFunction(const string &name, const string &context)
    {
        auto fname = name.length() ? name : "function" + to_string(functiontable.size()) + context;
        auto f = new Function(fname, (int)functiontable.size(), (int)scopelevels.size());
        functiontable.push_back(f);
        return *f;
    }

    Function &FunctionDecl(const string &name, int nargs, Lex &lex)
    {
        auto fit = functions.find(name);

        if (fit != functions.end())
        {
            if (fit->second->scopelevel != int(scopelevels.size()))
                lex.Error("cannot define a variation of function " + name + " at a different scope level");

            for (auto f = fit->second; f; f = f->sibf)
                if (f->nargs() == nargs)
                    return *f;
        }

        auto &f = CreateFunction(name, "");

        if (fit != functions.end())
        {
            f.sibf = fit->second->sibf;
            fit->second->sibf = &f;
        }
        else
        {
            functions[name] = &f;
        }

        return f;
    }

    Function *FindFunction(const string &name)
    {
        auto it = functions.find(name);
        return it != functions.end() ? it->second : nullptr;
    }

    bool ReadOnlyIdent(size_t v) { assert(v < identtable.size());    return identtable[v]->constant;  }
    
    const string &ReverseLookupIdent   (size_t v) const { assert(v < identtable.size());    return identtable[v]->name;    }
    const string &ReverseLookupType    (size_t v) const { assert(v < structtable.size());   return structtable[v]->name;   }
    const string &ReverseLookupFunction(size_t v) const { assert(v < functiontable.size()); return functiontable[v]->name; }

    void RegisterDefaultVectorTypes()
    {
        // TODO: this isn't great hardcoded in the compiler, would be better if it was declared in lobster code
        if (default_vector_types.size()) return;  // Already initialized.
        static const char *default_vector_type_names[] = { "xy", "xyz", "xyzw", nullptr };
        for (auto name = default_vector_type_names; *name; name++)
        {
            Struct *t = nullptr;
            // linear search because we may not have the map available if called from a VM loaded from bytecode.
            for (auto s : structtable) if (s->name == *name) { t = s; break; }
            default_vector_types.push_back(t);
        }
    }

    bool IsGeneric(TypeRef type)
    {
        if (type->t == V_ANY) return true;
        auto u = type->UnWrapped();
        return u->t == V_STRUCT && u->struc->generic;
    }

    Struct *GetVectorType(int which) { assert(which >= 2); return default_vector_types[which - 2]; }
    
    void Serialize(Serializer &ser, vector<int> &code, vector<LineInfo> &linenumbers)
    {
        auto curvers = __DATE__; // __TIME__;
        string vers = curvers;
        ser(vers);
        if (ser.rbuf && vers != curvers) throw string("cannot load bytecode from a different version of the compiler");

        ser(uses_frame_state);

        ser(identtable);
        ser(functiontable);
        ser(structtable);
        ser(fieldtable);

        ser(code);
        ser(filenames);
        ser(linenumbers);
    }
};

}  // namespace lobster
