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
    set<string_view> function_names;

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
        CheckCallNamesExist();
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

    // A call to a name that exists nowhere in the program can now be
    // reported even when it sits in code the typechecker never reaches.
    // This is existence only: which function/field/builtin applies (and
    // whether it is in scope) is still decided during typechecking, so a
    // name that exists anywhere at all passes here.
    void CheckCallNamesExist() {
        for (auto f : st.functiontable) function_names.insert(f->name);
        for (auto f : st.functiontable) {
            for (auto ov : f->overloads) {
                if (ov->gbody) CheckCallNamesRec(*ov->gbody);
            }
            for (auto da : f->default_args) {
                if (da) CheckCallNamesRec(*da);
            }
        }
        for (auto gudt : st.gudttable) {
            for (auto &field : gudt->fields) {
                if (field.gdefaultval) CheckCallNamesRec(*field.gdefaultval);
            }
        }
    }

    bool CallNameExists(string_view name, string_view ns) {
        if (function_names.find(name) != function_names.end()) return true;
        if (natreg.FindNative(name)) return true;
        if (st.FieldUse(name)) return true;
        if (!ns.empty() && name.find(".") == string_view::npos) {
            auto nsname = cat(ns, ".", name);
            if (function_names.find(string_view(nsname)) != function_names.end()) return true;
            if (natreg.FindNative(nsname)) return true;
        }
        return false;
    }

    void CheckCallNamesRec(Node &n) {
        if (auto call = Is<GenericCall>(&n)) {
            if (!CallNameExists(call->name, call->ns)) {
                st.lex.Error(cat("unknown field/function reference ", Q(call->name)),
                             &call->line);
            }
        }
        for (size_t i = 0; i < n.Arity(); i++) {
            CheckCallNamesRec(*n.Children()[i]);
        }
    }
};

}  // namespace lobster
