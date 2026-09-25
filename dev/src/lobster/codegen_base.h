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

enum {
    CODEGEN_SPECIAL_FUNCTION_ID_START = 10000000,
    CODEGEN_SPECIAL_FUNCTION_ID_DUMMY = CODEGEN_SPECIAL_FUNCTION_ID_START + 1,
    CODEGEN_SPECIAL_FUNCTION_ID_ENTRY = CODEGEN_SPECIAL_FUNCTION_ID_START + 2,
};

// The binary operators, in the order the helpers specialized for them come, so that a type
// prefix plus the name of one of these is the name of a helper, see MathOpName.
enum MathOp {
    MOP_ADD, MOP_SUB, MOP_MUL, MOP_DIV, MOP_MOD, MOP_LT, MOP_GT, MOP_LE, MOP_GE, MOP_EQ, MOP_NE
};

// The bitwise operators, in the order of the LvalOp modifiers for them.
enum BitOp { BIT_AND, BIT_OR, BIT_XOR, BIT_ASL, BIT_ASR };

// The ops that modify the value an lvalue points at. Which one an assignment uses depends on
// both the operator and the type it works on, see GenAssign, and most of them we emit inline
// rather than call. The name of each is the name of its op.
enum LvalOp {
    LV_DUP, LV_DUPV,
    LV_WRITE, LV_WRITEREF, LV_WRITEV, LV_WRITEREFV,
    // The arithmetic ones come in MathOp order per family, which is what lets GenAssign
    // retarget an operator at the type it is used on by adding a family offset.
    LV_IADD, LV_ISUB, LV_IMUL, LV_IDIV, LV_IMOD,
    LV_BINAND, LV_BINOR, LV_XOR, LV_ASL, LV_ASR,
    LV_FADD, LV_FSUB, LV_FMUL, LV_FDIV, LV_FMOD,
    LV_IVVADD, LV_IVVSUB, LV_IVVMUL, LV_IVVDIV, LV_IVVMOD,
    LV_FVVADD, LV_FVVSUB, LV_FVVMUL, LV_FVVDIV, LV_FVVMOD,
    LV_IVSADD, LV_IVSSUB, LV_IVSMUL, LV_IVSDIV, LV_IVSMOD,
    LV_FVSADD, LV_FVSSUB, LV_FVSMUL, LV_FVSDIV, LV_FVSMOD,
    LV_SADD,
    LV_IPP, LV_IMM, LV_FPP, LV_FMM,
};

// A return either ends the function it is in, returns past it to a specific function further up,
// or is the code at a callsite that picks up such a return passing thru.
enum ReturnKind { RET_LOCAL, RET_NONLOCAL, RET_ANY };

// Shared state, the recursion over the AST, and the emitter helpers used across topics, each kept
// with the rest of its group (the labels and jumps, say) even where only one topic uses part of
// it. Topics share this virtual base. The small virtual interface below lets a topic emit what
// another one owns (a pop, a reference count, a type table entry) without depending on a sibling
// class or moving that topic's implementation into this file.
struct CodeGenBase {
  private:
    // Only the AST dispatch needs the assembled code generator. Its type may remain incomplete
    // here: Node::Generate supplies the per-node dispatch.
    CodeGen &codegen;

  protected:
    CodeGen &ASTCodeGen() { return codegen; }

  public:
    struct SpecIdentInfo {
        int ididx;
        int typeidx;
        bool used_as_freevar;
        int idx;
    };
    vector<SpecIdentInfo> sids;
    Parser &parser;
    SymbolTable &st;
    vector<type_elem_t> type_table;
    // Offsets belong to this generated type table, not to the semantic UDTs. Reserve both
    // arrays up front so recursive type emission can keep a reference to an entry.
    vector<type_elem_t> udt_type_offsets, udt_nil_type_offsets;
    vector<TypeLT> rettypes, temptypestack;
    // The loop and the stack depth and target its break/continue sites need, kept together
    // so entering or leaving a loop changes a single stack.
    struct LoopState {
        const Node *node;
        size_t temp_level;
        int continue_label;
    };
    vector<LoopState> loops;
    vector<int> breaks;
    vector<const Node *> node_context;
    int runtime_checks;
    // See --rcstats: what the inc/dec being emitted is for (rc_tag groups sites, rc_extra
    // details one), and whether to leave the one being emitted uncounted because it is
    // counted elsewhere.
    bool rcstats;
    string rc_tag, rc_extra;
    bool rc_suppress = false;
    // How many stack slots the code emitted so far leaves in use.
    size_t tstack_size = 0;
    bool cpp = false;
    // Set when the C output is going to be fed to MIR rather than libtcc, for any places where
    // the two need different code.
    bool mir = false;

    // C/C++ codegen related.
    string cb;
    // One member of the struct for an object type, which is one field, or one slot of a field
    // when that field is a struct of more than one type.
    struct UDTMember {
        string name;
        string ctype;
        int slot;
        int count;  // More than one for a field that is an array, see Members.
    };
    // The object types whose fields the code names, which get a struct of their own at the
    // same spot, see UDTName, with the members it has, see Members.
    map<int, pair<const UDT *, vector<UDTMember>>> udts_used;
    // Whether the function calls a builtin that gets the profiler hooks, see EmitNativeCall.
    bool f_uses_pctx = false;
    int regso = 0;
    // The C type a value is kept in, which its static type decides: the scalars as themselves,
    // every reference as a pointer to its header, and a function value as a pointer to it. Only
    // memory that any type can land in, the elements of a vector and the globals, holds Values.
    // The references split by what the static type says they are, so the generated code can
    // use them as such; VK_REF is what is left: a resource, a nil of no type, or any.
    enum VKind { VK_INT, VK_FLOAT, VK_REF, VK_FUN, VK_STRING, VK_VECTOR, VK_OBJECT, VK_COUNT };
    // The runtime type of each slot of a run of values, which says both the kind of variable it
    // is kept in and the tag it carries when it is written to memory, see RtTypeOf.
    typedef vector<RTType> Types;

    // Where a value lives: a variable of its kind, or a Value in memory, which is read thru the
    // field for the kind and written along with the tag its static type says it carries.
    struct Place {
        string s;
        RTType rtt;
        // Whether it is read and written as the type its value is rather than as a Value, and
        // whether it is a variable, which is what lets it be named more than once and be
        // remembered in place of a write, see Defer. A field of an object is the first without
        // being the second.
        bool typed;
        bool var = false;
        int slot = -1;
        VKind k() const { return Kind(rtt); }
    };
    // A value pushed onto the stack, or computed from what is on it, is not written to its
    // slot: the slot remembers the expression instead, and whatever reads the slot uses that
    // in its place, see Defer, which is how the operators of one expression in the program
    // end up as one expression in C. The write is emitted after all where the expression may
    // stop holding: before a write to a variable it reads, and at control flow, see Flush.
    struct Pending {
        Place slot;
        string expr;
        // The variables `expr` reads, which for a constant are none.
        vector<string> vars;
        // The precedence of the operator on top of `expr`, 0 for an atom, see Operand.
        int prec;
    };
    vector<Pending> pending;

