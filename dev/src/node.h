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

namespace lobster {

struct SlabAllocatedSmall {
    #undef new
    void *operator new(size_t size) { return parserpool->alloc_small(size); }
    void *operator new(size_t size, int, const char *, int) {
        return parserpool->alloc_small(size);
    }
    void operator delete(void *p) { parserpool->dealloc_small(p); };
    void operator delete(void *p, int, const char *, int) { parserpool->dealloc_small(p); }
    #ifdef _WIN32
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #endif
    #endif
};

struct Node : SlabAllocatedSmall {
    Line line;
    TType type;
    TypeRef exptype;

    private:
    union {
        struct { Node *a_, *b_, *c_; };
        struct { Ident *ident_; SpecIdent *sid_; };
        int integer_;
        double flt_;
        char *str_;
        Struct *st_;
        SharedField *fld_;
        SubFunction *sf_;
        NativeFun *nf_;
        TypeRef type_;
    };
    public:

    Node(Line &ln, TType t) : line(ln), type(t), a_(nullptr), b_(nullptr), c_(nullptr) {
        assert(TArity(t) == 0);
    }

    Node(Line &ln, TType t, Node *a) : line(ln), type(t), a_(a), b_(nullptr), c_(nullptr) {
        assert(TArity(t) == 1);
    }

    Node(Line &ln, TType t, Node *a, Node *b) : line(ln), type(t), a_(a), b_(b), c_(nullptr) {
        assert(TArity(t) == 2);
    };

    Node(Line &ln, TType t, Node *a, Node *b, Node *c) : line(ln), type(t), a_(a), b_(b), c_(c) {
        assert(TArity(t) == 3);
    }

    Node(Line &ln, TType t, TypeRef tr) : line(ln), type(t), type_(tr) {}  // T_TYPE | T_NIL

    Node(Line &ln, Ident *id, SpecIdent *sid = nullptr)
        : line(ln), type(T_IDENT), ident_(id), sid_(sid) {}

    Node(Line &ln, int i)            : line(ln), type(T_INT), integer_(i) {}
    Node(Line &ln, double f)         : line(ln), type(T_FLOAT), flt_(f) {}
    Node(Line &ln, const string &s)  : line(ln), type(T_STR),
                                       str_(parserpool->alloc_string_sized(s)) {}
    Node(Line &ln, Struct *st)       : line(ln), type(T_STRUCT), st_(st) {}
    Node(Line &ln, SharedField *fld) : line(ln), type(T_FIELD), fld_(fld) {}
    Node(Line &ln, SubFunction *sf)  : line(ln), type(T_FUN), sf_(sf) {}
    Node(Line &ln, NativeFun *nf)    : line(ln), type(T_NATIVE), nf_(nf) {}

    int integer()       const { assert(type == T_INT);    return integer_; }
    double flt()        const { assert(type == T_FLOAT);  return flt_; }
    char * str()        const { assert(type == T_STR);    return str_; }
    char *&str()              { assert(type == T_STR);    return str_; }
    Ident * ident()     const { assert(type == T_IDENT);  return ident_; }
    Ident *&ident()           { assert(type == T_IDENT);  return ident_; }
    SpecIdent * sid()   const { assert(type == T_IDENT);  return sid_; }
    SpecIdent *&sid()         { assert(type == T_IDENT);  return sid_; }
    Struct *st()        const { assert(type == T_STRUCT); return st_; }
    SharedField *fld()  const { assert(type == T_FIELD);  return fld_; }
    NativeFun * nf()    const { assert(type == T_NATIVE); return nf_; }
    NativeFun *&nf()          { assert(type == T_NATIVE); return nf_; }
    SubFunction * sf()  const { assert(type == T_FUN);    return sf_; }
    SubFunction *&sf()        { assert(type == T_FUN);    return sf_; }
    TypeRef  typenode() const { assert(type == T_TYPE || type == T_NIL); return type_; }
    TypeRef &typenode()       { assert(type == T_TYPE || type == T_NIL); return type_; }

    Node * a() const { return TArity(type) > 0 ? a_ : nullptr; }
    Node * b() const { return TArity(type) > 1 ? b_ : nullptr; }
    Node * c() const { return TArity(type) > 2 ? c_ : nullptr; }
    Node *&aref()    { assert(TArity(type) > 0); return a_; }
    Node *&bref()    { assert(TArity(type) > 1); return b_; }
    Node *&cref()    { assert(TArity(type) > 2); return c_; }

    Node * left()  const { assert(TArity(type) >= 2); return a_; }
    Node *&left()        { assert(TArity(type) >= 2); return a_; }
    Node * right() const { assert(TArity(type) >= 2); return b_; }
    Node *&right()       { assert(TArity(type) >= 2); return b_; }

