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

struct Borrow : LValContext {
    int refc = 1;  // Number of outstanding borrowed values. While >0 can't assign.
    // Variables that hold one of those borrows speculatively, see SpecIdent::speculative:
    // a write that conflicts flips them to owning rather than being an error.
    small_vector<SpecIdent *, 2> spec_holders;
    Line line { 0, 0 };  // Where the borrow was taken, for errors.
    Borrow(const Node &n) : LValContext(n), line(n.line) {}
    Borrow(const LValContext &lv) : LValContext(lv) {}
    Borrow(const LValContext &lv, const Line &line) : LValContext(lv), line(line) {}
};

enum ConvertFlags {
    CF_NONE        = 0,

    CF_COERCIONS   = 1 << 0,
    CF_UNIFICATION = 1 << 1,
    CF_NUMERIC_NIL = 1 << 2,
    CF_EXACTTYPE   = 1 << 3,
    CF_COVARIANT   = 1 << 4,
};

// Shared state, recursive expression checking, and utilities used across topics, each kept
// with the rest of its group (the Make* conversions, say) even where only one topic uses part
// of it. TypeCheckLocations adds what is known about variables and fields, and each topic
// builds on one of the two as a virtual base. The small virtual interface below lets this code
// call into those without depending on them or moving their implementation into this file.
struct TypeCheckBase {
  private:
    // Only the AST dispatch boundary needs the assembled checker. Its type may
    // remain incomplete here: Node::TypeCheck supplies the per-node dispatch.
    TypeChecker &checker;

  protected:
    TypeChecker &ASTChecker() { return checker; }

  public:
    Parser &parser;
    SymbolTable &st;
    // A promotion from further out (an index into flowstack) that a branch or loop undid.
    typedef pair<size_t, const Demotion *> FlowDemotion;
    struct LoopFlow {
        vector<TypeRef> entry;
        vector<FlowDemotion> demoted;
    };
    struct Scope {
        SubFunction *sf = nullptr;
        const Node *call_context = nullptr;
        // Where in the writes recorded for the function the outermost loop currently being
        // typechecked started, see LoopWroteBefore.
        size_t loop_events_start = 0;
        vector<LoopFlow> loop_flow;
        vector<Member *> scoped_fields;
        size_t flowstack_size = 0;
    };
    vector<Scope> scopes;
    vector<Borrow> borrowstack;
    // The argument lists (of calls, and of returns of several values) whose elements are being
    // typechecked, innermost last: what the elements done so far borrow, a later element may
    // write, see CheckLvalBorrowed.
    vector<List *> arglists;
    vector<SpecIdent *> preferfreestack;
    vector<Define *> definestack;
    set<pair<Line, int64_t>> integer_literal_warnings;
    Query *query;
    bool full_error;
    // Typechecking unreached functions at the end, purely for their errors:
    // code that is only valid in the context of an active caller makes the
    // rest of the function being checked get skipped, see SkipDeadCode.
    bool checking_dead_code = false;
    bool dead_code_skipped = false;
    // The answer to opts.query, once found, see ProcessQuery.
    string query_result;

    TypeCheckBase(TypeChecker &checker, Parser &_p, SymbolTable &_st, const CompileOptions &opts)
        : checker(checker), parser(_p), st(_st), query(opts.query), full_error(opts.full_error) {}

    virtual ~TypeCheckBase() = default;

    // Implemented by TypeCheckConstructors.
    virtual void EnsureUDTChecked(UDT &udt, const Node &errn) = 0;

    // Implemented by TypeCheckFunctions.
    virtual void TypeCheckFunctionDef(SubFunction &sf, const Node &call_context) = 0;

    // Implemented by TypeCheckFunctions.
    virtual void ReplayReturns(const SubFunction *sf, const Node &call_context) = 0;

    // Implemented by TypeCheckCalls.
    virtual TypeRef TypeCheckCall(SubFunction *&csf, List &call_args, size_t reqret, int &vtable_idx,
                                  vector<UnTypeRef> *specializers, bool super) = 0;

    // Implemented by TypeCheckLocations.
    virtual string DemotionNote(const Node &n, SharedField *field = nullptr) = 0;

    // Implemented by TypeCheckQuery.
    virtual bool ProcessQuery() = 0;

    // Expression checking and value/lifetime adjustment.

    // This is the central function thru which all typechecking flows, so we can conveniently
    // match up what the node produces and what the recipient expects.
    void TT(Node *&n, size_t reqret, Lifetime recip, TypeRef parent_bound = {},
            node_small_vector *idents = nullptr) {
        STACK_PROFILE;
        // The rest of a dead function being skipped is not typechecked at all, see
        // SkipDeadCode.
        if (dead_code_skipped) {
            ErrorNode(*n);
            return;
        }
        // Central point from which each node is typechecked.
        n = n->TypeCheck(ASTChecker(), reqret, parent_bound);
        // Check if we need to do any type adjustmenst.
        auto &rtype = n->exptype;
        auto nret = rtype->NumValues();
        if (nret < reqret) {
            if (!n->Terminal(*this)) {
                Error(*n, Q(NiceName(*n)), " returns ", nret, " values, ", reqret, " needed");
                rtype = PadValues(rtype, reqret, n->lt, type_error);
            } else {
                // FIXME: would be better to have a general NORETURN type than patching things up
                // this way.
                rtype = PadValues(rtype, reqret, n->lt, type_any);
            }
        } else if (nret > reqret) {
            for (size_t i = reqret; i < nret; i++) {
                // This value will be dropped.
                DecBorrowers(rtype->GetLifetime(i, n->lt), *n);
                // If this is a LT_KEEP value, codegen will make sure to throw it away.
            }
            switch (reqret) {
                case 0:
                    n->lt = LT_ANY;
                    rtype = type_void;
                    break;
                case 1: {
                    auto typelt = TypeLT { *n, 0 };  // Get from tuple.
                    n->lt = typelt.lt;
                    rtype = typelt.type;
                    break;
                }
                default: {
                    auto nt = st.NewTuple(reqret);
                    nt->tup->assign(rtype->tup->begin(), rtype->tup->begin() + reqret);
                    rtype = nt;
                }
            }
        }
        // Check if we need to do any lifetime adjustments.
        AdjustLifetime(n, recip, idents);
        // Check for queries.
        if (query && query_result.empty()) {
            if ((reqret==0 && n->line == query->qloc) //reqret usually in the end of line
             || (n->line.line > query->qloc.line && n->line.fileidx==query->qloc.fileidx)) { //If above missed
                ProcessQuery();
            }
        }
    }

