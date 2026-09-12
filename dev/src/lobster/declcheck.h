// Copyright 2026 Wouter van Oortmerssen. All rights reserved.
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

// The declaration checking pass, run between the parser and the typechecker.
// It establishes whole-program declaration-level facts that the parser cannot
// know (declarations may follow their uses in the source) and that the
// typechecker would otherwise discover in callgraph order, which makes both
// errors and behavior depend on the order things happen to get typechecked.
// Whatever this pass establishes, the typechecker may rely on blindly.
struct DeclChecker {
    SymbolTable &st;
    NativeRegistry &natreg;
    set<GUDT *> fields_completed;
    unordered_map<string_view, vector<Function *>> function_scopes;
    vector<Function *> function_stack;
    set<Overload *> bodies_walked;
    set<Function *> default_args_walked;
    set<GUDT *> field_defaults_walked;

    DeclChecker(SymbolTable &st, NativeRegistry &natreg) : st(st), natreg(natreg) {}

    // Errors don't stop this pass: they get collected (see Lex::Report), and every check
    // below is one whose failure leaves the declarations usable as they are. The typechecker
    // relies on the parser's guarantees, not on this pass having found nothing, and stops
    // compilation before the optimizer if anything was reported (see Compile).
    void Error(string_view msg, const Line &ln) {
        st.lex.Report(msg, &ln);
    }

    void Check() {
        // Inheritance cycles error at the moment they are formed (see
        // ResolveFields), so all superclass chains walked below terminate.
        for (auto gudt : st.gudttable) InheritSuperFields(gudt);
        // Now that every declared specialization has its final fields and
        // superclass, the whole inheritance forest can be registered, such
        // that dispatches typechecked anywhere see all (declared) subclasses,
        // regardless of where their declarations sit relative to the code
        // being typechecked.
        for (auto udt : st.udttable) st.RegisterSubUDT(udt);
        // Any mention of a non generic type that parsed as V_UUDT (uses of a
        // then pre-declared type, the self arg of methods) is from here on
        // indistinguishable from its single specialization, so overwrite it
        // in place, which updates every type that wraps or stores it (like
        // UnifyVar does for type variables). This makes V_UUDT mean "involves
        // unbound type variables" everywhere after this pass.
        auto collapse = [](UnType &ut) {
            if (ut.t != V_UUDT) return;
            auto gudt = ut.spec_udt->gudt;
            if (gudt->IsGeneric() || !ut.spec_udt->specializers.empty()) return;
            if (!gudt->first || gudt->predeclaration) return;
            *(Type *)&ut = *SingleNonGenericSpecialization(*gudt);
        };
        for (auto ut : st.untypelist) collapse(*ut);
        for (auto gudt : st.gudttable) collapse(gudt->unspecialized_type);
        FinalizeFamilies();
        for (auto f : st.functiontable) {
            if (f->anonymous || f->overloads.empty()) continue;
            auto &fs = st.functions_by_name[f->name];
            // One entry per name group (sibf chains the arity variants).
            if (std::find(fs.begin(), fs.end(), f->first) == fs.end()) fs.push_back(f->first);
        }
        // A serializable id identifies a single class to the (de)serializers, see
        // CodeGen::ser_ids.
        unordered_map<int, UDT *> ser_ids;
        for (auto udt : st.udttable) {
            if (udt->g.is_abstract || udt->serializable_id < 0) continue;
            if (!ser_ids.insert({ udt->serializable_id, udt }).second) {
                Error(cat(udt->name, " has \"attribute serializable\" with index that is already"
                          " in use: ", udt->serializable_id), udt->g.line);
            }
        }
        CheckMethodScopes();
        auto toplevel = st.toplevel->overload;
        if (toplevel->gbody) ResolveBlock(*toplevel->gbody);
        bodies_walked.insert(toplevel);
        // Anything unreachable from the top level lexical tree (which should
        // not currently exist, but error recovery may produce it).
        for (auto f : st.functiontable) {
            for (auto ov : f->overloads) WalkBody(ov);
        }
    }