    // An expression an emitter builds out of what is on the stack: what it reads, whether it
    // could be deferred (no loads or calls in it), and its precedence.
    struct Expr {
        string text;
        vector<string> vars;
        bool pure;
        int prec;
    };
    // Which kinds of value each stack slot holds, a bit per VKind, which decides the variables
    // the function declares for it, only known at the end of its codegen.
    vector<int> f_slot_kinds;
    // What each temporary that keeps a reference alive holds, see EmitKeep.
    Types f_keeps;
    // What the lvalues of the function need declared, which is only known at the end of its
    // codegen, see DefineFunction: the lv, lo and lvec/lidx locals, and how wide the arrays
    // struct locals are staged in have to be, per numeric type one can be of, see f_writeback.
    bool f_uses_lval = false;
    bool f_uses_lobj = false;
    bool f_uses_lelem = false;
    int f_stage_max[2] = { 0, 0 };
    vector<int> funstarttables;
    // The C name of each function, by its SubFunction index, see FunName.
    vector<string> fun_names;
    vector<int> var_to_local;
    // The C variable each local (by var_to_local index) lives in, including its type, and
    // every name the function has handed out, to keep them apart.
    vector<Place> local_places;
    // Whether the code of the function names each of those, which is what decides if it is
    // declared, see Local and DefineFunction.
    vector<bool> local_used;
    set<string> f_names_used;
    // The static type of each slot of every variable, by its index in sids.
    vector<TypeRef> var_types;
    int nlabel = 0;

    CodeGenBase(CodeGen &codegen, Parser &_p, SymbolTable &_st, const CompileOptions &opts)
        : codegen(codegen), parser(_p), st(_st), runtime_checks(opts.runtime_checks),
          rcstats(opts.rcstats), cpp(!opts.jit_mode), mir(opts.jit_mode && opts.jit_options.mir) {}

    virtual ~CodeGenBase() = default;

    // Implemented by CodeGenRefCount.
    virtual void GenPop(TypeLT typelt) = 0;

    // Implemented by CodeGenRefCount.
    virtual void GenDecRef(string &sd, const Place &p) = 0;

    // Implemented by CodeGenRefCount.
    virtual void GenIncRef(string &sd, const Place &p) = 0;

    // Implemented by CodeGenRefCount.
    virtual string RcStatCall(bool inc, string_view extra = {}) = 0;

    // Implemented by CodeGenTypeTable.
    virtual type_elem_t GetTypeTableOffset(TypeRef type,
                                           type_elem_t non_nil_version = (type_elem_t)0) = 0;

    // Implemented by CodeGenOps.
    virtual void GenMathOp(TypeRef ltype, TypeRef rtype, TypeRef ptype, MathOp op,
                           bool divisor_safe = false) = 0;

    // Implemented by CodeGenPrologue.
    virtual int ConstantString(string_view s) = 0;

    // Implemented by CodeGenLval.
    virtual void GenPushVar(size_t retval, TypeRef type, int offset, bool used_as_freevar) = 0;

    void Gen(const Node *n, size_t retval) {
        // Generate() below generate no retvals if retval==0, otherwise they generate however many
        // they can irrespective of retval, optionally record that in rettypes for the more complex
        // cases. Then at the end of this function the two get matched up.
        auto tempstartsize = temptypestack.size();
        node_context.push_back(n);
        n->Generate(ASTCodeGen(), retval);
        node_context.pop_back();

        assert(n->exptype->t != V_UNDEFINED);

        assert(tempstartsize == temptypestack.size());
        (void)tempstartsize;
        // If 0, the above code already made sure to not generate value(s).
        if (retval) {
            // default case, no rettypes specified.
            if (rettypes.empty()) {
                for (size_t i = 0; i < n->exptype->NumValues(); i++)
                    rettypes.push_back(TypeLT { *n, i });
            }
            // if the caller doesn't want all return values, just pop em
            if (rettypes.size() > retval) {
                while (rettypes.size() > retval) {
                    GenPop(rettypes.back());
                    rettypes.pop_back();
                }
            }
            assert(rettypes.size() == retval);
            // Copy return types on temp stack.
            while (rettypes.size()) {
                temptypestack.push_back(rettypes.front());
                rettypes.erase(rettypes.begin());
            }
        }
        assert(rettypes.empty());
    }

    // The runtime type of a value of this type, for a nilable one that of what it holds. The
    // types that only exist at compile time have no variable to live in, but a nil of one is
    // still pushed here and there.
    static RTType RtTypeOf(TypeRef type) {
        switch (type->t) {
            case V_INT:
            case V_TYPEID: return RTT_INT;
            case V_FLOAT: return RTT_FLOAT;
            case V_FUNCTION: return RTT_FUNCTION;
            case V_STRING: return RTT_STRING;
            case V_VECTOR: return RTT_VECTOR;
            case V_CLASS: return RTT_CLASS;
            case V_RESOURCE: return RTT_RESOURCE;
            case V_STRUCT_R: return RTT_STRUCT_R;
            case V_STRUCT_S: return RTT_STRUCT_S;
            case V_NIL: return type->sub ? RtTypeOf(type->sub) : RTT_NIL;
            default: return RTT_INVALID;
        }
    }

    static VKind Kind(RTType t) {
        switch (t) {
            case RTT_INT: return VK_INT;
            case RTT_FLOAT: return VK_FLOAT;
            case RTT_FUNCTION: return VK_FUN;
            case RTT_STRING: return VK_STRING;
            case RTT_VECTOR: return VK_VECTOR;
            case RTT_CLASS: return VK_OBJECT;
            default: return VK_REF;
        }
    }

    static bool IsRefKind(VKind k) { return k != VK_INT && k != VK_FLOAT && k != VK_FUN; }

    static VKind KindOf(TypeRef type) { return Kind(RtTypeOf(type)); }

    // The runtime type of a kind, for the places whose exact type nothing needs: a reference
    // that is only ever read, or passed on.
    static RTType Rtt(VKind k) {
        static const RTType types[] = { RTT_INT, RTT_FLOAT, RTT_INVALID, RTT_FUNCTION,
                                        RTT_STRING, RTT_VECTOR, RTT_CLASS };
        return types[k];
    }

    // Slot i of a value of this type, which for a struct is one of its fields, see SlotTypeOf.
    static TypeRef SlotType(TypeRef type, int i) {
        return IsStruct(type->t) ? SlotTypeOf(*type->udt, i) : type;
    }

    // The runtime type of each slot of a value of this type, or of that many of them.
    static void AddTypes(Types &ts, TypeRef type) {
        for (int i = 0; i < ValWidth(type); i++) ts.push_back(RtTypeOf(SlotType(type, i)));
    }
    static Types TypesOf(TypeRef type, size_t nvals) {
        Types ts;
        for (size_t v = 0; v < nvals; v++) AddTypes(ts, type->Get(v));
        return ts;
    }

    string CType(VKind k) {
        static const char *names[] = { "long long", "double", "RefObj *", "fun_base_t",
                                       "LString *", "LVector *", "LObject *" };
        return k == VK_INT && cpp ? "iint" : names[k];
    }

    // The field of a Value in memory a kind is read and written thru, and the accessor the
    // C++ backend uses instead, which allows nil for the references.
    static const char *Member(VKind k) {
        static const char *names[] = { "ival", "fval", "ref", "ival", "sval", "vval", "oval" };
        return names[k];
    }
    static const char *Accessor(VKind k) {
        static const char *names[] = { "ival()", "fval()", "refany()", "ip()", "svalnil()",
                                       "vvalnil()", "ovalnil()" };
        return names[k];
    }

    // The same for reading a value whose tag may still say nil, which is what a global holds
    // before its initializer has run, see DefineFunction.
    static const char *AccessorNil(VKind k) {
        static const char *names[] = { "ivalnil()", "fvalnil()", "refany()", "ipnil()",
                                       "svalnil()", "vvalnil()", "ovalnil()" };
        return names[k];
    }

    static Place Var(string s, RTType rtt) { return { std::move(s), rtt, true, true }; }
    static Place Var(string s, VKind k) { return Var(std::move(s), Rtt(k)); }
    static Place Mem(string s, RTType rtt) { return { std::move(s), rtt, false }; }
    static Place Mem(string s, TypeRef type) { return Mem(std::move(s), RtTypeOf(type)); }
    // Memory that holds the type it says, which is what a field of an object is.
    static Place Direct(string s, RTType rtt) { return { std::move(s), rtt, true }; }