    void TypeCheckList(List *n, Lifetime lt, TypeRef parent_bound = {}) {
        arglists.push_back(n);
        for (auto &c : n->children) {
            TT(c, 1, lt, parent_bound);
        }
        arglists.pop_back();
    }

    // Diagnostics, error recovery, and current variable identities.

    // Needed for any sids in cloned code.
    void UpdateCurrentSid(SpecIdent *&sid) { sid = sid->Current(); }
    void RevertCurrentSid(SpecIdent *&sid) { sid->Current() = sid; }

    string SignatureWithFreeVars(const SubFunction &sf, set<Ident *> *already_seen) {
        string s = Signature(sf) + " { ";
        size_t j = 0;
        for (auto [i, freevar] : enumerate(sf.freevars)) {
            if (freevar.spec_type->t != V_FUNCTION &&
                !freevar.sid->id->static_constant &&
                (!already_seen || already_seen->find(freevar.sid->id) == already_seen->end())) {
                FormatArg(s, freevar.sid->id->name, j++, freevar.spec_type);
                if (already_seen) already_seen->insert(freevar.sid->id);
            }
        }
        s += "}";
        return s;
    }

    string ArgName(size_t i) {
        switch (i) {
            case 0: return "1st";
            case 1: return "2nd";
            case 2: return "3rd";
            default: return cat(i + 1, "th");
        }
    }

    string_view NiceName(const Node &n) {
        if (auto call = Is<Call>(n))
            if (!call->sf->parent->anonymous)
                return call->sf->parent->name;
        if (auto idr = Is<IdentRef>(n))
            return idr->sid->id->name;
        return n.Name();
    }

    void AddStackTrace(string &err) {
        set<Ident *> already_seen;
        if (!scopes.empty()) {
            size_t scope_count = 0;
            for (auto &scope : reverse(scopes)) {
                if (scope.sf == st.toplevel) continue;
                err += "\n  in " + parser.lex.Location(scope.call_context->line) + ": ";
                if (full_error) {
                    err += SignatureWithFreeVars(*scope.sf, &already_seen);
                    for (auto dl : scope.sf->sbody->children) {
                        if (auto def = Is<Define>(dl)) {
                            if (Is<DefaultVal>(def->child)) continue;  // A pre-decl.
                            for (auto p : def->tsids) {
                                err += ", " + p.sid->id->name + ":" + TypeName(p.sid->type);
                            }
                        }
                    }
                } else {
                    err += Signature(*scope.sf);
                    scope_count++;
                    if (scope_count == 5 && scopes.size() > 7) {
                        err += cat("\n  (", scopes.size() - scope_count,
                            " more functions omitted, --full-error to see more) ");
                        break;
                    }
                }
            }
        }
    }

    // Errors don't stop typechecking: they get collected (see Lex::Report), and the code that
    // hit one carries on, with the error type (see V_ERROR) standing in for whatever it could
    // not type. That type converts to and from anything (see UnConvertsTo), and an error
    // about a node whose own type or whose children's types involve it is not reported, since
    // it is almost certainly a consequence of the one already reported rather than an error
    // of its own. Compilation stops after typechecking if there were any, so nothing
    // downstream sees the placeholders. The helpers below this are the common recovery
    // patterns; the rule for every error site is that whatever it leaves behind is
    // well-formed for the rest of the typechecker (no null types, no borrows left
    // outstanding, no unbalanced stacks), even if it is nonsense.

    // Whether an error about this node would be a consequence of one already reported.
    bool Consequential(const Node &n) {
        if (!n.exptype.Null() && n.exptype->IsError()) return true;
        auto ch = const_cast<Node &>(n).Children();
        for (size_t i = 0; i < n.Arity(); i++) {
            if (!ch[i]->exptype.Null() && ch[i]->exptype->IsError()) return true;
        }
        return false;
    }

    // An error that does not depend on the types of the node's children (a wrong number of
    // arguments, say), so is one of its own even when one of those is erroneous.
    template<typename... Ts> void ErrorAlways(const Node &n, const Ts &...args) {
        if (dead_code_skipped) return;
        auto err = cat(args...);
        if (full_error || parser.lex.num_errors == 0) AddStackTrace(err);
        parser.lex.Report(err, &n.line);
    }

    template<typename... Ts> void Error(const Node &n, const Ts &...args) {
        if (!Consequential(n)) ErrorAlways(n, args...);
    }

