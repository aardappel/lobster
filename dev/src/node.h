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

    TypeRef exptype;

    Line line;

    AST(Line &_line, TType _t) : line(_line), type(_t) {}
};

struct IntConst : AST { int integer_;      IntConst(Line &ln, int i)            : AST(ln, T_INT), integer_(i) {}; };
struct FltConst : AST { double flt_;       FltConst(Line &ln, double f)         : AST(ln, T_FLOAT), flt_(f) {}; };
struct StrConst : AST { char *str_;        StrConst(Line &ln, const string &s)  : AST(ln, T_STR), str_(parserpool->alloc_string_sized(s.c_str())) {}; };
struct IdRef    : AST { Ident *ident_;     IdRef   (Line &ln, Ident *id)        : AST(ln, T_IDENT), ident_(id) {} };
struct StRef    : AST { Struct *st_;       StRef   (Line &ln, Struct *st)       : AST(ln, T_STRUCT), st_(st) {} };
struct FldRef   : AST { SharedField *fld_; FldRef  (Line &ln, SharedField *fld) : AST(ln, T_FIELD), fld_(fld) {} };
struct FunRef   : AST { SubFunction *sf_;  FunRef  (Line &ln, SubFunction *sf)  : AST(ln, T_FUN), sf_(sf) {} };
struct NatRef   : AST { NativeFun *nf_;    NatRef  (Line &ln, NativeFun *nf)    : AST(ln, T_NATIVE), nf_(nf) {} };
struct TypeNode : AST { TypeRef type_;     TypeNode(Line &ln, TType t)          : AST(ln, t) {} };  // T_TYPE, T_NIL

struct Unary : AST
{
    protected:
    Node *a_;
    public:

    Unary(Line &ln, TType t, AST *a) : AST(ln, t), a_((Node *)a)
    {
        assert(TArity(t) >= 1);
    }
}; 

struct Ternary : AST
{
    //protected:
    Node *a_, *b_, *c_;
    public:

    Ternary(Line &ln, TType t, AST *a, AST *b, AST *c) : AST(ln, t), a_((Node *)a), b_((Node *)b), c_((Node *)c)
    {
        assert(TArity(t) == 3);
    }
};

// Inverted subtyping: rather than have most of the compiler deal with the AST base type (which would require a ton
// of virtual methods or casts to access the subtypes), we make Node (the most common occurring subtype, a binary
// tree node) the type we use, and give it ACCs that can read all fields of all other subtypes, protected by an
// assert. This is safe, fast, and memory efficient.

struct Node : Unary
{
    private:
    Node *b_;
    public:

    Node(Line &ln, TType t, AST *a, AST *b) : Unary(ln, t, (Node *)a), b_((Node *)b)
    {
        assert(TArity(t) == 2);
    };

    int integer()       const { assert(type == T_INT);    return ((const IntConst *)this)->integer_; }
    double flt()        const { assert(type == T_FLOAT);  return ((const FltConst *)this)->flt_; }
    char * str()        const { assert(type == T_STR);    return ((const StrConst *)this)->str_; }
    char *&str()              { assert(type == T_STR);    return ((      StrConst *)this)->str_; }
    Ident * ident()     const { assert(type == T_IDENT);  return ((const IdRef *)this)->ident_; }
    Ident *&ident()           { assert(type == T_IDENT);  return ((      IdRef *)this)->ident_; }
    Struct *st()        const { assert(type == T_STRUCT); return ((const StRef *)this)->st_; }
    SharedField *fld()  const { assert(type == T_FIELD);  return ((const FldRef *)this)->fld_; }
    NativeFun * nf()    const { assert(type == T_NATIVE); return ((const NatRef *)this)->nf_; }
    NativeFun *&nf()          { assert(type == T_NATIVE); return ((      NatRef *)this)->nf_; }
    SubFunction * sf()  const { assert(type == T_FUN);    return ((const FunRef *)this)->sf_; }
    SubFunction *&sf()        { assert(type == T_FUN);    return ((      FunRef *)this)->sf_; }
    TypeRef  typenode() const { assert(type == T_TYPE || type == T_NIL); return ((const TypeNode *)this)->type_; }
    TypeRef &typenode()       { assert(type == T_TYPE || type == T_NIL); return ((      TypeNode *)this)->type_; }

    Node * a() const { return TArity(type) > 0 ? a_ : nullptr; }
    Node * b() const { return TArity(type) > 1 ? b_ : nullptr; }
    Node * c() const { return TArity(type) > 2 ? ((Ternary *)this)->c_ : nullptr; }
    Node *&aref()    { assert(TArity(type) > 0); return a_; }
    Node *&bref()    { assert(TArity(type) > 1); return b_; }
    Node *&cref()    { assert(TArity(type) > 2); return ((Ternary *)this)->c_; }

