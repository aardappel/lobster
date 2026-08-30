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
    unordered_map<string_view, vector<Function *>> all_functions;
    vector<Function *> function_stack;
    set<Overload *> bodies_walked;
    set<Function *> default_args_walked;
    set<GUDT *> field_defaults_walked;

    DeclChecker(SymbolTable &st, NativeRegistry &natreg) : st(st), natreg(natreg) {}

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
        for (auto f : st.functiontable) {
            if (f->anonymous || f->overloads.empty()) continue;
            auto &fs = st.functions_by_name[f->name];
            // One entry per name group (sibf chains the arity variants).
            if (std::find(fs.begin(), fs.end(), f->first) == fs.end()) fs.push_back(f->first);
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
        auto sup = GetGUDTSuper(gudt->gsuperclass);
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
            if (gudt->Has(fld.id) >= 0)
                st.lex.Error(cat("field ", Q(fld.id->name), " of ", Q(gudt->name),
                                 " is also a field of superclass ", Q(sup->name)),
                             &gudt->line);
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
        for (auto g = a; g; g = GetGUDTSuper(g->gsuperclass)) if (g == b) return true;
        for (auto g = b; g; g = GetGUDTSuper(g->gsuperclass)) if (g == a) return true;
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
                                st.lex.Error(cat("method ", Q(f->name), " of ", Q(g0->name),
                                    " is in a nested scope, but a method of this name on"
                                    " related type ", Q(oov->method_of->name),
                                    " exists in another scope; methods must be declared in"
                                    " the same scope to dispatch together"),
                                    &ov->declared_at);
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
        if (!call.cand_function && !call.cand_native && !call.cand_field) {
            // Local functions can additionally be called from outside their
            // lexical scope while their enclosing function is active
            // ("functions as environments"); which one applies depends on the
            // call path, so the typechecker picks (see GenericCall).
            if (FunctionExists(call.name, call.ns)) {
                call.cand_nonlexical = true;
            } else {
                st.lex.Error(cat("unknown field/function reference ", Q(call.name)),
                             &call.line);
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
            // Field default values belong to the scope the class is declared
            // in (they don't have access to other members).
            if (!gr->predeclaration && field_defaults_walked.insert(gr->gudt).second) {
                for (auto &field : gr->gudt->fields) {
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