    // The dead function being checked (see TypeCheckDeadCode) turns out to need an active
    // caller: not an error, since it may well have one when called. `n` gets the error type,
    // and so does everything typechecked after it in that function (see TT), without errors
    // (see ErrorAlways) or effects on the types of live code (see UnifyVar), as if the check
    // had been abandoned there.
    Node *SkipDeadCode(Node &n) {
        assert(checking_dead_code);
        dead_code_skipped = true;
        return ErrorNode(n);
    }

    // Stands in for a node whose typechecking could not be completed: it has the error type,
    // and, like any non-reference value, no lifetime the recipient of its value needs to do
    // anything about. Borrows its children still hold are for the caller to give up, since
    // which of those a node has already released differs per node.
    Node *ErrorNode(Node &n) {
        n.exptype = type_error;
        n.lt = LT_ANY;
        return &n;
    }

    template<typename... Ts> Node *ErrorNode(Node &n, const Ts &...args) {
        Error(n, args...);
        return ErrorNode(n);
    }

    // Gives up the borrows the children of `n` hold, for a node that stops before doing
    // whatever it normally does with them.
    void ReleaseChildren(Node &n) {
        for (size_t i = 0; i < n.Arity(); i++) DecBorrowers(n.Children()[i]->lt, n);
    }

    // `reqret` values for something that produced fewer: what it did produce, padded with
    // `with`.
    TypeRef PadValues(TypeRef type, size_t reqret, Lifetime lt, TypeRef with) {
        auto nret = type->NumValues();
        if (nret >= reqret) return type;
        if (reqret == 1) return with;
        auto nt = st.NewTuple(reqret);
        for (size_t i = 0; i < reqret; i++) {
            if (i < nret) nt->Set(i, type->Get(i), type->GetLifetime(i, lt));
            else nt->Set(i, &*with, LT_ANY);
        }
        return nt;
    }

    // The return type of a function that could not be typechecked, for its callers.
    TypeRef ErrorReturnType(size_t reqret) {
        return PadValues(type_void, reqret, LT_KEEP, type_error);
    }

    template<typename... Ts> void Warn(const Node &n, const Ts &...args) {
        auto err = cat(args...);
        if (full_error) AddStackTrace(err);
        parser.lex.Warn(err, &n.line);
    }

    // `value` is what has type `got`, when that is not `n` itself. `note` says why the type
    // given doesn't qualify, where the requirement alone doesn't make that apparent.
    void RequiresError(string_view required, TypeRef got, const Node &n, string_view argname = "",
                       string_view context = "", const Node *value = nullptr,
                       string_view note = "") {
        Error(n, Q(context.size() ? context : NiceName(n)), " ",
                 (argname.size() ? "(" + argname + " argument) " : ""),
                 "requires type ", Q(required), ", got ", Q(TypeName(got)),
                 DemotionNote(value ? *value : n),
                 (note.size() ? cat("\n  ", note) : ""));
    }

    void NoStruct(const Node &n, string_view context) {
        if (IsStruct(n.exptype->t)) Error(n, "struct value cannot be used in ", Q(context));
    }

    // Type compatibility, unification, and coercions.

    // FIXME: unifying UnTypeRef ideally should be fixed in the callers.
    // Binds the type variable `hasvar` to `type`. Returns false, binding nothing, when the
    // variable is the element type of a nil (var_parent == V_NIL) and `type` cannot be
    // nilable: a type such as `int?` never exists, so the caller reports a mismatch.
    bool UnifyVar(UnTypeRef type, UnTypeRef hasvar, ValueType var_parent) {
        // Typically Type is const, but this is the one place we overwrite them.
        // Type objects that are V_VAR are seperate heap instances, so overwriting them has no
        // side-effects on non-V_VAR Type instances.
        assert(hasvar->t == V_VAR);
        // The variable may belong to live code, which a skipped dead function must not touch.
        if (dead_code_skipped) return true;
        if (var_parent == V_NIL) {
            // A nil type binds the element to that nil's element, rather than making a nil
            // of a nil.
            if (type->t == V_NIL) type = type->sub;
            if (type->t != V_VAR && type->t != V_TYPEVAR && !st.IsNillable(type)) return false;
        }
        if (type->t == V_VAR) {
            // If these two are already part of the same cycle, don't do the swap, which
            // could disconnect the cycle!
            auto v = hasvar;
            do {  // Loop thru all vars in unification cycle.
                if (&*v == &*type) return true;  // Same cycle.
                v = v->sub;
            } while (&*v != &*hasvar);  // Force TypeRef pointer comparison.
            // Combine two cyclic linked lists.. elegant!
            swap((Type *&)hasvar->sub, (Type *&)type->sub);
        } else {
            auto v = hasvar;
            do { // Loop thru all vars in unification cycle.
                auto next = v->sub;
                *(Type *)&*v = *type;  // Overwrite Type struct!
                v = next;
            } while (&*v != &*hasvar);  // Force TypeRef pointer comparison.
            // TODO: A fundamental problem with this overwriting is that we have to rely on
            // the caller to not allow to create non-sensical types, which is only checked
            // above for the callers that know the parent of hasvar.
        }
        return true;
    }

