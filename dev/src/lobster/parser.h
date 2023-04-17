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

struct Parser {
    NativeRegistry &natreg;
    Lex &lex;
    Node *root = nullptr;
    SymbolTable &st;
    vector<Function *> functionstack;
    vector<UDT *> udtstack;
    bool call_noparens = false;
    set<string> pakfiles;
    struct BlockScope {
        Block *block;
        int for_nargs;
        int implicits;
    };
    vector<BlockScope> block_stack;

    Parser(NativeRegistry &natreg, Lex &lex, SymbolTable &st)
        : natreg(natreg), lex(lex), st(st) {}

    ~Parser() {
        delete root;
    }

    template<typename... Ts> void Error(const Ts &...args) {
        lex.Error(cat(args...), nullptr);
    }

    template<typename... Ts> void ErrorAt(const Node *what, const Ts &...args) {
        lex.Error(cat(args...), what ? &what->line : nullptr);
    }

    template<typename... Ts> void Warn(const Ts &...args) {
        lex.Warn(cat(args...), nullptr);
    }

    template<typename... Ts> void WarnAt(const Node *what, const Ts &...args) {
        lex.Warn(cat(args...), what ? &what->line : nullptr);
    }

    void Parse() {
        auto sf = st.FunctionScopeStart();
        st.toplevel = sf;
        auto &f = st.CreateFunction("__top_level_expression");
        f.overloads.emplace_back(new Overload { lex, false });
        auto &ov = *f.overloads[0];
        sf->SetParent(f, ov);
        f.anonymous = true;
        lex.Include("stdtype.lobster", false);
        ov.gbody = new Block(lex);
        ParseStatements(ov.gbody, T_ENDOFFILE);
        ImplicitReturn(ov);
        st.FunctionScopeCleanup();
        root = new Call(lex, sf);
    }

    void ParseStatements(Block *block, TType terminator) {
        for (;;) {
            #ifdef USE_EXCEPTION_HANDLING
                if (lex.max_errors > 1) {
                    // We're going to attempt error recovery by catching parsing errors!
                    try {
                        ParseTopExp(block);
                    } catch (string &s) {
                        if (lex.num_errors >= lex.max_errors) {
                            THROW_OR_ABORT(s);
                        }
                        LOG_ERROR(s);
                        // Consume rest of statement tokens in the hope next statement is
                        // again parseable.
                        while (!Either(T_LINEFEED, T_DEDENT, T_ENDOFFILE, T_ENDOFINCLUDE))
                            lex.Next();
                    }
                } else {
                    ParseTopExp(block);
                }
            #else
                ParseTopExp(block);
            #endif
            if (lex.token == T_ENDOFINCLUDE) {
                st.EndOfInclude();
                lex.PopIncludeContinue();
            } else if (!IsNext(T_LINEFEED)) {
                break;
            }
            if (Either(T_ENDOFFILE, T_DEDENT)) break;
        }
        Expect(terminator);
        auto b = block->children.back();
        if (Is<EnumRef>(b) || Is<UDTRef>(b) || Is<FunRef>(b) || Is<Define>(b)) {
            if (terminator == T_ENDOFFILE) block->Add(new IntConstant(lex, 0));
            else Error("last expression in list can\'t be a definition");
        }
        CleanupStatements(block);
    }

    void CleanupStatements(Block *list) {
        // See also Block::TypeCheck
        for (auto def : list->children) {
            if (auto er = Is<EnumRef>(def)) {
                st.Unregister(er->e, st.enums);
            } else if (auto sr = Is<UDTRef>(def)) {
                if (sr->udt->predeclaration)
                    Error("pre-declared struct ", Q(sr->udt->name), " never defined");
                st.Unregister(sr->udt, st.udts);
            } else if (auto fr = Is<FunRef>(def)) {
                auto f = fr->sf->parent;
                if (!f->anonymous) st.Unregister(f);
            } else if (auto d = Is<Define>(def)) {
                // For now, don't warn when declaring multiple vars and they are mixed const,
                // since there is no way to use var for one and let for the other.
                bool warn_all = true;
                for (auto p : d->sids) {
                    auto id = p.first->id;
                    id->static_constant =
                        id->single_assignment && d->child->IsConstInit();
                    if (!id->single_assignment || id->constant || d->line.fileidx != 0)
                        warn_all = false;
                    if (!id->read && !id->static_constant && id->scopelevel != 1 &&
                        (id->name[0] != '_' || d->sids.size() == 1))
                        Warn("unused variable ", Q(id->name));
                    if (id->predeclaration)
                        Error("missing declaration for ", id->name);
                }
                if (warn_all) {
                    for (auto p : d->sids) {
                        WarnAt(d, "use ", Q("let"), " to declare ", Q(p.first->id->name));
                    }
                }
            } else if (auto r = Is<Return>(def)) {
                if (r != list->children.back())
                    Error("return must be last in block");
            }
        };
    }

