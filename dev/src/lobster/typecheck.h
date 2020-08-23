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

struct LValContext {
    // For now, only: ident ( . field )*.
    const SpecIdent *sid;
    vector<SharedField *> derefs;
    LValContext(SpecIdent *sid) : sid(sid) {}
    LValContext(const Node &n) {
        auto t = &n;
        while (auto dot = Is<Dot>(t)) {
            derefs.insert(derefs.begin(), dot->fld);
            t = dot->children[0];
        }
        auto idr = Is<IdentRef>(t);
        sid = idr ? idr->sid : nullptr;
    }
    bool IsValid() { return sid; }
    bool DerefsEqual(const LValContext &o) {
        if (derefs.size() != o.derefs.size()) return false;
        for (auto &shf : derefs) if (shf != o.derefs[&shf - &derefs[0]]) return false;
        return true;
    }
    bool IsPrefix(const LValContext &o) {
        if (sid != o.sid || derefs.size() < o.derefs.size()) return false;
        for (auto &shf : o.derefs) if (shf != derefs[&shf - &o.derefs[0]]) return false;
        return true;
    }
    string Name() {
        auto s = sid ? sid->id->name : "<invalid>";
        for (auto &shf : derefs) {
            s += ".";
            s += shf->name;
        }
        return s;
    }
};

struct FlowItem : LValContext {
    TypeRef old, now;
    FlowItem(const Node &n, TypeRef type) : LValContext(n), old(n.exptype), now(type) {}
    FlowItem(SpecIdent *sid, TypeRef old, TypeRef now) : LValContext(sid), old(old), now(now) {}
};

struct Borrow : LValContext {
    int refc = 1;  // Number of outstanding borrowed values. While >0 can't assign.
    Borrow(const Node &n) : LValContext(n) {}
};

struct TypeChecker {
    Parser &parser;
    SymbolTable &st;
    struct Scope { SubFunction *sf; const Node *call_context; int loop_count = 0; };
    vector<Scope> scopes, named_scopes;
    vector<FlowItem> flowstack;
    vector<Borrow> borrowstack;

    TypeChecker(Parser &_p, SymbolTable &_st, size_t retreq) : parser(_p), st(_st) {
        // FIXME: this is unfriendly.
        if (!st.RegisterDefaultTypes())
            TypeError("cannot find standard types (from stdtype.lobster)", *parser.root);
        for (auto sf : st.subfunctiontable) {
            // TODO: This is not great, it mostly just substitutes SpecUDTs with no specialization for
            // the UDT such that function overload selection can work. We could also not do this
            // here, and instead improve that code.
            for (auto [i, arg] : enumerate(sf->args)) {
                auto type = sf->giventypes[i].utr;
                if (type->t == V_UUDT &&
                    (type->spec_udt->specializers.empty() || !type->spec_udt->is_generic)) {
                    arg.type = &type->spec_udt->udt->thistype;
                }
            }
        }
        AssertIs<Call>(parser.root)->sf->reqret = retreq;
        TT(parser.root, retreq, LT_KEEP);
        CleanUpFlow(0);
        assert(borrowstack.empty());
        assert(scopes.empty());
        assert(named_scopes.empty());
        Stats();
    }

    // Needed for any sids in cloned code.
    void UpdateCurrentSid(SpecIdent *&sid) { sid = sid->Current(); }
    void RevertCurrentSid(SpecIdent *&sid) { sid->Current() = sid; }

    void PromoteStructIdx(TypeRef &type, const UDT *olds, const UDT &news) {
        auto u = type;
        while (u->Wrapped()) u = u->Element();
        if (IsUDT(u->t) && u->udt == olds) type = PromoteStructIdxRec(type, news);
    }

    TypeRef PromoteStructIdxRec(TypeRef type, const UDT &news) {
        return type->Wrapped()
            ? st.Wrap(PromoteStructIdxRec(type->sub, news), type->t)
            : &news.thistype;
    }