    // The try-and-fail entry point for overload filtering ONLY: there, bound
    // comes from the given (unresolved) types of not yet typechecked
    // overloads, and anything unresolved in it (V_UUDT with unbound type
    // variables, V_TYPEVAR) simply fails to convert, moving matching on to
    // the next strategy. Everything else should use ConvertsTo below, which
    // demands resolved types.
    bool UnConvertsTo(UnTypeRef type, UnTypeRef bound, ConvertFlags cf,
                      ValueType type_parent = V_UNDEFINED, ValueType bound_parent = V_UNDEFINED) {
        if (bound->Equal(*type)) return true;
        if (type->t == V_VAR) {
            if (cf & CF_UNIFICATION) return UnifyVar(bound, type, type_parent);
            return true;
        }
        // The error type converts to and from anything, such that nothing that uses a value
        // an error was reported for reports errors of its own, see Error. A type variable it
        // meets takes it on, so the same holds for whatever that variable flows into.
        if (type->IsError() || bound->IsError()) {
            if (bound->t == V_VAR && (cf & CF_UNIFICATION)) UnifyVar(type, bound, bound_parent);
            return true;
        }
        switch (bound->t) {
            case V_VOID:
                return cf & CF_COERCIONS;
            case V_VAR:
                return (cf & CF_UNIFICATION) && UnifyVar(type, bound, bound_parent);
            case V_FLOAT:
                return type->t == V_INT && (cf & CF_COERCIONS);
            case V_INT:
                return (type->t == V_TYPEID && (cf & CF_COERCIONS)) ||
                       (type->t == V_INT && !bound->e);
            case V_FUNCTION:
                // This just here to accept overloads from "L" in natreg.h (hash).
                return type->t == V_FUNCTION &&
                       !bound->sf &&
                       type->sf->returntype->t == V_VOID &&
                       type->sf->args.empty();
            case V_NIL: {
                auto scf = ConvertFlags(cf & CF_UNIFICATION);
                return (type->t == V_NIL && UnConvertsTo(type->Element(), bound->Element(), scf, V_NIL, V_NIL)) ||
                       (!type->Numeric() && type->t != V_VOID && !IsStruct(type->t) &&
                        UnConvertsTo(type, bound->Element(), scf, type_parent, V_NIL)) ||
                       ((cf & CF_NUMERIC_NIL) && type->Numeric() &&  // For builtins.
                        UnConvertsTo(type, bound->Element(), scf, type_parent, V_NIL));
            }
            case V_VECTOR: {
                // We don't generally allow covariance here unless const (to avoid supertype
                // elements added to subtype vectors) and no contravariance.
                auto cov = cf & CF_COVARIANT ? CF_NONE : CF_EXACTTYPE;
                return type->t == V_VECTOR &&
                       UnConvertsTo(type->Element(), bound->Element(),
                                    ConvertFlags((cf & (CF_UNIFICATION | CF_NUMERIC_NIL)) | cov), V_VECTOR, V_VECTOR);
            }
            case V_CLASS: {
                if (type->t != V_CLASS) return false;
                auto sd = SuperDistance(bound->udt, type->udt);
                return cf & CF_EXACTTYPE ? sd == 0 : sd >= 0;
            }
            case V_STRUCT_R:
            case V_STRUCT_S: {
                if (type->t != bound->t) return false;
                auto sd = SuperDistance(bound->udt, type->udt);
                if (sd < 0) return false;
                // The members of an abstract struct family all have the family's layout, so
                // a value converts to any of its supertypes in it, like an object does (and
                // like for those, only to the exact type as a vector element).
                if (bound->udt->family_root) return cf & CF_EXACTTYPE ? sd == 0 : true;
                // A struct is exactly its fields, so a subclass only converts to a superclass
                // that adds none: losing the extra ones would be a silent truncation.
                return type->udt->sfields.size() == bound->udt->sfields.size();
            }
            case V_TUPLE:
                return type->t == V_TUPLE && ConvertsToTuple(*type->tup, *bound->tup);
            case V_TYPEID:
                if (type->t != V_TYPEID) return false;
                bound = bound->sub;
                type = type->sub;
                // This is minimalistic, but suffices for the current uses of V_TYPEID.
                return bound->t == V_ANY;
            default:
                return false;
        }
    }

    // Can a value of `type` be passed where `bound` is expected, possibly
    // with the given conversions? Both types must be resolved; see UnConvertsTo
    // above for the one context that cannot guarantee that.
    bool ConvertsTo(TypeRef type, TypeRef bound, ConvertFlags cf,
                    ValueType type_parent = V_UNDEFINED, ValueType bound_parent = V_UNDEFINED) {
        assert(bound->t != V_UUDT && bound->t != V_TYPEVAR);
        return UnConvertsTo(type, bound, cf, type_parent, bound_parent);
    }

    bool ConvertsToTuple(const vector<Type::TupleElem> &ttup, const vector<Type::TupleElem> &stup) {
        if (ttup.size() != stup.size()) return false;
        for (auto [i, te] : enumerate(ttup))
            if (!ConvertsTo(te.type, stup[i].type, CF_UNIFICATION, V_TUPLE, V_TUPLE))
                return false;
        return true;
    }