    void ParseTopExp(Block *list, bool isprivate = false) {
        switch(lex.token) {
            case T_NAMESPACE:
                if (st.scopelevels.size() != 1 || isprivate)
                    Error("namespace must be used at file scope");
                lex.Next();
                st.current_namespace = lex.sattr;
                Expect(T_IDENT);
                break;
            case T_PRIVATE:
                if (st.scopelevels.size() != 1 || isprivate)
                    Error("private must be used at file scope");
                lex.Next();
                ParseTopExp(list, true);
                break;
            case T_INCLUDE: {
                if (isprivate)
                    Error("import cannot be private");
                lex.Next();
                if (IsNext(T_FROM)) {
                    string fn = std::move(lex.sval);
                    Expect(T_STR);
                    AddDataDir(fn);
                } else {
                    string fn;
                    if (lex.token == T_STR) {
                        fn = std::move(lex.sval);
                        lex.Next();
                    } else {
                        fn = lex.sattr;
                        Expect(T_IDENT);
                        while (IsNext(T_DOT)) {
                            fn += "/";
                            fn += lex.sattr;
                            Expect(T_IDENT);
                        }
                        fn += ".lobster";
                    }
                    Expect(T_LINEFEED);
                    lex.Include(fn);
                    ParseTopExp(list);
                }
                break;
            }
            case T_STRUCT:
                ParseTypeDecl(true,  isprivate, list);
                break;
            case T_CLASS:
                ParseTypeDecl(false, isprivate, list);
                break;
            case T_FUN: {
                lex.Next();
                list->Add(ParseNamedFunctionDefinition(isprivate, nullptr));
                break;
            }
            case T_ENUM:
            case T_ENUM_FLAGS: {
                bool incremental = lex.token == T_ENUM;
                lex.Next();
                Line line = lex;
                int64_t cur = incremental ? 0 : 1;
                auto enumname = st.MaybeNameSpace(ExpectId(), true);
                auto def = st.EnumLookup(enumname, true);
                def->isprivate = isprivate;
                def->flags = !incremental;
                Expect(T_COLON);
                Expect(T_INDENT);
                for (;;) {
                    auto evname = st.MaybeNameSpace(ExpectId(), true);
                    if (IsNext(T_ASSIGN)) {
                        auto e = ParseExp();
                        Value val = NilVal();
                        auto t = e->ConstVal(nullptr, val);
                        delete e;
                        if (t != V_INT)
                            Error("enum value expression must evaluate to an integer constant");
                        cur = val.ival();
                    }
                    auto ev = st.EnumValLookup(evname, true);
                    ev->isprivate = isprivate;
                    ev->val = cur;
                    ev->e = def;
                    def->vals.emplace_back(ev);
                    if (incremental) cur++; else cur *= 2;
                    if ((!IsNext(T_LINEFEED) && !IsNext(T_COMMA)) ||
                        Either(T_ENDOFFILE, T_DEDENT)) break;
                }
                Expect(T_DEDENT);
                list->Add(new EnumRef(line, def));
                break;
            }
            case T_VAR:
            case T_CONST: {
                auto isconst = lex.token == T_CONST;
                lex.Next();
                auto def = new Define(lex, nullptr);
                bool has_predeclaration_init = false;
                for (;;) {
                    auto idname = ExpectId();
                    bool withtype = lex.token == T_TYPEIN;
                    UnresolvedTypeRef type = { nullptr };
                    if (lex.token == T_COLON || withtype) {
                        lex.Next();
                        type = ParseType(withtype);
                    }
                    auto id = st.LookupDef(idname, true, withtype);
                    if (id->predeclaration) {
                        if (type.utr.Null() || !type.utr->Equal(*id->giventype.utr))
                            Error("must specify same type as pre-declaration");
                        if (isconst != id->constant)
                            Error("let/var doesn\'t match pre-declaration");
                        if (isprivate != id->isprivate)
                            Error("private doesn\'t match pre-declaration");
                        has_predeclaration_init = true;
                        id->predeclaration = false;
                    }
                    if (isconst)  id->constant = true;
                    if (isprivate) id->isprivate = true;
                    def->sids.push_back({ id->cursid, type });
                    id->giventype = type;
                    if (!IsNext(T_COMMA)) break;
                }
                if (IsNext(T_ASSIGN)) {
                    def->child = ParseMultiRet(ParseOpExp());
                } else {
                    if (has_predeclaration_init)
                        Error("missing initialization");
                    if (st.scopelevels.size() != 1)
                        // For now, since we track it in Idents which don't work with specialization in TC.
                        Error("variable pre-declarations only allowed at top level");
                    for (auto &sid : def->sids) {
                        if (sid.second.utr.Null())
                            Error("a variable pre-declaration must have a type specified");
                        sid.first->id->predeclaration = true;
                    }
                    def->child = new DefaultVal(lex);
                }
                list->Add(def);
                break;
            }
            case T_STATIC:
            case T_STATIC_FRAME: {
                bool frame = lex.token == T_STATIC_FRAME;
                lex.Next();
                if (isprivate) Error("static declaration is always private");
                auto idname = ExpectId();
                if (st.scopelevels.size() == 1) {
                    // This still allows the use inside a top level for as opposed to
                    // requiring a function, which I suppose is useful?
                    Error("static must be used in a nested scope");
                }
                auto id = st.LookupDefStatic(idname);
                id->isprivate = true;
                UnresolvedTypeRef type = { nullptr };
                if (IsNext(T_COLON)) {
                    type = ParseType(false);
                }
                id->giventype = type;
                id->Assign(lex);
                // For now, pin these as freevars, just incase it all gets inlined and they're not.
                id->cursid->used_as_freevar = true;
                Expect(T_ASSIGN);
                Node *init = ParseExp();
                auto def = new Define(lex, init);
                def->sids.push_back({ id->cursid, type });
                // Add to toplevel scope in progress! Should end up before our parent.
                st.toplevel->parent->overloads[0]->gbody->Add(def);
                if (frame) {
                    // Create int var to store frame count.
                    // FIXME: this var is user accessible.. though that's a benign bug :)
                    auto cid = st.LookupDefStatic(idname + "_frame_count");
                    cid->isprivate = true;
                    cid->giventype = { type_int };
                    cid->Assign(lex);
                    cid->cursid->used_as_freevar = true;
                    auto cdef = new Define(lex, new IntConstant(lex, 0));
                    cdef->sids.push_back({ cid->cursid, { type_int } });
                    // Add to toplevel scope too.
                    st.toplevel->parent->overloads[0]->gbody->Add(cdef);
                }
                auto statik = new Static(lex, init->Clone());
                statik->sid = id->cursid;
                statik->giventype = type;
                statik->frame = frame;
                list->Add(statik);
                break;
            }
            case T_MEMBER:
            case T_MEMBER_FRAME: {
                bool frame = lex.token == T_MEMBER_FRAME;
                lex.Next();
                if (isprivate) Error("member declaration is always private");
                if (udtstack.empty()) Error("member declaration outside of class scope");
                // FIXME: this would also allow it to be declared in nested functions, which is
                // not really a direct problem but may want to tighten it up.
                auto udt = udtstack.back();
                // This is an arbitrary restriction that we could lift, just doesn't seem
                // great to have invisble extra members in structs.
                if (udt->is_struct) Error("member declaration only allowed in classes");
                st.bound_typevars_stack.push_back(&udt->generics);
                auto field_idx = udt->fields.size();
                ParseField(udt, true, true);
                st.bound_typevars_stack.pop_back();
                auto initc = udt->fields.back().defaultval->Clone();
                SpecIdent *this_sid = nullptr;
                if (frame) {
                    // Create int field to store frame count.
                    auto fname = udt->fields.back().id->name;
                    auto &fcsfield = st.FieldDecl(fname + "_frame_count", udt);
                    udt->fields.push_back(
                        Field(&fcsfield, { type_int }, new IntConstant(lex, 0), true, false, lex));
                    Ident *this_id = nullptr;
                    st.LookupWithStruct(fname, this_id);
                    assert(this_id);
                    this_sid = this_id->cursid;
                }
                auto member = new Member(lex, initc);
                member->udt = udt;
                member->field_idx = field_idx;
                member->frame = frame;
                member->this_sid = this_sid;
                list->Add(member);
                break;
            }
            case T_ATTRIBUTE: {
                lex.Next();
                auto key = ExpectId();
                if (IsNext(T_ASSIGN)) {
                    switch (lex.token) {
                        case T_IDENT:
                        case T_STR:
                        case T_INT:
                        case T_FLOAT:
                            st.defsubfunctionstack.back()->attributes[key] = lex.sattr;
                            lex.Next();
                            break;
                        default:
                            Error("attribute value must be an identifier or string/int/float constant");
                    }
                } else {
                    st.defsubfunctionstack.back()->attributes[key];
                }
                break;
            }
            default: {
                if (isprivate)
                    Error("private only applies to declarations");
                auto e = ParseExpStat();
                // TODO: add Indexed to this.
                if (Is<IdentRef>(e) || Is<Dot>(e) || Is<GenericCall>(e)) {
                    // Regular assign is handled in normal expression parsing below.
                    if (lex.token == T_COMMA) {
                        auto al = new AssignList(lex, Modify(e));
                        while (IsNext(T_COMMA)) {
                            e = ParseDeref();
                            if (Is<IdentRef>(e) || Is<Dot>(e) || Is<GenericCall>(e)) {
                                al->children.push_back(Modify(e));
                            } else {
                                Error("assignment list elements must be variables or class members");
                            }
                        }
                        Expect(T_ASSIGN);
                        al->children.push_back(ParseMultiRet(ParseOpExp()));
                        list->Add(al);
                        break;
                    }
                }
                list->Add(e);
                break;
            }
        }
    }

    void ParseField(UDT *udt, bool member_private, bool local_member) {
        ExpectId();
        auto &sfield = st.FieldDecl(lastid, udt);
        UnresolvedTypeRef type = { nullptr };
        if (IsNext(T_COLON)) {
            type = ParseType(false);
        }
        Node *init = IsNext(T_ASSIGN) ? ParseExp() : nullptr;
        if (local_member && !init) {
            Error("must specify default value");
        }
        if (type.utr.Null()) {
            if (!init) Error("must specify either type or default value");
            type.utr = init->CFType();
            if (type.utr.Null()) {
                // More complex exp, must be typechecked later.
                type.utr = type_any;
            }
        }
        udt->fields.push_back(Field(&sfield, type, init, member_private, !local_member, lex));
    }

