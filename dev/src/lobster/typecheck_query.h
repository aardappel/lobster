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

// Definition queries.
// This topic depends only on TypeCheckBase.

namespace lobster {

struct TypeCheckQuery : virtual TypeCheckBase {
    TypeCheckQuery() {}

    // The first definition found answers the query.
    void LocationQuery(Line &line, string_view type) {
        if (!query_result.empty()) return;
        query_result = cat("query_location: ", (*query->filenames)[line.fileidx].second, " ",
                           line.line, " ", type);
    }

    // Whether a declared name is the queried identifier, either as is or in a namespace (a
    // variable declared at file scope in a namespace, which its own file refers to
    // unqualified).
    static bool QueryNameMatches(string_view name, string_view iden) {
        if (name == iden) return true;
        return name.size() > iden.size() && name[name.size() - iden.size() - 1] == '.' &&
               name.substr(name.size() - iden.size()) == iden;
    }
    void FindVar(vector<Arg> &vars) {
        for (auto &var : vars) {
            if (QueryNameMatches(var.sid->id->name, query->iden)) {
                LocationQuery(var.sid->id->line, TypeName(var.sid->type));
            }
        }
    }
    TypeRef FindVarType(string_view ident, SubFunction *sf) {
        for (auto &vars : {sf->args, sf->locals, sf->freevars}) {
            for (auto &var : vars) {
                if (var.sid->id->name == ident && !var.sid->type.Null()) {
                    return var.sid->type;
                }
            }
        }
        return TypeRef(nullptr);
    }

    bool ProcessDefinition(GUDT *parent, string full_iden, SubFunction *sf) {
        size_t pos = full_iden.find('.');
        bool got_pos = pos != std::string::npos;
        string ident = full_iden;
        if (got_pos) {
            ident = full_iden.substr(0, pos);
            //Possible a class or a struct name
            auto ident_type = FindVarType(ident, sf);
            if (!ident_type.Null()) {
                ident = TypeName(ident_type);
            }
        }
        auto new_parent_struct = st.LookupStructQuery(ident);

        if (new_parent_struct && !got_pos){ //Just class instance
            LocationQuery(new_parent_struct->line, Signature(*new_parent_struct));
        }

        // FIXME: may not work when namespaces are involved.
        auto f = st.FindFunctionAnywhere(full_iden);
        if (f) {
            auto ov = f->overloads[0];
            if(parent) { //Try to find method of parent class with same name
                for (auto &candidate_ov : f->overloads) {
                    if (TypeName(candidate_ov->givenargs[0]) == parent->name) {
                        ov = candidate_ov;
                        break;
                    }
                }
            }
            if (ov->gbody) {  // FIXME: ignores function types. Now fixed (or not?)
                LocationQuery(ov->gbody->line, ov->sf ? Signature(*ov->sf) : "");
            }
        }
        auto fld = st.FieldUse(full_iden);
        if (fld && parent) {
            // To know what this belongs to, would need to find the object it belongs to.
            // For now, simply see if we can find any class that has this field.
            int fi = parent->Has(fld);
            if (fi >= 0) {
                auto struct_type = st.LookupStructQuery(TypeName(parent->fields[fi].giventype));
                if (got_pos) { //Go further with detected struct as a parent
                    ProcessDefinition(struct_type, full_iden.substr(pos+1), sf);
                }
                LocationQuery(parent->fields[fi].defined_in, TypeName(parent->fields[fi].giventype));
            }
        }
        auto nf = parser.natreg.FindNative(full_iden);
        if (nf && query_result.empty()) {
            // This doesn't have a source code location, so output a signature the IDE can display.
            query_result = "query_signature: " + Signature(*nf);
        }
        if (fld) { //Failed to find field in parent or no parent
            for (auto gudt : st.gudttable) {
                int fi = gudt->Has(fld);
                if (fi >= 0) {
                    // FIXME: this is really basic, lets at least find the field line.
                    LocationQuery(gudt->line, TypeName(gudt->fields[fi].giventype));
                }
            }
        }
        if (got_pos) { //Go further
            ProcessDefinition(new_parent_struct, full_iden.substr(pos+1), sf);
        }
        return !query_result.empty();
    }

    // Whether the query has its answer (in query_result) after this.
    bool ProcessQuery() override {
        if (!query_result.empty()) return true;
        if (query->kind == "definition") {
            // The top scope includes a list of free vars so should be able to resolve any var
            // at the given location.. if no scopes, use top fun.
            auto sf = scopes.empty() ? st.toplevel : scopes.back().sf;
            FindVar(sf->args);
            FindVar(sf->locals);
            FindVar(sf->freevars);
            return ProcessDefinition(nullptr, query->iden, sf);
        } else {
            query_result = "query_unknown_kind: " + query->kind;
            return true;
        }
    }
};

}  // namespace lobster