    TypeRef Union(TypeRef at, TypeRef bt, string_view aname, string_view bname,
                  ConvertFlags coercions, const Node *err,
                  ValueType a_parent = V_UNDEFINED, ValueType b_parent = V_UNDEFINED) {
        // The error type absorbs, such that what is computed from a value an error was
        // reported for stays silent as well.
        if (at->IsError()) return at;
        if (bt->IsError()) return bt;
        if (ConvertsTo(at, bt, ConvertFlags(coercions | CF_UNIFICATION), a_parent, b_parent))
            return bt;
        if (ConvertsTo(bt, at, ConvertFlags(coercions | CF_UNIFICATION), b_parent, a_parent))
            return at;
        if (at->t == V_VECTOR && bt->t == V_VECTOR) {
            auto et = Union(at->Element(), bt->Element(), aname, bname, CF_NONE, nullptr, V_VECTOR, V_VECTOR);
            if (et->t == V_UNDEFINED) goto error;
            return st.Wrap(et, V_VECTOR, err ? &err->line : nullptr);
        }
        if (at->t == V_NIL || bt->t == V_NIL) {
            auto ate = at->ElementIfNil();
            auto bte = bt->ElementIfNil();
            if (IsUnBoxedOrStruct(ate->t) || IsUnBoxedOrStruct(bte->t)) goto error;
            auto et = Union(ate, bte, aname, bname, CF_NONE, nullptr,
                        at->t == V_NIL ? V_NIL : a_parent,
                        bt->t == V_NIL ? V_NIL : b_parent);
            if (et->t == V_UNDEFINED) goto error;
            return st.Wrap(et, V_NIL, err ? &err->line : nullptr);
        }
        if (at->t == V_CLASS && bt->t == V_CLASS) {
            auto sstruc = CommonSuperType(at->udt, bt->udt);
            if (sstruc) return &sstruc->thistype;
        }
        if (IsStruct(at->t) && at->t == bt->t && at->udt->family_root &&
            at->udt->family_root == bt->udt->family_root) {
            auto sstruc = CommonSuperType(at->udt, bt->udt);
            if (sstruc) return &sstruc->thistype;
        }
        error:
        if (err) {
            Error(*err, Q(TypeName(at)), " (", aname, ") and ", Q(TypeName(bt)), " (", bname,
                        ") have no common supertype");
            return type_error;
        }
        return type_undefined;
    }

    void MakeString(Node *&a, Lifetime orig_recip) {
        assert(a->exptype->t != V_STRING);
        DecBorrowers(a->lt, *a);
        a = new ToString(a->line, a);
        a->exptype = type_string;
        a->lt = LT_KEEP;
        // Make sure whatever lifetime a was typechecked at is preserved.
        AdjustLifetime(a, orig_recip);
    }

    void MakeBool(Node *&a) {
        DecBorrowers(a->lt, *a);
        if (a->exptype->t == V_INT) return;
        a = new ToBool(a->line, a);
        a->exptype = &st.default_bool_type->thistype;
        a->lt = LT_ANY;
    }

    void MakeInt(Node *&a) {
        auto ti = new ToInt(a->line, a);
        ti->exptype = type_int;
        ti->lt = a->lt;
        a = ti;
    }

    void MakeFloat(Node *&a) {
        auto tf = new ToFloat(a->line, a);
        tf->exptype = type_float;
        tf->lt = a->lt;
        a = tf;
    }

    void MakeLifetime(Node *&n, Lifetime lt, uint64_t incref, uint64_t decref) {
        auto tlt = new ToLifetime(n->line, n, incref, decref);
        tlt->exptype = n->exptype;
        tlt->lt = lt;
        n = tlt;
    }

    void StorageType(TypeRef &type, const Node &context) {
        if (type->HasValueType(V_VOID)) {
            Error(context, "cannot store value of type ", Q(TypeName(type)));
            type = type_error;
        }
    }

    void SubTypeLR(TypeRef bound, BinOp &n) {
        SubType(n.left, bound, "left", n);
        SubType(n.right, bound, "right", n);
    }