    Node * left()  const { assert(TArity(type) >= 2); return a_; }
    Node *&left()        { assert(TArity(type) >= 2); return a_; }
    Node * right() const { assert(TArity(type) >= 2); return b_; }
    Node *&right()       { assert(TArity(type) >= 2); return b_; }

    Node * child() const { assert(TArity(type) == 1); return a_; }
    Node *&child()       { assert(TArity(type) == 1); return a_; }

    #define ACCESSOR(ENUM, NAME, AB) \
              Node *&NAME()       { assert(type == ENUM); return AB; } \
        const Node * NAME() const { assert(type == ENUM); return AB; }
    #define T0(ENUM, STR, CAT)
    #define T1(ENUM, STR, CAT, ONE)             ACCESSOR(ENUM, ONE, a_)
    #define T2(ENUM, STR, CAT, ONE, TWO)        ACCESSOR(ENUM, ONE, a_) ACCESSOR(ENUM, TWO, b_)
    #define T3(ENUM, STR, CAT, ONE, TWO, THREE) ACCESSOR(ENUM, ONE, a_) ACCESSOR(ENUM, TWO, b_) \
                                                                        ACCESSOR(ENUM, THREE, ((Ternary *)this)->c_)
        TTYPES_LIST
    #undef T0
    #undef T1
    #undef T2
    #undef T3
    #undef ACCESSOR

    ~Node()
    {
        if (type == T_STR)
        {
            parserpool->dealloc_sized(str());
        }
        else
        {
            // This looks odd, since it calls delete on potentially the incorrect type, but the implementation
            // is dealloc_small which knows the correct size.
            if (a()) delete a();
            if (b()) delete b();
            if (c()) delete c();
        }
    }

    Node *Clone()
    {
        auto n = (Node *)parserpool->clone_obj_small_unknown(this);
        if (a()) n->aref() = a()->Clone();
        if (b()) n->bref() = b()->Clone();
        if (c()) n->cref() = c()->Clone();
        if (type == T_STR)
        {
            n->str() = parserpool->alloc_string_sized(str());
        }
        return n;
    }
    
    int Count()
    {
        if (!this) return 0;
        int count = 1;
        if (a()) count += a()->Count();
        if (b()) count += b()->Count();
        if (c()) count += c()->Count();
        return count;
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
        return type == T_DEFAULTVAL ? 0 : (type == T_COCLOSURE ? 1 : sf()->parent->nargs());
    }
};

inline void DumpType(const Node &n, string &ns)
{
    if (n.exptype->t != V_ANY)
    {
        ns += ":";
        ns += TypeName(n.exptype);
    }
}

inline string Dump(const Node &n, int indent)
{
    switch (n.type)
    {
        case T_INT:   return to_string(n.integer());
        case T_FLOAT: return to_string(n.flt());
        case T_STR:   return string("\"") + n.str() + "\"";
        case T_NIL:   return "nil";

        case T_IDENT:  return n.ident()->name;
        case T_STRUCT: return n.st()->name;
        case T_FIELD:  return n.fld()->name;
        case T_NATIVE: return n.nf()->name;
        case T_TYPE:   return TypeName(n.typenode());

        case T_FUN:    return n.sf()
            ? "[fun " + n.sf()->parent->name + "]" /*+ sf()->body->Dump(indent + 2, symbols) */
            : "<>";

        default:
        {
            string s = TName(n.type);

            string as, bs, cs;
            bool ml = false;
            auto indenb = indent - (n.type == T_LIST) * 2;

            if (n.a()) { as = Dump(*n.a(), indent + 2); DumpType(*n.a(), as); if (as[0] == ' ') ml = true; }
            if (n.b()) { bs = Dump(*n.b(), indenb + 2); DumpType(*n.b(), bs); if (bs[0] == ' ') ml = true; }
            if (n.c()) { cs = Dump(*n.c(), indenb + 2); DumpType(*n.c(), cs); if (cs[0] == ' ') ml = true; }

            if (as.size() + bs.size() + cs.size() > 60) ml = true;

            if (ml)
            {
                if (n.a()) { if (as[0] != ' ') as = string(indent + 2, ' ') + as; }
                if (n.b()) { if (bs[0] != ' ') bs = string(indenb + 2, ' ') + bs; }
                if (n.c()) { if (cs[0] != ' ') cs = string(indenb + 2, ' ') + cs; }
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
                if (n.c()) s += "\n" + cs;
                return s;
            }
            else
            {
                if (n.b()) return n.c() ? "(" + s + " " + as + " " + bs + " " + cs + ")"
                                        : "(" + s + " " + as + " " + bs + ")";
                else return "(" + s + " " + as + ")";
            }
        }
    }
}

}  // namespace lobster