    string SignatureWithFreeVars(const SubFunction &sf, set<Ident *> *already_seen) {
        string s = Signature(sf) + " { ";
        size_t j = 0;
        for (auto [i, freevar] : enumerate(sf.freevars)) {
            if (freevar.type->t != V_FUNCTION &&
                !freevar.sid->id->static_constant &&
                (!already_seen || already_seen->find(freevar.sid->id) == already_seen->end())) {
                FormatArg(s, freevar.sid->id->name, j++, freevar.type);
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

    void TypeError(string_view required, TypeRef got, const Node &n, string_view argname = "",
                   string_view context = "") {
        TypeError(cat("\"", (context.size() ? context : NiceName(n)), "\" ",
                      (argname.size() ? "(" + argname + " argument) " : ""),
                      "requires type \"", required, "\", got: \"", TypeName(got), "\""), n);
    }

    void TypeError(string err, const Node &n) {
        set<Ident *> already_seen;
        if (!scopes.empty())
        for (auto scope : reverse(scopes)) {
            if (scope.sf == st.toplevel) continue;
            err += "\n  in " + parser.lex.Location(scope.call_context->line) + ": ";
            err += SignatureWithFreeVars(*scope.sf, &already_seen);
            for (auto dl : scope.sf->body->children) {
                if (auto def = Is<Define>(dl)) {
                    for (auto p : def->sids) {
                        err += ", " + p.first->id->name + ":" + TypeName(p.first->type);
                    }
                }
            }
        }
        parser.Error(err, &n);
    }

    void NoStruct(const Node &n, string_view context) {
        if (IsStruct(n.exptype->t)) TypeError("struct value cannot be used in: " + context, n);
    }

    void NatCallError(string_view errstr, const NativeFun *nf, const NativeCall &callnode) {
        auto err = errstr + nf->name;
        err += "\n  got:";
        for (auto c : callnode.children) {
            err += " " + TypeName(c->exptype);
        }
        for (auto cnf = nf->first; cnf; cnf = cnf->overloads) {
            err += "\n  overload: " + Signature(*cnf);
        }
        TypeError(err, callnode);
    }

    TypeRef NewTypeVar() {
        auto var = st.NewType();
        *var = Type(V_VAR);
        // Vars store a cycle of all vars its been unified with, starting with itself.
        var->sub = var;
        return var;
    }

    TypeRef NewNilTypeVar() {
        auto nil = st.NewType();
        *nil = Type(V_NIL);
        nil->sub = &*NewTypeVar();
        return nil;
    }

    void UnifyVar(TypeRef type, TypeRef hasvar) {
        // Typically Type is const, but this is the one place we overwrite them.
        // Type objects that are V_VAR are seperate heap instances, so overwriting them has no
        // side-effects on non-V_VAR Type instances.
        assert(hasvar->t == V_VAR);
        if (type->t == V_VAR) {
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
            // the caller to not allow to create non-sensical types, like a nil of nil,
            // but we can't assert that this isn't happening without knowing the parent of hasvar.
        }
    }

    bool ConvertsTo(TypeRef type, TypeRef bound, bool coercions, bool unifications = true,
                    bool allow_numeric_nil = false) {
        if (bound == type) return true;
        if (type->t == V_VAR) {
            if (unifications) UnifyVar(bound, type);
            return true;
        }
        switch (bound->t) {
            case V_VOID:
                return coercions;
            case V_VAR:
                if (unifications) UnifyVar(type, bound);
                return unifications;
            case V_FLOAT:
                return type->t == V_INT && coercions;
            case V_INT:
                return (type->t == V_TYPEID && coercions) || (type->t == V_INT && !bound->e);
            case V_FUNCTION:
                return type->t == V_FUNCTION && !bound->sf;
            case V_NIL:
                return (type->t == V_NIL &&
                        ConvertsTo(type->Element(), bound->Element(), false, unifications)) ||
                       (!type->Numeric() && type->t != V_VOID && !IsStruct(type->t) &&
                        ConvertsTo(type, bound->Element(), false, unifications)) ||
                       (allow_numeric_nil && type->Numeric() &&  // For builtins.
                        ConvertsTo(type, bound->Element(), false, unifications));
            case V_VECTOR:
                return (type->t == V_VECTOR &&
                        ConvertsTo(type->Element(), bound->Element(), false, unifications));
            case V_CLASS:
                return type->t == V_CLASS &&
                       st.SuperDistance(bound->udt, type->udt) >= 0;
            case V_STRUCT_R:
            case V_STRUCT_S:
                return type->t == bound->t && type->udt == bound->udt;
            case V_TUPLE:
                return type->t == V_TUPLE && ConvertsToTuple(*type->tup, *bound->tup);
            case V_TYPEID:
                if (type->t != V_TYPEID) return false;
                bound = bound->sub;
                type = type->sub;
                // FIXME: should bind to vector of var? See typeof return in map.
                if (type->t == V_VAR) return true;
                if (bound->t == V_VECTOR && type->t == V_VECTOR) {
                    bound = bound->sub;
                    type = type->sub;
                }
                // This is minimalistic, but suffices for the current uses of V_TYPEID.
                return bound->t == V_ANY;
            default:
                return false;
        }
    }

    bool ConvertsToTuple(const vector<Type::TupleElem> &ttup, const vector<Type::TupleElem> &stup) {
        if (ttup.size() != stup.size()) return false;
        for (auto [i, te] : enumerate(ttup))
            if (!ConvertsTo(te.type, stup[i].type, false))
                return false;
        return true;
    }

    TypeRef Union(TypeRef at, TypeRef bt, string_view aname, string_view bname, bool coercions,
                  const Node *err) {
        if (ConvertsTo(at, bt, coercions)) return bt;
        if (ConvertsTo(bt, at, coercions)) return at;
        if (at->t == V_VECTOR && bt->t == V_VECTOR) {
            auto et = Union(at->Element(), bt->Element(), aname, bname, false, nullptr);
            if (err && et == type_undefined) goto error;
            return st.Wrap(et, V_VECTOR);
        }
        if (at->t == V_CLASS && bt->t == V_CLASS) {
            auto sstruc = st.CommonSuperType(at->udt, bt->udt);
            if (sstruc) return &sstruc->thistype;
        }
        error:
        if (err) {
            TypeError(cat(TypeName(at), " (", aname, ") and ", TypeName(bt), " (", bname,
                          ") have no common supertype"), *err);
        }
        return type_undefined;
    }

    bool ExactType(TypeRef a, TypeRef b) {
        return a == b;  // Not inlined for documentation purposes.
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

    void StorageType(TypeRef type, const Node &context) {
        if (type->HasValueType(V_VOID)) TypeError("cannot store value of type \"void\"", context);
    }

    void SubTypeLR(TypeRef bound, BinOp &n) {
        SubType(n.left, bound, "left", n);
        SubType(n.right, bound, "right", n);
    }

    void SubType(Node *&a, TypeRef bound, string_view argname, const Node &context) {
        SubType(a, bound, argname, NiceName(context));
    }
    void SubType(Node *&a, TypeRef bound, string_view argname, string_view context) {
        if (ConvertsTo(a->exptype, bound, false)) return;
        switch (bound->t) {
            case V_FLOAT:
                if (a->exptype->t == V_INT) {
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
            case V_FUNCTION:
                if (a->exptype->IsFunction() && bound->sf) {
                    // See if these functions can be made compatible. Specialize and typecheck if
                    // needed.
                    auto sf = a->exptype->sf;
                    auto ss = bound->sf;
                    if (!ss->parent->istype)
                        TypeError("dynamic function value can only be passed to declared "
                                  "function type", *a);
                    if (sf->args.size() != ss->args.size()) break;
                    for (auto [i, arg] : enumerate(sf->args)) {
                        // Specialize to the function type, if requested.
                        if (!sf->typechecked && st.IsGeneric(sf->giventypes[i])) {
                            arg.type = ss->args[i].type;
                        } else {
                            arg.type = ResolveTypeVars(sf->giventypes[i], a);
                        }
                        // Note this has the args in reverse: function args are contravariant.
                        if (!ConvertsTo(ss->args[i].type, arg.type, false))
                            goto error;
                        // This function must be compatible with all other function values that
                        // match this type, so we fix lifetimes to LT_BORROW.
                        // See typechecking of istype calls.
                        arg.sid->lt = LT_BORROW;
                    }
                    if (sf->typechecked) {
                        if (sf->reqret != ss->reqret) goto error;
                    } else {
                        sf->reqret = ss->reqret;
                    }
                    sf->isdynamicfunctionvalue = true;
                    TypeCheckFunctionDef(*sf, *sf->body);
                    // Covariant again.
                    if (sf->returntype->NumValues() != ss->returntype->NumValues() ||
                        !ConvertsTo(sf->returntype, ss->returntype, false))
                            break;
                    // Parser only parses one ret type for function types.
                    assert(ss->returntype->NumValues() <= 1);
                    return;
                }
                break;
            default:
                ;
        }
        error:
        TypeError(TypeName(bound), a->exptype, *a, argname, context);
    }

    void SubTypeT(TypeRef type, TypeRef bound, const Node &n, string_view argname,
                  string_view context = {}) {
        if (!ConvertsTo(type, bound, false))
            TypeError(TypeName(bound), type, n, argname, context);
    }

    bool MathCheckVector(TypeRef &type, Node *&left, Node *&right) {
        TypeRef ltype = left->exptype;
        TypeRef rtype = right->exptype;
        // Special purpose check for vector * scalar etc.
        if (ltype->t == V_STRUCT_S && rtype->Numeric()) {
            auto etype = ltype->udt->sametype;
            if (etype->Numeric()) {
                if (etype->t == V_INT) {
                    // Don't implicitly convert int vectors to float.
                    if (rtype->t == V_FLOAT) return false;
                } else {
                    if (rtype->t == V_INT) SubType(right, type_float, "right", *right);
                }
                type = &ltype->udt->thistype;
                return true;
            }
        }
        return false;
    }

    const char *MathCheck(TypeRef &type, BinOp &n, bool &unionchecked,
                          bool typechangeallowed) {
        if (type->Numeric() || IsUDT(type->t))
            return nullptr;
        if (MathCheckVector(type, n.left, n.right)) {
            unionchecked = true;
            return nullptr;
        }
        if (!Is<Plus>(&n) && !Is<PlusEq>(&n))
            return "numeric/struct";
        // Special purpose checking for + on strings.
        auto ltype = n.left->exptype;
        auto rtype = n.right->exptype;
        if (ltype->t == V_STRING) {
            if (rtype->t != V_STRING) {
                // Anything can be added to a string on the right (because of +=).
                MakeString(n.right, LT_BORROW);
                // Make sure the overal type is string.
                type = type_string;
                unionchecked = true;
            }
            return nullptr;
        } else if (rtype->t == V_STRING && ltype->t != V_STRING && typechangeallowed) {
            // Only if not in a +=
            MakeString(n.left, LT_BORROW);
            type = type_string;
            unionchecked = true;
            return nullptr;
        } else {
            return "numeric/string/struct";
        }
    }

    void MathError(TypeRef &type, BinOp &n, bool &unionchecked, bool typechangeallowed) {
        auto err = MathCheck(type, n, unionchecked, typechangeallowed);
        if (err) {
            if (MathCheck(n.left->exptype, n, unionchecked, typechangeallowed))
                TypeError(err, n.left->exptype, n, "left");
            if (MathCheck(n.right->exptype, n, unionchecked, typechangeallowed))
                TypeError(err, n.right->exptype, n, "right");
            TypeError(cat("can\'t use \"", NiceName(n), "\" on \"", TypeName(n.left->exptype),
                          "\" and \"", TypeName(n.right->exptype), "\""), n);
        }
    }

    void TypeCheckMathOp(BinOp &n) {
        TT(n.left, 1, LT_BORROW);
        TT(n.right, 1, LT_BORROW);
        n.exptype = Union(n.left->exptype, n.right->exptype, "lhs", "rhs", true, nullptr);
        bool unionchecked = false;
        MathError(n.exptype, n, unionchecked, true);
        if (!unionchecked) SubTypeLR(n.exptype, n);
        DecBorrowers(n.left->lt, n);
        DecBorrowers(n.right->lt, n);
        n.lt = LT_KEEP;
    }

    void TypeCheckMathOpEqBit(BinOp &n) {
        TypeCheckBitOp(n);
        CheckLval(n.left);
    }

    void TypeCheckMathOpEq(BinOp &n) {
        TT(n.left, 1, LT_BORROW);
        DecBorrowers(n.left->lt, n);
        TT(n.right, 1, LT_BORROW);
        CheckLval(n.left);
        n.exptype = n.left->exptype;
        if (!MathCheckVector(n.exptype, n.left, n.right)) {
            bool unionchecked = false;
            MathError(n.exptype, n, unionchecked, false);
            if (!unionchecked) SubType(n.right, n.exptype, "right", n);
        }
        // This really does: "left = left op right" the result of op is LT_KEEP, which is
        // implicit, so the left var must be LT_KEEP as well. This is ensured elsewhere because
        // all !single_assignment vars are LT_KEEP.
        assert(!Is<IdentRef>(n.left) || LifetimeType(Is<IdentRef>(n.left)->sid->lt) != LT_BORROW);
        DecBorrowers(n.right->lt, n);
        n.lt = PushBorrow(n.left);
    }

    void TypeCheckComp(BinOp &n) {
        TT(n.left, 1, LT_BORROW);
        TT(n.right, 1, LT_BORROW);
        n.exptype = &st.default_bool_type->thistype;
        auto u = Union(n.left->exptype, n.right->exptype, "lhs", "rhs", true, nullptr);
        if (!u->Numeric() && u->t != V_STRING) {
            if (Is<Equal>(&n) || Is<NotEqual>(&n)) {
                // Comparison with one result, but still by value for structs.
                if (u->t != V_VECTOR && !IsUDT(u->t) && u->t != V_NIL && u->t != V_FUNCTION)
                    TypeError(TypeName(n.left->exptype), n.right->exptype, n, "right-hand side");
            } else {
                // Comparison vector op: vector inputs, vector out.
                if (u->t == V_STRUCT_S && u->udt->sametype->Numeric()) {
                    n.exptype = st.default_int_vector_types[0][u->udt->fields.size()];
                } else if (MathCheckVector(n.exptype, n.left, n.right)) {
                    n.exptype = st.default_int_vector_types[0][n.exptype->udt->fields.size()];
                    // Don't do SubTypeLR since type already verified and `u` not
                    // appropriate anyway.
                    goto out;
                } else {
                    TypeError(n.Name() + " doesn\'t work on " + TypeName(n.left->exptype) +
                              " and " + TypeName(n.right->exptype), n);
                }
            }
        }
        SubTypeLR(u, n);
        out:
        DecBorrowers(n.left->lt, n);
        DecBorrowers(n.right->lt, n);
        n.lt = LT_KEEP;
    }

    void TypeCheckBitOp(BinOp &n) {
        TT(n.left, 1, LT_BORROW);
        TT(n.right, 1, LT_BORROW);
        auto u = Union(n.left->exptype, n.right->exptype, "lhs", "rhs", true, nullptr);
        if (u->t != V_INT) u = type_int;
        SubTypeLR(u, n);
        n.exptype = u;
        DecBorrowers(n.left->lt, n);
        DecBorrowers(n.right->lt, n);
        n.lt = LT_ANY;
    }

    void TypeCheckPlusPlus(Unary &n) {
        TT(n.child, 1, LT_BORROW);
        CheckLval(n.child);
        n.exptype = n.child->exptype;
        if (!n.exptype->Numeric())
            TypeError("numeric", n.exptype, n);
        n.lt = n.child->lt;
    }

    SubFunction *TopScope(vector<Scope> &_scopes) {
        return _scopes.empty() ? nullptr : _scopes.back().sf;
    }

    void TypeCheckUDT(UDT &udt, const Node &errn) {
        for (auto &f : udt.fields) {
            if (f.defaultval && f.giventype.utr->t == V_ANY) {
                // FIXME: would be good to not call TT here generically but instead have some
                // specialized checking, just in case TT has a side effect.
                TT(f.defaultval, 1, LT_ANY);
                DecBorrowers(f.defaultval->lt, errn);
                f.defaultval->lt = LT_UNDEF;
                f.giventype.utr = f.defaultval->exptype;
                f.resolvedtype = f.defaultval->exptype;
            }
        }
        for (auto &g : udt.generics) {
            if (!g.giventype.utr.Null()) {
                g.resolvedtype = ResolveTypeVars(g.giventype, &errn);
            }
        }
        if (!udt.is_generic) {
            // Resolve any typevars in field types ahead of time.
            // We do this here rather than the parser so all named specializations are available.
            // FIXME: bound_typevars_stack does NOT contain any parent nested typevars!
            st.bound_typevars_stack.push_back(&udt.generics);
            for (auto &field : udt.fields) {
                field.resolvedtype = ResolveTypeVars(field.giventype, &errn);
            }
            st.bound_typevars_stack.pop_back();
        }
        if (udt.FullyBound()) {
            // NOTE: all users of sametype will only act on it if it is numeric, since
            // otherwise it would a scalar field to become any without boxing.
            // Much of the implementation relies on these being 2-4 component vectors, so
            // deny this functionality to any other structs.
            if (udt.fields.size() >= 2 && udt.fields.size() <= 4) {
                udt.sametype = udt.fields[0].resolvedtype;
                for (size_t i = 1; i < udt.fields.size(); i++) {
                    // Can't use Union here since it will bind variables, use simplified alternative:
                    if (!ExactType(udt.fields[i].resolvedtype, udt.sametype)) {
                        udt.sametype = type_undefined;
                        break;
                    }
                }
            }
            // Update the type to the correct struct type.
            if (udt.is_struct) {
                for (auto &field : udt.fields) {
                    if (IsRefNil(field.resolvedtype->t)) {
                        udt.hasref = true;
                        break;
                    }
                }
                const_cast<ValueType &>(udt.thistype.t) =
                    udt.hasref ? V_STRUCT_R : V_STRUCT_S;
            }
            if (!udt.first->FullyBound()) {
                // This was specialized from a generic udt, much like with superclass
                // below, promote generic fields.
                // FIXME: much better to require explicit template parameters on the
                // generic definition, e.g. `next:Node<T>` which then automatically
                // gets specialized for `SubNode<int>` etc.
                for (auto &field : udt.fields) {
                    PromoteStructIdx(field.resolvedtype, udt.first, udt);
                }
            }
        }
        if (!udt.given_superclass.Null()) {
            udt.resolved_superclass =
                ResolveTypeVars({ udt.given_superclass }, &errn)->udt;
        }
        if (!udt.given_superclass.Null()) {
            // This points to a generic version of the superclass of this class.
            // See if we can find a matching specialization instead.
            for (auto sti = udt.given_superclass->spec_udt->udt->first; sti; sti = sti->next) {
                for (size_t i = 0; i < sti->fields.size(); i++) {
                    if (sti->fields[i].resolvedtype != udt.fields[i].resolvedtype) {
                        goto fail;
                    }
                }
                {
                    auto nt = st.NewSpecUDT(sti);
                    udt.given_superclass = nt;
                    udt.resolved_superclass = sti;
                    goto done;
                }
                fail:;
            }
            TypeError("can't find specialized superclass for: " + udt.name, errn);
            //udt.given_superclass = nullptr;
            //udt.resolved_superclass = nullptr;
            done:;
        }
        if (udt.resolved_superclass) {
            // If this type has fields inherited from the superclass that refer to the
            // superclass, make it refer to this type instead. There may be corner cases where
            // this is not what you want, but generally you do.
            for (auto &field : make_span(udt.fields.data(),
                udt.resolved_superclass->fields.size())) {
                PromoteStructIdx(field.resolvedtype, udt.resolved_superclass, udt);
            }
        }
        if (udt.FullyBound()) {
            for (auto u = &udt; u; u = u->resolved_superclass) {
                u->subudts.push_back(&udt);
            }
        }
        for (auto [i, f] : enumerate(udt.fields)) {
            // FIXME: this is a temp limitation, remove.
            if (udt.thistype.t == V_STRUCT_R && i &&
                IsRefNil(f.resolvedtype->t) != IsRefNil(udt.fields[0].resolvedtype->t))
                TypeError("structs fields must be either all scalar or all references: " +
                    udt.name, errn);
        }
        if (!udt.ComputeSizes())
            TypeError("structs cannot be self-referential: " + udt.name, errn);
    }

    void RetVal(TypeRef type, SubFunction *sf, const Node &err, bool register_return = true) {
        if (register_return) {
            for (auto isc : reverse(scopes)) {
                if (isc.sf->parent == sf->parent) break;
                // isc.sf is a function in the call chain between the return statement and the
                // function it is returning from. Since we're affecting the return type of the
                // function we're returning from, if it gets specialized but a function along the
                // call chain (isc.sf) does not, we must ensure that return type affects the second
                // specialization.
                // We do this by tracking return types, and replaying them when a function gets
                // reused.
                // A simple test case is in return_from unit test, and recursive_exception is also
                // affected.
                isc.sf->reuse_return_events.push_back({ sf, type });
            }
        }
        sf->num_returns++;
        if (sf != scopes.back().sf) sf->num_returns_non_local++;
        if (sf->returngiventype.utr.Null()) {
            if (sf->reqret) {
                // We can safely generalize the type if needed, though not with coercions.
                sf->returntype = Union(type, sf->returntype, "return expression",
                                        "function return type", false, &err);
            } else {
                // The caller doesn't want return values.
                sf->returntype = type_void;
            }
        }
    }

    void TypeCheckFunctionDef(SubFunction &sf, const Node &call_context) {
        if (sf.typechecked) return;
        STACK_PROFILE;
        LOG_DEBUG("function start: ", SignatureWithFreeVars(sf, nullptr));
        Scope scope;
        scope.sf = &sf;
        scope.call_context = &call_context;
        scopes.push_back(scope);
        //for (auto &ns : named_scopes) LOG_DEBUG("named scope: ", ns.sf->parent->name);
        if (!sf.parent->anonymous) named_scopes.push_back(scope);
        sf.typechecked = true;
        for (auto &arg : sf.args) StorageType(arg.type, call_context);
        for (auto &fv : sf.freevars) UpdateCurrentSid(fv.sid);
        auto backup_vars = [&](vector<Arg> &in, vector<Arg> &backup) {
            for (auto [i, arg] : enumerate(in)) {
                // Need to not overwrite nested/recursive calls. e.g. map(): map(): ..
                backup[i].sid = arg.sid->Current();
                arg.sid->type = arg.type;
                RevertCurrentSid(arg.sid);
            }
        };
        auto backup_args = sf.args; backup_vars(sf.args, backup_args);
        auto backup_locals = sf.locals; backup_vars(sf.locals, backup_locals);
        auto enter_scope = [&](const Arg &var) {
            IncBorrowers(var.sid->lt, call_context);
        };
        for (auto &arg : sf.args) enter_scope(arg);
        for (auto &local : sf.locals) enter_scope(local);
        sf.returntype = sf.reqret
            ? (!sf.returngiventype.utr.Null()
                ? ResolveTypeVars(sf.returngiventype, &call_context)
                : NewTypeVar())
            : type_void;
        auto start_borrowed_vars = borrowstack.size();
        auto start_promoted_vars = flowstack.size();
        sf.body->TypeCheck(*this, 0);
        CleanUpFlow(start_promoted_vars);
        if (!sf.num_returns) {
            if (!sf.returngiventype.utr.Null() && sf.returngiventype.utr->t != V_VOID)
                TypeError("missing return statement", *sf.body->children.back());
            sf.returntype = type_void;
        }
        // Let variables go out of scope in reverse order of declaration.
        auto exit_scope = [&](const Arg &var) {
            DecBorrowers(var.sid->lt, call_context);
        };
        for (auto &local : reverse(sf.locals)) exit_scope(local);
        for (auto &arg : sf.args) exit_scope(arg);  // No order.
        while (borrowstack.size() > start_borrowed_vars) {
            auto &b = borrowstack.back();
            if (b.refc) {
                TypeError(cat("variable \"", b.Name(), "\" still has ", b.refc,
                              " borrowers"), *sf.body->children.back());
            }
            borrowstack.pop_back();
        }
        for (auto &back : backup_args)   RevertCurrentSid(back.sid);
        for (auto &back : backup_locals) RevertCurrentSid(back.sid);
        if (sf.returntype->HasValueType(V_VAR)) {
            // If this function return something with a variable in it, then it likely will get
            // bound by the caller. If the function then gets reused without specialization, it will
            // get the wrong return type, so we force specialization for subsequent calls of this
            // function. FIXME: check in which cases this is typically true, since its expensive
            // if done without reason.
            sf.mustspecialize = true;
        }
        if (!sf.parent->anonymous) named_scopes.pop_back();
        scopes.pop_back();
        LOG_DEBUG("function end ", Signature(sf), " returns ",
                             TypeName(sf.returntype));
    }

    bool FreeVarsSameAsCurrent(const SubFunction &sf, bool prespecialize) {
        for (auto &freevar : sf.freevars) {
            //auto atype = Promote(freevar.id->type);
            if (freevar.sid != freevar.sid->Current() ||
                !ExactType(freevar.type, freevar.sid->Current()->type)) {
                (void)prespecialize;
                assert(prespecialize ||
                       freevar.sid == freevar.sid->Current() ||
                       (freevar.sid && freevar.sid->Current()));
                return false;
            }
            //if (atype->t == V_FUNCTION) return false;
        }
        return true;
    }

    SubFunction *CloneFunction(SubFunction *csf, int i) {
        LOG_DEBUG("cloning: ", csf->parent->name);
        auto sf = st.CreateSubFunction();
        sf->SetParent(*csf->parent, csf->parent->overloads[i]);
        // Any changes here make sure this corresponds what happens in Inline() in the optimizer.
        st.CloneIds(*sf, *csf);
        sf->body = AssertIs<Block>(csf->body->Clone());
        sf->freevarchecked = true;
        sf->returngiventype = csf->returngiventype;
        sf->returntype = csf->returntype;
        sf->method_of = csf->method_of;
        sf->generics = csf->generics;
        sf->giventypes = csf->giventypes;
        return sf;
    }

    // See if returns produced by an existing specialization are compatible with our current
    // context of functions.
    bool CompatibleReturns(const SubFunction &ssf) {
        for (auto re : ssf.reuse_return_events) {
            auto sf = re.first;
            for (auto isc : reverse(scopes)) {
                if (isc.sf->parent == sf->parent) {
                    if (isc.sf->reqret != sf->reqret) return false;
                    goto found;
                }
            }
            return false;  // Function not in context.
            found:;
        }
        return true;
    }

    void CheckReturnPast(SubFunction *sf, const SubFunction *sf_to, const Node &context) {
        // Special case for returning out of top level, which is always allowed.
        if (sf_to == st.toplevel) return;
        if (sf->isdynamicfunctionvalue) {
            // This is because the function has been typechecked against one context, but
            // can be called again in a different context that does not have the same callers.
            TypeError("cannot return out of dynamic function value (" + sf_to->parent->name +
                      " not found on the callstack)", context);
        }
        // Marke any functions we may be returning thru as such.
        sf->returned_thru = true;
    }

    TypeRef TypeCheckMatchingCall(SubFunction *sf, List &call_args, bool static_dispatch,
                                  bool first_dynamic) {
        STACK_PROFILE;
        // Here we have a SubFunction witch matching specialized types.
        sf->numcallers++;
        Function &f = *sf->parent;
        if (!f.istype) TypeCheckFunctionDef(*sf, call_args);
        // Finally check all args. We do this after checking the function
        // definition, since SubType below can cause specializations of the current function
        // to be typechecked with strongly typed function value arguments.
        for (auto [i, c] : enumerate(call_args.children)) {
            if (i < f.nargs()) /* see below */ {
                auto &arg = sf->args[i];
                if (static_dispatch || first_dynamic) {
                    // Check a dynamic dispatch only for the first case, and then skip
                    // checking the first arg.
                    if (static_dispatch || i)
                        SubType(c, arg.type, ArgName(i), f.name);
                    AdjustLifetime(c, arg.sid->lt);
                    // We really don't want to specialize functions on variables, so we simply
                    // disallow them. This should happen only infrequently.
                    if (arg.type->HasValueType(V_VAR))
                        TypeError(cat("can\'t infer ", ArgName(i), " argument of call to ", f.name),
                                  call_args);
                }
            }
            // This has to happen even to dead args:
            if (static_dispatch || first_dynamic) DecBorrowers(c->lt, call_args);
        }
        for (auto &freevar : sf->freevars) {
            // New freevars may have been added during the function def typecheck above.
            // In case their types differ from the flow-sensitive value at the callsite (here),
            // we want to override them.
            freevar.type = freevar.sid->Current()->type;
        }
        // See if this call is recursive:
        for (auto &sc : scopes) if (sc.sf == sf) {
            sf->isrecursivelycalled = true;
            if (sf->returngiventype.utr.Null())
                TypeError("recursive function must have explicit return type: " + sf->parent->name,
                          call_args);
            break;
        }
        return sf->returntype;
    };

    bool SpecializationIsCompatible(const SubFunction &sf, size_t reqret) {
        return reqret == sf.reqret &&
            FreeVarsSameAsCurrent(sf, false) &&
            CompatibleReturns(sf);
    };

    void ReplayReturns(const SubFunction *sf, const Node &call_context) {
        // Apply effects of return statements for functions being reused, see
        // RetVal above.
        for (auto [isf, type] : sf->reuse_return_events) {
            for (auto isc : reverse(scopes)) {
                if (isc.sf->parent == isf->parent) {
                    // NOTE: will have to re-apply lifetimes as well if we change
                    // from default of LT_KEEP.
                    RetVal(type, isc.sf, call_context, false);
                    // This should in theory not cause an error, since the previous
                    // specialization was also ok with this set of return types.
                    // It could happen though if this specialization has an
                    // additional return statement that was optimized
                    // out in the previous one.
                    SubTypeT(type, isc.sf->returntype, call_context, "",
                        "reused return value");
                    goto destination_found;
                }
                CheckReturnPast(isc.sf, isf, call_context);
            }
            // This error should hopefully be rare, but still possible if this call is in
            // a very different context.
            TypeError(cat("return out of call to \"", sf->parent->name,
                          "\" can\'t find destination ", isf->parent->name),
                      call_context);
            destination_found:;
        }
    };

    TypeRef ResolveTypeVars(UnresolvedTypeRef utype, const Node *errn) {
        auto type = utype.utr;
        switch (type->t) {
            case V_NIL:
            case V_VECTOR: {
                auto nt = ResolveTypeVars({ type->Element() }, errn);
                if (&*nt != &*type->Element()) {
                    return st.Wrap(nt, type->t);
                }
                break;
            }
            case V_TUPLE: {
                vector<TypeRef> types;
                bool same = true;
                for (auto [i, te] : enumerate(*type->tup)) {
                    auto tr = ResolveTypeVars({ te.type }, errn);
                    types.push_back(tr);
                    if (tr != te.type) same = false;
                }
                if (same) break;
                auto nt = st.NewTuple(type->tup->size());
                for (auto [i, te] : enumerate(*type->tup)) {
                    nt->Set(i, &*types[i], te.lt);
                }
                return nt;
            }
            case V_UUDT: {
                if (type->spec_udt->specializers.empty() || !type->spec_udt->is_generic)
                    return &type->spec_udt->udt->thistype;
                vector<TypeRef> types;
                for (auto s : type->spec_udt->specializers) {
                    auto t = ResolveTypeVars({ s }, errn);
                    types.push_back(&*t);
                }
                for (auto udti = type->spec_udt->udt->first; udti; udti = udti->next) {
                    if (udti->FullyBound()) {
                        assert(udti->generics.size() == types.size());
                        for (auto [i, btv] : enumerate(udti->generics)) {
                            if (btv.resolvedtype != types[i]) goto nomatch;
                        }
                        return &udti->thistype;
                        nomatch:;
                    }
                }
                if (!errn)
                    TypeError("internal: need specialization for: " + TypeName(type), *parser.root);
                // No existing specialization found, create a new one.
                auto udt = new UDT("", (int)st.udttable.size(), type->spec_udt->udt->is_struct);
                st.udttable.push_back(udt);
                udt = type->spec_udt->udt->first->CloneInto(udt, type->spec_udt->udt->first->name,
                                                            st.udttable);
                udt->unnamed_specialization = true;
                assert(udt->generics.size() == types.size());
                udt->unspecialized.specializers.clear();
                for (auto [i, g] : enumerate(udt->generics)) {
                    g.Resolve(types[i]);
                    udt->unspecialized.specializers.push_back(&*types[i]);
                }
                TypeCheckUDT(*udt, *errn);
                return &udt->thistype;
            }
            case V_TYPEVAR: {
                for (auto bvec : reverse(st.bound_typevars_stack)) {
                    for (auto &btv : *bvec) {
                        if (btv.tv == type->tv && !btv.resolvedtype.Null()) return btv.resolvedtype;
                    }
                }
                if (errn) TypeError("could not resolve type variable: " + type->tv->name, *errn);
                break;
            }
        }
        return type;
    }

    void UnWrapBoth(TypeRef &otype, TypeRef &atype) {
        while (otype->Wrapped() && otype->t == atype->t) {
            otype = otype->Element();
            atype = atype->Element();
        }
    }

    void BindTypeVar(UnresolvedTypeRef giventype, TypeRef atype,
                     vector<BoundTypeVariable> &generics, UDT *parent = nullptr) {
        auto otype = giventype.utr;
        UnWrapBoth(otype, atype);
        if (otype->t == V_NIL) {
            otype = otype->Element();
        }
        if (otype->t == V_TYPEVAR) {
            for (auto &btv : generics) {
                if (btv.tv == otype->tv && btv.resolvedtype.Null()) {
                    btv.Resolve(atype);
                    break;
                }
            }
        } else if (otype->t == V_UUDT &&
                   IsUDT(atype->t) &&
                   otype->spec_udt->udt->thistype.t == atype->t &&
                   otype->spec_udt->udt != parent &&  // Avoid recursion!
                   otype->spec_udt->udt->first == atype->udt->first) {
            assert(otype->spec_udt->specializers.size() == atype->udt->generics.size());
            for (auto [i, s] : enumerate(otype->spec_udt->specializers)) {
                BindTypeVar({ s }, atype->udt->generics[i].resolvedtype, generics,
                            otype->spec_udt->udt);
            }
        }
    }

    TypeRef TypeCheckCallStatic(SubFunction *&sf, List &call_args, size_t reqret,
                                vector<UnresolvedTypeRef> *specializers, int overload_idx,
                                bool static_dispatch, bool first_dynamic) {
        STACK_PROFILE;
        Function &f = *sf->parent;
        sf = f.overloads[overload_idx];
        // Collect generic type values.
        vector<BoundTypeVariable> generics = sf->generics;
        for (auto &btv : generics) btv.resolvedtype = nullptr;
        if (specializers) {
            if (specializers->size() > generics.size())
                TypeError("too many specializers given", call_args);
            for (auto [i, type] : enumerate(*specializers))
                generics[i].Resolve(ResolveTypeVars(type, &call_args));
        }
        for (auto [i, c] : enumerate(call_args.children)) if (i < f.nargs()) {
            BindTypeVar(sf->giventypes[i], c->exptype, generics);
        }
        for (auto &btv : generics) if (btv.resolvedtype.Null())
            TypeError(cat("Cannot implicitly bind type variable \"", btv.tv->name,
                          "\" in call to \"", f.name, "\" (argument doesn't match?)"),
                      call_args);
        // Check if we need to specialize: generic args, free vars and need of retval
        // must match previous calls.
        auto AllowAnyLifetime = [&](const Arg &arg) {
            return arg.sid->id->single_assignment;
        };
        // Check if any existing specializations match.
        for (sf = f.overloads[overload_idx]; sf; sf = sf->next) {
            if (sf->typechecked && !sf->mustspecialize) {
                // We check against f.nargs because HOFs are allowed to call a function
                // value with more arguments than it needs (if we're called from
                // TypeCheckDynCall). Optimizer always removes these.
                // Note: we compare only lt, since calling with other borrowed sid
                // should be ok to reuse.
                for (auto [i, c] : enumerate(call_args.children)) if (i < f.nargs()) {
                    auto &arg = sf->args[i];
                    if ((IsBorrow(c->lt) != IsBorrow(arg.sid->lt) && AllowAnyLifetime(arg)) ||
                        // TODO: we need this check here because arg type may rely on parent
                        // struct (or function) generic, and thus isn't covered by the checking
                        // of sf->generics below. Can this be done more elegantly?
                        (st.IsGeneric(sf->giventypes[i]) && !ExactType(c->exptype, arg.type)))
                        goto fail;
                }
                for (auto [i, btv] : enumerate(sf->generics)) {
                    if (!ExactType(btv.resolvedtype, generics[i].resolvedtype)) goto fail;
                }
                if (SpecializationIsCompatible(*sf, reqret)) {
                    // This function can be reused.
                    // Make sure to add any freevars this call caused to be
                    // added to its parents also to the current parents, just in case
                    // they're different.
                    LOG_DEBUG("re-using: ", Signature(*sf));
                    for (auto &fv : sf->freevars) CheckFreeVariable(*fv.sid);
                    ReplayReturns(sf, call_args);
                    auto rtype = TypeCheckMatchingCall(sf, call_args, static_dispatch, first_dynamic);
                    if (!sf->isrecursivelycalled) ReplayAssigns(sf);
                    return rtype;
                }
                fail:;
            }
        }
        // No match.
        sf = f.overloads[overload_idx];
        // Specialize existing function, or its clone.
        if (sf->typechecked) sf = CloneFunction(sf, overload_idx);
        // Now specialize.
        sf->reqret = reqret;
        sf->generics = generics;
        if (sf->method_of)
            st.bound_typevars_stack.push_back(&call_args.children[0]->exptype->udt->generics);
        st.bound_typevars_stack.push_back(&sf->generics);
        for (auto [i, c] : enumerate(call_args.children)) if (i < f.nargs()) /* see above */ {
            auto &arg = sf->args[i];
            arg.sid->lt = AllowAnyLifetime(arg) ? c->lt : LT_KEEP;
            arg.type = ResolveTypeVars(sf->giventypes[i], &call_args);
            LOG_DEBUG("arg: ", arg.sid->id->name, ":", TypeName(arg.type));
        }
        // This must be the correct freevar specialization.
        assert(!f.anonymous || sf->freevarchecked);
        assert(!sf->freevars.size());
        LOG_DEBUG("specialization: ", Signature(*sf));
        auto rtype = TypeCheckMatchingCall(sf, call_args, static_dispatch, first_dynamic);
        st.bound_typevars_stack.pop_back();
        if (sf->method_of) st.bound_typevars_stack.pop_back();
        return rtype;
    };

    TypeRef TypeCheckCallDispatch(UDT &dispatch_udt, SubFunction *&csf, List &call_args,
                                  size_t reqret, vector<UnresolvedTypeRef> *specializers,
                                  int &vtable_idx) {
        Function &f = *csf->parent;
        // We must assume the instance may dynamically be different, so go thru vtable.
        // See if we already have a vtable entry for this type of call.
        for (auto [i, disp] : enumerate(dispatch_udt.dispatch)) {
            // FIXME: does this guarantee it find it in the recursive case?
            // TODO: we chould check for a superclass vtable entry also, but chances
            // two levels will be present are low.
            if (disp.sf && disp.sf->method_of == &dispatch_udt && disp.is_dispatch_root &&
                &f == disp.sf->parent && SpecializationIsCompatible(*disp.sf, reqret)) {
                for (auto [i, c] : enumerate(call_args.children)) if (i < f.nargs()) {
                    auto &arg = disp.sf->args[i];
                    if (i && !ConvertsTo(c->exptype, arg.type, false, false))
                        goto fail;
                }
                // If this ever fails, that means new types got added during typechecking..
                // which means we'd just have to create a new vtable entry instead, or somehow
                // avoid the new type.
                assert(disp.subudts_size == dispatch_udt.subudts.size());
                for (auto udt : dispatch_udt.subudts) {
                    // Since all functions were specialized with the same args, they should
                    // all be compatible if the root is.
                    auto sf = udt->dispatch[i].sf;
                    LOG_DEBUG("re-using dyndispatch: ", Signature(*sf));
                    if (sf->typechecked) {
                        // If sf is not typechecked here, it means a function before this in
                        // the list has a recursive call.
                        assert(SpecializationIsCompatible(*sf, reqret));
                        for (auto &fv : sf->freevars) CheckFreeVariable(*fv.sid);
                        ReplayReturns(sf, call_args);
                        ReplayAssigns(sf);
                    }
                }
                // Type check this as if it is a static dispatch to just the root function.
                TypeCheckMatchingCall(csf = disp.sf, call_args, true, false);
                vtable_idx = (int)i;
                return dispatch_udt.dispatch[i].returntype;
            }
            fail:;
        }
        // Must create a new vtable entry.
        // TODO: would be good to search superclass if it has this method also.
        // Probably not super important since dispatching on the "middle" type in a
        // hierarchy will be rare.
        // Find subclasses and max vtable size.
        {
            vector<pair<int, bool>> overload_idxs;
            for (auto sub : dispatch_udt.subudts) {
                int best = -1;
                int bestdist = -1;
                for (auto [i, sf] : enumerate(csf->parent->overloads)) {
                    if (sf->method_of) {
                        auto sdist = st.SuperDistance(sf->method_of, sub->first);
                        if (sdist >= 0 && (best < 0 || bestdist >= sdist)) {
                            if (bestdist == sdist)
                                TypeError(cat("more than implementation of ", f.name,
                                              " applies to ", sub->name), call_args);
                            best = (int)i;
                            bestdist = sdist;
                        }
                    }
                }
                if (best < 0) {
                    if (sub->constructed) {
                        TypeError("no implementation for: " + sub->name + "." + csf->parent->name,
                                  call_args);
                    } else {
                        // This UDT is unused, so we're ok there not being an implementation
                        // for it.. like e.g. an abstract base class.
                    }
                }
                overload_idxs.push_back({ best, bestdist == 0 });
                vtable_idx = max(vtable_idx, (int)sub->dispatch.size());
            }
            // Add functions to all vtables.
            for (auto [i, udt] : enumerate(dispatch_udt.subudts)) {
                auto &dt = udt->dispatch;
                assert((int)dt.size() <= vtable_idx);  // Double entry.
                // FIXME: this is not great, wasting space, but only way to do this
                // on the fly without tracking lots of things.
                while ((int)dt.size() < vtable_idx) dt.push_back({});
                dt.push_back({ overload_idxs[i].first < 0
                                ? nullptr
                                : csf->parent->overloads[overload_idxs[i].first] });
            }
            // FIXME: if any of the overloads below contain recursive calls, it may run into
            // issues finding an existing dispatch above? would be good to guarantee..
            // The fact that in subudts the superclass comes first will help avoid problems
            // in many cases.
            auto de = &dispatch_udt.dispatch[vtable_idx];
            de->is_dispatch_root = true;
            de->returntype = NewTypeVar();
            de->subudts_size = dispatch_udt.subudts.size();
            // Typecheck all the individual functions.
            SubFunction *last_sf = nullptr;
            bool any_recursive = false;
            bool any_returned_thru = false;
            for (auto [i, udt] : enumerate(dispatch_udt.subudts)) {
                auto sf = udt->dispatch[vtable_idx].sf;
                // Missing implementation for unused UDT.
                if (!sf) continue;
                // Skip if it is using a superclass method.
                if (!overload_idxs[i].second) continue;
                if (last_sf) {
                    // FIXME: good to have this check here so it only occurs for functions
                    // participating in the dispatch, but error now appears at the call site!
                    for (auto [j, arg] : enumerate(sf->args)) {
                        if (j && arg.type != last_sf->args[j].type &&
                            !st.IsGeneric(sf->giventypes[j]))
                            TypeError("argument " + to_string(j + 1) + " of \"" + f.name +
                                      "\" overload type mismatch", call_args);
                    }
                }
                call_args.children[0]->exptype = &udt->thistype;
                // FIXME: this has the side effect of giving call_args types relative to the last
                // overload type-checked, which is strictly speaking not correct, but may not
                // matter. Could call TypeCheckCallStatic once more at the end of this loop
                // to fix that?
                // FIXME: return value?
                /*auto rtype =*/
                TypeCheckCallStatic(csf, call_args, reqret, specializers,
                                    overload_idxs[i].first, false, !last_sf);
                de = &dispatch_udt.dispatch[vtable_idx];  // May have realloced.
                sf = csf;
                sf->method_of = udt;
                sf->method_of->dispatch[vtable_idx].sf = sf;
                if (sf->isrecursivelycalled) any_recursive = true;
                if (sf->returned_thru) any_returned_thru = true;
                auto u = sf->returntype;
                if (de->returntype->IsBoundVar()) {
                    // FIXME: can this still happen now that recursive cases use explicit return
                    // types? If not change into assert?
                    if (!ConvertsTo(u, de->returntype, false))
                        TypeError(cat("dynamic dispatch for \"", f.name, "\" return value type \"",
                                      TypeName(sf->returntype),
                                      "\" doesn\'t match other case returning \"",
                                      TypeName(de->returntype), "\""), *sf->body);
                } else {
                    if (i) {
                        // We have to be able to take the union of all retvals without
                        // coercion, since we're not fixing up any previously typechecked
                        // functions.
                        u = Union(u, de->returntype, "function return type", "other overloads",
                                  false, &call_args);
                        // Ensure we didn't accidentally widen the type from a scalar.
                        assert(IsRef(de->returntype->t) || !IsRef(u->t));
                    }
                    de->returntype = u;
                }
                last_sf = sf;
            }
            if (any_recursive) {
                for (auto udt : dispatch_udt.subudts) {
                    auto sf = udt->dispatch[vtable_idx].sf;
                    if (!sf) continue;
                    if (sf->returngiventype.utr.Null())
                        TypeError("recursive dynamic dispatch must have explicit return type: " +
                                  sf->parent->name, call_args);
                }
            }
            if (any_returned_thru) {
                dispatch_udt.dispatch[vtable_idx].returned_thru = true;
            }
            call_args.children[0]->exptype = &dispatch_udt.thistype;
        }
        return dispatch_udt.dispatch[vtable_idx].returntype;
    };

    TypeRef TypeCheckCall(SubFunction *&csf, List &call_args, size_t reqret, int &vtable_idx,
                          vector<UnresolvedTypeRef> *specializers, bool super) {
        STACK_PROFILE;
        Function &f = *csf->parent;
        vtable_idx = -1;
        if (f.istype) {
            // Function types are always fully typed.
            // All calls thru this type must have same lifetimes, so we fix it to LT_BORROW.
            return TypeCheckMatchingCall(csf, call_args, true, false);
        }
        // Check if we need to do dynamic dispatch. We only do this for functions that have a
        // explicit first arg type of a class (not structs, since they can never dynamically be
        // different from their static type), and only when there is a sub-class that has a
        // method that can be called also.
        UDT *dispatch_udt = nullptr;
        TypeRef type0;
        if (f.nargs()) {
            type0 = call_args.children[0]->exptype;
            if (type0->t == V_CLASS) dispatch_udt = type0->udt;
        }
        if (dispatch_udt) {
            if (super) {
                // We're forcing static dispatch to the superclass;
                type0 = &dispatch_udt->resolved_superclass->thistype;
            } else {
                // Go thru all other overloads, and see if any of them have this one as superclass.
                for (auto isf : csf->parent->overloads) {
                    if (isf->method_of && st.SuperDistance(dispatch_udt, isf->method_of) > 0) {
                        LOG_DEBUG("dynamic dispatch: ", Signature(*isf));
                        return TypeCheckCallDispatch(*dispatch_udt, csf, call_args,
                            reqret, specializers, vtable_idx);
                    }
                }
            }
            // Yay there are no sub-class implementations, we can just statically dispatch.
        }
        // Do a static dispatch, if there are overloads, figure out from first arg which to pick,
        // much like dynamic dispatch. Unlike dynamic dispatch, we also include non-class types.
        // TODO: also involve the other arguments for more complex static overloads?

        // FIXME: the use of args[0].type here and further downstream only works because
        // we pre-resolve these in the TypeChecker constructor, instead we should use giventypes
        // properly here, and resolve them.
        int overload_idx = 0;
        if (f.nargs() && f.overloads.size() > 1) {
            overload_idx = -1;
            // First see if there is an exact match.
            for (auto [i, isf] : enumerate(f.overloads)) {
                if (ExactType(type0, isf->args[0].type)) {
                    if (overload_idx >= 0)
                        TypeError(cat("multiple overloads have the same type: \"", f.name,
                                      "\", first arg \"", TypeName(type0), "\""), call_args);
                    overload_idx = (int)i;
                }
            }
            // Then see if there's a match if we'd instantiate a generic UDT  first arg.
            if (overload_idx < 0 && IsUDT(type0->t)) {
                for (auto [i, isf] : enumerate(f.overloads)) {
                    auto arg0 = isf->giventypes[0].utr;  // Want unresolved type.
                    if (arg0->t == V_UUDT && arg0->spec_udt->udt == type0->udt->first) {
                        if (overload_idx >= 0) {
                            TypeError(cat("multiple generic overloads can instantiate: \"", f.name,
                                "\", first arg \"", TypeName(type0), "\""), call_args);
                        }
                        overload_idx = (int)i;
                    }
                }
            }
            // Then see if there's a match by subtyping.
            if (overload_idx < 0) {
                for (auto [i, isf] : enumerate(f.overloads)) {
                    auto arg0 = isf->args[0].type;
                    if (ConvertsTo(type0, arg0, false, false)) {
                        if (overload_idx >= 0) {
                            if (type0->t == V_CLASS) {
                                auto oarg0 = f.overloads[overload_idx]->args[0].type;
                                // Prefer "closest" supertype.
                                auto dist = st.SuperDistance(arg0->udt, type0->udt);
                                auto odist = st.SuperDistance(oarg0->udt, type0->udt);
                                if (dist < odist) overload_idx = (int)i;
                                else if (odist < dist) { /* keep old one */ }
                                else {
                                    TypeError(cat("multiple overloads have the same class: \"",
                                                  f.name, "\", first arg: \"", TypeName(type0),
                                                  "\""), call_args);
                                }
                            } else {
                                TypeError(cat("multiple overloads apply: \"", f.name,
                                              "\", first arg \"", TypeName(type0), "\""),
                                          call_args);
                            }
                        } else {
                            overload_idx = (int)i;
                        }
                    }
                }
            }
            // Then see if there's a match if we'd instantiate a fully generic first arg.
            if (overload_idx < 0) {
                for (auto [i, isf] : enumerate(f.overloads)) {
                    auto arg0 = isf->giventypes[0].utr;  // Want unresolved type.
                    if (arg0->t == V_TYPEVAR) {
                        if (overload_idx >= 0) {
                            TypeError(cat("multiple generic overloads can instantiate: \"", f.name,
                                "\", first arg \"", TypeName(type0), "\""), call_args);
                        }
                        overload_idx = (int)i;
                    }
                }
            }
            // Then finally try with coercion.
            if (overload_idx < 0) {
                for (auto [i, isf] : enumerate(f.overloads)) {
                    if (ConvertsTo(type0, isf->args[0].type, true, false)) {
                        if (overload_idx >= 0) {
                            TypeError(cat("multiple overloads can coerce: \"", f.name,
                                      "\", first arg \"", TypeName(type0), "\""), call_args);
                        }
                        overload_idx = (int)i;
                    }
                }
            }
            if (overload_idx < 0)
                TypeError(cat("no overloads apply: \"", f.name, "\", first arg \"", TypeName(type0),
                              "\""), call_args);
        }
        LOG_DEBUG("static dispatch: ", Signature(*f.overloads[overload_idx]));
        return TypeCheckCallStatic(csf, call_args, reqret, specializers,
                                   overload_idx, true, false);
    }

    SubFunction *PreSpecializeFunction(SubFunction *hsf) {
        // Don't pre-specialize named functions, because this is not their call-site.
        if (!hsf->parent->anonymous) return hsf;
        assert(hsf->parent->overloads.size() == 1);
        hsf = hsf->parent->overloads[0];
        auto sf = hsf;
        if (sf->freevarchecked) {
            // See if there's an existing match.
            for (; sf; sf = sf->next) if (sf->freevarchecked) {
                if (FreeVarsSameAsCurrent(*sf, true)) return sf;
            }
            sf = CloneFunction(hsf, 0);
        } else {
            // First time this function has ever been touched.
            sf->freevarchecked = true;
        }
        assert(!sf->freevars.size());
        // Output without arg types, since those are yet to be overwritten.
        LOG_DEBUG("pre-specialization: ", SignatureWithFreeVars(*sf, nullptr));
        return sf;
    }

    pair<TypeRef, Lifetime> TypeCheckDynCall(SpecIdent *fval, List *args, SubFunction *&fspec,
                                             size_t reqret) {
        auto &ftype = fval->type;
        auto nargs = args->Arity();
        if (!ftype->IsFunction()) {
            TypeError(cat("dynamic function call value doesn\'t have a function type: \"",
                TypeName(ftype), "\""), *args);
            return { type_void, LT_ANY };
        }
        // We can statically typecheck this dynamic call. Happens for almost all non-escaping
        // closures.
        fspec = ftype->sf;
        if (nargs < fspec->parent->nargs())
            TypeError("function value called with too few arguments", *args);
        // In the case of too many args, TypeCheckCall will ignore them (and optimizer will
        // remove them).
        int vtable_idx = -1;
        auto type = TypeCheckCall(fspec, *args, reqret, vtable_idx, nullptr, false);
        assert(vtable_idx < 0);
        ftype = &fspec->thistype;
        return { type, fspec->ltret };
    }

    TypeRef TypeCheckBranch(bool iftrue, const Node *condition, Block *body, size_t reqret) {
        auto flowstart = CheckFlowTypeChanges(iftrue, condition);
        assert(reqret <= 1);
        body->TypeCheck(*this, reqret);
        CleanUpFlow(flowstart);
        return body->exptype;
    }

    void CheckFlowTypeIdOrDot(const Node &n, TypeRef type) {
        FlowItem fi(n, type);
        if (fi.IsValid()) flowstack.push_back(fi);
    }

    void CheckFlowTypeChangesSub(bool iftrue, const Node *condition) {
        condition = SkipCoercions(condition);
        auto type = condition->exptype;
        if (auto c = Is<IsType>(condition)) {
            if (iftrue) CheckFlowTypeIdOrDot(*c->child, c->resolvedtype);
        } else if (auto c = Is<Not>(condition)) {
            CheckFlowTypeChangesSub(!iftrue, c->child);
        } else if (auto eq = Is<Equal>(condition)) {
            if (Is<Nil>(eq->right)) CheckFlowTypeChangesSub(!iftrue, eq->left);
        } else if (auto neq = Is<NotEqual>(condition)) {
            if (Is<Nil>(neq->right)) CheckFlowTypeChangesSub(iftrue, neq->left);
        } else {
            if (iftrue && type->t == V_NIL) CheckFlowTypeIdOrDot(*condition, type->Element());
        }
    }

    void CheckFlowTypeChangesAndOr(bool iftrue, const BinOp *condition) {
        // AND only works for then, and OR only for else.
        if (iftrue == (Is<And>(condition) != nullptr)) {
            // This allows for a chain of and's without allowing mixed operators.
            auto cleft = SkipCoercions(condition->left);
            if (typeid(*cleft) == typeid(*condition)) {
                CheckFlowTypeChanges(iftrue, condition->left);
            } else {
                CheckFlowTypeChangesSub(iftrue, condition->left);
            }
            CheckFlowTypeChangesSub(iftrue, condition->right);
        }
    }

    size_t CheckFlowTypeChanges(bool iftrue, const Node *condition) {
        auto start = flowstack.size();
        condition = SkipCoercions(condition);
        if (auto c = Is<Or>(condition)) {
            CheckFlowTypeChangesAndOr(iftrue, c);
        } else if (auto c = Is<And>(condition)) {
            CheckFlowTypeChangesAndOr(iftrue, c);
        } else {
            CheckFlowTypeChangesSub(iftrue, condition);
        }
        return start;
    }

    void AssignFlowPromote(Node &left, TypeRef right) {
        if (left.exptype->t == V_NIL && right->t != V_NIL) {
            CheckFlowTypeIdOrDot(left, right);
        }
    }

    // FIXME: this can in theory find the wrong node, if the same function nests, and the outer
    // one was specialized to a nilable and the inner one was not.
    // This would be very rare though, and benign.
    TypeRef AssignFlowDemote(FlowItem &left, TypeRef overwritetype, bool coercions) {
        // Early out, numeric types are not nillable, nor do they make any sense for "is"
        auto &type = left.now;
        if (type->Numeric()) return type;
        for (auto &flow : reverse(flowstack)) {
            if (flow.sid == left.sid) {
                if (left.derefs.empty()) {
                    if (flow.derefs.empty()) {
                        type = flow.old;
                        goto found;
                    } else {
                        // We're writing to var V and V.f is in the stack: invalidate regardless.
                        goto found;
                    }
                } else {
                    if (flow.DerefsEqual(left)) {
                        type = flow.old;
                        goto found;
                    }
                }
            }
            continue;
            found:
            if (!ConvertsTo(overwritetype, flow.now, coercions, false)) {
                // FLow based promotion is invalidated.
                flow.now = flow.old;
                // TODO: It be cool to instead overwrite with whatever type is currently being
                // assigned. That currently doesn't work, since our flow analysis is a
                // conservative approximation, so if this assignment happens conditionally it
                // wouldn't work.
            }
            // We continue with the loop here, since a single assignment may invalidate multiple
            // promotions
        }
        return type;
    }

    TypeRef UseFlow(const FlowItem &left) {
        if (left.now->Numeric()) return left.now;  // Early out, same as above.
        for (auto flow : reverse(flowstack)) {
            if (flow.sid == left.sid &&	flow.DerefsEqual(left)) {
                return flow.now;
            }
        }
        return left.now;
    }

    void CleanUpFlow(size_t start) {
        while (flowstack.size() > start) flowstack.pop_back();
    }

    void TypeCheckAndOr(BinOp &ao, bool only_true_type, bool reqret, TypeRef &promoted_type) {
        // only_true_type supports patterns like ((a & b) | c) where the type of a doesn't matter,
        // and the overal type should be the union of b and c.
        // Or a? | b, which should also be the union of a and b.
        TypeRef tleft, tright;
        TypeCheckAndOrSub(ao.left, Is<Or>(ao), true, tleft);
        auto flowstart = CheckFlowTypeChanges(Is<And>(ao), ao.left);
        TypeCheckAndOrSub(ao.right, only_true_type, reqret, tright);
        CleanUpFlow(flowstart);
        if (only_true_type && Is<And>(ao)) {
            ao.exptype = tright;
            ao.lt = ao.right->lt;
            DecBorrowers(ao.left->lt, ao);
        } else {
            ao.exptype = Union(tleft, tright, "lhs", "rhs", false, nullptr);
            if (ao.exptype->t == V_UNDEFINED) {
                // Special case: unlike elsewhere, we allow merging scalar and reference types,
                // since they are just tested and thrown away. To make this work, we force all
                // values to bools.
                MakeBool(ao.left);
                MakeBool(ao.right);
                ao.exptype = &st.default_bool_type->thistype;
                ao.lt = LT_ANY;
            } else {
                ao.lt = LifetimeUnion(ao.left, ao.right, Is<And>(ao));
            }
        }
        promoted_type = ao.exptype;
    }

    void TypeCheckAndOrSub(Node *&n, bool only_true_type, bool reqret, TypeRef &promoted_type) {
        // only_true_type supports patterns like ((a & b) | c) where the type of a doesn't matter,
        // and the overal type should be the union of b and c.
        // Or a? | b, which should also be the union of a and b.
        n = RemoveCoercions(n);
        if (!Is<And>(n) && !Is<Or>(n)) {
            TT(n, reqret, LT_ANY);
            NoStruct(*n, "and / or");
            promoted_type = n->exptype;
            if (promoted_type->t == V_NIL && only_true_type)
                promoted_type = promoted_type->Element();
        } else {
            auto ao = dynamic_cast<BinOp *>(n);
            assert(ao);
            TypeCheckAndOr(*ao, only_true_type, reqret, promoted_type);
        }
    }

    optional<Value> TypeCheckCondition(Node *&condition, Node *context, const char *name) {
        TT(condition, 1, LT_BORROW);
        NoStruct(*condition, name);
        DecBorrowers(condition->lt, *context);
        Value cval;
        if (condition->ConstVal(this, cval) != V_VOID) return cval;
        return {};
    }

    void CheckLval(Node *n) {
        if (auto dot = Is<Dot>(n)) {
            auto type = dot->children[0]->exptype;
            if (IsStruct(type->t))
                TypeError(cat("cannot write to field of value \"", type->udt->name, "\""), *n);
        }
        // This can happen due to late specialization of GenericCall.
        if (Is<Call>(n) || Is<NativeCall>(n))
            TypeError("function-call cannot be an l-value", *n);
        Borrow lv(*n);
        if (!lv.IsValid()) return;  // FIXME: force these to LT_KEEP?
        if (IsRefNil(n->exptype->t)) {
            // If any of the functions this assign sits in is reused, we need to be able to replay
            // checking the errors in CheckLvalBorrowed, since the contents of the borrowstack
            // may be different.
            for (auto &sc : reverse(scopes)) {
                // we could uniqueify this vector, but that would entails comparing `n`
                // structurally (construct a Borrow for each?), which would probably be
                // slower than the redundant calls to CheckLvalBorrowed this causes later?
                // Especially since this uniqueifying cost is paid always, even when there
                // are no actual repeated assigns in a scope, which is not that common.
                sc.sf->reuse_assign_events.push_back(n);
                if (sc.sf == lv.sid->sf_def) break;  // Don't go further than where defined.
            }
        }
        CheckLvalBorrowed(n, lv);
    }

    void CheckLvalBorrowed(Node *n, Borrow &lv) {
        if (lv.derefs.empty() && LifetimeType(lv.sid->lt) == LT_BORROW) {
            // This should only happen for multimethods and anonymous functions used with istype
            // where we can't avoid arguments being LT_BORROW.
            // All others should have been specialized to LT_KEEP when a var is not
            // single_assignment.
            // This is not particularly elegant but should be rare.
            TypeError(cat("cannot assign to borrowed argument \"", lv.sid->id->name, "\""), *n);
        }
        // FIXME: make this faster.
        for (auto &b : reverse(borrowstack)) {
            if (!b.IsPrefix(lv)) continue;  // Not overwriting this one.
            if (!b.refc) continue;          // Lval is not borowed, writing is ok.
            TypeError(cat("cannot modify \"", lv.Name(), "\" while borrowed (in ",
                          lv.sid->sf_def->parent->name, ")"), *n);
        }
    }

    void ReplayAssigns(SubFunction *sf) {
        for (auto n : sf->reuse_assign_events) {
            Borrow lv(*n);
            CheckLvalBorrowed(n, lv);
        }
    }

    Lifetime PushBorrow(Node *n) {
        if (!IsRefNilVar(n->exptype->t)) return LT_ANY;
        Borrow lv(*n);
        // FIXME: if this is an exp we don't know how to borrow from (like a[i].b) we
        // return a generic borrow, but this disables lock checks so is unsafe.
        if (!lv.IsValid()) return LT_BORROW;
        for (auto &b : reverse(borrowstack)) {
            if (b.sid == lv.sid && b.DerefsEqual(lv)) {
                b.refc++;
                return (Lifetime)(&b - &borrowstack[0]);
            }
        }
        // FIXME: this path is slow, should not have to scan all of borrowstack.
        auto lt = (Lifetime)borrowstack.size();
        borrowstack.push_back(lv);
        return lt;
    }

    void CheckFreeVariable(SpecIdent &sid) {
        // If this is a free variable, record it in all parents up to the definition point.
        // FIXME: this is technically not the same as a "free variable" in the literature,
        // since HOFs get marked with freevars of their functionvalue this way.
        // This is benign, since the HOF will be specialized to the function value anyway,
        // but would be good to clean up.
        // We currently don't have an easy way to test for lexically enclosing functions.
        for (int i = (int)scopes.size() - 1; i >= 0; i--) {
            auto sf = scopes[i].sf;
            // Check if we arrived at the definition point.
            if (sid.sf_def == sf)
                break;
            // We use the id's type, not the flow sensitive type, just in case there's multiple uses
            // of the var. This will get corrected after the call this is part of.
            if (sf->Add(sf->freevars, Arg(&sid, sid.type, NF_NONE))) {
                //LOG_DEBUG("freevar added: ", id.name, " (", TypeName(id.type),
                //                     ") in ", sf->parent->name);
            }
        }
    }

    void TypeCheckList(List *n, Lifetime lt) {
        for (auto &c : n->children) {
            TT(c, 1, lt);
        }
    }

    TypeRef TypeCheckId(SpecIdent *sid) {
        auto type = sid->type;
        CheckFreeVariable(*sid);
        return type;
    }

    const Coercion *IsCoercion(const Node *n) {
        return dynamic_cast<const Coercion *>(n);
    }

    const Node *SkipCoercions(const Node *n) {
        auto c = IsCoercion(n);
        return c ? SkipCoercions(c->child) : n;
    }

    Node *RemoveCoercions(Node *n) {
        auto c = IsCoercion(n);
        return c ? RemoveCoercions(DeleteCoercion((Coercion *)c)) : n;
    }

    Node *DeleteCoercion(Coercion *c) {
        auto ch = c->child;
        c->child = nullptr;
        delete c;
        return ch;
    }

    Lifetime LvalueLifetime(const Node &lval, bool deref) {
        if (auto idr = Is<IdentRef>(lval)) return idr->sid->lt;
        if (deref) {
            if (auto dot = Is<Dot>(lval)) return LvalueLifetime(*dot->children[0], deref);
            if (auto idx = Is<Indexing>(lval)) return LvalueLifetime(*idx->object, deref);
        }
        return LT_KEEP;
    }

    Lifetime LifetimeUnion(Node *&a, Node *&b, bool is_and) {
        if (a->lt == b->lt) {
            DecBorrowers(b->lt, *b);
            return a->lt;
        } else if (a->lt == LT_ANY && b->lt >= LT_BORROW) {
            // This case may apply in an if-then between a var and nil, or an and/or between
            // a var and a scalar.
            return b->lt;
        } else if (b->lt == LT_ANY && a->lt >= LT_BORROW) {
            // Same.
            return a->lt;
        } else if (is_and && a->lt >= LT_BORROW && b->lt >= LT_BORROW) {
            // var_a and var_b never results in var_a.
            DecBorrowers(a->lt, *a);
            return b->lt;
        } else {
            // If it is an and we want to borrow the lhs since it will never be used.
            // Otherwise default to LT_KEEP for everything.
            // FIXME: for cases where both sides are >= LT_BORROW (in an if-then) we'd like to
            // combine both lifetimes into one, but we currently can't represent that.
            AdjustLifetime(a, is_and ? LT_BORROW : LT_KEEP);
            if (is_and) DecBorrowers(a->lt, *a);
            AdjustLifetime(b, LT_KEEP);
            return LT_KEEP;
        }
    }

    void Borrowers(Lifetime lt, int change, const Node &context) {
        if (lt < 0) return;
        auto &b = borrowstack[lt];
        assert(IsRefNilVar(b.sid->type->t));
        b.refc += change;
        LOG_DEBUG("borrow ", change, ": ", b.sid->id->name, " in ", NiceName(context),
               ", ", b.refc, " remain");
        // FIXME: this should really just not be possible, but hard to guarantee.
        if (b.refc < 0)
            TypeError(cat("\"", b.sid->id->name, "\" used in \"", NiceName(context),
                          "\" without being borrowed"), context);
        assert(b.refc >= 0);
        (void)context;
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

    void AdjustLifetime(Node *&n, Lifetime recip, const vector<Node *> *idents = nullptr) {
        assert(n->lt != LT_UNDEF && recip != LT_UNDEF);
        uint64_t incref = 0, decref = 0;
        auto rt = n->exptype;
        for (size_t i = 0; i < rt->NumValues(); i++) {
            assert (n->lt != LT_MULTIPLE || rt->t == V_TUPLE);
            auto givenlt = rt->GetLifetime(i, n->lt);
            auto given = LifetimeType(givenlt);
            if (idents) recip = LvalueLifetime(*(*idents)[i], false);  // FIXME: overwrite var?
            recip = LifetimeType(recip);
            if (given != recip) {
                auto rtt = rt->Get(i)->t;
                // Sadly, if it a V_VAR we have to be conservate and assume it may become a ref.
                if (IsRefNilVar(rtt)) {
                    // Special action required.
                    if (i >= sizeof(incref) * 8) TypeError("too many return values", *n);
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

    // This is the central function thru which all typechecking flows, so we can conveniently
    // match up what the node produces and what the recipient expects.
    void TT(Node *&n, size_t reqret, Lifetime recip, const vector<Node *> *idents = nullptr) {
        STACK_PROFILE;
        // Central point from which each node is typechecked.
        n = n->TypeCheck(*this, reqret);
        // Check if we need to do any type adjustmenst.
        auto &rt = n->exptype;
        n->exptype = rt;
        auto nret = rt->NumValues();
        if (nret < reqret) {
            if (!n->Terminal(*this)) {
                TypeError(cat("\"", NiceName(*n), "\" returns ", nret, " values, ", reqret,
                              " needed"),
                          *n);
            } else {
                // FIXME: would be better to have a general NORETURN type than patching things up
                // this way.
                if (reqret == 1) {
                    rt = type_any;
                } else {
                    auto nt = st.NewTuple(reqret);
                    for (size_t i = 0; i < reqret; i++) {
                        if (i < nret) nt->Set(i, rt->Get(i), rt->GetLifetime(i, n->lt));
                        else nt->Set(i, &*type_any, LT_ANY);
                    }
                    rt = nt;
                }
            }
        } else if (nret > reqret) {
            for (size_t i = reqret; i < nret; i++) {
                // This value will be dropped.
                DecBorrowers(rt->GetLifetime(i, n->lt), *n);
                // If this is a LT_KEEP value, codegen will make sure to throw it away.
            }
            switch (reqret) {
                case 0:
                    n->lt = LT_ANY;
                    rt = type_void;
                    break;
                case 1: {
                    auto typelt = TypeLT { *n, 0 };  // Get from tuple.
                    n->lt = typelt.lt;
                    rt = typelt.type;
                    break;
                }
                default: {
                    auto nt = st.NewTuple(reqret);
                    nt->tup->assign(rt->tup->begin(), rt->tup->begin() + reqret);
                    rt = nt;
                }
            }
        }
        // Check if we need to do any lifetime adjustments.
        AdjustLifetime(n, recip, idents);
    }

    // TODO: Can't do this transform ahead of time, since it often depends upon the input args.
    TypeRef ActualBuiltinType(int flen, TypeRef type, NArgFlags flags, Node *exp,
                              const NativeFun *nf, bool test_overloads, size_t argn,
                              const Node &errorn) {
        if (flags & NF_BOOL) {
            type = type->ElementIfNil();
            assert(type->t == V_INT);
            return &st.default_bool_type->thistype;
        }
        // See if we can promote the type to one of the standard vector types
        // (xy/xyz/xyzw).
        if (!flen) return type;
        type = type->ElementIfNil();
        auto etype = exp ? exp->exptype : nullptr;
        auto e = etype;
        size_t i = 0;
        for (auto vt = type; vt->t == V_VECTOR && i < SymbolTable::NUM_VECTOR_TYPE_WRAPPINGS;
            vt = vt->sub) {
            if (vt->sub->Numeric()) {
                // Check if we allow any vector length.
                if (!e.Null() && flen == -1 && e->t == V_STRUCT_S) {
                    flen = (int)e->udt->fields.size();
                }
                if (!etype.Null() && flen == -1 && etype->t == V_VAR) {
                    // Special case for "F}?" style types that can be matched against a
                    // DefaultArg, would be good to solve this more elegantly..
                    // FIXME: don't know arity, but it doesn't matter, so we pick 2..
                    return st.VectorType(vt, i, 2);
                }
                if (flen >= 2 && flen <= 4) {
                    if (!e.Null() && e->t == V_STRUCT_S && (int)e->udt->fields.size() == flen &&
                        e->udt->sametype == vt->sub) {
                        // Allow any similar vector type, like "color".
                        return etype;
                    }
                    else {
                        // Require xy/xyz/xyzw
                        return st.VectorType(vt, i, flen);
                    }
                }
            }
            e = !e.Null() && e->t == V_VECTOR ? e->sub : nullptr;
            i++;
        }
        // We arrive here typically if flen == -1 but we weren't able to derive a length.
        // Sadly, we can't allow to return a vector type instead of a struct, so we error out,
        // and rely on the user to specify more precise types.
        // Not sure if there is a better solution.
        if (!test_overloads) {
            TypeError(cat("cannot deduce struct type for ",
                          (argn ? cat("argument ", argn) : "return value"),
                          " of \"", nf->name,
                          (!etype.Null() ? "\", got: \"" + TypeName(etype) + "\"" : "\"")),
                errorn);
        }
        return type;
    };

    void Stats() {
        if (min_output_level > OUTPUT_INFO) return;
        int origsf = 0, clonesf = 0;
        size_t orignodes = 0, clonenodes = 0;
        typedef pair<size_t, Function *> Pair;
        vector<Pair> funstats;
        for (auto f : st.functiontable) funstats.push_back({ 0, f });
        for (auto sf : st.subfunctiontable) {
            auto count = sf->body ? sf->body->Count() : 0;
            if (!sf->next)        {
                origsf++;
                orignodes += count;
            } else {
                clonesf++;
                clonenodes += count;
                funstats[sf->parent->idx].first += count;
            }
        }
        LOG_INFO("SF count: orig: ", origsf, ", cloned: ", clonesf);
        LOG_INFO("Node count: orig: ", orignodes, ", cloned: ", clonenodes);
        sort(funstats.begin(), funstats.end(),
            [](const Pair &a, const Pair &b) { return a.first > b.first; });
        for (auto &[fsize, f] : funstats) if (fsize > orignodes / 100) {
            auto &pos = f->overloads.back()->body->line;
            LOG_INFO("Most clones: ", f->name, " (", st.filenames[pos.fileidx], ":", pos.line,
                     ") -> ", fsize, " nodes accross ", f->NumSubf() - f->overloads.size(),
                     " clones (+", f->overloads.size(), " orig)");
        }
    }
};

Node *Block::TypeCheck(TypeChecker &tc, size_t reqret) {
    for (auto &c : children) {
        tc.TT(c, c != children.back() ? 0 : reqret, LT_ANY);
    }
    lt = children.back()->lt;
    exptype = children.back()->exptype;
    return this;
}

Node *List::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/) {
    assert(false);  // Parents call TypeCheckList
    return this;
}

Node *Unary::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/) {
    assert(false);
    return this;
}

Node *BinOp::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/) {
    assert(false);
    return this;
}

Node *Or::TypeCheck(TypeChecker &tc, size_t reqret) {
    TypeRef dummy;
    tc.TypeCheckAndOr(*this, false, reqret, dummy);
    return this;
}

Node *And::TypeCheck(TypeChecker &tc, size_t reqret) {
    TypeRef dummy;
    tc.TypeCheckAndOr(*this, false, reqret, dummy);
    return this;
}

Node *IfThen::TypeCheck(TypeChecker &tc, size_t) {
    auto constant = tc.TypeCheckCondition(condition, this, "if");
    if (!constant || constant->True()) {
        tc.TypeCheckBranch(true, condition, truepart, 0);
        if (truepart->Terminal(tc)) {
            // This is an if ..: return, we should leave promotions for code after the if.
            tc.CheckFlowTypeChanges(false, condition);
        }
    } else {
        // constant == false: this if-then will get optimized out entirely, ignore it.
    }
    // No else: this always returns void.
    truepart->exptype = type_void;
    exptype = type_void;
    lt = LT_ANY;
    return this;
}

Node *IfElse::TypeCheck(TypeChecker &tc, size_t reqret) {
    auto constant = tc.TypeCheckCondition(condition, this, "if");
    if (!constant) {
        auto tleft = tc.TypeCheckBranch(true, condition, truepart, reqret);
        auto tright = tc.TypeCheckBranch(false, condition, falsepart, reqret);
        // FIXME: this is a bit of a hack. Much better if we had an actual type
        // to signify NORETURN, to be taken into account in more places.
        if (truepart->Terminal(tc)) {
            exptype = tright;
            lt = falsepart->lt;
        } else if (falsepart->Terminal(tc)) {
            exptype = tleft;
            lt = truepart->lt;
        } else {
            exptype = tc.Union(tleft, tright, "then branch", "else branch", true, this);
            tc.SubType(truepart->children.back(), exptype, "then branch", *this);
            tc.SubType(falsepart->children.back(), exptype, "else branch", *this);
            lt = tc.LifetimeUnion(truepart->children.back(), falsepart->children.back(), false);
        }
    } else if (constant->True()) {
        // Ignore the else part, optimizer guaranteed to cull it.
        exptype = tc.TypeCheckBranch(true, condition, truepart, reqret);
        lt = truepart->lt;
    } else {
        // Ignore the then part, optimizer guaranteed to cull it.
        exptype = tc.TypeCheckBranch(false, condition, falsepart, reqret);
        lt = falsepart->lt;
    }
    return this;
}

Node *While::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckCondition(condition, this, "while");
    tc.scopes.back().loop_count++;
    tc.TypeCheckBranch(true, condition, body, 0);
    tc.scopes.back().loop_count--;
    exptype = type_void;
    lt = LT_ANY;
    return this;
}

Node *For::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    // FIXME: would be good to detect when iter is not written to, so ForLoopElem can be LT_BORROW.
    // Alternatively we could IncBorrowers on iter, but that would be very restrictive.
    tc.TT(iter, 1, LT_BORROW);
    auto itertype = iter->exptype;
    if (itertype->t == V_INT) {}
    else if (itertype->t == V_STRING)
        itertype = type_int;
    else if (itertype->t == V_VECTOR)
        itertype = itertype->Element();
    else tc.TypeError(cat("for can only iterate over int / string / vector, not \"",
                          TypeName(itertype), "\""), *this);
    auto def = Is<Define>(body->children[0]);
    if (def) {
        auto fle = Is<ForLoopElem>(def->child);
        if (fle) fle->exptype = itertype;
    }
    tc.scopes.back().loop_count++;
    body->TypeCheck(tc, 0);
    tc.scopes.back().loop_count--;
    tc.DecBorrowers(iter->lt, *this);
    // Currently always return V_NIL
    exptype = type_void;
    lt = LT_ANY;
    return this;
}

Node *ForLoopElem::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/) {
    // Already been assigned a type in For.
    lt = LT_KEEP;
    return this;
}

Node *ForLoopCounter::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/) {
    exptype = type_int;
    lt = LT_ANY;
    return this;
}

Node *Break::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    if (!tc.scopes.back().loop_count)
        tc.TypeError("break must occur inside a while or for", *this);
    exptype = type_void;
    lt = LT_ANY;
    return this;
}

Node *Switch::TypeCheck(TypeChecker &tc, size_t reqret) {
    // TODO: much like If, should only typecheck one case if the value is constant, and do
    // the corresponding work in the optimizer.
    tc.TT(value, 1, LT_BORROW);
    tc.DecBorrowers(value->lt, *this);
    auto ptype = value->exptype;
    if (!ptype->Numeric() && ptype->t != V_STRING)
        tc.TypeError("switch value must be int / float / string", *this);
    exptype = nullptr;
    bool have_default = false;
    vector<bool> enum_cases;
    if (ptype->IsEnum()) enum_cases.resize(ptype->e->vals.size());
    for (auto &n : cases->children) {
        tc.TT(n, reqret, LT_KEEP);
        auto cas = AssertIs<Case>(n);
        if (cas->pattern->children.empty()) have_default = true;
        for (auto c : cas->pattern->children) {
            tc.SubTypeT(c->exptype, ptype, *c, "", "case");
            tc.DecBorrowers(c->lt, *cas);
            if (ptype->IsEnum()) {
                assert(c->exptype->IsEnum());
                Value v;
                if (c->ConstVal(&tc, v) != V_VOID) {
                    for (auto [i, ev] : enumerate(ptype->e->vals)) if (ev->val == v.ival()) {
                        enum_cases[i] = true;
                        break;
                    }
                }
            }
        }
        if (!cas->body->Terminal(tc)) {
            exptype = exptype.Null() ? cas->body->exptype
                                     : tc.Union(exptype, cas->body->exptype, "switch type", "case type",
                                                true, cas);
        }
    }
    for (auto n : cases->children) {
        auto cas = AssertIs<Case>(n);
        if (!cas->body->Terminal(tc)) {
            assert(!exptype.Null());
            tc.SubType(cas->body, exptype, "", "case block");
        }
    }
    if (exptype.Null()) exptype = type_void;  // Empty switch or all return statements.
    if (!have_default) {
        if (reqret) tc.TypeError("switch that returns a value must have a default case", *this);
        if (ptype->IsEnum()) {
            for (auto [i, ev] : enumerate(ptype->e->vals)) if (!enum_cases[i])
                tc.TypeError("enum value not tested in switch: " + ev->name, *value);
        }
    }
    lt = LT_KEEP;
    return this;
}

Node *Case::TypeCheck(TypeChecker &tc, size_t reqret) {
    // FIXME: Since string constants are the real use case, LT_KEEP would be more
    // natural here, as this will introduce a lot of keeprefs. Alternatively make sure
    // string consts don't introduce keeprefs.
    tc.TypeCheckList(pattern, LT_BORROW);
    tc.TT(body, reqret, LT_KEEP);
    exptype = body->exptype;
    lt = LT_KEEP;
    return this;
}

Node *Range::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TT(start, 1, LT_KEEP);
    tc.TT(end, 1, LT_KEEP);
    exptype = start->exptype;
    if (exptype->t != end->exptype->t || !exptype->Numeric())
        tc.TypeError("range can only be two equal numeric types", *this);
    lt = LT_ANY;
    return this;
}

Node *Define::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    for (auto &p : sids) {
        tc.UpdateCurrentSid(p.first);
        // We have to set these here just in case the init exp is a function call that
        // tries use/assign this variable, type_undefined will force that to be an error.
        // TODO: could make this a specialized error, but probably not worth it because it is rare.
        p.first->type = type_undefined;
        p.first->lt = LT_UNDEF;
    }
    // We default to LT_KEEP here.
    // There are case where we could allow borrow, but in practise this runs into trouble easily:
    // - Variables that later get assigned (!sid->id->single_assignment) where taking ownership
    //   was really what was intended (since the lval being assigned from may go away).
    // - old := cur cases, where old is meant to hang on to the previous value as cur gets updated,
    //   which then runs into borrowing errors.
    tc.TT(child, sids.size(), LT_KEEP);
    for (auto [i, p] : enumerate(sids)) {
        auto var = TypeLT(*child, i);
        if (!p.second.utr.Null()) {
            var.type = tc.ResolveTypeVars(p.second, this);
            // Have to subtype the initializer value, as that node may contain
            // unbound vars (a:[int] = []) or values that that need to be coerced
            // (a:float = 1)
            if (sids.size() == 1) {
                tc.SubType(child, var.type, "initializer", "definition");
            } else {
                // FIXME: no coercion when mult-return?
                tc.SubTypeT(child->exptype->Get(i), var.type, *this, p.first->id->name);
            }
            // In addition, the initializer may already cause the type to be promoted.
            // a:string? = ""
            FlowItem fi(p.first, var.type, child->exptype);
            // Similar to AssignFlowPromote (TODO: refactor):
            if (fi.now->t != V_NIL && fi.old->t == V_NIL) tc.flowstack.push_back(fi);
        }
        auto sid = p.first;
        sid->type = var.type;
        tc.StorageType(var.type, *this);
        sid->type = var.type;
        sid->lt = var.lt;
        LOG_DEBUG("var: ", sid->id->name, ":", TypeName(var.type));
    }
    exptype = type_void;
    lt = LT_ANY;
    return this;
}