    void SubType(Node *&a, TypeRef bound, string_view argname, const Node &context,
                 ConvertFlags extra = CF_NONE) {
        SubType(a, bound, argname, NiceName(context), extra);
    }
    void SubType(Node *&a, TypeRef bound, string_view argname, string_view context,
                 ConvertFlags extra = CF_NONE) {
        // Special path for functions, since they may need type-checking.
        if (bound->t == V_FUNCTION && a->exptype->IsFunction()) {
            // See if these functions can be made compatible. Specialize and typecheck if
            // needed.
            auto sf = a->exptype->sf;
            if (sf == bound->sf) {
                // Early out: this happens when function types pass to function types.
                // Or sometimes even specific function value types passed on to another copy.
                return;
            }
            size_t reqret = 0;
            auto returntype = type_void;
            vector<Arg> no_args;
            auto args = &no_args;
            if (bound->sf) {
                reqret = bound->sf->reqret;
                returntype = bound->sf->returntype;
                args = &bound->sf->args;
                if (!bound->sf->parent->istype)
                    Error(*a,
                            "dynamic function value can only be passed to declared function "
                            "type");
            }
            if (sf->args.size() != args->size())
                goto error;
            for (auto [i, arg] : enumerate(sf->args)) {
                // Specialize to the function type, if requested.
                if (!sf->parent->istype) {
                    if (!sf->typechecked && st.IsGeneric(sf->overload->givenargs[i])) {
                        arg.spec_type = (*args)[i].spec_type;
                    } else {
                        arg.spec_type = st.ResolveTypeVars(sf->overload->givenargs[i], a->line);
                    }
                }
                // Note this has the args in reverse: function args are contravariant.
                if (!ConvertsTo((*args)[i].spec_type, arg.spec_type, CF_UNIFICATION))
                    goto error;
                // Every function value of this type is called the same way, with borrowed
                // arguments (see TypeCheckDynCall), so a parameter that needs a reference
                // of its own copies the argument on entry, see SpecIdent::copy_on_entry.
                if (!sf->parent->istype) {
                    if (!sf->typechecked) {
                        auto owns = ParamOwns(arg, arg.spec_type);
                        arg.sid->lt = owns ? LT_KEEP : LT_BORROW;
                        arg.sid->copy_on_entry = owns;
                    }
                    // Only anonymous functions are values, and a specialization of one
                    // typechecked for a direct call was given these same lifetimes, see
                    // ArgLifetime.
                    assert(IsBorrow(arg.sid->lt) || arg.sid->copy_on_entry);
                }
            }
            if (sf->typechecked) {
                if (sf->reqret != reqret)
                    goto error;
            }
            if (!sf->parent->istype) {
                if (!sf->typechecked) {
                    sf->reqret = reqret;
                }
                sf->isdynamicfunctionvalue = true;
                assert(sf->freevarchecked);  // Must have been pre-specialized.
                // FIXME: shouldn't all functions arriving here already have
                // been cloned by PreSpecializeFunction? Move clone there?
                if (!sf->sbody) sf = CloneFunction(*sf->parent->overloads[0]);
                TypeCheckFunctionDef(*sf, *sf->sbody);
            }
            // FIXME: a function value with (non-global) free variables passed to a declared
            // function type should be an error, since the value may outlive them, but plenty
            // of code passes one in a non-escaping way (a function type on a chain of function
            // args whose only use is a dyncall down the line), which would need detecting first.
            // Covariant again.
            if (sf->returntype->NumValues() != returntype->NumValues() ||
                !ConvertsTo(sf->returntype, returntype, CF_UNIFICATION))
                goto error;
            // Parser only parses one ret type for function types.
            assert(returntype->NumValues() <= 1);
            return;
        }
        // TODO: generalize this into check if `a` is un-aliased.
        if (Is<ObjectConstructor>(a) || Is<VectorConstructor>(a)) {
            extra = ConvertFlags(CF_COVARIANT | extra);
        }
        if (ConvertsTo(a->exptype, bound, ConvertFlags(CF_UNIFICATION | extra))) {
            return;
        }
        // Here follow the cases that needs explicit coercion code to be made compatible.
        switch (bound->t) {
            case V_FLOAT:
                if (a->exptype->t == V_INT) {
                    if (auto ic = Is<IntConstant>(a)) {
                        auto this_warn = std::make_pair(ic->line, ic->integer);
                        if (integer_literal_warnings.insert(this_warn).second) {
                            parser.WarnAt(a, "integer literal (", ic->integer,
                                          ") where float expected");
                        }
                    }
                    MakeFloat(a);
                    return;
                }
                break;
            case V_INT:
                if (a->exptype->t == V_TYPEID) {
                    MakeInt(a);
                    return;
                }
                break;
            default:
                ;
        }
        error:
        RequiresError(TypeName(bound), a->exptype, *a, argname, context);
    }

    void SubTypeT(TypeRef type, TypeRef bound, const Node &n, string_view argname,
                  string_view context = {}) {
        if (!ConvertsTo(type, bound, CF_UNIFICATION))
            RequiresError(TypeName(bound), type, n, argname, context);
    }

    // Shared operator lookup and function specialization utilities.

    Node *OperatorOverload(Node &n) {
        TT(n.Children()[0], 1, LT_ANY);
        // If this is not an overload, we return nullptr and the first child
        // will already have been typechecked with LT_BORROW.
        auto no_overload = [&]() {
            AdjustLifetime(n.Children()[0], LT_BORROW);
            return nullptr;
        };
        auto child1 = n.Children()[0];
        auto ctype = child1->exptype;
        if (!IsUDT(ctype->t)) return no_overload();
        auto opname = TName(T_OPERATOR) + (n.Name() == "indexing operation" ? "[]" : n.Name());
        auto it = st.operators.find(opname);
        if (it == st.operators.end()) return no_overload();
        auto f = it->second->first;
        while (f->nargs() != n.Arity()) {
            f = f->sibf;
            if (!f) return no_overload();
        }
        // Here all we care about is to see that ctype has some kind of operator
        // overload specified for it, so that TypeCheckCall can do the actual work of finding
        // the correct overload (and we don't have to duplicate that here).
        for (auto [i, ov] : enumerate(f->overloads)) {
            auto atype = ov->sf->overload->givenargs[0];
            if ((atype->t == V_UUDT &&
                 (DistanceToSpecializedSuper(atype->spec_udt->gudt, ctype->udt) >= 0 ||
                  (ctype->udt->g.is_abstract && DistanceFromSpecializedSub(ctype->udt, atype->spec_udt->gudt) >= 0))) ||
                UnConvertsTo(ctype, atype, CF_NONE) ||
                (ctype->udt->g.is_abstract && UnConvertsTo(atype, ctype, CF_NONE))) {
                goto have_candidates;
            }
        }
        return no_overload();
        have_candidates:
        if (n.SideEffect() && IsStruct(ctype->t))
            Error(n, "struct types can\'t model side effecting overloaded operators");
        if (n.Arity() > 1) TT(n.Children()[1], 1, LT_ANY);
        auto c = new Call(n.line, f->overloads[0]->sf);
        c->children.append(n.Children(), n.Arity());
        n.ClearChildren();
        int vtable_idx = -1;
        vector<UnTypeRef> specializers;
        c->exptype = TypeCheckCall(c->sf, *c, 1, vtable_idx, &specializers, false);
        c->lt = LT_KEEP;
        delete &n;
        return c;
    }