    bool HasPending(int slot) {
        return slot >= 0 && slot < (int)pending.size() && !pending[slot].expr.empty();
    }

    // Remembers `expr` as what slot `d` holds in place of writing it there.
    void Defer(const Place &d, string expr, vector<string> vars, int prec) {
        assert(d.slot >= 0);
        if ((int)pending.size() <= d.slot) pending.resize(d.slot + 1);
        pending[d.slot] = { d, std::move(expr), std::move(vars), prec };
    }
    void Defer(const Place &d, string expr, string var) {
        vector<string> vars;
        if (!var.empty()) vars.push_back(std::move(var));
        Defer(d, std::move(expr), std::move(vars), 0);
    }

    // Emits the write a pending slot stands for, since from here on its expression may not
    // hold any more.
    void Materialize(Pending &p) {
        auto expr = std::move(p.expr);
        p.expr.clear();
        Write(cb, p.slot, expr);
    }

    static bool Reads(const Pending &p, string_view var) {
        for (auto &v : p.vars) if (v == var) return true;
        return false;
    }

    // Every slot still on the stack gets its value, for control flow: whatever reads it past a
    // label can come from either path. The ones above the stack were consumed already.
    void Flush() {
        for (auto [i, p] : enumerate(pending)) {
            if (p.expr.empty()) continue;
            if ((int)i < (int)tstack_size) Materialize(p); else p.expr.clear();
        }
    }

    // The same for the slots that read the variable about to be written, whether it is a
    // local or another slot.
    void FlushVar(string_view var) {
        for (auto &p : pending) if (!p.expr.empty() && Reads(p, var)) Materialize(p);
    }

    // The C precedence of the operators the emitters build expressions from, lower binding
    // tighter: 1 a call or index, 2 a prefix operator or cast, 3 * / %, 4 + -, 5 the shifts,
    // 6 the relational and 7 the equality comparisons, 8 &, 9 ^, 10 |, 11 &&, 12 ||,
    // 13 the conditional operator.
    // Which pairs C groups in a way that reads wrong to most people, and that compilers warn
    // about under -Wparentheses even where the grouping is the one we mean: a comparison in a
    // comparison, arithmetic in a shift, and arithmetic, a comparison or a tighter bitwise
    // operator in a bitwise one.
    static bool Surprising(int prec, int operand) {
        auto comparison = [](int p) { return p == 6 || p == 7; };
        switch (prec) {
            case 5: return operand == 4;
            case 6:
            case 7: return comparison(operand);
            case 8: return operand == 4 || comparison(operand);
            case 9:
            case 10:
                return operand == 4 || comparison(operand) || (operand >= 8 && operand < prec);
            default: return false;
        }
    }

    // An expression as an operand of an operator of precedence `prec`, in parentheses when it
    // binds looser, or as loose on the right, which keeps the grouping, or when the pair would
    // be surprising as written.
    static Expr &Parens(Expr &e, int prec, bool right = false) {
        if (e.prec > prec || (e.prec == prec && right) || Surprising(prec, e.prec)) {
            e.text = cat("(", e.text, ")");
            e.prec = 0;
        }
        return e;
    }

    // A place as an operand of an operator of precedence `prec`. A `prec` past the operators,
    // 15, is a context that groups by itself, such as an argument or a whole condition.
    Expr Operand(const Place &p, int prec, bool right = false) {
        Expr e;
        if (HasPending(p.slot)) {
            auto &q = pending[p.slot];
            e = { q.expr, q.vars, true, q.prec };
        } else if (p.var) {
            e = { p.s, { p.s }, true, 0 };
        } else {
            e = { Read(p), {}, false, 0 };
        }
        return Parens(e, prec, right);
    }

    // Truth tests the payload bits, as Value::True/False do: -0.0 is true. A typed
    // float must first be viewed through the union, without a numeric conversion.
    Expr TruthOperand(const Place &p, int prec) {
        if (p.k() != VK_FLOAT) return Operand(p, prec);
        auto e = Operand(p, 15);
        if (cpp) {
            e.text = cat("Value(", e.text, ").bits()");
        } else {
            // Write the union directly: a compound literal can call memset in TCC,
            // and boxing through a helper would add a call to every float test.
            auto bits = cat("_truth", Label());
            append(cb, "    Value ", bits, ";\n    ", bits, ".fval = ", e.text, ";\n");
            e.text = cat(bits, ".ival");
            e.vars.clear();
            e.pure = false;  // This snapshot must be consumed where it was written.
        }
        e.prec = 1;
        return e;
    }

    // What an operator makes of its operands.
    static Expr Combine(int prec, string text, const Expr &a, const Expr &b) {
        Expr e = { std::move(text), a.vars, a.pure && b.pure, prec };
        e.vars.insert(e.vars.end(), b.vars.begin(), b.vars.end());
        return e;
    }

    // A prefix operator on a place; a repeated minus gets parentheses, since two in a row
    // would read as a decrement.
    Expr Unary(string_view op, const Place &v) {
        auto e = Operand(v, 2);
        if (e.text[0] == op[0] && op[0] == '-') e.text = cat("(", e.text, ")");
        e.text = cat(op, e.text);
        e.prec = 2;
        return e;
    }

    // An operand of one of the wrapping integer operations (see BinExpr) that is itself the
    // result of one (or of a shift, see BitExpr), recognizable as the conversion back to
    // signed of an expression that starts with the conversion to unsigned: since the
    // operation converts it to unsigned again, both conversions are dropped, which keeps a
    // chain of them readable. Returns whether the operand is now that unsigned expression,
    // whose own operator is at most as loose as a shift.
    bool UnsignedOperand(Expr &e) {
        auto back = cat("(", IType(), ")(");
        if (e.prec != 2 || e.text.compare(0, back.size(), back) != 0) return false;
        // The conversion to unsigned may sit behind the parentheses of an operand that was
        // itself one of these.
        auto p = back.size();
        while (p < e.text.size() && e.text[p] == '(') p++;
        auto to_unsigned = cat(UType(), ")");
        auto negated = cat("0 - (", UType(), ")");
        if (e.text.compare(p, to_unsigned.size(), to_unsigned) != 0 &&
            e.text.compare(p, negated.size(), negated) != 0) return false;
        e.text = e.text.substr(back.size(), e.text.size() - back.size() - 1);
        e.prec = 5;
        return true;
    }

    // The negation of an int, which wraps (the smallest int negates to itself) where C leaves
    // it undefined: done on the unsigned type and converted back, see BinExpr.
    Expr WrappingNegate(const Place &v) {
        auto e = Operand(v, 2);
        if (UnsignedOperand(e)) Parens(e, 4, true);
        else e.text = cat("(", UType(), ")", e.text);
        e.text = cat("(", IType(), ")(0 - ", e.text, ")");
        e.prec = 2;
        return e;
    }

    // Whether a place is the float constant zero, told by the literal a zero is written as,
    // which is what a division needs to know about its divisor, see BinExpr.
    bool IsFloatZeroLiteral(const Place &p) {
        if (!HasPending(p.slot)) return false;
        auto &q = pending[p.slot];
        return q.vars.empty() && !q.prec &&
               (q.expr == FloatLiteral(0.0) || q.expr == FloatLiteral(-0.0));
    }

    // Whether a place is an integer constant, and which, told by the literal it is written as,
    // see IntLiteral, which is what a bit op on two of them folds, see BitExpr.
    bool IsIntLiteral(const Place &p, int64_t &val) {
        if (!HasPending(p.slot)) return false;
        auto &q = pending[p.slot];
        if (!q.vars.empty() || q.prec) return false;
        auto s = q.expr.c_str();
        if (*s == '(') s++;
        char *end;
        val = strtoll(s, &end, 10);
        return end != s && IntLiteral(val) == q.expr;
    }