Node *AssignList::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    for (auto &c : children) {
        if (c != children.back()) {
            tc.TT(c, 1, LT_BORROW);
            tc.DecBorrowers(c->lt, *this);
        } else {
            tc.TT(c, children.size() - 1, LT_MULTIPLE /*unused*/, &children);
        }
    }
    for (size_t i = 0; i < children.size() - 1; i++) {
        auto left = children[i];
        tc.CheckLval(left);
        TypeRef righttype = children.back()->exptype->Get(i);
        FlowItem fi(*left, left->exptype);
        assert(fi.IsValid());
        tc.AssignFlowDemote(fi, righttype, false);
        tc.SubTypeT(righttype, left->exptype, *this, "right");
        tc.StorageType(left->exptype, *left);
        // TODO: should call tc.AssignFlowPromote(*left, vartype) here?
    }
    exptype = type_void;
    lt = LT_ANY;
    return this;
}

Node *IntConstant::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/) {
    exptype = from ? &from->e->thistype : type_int;
    lt = LT_ANY;
    return this;
}

Node *FloatConstant::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/) {
    exptype = type_float;
    lt = LT_ANY;
    return this;
}

Node *StringConstant::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/) {
    exptype = type_string;
    // The VM keeps all the constant strings for the length of the program,
    // so these can be borrow, avoiding a ton of keepvars when used in + and
    // builtin functions etc (at the cost of some increfs when stored in vars
    // and data structures).
    lt = STRING_CONSTANTS_KEEP ? LT_KEEP : LT_BORROW;
    return this;
}

