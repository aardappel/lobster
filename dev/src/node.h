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
    
struct SlabAllocatedSmall
{
    #undef new
    void *operator new(size_t size)                         { return parserpool->alloc_small(size); }
    void *operator new(size_t size, int, const char *, int) { return parserpool->alloc_small(size); }
    void operator delete(void *p)                           { parserpool->dealloc_small(p); };
    void operator delete(void *p, int, const char *, int)   { parserpool->dealloc_small(p); }
    #ifdef WIN32
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #endif
    #endif
};

struct AST : SlabAllocatedSmall
{
    TType type;

    Type exptype;

    int linenumber;
    int fileidx;

    AST(Lex &lex, TType _t) : type(_t)
    {
        linenumber = lex.errorline;
        fileidx = lex.fileidx;
    }
};

struct IntConst : AST { int integer_;      IntConst(Lex &lex, int i)            : AST(lex, T_INT), integer_(i) {}; };  // T_INT
struct FltConst : AST { double flt_;       FltConst(Lex &lex, double f)         : AST(lex, T_FLOAT), flt_(f) {}; };  // T_FLOAT
struct StrConst : AST { char *str_;        StrConst(Lex &lex, const string &s)  : AST(lex, T_STR), str_(parserpool->alloc_string_sized(s.c_str())) {}; };  // T_STR
struct IdRef    : AST { Ident *ident_;     IdRef   (Lex &lex, Ident *id)        : AST(lex, T_IDENT), ident_(id) {} }; // T_IDENT
struct StRef    : AST { Struct *st_;       StRef   (Lex &lex, Struct *st)       : AST(lex, T_STRUCT), st_(st) {} };  // T_STRUCT
struct FldRef   : AST { SharedField *fld_; FldRef  (Lex &lex, SharedField *fld) : AST(lex, T_FIELD), fld_(fld) {} };  // T_FIELD
struct FunRef   : AST { SubFunction *sf_;  FunRef  (Lex &lex, SubFunction *sf)  : AST(lex, T_FUN), sf_(sf) {} };  // T_FUN
struct NatRef   : AST { NativeFun *nf_;    NatRef  (Lex &lex, NativeFun *nf)    : AST(lex, T_NATIVE), nf_(nf) {} };  // T_NATIVE
struct TypeNode : AST { Type type_;        TypeNode(Lex &lex, TType t)          : AST(lex, t) {} };  // T_TYPE, T_NIL

// Inverted subtyping: rather than have most of the compiler deal with the AST base type (which would require a ton
// of virtual methods or casts to access the subtypes), we make Node (the most common occurring subtype) the
// type we use, and give it accessors that can read all fields of all subtypes, protected by an assert.
// This is safe, fast, and memory efficient.
struct Node : AST
{
    private:
    Node *a_, *b_;
    public:

    Node(Lex &lex, TType t, AST *a, AST *b) : AST(lex, t), a_((Node *)a), b_((Node *)b) {};

    bool HasChildren() const { return TCat(type) != TT_NOCHILD; }

    int integer()      const { assert(type == T_INT);    return ((const IntConst *)this)->integer_; }
    double flt()       const { assert(type == T_FLOAT);  return ((const FltConst *)this)->flt_; }
    char * str()       const { assert(type == T_STR);    return ((const StrConst *)this)->str_; }
    char *&str()             { assert(type == T_STR);    return ((      StrConst *)this)->str_; }
    Ident *ident()     const { assert(type == T_IDENT);  return ((const IdRef *)this)->ident_; }
    Struct *st()       const { assert(type == T_STRUCT); return ((const StRef *)this)->st_; }
    SharedField *fld() const { assert(type == T_FIELD);  return ((const FldRef *)this)->fld_; }
    NativeFun * nf()   const { assert(type == T_NATIVE); return ((const NatRef *)this)->nf_; }
    NativeFun *&nf()         { assert(type == T_NATIVE); return ((NatRef *)this)->nf_; }
    SubFunction * sf() const { assert(type == T_FUN);    return ((const FunRef *)this)->sf_; }
    SubFunction *&sf()       { assert(type == T_FUN);    return ((FunRef *)this)->sf_; }
    Type typenode()    const { assert(type == T_TYPE || type == T_NIL); return ((const TypeNode *)this)->type_; }

    Node * a() const { assert(TCat(type) != TT_NOCHILD); return a_; }
    Node *&a()       { assert(TCat(type) != TT_NOCHILD); return a_; }
    Node * b() const { assert(TCat(type) != TT_NOCHILD); return b_; }
    Node *&b()       { assert(TCat(type) != TT_NOCHILD); return b_; }