    // A binary operator on scalars is the C operator, except for integer division and modulo,
    // which check their divisor and so have to run where they are, float modulo, which is
    // fmod, and a float division by a literal zero: Lobster wants the infinity or nan that
    // produces, but C rejects the constant expression it would otherwise be written as (MSVC
    // C2124), and a call is not a constant expression.
    // An integer division or modulo whose divisor the optimizer proved safe skips the check and
    // is the operator like the rest, which also keeps it a pending expression rather than
    // forcing it through a slot.
    Expr BinExpr(bool isfloat, MathOp op, const Place &a, const Place &b,
                 bool divisor_safe = false) {
        static const char *cops[] = { "+", "-", "*", "/", "%",
                                      "<", ">", "<=", ">=", "==", "!=" };
        static const int precs[] = { 4, 4, 3, 3, 3, 6, 6, 6, 6, 7, 7 };
        auto fdivzero = op == MOP_DIV && isfloat && IsFloatZeroLiteral(b);
        auto unchecked = divisor_safe && !isfloat && (op == MOP_DIV || op == MOP_MOD);
        if (!unchecked && (op == MOP_MOD || (op == MOP_DIV && !isfloat) || fdivzero)) {
            auto x = Operand(a, 15), y = Operand(b, 15);
            // The only helper without an argument the C++ backend could find it thru.
            auto call = fdivzero ? cat(cpp ? "lobster::" : "", "RtFDiv(", x.text, ", ", y.text, ")")
                      : op == MOP_DIV ? cat("RtIDiv(vm, ", x.text, ", ", y.text, ")")
                      : isfloat ? cat(cpp ? "lobster::" : "", "RtFMod(", x.text, ", ", y.text, ")")
                                : cat("RtIMod(vm, ", x.text, ", ", y.text, ")");
            auto e = Combine(1, call, x, y);
            e.pure = e.pure && isfloat;
            return e;
        }
        auto prec = precs[op];
        if (!isfloat && op <= MOP_MUL) {
            // Integer + - * wrap (two's complement), which C leaves undefined for signed
            // operands: done on the unsigned type and converted back, which costs nothing on
            // a two's complement target. The right operand converts to unsigned implicitly.
            auto x = Operand(a, 2), y = Operand(b, prec, true);
            if (UnsignedOperand(x)) Parens(x, prec);
            else x.text = cat("(", UType(), ")", x.text);
            if (UnsignedOperand(y)) Parens(y, prec, true);
            return Combine(2, cat("(", IType(), ")(", x.text, " ", cops[op], " ", y.text, ")"),
                           x, y);
        }
        auto x = Operand(a, prec), y = Operand(b, prec, true);
        return Combine(prec, cat(x.text, " ", cops[op], " ", y.text), x, y);
    }

    // What one produces: an int for a comparison whatever it compared.
    static VKind BinKind(bool isfloat, MathOp op) {
        return isfloat && op < MOP_LT ? VK_FLOAT : VK_INT;
    }
    static VKind ScalarKind(bool isfloat) { return isfloat ? VK_FLOAT : VK_INT; }

    // The shifts mask their count to the width of an int, see MaskedShiftLeft, and both shift
    // what they are given as 64 bits, which a constant is not by itself.
    // A bit op on two constants is written as its value, which reads better than the shift does,
    // and keeps a C++ compiler from taking an xor of a literal 2 or 10 for a power gone wrong
    // (clang -Wxor-used-as-pow).
    Expr BitExpr(BitOp op, const Place &a, const Place &b) {
        int64_t x, y;
        if (IsIntLiteral(a, x) && IsIntLiteral(b, y)) {
            auto v = op == BIT_AND ? x & y
                   : op == BIT_OR  ? x | y
                   : op == BIT_XOR ? x ^ y
                   : op == BIT_ASL ? MaskedShiftLeft(x, y)
                                   : MaskedShiftRight(x, y);
            return { IntLiteral(v), {}, true, 0 };
        }
        switch (op) {
            case BIT_AND:
            case BIT_OR:
            case BIT_XOR: {
                auto prec = op == BIT_AND ? 8 : op == BIT_XOR ? 9 : 10;
                auto x = Operand(a, prec), y = Operand(b, prec, true);
                auto cop = op == BIT_AND ? " & " : op == BIT_XOR ? " ^ " : " | ";
                return Combine(prec, cat(x.text, cop, y.text), x, y);
            }
            case BIT_ASL: {
                auto x = Operand(a, 2), y = Operand(b, 8);
                return Combine(2, cat("(long long)((unsigned long long)", x.text, " << (", y.text,
                                      " & 63))"), x, y);
            }
            default: {
                auto x = Operand(a, 2), y = Operand(b, 8);
                return Combine(5, cat("(long long)", x.text, " >> (", y.text, " & 63)"), x, y);
            }
        }
    }

    // The bits a value of a field stored in part of its slot (see SField::bits) occupies in it.
    static uint64_t BitMask(const SField &sfield) {
        return (((uint64_t)1 << sfield.bits) - 1) << sfield.bitoff;
    }
    static Expr &Grouped(Expr &e) {
        if (e.prec) {
            e.text = cat("(", e.text, ")");
            e.prec = 0;
        }
        return e;
    }

    // The value of such a field out of the int slot at `word`, as the type it is: an int sign
    // extended from its bits (a single shift when they are the top ones), a float widened from
    // the 32-bit float they are (see mkvalF32 in the Prologue).
    Expr ExtractExpr(const Place &word, const SField &sfield) {
        if (sfield.isfloat()) {
            auto w = Operand(word, 5);
            auto bits = sfield.bitoff ? cat(w.text, " >> ", sfield.bitoff) : w.text;
            auto call = cpp ? "lobster::UnpackFloat32" : "mkvalF32";
            return { cat(call, "(", bits, ")"), w.vars, w.pure, 1 };
        }
        auto top = 64 - sfield.bitoff - sfield.bits;
        if (!top) {
            auto w = Operand(word, 5);
            return { cat(w.text, " >> ", sfield.bitoff), w.vars, w.pure, 5 };
        }
        auto w = Operand(word, 2);
        return { cat("(", IType(), ")((", UType(), ")", w.text, " << ", top, ") >> ",
                     64 - sfield.bits),
                 w.vars, w.pure, 5 };
    }

    // The bits such a field's value `val` takes in its slot, shifted into place, with nothing
    // outside them: the bits of the 32-bit float it is for a float, the low bits for an int,
    // whose sign extension the mask takes off.
    Expr FieldBitsExpr(Expr val, const SField &sfield) {
        if (sfield.isfloat()) {
            auto call = cpp ? "lobster::PackFloat32" : "mkvalBits32";
            auto bits = cat("(", UType(), ")", call, "(", val.text, ")");
            if (!sfield.bitoff) return { bits, val.vars, val.pure, 2 };
            return { cat(bits, " << ", sfield.bitoff), val.vars, val.pure, 5 };
        }
        Parens(val, 2);
        auto bits = cat("((", UType(), ")", val.text, " & ",
                        MaskLiteral(BitMask(sfield) >> sfield.bitoff), ")");
        if (!sfield.bitoff) return { bits, val.vars, val.pure, 0 };
        return { cat(bits, " << ", sfield.bitoff), val.vars, val.pure, 5 };
    }

    // The int slot `word` with such a field's value `val` written into its bits, the other
    // bits as they were.
    Expr InsertExpr(const Place &word, Expr val, const SField &sfield) {
        auto bits = FieldBitsExpr(std::move(val), sfield);
        Grouped(bits);
        int64_t zero;
        if (IsIntLiteral(word, zero) && !zero) {
            // A slot that is still 0 needs nothing kept.
            return { cat("(", IType(), ")", bits.text), bits.vars, bits.pure, 2 };
        }
        auto w = Operand(word, 2);
        auto text = cat("(", IType(), ")(((", UType(), ")", w.text, " & ~",
                        MaskLiteral(BitMask(sfield)), ") | ", bits.text, ")");
        return Combine(2, text, w, bits);
    }