Node *Nil::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    if (giventype.utr.Null()) {
        exptype = tc.st.Wrap(tc.NewTypeVar(), V_NIL);
    } else {
        exptype = tc.ResolveTypeVars(giventype, this);
        if (!tc.st.IsNillable(exptype->sub)) tc.TypeError("illegal nil type", *this);
    }
    lt = LT_ANY;
    return this;
}

Node *Plus::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOp(*this);
    return this;
}

Node *Minus::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOp(*this);
    return this;
}

Node *Multiply::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOp(*this);
    return this;
}

Node *Divide::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOp(*this);
    return this;
}

Node *Mod::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOp(*this);
    return this;
}

Node *PlusEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOpEq(*this);
    return this;
}

Node *MultiplyEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOpEq(*this);
    return this;
}

Node *MinusEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOpEq(*this);
    return this;
}

Node *DivideEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOpEq(*this);
    return this;
}

Node *ModEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOpEq(*this);
    return this;
}

Node *AndEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOpEqBit(*this);
    return this;
}

Node *OrEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOpEqBit(*this);
    return this;
}

Node *XorEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOpEqBit(*this);
    return this;
}

Node *ShiftLeftEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOpEqBit(*this);
    return this;
}

Node *ShiftRightEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckMathOpEqBit(*this);
    return this;
}