    Node * left()  const { assert(TCat(type) == TT_BINARY); return a_; }
    Node *&left()        { assert(TCat(type) == TT_BINARY); return a_; }
    Node * right() const { assert(TCat(type) == TT_BINARY); return b_; }
    Node *&right()       { assert(TCat(type) == TT_BINARY); return b_; }

    Node * child() const { assert(TCat(type) == TT_UNARY); return a_; }
    Node *&child()       { assert(TCat(type) == TT_UNARY); return a_; }

    #define GEN_ACCESSOR_NO(ENUM, NAME, AB)
    #define GEN_ACCESSOR_YES(ENUM, NAME, AB) \
              Node *&NAME()       { assert(type == ENUM); return AB; } \
        const Node * NAME() const { assert(type == ENUM); return AB; }
    #define T(ENUM, STR, CAT, HASLEFT, LEFT, HASRIGHT, RIGHT) GEN_ACCESSOR_##HASLEFT(ENUM, LEFT, a_) \
                                                              GEN_ACCESSOR_##HASRIGHT(ENUM, RIGHT, b_)
    TTYPES_LIST
    #undef T
    #undef GEN_ACCESSOR_NO
    #undef GEN_ACCESSOR_YES

    ~Node()
    {
        if (type == T_STR)
        {
            parserpool->dealloc_sized(str());
        }
        else if (HasChildren())
        {
            if (a()) delete a();
            if (b()) delete b();
        }
    }

    Node *Clone()
    {
        auto n = (Node *)parserpool->clone_obj_small_unknown(this);
        if (HasChildren())
        {
            if (a()) n->a() = a()->Clone();
            if (b()) n->b() = b()->Clone();
        }
        else if (type == T_STR)
        {
            n->str() = parserpool->alloc_string_sized(str());
        }
        return n;
    }

    bool IsConst()      // used to see if a var is worth outputting in a stacktrace
    {
        switch (type)
        {
            case T_INT:
            case T_FLOAT:
            case T_STR:
            case T_FUN:
                return true;

            case T_IDENT:
                return ident()->static_constant;

            case T_CONSTRUCTOR:
            {
                for (Node *n = constructor_args(); n; n = n->tail())
                {
                    if (!n->head()->IsConst()) return false;
                }
                return true;
            }

            // TODO: support more types of exps?

            default:
                return false;
        }
    }

    int ClosureArgs()
    {
        return type == T_NIL ? 0 : (type == T_COCLOSURE ? 1 : sf()->parent->nargs());
    }
};