    void ParseTypeDecl(bool is_struct, bool isprivate, Block *parent_list) {
        lex.Next();
        Line line = lex;
        auto sname = st.MaybeNameSpace(ExpectId(), true);
        UDT *udt = &st.StructDecl(sname, is_struct, lex);
        udtstack.push_back(udt);
        auto parse_sup = [&] () {
            ExpectId();
            auto sup = &st.StructUse(lastid);
            if (sup == udt) Error("can\'t inherit from ", Q(lastid));
            if (is_struct != sup->is_struct)
                Error("class/struct must match parent");
            sup->has_subclasses = true;
            return sup;
        };
        auto parse_specializers = [&] () {
            int i = 0;
            if (IsNext(T_LT)) {
                size_t j = 0;
                // Find first unbound generic.
                while (j < udt->generics.size() && !udt->generics[j].giventype.utr.Null()) j++;
                for (;;) {
                    if (j == udt->generics.size()) Error("too many type specializers");
                    udt->generics[j].giventype = ParseType(false);
                    if (IsNext(T_ASSIGN)) {
                        // FIXME: this is a bit of a hack now. We allow default values to be
                        // specified for specializers that apply to fields that have this type,
                        // but given that typevars can be a subtype of a fields type, this is
                        // now a bit odd.
                        auto def = ParseFactor();
                        for (auto &field : udt->fields) {
                            if (field.giventype.utr->t == V_TYPEVAR &&
                                field.giventype.utr->tv == udt->generics[j].tv) {
                                if (field.defaultval) Error("field already has a default value");
                                field.defaultval = def;
                            }
                        }
                    }
                    i++;
                    j++;
                    if (lex.token == T_GT) {
                        lex.OverrideCont(false);  // T_GT here should not continue the line.
                        lex.Next();
                        break;
                    }
                    Expect(T_COMMA);
                }
            }
            return i;
        };
        if (IsNext(T_ASSIGN)) {
            // A specialization of an existing struct
            auto sup = parse_sup();
            udt = sup->CloneInto(udt, sname, st.udttable);
            if (!parse_specializers())
                Error("no specialization types specified");
            if (isprivate != sup->isprivate)
                Error("specialization must have same privacy level");
            if (sup->predeclaration)
                Error("must specialization fully defined type");
            if (udt->generics.back().giventype.utr.Null())
                Error("missing specializers");
        } else if (Either(T_COLON, T_LT)) {
            // A regular struct declaration
            udt->isprivate = isprivate;
            if (IsNext(T_LT)) {
                for (;;) {
                    auto id = ExpectId();
                    for (auto &g : udt->generics)
                        if (g.tv->name == id)
                            Error("re-declaration of generic type");
                    udt->generics.push_back({ { nullptr }, st.NewGeneric(id) });
                    if (IsNext(T_GT)) break;
                    Expect(T_COMMA);
                }
            }
            Expect(T_COLON);
            if (lex.token == T_IDENT) {
                auto sup = parse_sup();
                if (sup->predeclaration) sup->predeclaration = false;  // Empty base class.
                udt->superclass.set_resolvedtype(&sup->thistype);
                udt->superclass.giventype = { st.NewSpecUDT(sup) };
                udt->generics.insert(udt->generics.begin(), sup->generics.begin(),
                                     sup->generics.end());
                for (auto &fld : sup->fields) {
                    udt->fields.push_back(fld);
                }
                st.bound_typevars_stack.push_back(&udt->generics);
                parse_specializers();
                st.bound_typevars_stack.pop_back();
                if (udt->FullyBound()) {
                    for (auto &g : udt->generics) {
                        udt->superclass.giventype.utr->spec_udt->specializers.push_back(&*g.giventype.utr);
                    }
                    udt->superclass.giventype.utr->spec_udt->is_generic = true;
                }
            }
            if (IsNext(T_INDENT)) {
                bool fieldsdone = false;
                st.bound_typevars_stack.push_back(&udt->generics);
                for (;;) {
                    bool member_private = IsNext(T_PRIVATE);
                    if (IsNext(T_FUN)) {
                        fieldsdone = true;
                        parent_list->Add(ParseNamedFunctionDefinition(member_private, udt));
                    }
                    else {
                        if (fieldsdone) Error("fields must be declared before methods");
                        ParseField(udt, member_private, false);
                    }
                    if (!IsNext(T_LINEFEED) || Either(T_ENDOFFILE, T_DEDENT)) break;
                }
                Expect(T_DEDENT);
                st.bound_typevars_stack.pop_back();
            }
            if (udt->fields.empty() && udt->is_struct)
                Error("structs cannot be empty");
        } else {
            // A pre-declaration.
            udt->predeclaration = true;
        }
        udt->is_generic = false;
        udt->unspecialized.specializers.clear();
        for (auto &g : udt->generics) {
            auto type = g.giventype.utr.Null()
                ? UnresolvedTypeRef { &g.tv->thistype }
                : g.giventype;
            // This test works correctly if a generic refers to its own struct, since either
            // is_generic is still false, or it is already true if theres other generics.
            if (st.IsGeneric(type)) udt->is_generic = true;
            udt->unspecialized.specializers.push_back(&*type.utr);
            g.set_resolvedtype_default(type.utr);
        }
        udt->unspecialized.is_generic = udt->is_generic;
        udtstack.pop_back();
        parent_list->Add(new UDTRef(line, udt, udt->predeclaration));
    }

    FunRef *ParseNamedFunctionDefinition(bool isprivate, UDT *self) {
        string idname;
        if (IsNext(T_OPERATOR)) {
            auto op = lex.token;
            if ((op < T_PLUS || op > T_ASREQ) && op != T_LEFTBRACKET)
                Error(cat("illegal token for operator overloading: ", TName(op)));
            idname = cat(TName(T_OPERATOR), TName(op));
            lex.Next();
            if (op == T_LEFTBRACKET) {
                Expect(T_RIGHTBRACKET);
                idname += ']';
            }
        } else {
            // TODO: also exclude functions from namespacing whose first arg is a type namespaced to
            // current namespace (which is same as !self).
            idname = st.MaybeNameSpace(ExpectId(), !self);
        }
        return ParseFunction(&idname, isprivate, true, true, self);
    }

    void ImplicitReturn(Overload &ov) {
        // Anonymous functions and one-liners have an implicit return value.
        auto &stats = ov.gbody->children;
        if (!Is<Return>(stats.back())) {
            // Conversely, if named functions have no return at the end, we should
            // ensure any value accidentally available gets ignored and does not become a return
            // value.
            auto make_void = !ov.sf->parent->anonymous;
            // All function bodies end in return, simplifying code downstream.
            stats.back() = new Return(stats.back()->line, stats.back(), ov.sf, make_void);
        }
    }

    void GenImplicitGenericForLastArg() {
        auto sf = st.defsubfunctionstack.back();
        static const char *typevar_names = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        string_view nn;
        size_t gen_generics = 0;
        {
            again:
            if (gen_generics == 26) Error("too many implicit generics");
            nn = { typevar_names + gen_generics++, 1 };
            for (auto &btv : sf->generics) if (btv.tv->name == nn) goto again;
        }
        auto ng = st.NewGeneric(nn);
        sf->generics.push_back({ { nullptr }, ng });
        sf->args.back().type = &ng->thistype;
        sf->giventypes.push_back({ &ng->thistype });
    }

