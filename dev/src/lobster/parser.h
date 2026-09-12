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
    vector<Function *> namedfunctionstack;
    vector<GUDT *> gudtstack;
    bool call_noparens = false;
    set<string> pakfiles;
    struct BlockScope {
        Block *block;
        int for_nargs;
        int implicits;
    };
    vector<BlockScope> block_stack;
    int serializable_id_last = -1;

    Parser(NativeRegistry &natreg, Lex &lex, SymbolTable &st)
        : natreg(natreg), lex(lex), st(st) {}

    ~Parser() {
        delete root;
    }

    // Errors don't stop parsing: they get collected (see Lex::Report), and the code that hit
    // one carries on with a placeholder for whatever it could not parse, chosen such that the
    // rest of the parser sees well-formed data and reports as few consequential errors as
    // possible. Compilation stops after parsing if there were any, so placeholders never reach
    // the typechecker. The helpers below this are the common recovery patterns.

    template<typename... Ts> void Error(const Ts &...args) {
        lex.Report(cat(args...), nullptr);
    }

    template<typename... Ts> void ErrorAt(const Node *what, const Ts &...args) {
        lex.Report(cat(args...), what ? &what->line : nullptr);
    }

    // Stands in for an expression that failed to parse.
    Node *ErrorExp() { return new IntConstant(lex, 0); }

    template<typename... Ts> Node *ErrorExp(const Ts &...args) {
        Error(args...);
        return ErrorExp();
    }

    // Stands in for a type that failed to parse.
    template<typename T = UnTypeRef> T ErrorType() { return T(type_any); }

    // Gives a reference to a generic type as many specializers as the type has generics, for
    // after an error in (or the absence of) its specializer list, since everything downstream
    // assumes those match.
    void PadSpecializers(SpecUDT &su) {
        while (su.specializers.size() < su.gudt->generics.size())
            su.specializers.push_back(&*ErrorType());
    }

    // Skips tokens until one of `ts`, or, when that is not found first, the end of the current
    // statement (the linefeed, dedent or end of file ending it), leaving that as the current
    // token. Bracketed groups and indented blocks in between are skipped whole, since stopping
    // inside one would make an enclosing construct end in the wrong place. Returns whether it
    // found one of `ts`. Whatever it stopped at is not blamed for further errors, since it is
    // not what the error was about.
    template<typename... Ts> bool SkipTo(Ts... ts) {
        int depth = 0;
        for (;; lex.Next()) {
            auto t = lex.token;
            if (t == T_ENDOFFILE || t == T_ENDOFINCLUDE) break;
            if (!depth && (((t == ts) || ...) || t == T_LINEFEED || t == T_DEDENT)) break;
            switch (t) {
                case T_INDENT:
                case T_LEFTPAREN:
                case T_LEFTBRACKET:
                case T_LEFTCURLY:
                case T_STR_INT_START:
                    depth++;
                    break;
                case T_DEDENT:
                case T_RIGHTPAREN:
                case T_RIGHTBRACKET:
                case T_RIGHTCURLY:
                case T_STR_INT_END:
                    // At depth 0 this closes a group the caller gave up on.
                    if (depth) depth--;
                    break;
                default:
                    break;
            }
        }
        lex.SuppressErrorsHere();
        return ((lex.token == ts) || ...);
    }

    // For loops over lines (statements, fields, enum values, cases): after parsing one, it
    // must be at the end of its line. Anything else there is an error, unless one was already
    // reported inside it (in which case it is what that error left behind), and gets skipped.
    void EndOfLine(int errors_before) {
        if (Either(T_LINEFEED, T_DEDENT, T_ENDOFFILE, T_ENDOFINCLUDE)) return;
        if (lex.num_errors == errors_before)
            Error("end of line expected, found ", Q(lex.TokStr()));
        SkipTo();
    }

    // The separator between two elements of a list ending in `closing`. When it is missing,
    // reports that and skips the rest of the list, leaving `closing` (if found) for the
    // caller. Returns whether the list continues.
    bool ListSep(TType sep, TType closing) {
        if (IsNext(sep)) return true;
        Expected(sep);
        SkipTo(closing);
        return false;
    }

    // The closing token of a bracketed group. When it is missing, skips to it (or the end of
    // the statement), such that the caller continues after the group.
    void ExpectClosing(TType t) {
        if (IsNext(t)) return;
        Expected(t);
        if (SkipTo(t)) lex.Next();
    }

    template<typename... Ts> void Warn(const Ts &...args) {
        lex.Warn(cat(args...), nullptr);
    }

    template<typename... Ts> void WarnAt(const Node *what, const Ts &...args) {
        lex.Warn(cat(args...), what ? &what->line : nullptr);
    }

    // Non-zero while parsing the initializer of a `member` declaration, holding
    // the number of enclosing function scopes at that point. That initializer
    // is the field's default, so it runs wherever the class is constructed and
    // can't see anything declared by the method it is written in. Anything the
    // initializer itself declares (a lambda's args, say) is fine.
    size_t field_init_scopes = 0;

    // Whether something declared by `sf_def` is one of those out of reach.
    bool OutsideFieldInit(SubFunction *sf_def) {
        // Index 0 is the top level, whose variables are globals, so in reach.
        for (size_t i = 1; i < field_init_scopes; i++)
            if (st.defsubfunctionstack[i] == sf_def) return true;
        return false;
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
        st.FunctionScopeCleanup(ov.gbody->Count());
        root = new Call(lex, sf);
    }

    void ParseStatements(Block *block, TType terminator) {
        for (;;) {
            auto errors_before = lex.num_errors;
            ParseTopExp(block);
            EndOfLine(errors_before);
            if (lex.token == T_ENDOFINCLUDE) {
                st.EndOfInclude();
                lex.PopIncludeContinue();
            } else if (!IsNext(T_LINEFEED)) {
                break;
            }
            if (Either(T_ENDOFFILE, T_DEDENT)) break;
        }
        if (terminator != T_NONE) Expect(terminator);
        if (!block->Arity()) {
            // Typically can't happen, but statements like "attribute" don't generate a node.
            Error("block can\'t be empty");
            block->Add(ErrorExp());
        }
        auto b = block->children.back();
        if (Is<EnumRef>(b) || Is<GUDTRef>(b) || Is<UDTRef>(b) || Is<FunRef>(b) || Is<Define>(b)) {
            if (terminator != T_ENDOFFILE) Error("last expression in block can\'t be a definition");
            block->Add(new IntConstant(lex, 0));
        }
        CleanupStatements(block);
    }

    void CleanupStatements(Block *list) {
        // See also Block::TypeCheck
        for (auto def : list->children) {
            if (auto er = Is<EnumRef>(def)) {
                st.UnregisterEnum(er->e);
            } else if (auto sr = Is<GUDTRef>(def)) {
                if (sr->gudt->predeclaration)
                    Error("pre-declared struct ", Q(sr->gudt->name), " never defined");
                UnregisterT(sr->gudt, st.gudts);
            } else if (auto sr = Is<UDTRef>(def)) {
                UnregisterT(sr->udt, st.udts);
            } else if (auto fr = Is<FunRef>(def)) {
                auto f = fr->sf->parent;
                if (!f->anonymous) st.Unregister(f);
            } else if (auto d = Is<Define>(def)) {
                // TODO: move more of this to TypeCheckFunctionDef ?
                for (auto p : d->tsids) {
                    auto id = p.sid->id;
                    id->static_constant =
                        id->single_assignment && d->child->IsConstInit();
                    if (!id->read && !id->static_constant && id->scopelevel != 1 &&
                        (id->name[0] != '_' || d->tsids.size() == 1))
                        Warn("unused variable ", Q(id->name));
                    if (id->predeclaration)
                      ErrorAt(d,"missing declaration for ", id->name);
                }
            } else if (auto r = Is<Return>(def)) {
                if (r != list->children.back())
                    Error("return must be last in block");
            } else if (def != list->children.back() && !def->ValidStatement() &&
                       !def->SideEffectRec()) {
                // Catches errors like == where = was intended, or a multi-line expression
                // where the operator connecting the lines was forgotten.
                // Calls conservatively count as having side effects, so are never warned about.
                // The last statement in a block is exempt since its value may be used.
                WarnAt(def, "expression statement has no effect");
            }
        };
    }

    void ParseTopExp(Block *list, bool isprivate = false) {
        switch(lex.token) {
            case T_NAMESPACE:
                if (st.scopelevels.size() != 1 || isprivate)
                    Error("namespace must be used at file scope");
                lex.Next();
                if (lex.token == T_LINEFEED) {
                    // Allow empty namespace decl to reset namespace to nothing.
                    st.current_namespace = {};
                } else {
                    st.current_namespace = lex.sattr;
                    Expect(T_IDENT);
                    while (IsNext(T_DOT)) {
                        lex.namespaces.insert(st.current_namespace);
                        st.current_namespace = st.StoreName(st.NameSpaced(lex.sattr));
                        Expect(T_IDENT);
                    }
                    lex.namespaces.insert(st.current_namespace);
                }
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
                auto errors_before = lex.num_errors;
                lex.Next();
                if (IsNext(T_FROM)) {
                    string fn = std::move(lex.sval);
                    Expect(T_STR);
                    AddDataDir(fn);
                } else {
                    string fn;
                    bool relative = false;
                    if (lex.token == T_STR) {
                        fn = std::move(lex.sval);
                        lex.Next();
                    } else {
                        // A leading dot makes the import relative to the directory of
                        // the importing file, e.g. "import .foo" or "import .sub.foo".
                        relative = IsNext(T_DOT);
                        fn = lex.sattr;
                        Expect(T_IDENT);
                        while (IsNext(T_DOT)) {
                            fn += "/";
                            fn += lex.sattr;
                            Expect(T_IDENT);
                        }
                        fn += ".lobster";
                        if (relative) {
                            // Prefix the directory of the current file. Normalize any \ to /
                            // (e.g. from a command-line main file) such that files imported
                            // both relatively and thru a path from the main dir get the
                            // same name for import deduplication.
                            auto dir = lex.filename.substr(
                                0, lex.filename.find_last_of("/\\") + 1);
                            for (auto &c : dir) if (c == '\\') c = '/';
                            fn = dir + fn;
                        }
                    }
                    // The lexer generates no linefeed at the end of a file, so
                    // an import may be the last thing in one.
                    if (!AtEndOfFile()) Expect(T_LINEFEED);
                    // What a malformed import names is not worth looking for.
                    if (lex.num_errors != errors_before) break;
                    // A file already imported elsewhere is not included again,
                    // and thus also has no T_ENDOFINCLUDE to close a scope with.
                    if (lex.Include(fn, true, relative)) st.StartOfInclude();
                    // Parses the first statement of the included file, or, if
                    // there was none, the next statement of the current one.
                    if (!AtEndOfFile()) ParseTopExp(list);
                }
                break;
            }
            case T_STRUCT:
                lex.Next();
                ParseTypeDecl(true, isprivate, list, false);
                break;
            case T_CLASS:
                lex.Next();
                ParseTypeDecl(false, isprivate, list, false);
                break;
            case T_ABSTRACT:
                lex.Next();
                if (IsNext(T_STRUCT)) {
                    ParseTypeDecl(true, isprivate, list, true);
                } else {
                    Expect(T_CLASS);
                    ParseTypeDecl(false, isprivate, list, true);
                }
                break;
            case T_UNION: {
                lex.Next();
                auto is_struct = IsNext(T_STRUCT);
                if (!is_struct && !IsNext(T_CLASS))
                    Error(Q("class"), " or ", Q("struct"), " expected after ", Q("union"),
                          ", found ", Q(lex.TokStr()));
                ParseUnionDecl(is_struct, isprivate, list);
                break;
            }
            case T_CONSTRUCTOR:
            case T_FUN: {
                auto is_constructor = lex.token == T_CONSTRUCTOR;
                lex.Next();
                list->Add(ParseNamedFunctionDefinition(is_constructor, isprivate, nullptr));
                break;
            }
            case T_ENUM:
            case T_ENUM_FLAGS: {
                bool incremental = lex.token == T_ENUM;
                lex.Next();
                Line line = lex;
                int64_t cur = incremental ? 0 : 1;
                auto enumname = st.MaybeMakeNameSpace(ExpectId(), true);
                auto def = st.EnumLookup(enumname, true);
                def->isprivate = isprivate;
                def->flags = !incremental;
                Expect(T_COLON);
                if (!IsNext(T_INDENT)) {
                    Expected(T_INDENT);
                } else {
                    for (;;) {
                        auto errors_before = lex.num_errors;
                        auto evname = st.MaybeMakeNameSpace(ExpectId(), true);
                        if (IsNext(T_ASSIGN)) {
                            auto e = ParseExp();
                            VTValue val;
                            auto t = e->ConstVal(nullptr, val);
                            delete e;
                            if (t != V_INT)
                                Error("enum value expression must evaluate to an integer constant");
                            else
                                cur = val.i;
                        }
                        auto ev = st.EnumValLookup(evname, true);
                        ev->isprivate = isprivate;
                        ev->val = cur;
                        ev->e = def;
                        def->vals.emplace_back(ev);
                        if (incremental) cur++; else cur *= 2;
                        if (IsNext(T_COMMA)) continue;
                        EndOfLine(errors_before);
                        if (!IsNext(T_LINEFEED) || Either(T_ENDOFFILE, T_DEDENT)) break;
                    }
                    Expect(T_DEDENT);
                }
                list->Add(new EnumRef(line, def));
                break;
            }
            case T_VAR:
            case T_CONST: {
                auto def = ParseDefine(isprivate, false, false);
                if (st.scopelevels.size() == 1 && def->tsids.size() == 1 &&
                    !Is<DefaultVal>(def->child)) {
                    def->tsids[0].sid->id->toplevel_initializer = def->child;
                }
                list->Add(def);
                break;
            }
            case T_PREFERFREE: {
                lex.Next();
                if (lex.token == T_FROM) {
                    lex.Next();
                    for (;;) {
                        auto id = ExpectId();
                        st.declared_explicit_free_variables.insert(id);
                        if (!IsNext(T_COMMA)) break;
                    }
                } else if (lex.token != T_VAR && lex.token != T_CONST) {
                    Error("nonlocal must be followed by var, let or from");
                } else {
                    list->Add(ParseDefine(isprivate, false, true));
                }
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
                UnTypeRef type = (UnType *)nullptr;
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
                def->tsids.push_back({ id->cursid, type });
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
                    cdef->tsids.push_back({ cid->cursid, { type_int } });
                    // Add to toplevel scope too.
                    st.toplevel->parent->overloads[0]->gbody->Add(cdef);
                }
                auto statik = new Static(lex, init->Clone(true));
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
                if (gudtstack.empty()) {
                    Error("member declaration outside of class scope");
                    break;
                }
                // FIXME: this would also allow it to be declared in nested functions, which is
                // not really a direct problem but may want to tighten it up.
                auto gudt = gudtstack.back();
                // This is an arbitrary restriction that we could lift, just doesn't seem
                // great to have invisble extra members in structs.
                if (gudt->is_struct) Error("member declaration only allowed in classes");
                // TODO: This is not great: when "member" is used in an inline method decl, this should
                // never happen, and it can later still be subclassed, but when it is used outside
                // in a free-standing method we can't allow new fields to be added when a subclass
                // has already copied them. We can maybe lift this restriction.
                if (gudt->has_subclasses) Error("member cannot be added in freestanding method to class that has been subclassed");
                st.bound_typevars_stack.push_back(gudt->generics);
                auto field_idx = gudt->fields.size();
                // FIXME: this will lookup idents in this exp in the current context, and it should do so
                // outside the current class it is in somehow.
                ParseField(gudt, true, true);
                // Only the method this sits in may access it, see Dot::TypeCheck.
                gudt->fields.back().member_of = st.defsubfunctionstack.back()->overload;
                st.bound_typevars_stack.pop_back();
                auto initc = gudt->fields.back().gdefaultval->Clone(true);
                SpecIdent *this_sid = nullptr;
                if (frame) {
                    // Create int field to store frame count.
                    auto fname = gudt->fields.back().id->name;
                    auto &fcsfield = st.FieldDecl(fname + "_frame_count", gudt);
                    gudt->fields.push_back(
                        Field(&fcsfield, { type_int }, new IntConstant(lex, 0), true, false, lex));
                    SymbolTable::WithStackElem *wse = nullptr;
                    st.LookupWithStruct(fname, wse);
                    assert(wse);
                    this_sid = wse->id->cursid;
                }
                auto member = new Member(lex, initc);
                member->gudt = gudt;
                member->field_idx = field_idx;
                member->frame = frame;
                member->this_sid = this_sid;
                for (UDT *udt = gudt->first; udt; udt = udt->next) {
                    // New fields have been added possibly non-inline, run this again to be sure.
                    st.ResolveFields(*udt, lex);
                }
                list->Add(member);
                break;
            }
            case T_ATTRIBUTE: {
                lex.Next();
                ParseAttribute(st.defsubfunctionstack.back()->attributes);
                break;
            }
            case T_GUARD: {
                lex.Next();
                list->Add(ParseGuard(lex, list));
                break;
            }
            // These two also exist at factor level (where a declaration inside them is not allowed)
            case T_IF: {
                lex.Next();
                list->Add(ParseIf(lex, list, true));
                break;
            }
            case T_ASSERT: {
                lex.Next();
                Line line = lex;
                auto cond = ParseExpCond(list);
                list->Add(new Assert(line, cond));
                break;
            }
            default: {
                if (isprivate)
                    Error("private only applies to declarations");
                ParseExpStat(list, true);
                break;
            }
        }
    }

    Define *ParseDefine(bool isprivate, bool iscond, bool ispreferfree) {
        auto isconst = lex.token == T_CONST;
        lex.Next();
        auto def = new Define(lex, nullptr);
        bool has_predeclaration_init = false;
        for (;;) {
            auto idname = ExpectId();
            bool withtype = lex.token == T_TYPEIN;
            UnTypeRef type = (UnType *)nullptr;
            if (lex.token == T_COLON || withtype) {
                lex.Next();
                type = ParseType(withtype);
            }
            auto id = st.LookupDef(idname, true, withtype);
            if (id->predeclaration) {
                if (iscond)
                    Error("can\'t define pre-declaration in condition");
                if (type.Null() || !type->Equal(*id->giventype))
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
            if (ispreferfree) id->preferfree = true;
            def->tsids.push_back({ id->cursid, type });
            id->giventype = type;
            if (!IsNext(T_COMMA)) break;
            if (iscond)
                Error("can only declare a single variable in condition");
        }
        if (IsNext(T_ASSIGN)) {
            def->child = iscond ? ParseOpExp(5 /* no and/or */) : ParseMultiRet(ParseOpExp());
        } else if (iscond) {
            Error("variable definition in condition must have initializer");
            def->child = ErrorExp();
        } else if (has_predeclaration_init) {
            Error("missing initialization");
            def->child = ErrorExp();
        } else {
            if (st.scopelevels.size() != 1)
                // For now, since we track it in Idents which don't work with specialization in TC.
                Error("variable pre-declarations only allowed at top level");
            for (auto &tsid : def->tsids) {
                if (tsid.giventype.Null())
                    Error("a variable pre-declaration must have a type specified");
                tsid.sid->id->predeclaration = true;
            }
            def->child = new DefaultVal(lex);
        }
        return def;
    }

    pair<string_view, string_view> ParseAttribute(map<string_view, string_view> &attributes) {
        auto key = ExpectId();
        string_view value;
        if (IsNext(T_ASSIGN)) {
            switch (lex.token) {
                case T_IDENT:
                case T_STR:
                case T_INT:
                case T_FLOAT:
                    value = lex.sattr;
                    lex.Next();
                    break;
                default:
                    Error("attribute value must be an identifier or string/int/float constant");
            }
        }
        attributes[key] = value;
        return { key, value };
    }

    // The storage width a field is given behind its type, `x:int<8>`, see Field::bits: a
    // multiple of 8 bits for an int (or enum), 32 for a float, and 64 means the whole slot
    // it would have anyway. Not part of the type: the field holds a plain int/float as far
    // as everything but its storage is concerned, so this is only accepted here.
    int ParseFieldBits(UnTypeRef type) {
        Expect(T_LT);
        int64_t bits = 0;
        if (lex.token == T_INT) {
            bits = lex.ival;
            lex.Next();
        } else {
            Error("bit width expected, found ", Q(lex.TokStr()));
        }
        // This may be the end of the line, so make sure Lex doesn't see it as a GT op.
        lex.OverrideCont(false);
        Expect(T_GT);
        if (type->t == V_INT) {
            if (bits <= 0 || bits > 64 || bits % 8) {
                Error("bit width of an int field must be a multiple of 8 up to 64, not ", bits);
                bits = 0;
            }
        } else if (type->t == V_FLOAT) {
            if (bits != 32 && bits != 64) {
                Error("bit width of a float field must be 32 or 64, not ", bits);
                bits = 0;
            }
        } else {
            Error("only int, float and enum fields can be given a bit width");
            bits = 0;
        }
        return bits == 64 ? 0 : (int)bits;
    }

    void ParseField(GUDT *gudt, bool member_private, bool local_member) {
        ExpectId();
        auto &sfield = st.FieldDecl(lastid, gudt);
        UnTypeRef type = (UnType *)nullptr;
        int bits = 0;
        if (IsNext(T_COLON)) {
            type = ParseType(false);
            if (lex.token == T_LT) bits = ParseFieldBits(type);
        }
        Node *init = nullptr;
        if (IsNext(T_ASSIGN)) {
            // Only for `member`: the default of an ordinary field may refer to
            // an enclosing function's locals, since it is cloned into each
            // construction site, which is typically in that same scope. A
            // `member` is declared inside a method, but its default is used
            // wherever the class gets constructed, which is not.
            DS<size_t> ds(field_init_scopes,
                          local_member ? st.defsubfunctionstack.size() : size_t(0));
            init = ParseExp();
        }
        if (local_member && !init) {
            Error("must specify default value");
            init = ErrorExp();
        }
        if (type.Null()) {
            if (!init) {
                Error("must specify either type or default value");
                type = ErrorType();
            } else {
                type = init->CFType();
                if (type.Null()) {
                    // More complex exp, must be typechecked later.
                    type = type_any;
                }
            }
        }
        gudt->fields.push_back(Field(&sfield, type, init, member_private, !local_member, lex));
        gudt->fields.back().bits = bits;
    }

    pair<GUDT *, UDT *> ParseSup(bool is_struct) {
        ExpectId();
        auto p = st.StructOrSpecializationUse(lastid);
        if (is_struct != p.first->is_struct) Error("class/struct must match parent");
        return p;
    }

    void ParseTypeDecl(bool is_struct, bool isprivate, Block *parent_list, bool is_abstract) {
        Line line = lex;
        auto sname = st.MaybeMakeNameSpace(ExpectId(), true);
        if (IsNext(T_ASSIGN)) {
            // A specialization of an existing struct
            // ParseSup's result is unpacked by hand rather than with a structured binding,
            // since capturing one in a lambda (below) is not allowed before C++20, and the NDK's
            // clang enforces that.
            auto gsup = ParseSup(is_struct).first;
            auto udt = st.MakeSpecialization(*gsup, sname, true, true);
            Expect(T_LT);
            ParseSpecializerList([&]() {
                auto type = ParseType<TypeRef>(false, nullptr, false);
                if (udt->bound_generics.size() == gsup->generics.size())
                    Error("too many type specializers");
                else
                    udt->bound_generics.push_back(type);
            });
            if (isprivate != gsup->isprivate) Error("specialization must have same privacy level");
            if (gsup->predeclaration) Error("must specialize fully defined type");
            if (is_abstract) Error("specialization cannot be abstract");
            if (udt->bound_generics.size() != gsup->generics.size()) {
                Error("missing specializers");
                while (udt->bound_generics.size() != gsup->generics.size())
                    udt->bound_generics.push_back(ErrorType<TypeRef>());
            }
            st.ResolveFields(*udt, lex);
            parent_list->Add(new UDTRef(line, udt));
            return;
        }
        GUDT *gudt = st.LookupStruct(sname);
        bool was_predeclaration = gudt && gudt->predeclaration;
        gudt = &st.StructDecl(sname, is_struct, lex);
        UDT *udt = nullptr;
        if (Either(T_COLON, T_LT)) {
            // A regular struct declaration
            gudt->is_abstract = is_abstract;
            gudt->isprivate = isprivate;
            if (IsNext(T_LT)) {
                if (was_predeclaration)
                    Error("pre-declared type cannot be made generic afterwards");
                ParseSpecializerList([&]() {
                    // A missing name declares nothing, rather than a placeholder generic
                    // that then makes every use of the type have too few specializers.
                    if (!IsNextId()) {
                        Expected(T_IDENT);
                        return;
                    }
                    auto id = lastid;
                    for (auto &g : gudt->generics)
                        if (g.tv->name == id)
                            Error("re-declaration of generic type");
                    gudt->generics.push_back({ TypeRef { nullptr }, st.NewGeneric(id) });
                });
            }
            Expect(T_COLON);
            if (!gudt->IsGeneric()) {
                // We create a specialization since there will be no explicit specializations.
                // Create it early since the superclass specializers below may refer to it.
                if (was_predeclaration) {
                    udt = gudt->first;
                    assert(udt && !udt->next);
                } else { 
                    udt = st.MakeSpecialization(*gudt, sname, false, false);
                }
            }
            if (lex.token == T_IDENT) {
                // Unpacked by hand, see the lambda capture comment above.
                auto sup = ParseSup(is_struct);
                auto gsup = sup.first;
                auto ssup = sup.second;
                if (gsup == gudt) {
                    Error("can\'t inherit from ", Q(lastid));
                    gsup = &st.ErrorStruct();
                    ssup = nullptr;
                }
                InheritFrom(gudt, gsup);
                st.bound_typevars_stack.push_back(gudt->generics);
                if (ssup) {
                    gudt->gsuperclass = { &ssup->thistype };
                    st.bound_typevars_stack.pop_back();
                } else {
                    gudt->gsuperclass = { st.NewSpecUDT(gsup) };
                    auto &specializers = gudt->gsuperclass->spec_udt->specializers;
                    if (IsNext(T_LT)) {
                        ParseSpecializerList([&]() {
                            auto type = ParseType(false);
                            if (specializers.size() == gsup->generics.size())
                                Error("too many type specializers");
                            else
                                specializers.push_back(&*type);
                        });
                    }
                    st.bound_typevars_stack.pop_back();
                    if (specializers.size() < gsup->generics.size())
                        Error("too few type specializers");
                    PadSpecializers(*gudt->gsuperclass->spec_udt);
                }
            }
            if (IsNext(T_INDENT)) {
                bool fieldsdone = false;
                st.bound_typevars_stack.push_back(gudt->generics);
                for (;;) {
                    auto errors_before = lex.num_errors;
                    if (IsNext(T_ATTRIBUTE)) {
                        auto [key, value] = ParseAttribute(gudt->attributes);
                        if (key == "serializable") {
                            // A struct in an abstract struct family carries its type, so it
                            // can be serialized by id like a class.
                            if (!udt || gudt->IsGeneric() || is_abstract ||
                                (is_struct && !FamilyRootOf(gudt))) {
                                Error("serializable attribute only for non-generic non-abstract"
                                      " classes and structs with an abstract struct superclass");
                            } else {
                                if (value.empty()) {
                                    ++serializable_id_last;
                                } else {
                                    serializable_id_last = parse_int<int>(value);
                                    if (serializable_id_last < 0 || serializable_id_last > 0x10000)
                                        Error("serializable attribute value out of range");
                                }
                                udt->serializable_id = serializable_id_last;
                            }
                        }
                    } else {
                        bool member_private = IsNext(T_PRIVATE);
                        if (IsNext(T_CONSTRUCTOR)) {
                            Error("constructors must be declared outside the type since they don't"
                                  " have access to the instance");
                        } else if (IsNext(T_FUN)) {
                            fieldsdone = true;
                            parent_list->Add(ParseNamedFunctionDefinition(false, member_private, gudt));
                        } else {
                            if (fieldsdone) Error("fields must be declared before methods");
                            ParseField(gudt, member_private, false);
                        }
                    }
                    EndOfLine(errors_before);
                    if (!IsNext(T_LINEFEED) || Either(T_ENDOFFILE, T_DEDENT)) break;
                }
                Expect(T_DEDENT);
                st.bound_typevars_stack.pop_back();
            }
            // A struct with an abstract struct superclass has the type field, so it may be
            // without fields of its own, like a value of an enum.
            if (gudt->fields.empty() && gudt->is_struct && !gudt->is_abstract &&
                !FamilyRootOf(gudt))
                Error("structs cannot be empty");
        } else {
            // Abstract or pre-declaration.
            if (is_abstract) {
                gudt->is_abstract = true;
            } else {
                gudt->predeclaration = true;
            }
            if (was_predeclaration && gudt->predeclaration) {
                // Multiple pre-declarations, don't add another specialization.
                udt = gudt->first;
                assert(udt && !udt->next);
            } else {
                // Also make a specialization, since it will typically be referred to in fields of
                // other types (that get resolved) before this is fully declared.
                udt = st.MakeSpecialization(*gudt, sname, false, false);
            }
        }
        FinishTypeDecl(gudt, udt, line, parent_list);
    }

    // Makes `gudt` a subclass of `gsup`: it starts out with the fields of `gsup`.
    void InheritFrom(GUDT *gudt, GUDT *gsup) {
        gsup->has_subclasses = true;
        for (auto &fld : gsup->fields) {
            gudt->fields.push_back(fld);
        }
    }

    // The end of any type declaration, once its generics and fields are known: gives the
    // unspecialized type and (for a non-generic type) its one specialization their types,
    // and records the declaration.
    void FinishTypeDecl(GUDT *gudt, UDT *udt, const Line &line, Block *parent_list) {
        gudt->unspecialized.specializers.clear();
        for (auto &g : gudt->generics) {
            auto type = g.type.Null()
                ? &g.tv->thistype
                : g.type;
            gudt->unspecialized.specializers.push_back(&*type);
            g.type = type;
        }
        if (udt && !gudt->predeclaration) {
            for (auto &g : gudt->generics) {
                auto type =
                    g.type.Null() ? &g.tv->thistype : g.type;
                assert(!st.IsGeneric({ type }));
                udt->bound_generics.push_back(type);
            }
            st.ResolveFields(*udt, lex);
            parent_list->Add(new UDTRef(line, udt));
        }
        parent_list->Add(new GUDTRef(line, gudt, gudt->predeclaration));
    }

    // Declares the non-generic type `sname` (possibly pre-declared) with its single
    // specialization, as ParseTypeDecl does for `class sname:` / `struct sname:`.
    pair<GUDT *, UDT *> DeclareType(string_view sname, bool is_struct, bool isprivate,
                                    bool is_abstract) {
        auto gudt = st.LookupStruct(sname);
        bool was_predeclaration = gudt && gudt->predeclaration;
        gudt = &st.StructDecl(sname, is_struct, lex);
        gudt->is_abstract = is_abstract;
        gudt->isprivate = isprivate;
        UDT *udt = nullptr;
        if (was_predeclaration) {
            udt = gudt->first;
            assert(udt && !udt->next);
        } else {
            udt = st.MakeSpecialization(*gudt, sname, false, false);
        }
        return { gudt, udt };
    }

    // `union class U:` with an indented list of `M(field:type, ...)` lines (the parens may
    // be left out for a member without fields) declares `abstract class U` and, for each
    // line, `class M : U` with those fields. `union struct U:` does the same with structs,
    // i.e. declares an abstract struct family.
    void ParseUnionDecl(bool is_struct, bool isprivate, Block *parent_list) {
        Line line = lex;
        auto sname = st.MaybeMakeNameSpace(ExpectId(), true);
        auto up = DeclareType(sname, is_struct, isprivate, true);
        auto gudt = up.first;
        auto udt = up.second;
        FinishTypeDecl(gudt, udt, line, parent_list);
        Expect(T_COLON);
        if (!IsNext(T_INDENT)) {
            Expected(T_INDENT);
            return;
        }
        for (;;) {
            auto errors_before = lex.num_errors;
            Line mline = lex;
            auto mname = st.MaybeMakeNameSpace(ExpectId(), true);
            // Unpacked by hand, see the lambda capture comment in ParseTypeDecl.
            auto mp = DeclareType(mname, is_struct, isprivate, false);
            auto mgudt = mp.first;
            auto mudt = mp.second;
            InheritFrom(mgudt, gudt);
            mgudt->gsuperclass = { &udt->thistype };
            if (IsNext(T_LEFTPAREN)) {
                ParseVector([&]() {
                    ParseField(mgudt, IsNext(T_PRIVATE), false);
                }, T_RIGHTPAREN);
            }
            FinishTypeDecl(mgudt, mudt, mline, parent_list);
            EndOfLine(errors_before);
            if (!IsNext(T_LINEFEED) || Either(T_ENDOFFILE, T_DEDENT)) break;
        }
        Expect(T_DEDENT);
    }

    FunRef *ParseNamedFunctionDefinition(bool is_constructor, bool isprivate, GUDT *self) {
        if (IsNext(T_OPERATOR)) {
            auto op = lex.token;
            if ((op < T_PLUS || op > T_ASREQ) && op != T_LEFTBRACKET)
                Error(cat("illegal token for operator overloading: ", TName(op)));
            auto idname = cat(TName(T_OPERATOR), TName(op));
            lex.Next();
            if (op == T_LEFTBRACKET) {
                Expect(T_RIGHTBRACKET);
                idname += ']';
            }
            return ParseFunction(&idname, is_constructor, isprivate, true, true, self, 2);
        } else {
            // TODO: also exclude functions from namespacing whose first arg is a type namespaced to
            // current namespace (which is same as !self).
            auto idname = string(st.MaybeMakeNameSpace(ExpectId(), !self));
            return ParseFunction(&idname, is_constructor, isprivate, true, true, self);
        }
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

    void GenImplicitGenericForLastArg(SubFunction *sf, Overload *ov) {
        static const char *typevar_names = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        string_view nn;
        size_t gen_generics = 0;
        {
            again:
            if (gen_generics == 26) {
                Error("too many implicit generics");
                ov->givenargs.push_back({ ErrorType() });
                return;
            }
            nn = { typevar_names + gen_generics++, 1 };
            for (auto &btv : sf->generics) if (btv.tv->name == nn) goto again;
        }
        auto ng = st.NewGeneric(nn);
        sf->generics.push_back({ { nullptr }, ng });
        ov->givenargs.push_back({ &ng->thistype });
    }

    void ParseBody(Block *block, int for_nargs, bool allow_multi_assign) {
        block_stack.push_back({ block, for_nargs, 0 });
        if (IsNext(T_INDENT)) {
            ParseStatements(block, T_DEDENT);
        } else {
            ParseExpStat(block, allow_multi_assign);
            CleanupStatements(block);
        }
        block_stack.pop_back();
    }

    FunRef *ParseFunction(string *name, bool is_constructor, bool isprivate, bool parens,
                          bool parseargs, GUDT *self, size_t maxargs = 1024) {
        bool in_class = !!self;
        auto sf = st.FunctionScopeStart();
        auto ov = new Overload{ lex, isprivate };
        if (name) {
            // Parse generic params if any.
            // TODO: can this be extended to non-named functions syntactically?
            if (IsNext(T_LT)) {
                sf->explicit_generics = true;
                ParseSpecializerList([&]() {
                    // See the same in ParseTypeDecl.
                    if (!IsNextId()) {
                        Expected(T_IDENT);
                        return;
                    }
                    auto ng = st.NewGeneric(lastid);
                    for (auto &btv : sf->generics) if (btv.tv->name == ng->name)
                        Error("re-definition of generic ", Q(ng->name));
                    sf->generics.push_back({ { nullptr }, ng });
                });
            }
        }
        st.bound_typevars_stack.push_back(sf->generics);
        if (parens) Expect(T_LEFTPAREN);
        size_t nargs = 0;
        bool self_withtype = false;
        if (self) {
            nargs++;
            self_withtype = true;
            auto id = st.LookupDef("this", false, true);
            auto atype = &self->unspecialized_type;
            ov->givenargs.push_back({ atype });
            st.AddWithStruct(GetGUDTAny(atype), id, sf);
            id->cursid->withtype = true;
        }
        bool non_inline_method = false;
        node_small_vector default_args;
        if (lex.token != T_RIGHTPAREN && parseargs) {
            for (;;) {
                ExpectId();
                nargs++;
                bool withtype = lex.token == T_TYPEIN;
                auto id = st.LookupDef(lastid, false, withtype);
                if (parens && (lex.token == T_COLON || withtype)) {
                    lex.Next();
                    auto atype = ParseType(withtype, nullptr);
                    if (withtype) st.AddWithStruct(GetGUDTAny(atype), id, sf);
                    if (nargs == 1 && (atype->t == V_UUDT || IsUDT(atype->t))) {
                        non_inline_method = true;
                        self = GetGUDTAny(atype);
                        self_withtype = withtype;
                        st.bound_typevars_stack.push_back(self->generics);
                    }
                    ov->givenargs.push_back({ atype });
                } else {
                    GenImplicitGenericForLastArg(sf, ov);
                }
                if (parens && IsNext(T_ASSIGN)) {
                    st.PopOutOfFunctionScope([&]() {
                        default_args.push_back(ParseExp());
                    });
                } else if (!default_args.empty()) {
                    Error("missing default argument");
                    default_args.push_back(ErrorExp());
                }
                if (!IsNext(T_COMMA)) break;
                if (sf->args.size() == maxargs) Error("too many arguments for ", Q(*name));
            }
        }
        if (parens) ExpectClosing(T_RIGHTPAREN);
        auto &f = name ? st.FunctionDecl(*name, nargs) : st.CreateFunction("");
        auto nf = natreg.FindNative(f.name);
        if (nf && nf->args.size() >= nargs) {
            // TODO: could allow less args if we check nf's default args.
            // TODO: with receiver-based selection in GenericCall, an overload
            // whose first arg is a class/struct type could now safely shadow a
            // builtin of the same arity.
            Error("cannot define function ", Q(f.name), " with ", nargs,
                  nargs == 1 ? " argument" : " arguments",
                  ": a call with that many would always select the built-in ",
                  Q(Signature(*nf)), ". Give it more arguments, a different name, or put it"
                  " in a namespace");
        }
        // Check default args are being used consistently with the overloads & siblings.
        auto is_constructor_of = is_constructor ? &st.StructUse(*name) : nullptr;
        if (f.overloads.empty()) {
            f.default_args = default_args;
            f.is_constructor_of = is_constructor_of;
        } else {
            if (f.default_args.size() != default_args.size()) {
                Error("number of default arguments must be the same as previous overload");
                for (auto da : default_args) delete da;
            } else {
                for (auto [i, da] : enumerate(f.default_args)) {
                    if (da && !da->Equal(default_args[i]))
                        Error("default argument ", i + 1, " must be same as previous overload");
                    delete default_args[i];
                }
            }
            if (f.is_constructor_of != is_constructor_of)
                Error("either all overloads of ", Q(f.name), " must be a constructor, or none");
        }
        // Connect the overload.
        f.overloads.emplace_back(ov);
        ov->method_of = self;
        sf->SetParent(f, *ov);
        if (name && IsNext(T_LEFTBRACKET)) {
            for (;;) {
                auto id = ExpectId();
                for (auto fvd : ov->freevardecls) {
                    if (id == fvd->name) {
                        Error("explicit free variable double declaration: ", Q(id));
                    }
                }
                for (auto f : reverse(namedfunctionstack)) {
                    auto ov = f->overloads.back();
                    for (auto fvd : ov->freevardecls) {
                        if (id == fvd->name) {
                            Warn("explicit free variable shadowing: ", Q(id));
                        }
                    }
                }
                if (!st.declared_explicit_free_variables.empty()) {
                    if (!st.declared_explicit_free_variables.count(id)) {
                        Error("explicit free variable not declared in ", Q("nonlocal from"), " : ", Q(id));
                    }
                }
                UnTypeRef type = (UnType *)nullptr;
                if (IsNext(T_COLON)) {
                    type = ParseType(false);
                }
                ov->freevardecls.push_back(new ExplicitFreeVar { id, type });
                if (!IsNext(T_COMMA)) break;
            }
            ExpectClosing(T_RIGHTBRACKET);
        }
        if (IsNext(T_RETURNTYPE)) {  // Return type decl.
            sf->returngiventype = ParseTypes(sf);
        }
        auto has_body = IsNext(T_COLON);
        if (!has_body &&
            (!name || !Either(T_LINEFEED, T_DEDENT, T_ENDOFFILE, T_ENDOFINCLUDE))) {
            // An anonymous function always has a body, and so does a header that is followed
            // by anything at all (an indented block, or a one-liner): the `:` is missing.
            Expected(T_COLON);
            has_body = true;
        }
        if (!has_body) {
            // This must be a function type.
            if (f.istype || f.overloads.size() > 1)
                Error("redefinition of function type ", Q(*name));
            f.istype = true;
            sf->typechecked = true;
            if (in_class || st.scopelevels.size() != 2)
                Error("function type must be declared at top level");
            for (auto [i, arg] : enumerate(sf->args)) {
                // The self argument of a method is of the class's unspecialized type, whose
                // specializers are only filled in once its body is done, so can't be
                // asked whether it is generic (a method can't be a function type anyway).
                if (!in_class && st.IsGeneric(ov->givenargs[i]))
                    Error("function type arguments can\'t be generic (missing ", Q(":"), " ?)");
                // No idea what the function is going to be, so have to default to borrow.
                arg.sid->lt = LT_BORROW;
            }
            if (sf->returngiventype.Null()) {
                Error("missing return type or ", Q(":"), " in function definition header");
                sf->returngiventype = ErrorType();
            }
            if (!sf->generics.empty())
                Error("function type cannot have generics");
            sf->reqret = sf->returngiventype->NumValues();
        }
        // Check if there's mixed function types.
        for (auto ff = st.GetFirstFunction(f.name); ff; ff = ff->sibf) {
            if (ff == &f) continue;
            // FIXME: we shouldn't have `istype`, instead function types should not be a `Function`
            // but their own thing with their own lookup.
            if (ff->istype != f.istype)
                Error("function ", Q(f.name), " is declared both as function type and regular function");
        }
        if (name) {
            if (f.overloads.size() > 1) {
                // We could check here for "double declaration", but since that entails
                // detecting what is a legit overload or not, this is in general better left to the
                // type checker.
                if (!f.nargs()) Error("double declaration of ", Q(f.name));
            }
            namedfunctionstack.push_back(&f);
            if (is_constructor_of) {
                is_constructor_of->has_constructor_function = true;
            }
        } else {
            f.anonymous = true;
        }
        if (self_withtype) gudtstack.push_back(self);
        // Parse the body.
        Line line = lex;
        if (!f.istype) {
            auto block = new Block(lex);
            ov->gbody = block;
            ParseBody(block, -1, false);
            ImplicitReturn(*ov);
        }
        if (self_withtype) gudtstack.pop_back();
        if (name) namedfunctionstack.pop_back();
        if (non_inline_method) st.bound_typevars_stack.pop_back();
        st.bound_typevars_stack.pop_back();
        st.FunctionScopeCleanup(ov->gbody ? ov->gbody->Count() : 0);
        return new FunRef(line, sf);
    }

    // The type specializers (or generic type names) between < and >, `f` parsing each. The >
    // is an operator to the lexer, which would take a line break after it as continuing the
    // line.
    template<typename F> void ParseSpecializerList(F f) {
        lex.allow_shift_right = false;
        for (;;) {
            f();
            if (lex.token == T_GT || !ListSep(T_COMMA, T_GT)) break;
        }
        lex.allow_shift_right = true;
        lex.OverrideCont(false);
        Expect(T_GT);
    }

    // The return types of a function, which are a tuple when there is more than one.
    UnTypeRef ParseTypes(SubFunction *sfreturntype) {
        auto dest = ParseType(false, sfreturntype);
        if (!IsNext(T_COMMA)) return dest;
        vector<UnTypeRef> types;
        types.push_back(dest);
        do {
            types.push_back(ParseType(false, sfreturntype));
        } while (IsNext(T_COMMA));
        dest = st.NewTuple(types.size());
        for (auto [i, type] : enumerate(types))
            dest->Set(i, &*type, IsRefNil(type->t) ? LT_KEEP : LT_ANY);
        return dest;
    }

    TypeRef FindTypeVar(string_view name) {
        for (auto &gv : reverse(st.bound_typevars_stack)) {
            for (auto &btv : gv) {
                if (btv.tv->name == name) {
                    return &btv.tv->thistype;
                }
            }
        }
        return nullptr;
    }

    template<typename T = UnTypeRef> T ParseType(bool withtype, SubFunction *sfreturntype = nullptr, bool allow_unresolved = true) {
        T dest;
        switch(lex.token) {
            case T_INTTYPE:
                dest = type_int;
                lex.Next();
                break;
            case T_FLOATTYPE:
                dest = type_float;
                lex.Next();
                break;
            case T_STRTYPE:
                dest = type_string;
                lex.Next();
                break;
            case T_RESOURCE: {
                lex.Next();
                Expect(T_LT);
                auto id = ExpectId();
                auto rt = LookupResourceType(id);
                if (rt) {
                    dest = &rt->thistype;
                } else {
                    Error("unknown resource type ", Q(id));
                    dest = ErrorType<T>();
                }
                // This may be the end of the line, so make sure Lex doesn't see it
                // as a GT op.
                lex.OverrideCont(false);
                Expect(T_GT);
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
                    if (!allow_unresolved) Error("no type variables allowed in concrete type");
                    lex.Next();
                    break;
                }
                // Not a structured binding, since those can't be captured by the lambdas
                // below in C++17.
                auto gudt_udt = st.StructOrSpecializationUse(lex.sattr);
                auto gudt = gudt_udt.first;
                auto udt = gudt_udt.second;
                lex.Next();
                if (udt) {
                    dest = &udt->thistype;
                    if (IsNext(T_LT)) {
                        Error("concrete type ", Q(udt->g.name), " does not take generic parameters");
                        ParseSpecializerList([&]() { ParseType<T>(false, nullptr, allow_unresolved); });
                    }
                } else {
                    if (IsNext(T_LT)) {
                        if (!allow_unresolved) {
                            // FIXME: can instead see if any equivalent named specializations
                            // exist, or even create a specialization if all types are concrete?
                            Error("no ad-hoc specialization allowed in concrete type (use a "
                                    "named specialization)");
                        }
                        dest = (const Type *)st.NewSpecUDT(gudt).get();
                        auto &specializers = dest->spec_udt->specializers;
                        ParseSpecializerList([&]() {
                            auto type = ParseType<T>(false, nullptr, allow_unresolved);
                            if (specializers.size() == gudt->generics.size())
                                Error("too many type specializers");
                            else
                                specializers.push_back(&*type);
                        });
                        if (specializers.size() < gudt->generics.size())
                            Error("too few type specializers");
                        PadSpecializers(*dest->spec_udt);
                    } else {
                        if (!gudt->predeclaration) {
                            if (allow_unresolved)
                                Error("use of type ", Q(gudt->name), " requires specializers");
                            else
                                Error(Q(gudt->name), " is not a concrete type");
                        }
                        dest = (const Type *)st.NewSpecUDT(gudt).get();
                        PadSpecializers(*dest->spec_udt);
                    }
                }
                break;
            }
            case T_LEFTBRACKET: {
                lex.Next();
                T elem = ParseType<T>(false, nullptr, allow_unresolved);
                ExpectClosing(T_RIGHTBRACKET);
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
                [[fallthrough]];
            default:
                Error("illegal type syntax: ", Q(lex.TokStr()));
                dest = ErrorType<T>();
        }
        assert(!dest.Null() && dest->t != V_UNDEFINED);
        if (IsNext(T_QUESTIONMARK)) {
            if (!st.IsNillable(dest) && dest->t != V_TYPEVAR)
                Error("value types can\'t be made nilable");
            else
                dest = st.Wrap(dest, V_NIL);
        }
        if (withtype && dest->t != V_UUDT && !IsUDT(dest->t))
            Error(":: must be used with a class type");
        return dest;
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

    void ParseExpStat(Block *list, bool allow_multi_assign = false) {
        switch (lex.token) {
            case T_FOR: {
                lex.Next();
                Line line = lex;
                Node *iter;
                if (IsNext(T_LEFTPAREN)) {
                    iter = ParseExp(false);
                    ExpectClosing(T_RIGHTPAREN);
                    list->Add(new For(line, iter, ParseBlock(0, true, allow_multi_assign)));
                } else {
                    iter = ParseExp(true);
                    list->Add(new For(line, iter, ParseBlock(0, false, allow_multi_assign)));
                }
                break;
            }
            case T_WHILE: {
                lex.Next();
                Line line = lex;
                // TODO: This could call ParseExpCond(list) which supports "while let", but that gives the impression the initializer will be
                // evaluated in a loop, and it will only be done once. So for now, disable. To support, we'd need to move while to be
                // Made out of two blocks, an outer which can hold all these defines, and a body.
                auto cond = ParseExp(true);
                auto w = new While(line, cond, ParseBlock(-1, false, allow_multi_assign));
                list->Add(w);
                break;
            }
            case T_RETURN: {
                lex.Next();
                Node *rv = nullptr;
                if (!Either(T_LINEFEED, T_DEDENT, T_FROM)) {
                    rv = ParseMultiRet(ParseOpExp());
                } else {
                    rv = new DefaultVal(lex);
                }
                auto sf = st.toplevel;
                if (IsNext(T_FROM)) {
                    if(!IsNext(T_PROGRAM)) {
                        if (!IsNextId()) {
                            Error(Q("return from"), " must be followed by function identifier or ",
                                  Q("program"));
                        } else if (auto f = st.FindFunction(lastid); !f) {
                            Error(Q(lastid), " is not a known function for use with ",
                                  Q("return from"));
                        } else if (f->sibf || f->overloads.size() > 1) {
                            Error("function ", Q(lastid),
                                  " must have single implementation to be used with ",
                                  Q("return from"));
                        } else {
                            sf = f->overloads[0]->sf;
                        }
                    }
                } else {
                    if (namedfunctionstack.size())
                        sf = namedfunctionstack.back()->overloads.back()->sf;
                }
                list->Add(new Return(lex, rv, sf, false));
                break;
            }
            case T_BREAK: {
                lex.Next();
                list->Add(new Break(lex));
                break;
            }
            case T_CONTINUE: {
                lex.Next();
                list->Add(new Continue(lex));
                break;
            }
            default: {
                auto e = ParseExp();
                // Multi-assign only in statement contexts, since e.g. in a one-liner
                // lambda body inside a call, a following comma separates arguments.
                // TODO: add Indexed to this.
                if (allow_multi_assign &&
                    (Is<IdentRef>(e) || Is<Dot>(e) || Is<GenericCall>(e)) &&
                    lex.token == T_COMMA) {
                    // A multi-assign statement, e.g. a, b = f()
                    // (regular assign is handled in normal expression parsing).
                    auto al = new AssignList(lex, Modify(e));
                    while (IsNext(T_COMMA)) {
                        e = ParseDeref();
                        if (!Is<IdentRef>(e) && !Is<Dot>(e) && !Is<GenericCall>(e))
                            Error("assignment list elements must be variables or class members");
                        al->children.push_back(Modify(e));
                    }
                    Expect(T_ASSIGN);
                    al->children.push_back(ParseMultiRet(ParseOpExp()));
                    list->Add(al);
                    break;
                }
                list->Add(e);
                break;
            }
        }
    }

    Node *ParseExpCond(Block *list) {
        if (lex.token == T_CONST || lex.token == T_VAR) {
            if (!list)
                return ErrorExp("declaration inside condition only allowed as statement");
            DS<bool> ds(call_noparens, true);
            Node *andexp = nullptr;
            for (;;) {
                auto def = ParseDefine(false, true, false);
                list->Add(def);
                // TODO: this reads the variable after it has been set. Unless codegen recognizes
                // this case, it is slower than simply having the define leave a value on the stack.
                auto idexp = new IdentRef(lex, def->tsids[0].sid);
                andexp = andexp ? (Node *)new And(lex, andexp, idexp) : (Node *)idexp;
                if (lex.token != T_AND) break;
                lex.Next();
                if (lex.token != T_CONST && lex.token != T_VAR) {
                    Error("expected ", Q("let"), " or ", Q("var"), " after ", Q("and"));
                    andexp = new And(lex, andexp, ParseExp(true));
                    break;
                }
            }
            return andexp;
        } else {
            return ParseExp(true);
        }
    }

    Node *Modify(Node *e) {
        if (auto idr = Is<IdentRef>(e)) {
            idr->sid->id->Assign(lex);
        }
        return e;
    }

    void CheckOpEq(Node *e) {
        if (!Is<IdentRef>(e) && !Is<Indexing>(e) && !Is<GenericCall>(e) && !Is<Dot>(e) && !Is<FreeVarRef>(e))
            Error("illegal left hand side of assignment");
        Modify(e);
        lex.Next();
    }

    template<typename T> Node *ParseAssign(unique_ptr<Node> &lhs) {
        CheckOpEq(lhs.get());
        auto rhs = ParseExp();
        return new T(lex, lhs.release(), rhs);
    }

    Node *ParseExp(bool parent_noparens = false) {
        DS<bool> ds(call_noparens, parent_noparens);
        unique_ptr<Node> e(ParseOpExp());
        switch (lex.token) {
            case T_ASSIGN:  return ParseAssign<Assign>(e);
            case T_PLUSEQ:  return ParseAssign<PlusEq>(e);
            case T_MINUSEQ: return ParseAssign<MinusEq>(e);
            case T_MULTEQ:  return ParseAssign<MultiplyEq>(e);
            case T_DIVEQ:   return ParseAssign<DivideEq>(e);
            case T_MODEQ:   return ParseAssign<ModEq>(e);
            case T_ANDEQ:   return ParseAssign<AndEq>(e);
            case T_OREQ:    return ParseAssign<OrEq>(e);
            case T_XOREQ:   return ParseAssign<XorEq>(e);
            case T_ASLEQ:   return ParseAssign<ShiftLeftEq>(e);
            case T_ASREQ:   return ParseAssign<ShiftRightEq>(e);
            default:        return e.release();
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
        unique_ptr<Node> exp(level ? ParseOpExp(level - 1) : ParseUnary());
        TType *o = &ops[level][0];
        while (Either(o[0], o[1]) || Either(o[2], o[3])) {
            TType op = lex.token;
            lex.Next();
            auto rhs = level ? ParseOpExp(level - 1) : ParseUnary();
            auto lhs = exp.release();
            switch (op) {
                case T_MULT:   exp.reset(new Multiply(lex, lhs, rhs)); break;
                case T_DIV:    exp.reset(new Divide(lex, lhs, rhs)); break;
                case T_MOD:    exp.reset(new Mod(lex, lhs, rhs)); break;
                case T_PLUS:   exp.reset(new Plus(lex, lhs, rhs)); break;
                case T_MINUS:  exp.reset(new Minus(lex, lhs, rhs)); break;
                case T_ASL:    exp.reset(new ShiftLeft(lex, lhs, rhs)); break;
                case T_ASR:    exp.reset(new ShiftRight(lex, lhs, rhs)); break;
                case T_BITAND: exp.reset(new BitAnd(lex, lhs, rhs)); break;
                case T_BITOR:  exp.reset(new BitOr(lex, lhs, rhs)); break;
                case T_XOR:    exp.reset(new Xor(lex, lhs, rhs)); break;
                case T_LT:     exp.reset(new LessThan(lex, lhs, rhs)); break;
                case T_GT:     exp.reset(new GreaterThan(lex, lhs, rhs)); break;
                case T_LTEQ:   exp.reset(new LessThanEq(lex, lhs, rhs)); break;
                case T_GTEQ:   exp.reset(new GreaterThanEq(lex, lhs, rhs)); break;
                case T_EQ:     exp.reset(new Equal(lex, lhs, rhs)); break;
                case T_NEQ:    exp.reset(new NotEqual(lex, lhs, rhs)); break;
                case T_AND:    exp.reset(new And(lex, lhs, rhs)); break;
                case T_OR:     exp.reset(new Or(lex, lhs, rhs)); break;
                default: assert(false);
            }
        }
        return exp.release();
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

    // The arguments of a call to `idname`, into a GenericCall for the declchecker and the
    // typechecker to decide what it calls. `f` is only what a function of that name is known
    // to be here, for an error about how a constructor is called.
    GenericCall *ParseFunctionCall(Line line, Function *f, string_view idname, Node *dotarg,
                                   bool noparenscall, vector<UnTypeRef> *specializers) {
        node_small_vector list;
        bool parens_parsed = false;
        [&]() {
            if (dotarg) {
                list.push_back(dotarg);
                if (!IsNext(T_LEFTPAREN)) return;
                parens_parsed = true;
            } else {
                if (!noparenscall) {
                    if (f && f->is_constructor_of && lex.token == T_LEFTCURLY)
                        Error(
                            "This type has an explicit constructor defined and must be called like "
                            "a function (use () instead of {})");
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
                if (needscomma && !ListSep(T_COMMA, T_RIGHTPAREN)) {
                    if (lex.token == T_RIGHTPAREN) continue;
                    break;
                }
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
                    case T_COLON:
                        e = ParseFunction(nullptr, false, false, false, false, nullptr);
                        break;
                    case T_IDENT: {
                        // An identifier after a function call tends to happen a lot accidentally,
                        // which then produces very confusing errors about shadowing.
                        // So, check the next token is a possible start of a function value first.
                        auto idn = ExpectId();
                        auto tok = lex.token;
                        lex.PushCur();
                        lex.Push(T_IDENT, idn);
                        lex.Next();
                        if (tok == T_COMMA || tok == T_COLON) {
                            // Plausable start of function value.
                            e = ParseFunction(nullptr, false, false, false, true, nullptr);
                        } else {
                            return;
                        }
                        break;
                    }
                    case T_LEFTPAREN:
                        e = ParseFunction(nullptr, false, false, true, true, nullptr);
                        break;
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
        auto call = new GenericCall(line, idname, st.current_namespace, dotarg != nullptr,
                                    !parens_parsed, false, specializers);
        call->children = list;
        if (!dotarg) {
            // A variable of this name is a candidate for what is called, see
            // GenericCall::TypeCheck, which has to know about it since the variable scopes
            // are gone by then.
            auto id = st.Lookup(idname);
            if (id) call->cand_var = id->cursid;
        }
        return call;
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
                    n = ParseFunctionCall(lex, f, idname, n, false, &specializers);
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
                ExpectClosing(T_RIGHTBRACKET);
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
                        // all subsequent Plus ops are string concats. A call like any other, which
                        // can only be the builtin since `string` is a keyword.
                        auto call = new GenericCall(lex, "string", st.current_namespace, false,
                                                    false, false, nullptr);
                        call->Add(e);
                        si = call;
                    }
                    if (!lex.sval.empty())
                        si = new Plus(lex, si, new StringConstant(lex, std::move(lex.sval)));
                    if (lex.token == T_STR_INT_MIDDLE) {
                        lex.Next();
                    } else {
                        ExpectClosing(T_STR_INT_END);
                        return si;
                    }
                }
            }
            case T_NIL: {
                lex.Next();
                auto n = new Nil(lex, { nullptr });
                if (IsNext(T_TYPEIN)) {
                    auto type = ParseType(false);
                    if (type->t != V_TYPEVAR && !st.IsNillable(type))
                        Error("cannot construct nillable type from ", Q(TypeName(type)));
                    else
                        n->giventype = st.Wrap(type, V_NIL);
                }
                return n;
            }
            case T_LEFTPAREN: {
                lex.Next();
                auto n = ParseExp();
                if (lex.token == T_COMMA) {
                    n = ParseMultiRet(n);
                }
                ExpectClosing(T_RIGHTPAREN);
                return n;
            }
            case T_LEFTBRACKET: {
                lex.Next();
                auto constructor = new VectorConstructor(lex);
                ParseVector([this, &constructor] () {
                    constructor->Add(this->ParseExp());
                }, T_RIGHTBRACKET);
                if (IsNext(T_TYPEIN)) {
                    constructor->giventype = ParseType(false);
                    constructor->giventype = st.Wrap(constructor->giventype, V_VECTOR);
                }
                return constructor;
            }
            case T_LEFTCURLY: {
                lex.Next();
                return ParseAutoConstructor();
            }
            case T_LAMBDA: {
                lex.Next();
                return ParseFunction(nullptr, false, false, lex.token == T_LEFTPAREN,
                                     lex.token != T_COLON, nullptr);
            }
            case T_FLOATTYPE:
            case T_INTTYPE:
            case T_STRTYPE:
            case T_ANYTYPE: {
                // These are also used as built-in functions, so allow them to function as
                // identifier for calls.
                auto idname = lex.sattr;
                lex.Next();
                if (lex.token != T_LEFTPAREN) return ErrorExp("type used as expression");
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
                auto call = ParseFunctionCall(lex, st.FindFunction(idname), idname, nullptr, false,
                                              nullptr);
                call->super = true;
                return call;
            }
            case T_PAKFILE: {
                lex.Next();
                string s = std::move(lex.sval);
                Expect(T_STR);
                pakfiles.insert(s);
                return new StringConstant(lex, std::move(s));
            }
            // These two also exist at statement level where they can also have a definition inside the condition.
            case T_ASSERT: {
                lex.Next();
                Line line = lex;
                auto cond = ParseExpCond(nullptr);
                return new Assert(line, cond);
            }
            case T_IF: {
                lex.Next();
                return ParseIf(lex, nullptr, false);
            }
            case T_SWITCH: {
                lex.Next();
                Line line = lex;
                auto value = ParseExp(true);
                Expect(T_COLON);
                bool have_default = false;
                bool have_out_of_range = false;
                auto cases = new List(lex);
                if (!IsNext(T_INDENT)) {
                    Expected(T_INDENT);
                    return new Switch(line, value, cases);
                }
                for (;;) {
                    auto errors_before = lex.num_errors;
                    List *pattern = new List(lex);
                    Line cline = lex;
                    bool out_of_range = false;
                    UDT *case_udt = nullptr;
                    bool withtype = false;
                    vector<string_view> patvars;
                    if (lex.token == T_DEFAULT || lex.token == T_OUT_OF_RANGE) {
                        out_of_range = lex.token == T_OUT_OF_RANGE;
                        if (out_of_range ? have_out_of_range : have_default)
                            Error("cannot have more than one ", Q(TName(lex.token)), " in a switch");
                        // They occupy the same slot in the generated code, and `default` already
                        // runs for out of range values, so only one of the two is allowed.
                        if (have_default || have_out_of_range)
                            Error("cannot have both ", Q(TName(T_DEFAULT)), " and ",
                                  Q(TName(T_OUT_OF_RANGE)), " in a switch");
                        lex.Next();
                        have_default = !out_of_range;
                        have_out_of_range = out_of_range;
                    } else {
                        Expect(T_CASE);
                        for (;;) {
                            Node *f = nullptr;
                            if (lex.token == T_IDENT) {
                                auto udt = st.LookupSpecialization(lex.sattr);
                                if (udt) {
                                    if (pattern->Arity()) {
                                        // We can lift this restriction in theory, by finding the common
                                        // supertype for these multiple types in Case::TypeCheck, but
                                        // then then exhaustiveness checking would get complicated if that
                                        // doesn't cover all subtype cases.. so then the user would be better
                                        // of using that supertype directly.
                                        Error("can currently match only one type per case");
                                    }
                                    lex.Next();
                                    f = new UDTRef(lex, udt);
                                    case_udt = udt;
                                    // `case T::` and `case T(names)`, see ParseCaseBlock.
                                    if (lex.token == T_TYPEIN) {
                                        withtype = true;
                                    } else if (IsNext(T_LEFTPAREN)) {
                                        ParseVector([&]() { patvars.push_back(ExpectId()); },
                                                    T_RIGHTPAREN);
                                    }
                                }
                            }
                            if (!f) {
                                f = ParseUnary();
                                if (lex.token == T_DOTDOT) {
                                    lex.Next();
                                    f = new Range(lex, f, ParseUnary());
                                }
                            }
                            pattern->Add(f);
                            // The :: is the block's, in place of its colon.
                            if (withtype) break;
                            if (lex.token == T_COLON || !ListSep(T_COMMA, T_COLON)) break;
                        }
                    }
                    auto cas = new Case(cline, pattern,
                                        ParseCaseBlock(value, case_udt, withtype, patvars));
                    cas->out_of_range = out_of_range;
                    cas->withtype = withtype;
                    cases->Add(cas);
                    EndOfLine(errors_before);
                    if (!IsNext(T_LINEFEED) || Either(T_ENDOFFILE, T_DEDENT)) break;
                }
                Expect(T_DEDENT);
                return new Switch(line, value, cases);
            }
            default:
                return ErrorExp("illegal start of expression: ", Q(lex.TokStr()));
        }
    }

    Node *ParseIf(Line line, Block *list, bool allow_multi_assign) {
        st.BlockScopeStart();  // Just in case condition has a define.
        auto cond = ParseExpCond(list);
        auto thenp = ParseBlock(-1, false, allow_multi_assign);
        st.BlockScopeCleanup();
        auto islf = IsNext(T_LINEFEED);
        if (IsNext(T_ELIF)) {
            // FIXME: we can't support a definition inside condition here because it would put it
            // before the entire nested if!
            return new IfElse(line, cond, thenp,
                              (new Block(lex))->Add(ParseIf(lex, nullptr, allow_multi_assign)));
        } else if (IsNext(T_ELSE)) {
            return new IfElse(line, cond, thenp, ParseBlock(-1, false, allow_multi_assign));
        } else {
            lex.PushCur();
            if (islf) lex.Push(T_LINEFEED);
            lex.Next();
            return new IfThen(line, cond, thenp);
        }
    }

    Node *ParseGuard(Line line, Block *list) {
        auto cond = ParseExpCond(list);
        auto block = new Block(lex);
        if (lex.token != T_COLON) {
            Expect(T_LINEFEED);
            ParseStatements(block, T_NONE);
            return new IfThen(line, cond, block);
        } else {
            auto exitblock = ParseBlock(-1, false, true);
            Expect(T_LINEFEED);
            ParseStatements(block, T_NONE);
            return new IfElse(line, cond, block, exitblock);
        }
    }

    void ForLoopVar(int existing, SpecIdent *sid, UnTypeRef type, node_small_vector &list) {
        Node *init = nullptr;
        if (existing == 0)
            init = new ForLoopElem(lex);
        else if (existing == 1)
            init = new ForLoopCounter(lex);
        else
            init = ErrorExp("for loop takes at most an element and index variable");
        auto def = new Define(lex, init);
        def->tsids.push_back({ sid , type });
        list.insert(existing, def);
    }

    // The body of a switch case. With `case T::` (withtype) all fields of the T the switch
    // value is in the body are in scope by their own name, with `case T(names)` those in
    // field order under `names` (as many as given, `_` for a field not needed), as
    // pattern variables. Either way an occurrence in the body reads the field of the switch
    // value (see WithStackElem), which must thus be a variable or field path: that is also
    // what makes the value be of type T in the body at all.
    Block *ParseCaseBlock(Node *value, UDT *udt, bool withtype,
                          const vector<string_view> &patvars) {
        st.BlockScopeStart();
        auto block = new Block(lex);
        if (withtype || !patvars.empty()) {
            Ident *id = nullptr;
            FieldPath derefs;
            if (!SwitchValuePath(value, id, derefs)) {
                Error("a case with ", Q("::"), " or pattern variables requires switching on a"
                      " variable or field");
            } else {
                auto sf = st.defsubfunctionstack.back();
                if (withtype) {
                    st.AddWithStruct(&udt->g, id, sf, derefs);
                } else {
                    auto &fields = udt->g.fields;
                    for (auto [i, name] : enumerate(patvars)) {
                        if (name == "_") continue;
                        // Such a name would be taken for an implicit argument.
                        if (name[0] == '_') {
                            Error("pattern variable ", Q(name), " cannot start with ", Q("_"));
                            continue;
                        }
                        if (i >= fields.size()) {
                            Error("type ", Q(udt->name), " has only ", fields.size(),
                                  " field(s), none for pattern variable ", Q(name));
                            break;
                        }
                        st.AddPatternVar(name, &udt->g, fields[i].id, id, sf, derefs);
                    }
                }
            }
        }
        if (withtype) {
            // A :: normally continues the line (a type follows), but here the body may be
            // an indented block.
            lex.OverrideCont(false);
            Expect(T_TYPEIN);
        } else {
            Expect(T_COLON);
        }
        ParseBody(block, -1, false);
        st.BlockScopeCleanup();
        return block;
    }

    // The variable and the fields from it a switch value names, if it is a variable or a
    // field path (see ParseCaseBlock). A field access is still a GenericCall at this point
    // (the typechecker makes it a Dot), so whether the name is a field of the object's type
    // is only known there.
    bool SwitchValuePath(Node *value, Ident *&id, FieldPath &derefs) {
        for (;;) {
            if (auto idr = Is<IdentRef>(value)) {
                id = idr->sid->id;
                return true;
            } else if (auto dot = Is<Dot>(value)) {
                derefs.insert(0, dot->fld);
                value = dot->child;
            } else if (auto gc = Is<GenericCall>(value);
                       gc && gc->fromdot && gc->noparens && gc->Arity() == 1) {
                auto fld = st.FieldUse(gc->name);
                if (!fld) return false;
                derefs.insert(0, fld);
                value = gc->children[0];
            } else {
                return false;
            }
        }
    }

    Block *ParseBlock(int for_args = -1, bool parse_args = false,
                      bool allow_multi_assign = false) {
        st.BlockScopeStart();
        auto block = new Block(lex);
        if (parse_args && lex.token != T_COLON) {
            auto parens = IsNext(T_LEFTPAREN);
            for (;;) {
                ExpectId();
                bool withtype = lex.token == T_TYPEIN;
                auto id = st.LookupDef(lastid, true, withtype);
                id->loop_var = true;
                UnTypeRef type = (UnType *)nullptr;
                if (parens && (lex.token == T_COLON || withtype)) {
                    lex.Next();
                    type = ParseType(withtype, nullptr);
                    if (withtype) {
                        st.AddWithStruct(GetGUDTAny(type), id, st.defsubfunctionstack.back());
                    }
                }
                id->cursid->withtype = withtype;
                ForLoopVar(for_args, id->cursid, type, block->children);
                for_args++;
                if (!IsNext(T_COMMA)) break;
            }
            if (parens) ExpectClosing(T_RIGHTPAREN);
        }
        Expect(T_COLON);
        ParseBody(block, for_args, allow_multi_assign);
        st.BlockScopeCleanup();
        return block;
    }

    void ParseVector(const function<void()> &f, TType closing) {
        if (IsNext(closing)) return;
        assert(lex.token != T_INDENT);  // Not generated inside brackets/braces.
        for (;;) {
            f();
            if (lex.token == closing || !ListSep(T_COMMA, closing) || lex.token == closing) break;
        }
        ExpectClosing(closing);
    }

    vector<UnTypeRef> ParseSpecializers(bool likely_named_function) {
        vector<UnTypeRef> specializers;
        // Check for function call with generic params.
        // This is not a great way to distinguish from < operator exps, but best we can do?
        if (likely_named_function && lex.whitespacebefore == 0 && IsNext(T_LT)) {
            ParseSpecializerList([&]() { specializers.push_back(ParseType(false)); });
        }
        return specializers;
    }

    Node *ParseAutoConstructor() {
        Line line = lex;
        auto constructor = new AutoConstructor(line);
        ParseVector(
            [&]() {
                SharedField *fld = nullptr;
                auto tag = lex.sattr;
                if (IsNext(T_IDENT)) {
                    if (IsNext(T_COLON)) {
                        fld = st.FieldUse(tag);
                        if (!fld) Error("unknown field ", Q(tag));
                    } else {
                        lex.Undo(T_IDENT, tag);
                    }
                }
                constructor->tags.push_back(fld);
                constructor->Add(ParseExp());
            },
            T_RIGHTCURLY);
        return constructor;
    }

    Node *IdentFactor(string_view idname) {
        // First see if this a type constructor.
        auto udt = st.LookupSpecialization(idname);
        auto gudt = udt ? &udt->g : st.LookupStruct(idname);
        auto curf = namedfunctionstack.empty() ? nullptr : namedfunctionstack.back();
        UnTypeRef type = (UnType *)nullptr;
        if (gudt &&
            lex.token == T_LT &&
            (!gudt->has_constructor_function || (curf && curf->is_constructor_of == gudt))) {
            lex.Undo(T_IDENT, idname);
            type = ParseType(false);
        } else if (lex.token == T_LEFTCURLY) {
            auto tv = FindTypeVar(idname);
            if (!tv.Null()) {
                type = { tv };
            } else {
                auto [gu, su] = st.StructOrSpecializationUse(idname);
                gudt = gu;
                udt = su;
                if (udt) {
                    type = { &udt->thistype };
                } else {
                    // FIXME: would like to enforce this in the future.
                    //Error(cat(gudt->name, " requires specializers"));
                    // This is a special type we allow no-where else: a spec UDT with no specializers.
                    type = { st.NewSpecUDT(gudt) };
                }
            }
        } else {
            gudt = nullptr;
            udt = nullptr;
        }
        if (gudt) {
            Expect(T_LEFTCURLY);
            if (gudt->is_abstract)
                Error("cannot instantiate abstract class/struct ", Q(gudt->name));
            // Field/tag resolution and default values are filled in by
            // AutoConstructor::TypeCheck, since the fields of this type may
            // not be complete yet (a pre-declared type, or a superclass that
            // is), and defaults cloned here would not track that.
            auto constructor = AssertIs<AutoConstructor>(ParseAutoConstructor());
            constructor->giventype = type;
            return constructor;
        }
        if (!type.Null()) {
            Expect(T_LEFTCURLY);
            if (type->t != V_TYPEVAR)
                Error("type ", Q(TypeName(type)), " does not have a {} constructor");
            // Which fields this has is only known once the type variable is
            // bound, so tags, defaults and the arg count are all resolved
            // by AutoConstructor::TypeCheck, same as for a named type.
            auto constructor = AssertIs<AutoConstructor>(ParseAutoConstructor());
            constructor->giventype = type;
            return constructor;
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
                ExpectClosing(T_RIGHTPAREN);
                return ec;
            }
            return ParseFunctionCall(lex, f, idname, nullptr, false, nullptr);
        }
        auto specializers = ParseSpecializers(f && !nf && !e);
        if (!specializers.empty())
            return ParseFunctionCall(lex, f, idname, nullptr, false, &specializers);
        // Check for implicit variable.
        if (idname[0] == '_') {
            if (block_stack.empty())
                return ErrorExp("cannot add implicit argument ", Q(idname), " at top level");
            auto &bs = block_stack.back();
            auto id = st.Lookup(idname);
            auto sf = st.defsubfunctionstack.back();
            if (!id || id->cursid->sf_def != sf) {
                if (bs.for_nargs >= 0) {
                    id = st.LookupDef(idname, true, false);
                    // One of an enclosing function (an error LookupDef reported) is used as is.
                    if (id->cursid->sf_def != sf) return new IdentRef(lex, id->cursid);
                    if (bs.for_nargs > 0) {
                        Error("cannot add implicit argument ", Q(idname), " to ", Q("for"),
                              " with existing arguments");
                    }
                    id->constant = true;
                    ForLoopVar(bs.implicits, id->cursid, type, bs.block->children);
                    bs.implicits++;
                } else {
                    id = st.LookupDef(idname, false, false);
                    if (id->cursid->sf_def != sf) return new IdentRef(lex, id->cursid);
                    if (st.defsubfunctionstack.size() <= 1)
                        Error("cannot add implicit argument ", Q(idname), " to top level");
                    if (!sf->parent->anonymous)
                        Error("cannot use implicit argument ", Q(idname),
                              " in named function ", Q(sf->parent->name));
                    if (sf->args[0].sid->id->name[0] != '_')
                        Error("cannot mix implicit argument ", Q(idname),
                              " with declared arguments in function");
                    GenImplicitGenericForLastArg(sf, sf->overload);
                }
                id->Read();
            }
            return new IdentRef(lex, id->cursid);
        }
        auto id = st.Lookup(idname);
        SymbolTable::WithStackElem *wse = nullptr;
        auto field = st.LookupWithStruct(idname, wse);
        // Check for function call without ().
        if (!id &&
            !field &&
            (nf || f) &&
            lex.whitespacebefore > 0 &&
            lex.token != T_LINEFEED) {
            return ParseFunctionCall(lex, f, idname, nullptr, true, nullptr);
        }
        // Check for enum value.
        auto ev = st.EnumValLookup(idname, false);
        if (ev) {
            auto ic = new IntConstant(lex, ev->val);
            ic->from = ev;
            return ic;
        }
        // Check for field reference in a :: scope, or a pattern variable.
        if (field) {
            assert(wse);
            if (OutsideFieldInit(wse->id->cursid->sf_def))
                Error("field ", Q(idname), " cannot be used in a ", Q("member"), " initializer:"
                      " it is evaluated wherever the class is constructed, where there is no"
                      " instance to read it from yet");
            wse->id->Read();
            return new Dot(field, lex, wse->Object(lex));
        }
        // Check any non-lexical-scope freevars.
        for (auto f : reverse(namedfunctionstack)) {
            auto ov = f->overloads.back();
            for (auto fvd : ov->freevardecls) {
                if (idname == fvd->name) {
                    return new FreeVarRef(lex, fvd);
                }
            }
        }
        // It's likely a regular variable.
        if (!id) {
            if (f || nf) return ErrorExp("can\'t use named function ", Q(idname), " as value");
            else return ErrorExp("unknown identifier ", Q(idname));
        }
        if (OutsideFieldInit(id->cursid->sf_def))
            Error("local variable ", Q(idname), " cannot be used in a ", Q("member"),
                  " initializer: it is evaluated wherever the class is constructed, not where"
                  " the declaration is written");
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

    // Where an identifier is expected but there is none: a name nothing else can have, such
    // that whatever gets declared with it clashes with nothing, and uses of it find nothing.
    size_t error_ids = 0;

    string_view ExpectId() {
        if (IsNextId()) return lastid;
        Expected(T_IDENT);
        lastid = st.StoreName(cat("<error ", ++error_ids, ">"));
        return lastid;
    }

    // No linefeed is generated for the last line of a file, so statements that
    // want one need to accept this instead.
    bool AtEndOfFile() {
        return Either(T_ENDOFFILE, T_ENDOFINCLUDE);
    }

    template<typename... Ts> bool Either(Ts... ts) {
        return ((lex.token == ts) || ...);
    }

    void Expected(TType t) {
        Error(Q(lex.TokStr(t)), " expected, found ", Q(lex.TokStr()));
    }

    void Expect(TType t) {
        if (!IsNext(t)) Expected(t);
    }

    string DumpAll(bool onlytypechecked = false) {
        string s;
        for (auto f : st.functiontable) {
            for (auto ov : f->overloads) {
                for (auto sf = ov->sf; sf; sf = sf->next) {
                    if (!onlytypechecked || sf->typechecked) {
                        s += "FUNCTION: " + f->name + "(";
                        for (auto &arg : sf->args) {
                            s += arg.sid->id->name + ":" + TypeName(arg.spec_type) + " ";
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