// this "evaluates" an exp, by iterating thru all subexps and thru function calls, ignoring recursive calls,
// and tracking the value of idents as the value nodes they refer to
const char *FindIdentsUpToYield(const Node *start_call, const function<void(const vector<const Ident *> &istack)> &customf)
{
    vector<const Ident *> istack;
    vector<const Node *> vstack; // current value of each ident
    vector<const Function *> fstack;

    const char *err = nullptr;

    auto lookup = [&](const Node *n) -> const Node *
    {
        if (n->type == T_IDENT)
        for (size_t i = 0; i < istack.size(); i++)
            if (n->ident() == istack[i] && vstack[i])
                return vstack[i];

        return n;
    };

    std::function<void(const Node *)> eval;
    std::function<void(const SubFunction *, const Node *, bool)> evalblock;
    std::function<void(const Node *)> evalnatarg;

    eval = [&] (const Node *n)
    {
        if (!n) return;

        //customf(n, istack);

        if (n->type == T_FUN)
            return;

        if (n->type == T_LIST && n->head()->type == T_DEF)
        {
            eval(n->head());
            for (auto dl = n->head(); dl->type == T_DEF; dl = dl->right())
            {
                // FIXME: this is incorrect in the multiple assignments case, though not harmful
                auto val = lookup(dl->right());
                istack.push_back(dl->left()->ident());
                vstack.push_back(val);
            }
            eval(n->tail());
            for (auto dl = n->head(); dl->type == T_DEF; dl = dl->right())
            {
                istack.pop_back();
                vstack.pop_back();
            }
            return;
        }

        if (n->HasChildren())
        {
            eval(n->a());
            eval(n->b());
        }

        switch (n->type)
        {
            case T_CALL:
            {
                auto cf = n->call_function()->sf()->parent;
                for (auto f : fstack) if (f == cf) return;    // ignore recursive call
                for (auto args = n->call_args(); args; args = args->tail())
                    if (args->head()->type == T_COCLOSURE && n != start_call) return;  // coroutine constructor, don't enter
                fstack.push_back(cf);
                if (cf->multimethod) err = "multi-method call";
                evalblock(cf->subf, n->call_args(), false);
                fstack.pop_back();
                break;
            }
            case T_DYNCALL:
            {
                auto f = lookup(n->dcall_fval());
                if (f->type == T_COCLOSURE) { customf(istack); return; }
                // ignore dynamic calls to non-function-vals, could make this an error?
                if (f->type != T_FUN) { assert(0); return; }
                evalblock(f->sf(), n->dcall_info()->dcall_args(), false);
                break;
            }
            case T_NATCALL:
            {
                for (const Node *list = n->ncall_args(); list; list = list->tail())
                {
                    evalnatarg(list->head());
                }
                break;
            }
            case T_IF:
                evalnatarg(n->if_condition());
                evalnatarg(n->if_branches()->left());
                evalnatarg(n->if_branches()->right());
                break;
            case T_WHILE:
                evalnatarg(n->while_condition());
                evalnatarg(n->while_body());
                break;
            case T_FOR:
                evalnatarg(n->for_iter());
                evalnatarg(n->for_body());
                break;
        }
    };

    evalnatarg = [&] (const Node *arg)
    {
        auto a = lookup(arg);
        if (a->type == T_COCLOSURE) customf(istack);
        if (a->type == T_FUN) evalblock(a->sf(), nullptr, true);
    };

    evalblock = [&] (const SubFunction *sf, const Node *args, bool fakeargs)
    {
        const Node *a = args;
        for (auto &arg : sf->args.v)
        {
            if (fakeargs)
            {
                istack.push_back(arg.id);
                vstack.push_back(nullptr);
            }
            else if (a)
            {
                auto val = lookup(a->head());
                istack.push_back(arg.id);
                vstack.push_back(val);
                a = a->tail();
            }
            else
            {
                // this is a _ var that's referring to a past version, ok to ignore
            }
        }

        eval(sf->body);

        a = args;
        for (auto &arg : sf->args.v) if (a)
        {
            (void)arg;
            istack.pop_back();
            vstack.pop_back();
            a = a->tail();
        }
    };

    eval(start_call);

    return err;
}

inline void DumpType(const Node &n, string &ns, SymbolTable &symbols)
{
    if (n.exptype.t != V_ANY)
    {
        ns += ":";
        ns += symbols.TypeName(n.exptype);
    }
}

inline string Dump(const Node &n, int indent, SymbolTable &symbols)
{
    switch (n.type)
    {
        case T_INT:   return inttoa(n.integer());
        case T_FLOAT: return flttoa(n.flt());
        case T_STR:   return string("\"") + n.str() + "\"";
        case T_NIL:   return "nil";

        case T_IDENT:  return n.ident()->name;
        case T_STRUCT: return n.st()->name;
        case T_FIELD:  return n.fld()->name;
        case T_NATIVE: return n.nf()->name;
        case T_TYPE:   return symbols.TypeName(n.typenode());

        case T_FUN:    return n.sf()
            ? "[fun " + n.sf()->parent->name + "]" /*+ sf()->body->Dump(indent + 2, symbols) */
            : "<>";

        default:
        {
            string s = TName(n.type);

            string as, bs;
            bool ml = false;
            auto indenb = indent - (n.type == T_LIST) * 2;

            if (n.HasChildren())
            {
                if (n.a()) { as = Dump(*n.a(), indent + 2, symbols); DumpType(*n.a(), as, symbols); if (as[0] == ' ') ml = true; }
                if (n.b()) { bs = Dump(*n.b(), indenb + 2, symbols); DumpType(*n.b(), bs, symbols); if (bs[0] == ' ') ml = true; }
            }

            if (as.size() + bs.size() > 60) ml = true;

            if (ml)
            {
                if (n.a()) { if (as[0] != ' ') as = string(indent + 2, ' ') + as; }
                if (n.b()) { if (bs[0] != ' ') bs = string(indenb + 2, ' ') + bs; }
                if (n.type == T_LIST)
                {
                    s = "";
                }
                else
                {
                    s = string(indent, ' ') + s;
                    if (n.a()) s += "\n";
                }
                if (n.a()) s += as;
                if (n.b()) s += "\n" + bs;
                return s;
            }
            else
            {
                if (n.HasChildren() && n.b()) return "(" + s + " " + as + " " + bs + ")";
                else return "(" + s + " " + as + ")";
            }
        }
    }
}

}  // namespace lobster