Node *NotEqual::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckComp(*this);
    return this;
}

Node *Equal::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckComp(*this);
    return this;
}

Node *GreaterThanEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckComp(*this);
    return this;
}

Node *LessThanEq::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckComp(*this);
    return this;
}

Node *GreaterThan::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckComp(*this);
    return this;
}

Node *LessThan::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckComp(*this);
    return this;
}

Node *Not::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TT(child, 1, LT_BORROW);
    tc.DecBorrowers(child->lt, *this);
    tc.NoStruct(*child, "not");
    exptype = &tc.st.default_bool_type->thistype;
    lt = LT_ANY;
    return this;
}

Node *BitAnd::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckBitOp(*this);
    return this;
}

Node *BitOr::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckBitOp(*this);
    return this;
}

Node *Xor::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckBitOp(*this);
    return this;
}

Node *ShiftLeft::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckBitOp(*this);
    return this;
}

Node *ShiftRight::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckBitOp(*this);
    return this;
}

Node *Negate::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TT(child, 1, LT_BORROW);
    tc.SubType(child, type_int, "negated value", *this);
    tc.DecBorrowers(child->lt, *this);
    exptype = child->exptype;
    lt = LT_ANY;
    return this;
}

Node *PostDecr::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckPlusPlus(*this);
    return this;
}