    void ParseBody(Block *block, int for_nargs) {
        block_stack.push_back({ block, for_nargs, 0 });
        if (IsNext(T_INDENT)) {
            return ParseStatements(block, T_DEDENT);
        } else {
            block->children.push_back(ParseExpStat());
            CleanupStatements(block);
        }
        block_stack.pop_back();
    }

    FunRef *ParseFunction(string *name, bool isprivate, bool parens, bool parseargs,
                        UDT *self = nullptr) {
        auto sf = st.FunctionScopeStart();
        st.bound_typevars_stack.push_back(&sf->generics);
        if (name) {
            // Parse generic params if any.
            // TODO: can this be extended to non-named functions syntactically?
            if (IsNext(T_LT)) {
                sf->explicit_generics = true;
                for (;;) {
                    auto ng = st.NewGeneric(ExpectId());
                    for (auto &btv : sf->generics) if (btv.tv->name == ng->name)
                        Error("re-definition of generic ", Q(ng->name));
                    sf->generics.push_back({ { nullptr }, ng });
                    if (IsNext(T_GT)) break;
                    Expect(T_COMMA);
                }
            }
        }
        if (parens) Expect(T_LEFTPAREN);
        size_t nargs = 0;
        if (self) {
            nargs++;
            auto id = st.LookupDef("this", false, true);
            auto &arg = sf->args.back();
            arg.type = &self->unspecialized_type;
            sf->giventypes.push_back({ arg.type });
            st.AddWithStruct(arg.type, id, sf);
            id->cursid->withtype = true;
        }
        bool non_inline_method = false;
        int first_default_arg = -1;
        vector<Node *> default_args;
        if (lex.token != T_RIGHTPAREN && parseargs) {
            for (;;) {
                ExpectId();
                nargs++;
                bool withtype = lex.token == T_TYPEIN;
                auto id = st.LookupDef(lastid, false, withtype);
                auto &arg = sf->args.back();
                if (parens && (lex.token == T_COLON || withtype)) {
                    lex.Next();
                    arg.type = ParseType(withtype, nullptr).utr;
                    if (withtype) st.AddWithStruct(arg.type, id, sf);
                    if (nargs == 1 && arg.type->t == V_UUDT) {
                        non_inline_method = true;
                        self = arg.type->spec_udt->udt;
                        st.bound_typevars_stack.push_back(&self->generics);
                    }
                    sf->giventypes.push_back({ arg.type });
                } else {
                    GenImplicitGenericForLastArg();
                }
                if (parens && IsNext(T_ASSIGN)) {
                    if (first_default_arg < 0) first_default_arg = (int)sf->args.size() - 1;
                    default_args.push_back(ParseExp());
                } else {
                    if (first_default_arg >= 0) Error("missing default argument");
                }
                if (!IsNext(T_COMMA)) break;
            }
        }
        if (parens) Expect(T_RIGHTPAREN);
        sf->method_of = self;
        auto &f = name ? st.FunctionDecl(*name, nargs) : st.CreateFunction("");
        auto nf = natreg.FindNative(f.name);
        if (nf && nf->args.size() >= nargs) {
            // TODO: could allow less args if we check nf's default args.
            Error("cannot override built-in function ", Q(f.name), " with same (or less) arity");
        }
        // Check default args are being used consistently with the overloads & siblings.
        if (first_default_arg < 0) first_default_arg = (int)nargs;
        if (f.overloads.empty()) {
            f.first_default_arg = first_default_arg;
            f.default_args = default_args;
        } else {
            if (f.first_default_arg != first_default_arg)
                Error("number of default arguments must be the same as previous overload");
            for (auto [i, da] : enumerate(f.default_args)) {
                if (da && !da->Equal(default_args[i]))
                    Error("default argument ", i + 1, " must be same as previous overload");
                delete default_args[i];
            }
        }
        // Create the overload.
        f.overloads.emplace_back(new Overload { lex, isprivate });
        sf->SetParent(f, *f.overloads.back());
        // Check if there's any overlap in default argument ranges.
        auto ff = st.GetFirstFunction(f.name);
        while (ff) {
            if (ff != &f) {
                if (first_default_arg <= (int)ff->nargs() &&
                    ff->first_default_arg <= (int)f.nargs())
                    Error("function ", Q(f.name), " with ", f.nargs(),
                          " arguments is ambiguous with the ", ff->nargs(),
                          " version because of default arguments");
            }
            ff = ff->sibf;
        }
        if (IsNext(T_RETURNTYPE)) {  // Return type decl.
            sf->returngiventype = ParseTypes(sf, LT_KEEP);
            sf->returntype = sf->returngiventype.utr;
        }
        if (!IsNext(T_COLON)) {
            // This must be a function type.
            if (lex.token == T_IDENT || !name) Expect(T_COLON);
            if (f.istype || f.overloads.size() > 1)
                Error("redefinition of function type ", Q(*name));
            f.istype = true;
            sf->typechecked = true;
            for (auto [i, arg] : enumerate(sf->args)) {
                if (st.IsGeneric(sf->giventypes[i]))
                    Error("function type arguments can\'t be generic (missing ", Q(":"), " ?)");
                // No idea what the function is going to be, so have to default to borrow.
                arg.sid->lt = LT_BORROW;
            }
            if (sf->returngiventype.utr.Null())
                Error("missing return type or ", Q(":"), " in function definition header");
            if (!sf->generics.empty())
                Error("function type cannot have generics");
            sf->reqret = sf->returntype->NumValues();
        }
        if (name) {
            if (f.overloads.size() > 1) {
                // We could check here for "double declaration", but since that entails
                // detecting what is a legit overload or not, this is in general better left to the
                // type checker.
                if (!f.nargs()) Error("double declaration of ", Q(f.name));
            }
            functionstack.push_back(&f);
        } else {
            f.anonymous = true;
        }
        // Parse the body.
        Line line = lex;
        if (!f.istype) {
            auto block = new Block(lex);
            auto &ov = *f.overloads.back();
            ov.gbody = block;
            ParseBody(block, -1);
            ImplicitReturn(ov);
        }
        if (name) functionstack.pop_back();
        if (non_inline_method) st.bound_typevars_stack.pop_back();
        st.bound_typevars_stack.pop_back();
        st.FunctionScopeCleanup();
        return new FunRef(line, sf);
    }

    UnresolvedTypeRef ParseTypes(SubFunction *sfreturntype, Lifetime lt) {
        auto dest = ParseType(false, sfreturntype);
        if (!IsNext(T_COMMA)) return dest;
        vector<TypeRef> types;
        types.push_back(dest.utr);
        do {
            types.push_back(ParseType(false, sfreturntype).utr);
        } while (IsNext(T_COMMA));
        dest = { st.NewTuple(types.size()) };
        for (auto [i, type] : enumerate(types))
            dest.utr->Set(i, &*type, IsRefNil(type->t) ? lt : LT_ANY);
        return dest;
    }

    TypeRef FindTypeVar(string_view name) {
        for (auto gv : reverse(st.bound_typevars_stack)) {
            for (auto &btv : *gv) {
                if (btv.tv->name == name) {
                    return &btv.tv->thistype;
                }
            }
        }
        return nullptr;
    }

