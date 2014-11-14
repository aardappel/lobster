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

template <typename T> struct SlabAllocated
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

struct Node : SlabAllocated<Node>
{
    TType type;

    private:
    union
    {
        struct { Node *a_, *b_;     }; // default
        int integer_;                  // T_INT
        double flt_;                   // T_FLOAT
        struct { char *str_;        }; // T_STR

        struct { Ident *ident_;     }; // T_IDENT
        struct { Struct *st_;       }; // T_STRUCT
        struct { SharedField *fld_; }; // T_FIELD
        struct { SubFunction *sf_;  }; // T_FUN
        struct { NativeFun *nf_;    }; // T_NATIVE
        struct { Type *type_;       }; // T_TYPE
    };
    public:

    Type exptype;

    int linenumber;
    int fileidx;

    Node(Lex &lex, TType _t)                     : type(_t), a_(nullptr), b_(nullptr) { L(lex); }
    Node(Lex &lex, TType _t, Node *_a)           : type(_t), a_(_a), b_(nullptr)      { L(lex); }
    Node(Lex &lex, TType _t, Node *_a, Node *_b) : type(_t), a_(_a), b_(_b)           { L(lex); }
    Node(Lex &lex, TType _t, int _i)             : type(_t), integer_(_i)             { L(lex); }
    Node(Lex &lex, TType _t, double _f)          : type(_t), flt_(_f)                 { L(lex); }
    Node(Lex &lex, TType _t, string &_s)         : type(_t), str_(parserpool->alloc_string_sized(_s.c_str())) { L(lex); }

    Node(Lex &lex, Ident *_id)        : type(T_IDENT),  ident_(_id)  { assert(_id);  L(lex); }
    Node(Lex &lex, Struct *_st)       : type(T_STRUCT), st_(_st)     { assert(_st);  L(lex); }
    Node(Lex &lex, SharedField *_fld) : type(T_FIELD),  fld_(_fld)   { assert(_fld); L(lex); }
    Node(Lex &lex, SubFunction *_sf)  : type(T_FUN),    sf_(_sf)     {               L(lex); }
    Node(Lex &lex, NativeFun *_nf)    : type(T_NATIVE), nf_(_nf)     { assert(_nf);  L(lex); }
    Node(Lex &lex, Type &_type)       : type(T_TYPE),   type_(parserpool->clone_obj_small(&_type)) { L(lex); }

    void L(Lex &lex)
    {
        linenumber = lex.errorline;
        fileidx = lex.fileidx;
    }

    bool HasChildren() const { return TCat(type) != TT_NOCHILD; }

    int integer()      const { assert(type == T_INT);    return integer_; }
    double flt()       const { assert(type == T_FLOAT);  return flt_; }
    char *str()        const { assert(type == T_STR);    return str_; }
    Ident *ident()     const { assert(type == T_IDENT);  return ident_; }
    Struct *st()       const { assert(type == T_STRUCT); return st_; }
    SharedField *fld() const { assert(type == T_FIELD);  return fld_; }
    NativeFun *nf()    const { assert(type == T_NATIVE); return nf_; }
    Type *typenode()   const { assert(type == T_TYPE);   return type_; }
    SubFunction * sf() const { assert(type == T_FUN);    return sf_; }
    SubFunction *&sf()       { assert(type == T_FUN);    return sf_; }

    Node *a()  const { assert(TCat(type) != TT_NOCHILD); return a_; }
    Node * b() const { assert(TCat(type) != TT_NOCHILD); return b_; }
    Node *&b()       { assert(TCat(type) != TT_NOCHILD); return b_; }

    Node * left()  const { assert(TCat(type) == TT_BINARY); return a_; }
    Node *&left()        { assert(TCat(type) == TT_BINARY); return a_; }
    Node * right() const { assert(TCat(type) == TT_BINARY); return b_; }
    Node *&right()       { assert(TCat(type) == TT_BINARY); return b_; }

    Node *child() const { assert(TCat(type) == TT_UNARY); return a_; }