Node *PostIncr::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckPlusPlus(*this);
    return this;
}

Node *PreDecr::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckPlusPlus(*this);
    return this;
}

Node *PreIncr::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckPlusPlus(*this);
    return this;
}

Node *UnaryMinus::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TT(child, 1, LT_BORROW);
    exptype = child->exptype;
    if (!exptype->Numeric() &&
        (exptype->t != V_STRUCT_S || !exptype->udt->sametype->Numeric()))
        tc.TypeError("numeric / numeric struct", exptype, *this);
    tc.DecBorrowers(child->lt, *this);
    lt = LT_KEEP;
    return this;
}

Node *IdentRef::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.UpdateCurrentSid(sid);
    for (auto &sc : reverse(tc.scopes)) if (sc.sf == sid->sf_def) goto in_scope;
    tc.TypeError("free variable not in scope: " + sid->id->name, *this);
    in_scope:
    exptype = tc.TypeCheckId(sid);
    FlowItem fi(*this, exptype);
    assert(fi.IsValid());
    exptype = tc.UseFlow(fi);
    lt = tc.PushBorrow(this);
    return this;
}

Node *Assign::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TT(left, 1, LT_BORROW);
    tc.DecBorrowers(left->lt, *this);
    tc.TT(right, 1, tc.LvalueLifetime(*left, false));
    tc.CheckLval(left);
    FlowItem fi(*left, left->exptype);
    if (fi.IsValid()) {
        left->exptype = tc.AssignFlowDemote(fi, right->exptype, true);
    }
    tc.SubType(right, left->exptype, "right", *this);
    if (fi.IsValid()) tc.AssignFlowPromote(*left, right->exptype);
    exptype = left->exptype;
    if (fi.IsValid()) exptype = tc.UseFlow(fi);
    lt = tc.PushBorrow(left);
    return this;
}

