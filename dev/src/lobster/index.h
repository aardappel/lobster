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

// The definitions of a program and where each is used, as JSON for editors (see --index), from
// the typechecked program: what a call or a field access refers to depends on types.
//
//   { "files": [ name, ... ],
//     "defs": [ { "k": kind, "n": name, "f": file, "l": line, ... }, ... ],
//     "refs": [ [ file, line, def ], ... ] }
//
// Kinds are "function" (one per overload, with "e" the last line of its body, "t" its declared
// signature and "c" the class it is a method of), "lambda" (a function value, with "e"), "var"
// ("t" its type, "c" the function or lambda it is local to, none for a global), "class",
// "struct", "field" ("c" its class, "t" its type), "enum" and "enumval" ("c" its enum). Lines
// are 1 based. Uses only have lines: an editor finds the name on the line.
// FIXME: the body of a function nothing calls isn't typechecked, so in it a call whose overload
// depends on types refers to the function's first overload, and field accesses aren't known.

namespace lobster {

struct Indexer {
    SymbolTable &st;
    const vector<pair<string, string>> &filenames;
    string defs, refs;
    int numdefs = 0;
    map<const void *, int> ids;
    set<std::tuple<int, int, int>> seen;

    Indexer(SymbolTable &st, const vector<pair<string, string>> &filenames)
        : st(st), filenames(filenames) {}

    static string Str(string_view s) {
        string r;
        flatbuffers::EscapeString(s.data(), s.size(), &r, false, false);
        return r;
    }

    int Id(const void *what) {
        auto it = ids.find(what);
        return it == ids.end() ? -1 : it->second;
    }

    // Adds a definition of `what`, with the JSON fields after its kind and name in `extra`.
    int Def(const void *what, string_view kind, string_view name, const Line &line,
            const string &extra = {}) {
        auto id = numdefs++;
        ids[what] = id;
        if (!defs.empty()) defs += ",\n";
        append(defs, "{\"k\":\"", kind, "\",\"n\":", Str(name), ",\"f\":", line.fileidx, ",\"l\":",
               line.line, extra, "}");
        return id;
    }

    void Ref(const Line &line, const void *what) {
        auto id = Id(what);
        if (id < 0 || !seen.insert({ line.fileidx, line.line, id }).second) return;
        if (!refs.empty()) refs += ",";
        append(refs, "[", line.fileidx, ",", line.line, ",", id, "]");
    }

    static string Field(string_view name, int id) {
        return id < 0 ? string() : cat(",\"", name, "\":", id);
    }

    void Uses(Node *body) {
        body->Iterate([&](Node *n) {
            if (auto ir = Is<IdentRef>(n)) {
                Ref(n->line, ir->sid->id);
            } else if (auto c = Is<Call>(n)) {
                if (!c->failed) Ref(n->line, c->sf->overload);
            } else if (auto fr = Is<FunRef>(n)) {
                Ref(n->line, fr->sf->overload);
            } else if (auto gc = Is<GenericCall>(n)) {
                if (gc->cand_function) Ref(n->line, gc->cand_function->overloads[0]);
            } else if (auto d = Is<Dot>(n)) {
                auto type = d->child->exptype;
                if (!type.Null() && IsUDT(type->t)) {
                    auto &g = type->udt->g;
                    auto fi = g.Has(d->fld);
                    if (fi >= 0) Ref(n->line, &g.fields[fi]);
                }
            } else if (auto oc = Is<ObjectConstructor>(n)) {
                auto type = oc->exptype;
                if (!type.Null() && IsUDT(type->t)) Ref(n->line, &type->udt->g);
            } else if (auto ic = Is<IntConstant>(n)) {
                if (ic->from) Ref(n->line, ic->from);
            }
        });
    }

    // Whether a variable has a type: those of a function nothing calls, or a function value
    // nothing calls, have none.
    static bool Typed(SpecIdent *sid) {
        auto &type = sid->type;
        return !type.Null() && type->t != V_UNDEFINED &&
               (type->t != V_FUNCTION || type->sf->typechecked);
    }

    string Index() {
        auto toplevel = st.toplevel->overload;
        // Types first: functions and fields refer to them.
        for (auto g : st.gudttable) {
            auto gid = Def(g, g->is_struct ? "struct" : "class", g->name, g->line);
            for (auto &f : g->fields) {
                Def(&f, "field", f.id->name, f.defined_in,
                    cat(",\"t\":", Str(TypeName(f.giventype)), Field("c", gid)));
            }
        }
        for (auto c : st.toplevel->sbody->children) {
            auto er = Is<EnumRef>(c);
            if (!er) continue;
            auto eid = Def(er->e, "enum", er->e->name, c->line);
            for (auto &v : er->e->vals) Def(v.get(), "enumval", v->name, v->line, Field("c", eid));
        }
        for (auto f : st.functiontable) {
            if (f->istype) continue;
            for (auto ov : f->overloads) {
                if (!ov->gbody || ov == toplevel) continue;
                auto [first, last] = Lines(*ov);
                if (f->anonymous) {
                    Def(ov, "lambda", "", ov->declared_at, cat(",\"e\":", last));
                    continue;
                }
                Def(ov, "function", f->name, ov->declared_at,
                    cat(",\"e\":", last, ",\"t\":", Str(DeclSignature(*ov)),
                        Field("c", Id(ov->method_of))));
            }
        }
        // Variables, with their type in a typechecked specialization of the function they're
        // in, if any.
        vector<Ident *> order;
        map<Ident *, pair<SpecIdent *, Overload *>> vars;
        for (auto sf : st.subfunctiontable) {
            for (auto args : { &sf->args, &sf->locals }) {
                for (auto &a : *args) {
                    auto &v = vars[a.sid->id];
                    if (!v.first) order.push_back(a.sid->id);
                    if (!v.first || (!Typed(v.first) && Typed(a.sid))) v = { a.sid, sf->overload };
                }
            }
        }
        for (auto id : order) {
            auto [sid, ov] = vars[id];
            Def(id, "var", id->name, id->line,
                cat(Typed(sid) ? ",\"t\":" + Str(TypeName(sid->type)) : string(),
                    Field("c", ov == toplevel ? -1 : Id(ov))));
        }
        // Uses, in what was typechecked, or for a function nothing calls, in its declaration.
        for (auto sf : st.subfunctiontable) {
            if (sf->typechecked && sf->sbody) Uses(sf->sbody);
        }
        for (auto f : st.functiontable) {
            for (auto ov : f->overloads) {
                if (ov->gbody && !ov->Typechecked()) Uses(ov->gbody);
            }
        }
        string files;
        for (auto &fn : filenames) {
            if (!files.empty()) files += ",";
            files += Str(fn.first);
        }
        return cat("{\"files\":[", files, "],\n\"defs\":[\n", defs, "],\n\"refs\":[", refs,
                   "]}\n");
    }
};

}  // namespace lobster
