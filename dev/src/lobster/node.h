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

#ifndef LOBSTER_NODE
#define LOBSTER_NODE

namespace lobster {

typedef const function<void (Node *)> &IterateFun;

struct TypeChecker;
struct Optimizer;
struct CodeGen;

struct Node {
    Line line { 0, 0 };
    TypeRef exptype;
    Lifetime lt = LT_UNDEF;
    virtual ~Node() {};
    virtual size_t Arity() const { return 0; }
    virtual Node **Children() { return nullptr; }
    virtual Node *Clone() = 0;
    virtual bool IsConstInit() const { return false; }
    virtual bool ReturnsOutOf() const { return false; }
    virtual string_view Name() const = 0;
    virtual void Dump(string &sd) const { sd += Name(); }
    void Iterate(IterateFun f) {
        f(this);
        auto ch = Children();
        if (ch) for (size_t i = 0; i < Arity(); i++) ch[i]->Iterate(f);
    }
    // Used in the optimizer to see if this node can be discarded without consequences.
    virtual bool SideEffect() const = 0;  // Just this node.
    bool HasSideEffects() {  // Transitively.
        bool se = false;
        Iterate([&](Node *n) { se = se || n->SideEffect(); });
        return se;
    }
    size_t Count() {
        size_t count = 0;
        Iterate([&](Node *) { count++; });
        return count;
    }
    // Used by type-checker to and optimizer.
    // If it returns true, sets val to a value that gives the correct True().
    // Also sets correct scalar values.
    virtual bool ConstVal(TypeChecker &, Value &) const { return false; }
    virtual Node *TypeCheck(TypeChecker &tc, size_t reqret) = 0;
    virtual Node *Optimize(Optimizer &opt);
    virtual void Generate(CodeGen &cg, size_t retval) const = 0;
  protected:
    Node(const Line &ln) : line(ln) {}
    Node() = default;
};

struct TypeLT {
    TypeRef type;
    Lifetime lt;

    TypeLT(TypeRef type, Lifetime lt)
        : type(type), lt(lt) {}

    TypeLT(const SpecIdent &sid)
        : type(sid.type), lt(sid.lt) {}

    TypeLT(const Node &n, size_t i)
        : type(n.exptype->Get(i)), lt(n.exptype->GetLifetime(i, n.lt)) {}