    UnresolvedTypeRef ParseType(bool withtype, SubFunction *sfreturntype = nullptr) {
        TypeRef dest;
        switch(lex.token) {
            case T_INTTYPE:   dest = type_int;        lex.Next(); break;
            case T_FLOATTYPE: dest = type_float;      lex.Next(); break;
            case T_STRTYPE:   dest = type_string;     lex.Next(); break;
            case T_RESOURCE: {
                lex.Next();
                Expect(T_LT);
                auto id = ExpectId();
                auto rt = LookupResourceType(id);
                if (!rt) Error("unknown resource type ", Q(id));
                // This may be the end of the line, so make sure Lex doesn't see it
                // as a GT op.
                lex.OverrideCont(false);
                Expect(T_GT);
                dest = &rt->thistype;
                break;
            }
            case T_IDENT: {
                auto f = st.FindFunction(lex.sattr);
                if (f && f->istype) {
                    dest = &f->overloads[0]->sf->thistype;
                    lex.Next();
                    break;
                }
                auto e = st.EnumLookup(lex.sattr, false);
                if (e) {
                    dest = &e->thistype;
                    lex.Next();
                    break;
                }
                dest = FindTypeVar(lex.sattr);
                if (!dest.Null()) {
                    lex.Next();
                    goto done;
                }
                dest = &st.StructUse(lex.sattr).unspecialized_type;
                lex.Next();
                if (IsNext(T_LT)) {
                    dest = st.NewSpecUDT(dest->spec_udt->udt);
                    if (dest->spec_udt->udt->is_generic) dest->spec_udt->is_generic = true;
                    for (;;) {
                        auto s = ParseType(false);
                        if (st.IsGeneric(s)) dest->spec_udt->is_generic = true;
                        dest->spec_udt->specializers.push_back(&*s.utr);
                        if (lex.token == T_GT) {
                            // This may be the end of the line, so make sure Lex doesn't see it
                            // as a GT op.
                            lex.OverrideCont(false);
                            lex.Next();
                            break;
                        }
                        Expect(T_COMMA);
                    }
                } else {
                    if (dest->spec_udt->udt->is_generic)
                        Error("use of type ", Q(dest->spec_udt->udt->name),
                              " requires specializers");
                }
                done:
                break;
            }
            case T_LEFTBRACKET: {
                lex.Next();
                TypeRef elem = ParseType(false).utr;
                Expect(T_RIGHTBRACKET);
                dest = st.Wrap(elem, V_VECTOR);
                break;
            }
            case T_VOIDTYPE:
                if (sfreturntype) {
                    lex.Next();
                    dest = type_void;
                    sfreturntype->reqret = 0;
                    break;
                }
                // FALL-THRU:
            default:
                Error("illegal type syntax: ", Q(lex.TokStr()));
        }
        if (IsNext(T_QUESTIONMARK)) {
            if (!st.IsNillable(dest) && dest->t != V_TYPEVAR)
                Error("value types can\'t be made nilable");
            dest = st.Wrap(dest, V_NIL);
        }
        if (withtype && dest->t != V_UUDT)
            Error(":: must be used with a class type");
        return { dest };
    }

    Node *ParseMultiRet(Node *first) {
        if (lex.token != T_COMMA) return first;
        auto list = new MultipleReturn(lex);
        list->Add(first);
        while (IsNext(T_COMMA)) {
            list->Add(ParseOpExp());
        }
        return list;
    }

    Node *ParseExpStat() {
        if (IsNext(T_RETURN)) {
            Node *rv = nullptr;
            if (!Either(T_LINEFEED, T_DEDENT, T_FROM)) {
                rv = ParseMultiRet(ParseOpExp());
            } else {
                rv = new DefaultVal(lex);
            }
            auto sf = st.toplevel;
            if (IsNext(T_FROM)) {
                if(!IsNext(T_PROGRAM)) {
                    if (!IsNextId())
                        Error(Q("return from"), " must be followed by function identifier or ",
                              Q("program"));
                    auto f = st.FindFunction(lastid);
                    if (!f)
                        Error(Q(lastid), " is not a known function for use with ",
                              Q("return from"));
                    if (f->sibf || f->overloads.size() > 1)
                        Error("function ", Q(lastid),
                              " must have single implementation to be used with ",
                              Q("return from"));
                    sf = f->overloads[0]->sf;
                }
            } else {
                if (functionstack.size())
                    sf = functionstack.back()->overloads.back()->sf;
            }
            return new Return(lex, rv, sf, false);
        } else if (IsNext(T_BREAK)) {
            return new Break(lex);
        }
        auto e = ParseExp();
        while (IsNext(T_SEMICOLON)) {
            if (IsNext(T_LINEFEED)) {
                // specialized error for all the C-style language users
                Error(Q(";"), " is not a statement terminator");
            }
            e = new Seq(lex, e, ParseExp());
        }
        return e;
    }

    Node *Modify(Node *e) {
        if (auto idr = Is<IdentRef>(e))
            idr->sid->id->Assign(lex);
        return e;
    }

    void CheckOpEq(Node *e) {
        if (!Is<IdentRef>(e) && !Is<Indexing>(e) && !Is<GenericCall>(e) && !Is<Dot>(e))
            Error("illegal left hand side of assignment");
        Modify(e);
        lex.Next();
    }

    Node *ParseExp(bool parent_noparens = false) {
        DS<bool> ds(call_noparens, parent_noparens);
        auto e = ParseOpExp();
        switch (lex.token) {
            case T_ASSIGN:  CheckOpEq(e); return new Assign(lex, e, ParseExp());
            case T_PLUSEQ:  CheckOpEq(e); return new PlusEq(lex, e, ParseExp());
            case T_MINUSEQ: CheckOpEq(e); return new MinusEq(lex, e, ParseExp());
            case T_MULTEQ:  CheckOpEq(e); return new MultiplyEq(lex, e, ParseExp());
            case T_DIVEQ:   CheckOpEq(e); return new DivideEq(lex, e, ParseExp());
            case T_MODEQ:   CheckOpEq(e); return new ModEq(lex, e, ParseExp());
            case T_ANDEQ:   CheckOpEq(e); return new AndEq(lex, e, ParseExp());
            case T_OREQ:    CheckOpEq(e); return new OrEq(lex, e, ParseExp());
            case T_XOREQ:   CheckOpEq(e); return new XorEq(lex, e, ParseExp());
            case T_ASLEQ:   CheckOpEq(e); return new ShiftLeftEq(lex, e, ParseExp());
            case T_ASREQ:   CheckOpEq(e); return new ShiftRightEq(lex, e, ParseExp());
            default:        return e;
        }
    }

    Node *ParseOpExp(int level = 6) {
        static TType ops[][4] = {
            { T_MULT, T_DIV, T_MOD, T_NONE },
            { T_PLUS, T_MINUS, T_NONE, T_NONE },
            { T_ASL, T_ASR, T_NONE, T_NONE },
            { T_BITAND, T_BITOR, T_XOR, T_NONE },
            { T_LT, T_GT, T_LTEQ, T_GTEQ },
            { T_EQ, T_NEQ, T_NONE, T_NONE },
            { T_AND, T_OR, T_NONE, T_NONE },
        };
        Node *exp = level ? ParseOpExp(level - 1) : ParseUnary();
        TType *o = &ops[level][0];
        while (Either(o[0], o[1]) || Either(o[2], o[3])) {
            TType op = lex.token;
            lex.Next();
            auto rhs = level ? ParseOpExp(level - 1) : ParseUnary();
            switch (op) {
                case T_MULT:   exp = new Multiply(lex, exp, rhs); break;
                case T_DIV:    exp = new Divide(lex, exp, rhs); break;
                case T_MOD:    exp = new Mod(lex, exp, rhs); break;
                case T_PLUS:   exp = new Plus(lex, exp, rhs); break;
                case T_MINUS:  exp = new Minus(lex, exp, rhs); break;
                case T_ASL:    exp = new ShiftLeft(lex, exp, rhs); break;
                case T_ASR:    exp = new ShiftRight(lex, exp, rhs); break;
                case T_BITAND: exp = new BitAnd(lex, exp, rhs); break;
                case T_BITOR:  exp = new BitOr(lex, exp, rhs); break;
                case T_XOR:    exp = new Xor(lex, exp, rhs); break;
                case T_LT:     exp = new LessThan(lex, exp, rhs); break;
                case T_GT:     exp = new GreaterThan(lex, exp, rhs); break;
                case T_LTEQ:   exp = new LessThanEq(lex, exp, rhs); break;
                case T_GTEQ:   exp = new GreaterThanEq(lex, exp, rhs); break;
                case T_EQ:     exp = new Equal(lex, exp, rhs); break;
                case T_NEQ:    exp = new NotEqual(lex, exp, rhs); break;
                case T_AND:    exp = new And(lex, exp, rhs); break;
                case T_OR:     exp = new Or(lex, exp, rhs); break;
                default: assert(false);
            }
        }
        return exp;
    }