Node *DefaultVal::TypeCheck(TypeChecker &tc, size_t reqret) {
    // This is used as a default value for native call arguments. The variable
    // makes it equal to whatever the function expects, then codegen can use that type
    // to generate a correct value.
    // Also used as an empty else branch.
    exptype = reqret ? tc.NewTypeVar() : type_void;
    lt = LT_ANY;
    return this;
}

Node *GenericCall::TypeCheck(TypeChecker &tc, size_t reqret) {
    STACK_PROFILE;
    // Here we decide which of Dot / Call / NativeCall this call should be transformed into.
    tc.TypeCheckList(this, LT_ANY);
    auto nf = tc.parser.natreg.FindNative(name);
    auto fld = tc.st.FieldUse(name);
    TypeRef type;
    UDT *udt = nullptr;
    if (children.size()) {
        type = children[0]->exptype;
        if (IsUDT(type->t)) udt = type->udt;
    }
    Node *r = nullptr;
    if (fld && dotnoparens && udt && udt->Has(fld) >= 0) {
        auto dot = new Dot(fld, *this);
        dot->children = children;
        dot->TypeCheckSpecialized(tc, reqret);
        r = dot;
    } else {
        // See if any of sf's specializations matches type exactly, then it overrides nf.
        bool prefer_sf = false;
        if (sf && udt && sf->parent->nargs()) {
            for (auto sfi : sf->parent->overloads) {
                auto ti = sfi->args[0].type;
                if (IsUDT(ti->t) && ti->udt == udt) {
                    prefer_sf = true;
                    break;
                }
            }
        }
        if (nf && !prefer_sf) {
            auto nc = new NativeCall(nf, *this);
            nc->children = children;
            nc->TypeCheckSpecialized(tc, reqret);
            r = nc;
        } else if (sf) {
            auto fc = new Call(*this);
            fc->children = children;
            fc->TypeCheckSpecialized(tc, reqret);
            r = fc;
        } else {
            if (fld && dotnoparens) {
                tc.TypeError(cat("type \"", TypeName(type), "\" does not have field \"", fld->name,
                                 "\""), *this);
            }
            tc.TypeError("unknown field/function reference: " + name, *this);
        }
    }
    children.clear();
    delete this;
    return r;
}

void NativeCall::TypeCheckSpecialized(TypeChecker &tc, size_t /*reqret*/) {
    if (nf->first->overloads) {
        // Multiple overloads available, figure out which we want to call.
        auto cnf = nf->first;
        auto nargs = Arity();
        for (; cnf; cnf = cnf->overloads) {
            if (cnf->args.size() != nargs) continue;
            for (auto [i, arg] : enumerate(cnf->args)) {
                // Special purpose treatment of V_ANY to allow generic vectors in overloaded
                // length() etc.
                if (arg.type->t != V_ANY &&
                    (arg.type->t != V_VECTOR ||
                     children[i]->exptype->t != V_VECTOR ||
                     arg.type->sub->t != V_ANY) &&
                    !tc.ConvertsTo(children[i]->exptype,
                                   tc.ActualBuiltinType(arg.fixed_len, arg.type, arg.flags,
                                                        children[i], nf, true, i + 1, *this),
                                   arg.type->t != V_STRING, false, true)) goto nomatch;
            }
            nf = cnf;
            break;
            nomatch:;
        }
        if (!cnf)
            tc.NatCallError("arguments match no overloads of ", nf, *this);
    }
    if (children.size() != nf->args.size())
        tc.NatCallError("wrong number of many arguments for ", nf, *this);
    vector<TypeRef> argtypes(children.size());
    for (auto [i, c] : enumerate(children)) {
        auto &arg = nf->args[i];
        auto argtype = tc.ActualBuiltinType(arg.fixed_len, arg.type, arg.flags, children[i], nf, false, i + 1, *this);
        // Filter out functions that are not struct aware.
        bool typed = false;
        if (argtype->t == V_NIL && argtype->sub->Numeric() && !Is<DefaultVal>(c)) {
            // This is somewhat of a hack, because we conflate V_NIL with being optional
            // for native functions, but we don't want numeric types to be nilable.
            // Codegen has a special case for T_DEFAULTVAL however.
            argtype = argtype->sub;
        }
        if (arg.flags & NF_CONVERTANYTOSTRING && c->exptype->t != V_STRING) {
            tc.AdjustLifetime(c, LT_BORROW);  // MakeString wants to borrow.
            tc.MakeString(c, arg.lt);
            argtype = type_string;
            typed = true;
        }
        int flag = NF_SUBARG1;
        for (int sa = 0; sa < 3; sa++) {
            if (arg.flags & flag) {
                tc.SubType(c,
                        nf->args[sa].type->t == V_VECTOR && argtype->t != V_VECTOR
                            ? argtypes[sa]->sub
                            : argtypes[sa],
                        tc.ArgName(i),
                        nf->name);
                // Stop these generic params being turned into any by SubType below.
                typed = true;
            }
            flag *= 2;
        }
        if (arg.flags & NF_ANYVAR) {
            if (argtype->t == V_VECTOR)
                argtype = tc.st.Wrap(tc.NewTypeVar(), V_VECTOR);
            else if (argtype->t == V_ANY) argtype = tc.NewTypeVar();
            else assert(0);
        }
        if (argtype->t == V_ANY) {
            if (!arg.flags) {
                // Special purpose type checking to allow any reference type for functions like
                // copy/equal/hash etc. Note that this is the only place in the language where
                // we allow this!
                if (!IsRefNilNoStruct(c->exptype->t))
                    tc.TypeError("reference type", c->exptype, *c, nf->args[i].name, nf->name);
                typed = true;
            } else if (IsStruct(c->exptype->t) &&
                       !(arg.flags & NF_PUSHVALUEWIDTH) &&
                       c->exptype->udt->numslots > 1) {
                // Avoid unsuspecting generic functions taking values as args.
                // TODO: ideally this does not trigger for any functions.
                tc.TypeError("function does not support this struct type", *this);
            }
        }
        if (nf->fun.fnargs >= 0 && !arg.fixed_len && !(arg.flags & NF_PUSHVALUEWIDTH))
            tc.NoStruct(*c, nf->name);
        if (!typed)
            tc.SubType(c, argtype, tc.ArgName(i), nf->name);
        auto actualtype = c->exptype;
        if (actualtype->IsFunction()) {
            // We must assume this is going to get called and type-check it
            auto fsf = actualtype->sf;
            if (fsf->args.size()) {
                // we have no idea what args.
                tc.TypeError("function passed to \"" + nf->name +
                             "\" cannot take any arguments", *this);
            }
            auto chosen = fsf;
            List args(c->line);  // If any error, on same line as c.
            int vtable_idx = -1;
            tc.TypeCheckCall(fsf, args, false, vtable_idx, nullptr, false);
            assert(vtable_idx < 0);
            assert(fsf == chosen); (void)chosen;
        }
        argtypes[i] = actualtype;
        tc.StorageType(actualtype, *this);
        tc.AdjustLifetime(c, arg.lt);
        tc.DecBorrowers(c->lt, *this);
    }

    exptype = type_void;  // no retvals
    lt = LT_ANY;
    if (nf->retvals.size() > 1) exptype = tc.st.NewTuple(nf->retvals.size());
    for (auto [i, ret] : enumerate(nf->retvals)) {
        int sa = 0;
        auto type = ret.type;
        auto rlt = ret.lt;
        switch (ret.flags) {
            case NF_SUBARG3: sa++;
            case NF_SUBARG2: sa++;
            case NF_SUBARG1: {
                type = argtypes[sa];
                auto nftype = nf->args[sa].type;

                if (nftype->t == V_TYPEID) {
                    assert(!sa);  // assumes always first.
                    auto tin = AssertIs<TypeOf>(children[0]);
                    if (!Is<DefaultVal>(tin->child)) type = tin->child->exptype;
                }

                if (ret.type->t == V_NIL) {
                    if (!tc.st.IsNillable(type))
                        tc.TypeError(cat("argument ", sa + 1, " to \"", nf->name,
                                    "\" has to be a reference type"), *this);
                    type = tc.st.Wrap(type, V_NIL);
                } else if (nftype->t == V_VECTOR && ret.type->t != V_VECTOR) {
                    if (type->t == V_VECTOR) type = type->sub;
                } else if (nftype->t == V_FUNCTION) {
                    auto csf = type->sf;
                    assert(csf);
                    // In theory it is possible this hasn't been generated yet..
                    type = csf->returntype;
                }
                if (rlt == LT_BORROW) {
                    auto alt = nf->args[sa].lt;
                    assert(alt >= LT_BORROW);
                    rlt = alt;
                }
                break;
            }
            case NF_ANYVAR:
                type = ret.type->t == V_VECTOR ? tc.st.Wrap(tc.NewTypeVar(), V_VECTOR)
                                                  : tc.NewTypeVar();
                assert(rlt == LT_KEEP);
                break;
        }
        type = tc.ActualBuiltinType(ret.fixed_len, type, ret.flags,
                                    !i && Arity() ? children[0] : nullptr, nf, false,
                                    0, *this);
        if (!IsRefNilVar(type->t)) rlt = LT_ANY;
        if (nf->retvals.size() > 1) {
            exptype->Set(i, type.get(), rlt);
            lt = LT_MULTIPLE;
        } else {
            exptype = type;
            lt = rlt;
        }
    }

    if (nf->IsAssert()) {
        // Special case, add to flow:
        tc.CheckFlowTypeChanges(true, children[0]);
        // Also make result non-nil, if it was.
        if (exptype->t == V_NIL) exptype = exptype->Element();
    }

    nattype = exptype;
    natlt = lt;
}