    Node * child() const { assert(TArity(type) == 1); return a_; }
    Node *&child()       { assert(TArity(type) == 1); return a_; }

    #define ACCESSOR(ENUM, NAME, AB) \
              Node *&NAME()       { assert(type == ENUM); return AB; } \
        const Node * NAME() const { assert(type == ENUM); return AB; }
    #define T0(ENUM, STR, CAT, SE)
    #define T1(ENUM, STR, CAT, SE, ONE) \
                ACCESSOR(ENUM, ONE, a_)
    #define T2(ENUM, STR, CAT, SE, ONE, TWO) \
                ACCESSOR(ENUM, ONE, a_) ACCESSOR(ENUM, TWO, b_)
    #define T3(ENUM, STR, CAT, SE, ONE, TWO, THREE) \
                ACCESSOR(ENUM, ONE, a_) ACCESSOR(ENUM, TWO, b_) ACCESSOR(ENUM, THREE, c_)
        TTYPES_LIST
    #undef T0
    #undef T1
    #undef T2
    #undef T3
    #undef ACCESSOR

    ~Node() {
        if (type == T_STR) {
            parserpool->dealloc_sized(str());
        } else {
            if (a()) delete a();
            if (b()) delete b();
            if (c()) delete c();
        }
    }

    Node *Clone() {
        auto n = parserpool->clone_obj_small(this);
        if (a()) n->aref() = a()->Clone();
        if (b()) n->bref() = b()->Clone();
        if (c()) n->cref() = c()->Clone();
        if (type == T_STR) {
            n->str() = (char *)parserpool->clone_sized(str());
        }
        return n;
    }

    // Used to see if a var is worth outputting in a stacktrace.
    bool IsConstInit() {
        switch (type) {
            case T_INT:
            case T_FLOAT:
            case T_STR:
            case T_FUN:
                return true;
            case T_IDENT:
                return ident()->static_constant;
            case T_CONSTRUCTOR: {
                for (Node *n = constructor_args(); n; n = n->tail()) {
                    if (!n->head()->IsConstInit()) return false;
                }
                return true;
            }
            // TODO: support more types of exps?
            default:
                return false;
        }
    }

    // Used in the optimizer to see if this node can be discarded without consequences.
    bool HasSideEffects() {
        return TSideEffect(type) ||
               (a() && a()->HasSideEffects()) ||
               (b() && b()->HasSideEffects()) ||
               (c() && c()->HasSideEffects());
    }
};

inline int CountNodes(const Node *n) {
    if (!n) return 0;
    int count = 1;
    if (n->a()) count += CountNodes(n->a());
    if (n->b()) count += CountNodes(n->b());
    if (n->c()) count += CountNodes(n->c());
    return count;
}

inline void DumpType(const Node &n, string &ns) {
    if (n.exptype->t != V_ANY) {
        ns += ":";
        ns += TypeName(n.exptype);
    }
}

inline string Dump(const Node &n, int indent) {
    switch (n.type) {
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
        default: {
            string s = TName(n.type);
            string as, bs, cs;
            bool ml = false;
            auto indenb = indent - (n.type == T_LIST) * 2;
            if (n.a()) {
                as = Dump(*n.a(), indent + 2);
                DumpType(*n.a(), as);
                if (as[0] == ' ') ml = true;
            }
            if (n.b()) {
                bs = Dump(*n.b(), indenb + 2);
                DumpType(*n.b(), bs);
                if (bs[0] == ' ') ml = true;
            }
            if (n.c()) {
                cs = Dump(*n.c(), indenb + 2);
                DumpType(*n.c(), cs);
                if (cs[0] == ' ') ml = true;
            }
            if (as.size() + bs.size() + cs.size() > 60) ml = true;
            if (ml) {
                if (n.a()) { if (as[0] != ' ') as = string(indent + 2, ' ') + as; }
                if (n.b()) { if (bs[0] != ' ') bs = string(indenb + 2, ' ') + bs; }
                if (n.c()) { if (cs[0] != ' ') cs = string(indenb + 2, ' ') + cs; }
                if (n.type == T_LIST) {
                    s = "";
                } else {
                    s = string(indent, ' ') + s;
                    if (n.a()) s += "\n";
                }
                if (n.a()) s += as;
                if (n.b()) s += "\n" + bs;
                if (n.c()) s += "\n" + cs;
                return s;
            } else {
                if (n.b()) return n.c() ? "(" + s + " " + as + " " + bs + " " + cs + ")"
                                        : "(" + s + " " + as + " " + bs + ")";
                else return "(" + s + " " + as + ")";
            }
        }
    }
}

}  // namespace lobster