    #define GEN_ACCESSOR_NO(ENUM, NAME, AB)
    #define GEN_ACCESSOR_YES(ENUM, NAME, AB)       Node *&NAME()       { assert(type == ENUM); return AB; } \
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
            parserpool->dealloc_sized(str_);
        }
        else if (type == T_TYPE)
        {
            parserpool->dealloc_small(type_);
        }
        else if (HasChildren())
        {
            if (a_) delete a_;
            if (b_) delete b_;
        }
    }

    Node *Clone()
    {
        auto n = parserpool->clone_obj_small(this);
        if (HasChildren())
        {
            if (a_) n->a_ = a_->Clone();
            if (b_) n->b_ = b_->Clone();
        }
        else if (type == T_STR)
        {
            n->str_ = parserpool->alloc_string_sized(str_);
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
            case T_CLOSUREDEF:
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
        return type == T_NIL ? 0 : (type == T_COCLOSURE ? 1 : closure_def()->sf()->parent->nargs());
    }

    // this "evaluates" an exp, by iterating thru all subexps and thru function calls, ignoring recursive calls,
    // and tracking the value of idents as the value nodes they refer to
    const char *FindIdentsUpToYield(const function<void (const vector<const Ident *> &istack)> &customf)
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

        eval = [&](const Node *n)
        {
            if (!n) return;

            //customf(n, istack);

            if (n->type == T_CLOSUREDEF || n->type == T_FUNDEF)
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
                eval (n->tail());
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
                        if (args->head()->type == T_COCLOSURE && n != this) return;  // coroutine constructor, don't enter
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
                    if (f->type != T_CLOSUREDEF) { assert(0); return; }
                    evalblock(f->closure_def()->sf(), n->dcall_info()->dcall_args(), false);
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

        evalnatarg = [&](const Node *arg)
        {
            auto a = lookup(arg);
            if (a->type == T_COCLOSURE) customf(istack);
            if (a->type == T_CLOSUREDEF) evalblock(a->closure_def()->sf(), nullptr, true);
        };

        evalblock = [&](const SubFunction *sf, const Node *args, bool fakeargs)
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

        eval(this);

        return err;
    }

    string Dump(int indent, SymbolTable &symbols)
    {
        switch (type)
        {
            case T_INT:   return inttoa(integer());
            case T_FLOAT: return flttoa(flt());
            case T_STR:   return string("\"") + str() + "\"";
            case T_NIL:   return "nil";

            case T_IDENT:  return ident()->name;
            case T_STRUCT: return st()->name;
            case T_FIELD:  return fld()->name;
            case T_FUN:    return sf() ? sf()->parent->name : "<>";
            case T_NATIVE: return nf()->name;
            case T_TYPE:   return symbols.TypeName(*typenode());

            case T_FUNDEF: return "[fundef " + function_def()->sf()->parent->name + "]";

            default:
            {
                string s = TName(type);

                string as, bs;
                bool ml = false;
                auto indenb = indent - (type == T_LIST) * 2;

                if (HasChildren())
                {
                    if (a()) { as = a()->Dump(indent + 2, symbols); DumpType(a(), as, symbols); if (as[0] == ' ') ml = true; }
                    if (b()) { bs = b()->Dump(indenb + 2, symbols); DumpType(b(), bs, symbols); if (bs[0] == ' ') ml = true; }
                }

                if (as.size() + bs.size() > 60) ml = true;

                if (ml)
                {
                    if (a()) { if (as[0] != ' ') as = string(indent + 2, ' ') + as; }
                    if (b()) { if (bs[0] != ' ') bs = string(indenb + 2, ' ') + bs; }
                    if (type == T_LIST)
                    {
                        s = "";
                    }
                    else
                    {
                        s = string(indent, ' ') + s;
                        if (a()) s += "\n";
                    }
                    if (a()) s += as;
                    if (b()) s += "\n" + bs;
                    return s;
                }
                else
                {
                    if (HasChildren() && b()) return "(" + s + " " + as + " " + bs + ")";
                    else return "(" + s + " " + as + ")";
                }
            }
        }
    }

    void DumpType(Node *n, string &ns, SymbolTable &symbols)
    {
        if (n->exptype.t != V_ANY)
        {
            ns += ":";
            ns += symbols.TypeName(n->exptype);
        }
    }
};