    Node *UnaryArg() {
        auto t = lex.token;
        lex.Next();
        auto e = ParseUnary();
        return t == T_INCR || t == T_DECR ? Modify(e) : e;
    }

    Node *ParseUnary() {
        switch (lex.token) {
            case T_MINUS: return new UnaryMinus(lex, UnaryArg());
            case T_NEG:   return new Negate(lex, UnaryArg());
            case T_INCR:  return new PreIncr(lex, UnaryArg());
            case T_DECR:  return new PreDecr(lex, UnaryArg());
            case T_NOT:  // Different precedence.
                lex.Next();
                return new Not(lex, ParseOpExp(5));
            default:
                return ParseDeref();
        }
    }

    List *ParseFunctionCall(Line line, Function *f, NativeFun *nf, string_view idname, Node *dotarg,
                            bool noparenscall, vector<UnresolvedTypeRef> *specializers) {
        vector<Node *> list;
        bool parens_parsed = false;
        [&]() {
            if (dotarg) {
                list.push_back(dotarg);
                if (!IsNext(T_LEFTPAREN)) return;
                parens_parsed = true;
            } else {
                if (!noparenscall) {
                    Expect(T_LEFTPAREN);
                    parens_parsed = true;
                }
            }
            // Parse regular arguments.
            bool needscomma = false;
            for (;;) {
                if (!noparenscall && IsNext(T_RIGHTPAREN)) {
                    if (call_noparens) {  // This call is an arg to a call that has no parens.
                        // Don't unnecessarily parse funvals. Means "if f(x):" parses as expected.
                        return;
                    }
                    break;
                }
                if (needscomma) Expect(T_COMMA);
                list.push_back(ParseExp(noparenscall));
                if (noparenscall) {
                    if (lex.token == T_COLON) break;
                    return;
                } else {
                    needscomma = true;
                }
            }
            // Parse trailing function values.
            for (;;) {
                Node *e = nullptr;
                switch (lex.token) {
                    case T_COLON: e = ParseFunction(nullptr, false, false, false); break;
                    case T_IDENT: e = ParseFunction(nullptr, false, false, true); break;
                    case T_LEFTPAREN: e = ParseFunction(nullptr, false, true, true); break;
                    default: return;
                }
                list.push_back(e);
                auto islf = IsNext(T_LINEFEED);
                if (!islf && lex.token != T_LAMBDA) { return; }
                if (!IsNext(T_LAMBDA)) {
                    lex.PushCur();
                    if (islf) lex.Push(T_LINEFEED);
                    lex.Next();
                    return;
                }
            }
        }();
        // FIXME: move more of the code below into the type checker, and generalize the remaining
        // code to be as little dependent as possible on whether nf or f are available.
        // It should only parse args and construct a GenericCall.
        auto id = dotarg ? nullptr : st.Lookup(idname);
        // If both a var and a function are in scope, the deepest scope wins.
        // Note: <, because functions are inside their own scope.
        if (nf || (f && (!id || id->scopelevel < f->scopelevel))) {
            if (f && f->istype) Error("can\'t call function type ", Q(f->name));
            auto call = new GenericCall(line, idname, st.current_namespace, dotarg != nullptr,
                                        !parens_parsed, false, specializers);
            call->children = list;
            return call;
        }
        if (noparenscall) Error("call requires ()");
        if (id) {
            auto dc = new DynCall(lex, nullptr, id->cursid);
            dc->children = list;
            return dc;
        } else {
            auto call = new GenericCall(line, idname, st.current_namespace, dotarg != nullptr,
                                        !parens_parsed, false, specializers);
            call->children = list;
            return call;
        }
    }

    Node *ParseDeref() {
        auto n = ParseFactor();
        // FIXME: it would be good to narrow the kind of factors these derefs can attach to,
        // since for some of them it makes no sense (e.g. function call with lambda args).
        for (;;) switch (lex.token) {
            case T_DOT: {
                lex.Next();
                auto idname = ExpectId();
                auto fld = st.FieldUse(idname);
                if (fld && lex.token != T_LEFTPAREN) {
                    auto dot = new GenericCall(lex, idname, st.current_namespace,
                                               true, true, false, nullptr);
                    dot->Add(n);
                    n = dot;
                } else {
                    auto nf = natreg.FindNative(idname);
                    auto f = st.FindFunction(idname);
                    auto specializers = ParseSpecializers(f && !nf);
                    n = ParseFunctionCall(lex, f, nf, idname, n, false, &specializers);
                }
                break;
            }
            case T_LEFTPAREN: {
                // Special purpose error to make this more understandable for the user.
                // FIXME: can remove this restriction if we make DynCall work with any node.
                Error("dynamic function value call must be on variable");
                return n;
            }
            case T_LEFTBRACKET: {
                lex.Next();
                n = new Indexing(lex, n, ParseExp());
                Expect(T_RIGHTBRACKET);
                break;
            }
            case T_INCR:
                n = new PostIncr(lex, Modify(n));
                lex.Next();
                return n;
            case T_DECR:
                n = new PostDecr(lex, Modify(n));
                lex.Next();
                return n;
            case T_IS: {
                lex.Next();
                auto is = new IsType(lex, n, ParseType(false));
                return is;
            }
            default:
                return n;
        }
    }