    // The int slot with the values `vals` of all the fields it holds in it, made from 0, so
    // the bits none of them has are 0 as well.
    Expr PackExpr(const vector<pair<Place, const SField *>> &vals) {
        Expr acc;
        for (auto &[place, sfield] : vals) {
            auto bits = FieldBitsExpr(Operand(place, 15), *sfield);
            Grouped(bits);
            acc = acc.text.empty() ? bits : Combine(10, cat(acc.text, " | ", bits.text), acc, bits);
        }
        Grouped(acc);
        return { cat("(", IType(), ")", acc.text), acc.vars, acc.pure, 2 };
    }

    // Writes an expression to a slot, which for an int or float that can be deferred is
    // remembering it, unless it has grown long enough to be worth a line of its own.
    void WriteExpr(const Place &d, const Expr &e) {
        if (d.slot >= 0 && e.pure && (d.k() == VK_INT || d.k() == VK_FLOAT) &&
            e.text.size() <= 80) {
            Defer(d, e.text, e.vars, e.prec);
        } else {
            Write(cb, d, e.text);
        }
    }

    // A slot needs a variable for each kind of value it holds, see f_slot_kinds.
    void SlotVarUsed(const Place &p) {
        if (p.slot < 0) return;
        if ((int)f_slot_kinds.size() <= p.slot) f_slot_kinds.resize(p.slot + 1, 0);
        f_slot_kinds[p.slot] |= 1 << p.k();
    }

    // A place, as the C expression of its kind.
    string Read(const Place &p) {
        if (HasPending(p.slot)) return pending[p.slot].expr;
        // The code after something that never completes (a return in a value position, a call
        // to a function whose every return is non-local) reads a slot nothing wrote, see
        // Return::Generate: it can never run, but it still has to compile.
        SlotVarUsed(p);
        if (p.typed) return p.s;
        if (cpp) return cat(p.s, ".", Accessor(p.k()));
        if (p.k() == VK_FUN) return cat("(fun_base_t)", p.s, ".ival");
        return cat(p.s, ".", Member(p.k()));
    }

    // The same where the value may still be a nil, which only the C++ backend asserts against,
    // since the C one reads the union field without a tag check either way.
    string ReadNil(const Place &p) {
        if (!cpp || p.typed || HasPending(p.slot)) return Read(p);
        return cat(p.s, ".", AccessorNil(p.k()));
    }

    // The same as a value of another kind, which for two kinds of reference is a cast.
    string ReadAs(const Place &p, VKind k) {
        auto r = Read(p);
        if (p.k() == k || !IsRefKind(k) || !IsRefKind(p.k())) return r;
        return cat("(", CType(k), ")", r);
    }

    // Writing an expression of the place's kind to it. The C++ backend goes thru Value's
    // constructor for memory, which sets the tag from the type, the C one writes the field.
    string WriteText(const Place &d, string_view expr) {
        SlotVarUsed(d);
        if (d.typed) return cat(d.s, " = ", expr, ";");
        if (cpp) return cat(d.s, " = Value(", expr, ");");
        switch (d.k()) {
            case VK_INT: return cat(d.s, ".ival = ", expr, ";");
            case VK_FLOAT: return cat(d.s, ".fval = ", expr, ";");
            case VK_FUN: return cat(d.s, ".ival = (long long)(", expr, ");");
            default: return cat(d.s, ".", Member(d.k()), " = ", expr, ";");
        }
    }

    void Write(string &sd, const Place &d, string_view expr, string_view lf = "\n") {
        if (HasPending(d.slot)) pending[d.slot].expr.clear();
        if (d.typed) FlushVar(d.s);
        append(sd, "    ", WriteText(d, expr), lf);
    }

    // A copy of one place to another. Between two in memory the C backend copies the field and
    // the tag rather than the struct, since libtcc turns a struct assignment into a memcpy call.
    string CopyValueText(const Place &d, const Place &s) {
        assert(d.k() == s.k() || (IsRefKind(d.k()) && IsRefKind(s.k())));
        if (d.typed || s.typed) return WriteText(d, ReadAs(s, d.k()));
        if (cpp) return cat(d.s, " = ", s.s, ";");
        auto fld = cat(".", Member(s.k()));
        return cat(d.s, fld, " = ", s.s, fld, ";");
    }

    void CopyValue(string &sd, const Place &d, const Place &s, string_view lf = "\n") {
        if (d.slot >= 0 && s.var && &sd == &cb && d.k() == s.k()) {
            // A push of a variable, or of a slot that is itself pending, is only remembered.
            // A computed expression is written first rather than computed twice.
            if (HasPending(s.slot) && pending[s.slot].prec) Materialize(pending[s.slot]);
            if (HasPending(s.slot)) {
                auto &q = pending[s.slot];
                Defer(d, q.expr, q.vars, q.prec);
            } else {
                Defer(d, s.s, s.s);
            }
        } else if (d.typed || s.typed) {
            Write(sd, d, ReadAs(s, d.k()), lf);
        } else {
            append(sd, "    ", CopyValueText(d, s), lf);
        }
    }

    // A copy out of a slot this op consumes, which is the last read of it: an expression the
    // slot holds goes into the copy and nowhere else, so it is dropped rather than written to
    // the slot as well when the destination is a variable it reads, as in `x = x * 2`.
    void CopyConsumed(string &sd, const Place &d, const Place &s) {
        if (s.slot < 0 || !HasPending(s.slot) || !(d.typed || s.typed)) {
            CopyValue(sd, d, s);
            return;
        }
        auto r = ReadAs(s, d.k());
        pending[s.slot].expr.clear();
        Write(sd, d, r);
    }

    void SetNil(string &sd, const Place &d) {
        if (d.slot >= 0 && &sd == &cb) Defer(d, d.k() == VK_FLOAT ? "0.0" : "0", "");
        else if (d.typed) Write(sd, d, d.k() == VK_FLOAT ? "0.0" : "0");
        else if (cpp) append(sd, "    ", d.s, " = lobster::NilVal();\n");
        else append(sd, "    ", d.s, ".ival = 0;\n");
    }

    // The name of variable slot `i`, which for a slot of a struct says which one it is.
    string IdName(int i, TypeRef type) {
        auto ididx = sids[i].ididx;
        auto idx = sids[i].idx;
        auto &basename = st.identtable[ididx]->name;
        if (!IsStruct(type->t)) return basename;
        int j = i;
        while (j && sids[j - 1].idx == idx) j--;
        return cat(basename, "+", i - j);
    }

    // The name of slot `slot` of a struct: the field it is in, and for a nested struct that
    // field's own slot name behind it. A hidden slot (see UDT::hidden_sfields) is named for
    // what it is.
    string StructSlotName(const UDT &udt, int slot) {
        for (auto [k, sfield] : enumerate(udt.sfields)) {
            if (slot >= sfield.slot && slot < sfield.slot + ValWidth(sfield.type)) {
                string name = udt.g.fields[k].id->name;
                if (IsStruct(sfield.type->t)) {
                    append(name, "_", StructSlotName(*sfield.type->udt, slot - sfield.slot));
                }
                return name;
            }
        }
        for (auto &sfield : udt.hidden_sfields) {
            if (sfield.slot == slot) return slot ? cat("pad", slot) : string("type");
        }
        assert(false);
        return cat("slot", slot);
    }

    // These namespaces are synthesized by the emitters. Both reservation and
    // allocation use this list: adding a suffix cannot escape a reserved prefix.
    static bool HasReservedPrefix(string_view name) {
        return (!name.empty() && name[0] == '_') || name.substr(0, 2) == "Rt" ||
               name.substr(0, 8) == "builtin_" || name.substr(0, 4) == "fun_" ||
               name.substr(0, 4) == "udt_" || name.substr(0, 5) == "elem_" ||
               name.substr(0, 4) == "ret_" || name.substr(0, 5) == "mkval";
    }