    SubFunction *CloneFunction(Overload &ov) {
        auto esf = ov.sf;
        LOG_DEBUG("cloning: ", esf->parent->name);
        auto sbody = AssertIs<Block>(ov.gbody->Clone(true));
        if (!esf->sbody) {
            esf->sbody = sbody;
            return esf;
        }
        auto sf = st.CreateSubFunction();
        sf->SetParent(*esf->parent, ov);
        sf->sbody = sbody;
        // Any changes here make sure this corresponds what happens in Inline() in the
        // optimizer.
        st.CloneIds(*sf, *esf);
        sf->freevarchecked = true;
        sf->returngiventype = esf->returngiventype;
        sf->returntype = esf->returntype;
        sf->method_of = esf->method_of;
        sf->generics = esf->generics;
        sf->overload->givenargs = esf->overload->givenargs;
        sf->returned_thru_to_max = -1;
        sf->attributes = esf->attributes;
        sf->lexical_parent = esf->lexical_parent;
        sf->overload = esf->overload;
        return sf;
    }

    // Whether a parameter needs a reference of its own rather than borrowing the argument:
    // one the function assigns to (with a borrowed value at refc==1 the overwrite would free
    // what the caller still uses, and in general the overwrites can be too complicated to
    // track, loops etc.), and a struct of references, which is several such values.
    // FIXME: the latter is conservative, since a struct that never gets assigned to would
    // not need this. But where we track assignment in the parser we have no idea of types,
    // and here we don't know if it is assigned to, so that would require some new kind of
    // tracking this info.
    bool ParamOwns(const Arg &arg, TypeRef argtype) {
        return !arg.sid->id->single_assignment || argtype->t == V_STRUCT_R;
    }

    // Dispatch tables, shared by dynamic dispatch and switches on types.

    // Adds an entry to the dispatch table of each class in root.subudts (which includes
    // `root`), at the first index free in all of them, with `entry(i)` giving the one for
    // root.subudts[i]. Returns the entry of `root`, which stands for the whole dispatch.
    template<typename F> DispatchEntry *AddDispatchEntries(UDT &root, F entry) {
        int vtable_idx = -1;
        for (auto udt : root.subudts) {
            vtable_idx = std::max(vtable_idx, (int)udt->dispatch_table.size());
        }
        assert(vtable_idx >= 0);
        for (auto [i, udt] : enumerate(root.subudts)) {
            auto &dt = udt->dispatch_table;
            assert((int)dt.size() <= vtable_idx);  // Double entry.
            // FIXME: this is not great, wasting space, but only way to do this
            // on the fly without tracking lots of things.
            while ((int)dt.size() < vtable_idx)
                dt.push_back(make_unique<DispatchEntry>(DispatchEntry{}));
            dt.push_back(make_unique<DispatchEntry>(entry(i)));
        }
        auto de = root.dispatch_table[vtable_idx].get();
        de->dispatch_root = &root;
        de->subudts_size = root.subudts.size();
        de->vtable_idx = vtable_idx;
        return de;
    }

    // Borrow counting and lifetime adjustment.

    const Coercion *IsCoercion(const Node *n) {
        return dynamic_cast<const Coercion *>(n);
    }

    const Node *SkipCoercions(const Node *n) {
        auto c = IsCoercion(n);
        return c ? SkipCoercions(c->child) : n;
    }

    Lifetime LvalueLifetime(const Node &lval, bool deref) {
        if (auto idr = Is<IdentRef>(lval)) return idr->sid->lt;
        if (deref) {
            if (auto dot = Is<Dot>(lval)) return LvalueLifetime(*dot->child, deref);
            if (auto idx = Is<Indexing>(lval)) return LvalueLifetime(*idx->object, deref);
        }
        return LT_KEEP;
    }

    void Borrowers(Lifetime lt, int change, const Node &context) {
        if (lt < 0) return;
        auto &b = borrowstack[lt];
        // NOTE: b may refer to a type that is not a ref, so we can't assert on IsRefNilVar,
        // see PushBorrow.
        b.refc += change;
        LOG_DEBUG("borrow ", change, ": ", b.sid->id->name, " in ", NiceName(context),
               ", ", b.refc, " remain");
        // FIXME: this should really just not be possible, but hard to guarantee.
        if (b.refc < 0) {
            ErrorAlways(context, Q(b.sid->id->name), " used in ", Q(NiceName(context)),
                                 " without being borrowed");
            b.refc = 0;
        }
    }

    void IncBorrowers(Lifetime lt, const Node &context) { Borrowers(lt, 1, context); }
    void DecBorrowers(Lifetime lt, const Node &context) { Borrowers(lt, -1, context); }

    void ModifyLifetime(Node *n, size_t i, Lifetime lt) {
        if (n->lt == LT_MULTIPLE) {
            n->exptype->Set(i, n->exptype->Get(i), lt);
        } else {
            n->lt = lt;
        }
    }