    // Superclass fields are copied into subclasses at parse time, which
    // misses any fields the superclass only gained after the subclass was
    // parsed (i.e. subclassing a pre-declared superclass). Complete those
    // copies now that all declarations are known.
    void InheritSuperFields(GUDT *gudt) {
        if (!fields_completed.insert(gudt).second) return;
        auto sup = GetGUDTAny(gudt->gsuperclass);
        if (!sup) return;
        InheritSuperFields(sup);
        size_t k = 0;
        while (k < gudt->fields.size() && k < sup->fields.size() &&
               gudt->fields[k].id == sup->fields[k].id) {
            k++;
        }
        if (k == sup->fields.size()) return;
        for (size_t i = k; i < sup->fields.size(); i++) {
            auto &fld = sup->fields[i];
            // Inserted regardless, such that the field layout stays that of the superclass
            // followed by the subclass's own fields, which the typechecker assumes.
            if (gudt->Has(fld.id) >= 0)
                Error(cat("field ", Q(fld.id->name), " of ", Q(gudt->name),
                          " is also a field of superclass ", Q(sup->name)),
                      gudt->line);
            gudt->fields.insert(gudt->fields.begin() + i, Field(fld));
        }
        // Any specializations resolved against the old field list must be
        // redone from scratch. This is safe here since no sfield has a
        // typechecked defaultval yet.
        for (auto udt = gudt->first; udt; udt = udt->next) {
            udt->sfields.clear();
            udt->state = UDTState::DECLARED;
            st.ResolveFields(*udt, gudt->line);
        }
    }

    // Whether a field of this declared type may hold a reference in some specialization:
    // anything but a definitely scalar type, including one still to be inferred.
    static bool MayBeRef(UnTypeRef type) {
        switch (type->t) {
            case V_INT:
            case V_FLOAT:
            case V_FUNCTION:
                return false;
            case V_STRUCT_S: {
                // A member of a family that may still turn out to hold references, see
                // the fixpoint in FinalizeFamilies.
                auto root = FamilyRootOf(&type->udt->g);
                return root && root->family_hasref;
            }
            default:
                // A generic struct (V_UUDT) can be given reference type arguments.
                return true;
        }
    }

    // Decides for every abstract struct family whether it is a family of structs of
    // references (see GUDT::family_hasref), which makes every specialization in it one,
    // as well as every struct that (transitively) has a field of such a type, whose
    // kind was decided by the parser before that was known.
    void FinalizeFamilies() {
        // Over declarations, since a generic member's field may hold a reference in one
        // specialization and not in another, and a family is one kind in all of them.
        // A nested family value counts once its own family is known to hold references.
        for (bool changed = true; changed;) {
            changed = false;
            for (auto gudt : st.gudttable) {
                auto root = FamilyRootOf(gudt);
                if (!root || root->family_hasref) continue;
                for (auto &f : gudt->fields) {
                    if (MayBeRef(f.giventype)) {
                        root->family_hasref = true;
                        changed = true;
                        break;
                    }
                }
            }
        }
        for (bool changed = true; changed;) {
            changed = false;
            for (auto udt : st.udttable) {
                if (!udt->g.is_struct) continue;
                // The root's declaration may have followed the member's (a pre-declared
                // struct that was then declared abstract).
                udt->family_root = FamilyRootOf(udt);
                if (udt->family_root) udt->sametype = type_undefined;
                if (udt->UpdateStructType()) changed = true;
            }
        }
    }

    // Resolve what the name of every GenericCall can refer to, walking all
    // code in lexical nesting order. Functions are visible in the whole
    // block they are declared in (also before their declaration statement)
    // and everything lexically nested in it, shadowing outer ones, which is
    // what the typechecker used to approximate with its scope emulation
    // during callgraph-order traversal (which also leaked functions to
    // lexically unrelated code that happened to be typechecked from inside
    // their scope). Which candidate applies is type-dependent and still
    // decided during typechecking; a name with no candidates at all is an
    // error here, which covers code the typechecker never reaches.

    Function *FindLexical(string_view name, string_view ns) {
        if (!ns.empty() && name.find(".") == string_view::npos) {
            auto it = function_scopes.find(cat(ns, ".", name));
            if (it != function_scopes.end() && !it->second.empty()) return it->second.back();
        }
        auto it = function_scopes.find(name);
        if (it != function_scopes.end() && !it->second.empty()) return it->second.back();
        return nullptr;
    }

    bool RelatedGUDT(GUDT *a, GUDT *b) {
        for (auto g = a; g; g = GetGUDTAny(g->gsuperclass)) if (g == b) return true;
        for (auto g = b; g; g = GetGUDTAny(g->gsuperclass)) if (g == a) return true;
        return false;
    }