    Node *ParseFactor() {
        switch (lex.token) {
            case T_INT: {
                auto i = lex.ival;
                lex.Next();
                return new IntConstant(lex, i);
            }
            case T_FLOAT: {
                auto f = lex.fval;
                lex.Next();
                return new FloatConstant(lex, f);
            }
            case T_STR: {
                auto s = new StringConstant(lex, std::move(lex.sval));
                lex.Next();
                return s;
            }
            case T_STR_INT_START: {
                Node *si = lex.sval.empty() ? nullptr : new StringConstant(lex, std::move(lex.sval));
                lex.Next();
                for (;;) {
                    auto e = ParseOpExp();
                    if (si) {
                        si = new Plus(lex, si, e);
                    } else {
                        // We start with an exp, but we have to force this to be a string to ensure
                        // all subsequent Plus ops are string concats.
                        auto call = new NativeCall(natreg.FindNative("string"), lex);
                        call->children.push_back(e);
                        si = call;
                    }
                    if (!lex.sval.empty())
                        si = new Plus(lex, si, new StringConstant(lex, std::move(lex.sval)));
                    if (lex.token == T_STR_INT_MIDDLE) {
                        lex.Next();
                    } else {
                        Expect(T_STR_INT_END);
                        return si;
                    }
                }
            }
            case T_NIL: {
                lex.Next();
                auto n = new Nil(lex, { nullptr });
                if (IsNext(T_TYPEIN)) {
                    n->giventype = ParseType(false);
                    n->giventype.utr = st.Wrap(n->giventype.utr, V_NIL);
                }
                return n;
            }
            case T_LEFTPAREN: {
                lex.Next();
                auto n = ParseExp();
                Expect(T_RIGHTPAREN);
                return n;
            }
            case T_LEFTBRACKET: {
                lex.Next();
                auto constructor = new Constructor(lex, { nullptr });
                ParseVector([this, &constructor] () {
                    constructor->Add(this->ParseExp());
                }, T_RIGHTBRACKET);
                if (IsNext(T_TYPEIN)) {
                    constructor->giventype = ParseType(false);
                    constructor->giventype.utr = st.Wrap(constructor->giventype.utr, V_VECTOR);
                }
                return constructor;
            }
            case T_LAMBDA: {
                lex.Next();
                return ParseFunction(nullptr, false, lex.token == T_LEFTPAREN,
                    lex.token != T_COLON);
            }
            case T_FLOATTYPE:
            case T_INTTYPE:
            case T_STRTYPE:
            case T_ANYTYPE: {
                // These are also used as built-in functions, so allow them to function as
                // identifier for calls.
                auto idname = lex.sattr;
                lex.Next();
                if (lex.token != T_LEFTPAREN) Error("type used as expression");
                return IdentFactor(idname);
            }
            case T_TYPEOF: {  // ident or type.
                lex.Next();
                if (lex.token == T_IDENT) {
                    auto id = st.Lookup(lex.sattr);
                    if (id) {
                        lex.Next();
                        return new TypeOf(lex, new IdentRef(lex, id->cursid));
                    }
                }
                auto tn = new TypeAnnotation(lex, ParseType(false));
                return new TypeOf(lex, tn);
            }
            case T_IDENT: {
                auto idname = lex.sattr;
                lex.Next();
                return IdentFactor(idname);
            }
            case T_SUPER: {
                lex.Next();
                auto idname = ExpectId();
                auto n = ParseFunctionCall(lex, st.FindFunction(idname), nullptr, idname, nullptr, false,
                                           nullptr);
                auto call = Is<GenericCall>(*n);
                call->super = true;
                return n;
            }
            case T_PAKFILE: {
                lex.Next();
                string s = std::move(lex.sval);
                Expect(T_STR);
                pakfiles.insert(s);
                return new StringConstant(lex, std::move(s));
            }
            case T_IF: {
                lex.Next();
                return ParseIf(lex);
            }
            case T_WHILE: {
                lex.Next();
                Line line = lex;
                auto cond = ParseExp(true);
                return new While(line, cond, ParseBlock());
            }
            case T_FOR: {
                lex.Next();
                Line line = lex;
                Node *iter;
                if (IsNext(T_LEFTPAREN)) {
                    iter = ParseExp(false);
                    Expect(T_RIGHTPAREN);
                    return new For(line, iter, ParseBlock(0, true));
                } else {
                    iter = ParseExp(true);
                    return new For(line, iter, ParseBlock(0));
                }
            }
            case T_SWITCH: {
                lex.Next();
                Line line = lex;
                auto value = ParseExp(true);
                Expect(T_COLON);
                Expect(T_INDENT);
                bool have_default = false;
                auto cases = new List(lex);
                for (;;) {
                    List *pattern = new List(lex);
                    Line cline = lex;
                    if (lex.token == T_DEFAULT) {
                        if (have_default) Error("cannot have more than one default in a switch");
                        lex.Next();
                        have_default = true;
                    } else {
                        Expect(T_CASE);
                        for (;;) {
                            auto f = ParseUnary();
                            if (lex.token == T_DOTDOT) {
                                lex.Next();
                                f = new Range(lex, f, ParseUnary());
                            }
                            pattern->Add(f);
                            if (lex.token == T_COLON) break;
                            Expect(T_COMMA);
                        }
                    }
                    cases->Add(new Case(cline, pattern, ParseBlock()));
                    if (!IsNext(T_LINEFEED)) break;
                    if (lex.token == T_DEDENT) break;
                }
                Expect(T_DEDENT);
                return new Switch(line, value, cases);
            }
            default:
                Error("illegal start of expression: ", Q(lex.TokStr()));
                return nullptr;
        }
    }

    Node *ParseIf(Line line) {
        auto cond = ParseExp(true);
        auto thenp = ParseBlock();
        auto islf = IsNext(T_LINEFEED);
        if (IsNext(T_ELIF)) {
            return new IfElse(line, cond, thenp, (new Block(lex))->Add(ParseIf(lex)));
        } else if (IsNext(T_ELSE)) {
            return new IfElse(line, cond, thenp, ParseBlock());
        } else {
            lex.PushCur();
            if (islf) lex.Push(T_LINEFEED);
            lex.Next();
            return new IfThen(line, cond, thenp);
        }
    }

    void ForLoopVar(int existing, SpecIdent *sid, UnresolvedTypeRef type, vector<Node *> &list) {
        Node *init = nullptr;
        if (existing == 0)
            init = new ForLoopElem(lex);
        else if (existing == 1)
            init = new ForLoopCounter(lex);
        else
            Error("for loop takes at most an element and index variable");
        auto def = new Define(lex, init);
        def->sids.push_back({ sid , type });
        list.insert(list.begin() + existing, def);
    }

    Block *ParseBlock(int for_args = -1, bool parse_args = false) {
        st.BlockScopeStart();
        auto block = new Block(lex);
        if (parse_args && lex.token != T_COLON) {
            auto parens = IsNext(T_LEFTPAREN);
            for (;;) {
                ExpectId();
                bool withtype = lex.token == T_TYPEIN;
                auto id = st.LookupDef(lastid, true, withtype);
                id->single_assignment = false;  // Mostly to stop warning that it is constant.
                UnresolvedTypeRef type = { nullptr };
                if (parens && (lex.token == T_COLON || withtype)) {
                    lex.Next();
                    type = ParseType(withtype, nullptr);
                    if (withtype) st.AddWithStruct(type.utr, id, st.defsubfunctionstack.back());
                }
                id->cursid->withtype = withtype;
                ForLoopVar(for_args, id->cursid, type, block->children);
                for_args++;
                if (!IsNext(T_COMMA)) break;
            }
            if (parens) Expect(T_RIGHTPAREN);
        }
        Expect(T_COLON);
        ParseBody(block, for_args);
        st.BlockScopeCleanup();
        return block;
    }

    void ParseVector(const function<void()> &f, TType closing) {
        if (IsNext(closing)) return;
        assert(lex.token != T_INDENT);  // Not generated inside brackets/braces.
        for (;;) {
            f();
            if (!IsNext(T_COMMA) || lex.token == closing) break;
        }
        Expect(closing);
    }

    vector<UnresolvedTypeRef> ParseSpecializers(bool likely_named_function) {
        vector<UnresolvedTypeRef> specializers;
        // Check for function call with generic params.
        // This is not a great way to distinguish from < operator exps, but best we can do?
        if (likely_named_function && lex.whitespacebefore == 0 && IsNext(T_LT)) {
            for (;;) {
                specializers.push_back(ParseType(false));
                if (IsNext(T_GT)) break;
                Expect(T_COMMA);
            }
        }
        return specializers;
    }