    void AdjustLifetime(Node *&n, Lifetime recip, const node_small_vector *idents = nullptr) {
        assert(n->lt != LT_UNDEF && recip != LT_UNDEF);
        uint64_t incref = 0, decref = 0;
        auto rtype = n->exptype;
        for (size_t i = 0; i < rtype->NumValues(); i++) {
            assert (n->lt != LT_MULTIPLE || rtype->t == V_TUPLE);
            auto givenlt = rtype->GetLifetime(i, n->lt);
            auto given = LifetimeType(givenlt);
            if (idents) recip = LvalueLifetime(*(*idents)[i], false);  // FIXME: overwrite var?
            recip = LifetimeType(recip);
            if (given != recip) {
                auto rtt = rtype->Get(i)->t;
                // Sadly, if it a V_VAR we have to be conservate and assume it may become a ref.
                if (IsRefNilVar(rtt)) {
                    // Special action required.
                    if (i >= sizeof(incref) * 8) ErrorAlways(*n, "too many return values");
                    if (given == LT_BORROW && recip == LT_KEEP) {
                        incref |= 1LL << i;
                        DecBorrowers(givenlt, *n);
                    } else if (given == LT_KEEP && recip == LT_BORROW) {
                        decref |= 1LL << i;
                    } else if (given == LT_ANY) {
                        // These are compatible with whatever recip wants.
                    } else if (recip == LT_ANY) {
                        // recipient doesn't care, e.g. void statement.
                    } else {
                        assert(false);
                    }
                } else {
                    if (given == LT_BORROW) {
                        // This is a scalar that depends on a borrowed value, but the recipient
                        // doesn't care.
                        ModifyLifetime(n, i, LT_ANY);  // Avoid it travelling any further.
                        DecBorrowers(givenlt, *n);
                    }
                }
                if (given == LT_ANY) {
                    // Fill in desired lifetime, for consistency.
                    ModifyLifetime(n, i, recip);
                }
            }
        }
        if (incref || decref) {
            LOG_DEBUG("lifetime adjust for ", NiceName(*n), " to ", incref, "/", decref);
            MakeLifetime(n, idents ? LT_MULTIPLE: recip, incref, decref);
        }
    }

    // Builtin type descriptions used by call resolution and checking.

    // TODO: Can't do this transform ahead of time, since it often depends upon the input args.
    TypeRef ActualBuiltinType(TypeRef type, NArgFlags flags, TypeRef etype,
                              const NativeFun *nf, bool test_overloads, size_t argn,
                              const Node &errorn) {
        if (flags & NF_BOOL) {
            assert(type->t == V_INT);
            return &st.default_bool_type->thistype;
        }
        // See if we can promote the type to one of the standard vector types
        // (xy/xyz/xyzw).
        // First unwrap if its inside any vectors:
        size_t num_wrappings = 0;
        auto e = etype;
        auto vt = type;
        for (; vt->t == V_VECTOR; vt = vt->sub) {
            e = !e.Null() && e->t == V_VECTOR ? e->sub : nullptr;
            num_wrappings++;
        }
        if (vt->t != V_STRUCT_NUM) return type;
        if (num_wrappings >= SymbolTable::NUM_VECTOR_TYPE_WRAPPINGS) {
            Error(errorn, "INTERNAL: vector type too deeply nested for builtin");
            return type_error;
        }
        // How wide the given struct is, which is the slots it takes rather than the fields it
        // declares, since the slots are what a builtin taking a numeric struct is given. Those
        // differ when a field is a struct of its own, which takes as many slots as it has.
        auto slots = !e.Null() && e->t == V_STRUCT_S ? e->udt->numslots : -1;
        auto flen = vt->ns->flen;
        // Check if we allow any vector length.
        if (flen == -1) flen = slots;
        if (flen >= 1) {
            if (slots == flen && e->udt->sametype->t == vt->ns->t) {
                // Allow any similar vector type, like "color".
                return etype;
            } else {
                // Require xy/xyz/xyzw
                auto nvt = st.GetVectorType(vt->ns->t, num_wrappings, flen);
                if (!nvt.Null())
                    return nvt;
            }
        }
        // We arrive here typically if flen == -1 but we weren't able to derive a length.
        // Sadly, we can't allow to return a vector type instead of a struct, so we error out,
        // and rely on the user to specify more precise types.
        // Not sure if there is a better solution.
        if (!test_overloads) {
            Error(errorn, "cannot deduce struct type for ",
                          (argn ? cat("argument ", argn) : "return value"),
                          " of ", Q(nf->name),
                          (!etype.Null() ? ", got " + Q(TypeName(etype)) : ""));
            return type_error;
        }
        return type;
    }
};

// Shared by the SimpleType implementations, see Node::SimpleType.
// Resolve a given type the way ResolveTypeVars would, for the subset of
// shapes where that can't have side effects: concrete types, wrappings of
// them, and type variables that are already bound (EnsureUDTChecked pushes
// the specialization's bindings). Null for anything else, notably V_UUDT,
// whose resolution can create new specializations.
static TypeRef SimpleResolve(UnTypeRef t, SymbolTable &st) {
    switch (t->t) {
        case V_NIL:
        case V_VECTOR: {
            auto e = SimpleResolve({ t->Element() }, st);
            if (e.Null()) return nullptr;
            if (t->t == V_NIL && !st.IsNillable(e)) return nullptr;  // TT reports this error.
            if (&*e == t->Element()) return TypeRef(t.get());
            return st.Wrap(e, t->t);
        }
        case V_TYPEVAR: {
            for (auto &bvec : reverse(st.bound_typevars_stack)) {
                for (auto &gtv : bvec) {
                    if (gtv.tv == t->tv && !gtv.type.Null()) return gtv.type;
                }
            }
            return nullptr;
        }
        case V_UUDT:
        case V_VAR:
        case V_ANY:
        case V_UNDEFINED:
            return nullptr;
        default:
            return TypeRef(t.get());
    }
}

}  // namespace lobster