void Call::TypeCheckSpecialized(TypeChecker &tc, size_t reqret) {
    STACK_PROFILE;
    sf = tc.PreSpecializeFunction(sf);
    exptype = tc.TypeCheckCall(sf, *this, reqret, vtable_idx, &specializers, super);
    lt = sf->ltret;
}

Node *FunRef::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    sf = tc.PreSpecializeFunction(sf);
    if (sf->parent->istype) {
        for (auto [i, arg] : enumerate(sf->args)) {
            arg.type = tc.ResolveTypeVars(sf->giventypes[i], this);
        }
        sf->returntype = tc.ResolveTypeVars(sf->returngiventype, this);
    }
    exptype = &sf->thistype;
    lt = LT_ANY;
    return this;
}

Node *DynCall::TypeCheck(TypeChecker &tc, size_t reqret) {
    tc.UpdateCurrentSid(sid);
    tc.TypeCheckId(sid);
    //if (sid->type->IsFunction()) sid->type = &tc.PreSpecializeFunction(sid->type->sf)->thistype;
    tc.TypeCheckList(this, LT_ANY);
    tie(exptype, lt) = tc.TypeCheckDynCall(sid, this, sf, reqret);
    return this;
}

Node *Return::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    exptype = type_void;
    lt = LT_ANY;
    // Ensure what we're returning from is going to be on the stack at runtime.
    for (auto isc : reverse(tc.scopes)) {
        if (isc.sf->parent == sf->parent) {
            sf = isc.sf;  // Take specialized version.
            goto destination_found;
        }
        tc.CheckReturnPast(isc.sf, sf, *this);
    }
    tc.TypeError(cat("return from \"", sf->parent->name, "\" called out of context"), *this);
    destination_found:
    // TODO: LT_KEEP here is to keep it simple for now, since ideally we want to also allow
    // LT_BORROW, but then we have to prove that we don't outlive the owner.
    // Additionally, we have to do this for reused specializations on new SpecIdents.
    auto reqlt = LT_KEEP;
    auto reqret = make_void ? 0 : sf->reqret;
    // Special (but very common) case: optimize lifetime for "return var" case, where var owns
    // and this is the only return statement. Without this we'd get an inc on the var that's
    // immediately undone as the scope ends.
    auto ir = Is<IdentRef>(child);
    if (ir) {
        tc.UpdateCurrentSid(ir->sid);  // Ahead of time, because ir not typechecked yet.
        if (ir->sid->lt == LT_KEEP &&
            IsRefNil(ir->sid->type->t) &&
            ir->sid->sf_def == sf &&
            sf->num_returns == 0 &&
            reqret &&
            sf->body->children.back() == this) {
            // NOTE: see also Call::Optimize where we potentially have to undo this when inlined.
            reqlt = LT_BORROW;  // Fake that we're cool borrowing this.
            sf->consumes_vars_on_return = true;  // Don't decref this one when going out of scope.
        }
    }
    tc.TT(child, reqret, reqlt);
    tc.DecBorrowers(child->lt, *this);
    if (sf == tc.st.toplevel) {
        // return from program
        if (child->exptype->NumValues() > 1)
            tc.TypeError("cannot return multiple values from top level", *this);
    }
    auto never_returns = child->Terminal(tc);
    if (never_returns && make_void && sf->num_returns) {
        // A return with other returns inside of it that always bypass this return,
        // so should not contribute to return types.
        if (child->exptype->t == V_VAR) tc.UnifyVar(type_void, child->exptype);
        assert(child->exptype->t == V_VOID);
        // Call this for correct counting of number of returns, with existing type, should
        // have no effect on the type.
        tc.RetVal(sf->returntype, sf, *this);
        return this;
    }
    if (never_returns && sf->reqret && sf->parent->anonymous) {
        // A return to the immediately enclosing anonymous function that needs to return a value
        // but is bypassed.
        tc.RetVal(child->exptype, sf, *this);  // If it's a variable, bind it.
        return this;
    }
    if (!Is<DefaultVal>(child)) {
        if (auto mrs = Is<MultipleReturn>(child)) {
            tc.RetVal(mrs->exptype, sf, *this);
            for (auto [i, mr] : enumerate(mrs->children)) {
                if (i < sf->reqret)
                    tc.SubType(mr, sf->returntype->Get(i), tc.ArgName(i), *this);
            }
        } else {
            tc.RetVal(child->exptype, sf, *this);
            tc.SubType(child, sf->returntype, "", *this);
        }
    } else {
        tc.RetVal(type_void, sf, *this);
        tc.SubType(child, sf->returntype, "", *this);
    }
    return this;
}

Node *TypeAnnotation::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    exptype = tc.ResolveTypeVars(giventype, this);
    lt = LT_ANY;
    return this;
}

Node *IsType::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TT(child, 1, LT_BORROW);
    tc.NoStruct(*child, "is");  // FIXME
    tc.DecBorrowers(child->lt, *this);
    resolvedtype = tc.ResolveTypeVars(giventype, this);
    exptype = &tc.st.default_bool_type->thistype;
    lt = LT_ANY;
    return this;
}

Node *Constructor::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckList(this, LT_KEEP);
    if (giventype.utr.Null()) {
        if (Arity()) {
            // No type was specified.. first find union of all elements.
            TypeRef u(nullptr);
            for (auto c : children) {
                u = u.Null() ? c->exptype : tc.Union(u, c->exptype, "constructor",
                                                     "constructor element", true, c);
            }
            exptype = tc.st.Wrap(u, V_VECTOR);
            tc.StorageType(exptype, *this);
        } else {
            // special case for empty vectors
            exptype = tc.st.Wrap(tc.NewTypeVar(), V_VECTOR);
        }
    } else {
        exptype = tc.ResolveTypeVars(giventype, this);
    }
    if (IsUDT(exptype->t)) {
        // We have to check this here, since the parser couldn't check this yet.
        if (exptype->udt->fields.size() < children.size())
            tc.TypeError("too many initializers for: " + exptype->udt->name, *this);
        auto udt = exptype->udt;
        if (!udt->FullyBound()) {
            // This is the generic type, try and find a matching named specialization.
            auto head = udt->first;
            assert(Arity() == head->fields.size());
            // Now find a match:
            int bestmatch = 0;
            for (auto udti = head->next; udti; udti = udti->next) {
                if (udti->unnamed_specialization) continue;
                int nmatches = 0;
                for (auto [i, arg] : enumerate(children)) {
                    auto &field = udti->fields[i];
                    if (tc.ConvertsTo(arg->exptype, field.resolvedtype, false, false)) nmatches++;
                    else break;
                }
                if (nmatches > bestmatch) {
                    bestmatch = nmatches;
                    udt = udti;
                }
            }
            if (udt == udt->first) {
                string s;
                for (auto &arg : children) s += " " + TypeName(arg->exptype);
                auto err = "generic constructor matches no named explicit specialization of " +
                           udt->first->name + " with types:" + s;
                for (auto udti = udt->first->next; udti; udti = udti->next) {
                    err += "\n  specialization: ";
                    err += Signature(*udti);
                }
                tc.TypeError(err, *this);
            }
        }
        exptype = &udt->thistype;
    }
    for (auto [i, c] : enumerate(children)) {
        TypeRef elemtype = IsUDT(exptype->t) ? exptype->udt->fields[i].resolvedtype
                                             : exptype->Element();
        tc.SubType(c, elemtype, tc.ArgName(i), *this);
    }
    lt = LT_KEEP;
    return this;
}

void Dot::TypeCheckSpecialized(TypeChecker &tc, size_t /*reqret*/) {
    tc.AdjustLifetime(children[0], LT_BORROW);
    tc.DecBorrowers(children[0]->lt, *this);  // New borrow created below.
    auto stype = children[0]->exptype;
    if (!IsUDT(stype->t))
        tc.TypeError("class/struct", stype, *this, "object");
    auto udt = stype->udt;
    auto fieldidx = udt->Has(fld);
    if (fieldidx < 0)
        tc.TypeError("type \"" + udt->name + "\" has no field named " + fld->name, *this);
    auto &field = udt->fields[fieldidx];
    if (field.isprivate && line.fileidx != field.defined_in.fileidx)
        tc.TypeError(cat("field ", field.id->name, " is private"), *this);
    exptype = field.resolvedtype;
    FlowItem fi(*this, exptype);
    if (fi.IsValid()) exptype = tc.UseFlow(fi);
    lt = tc.PushBorrow(this);
    //lt = children[0]->lt;  // Also LT_BORROW, also depending on the same variable.
}

Node *Indexing::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TT(object, 1, LT_BORROW);
    tc.TT(index, 1, LT_BORROW);
    tc.DecBorrowers(index->lt, *this);
    auto vtype = object->exptype;
    if (vtype->t != V_VECTOR &&
        vtype->t != V_STRING &&
        (!IsStruct(vtype->t) || !vtype->udt->sametype->Numeric()))
        tc.TypeError("vector/string/numeric struct", vtype, *this, "container");
    auto itype = index->exptype;
    switch (itype->t) {
        case V_INT:
            exptype = vtype->t == V_VECTOR
                ? vtype->Element()
                : (IsUDT(vtype->t) ? vtype->udt->sametype : type_int);
            break;
        case V_STRUCT_S: {
            if (vtype->t != V_VECTOR)
                tc.TypeError("multi-dimensional indexing on non-vector", *this);
            auto &udt = *itype->udt;
            exptype = vtype;
            for (auto &field : udt.fields) {
                if (field.resolvedtype->t != V_INT)
                    tc.TypeError("int field", field.resolvedtype, *this, "index");
                if (exptype->t != V_VECTOR)
                    tc.TypeError("nested vector", exptype, *this, "container");
                exptype = exptype->Element();
            }
            break;
        }
        default: tc.TypeError("int/struct of int", itype, *this, "index");
    }
    lt = object->lt;  // Also LT_BORROW, also depending on the same variable.
    return this;
}

Node *Seq::TypeCheck(TypeChecker &tc, size_t reqret) {
    tc.TT(head, 0, LT_ANY);
    tc.TT(tail, reqret, LT_ANY);
    exptype = tail->exptype;
    lt = tail->lt;
    return this;
}

Node *TypeOf::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TT(child, 1, LT_BORROW);
    tc.DecBorrowers(child->lt, *this);
    auto ti = tc.st.NewType();
    exptype = child->exptype->Wrap(ti, V_TYPEID);
    lt = LT_ANY;
    return this;
}

Node *EnumCoercion::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TT(child, 1, LT_BORROW);
    tc.SubType(child, type_int, "coerced value", *this);
    tc.DecBorrowers(child->lt, *this);
    exptype = &e->thistype;
    lt = LT_ANY;
    return this;
}

Node *MultipleReturn::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckList(this, LT_ANY);
    exptype = tc.st.NewTuple(children.size());
    for (auto [i, mrc] : enumerate(children))
        exptype->Set(i, mrc->exptype.get(), mrc->lt);
    lt = LT_MULTIPLE;
    return this;
}

Node *EnumRef::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/) {
    return this;
}

Node *UDTRef::TypeCheck(TypeChecker &tc, size_t /*reqret*/) {
    tc.TypeCheckUDT(*udt, *this);
    exptype = type_void;
    lt = LT_ANY;
    return this;
}

Node *Coercion::TypeCheck(TypeChecker &tc, size_t reqret) {
    // These have been added by another specialization. We could check if they still apply, but
    // even more robust is just to remove them, and let them be regenerated if need be.
    tc.TT(child, reqret, LT_ANY);
    return tc.DeleteCoercion(this);
}

bool Return::Terminal(TypeChecker &) const {
    return true;
}

bool Block::Terminal(TypeChecker &tc) const {
    return children.back()->Terminal(tc);
}

bool IfElse::Terminal(TypeChecker &tc) const {
    return truepart->Terminal(tc) && falsepart->Terminal(tc);
}

bool While::Terminal(TypeChecker &tc) const {
    // NOTE: if body is terminal, that does not entail the loop is, since
    // condition may be false on first iteration.
    // Instead, it is only terminal if this is an infinite loop.
    Value val;
    return condition->ConstVal(&tc, val) != V_VOID && val.True();
}

bool Break::Terminal(TypeChecker &) const {
    return true;
}

bool Switch::Terminal(TypeChecker &tc) const {
    auto have_default = false;
    for (auto c : cases->children) {
        auto cas = AssertIs<Case>(c);
        if (cas->pattern->children.empty()) have_default = true;
        if (!cas->body->Terminal(tc)) return false;
    }
    return have_default;
}

bool NativeCall::Terminal(TypeChecker &) const {
    // A function may end in "assert false" and have only its previous return statements
    // taken into account.
    if (nf->IsAssert()) {
        auto i = Is<IntConstant>(children[0]);
        return i && !i->integer;
    }
    return false;
}

bool Call::Terminal(TypeChecker &tc) const {
    // Have to be conservative for recursive calls since we're not done typechecking it.
    if (sf->isrecursivelycalled ||
        sf->method_of ||
        sf->parent->istype) return false;
    if (!sf->num_returns) return true;  // The minimum 1 return is apparently returning out of it.
    if (sf->num_returns == 1) {
        auto ret = AssertIs<Return>(sf->body->children.back());
        if (ret->sf == sf) {
            return ret->child->Terminal(tc);
        } else {
            // This is a "return from", which means the real return is elsewhere, and thus
            // this call may not be terminal.
        }
    }
    // TODO: could also check num_returns > 1, but then have to scan all children.
    return false;
}

}  // namespace lobster