    // Whether the generated code uses a name for something of its own, which a local may then
    // not be called: the keywords of C and C++, what the prologues declare, the helpers, and the
    // names the emitters make up, which all end in a number or start with an underscore.
    static bool IsReservedName(string_view name) {
        static const set<string_view> reserved = {
            "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
            "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long",
            "register", "restrict", "return", "short", "signed", "sizeof", "static", "struct",
            "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
            "alignas", "alignof", "and", "and_eq", "asm", "bitand", "bitor", "bool", "catch",
            "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "consteval",
            "constexpr", "constinit", "const_cast", "co_await", "co_return", "co_yield",
            "decltype", "delete", "dynamic_cast", "explicit", "export", "false", "friend",
            "import", "module", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
            "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
            "reinterpret_cast", "requires", "static_assert", "static_cast", "template", "this",
            "thread_local", "throw", "true", "try", "typeid", "typename", "using", "virtual",
            "wchar_t", "xor", "xor_eq", "override", "final", "NULL",
            "vm", "lv", "lo", "lvec", "lidx", "locals", "ctx", "tsld", "top", "rs", "ret",
            "epilogue", "main", "argc", "argv", "vmmeta", "Value", "VMRef", "StackPtr",
            "RefObj", "LVector", "LString", "LObject", "VMBase", "fun_base_t", "type_elem_t",
            "object_dec_t", "vec", "VMSpan", "VMString", "VMField", "VMUDT", "VMSpecIdent",
            "VMEnumVal", "VMEnum", "VMMetaData",
            "vtables", "object_decs", "const_strings", "funinfo_table", "compiled_entry_point",
            "type_table",
            "file_names", "function_names", "udts", "specidents", "enums", "ser_ids",
            "subfunctions_to_function", "iint", "int2float64", "lobster", "std", "string_view",
            "span", "uint64_t", "int64_t", "memcpy", "memmove", "GLFrame", "Entry", "IDXErr",
            "IDXErrS", "BackupVar", "DecOwned", "RcStat", "DecDelete", "DecDeleteVec", "DecDeleteObj",
            "DecDeleteStr", "AssertFailed",
            "RestoreBackup", "GetTypeSwitchID", "PushFunId", "PopFunId", "StartProfile",
            "EndProfile", "STRING_DATA", "pctx",
        };
        if (reserved.count(name)) return true;
        if (HasReservedPrefix(name)) return true;
        auto numbered = [&](string_view prefix) {
            if (name.size() <= prefix.size() || name.substr(0, prefix.size()) != prefix) return false;
            for (auto c : name.substr(prefix.size())) if (!isdigit((uint8_t)c)) return false;
            return true;
        };
        return numbered("i") || numbered("f") || numbered("p") || numbered("fn") ||
               numbered("s") || numbered("v") || numbered("o") || numbered("keep") ||
               numbered("block") || numbered("ivec") || numbered("fvec") ||
               numbered("mkivec") || numbered("mkfvec") || numbered("str");
    }

    // A C name unique within its namespace: the name it has in the program,
    // made a C identifier if it is not one, with a number behind it if that is taken. A prefix
    // the generated code claims for itself gets a letter in front instead, since a number
    // behind it would not lose it.
    string UniqueName(string name, set<string> &names_used) {
        for (auto &c : name) if (!isalnum((uint8_t)c) && c != '_') c = '_';
        // The suffix can itself enter a reserved namespace (ret -> ret_2), so
        // check its separator as well before choosing a base for the loop.
        if (name.empty() || isdigit((uint8_t)name[0]) || HasReservedPrefix(name) ||
            HasReservedPrefix(cat(name, "_"))) {
            name = "v" + name;
        }
        auto base = name;
        for (int n = 2; IsReservedName(name) || names_used.count(name); n++) {
            name = cat(base, "_", n);
        }
        names_used.insert(name);
        return name;
    }

    string UniqueName(string name) { return UniqueName(std::move(name), f_names_used); }

    // Slot `slot` of the variable sid, which for a struct is one of its fields.
    string LocalName(const SpecIdent &sid, int slot) {
        string name = sid.id->name;
        if (IsStruct(sid.type->t)) append(name, "_", StructSlotName(*sid.type->udt, slot));
        return UniqueName(name);
    }

    // The stack slots, counted the way the emitters do: Slot(1) is the top of the stack before
    // the current op, Slot(0) the first one above it, Slot(-1) the one after that. Each is a
    // variable per kind of value it holds, i5, f5, s5, v5, o5, p5 and fn5 for slot 5, which
    // is what lets the C compiler keep them in registers: nothing ever takes their address.
    Place SlotVar(int idx, RTType rtt) {
        static const char *prefix[] = { "i", "f", "p", "fn", "s", "v", "o" };
        auto p = Var(cat(prefix[Kind(rtt)], idx), rtt);
        p.slot = idx;
        return p;
    }
    Place SlotVar(int idx, VKind k) { return SlotVar(idx, Rtt(k)); }
    Place Slot(int off, RTType rtt) { return SlotVar(regso - off, rtt); }
    Place Slot(int off, VKind k) { return SlotVar(regso - off, k); }
    // The slot holding slot i of a value of this type.
    Place Slot(int off, TypeRef type, int i = 0) {
        return SlotVar(regso - off, RtTypeOf(SlotType(type, i)));
    }

    // The array a struct local goes into to be indexed at runtime, one per numeric type its
    // fields can all be of, see EmitLvalStructIndex.
    static string StageArray(VKind k) { return k == VK_FLOAT ? "_lsf" : "_lsi"; }
    // A local is a variable of its own like a stack slot, see LocalName. Asking for it is what
    // declares it: a variable whose every read and write the optimizer replaced by the constant
    // it holds, or removed with the code around it, is never asked for and so never declared.
    Place Local(int i) {
        local_used[i] = true;
        return local_places[i];
    }
    // With stack traces on, every write to a local also lands in an array, since that is where
    // a trace dumps them from, see PushFunId.
    bool ShadowLocals() { return runtime_checks >= RUNTIME_STACK_TRACE; }
    Place Shadow(int i) { return Mem(cat("locals[", i, "]"), local_places[i].rtt); }
    void LocalWritten(int idx, int width) {
        if (!ShadowLocals()) return;
        for (int i = 0; i < width; i++) CopyValue(cb, Shadow(idx + i), Local(idx + i));
    }
    // The C++ backend addresses the VM's own array of globals at a constant offset, which is why
    // that array sits at the end of the VM; the C one has no way to know where that is, so it
    // goes thru the pointer to it that VMBase carries for that purpose.
    Place Global(int varidx) {
        return Mem(cpp ? cat("vm.fvars[", varidx, "]") : cat("vm->fvars_ptr[", varidx, "]"),
                   var_types[varidx]);
    }

    Place KeepVar(int i) { return Var(cat("keep", i), f_keeps[i]); }

    void comment(string_view c) { append(cb, " // ", c, "\n"); }

    void TypeComment(TypeRef type) {
        if (IsUDT(type->t)) comment(type->udt->name); else cb += "\n";
    }

    // The unsigned 64-bit type, for the shifts and masks of the fields stored in part of a
    // slot, which must not be done in signed arithmetic.
    string UType() { return cpp ? "uint64_t" : "unsigned long long"; }
    string IType() { return CType(VK_INT); }

    // The struct the generated code gets for a type, whose members are its fields by the names
    // they have in the program. It is emitted once the code is known to name them, the way the
    // builtins it calls are, see udts_used. An object carries the header every reference does
    // in front of its fields, where a struct is only the fields, since a run of them is what
    // the elements of a vector are, see Elem.
    string UDTName(const UDT &udt) {
        MembersOf(udt);
        auto name = cat(udt.g.is_struct ? "elem_" : "udt_", udt.name, udt.idx);
        for (auto &c : name) if (!isalnum((uint8_t)c) && c != '_') c = '_';
        return name;
    }