    Node *IdentFactor(string_view idname) {
        // First see if this a type constructor.
        auto udt = st.LookupStruct(idname);
        UnresolvedTypeRef type = { nullptr };
        if (udt && lex.token == T_LT) {
            lex.Undo(T_IDENT, idname);
            type = ParseType(false);
        } else if (lex.token == T_LEFTCURLY) {
            auto tv = FindTypeVar(idname);
            if (!tv.Null()) {
                type = { tv };
            } else {
                udt = &st.StructUse(idname);
                type = { st.NewSpecUDT(udt) };
                type.utr->spec_udt->is_generic = udt->is_generic;
            }
        } else {
            udt = nullptr;
        }
        if (udt) {
            Expect(T_LEFTCURLY);
            Line line = lex;
            udt->constructed = true;
            vector<Node *> exps(udt->fields.size(), nullptr);
            ParseVector([&] () {
                auto id = lex.sattr;
                if (IsNext(T_IDENT)) {
                    if (IsNext(T_COLON)) {
                        auto fld = st.FieldUse(id);
                        auto field = udt->Has(fld);
                        if (field < 0) Error("unknown field ", Q(id));
                        if (exps[field]) Error("field ", Q(id), " initialized twice");
                        exps[field] = ParseExp();
                        return;
                    } else {
                        lex.Undo(T_IDENT, id);
                    }
                }
                // An initializer without a tag. Find first field without a default thats not
                // set yet.
                for (size_t i = 0; i < exps.size(); i++) {
                    if (!exps[i] && !udt->fields[i].defaultval) {
                        exps[i] = ParseExp();
                        return;
                    }
                }
                // Since this struct may be pre-declared, we allow to parse more initializers
                // than there are fields. We will catch this in the type checker.
                exps.push_back(ParseExp());
            }, T_RIGHTCURLY);
            // Now fill in defaults, check for missing fields, and construct list.
            auto constructor = new Constructor(line, type);
            for (size_t i = 0; i < exps.size(); i++) {
                if (!exps[i]) {
                    if (udt->fields[i].defaultval)
                        exps[i] = udt->fields[i].defaultval->Clone();
                    else
                        Error("field ", Q(udt->fields[i].id->name), " not initialized");
                }
                constructor->Add(exps[i]);
            }
            return constructor;
        }
        if (!type.utr.Null()) {
            Expect(T_LEFTCURLY);
            if (type.utr->t == V_TYPEVAR) {
                auto constructor = new Constructor(lex, type);
                // We don't know what args this type has, so parse any number of them, without tags.
                while (lex.token != T_RIGHTCURLY) {
                    constructor->Add(ParseExp());
                    if (lex.token != T_RIGHTCURLY) Expect(T_COMMA);
                }
                lex.Next();
                return constructor;
            } else {
                Error("type ", Q(TypeName(type.utr)), " does not have a {} constructor");
            }
        }
        // If we see "f(" the "(" is the start of an argument list, but for "f (", "(" is
        // part of an expression of a single argument with no extra "()".
        // This avoids things like "f (1 + 2) * 3" ("* 3" part of the single arg) being
        // interpreted as "f(1 + 2) * 3" (not part of the arg).
        // This is benign, since single arg calls with "()" work regardless of whitespace,
        // and multi-arg calls with whitespace will now error on the first "," (since we
        // don't have C's ","-operator).
        auto nf = natreg.FindNative(idname);
        auto f = st.FindFunction(idname);
        auto e = st.EnumLookup(idname, false);
        if (lex.token == T_LEFTPAREN && lex.whitespacebefore == 0) {
            if (e && !f && !nf) {
                lex.Next();
                auto ec = new EnumCoercion(lex, ParseExp(), e);
                Expect(T_RIGHTPAREN);
                return ec;
            }
            return ParseFunctionCall(lex, f, nf, idname, nullptr, false, nullptr);
        }
        auto specializers = ParseSpecializers(f && !nf && !e);
        if (!specializers.empty())
            return ParseFunctionCall(lex, f, nf, idname, nullptr, false, &specializers);
        // Check for implicit variable.
        if (idname[0] == '_') {
            if (block_stack.empty())
                Error("cannot add implicit argument ", Q(idname), " at top level");
            auto &bs = block_stack.back();
            auto id = st.Lookup(idname);
            auto sf = st.defsubfunctionstack.back();
            if (!id || id->cursid->sf_def != sf) {
                if (bs.for_nargs >= 0) {
                    id = st.LookupDef(idname, true, false);
                    if (bs.for_nargs > 0) {
                        Error("cannot add implicit argument ", Q(idname), " to ", Q("for"),
                              " with existing arguments");
                    }
                    id->constant = true;
                    ForLoopVar(bs.implicits, id->cursid, type, bs.block->children);
                    bs.implicits++;
                } else {
                    id = st.LookupDef(idname, false, false);
                    if (st.defsubfunctionstack.size() <= 1)
                        Error("cannot add implicit argument ", Q(idname), " to top level");
                    if (!sf->parent->anonymous)
                        Error("cannot use implicit argument ", Q(idname),
                              " in named function ", Q(sf->parent->name));
                    if (sf->args[0].sid->id->name[0] != '_')
                        Error("cannot mix implicit argument ", Q(idname),
                              " with declared arguments in function");
                    if (st.defsubfunctionstack.back()->args.back().type->Equal(*type_any))
                        GenImplicitGenericForLastArg();
                }
                id->Read();
            }
            return new IdentRef(lex, id->cursid);
        }
        auto id = st.Lookup(idname);
        Ident *fieldid = nullptr;
        auto field = st.LookupWithStruct(idname, fieldid);
        // Check for function call without ().
        if (!id &&
            !field &&
            (nf || f) &&
            lex.whitespacebefore > 0 &&
            lex.token != T_LINEFEED) {
            return ParseFunctionCall(lex, f, nf, idname, nullptr, true, nullptr);
        }
        // Check for enum value.
        auto ev = st.EnumValLookup(idname, false);
        if (ev) {
            auto ic = new IntConstant(lex, ev->val);
            ic->from = ev;
            return ic;
        }
        // Check for field reference in function with :: arguments.
        if (field) {
            return new Dot(field, lex, new IdentRef(lex, fieldid->cursid));
        }
        // It's likely a regular variable.
        if (!id) {
            if (f || nf) Error("can\'t use named function ", Q(idname), " as value");
            else Error("unknown identifier ", Q(idname));
        }
        return new IdentRef(lex, id->cursid);
    }

    bool IsNext(TType t) {
        bool isnext = lex.token == t;
        if (isnext) lex.Next();
        return isnext;
    }

    string_view lastid;

    bool IsNextId() {
        if (lex.token != T_IDENT) return false;
        lastid = lex.sattr;
        lex.Next();
        return true;
    }

    string_view ExpectId() {
        lastid = lex.sattr;
        Expect(T_IDENT);
        return lastid;
    }

    bool Either(TType t1, TType t2) {
        return lex.token == t1 || lex.token == t2;
    }
    bool Either(TType t1, TType t2, TType t3) {
        return lex.token == t1 || lex.token == t2 || lex.token == t3;
    }
    bool Either(TType t1, TType t2, TType t3, TType t4) {
        return lex.token == t1 || lex.token == t2 || lex.token == t3 || lex.token == t4;
    }

    void Expect(TType t) {
        if (!IsNext(t))
            Error(Q(lex.TokStr(t)) + " expected, found " + Q(lex.TokStr()));
    }

    string DumpAll(bool onlytypechecked = false) {
        string s;
        for (auto f : st.functiontable) {
            for (auto ov : f->overloads) {
                for (auto sf = ov->sf; sf; sf = sf->next) {
                    if (!onlytypechecked || sf->typechecked) {
                        s += "FUNCTION: " + f->name + "(";
                        for (auto &arg : sf->args) {
                            s += arg.sid->id->name + ":" + TypeName(arg.type) + " ";
                        }
                        s += ") -> ";
                        s += TypeName(sf->returntype);
                        s += "\n";
                        if (sf->sbody) s += DumpNode(*sf->sbody, 4, false);
                        s += "\n\n";
                    }
                }
            }
        }
        return s;
    }
};

}  // namespace lobster