    TypeLT(const SubFunction &sf, size_t i)
        : type(sf.returntype->Get(i)), lt(sf.ltret) {}
};


template<typename T> T *DoClone(T *dest, T *src) {
    *dest = *src;  // Copy contructor copies all values & non-owned.
    for (size_t i = 0; i < src->Arity(); i++) dest->Children()[i] = src->Children()[i]->Clone();
    return dest;
}

#define SHARED_SIGNATURE_NO_TT(NAME, STR, SE) \
  protected: \
    NAME() {}; \
  public: \
    string_view Name() const { return STR; } \
    bool SideEffect() const { return SE; } \
    void Generate(CodeGen &cg, size_t retval) const; \
    Node *Clone() { return DoClone<NAME>(new NAME(), this); }
#define SHARED_SIGNATURE(NAME, STR, SE) \
    Node *TypeCheck(TypeChecker &tc, size_t reqret); \
    SHARED_SIGNATURE_NO_TT(NAME, STR, SE)

#define ZERO_NODE(NAME, STR, SE, METHODS) \
struct NAME : Node { \
    NAME(const Line &ln) : Node(ln) {} \
    SHARED_SIGNATURE(NAME, STR, SE) \
    METHODS \
};

#define UNARY_NODE(NAME, STR, SE, A, METHODS) \
struct NAME : Node { \
    Node *A; \
    NAME(const Line &ln, Node *_a) : Node(ln), A(_a) {} \
    ~NAME() { delete A; } \
    size_t Arity() const { return 1; } \
    Node **Children() { return &A; } \
    SHARED_SIGNATURE(NAME, STR, SE) \
    METHODS \
};

#define UNOP_NODE(NAME, STR, SE, METHODS) \
struct NAME : Unary { \
    NAME(const Line &ln, Node *_a) : Unary(ln, _a) {} \
    SHARED_SIGNATURE(NAME, STR, SE) \
    METHODS \
};
#define COER_NODE(NAME, STR) \
struct NAME : Coercion { \
    NAME(const Line &ln, Node *_a) : Coercion(ln, _a) {} \
    SHARED_SIGNATURE_NO_TT(NAME, STR, false) \
};

#define BINARY_NODE_T(NAME, STR, SE, AT, A, BT, B, METHODS) \
struct NAME : Node { \
    AT *A; BT *B; \
    NAME(const Line &ln, AT *_a, BT *_b) : Node(ln), A(_a), B(_b) {}; \
    ~NAME() { delete A; delete B; } \
    size_t Arity() const { return 2; } \
    Node **Children() { return (Node **)&A; } \
    SHARED_SIGNATURE(NAME, STR, SE) \
    METHODS \
};
#define BINARY_NODE(NAME, STR, SE, A, B, METHODS) \
    BINARY_NODE_T(NAME, STR, SE, Node, A, Node, B, METHODS)

#define BINOP_NODE(NAME, STR, SE, METHODS) \
struct NAME : BinOp { \
    NAME(const Line &ln, Node *_a, Node *_b) : BinOp(ln, _a, _b) {}; \
    SHARED_SIGNATURE(NAME, STR, SE) \
    METHODS \
};

#define TERNARY_NODE_T(NAME, STR, SE, AT, A, BT, B, CT, C, METHODS) \
struct NAME : Node { \
    AT *A; BT *B; CT *C; \
    NAME(const Line &ln, AT *_a, BT *_b, CT *_c) : Node(ln), A(_a), B(_b), C(_c) {} \
    ~NAME() { delete A; delete B; delete C; } \
    size_t Arity() const { return 3; } \
    Node **Children() { return &A; } \
    SHARED_SIGNATURE(NAME, STR, SE) \
    METHODS \
};

#define TERNARY_NODE(NAME, STR, SE, A, B, C, METHODS) \
    TERNARY_NODE_T(NAME, STR, SE, Node, A, Node, B, Node, C, METHODS)

#define NARY_NODE(NAME, STR, SE, METHODS) \
struct NAME : Node { \
    vector<Node *> children; \
    NAME(const Line &ln) : Node(ln) {}; \
    ~NAME() { for (auto n : children) delete n; } \
    size_t Arity() const { return children.size(); } \
    Node **Children() { return children.data(); } \
    NAME *Add(Node *a) { children.push_back(a); return this; }; \
    SHARED_SIGNATURE(NAME, STR, SE) \
    METHODS \
};

struct TypeAnnotation : Node {
    UnresolvedTypeRef giventype;
    TypeAnnotation(const Line &ln, UnresolvedTypeRef tr) : Node(ln), giventype(tr) {}
    void Dump(string &sd) const { sd += TypeName(giventype.utr); }
    SHARED_SIGNATURE(TypeAnnotation, "type", false)
};

#define RETURNSMETHOD bool ReturnsOutOf() const;
#define CONSTVALMETHOD bool ConstVal(TypeChecker &tc, Value &val) const;
#define OPTMETHOD Node *Optimize(Optimizer &opt);

// generic node types
NARY_NODE(List, "list", false, )
UNARY_NODE(Unary, "unary", false, child, )
BINARY_NODE(BinOp, "binop", false, left, right, )
UNOP_NODE(Coercion, "coercion", false, )

BINOP_NODE(Plus, TName(T_PLUS), false, )
BINOP_NODE(Minus, TName(T_MINUS), false, )
BINOP_NODE(Multiply, TName(T_MULT), false, )
BINOP_NODE(Divide, TName(T_DIV), false, )
BINOP_NODE(Mod, TName(T_MOD), false, )
BINOP_NODE(And, TName(T_AND), false, CONSTVALMETHOD)
BINOP_NODE(Or, TName(T_OR), false, CONSTVALMETHOD)
UNARY_NODE(Not, TName(T_NOT), false, child, CONSTVALMETHOD)
UNOP_NODE(PreIncr, TName(T_INCR), true, )
UNOP_NODE(PreDecr, TName(T_DECR), true, )
BINOP_NODE(Equal, TName(T_EQ), false, )
BINOP_NODE(NotEqual, TName(T_NEQ), false, )
BINOP_NODE(LessThan, TName(T_LT), false, )
BINOP_NODE(GreaterThan, TName(T_GT), false, )
BINOP_NODE(LessThanEq, TName(T_LTEQ), false, )
BINOP_NODE(GreaterThanEq, TName(T_GTEQ), false, )
BINOP_NODE(BitAnd, TName(T_BITAND), false, )
BINOP_NODE(BitOr, TName(T_BITOR), false, )
BINOP_NODE(Xor, TName(T_XOR), false, )
UNARY_NODE(Negate, TName(T_NEG), false, child, )
BINOP_NODE(ShiftLeft, TName(T_ASL), false, )
BINOP_NODE(ShiftRight, TName(T_ASR), false, )
BINOP_NODE(Assign, TName(T_ASSIGN), true, )
BINOP_NODE(LogAssign, TName(T_LOGASSIGN), true, )
BINOP_NODE(PlusEq, TName(T_PLUSEQ), true, )
BINOP_NODE(MinusEq, TName(T_MINUSEQ), true, )
BINOP_NODE(MultiplyEq, TName(T_MULTEQ), true, )
BINOP_NODE(DivideEq, TName(T_DIVEQ), true, )
BINOP_NODE(ModEq, TName(T_MODEQ), true, )
BINOP_NODE(AndEq, TName(T_ANDEQ), true, )
BINOP_NODE(OrEq, TName(T_OREQ), true, )
BINOP_NODE(XorEq, TName(T_XOREQ), true, )
BINOP_NODE(ShiftLeftEq, TName(T_ASLEQ), true, )
BINOP_NODE(ShiftRightEq, TName(T_ASREQ), true, )
BINARY_NODE(CoDot, TName(T_CODOT), false, coroutine, variable, )
ZERO_NODE(DefaultVal, "default value", false, )
UNARY_NODE(TypeOf, TName(T_TYPEOF), false, child, )
UNARY_NODE(CoRoutine, TName(T_COROUTINE), true, call, )

ZERO_NODE(CoClosure, "coroutine yield", false, )
BINARY_NODE(Seq, "statements", false, head, tail, )
BINARY_NODE(Indexing, "indexing operation", false, object, index, )
UNOP_NODE(PostIncr, TName(T_INCR), true, )
UNOP_NODE(PostDecr, TName(T_DECR), true, )
UNARY_NODE(UnaryMinus, TName(T_MINUS), false, child, )
COER_NODE(ToFloat, "tofloat")
COER_NODE(ToString, "tostring")
COER_NODE(ToBool, "tobool")
COER_NODE(ToInt, "toint")
NARY_NODE(Block, "block", false, RETURNSMETHOD)
BINARY_NODE_T(IfThen, "if", false, Node, condition, Block, truepart, OPTMETHOD)
TERNARY_NODE_T(IfElse, "if", false, Node, condition, Block, truepart, Block, falsepart, OPTMETHOD RETURNSMETHOD)
BINARY_NODE_T(While, "while", false, Node, condition, Block, body, )
BINARY_NODE_T(For, "for", false, Node, iter, Block, body, )
ZERO_NODE(ForLoopElem, "for loop element", false, )
ZERO_NODE(ForLoopCounter, "for loop counter", false, )
BINARY_NODE_T(Switch, "switch", false, Node, value, List, cases, RETURNSMETHOD)
BINARY_NODE_T(Case, "case", false, List, pattern, Node, body, )
BINARY_NODE(Range, "range", false, start, end, )

struct Nil : Node {
    UnresolvedTypeRef giventype;
    Nil(const Line &ln, UnresolvedTypeRef tr) : Node(ln), giventype(tr) {}
    bool ConstVal(TypeChecker &, Value &val) const {
        val = Value();
        return true;
    }
    SHARED_SIGNATURE(Nil, TName(T_NIL), false)
};

struct IdentRef : Node {
    SpecIdent *sid;
    IdentRef(const Line &ln, SpecIdent *_sid)
        : Node(ln), sid(_sid) {}
    bool IsConstInit() const { return sid->id->static_constant; }
    void Dump(string &sd) const { sd += sid->id->name; }
    SHARED_SIGNATURE(IdentRef, TName(T_IDENT), false)
};

struct IntConstant : Node {
    int64_t integer;
    EnumVal *from;
    IntConstant(const Line &ln, int64_t i) : Node(ln), integer(i), from(nullptr) {}
    bool IsConstInit() const { return true; }
    void Dump(string &sd) const { if (from) sd += from->name; else append(sd, integer); }
    bool ConstVal(TypeChecker &, Value &val) const {
        val = Value(integer);  // FIXME: this clips.
        return true;
    }
    SHARED_SIGNATURE(IntConstant, TName(T_INT), false)
};

struct FloatConstant : Node {
    double flt;
    FloatConstant(const Line &ln, double f) : Node(ln), flt(f) {}
    bool IsConstInit() const { return true; }
    void Dump(string &sd) const { sd += to_string_float(flt); }
    bool ConstVal(TypeChecker &, Value &val) const {
        val = Value(flt);
        return true;
    }
    SHARED_SIGNATURE(FloatConstant, TName(T_FLOAT), false)
};

struct StringConstant : Node {
    string str;
    StringConstant(const Line &ln, string_view s) : Node(ln), str(s) {}
    bool IsConstInit() const { return true; }
    void Dump(string &sd) const { EscapeAndQuote(str, sd); }
    SHARED_SIGNATURE(StringConstant, TName(T_STR), false)
};

struct EnumRef : Node {
    Enum *e;
    EnumRef(const Line &ln, Enum *_e) : Node(ln), e(_e) {}
    void Dump(string &sd) const { append(sd, "enum", e->name); }
    SHARED_SIGNATURE(EnumRef, TName(T_ENUM), false)
};

struct UDTRef : Node {
    UDT *udt;
    UDTRef(const Line &ln, UDT *_udt) : Node(ln), udt(_udt) {}
    void Dump(string &sd) const { append(sd, udt->is_struct ? "struct " : "class ", udt->name); }
    SHARED_SIGNATURE(UDTRef, TName(T_CLASS), false)
};

struct FunRef : Node {
    SubFunction *sf;
    FunRef(const Line &ln, SubFunction *_sf) : Node(ln), sf(_sf) {}
    bool IsConstInit() const { return true; }
    void Dump(string &sd) const {
        append(sd, "(def ", sf->parent->name, ")");
    }
    SHARED_SIGNATURE(FunRef, TName(T_FUN), false)
};

// This is either a Dot, Call, or NativeCall, to be specialized by the typechecker
struct GenericCall : List {
    string_view name;
    SubFunction *sf;  // Need to store this, since only parser tracks scopes.
    bool dotnoparens;
    vector<UnresolvedTypeRef> specializers;
    GenericCall(const Line &ln, string_view name, SubFunction *sf, bool dotnoparens,
                vector<UnresolvedTypeRef> *spec)
        : List(ln), name(name), sf(sf), dotnoparens(dotnoparens) {
        if (spec) specializers = *spec;
    };
    SHARED_SIGNATURE(GenericCall, "generic call", true)
};

struct Constructor : List {
    UnresolvedTypeRef giventype;
    Constructor(const Line &ln, UnresolvedTypeRef _type) : List(ln), giventype(_type) {};
    bool IsConstInit() const {
        for (auto n : children) {
            if (!n->IsConstInit()) return false;
        }
        return true;
    }
    SHARED_SIGNATURE(Constructor, "constructor", false)
};

struct Call : GenericCall {
    int vtable_idx = -1;
    explicit Call(GenericCall &gc)
        : GenericCall(gc.line, gc.name, gc.sf, gc.dotnoparens, &gc.specializers) {};
    Call(Line &ln, SubFunction *sf) : GenericCall(ln, sf->parent->name, sf, false, nullptr) {};
    void Dump(string &sd) const { sd += sf->parent->name; }
    void TypeCheckSpecialized(TypeChecker &tc, size_t reqret);
    SHARED_SIGNATURE_NO_TT(Call, "call", true)
    OPTMETHOD
    RETURNSMETHOD
};

struct DynCall : List {
    SubFunction *sf;
    SpecIdent *sid;
    DynCall(const Line &ln, SubFunction *_sf, SpecIdent *_sid)
        : List(ln), sf(_sf), sid(_sid) {};
    void Dump(string &sd) const { sd += sid->id->name; }
    SHARED_SIGNATURE(DynCall, "dynamic call", true)
    OPTMETHOD
};

struct NativeCall : GenericCall {
    NativeFun *nf;
    TypeRef nattype = nullptr;
    Lifetime natlt = LT_UNDEF;
    NativeCall(NativeFun *_nf, GenericCall &gc)
        : GenericCall(gc.line, gc.name, gc.sf, gc.dotnoparens, &gc.specializers), nf(_nf) {};
    void Dump(string &sd) const { sd += nf->name; }
    void TypeCheckSpecialized(TypeChecker &tc, size_t reqret);
    SHARED_SIGNATURE_NO_TT(NativeCall, "native call", true)
    RETURNSMETHOD
};

struct Return : Unary {
    SubFunction *sf;
    bool make_void;
    Return(const Line &ln, Node *_a, SubFunction *sf, bool make_void)
        : Unary(ln, _a), sf(sf), make_void(make_void) {}
    SHARED_SIGNATURE(Return, TName(T_RETURN), true)
    RETURNSMETHOD
};

struct MultipleReturn : List {
    MultipleReturn(const Line &ln) : List(ln) {};
    SHARED_SIGNATURE(MultipleReturn, "multiple return", false)
};

struct AssignList : List {
    AssignList(const Line &ln, Node *a) : List(ln) {
        children.push_back(a);
    }
    SHARED_SIGNATURE(AssignList, "assign list", true)
};

struct Define : Unary {
    vector<pair<SpecIdent *, UnresolvedTypeRef>> sids;
    Define(const Line &ln, Node *_a) : Unary(ln, _a) {}
    void Dump(string &sd) const {
        for (auto p : sids) append(sd, p.first->id->name, " ");
        sd += Name();
    }
    SHARED_SIGNATURE(Define, "var", true)
};

struct Dot : GenericCall {
    SharedField *fld;  // FIXME
    Dot(SharedField *_fld, GenericCall &gc)
        : GenericCall(gc.line, gc.name, gc.sf, gc.dotnoparens, &gc.specializers), fld(_fld) {}
    void Dump(string &sd) const { append(sd, Name(), fld->name); }
    void TypeCheckSpecialized(TypeChecker &tc, size_t reqret);
    SHARED_SIGNATURE_NO_TT(Dot, TName(T_DOT), false)
};

struct IsType : Unary {
    UnresolvedTypeRef giventype;
    TypeRef resolvedtype;
    IsType(const Line &ln, Node *_a) : Unary(ln, _a) {}
    void Dump(string &sd) const { append(sd, Name(), ":", TypeName(giventype.utr)); }
    CONSTVALMETHOD
    SHARED_SIGNATURE(IsType, TName(T_IS), false)
    OPTMETHOD
};

struct EnumCoercion : Unary {
    Enum *e;
    EnumCoercion(const Line &ln, Node *_a, Enum *e) : Unary(ln, _a), e(e) {}
    void Dump(string &sd) const { sd += e->name; }
    CONSTVALMETHOD
    SHARED_SIGNATURE(EnumCoercion, e->name, false)
};

struct ToLifetime : Coercion {
    uint64_t incref, decref;
    ToLifetime(const Line &ln, Node *_a, uint64_t incref, uint64_t decref)
        : Coercion(ln, _a), incref(incref), decref(decref) {}
    void Dump(string &sd) const { append(sd, Name(), "<", incref, "|", decref, ">"); }
    SHARED_SIGNATURE_NO_TT(ToLifetime, "lifetime change", true)
};

inline string DumpNode(Node &n, int indent, bool single_line) {
    string sd;
    n.Dump(sd);
    auto arity = n.Arity();
    if (!arity) return sd;
    bool ml = false;
    auto ch = n.Children();
    vector<string> sv;
    size_t total = 0;
    for (size_t i = 0; i < arity; i++) {
        auto a = DumpNode(*ch[i], indent + 2, single_line);
        a += ":";
        a += TypeName(ch[i]->exptype);
        if (a[0] == ' ') ml = true;
        total += a.length();
        sv.push_back(a);
    }
    if (total > 60) ml = true;
    if (ml && !single_line) {
        sd.insert(0, string(indent, ' ') + "(");
        sd += "\n";
        for (size_t i = 0; i < arity; i++) {
            if (i) sd += "\n";
            if (sv[i][0] != ' ') sd += string(indent + 2, ' ');
            sd += sv[i];
        }
        sd += ")";
    } else {
        sd.insert(0, "(");
        for (size_t i = 0; i < arity; i++) append(sd, " ", sv[i]);
        sd += ")";
    }
    return sd;
}

}  // namespace lobster

#endif  // LOBSTER_NODE