    // Methods only participate in overloading/dispatch with methods of the
    // same name declared in the same scope: name resolution is lexical, so a
    // method in a nested scope shadows rather than joins. When that splits
    // methods of related classes over different scopes, dispatch would
    // silently pick from only one of them, so make it an error.
    void CheckMethodScopes() {
        for (auto f : st.functiontable) {
            // Top level functions are at scopelevel 2 (1 is the file scope).
            if (f->anonymous || f->overloads.empty() || f->scopelevel <= 2) continue;
            for (auto ov : f->overloads) {
                auto g0 = ov->method_of;
                if (!g0) continue;
                auto it = st.functions_by_name.find(f->name);
                if (it == st.functions_by_name.end()) continue;
                for (auto of : it->second) {
                    if (of == f->first) continue;
                    for (auto off = of; off; off = off->sibf) {
                        if (off->nargs() != f->nargs()) continue;
                        for (auto oov : off->overloads) {
                            // Only a strict super/sub relation splits a
                            // dispatch; a method on the exact same class in
                            // another scope is ordinary shadowing (and if
                            // subclass overrides exist anywhere, those are
                            // strictly related and still get here).
                            if (oov->method_of && oov->method_of != g0 &&
                                RelatedGUDT(g0, oov->method_of)) {
                                Error(cat("method ", Q(f->name), " of ", Q(g0->name),
                                    " is in a nested scope, but a method of this name on"
                                    " related type ", Q(oov->method_of->name),
                                    " exists in another scope; methods must be declared in"
                                    " the same scope to dispatch together"),
                                    ov->declared_at);
                            }
                        }
                    }
                }
            }
        }
    }

    // Does any function of this name exist at all, in any scope?
    bool FunctionExists(string_view name, string_view ns) {
        if (!ns.empty() && name.find(".") == string_view::npos) {
            if (st.functions_by_name.find(cat(ns, ".", name)) != st.functions_by_name.end())
                return true;
        }
        return st.functions_by_name.find(name) != st.functions_by_name.end();
    }

    NativeFun *FindNativeNS(string_view name, string_view ns) {
        if (!ns.empty() && name.find(".") == string_view::npos) {
            auto nf = natreg.FindNative(cat(ns, ".", name));
            if (nf) return nf;
        }
        return natreg.FindNative(name);
    }

    void ResolveCall(GenericCall &call) {
        call.cand_function = FindLexical(call.name, call.ns);
        call.cand_native = FindNativeNS(call.name, call.ns);
        call.cand_field = st.FieldUse(call.name);
        // A variable in scope at the call (cand_var) is a candidate the parser found.
        if (!call.cand_function && !call.cand_native && !call.cand_field && !call.cand_var) {
            // Local functions can additionally be called from outside their
            // lexical scope while their enclosing function is active
            // ("functions as environments"); which one applies depends on the
            // call path, so the typechecker picks (see GenericCall).
            if (FunctionExists(call.name, call.ns)) {
                call.cand_nonlexical = true;
            } else {
                // The call stays without candidates, which is how GenericCall::TypeCheck
                // knows this was reported.
                Error(cat("unknown field/function reference ", Q(call.name)), call.line);
            }
        }
    }

    void WalkBody(Overload *ov) {
        if (!ov->gbody) return;
        if (!bodies_walked.insert(ov).second) return;
        ResolveBlock(*ov->gbody);
    }

    void ResolveBlock(Block &b) {
        // All functions declared directly in this block are visible in the
        // whole of it.
        auto stack_level = function_stack.size();
        for (auto c : b.children) {
            auto fr = Is<FunRef>(c);
            if (!fr || fr->sf->parent->anonymous) continue;
            auto f = fr->sf->parent->first;
            auto &fscope = function_scopes[f->name];
            if (fscope.empty() || fscope.back() != f) {
                fscope.push_back(f);
                function_stack.push_back(f);
            }
        }
        for (auto c : b.children) ResolveRec(*c);
        while (function_stack.size() > stack_level) {
            function_scopes[function_stack.back()->name].pop_back();
            function_stack.pop_back();
        }
    }

    void ResolveRec(Node &n) {
        if (auto fr = Is<FunRef>(&n)) {
            // The definition site: resolve the body (and one-time extras)
            // in the scope it is declared in.
            auto f = fr->sf->parent;
            if (default_args_walked.insert(f).second) {
                for (auto da : f->default_args) {
                    if (da) ResolveRec(*da);
                }
            }
            WalkBody(fr->sf->overload);
            return;
        }
        if (auto gr = Is<GUDTRef>(&n)) {
            // Only this class's own defaults belong to its declaration scope. Inherited
            // defaults share their superclass's expression, so receive its bindings there.
            if (!gr->predeclaration && field_defaults_walked.insert(gr->gudt).second) {
                auto sup = GetGUDTAny(gr->gudt->gsuperclass);
                auto inherited = sup ? sup->fields.size() : 0;
                for (size_t i = inherited; i < gr->gudt->fields.size(); i++) {
                    auto &field = gr->gudt->fields[i];
                    if (field.gdefaultval) ResolveRec(*field.gdefaultval);
                }
            }
            return;
        }
        if (auto call = Is<GenericCall>(&n)) {
            ResolveCall(*call);
        } else if (auto blk = Is<Block>(&n)) {
            ResolveBlock(*blk);
            return;
        }
        for (size_t i = 0; i < n.Arity(); i++) {
            ResolveRec(*n.Children()[i]);
        }
    }
};

}  // namespace lobster