    // Whether every slot of a value of this type holds the same C type, which makes a run of
    // them a flat array of it, so that a slot is an index into that and no struct is needed.
    bool UniformSlots(TypeRef type, int width) {
        auto ct = SlotCType(type, 0);
        for (int i = 1; i < width; i++) if (SlotCType(type, i) != ct) return false;
        return true;
    }

    // Slot `slot` of the element at `idx` of a vector whose slots start at `elems`.
    Place Elem(string_view elems, TypeRef elemtype, string_view idx, int slot) {
        auto width = ValWidth(elemtype);
        auto rtt = RtTypeOf(SlotType(elemtype, slot));
        if (UniformSlots(elemtype, width)) {
            auto i = width == 1
                ? (slot ? cat("(", idx, ") + ", slot) : string(idx))
                : cat("(", idx, ") * ", width, slot ? cat(" + ", slot) : string());
            auto ct = SlotCType(elemtype, 0);
            return Direct(cat("((", ct, ct.back() == '*' ? "" : " ", "*)", elems, ")[", i,
                              "]"), rtt);
        }
        // A struct of more than one type is an array of the struct emitted for it.
        return Direct(cat("((", UDTName(*elemtype->udt), " *)", elems, ")[", idx, "].",
                          MemberAt(*elemtype->udt, slot)), rtt);
    }

    // The C type slot `i` of a value of this type holds.
    string SlotCType(TypeRef type, int i) {
        return CType(KindOf(SlotType(type, i)));
    }

    // The members of a UDT. A field that is a struct of one type over and over is an array of
    // it, which lets the program index one at runtime, see EmitLvalStructIndex.
    // The hidden slots of a member of an abstract struct family are members of their own. A
    // slot several fields are stored in (see SField::bits) is one int member, named for the
    // first of them.
    vector<UDTMember> Members(const UDT &udt) {
        vector<UDTMember> ms;
        // Renaming a keyword or flattening a nested field can produce another
        // field's name. Allocate every member in this UDT's namespace, including
        // the object header, and cache the resulting names in MembersOf.
        set<string> names_used;
        if (!udt.g.is_struct) names_used = { "typeinfo", "refc" };
        auto field = [&](size_t k) {
            auto &sfield = udt.sfields[k];
            if (sfield.bits) {
                if (sfield.bitoff) return 0;
                ms.push_back({ UniqueName(string(udt.g.fields[k].id->name), names_used),
                               CType(VK_INT), sfield.slot, 1 });
                return 1;
            }
            auto width = ValWidth(sfield.type);
            auto ct = SlotCType(sfield.type, 0);
            auto same = true;
            for (int i = 1; i < width; i++) same = same && SlotCType(sfield.type, i) == ct;
            if (same) {
                ms.push_back({ UniqueName(string(udt.g.fields[k].id->name), names_used), ct,
                               sfield.slot, width });
            } else {
                for (int i = 0; i < width; i++) {
                    ms.push_back({ UniqueName(StructSlotName(udt, sfield.slot + i), names_used),
                                   SlotCType(sfield.type, i), sfield.slot + i, 1 });
                }
            }
            return width;
        };
        if (udt.family_root) {
            // In slot order, which is not field order for a member of an abstract struct
            // family (see SymbolTable::LayoutFamily), with its hidden slots as members of
            // their own.
            for (int s = 0; s < udt.numslots;) {
                int k = -1;
                for (auto [i, sfield] : enumerate(udt.sfields)) {
                    if (sfield.slot == s && (k < 0 || sfield.bitoff < udt.sfields[k].bitoff)) {
                        k = (int)i;
                    }
                }
                if (k < 0) {
                    // A hidden slot.
                    ms.push_back({ UniqueName(StructSlotName(udt, s), names_used),
                                   SlotCType(&udt.thistype, s), s, 1 });
                    s++;
                } else if (udt.sfields[k].bits) {
                    // A slot several fields share, named for the lowest of them, which in the
                    // type slot sits above the family index.
                    ms.push_back({ UniqueName(string(udt.g.fields[k].id->name), names_used),
                                   CType(VK_INT), s, 1 });
                    s++;
                } else {
                    s += field((size_t)k);
                }
            }
        } else {
            for (size_t k = 0; k < udt.sfields.size(); k++) field(k);
        }
        return ms;
    }

    // Kept once the code is known to name them, since finding one is a walk over the fields.
    const vector<UDTMember> &MembersOf(const UDT &udt) {
        auto it = udts_used.find(udt.idx);
        if (it == udts_used.end()) it = udts_used.insert({ udt.idx, { &udt, Members(udt) } }).first;
        return it->second.second;
    }

    // The object types whose fields the code names, see UDTName. Each is the header every
    // reference carries and then the members of its fields (see Members), whose slots are the
    // size of a Value whatever they hold, which is what an object is; Entry() checks the size of
    // both of those, so nothing here needs a check of its own.
    void DeclareUDTStructs(string &sd) {
        for (auto &[idx, um] : udts_used) {
            auto &[udt, members] = um;
            sd += "typedef struct {\n";
            if (!udt->g.is_struct) sd += "    int typeinfo;\n    int refc;\n";
            for (auto &m : members) {
                append(sd, "    ", m.ctype, m.ctype.back() == '*' ? "" : " ", m.name);
                if (m.count > 1) append(sd, "[", m.count, "]");
                sd += ";\n";
            }
            append(sd, "} ", UDTName(*udt), ";  // ", udt->name, "\n");
        }
    }

    // The member slot `slot` lives in, and where in it when that member is an array.
    string MemberAt(const UDT &udt, int slot) {
        for (auto &m : MembersOf(udt)) {
            if (slot >= m.slot && slot < m.slot + m.count) {
                return m.count > 1 ? cat(m.name, "[", slot - m.slot, "]") : m.name;
            }
        }
        // A slot no field claims, which only a malformed type would have.
        assert(false);
        return cat("slot", slot);
    }

    // A member at the object `obj`, as a C expression.
    string FieldName(string_view obj, const UDT &udt, int slot) {
        return cat("((", UDTName(udt), " *)", obj, ")->", MemberAt(udt, slot));
    }

    // One of those members as a place. It holds the type the slot is (see SlotTypeOf, which
    // is what Members declared it as), which the runtime only ever reads a byte at a time, see
    // LoadSlot. Whatever reads or writes it as a field of a more specific type does so thru a
    // cast, see CopyValue.
    Place Field(string_view obj, const UDT &udt, int slot) {
        return Direct(FieldName(obj, udt, slot), RtTypeOf(SlotTypeOf(udt, slot)));
    }

    // Pointer arithmetic is only valid within one actual C member (including a scalar as an
    // array of length one). Consecutive Lobster slots can instead be separate C members.
    bool MemberContainsRange(const UDT &udt, int slot, int numslots) {
        for (auto &m : MembersOf(udt)) {
            if (slot >= m.slot && slot < m.slot + m.count)
                return slot + numslots <= m.slot + m.count;
        }
        assert(false);
        return false;
    }

    // The type index an object carries, which both backends keep in the first field of the
    // header every reference has, see Prologue and the C++ DynAlloc.
    string TypeIdOf(string_view obj) {
        return cpp ? cat("(int)", obj, "->tti") : cat(obj, "->typeinfo");
    }

    // The family index in the type slot `word` of a struct in an abstract struct family: the
    // whole slot, or its low bits when fields share the slot, see UDT::family_type_slot_shared.
    string FamilyIndexText(const Place &word, const UDT &root) {
        if (!root.family_type_slot_shared) return Read(word);
        auto w = Operand(word, 8);
        return cat("(", w.text, " & ", MaskLiteral((1 << FAMILY_INDEX_BITS) - 1), ")");
    }

    int ComputeBitMask(const UDT &udt) {
        int bits = 0;
        for (int j = 0; j < udt.numslots; j++) {
            if (IsRefNil(SlotTypeOf(udt, j)->t)) {
                if (j > 31) {
                    Error("internal error: struct with too many reference fields",
                          node_context.back()->line);
                    break;
                }
                bits |= 1 << j;
            }
        }
        return bits;
    }

    int BitMaskForRefStruct(TypeRef type) {
        assert(type->t == V_STRUCT_R);
        return ComputeBitMask(*type->udt);
    }

    bool ShouldDec(TypeLT typelt) {
        return IsRefNil(typelt.type->t) && typelt.lt == LT_KEEP;
    }

    // The node a value comes from, past the lifetime conversion that gives it up rather than
    // takes it, which a value that is not made into one of its own has no use for.
    static const Node *SkipDecrefWrapper(const Node *n) {
        if (auto lt = Is<ToLifetime>(n); lt && lt->decref && !lt->incref) return lt->child;
        return n;
    }

    // The operands of a chain of string concatenations, in order: what GenConcatOp allocates
    // one string for, and GenStringAppendOps appends one by one.
    static void FlattenConcat(const Node *n, node_small_vector &strs) {
        strs.push_back((Node *)n);
        for (;;) {
            auto c = SkipDecrefWrapper(strs[0]);
            auto p = Is<Plus>(c);
            if (p && p->left->exptype->t == V_STRING && p->right->exptype->t == V_STRING) {
                strs.erase(0);
                strs.insert(0, p->right);
                strs.insert(0, p->left);
            } else {
                break;
            }
        }
    }

    int TempStackSize() {
        return (int)tstack_size;
    }

    void PopTemp() {
        assert(tstack_size);
        tstack_size--;
    }

    void PushTemp() {
        tstack_size++;
    }

    // Track the stack the generated code works on: the caller says how many slots the code it is
    // about to emit reads and how many it leaves behind.
    void TrackUseDef(int useslots, int defslots) {
        regso = TempStackSize();
        // What the previous op consumed is gone, whether or not it ever got written.
        for (int i = regso; i < (int)pending.size(); i++) pending[i].expr.clear();
        for (int i = 0; i < useslots; i++) PopTemp();
        for (int i = 0; i < defslots; i++) PushTemp();
    }

    // This must be called explicitly when any values are consumed.
    void TakeTemp(size_t n, bool can_handle_structs) {
        for (; n; n--) {
            auto tlt = temptypestack.back();
            temptypestack.pop_back();
            assert(can_handle_structs || ValWidth(tlt.type) == 1); (void)tlt;
            (void)can_handle_structs;
        }
    }

    // Where the stack top is is something we track statically, so popping a slot needs no code
    // at all, just the bookkeeping.
    void GenPopSlot() { TrackUseDef(1, 0); }

    void GenDup(TypeLT tlt) {
        TrackUseDef(1, 2);
        CopyValue(cb, Slot(0, tlt.type), Slot(1, tlt.type));
        temptypestack.push_back(tlt);
    }

    int Label() { return nlabel++; }

    void EmitLabelDef(int lab) {
        TrackUseDef(0, 0);
        Flush();
        append(cb, "    block", lab, ":;\n");
    }

    void EmitLabelDefs(vector<int> &labs) {
        for (auto lab : labs) {
            EmitLabelDef(lab);
        }
        labs.clear();
    }

    int EmitLabelDefBackwards() {
        auto lab = Label();
        EmitLabelDef(lab);
        return lab;
    }

    void EmitJumpBack(int lab) {
        TrackUseDef(0, 0);
        Flush();
        append(cb, "    goto block", lab, ";\n");
    }

    int EmitJump() {
        auto lab = Label();
        EmitJumpBack(lab);
        return lab;
    }

    // Errors here don't stop code generation (see Lex::Report): nothing runs the result when
    // there were any (see Compile), so a site only has to leave code generation in a state
    // it can finish in. Only for what the typechecker can't check.
    void Error(string_view msg, const Line &ln) {
        parser.lex.Report(msg, &ln);
    }

    string FunName(int idx) {
        switch (idx) {
            case CODEGEN_SPECIAL_FUNCTION_ID_DUMMY: return "fun_dummy";
            case CODEGEN_SPECIAL_FUNCTION_ID_ENTRY: return "fun_entry";
            default: assert(!fun_names[idx].empty()); return fun_names[idx];
        }
    }

    // The helpers that work on a value of any type come one per kind of value rather than
    // taking a Value, so the name of each ends in the kind it is for, and the value goes in as
    // the type the code holds it as, references all as the one type they have in common.
    static string KindName(VKind k) {
        switch (k) {
            case VK_INT: return "Int";
            case VK_FLOAT: return "Float";
            case VK_FUN: return "Fun";
            default: return "Ref";
        }
    }
    string ReadTyped(const Place &p) {
        return IsRefKind(p.k()) ? ReadAs(p, VK_REF) : Read(p);
    }

    // The name of element `i` of a vector, which both the C mirror and the C++ vec give its
    // fields rather than an array, see Prologue.
    static const char *VecField(int i) {
        static const char *fields[] = { "x", "y", "z", "w" };
        return fields[i];
    }

    static string MaskLiteral(uint64_t mask) {
        char buf[32];
        snprintf(buf, sizeof(buf), "0x%llxULL", (unsigned long long)mask);
        return buf;
    }

    // A constant is an expression like any other, so it goes in parentheses when it starts
    // with a sign, which would otherwise pair up with an operator in front of it.
    static string Parenthesized(string lit) {
        return lit[0] == '-' ? cat("(", lit, ")") : lit;
    }

    // An integer as a C literal. The most negative value has no literal of its own, since its
    // negation does not fit.
    static string IntLiteral(int64_t val) {
        auto lit = val == INT64_MIN ? string("(-9223372036854775807LL - 1)")
                 : val == (int)val  ? to_string(val)
                                    : cat(val, "LL");
        return Parenthesized(lit);
    }

    void EmitPushInt(int64_t val) {
        TrackUseDef(0, 1);
        Defer(Slot(0, VK_INT), IntLiteral(val), "");
    }

    // A double as a C literal: its decimal form when that reads back to the same value, the
    // hex float with the decimal alongside otherwise.
    static string FloatLiteral(double f) {
        auto dec = to_string_float(f);
        if (dec.find_first_of(".eE") == string::npos) dec += ".0";
        if (strtod(dec.c_str(), nullptr) == f) return Parenthesized(dec);
        char hex[64];
        snprintf(hex, sizeof hex, "%a", f);
        return Parenthesized(cat(hex, " /* ", dec, " */"));
    }

    void GenFloat(double f) {
        TrackUseDef(0, 1);
        if (isfinite(f)) {
            Defer(Slot(0, VK_FLOAT), FloatLiteral(f), "");
            return;
        }
        // An infinity or a nan has no literal, so this goes thru its bits.
        string hex;
        to_string_hex(hex, (uint64_t)int2float64(f).i);
        if (cpp) {
            Defer(Slot(0, VK_FLOAT), cat("int2float64((int64_t)", hex, "ULL).f"), "");
        } else {
            append(cb, "    { Value _v; _v.ival = (long long)", hex, "ULL; ",
                   WriteText(Slot(0, VK_FLOAT), "_v.fval"), " }\n");
        }
    }

    // What is pushed is a nil of the type, a single slot whatever the type, so this does not
    // look at the slots of a struct.
    void EmitPushNil(TypeRef type) {
        TrackUseDef(0, 1);
        SetNil(cb, SlotVar(regso, RtTypeOf(type)));
    }
};

}  // namespace lobster
