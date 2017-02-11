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
    Lex lex;
    Node *root;
    SymbolTable &st;
    vector<Ident *> maybeundefined;
    vector<int> functionstack;
    vector<string> trailingkeywordedfunctionvaluestack;
    struct ForwardFunctionCall { string idname; size_t maxscopelevel; Node *n; };
    vector<ForwardFunctionCall> forwardfunctioncalls;
    vector<TType> expparents;

    Parser(const char *_src, SymbolTable &_st, char *_stringsource)
        : lex(_src, _st.filenames, _stringsource), root(nullptr), st(_st) {
        assert(parserpool == nullptr);
        parserpool = new SlabAlloc();
    }

    ~Parser() {
        delete root;
        delete parserpool;
        parserpool = nullptr;
    }

    void Error(string err, const Node *what = nullptr) {
        lex.Error(err, what ? &what->line : nullptr);
    }

    void Parse() {
        auto sf = st.ScopeStart();
        st.toplevel = sf;
        auto &f = st.CreateFunction("__top_level_expression", "");
        sf->SetParent(f, f.subf);
        f.anonymous = true;
        sf->body = ParseStatements();
        st.ScopeCleanup();
        root = new Node(lex, T_LIST,
                   new Node(lex, sf),
                   new Node(lex, T_LIST,
                       new Node(lex, T_CALL, new Node(lex, sf), nullptr),
                       nullptr));
        Expect(T_ENDOFFILE);
        assert(forwardfunctioncalls.empty());
    }

    void AddTail(Node **&tail, Node *a) {
        *tail = new Node(lex, T_LIST, a, nullptr);
        tail = &(*tail)->tail();
    }

    Node *ParseStatements() {
        Node *list = nullptr;
        Node **tail = &list;
        for (;;) {
            ParseTopExp(tail);
            if (lex.token == T_ENDOFINCLUDE) {
                st.EndOfInclude();
                lex.PopIncludeContinue();
            } else if (!IsNext(T_LINEFEED)) {
                break;
            }
            if (Either(T_ENDOFFILE, T_DEDENT)) break;
        }
        ResolveForwardFunctionCalls();
        for (auto n = list; n; n = n->tail()) {
            auto def = n->head();
            switch (def->type) {
                case T_STRUCTDEF: {
                    st.UnregisterStruct(def->struct_id()->st(), lex);
                    break;
                }
                case T_FUN: {
                    auto f = def->sf()->parent;
                    if (!f->anonymous) st.UnregisterFun(f);
                    break;
                }
                case T_DEF: {
                    // FIXME: this never makes anything const if def->right() is another T_DEF
                    auto id = def->left()->ident();
                    id->static_constant = id->single_assignment && def->right()->IsConstInit();
                    break;
                }
            }
        }
        return list;
    }

    void ParseVector(const function<void ()> &f, TType closing) {
        if (IsNext(closing)) return;
        bool indented = IsNext(T_INDENT);
        for (;;) {
            f();
            if (!IsNext(T_COMMA)) break;
        }
        if (indented) {
            IsNext(T_LINEFEED);
            Expect(T_DEDENT);
            Expect(T_LINEFEED);
        }
        Expect(closing);
    }

    Node *DefineWith(const string &idname, Node *e, bool isprivate, bool isdef, bool islogvar) {
        auto id = isdef ? st.LookupDef(idname, lex.errorline, lex, false, true)
                        : st.LookupUse(idname, lex);
        if (islogvar) st.MakeLogVar(id);
        if (isprivate) {
            if (!isdef) Error("assignment cannot be made private");
            id->isprivate = true;
        }
        auto idr = new Node(lex, id);
        return isdef ? new Node(lex, T_DEF, idr, e, nullptr) : new Node(lex, T_ASSIGNLIST, idr, e);
    }

    Node *RecMultiDef(const string &idname, bool isprivate, int nids, bool &isdef, bool &islogvar) {
        Node *e = nullptr;
        if (IsNextId()) {
            string id2 = lastid;
            nids++;
            if (Either(T_DEF, T_LOGASSIGN, T_ASSIGN)) {
                isdef = lex.token != T_ASSIGN;
                islogvar = lex.token == T_LOGASSIGN;
                lex.Next();
                int nrv;
                e = DefineWith(id2, ParseMultiRet(ParseOpExp(), nrv), isprivate, isdef, islogvar);
                if (nrv > 1 && nrv != nids)
                    Error("number of values doesn't match number of variables");
            } else if (IsNext(T_COMMA)) {
                e = RecMultiDef(id2, isprivate, nids, isdef, islogvar);
            } else {
                lex.Undo(T_IDENT, id2);
            }
        }
        if (e) {
            e = DefineWith(idname, e, isprivate, isdef, islogvar);
        } else {
            lex.Undo(T_COMMA);
            lex.Undo(T_IDENT, idname);
        }
        return e;
    }

    void ParseTopExp(Node **&tail, bool isprivate = false) {
        switch(lex.token) {
            case T_PRIVATE:
                if (st.scopelevels.size() != 1 || isprivate)
                    Error("private must be used at file scope");
                lex.Next();
                ParseTopExp(tail, true);
                break;
            case T_INCLUDE: {
                if (isprivate)
                    Error("include cannot be private");
                lex.Next();
                string fn = lex.sattr;
                Expect(T_STR);
                lex.Include((char *)fn.c_str());
                ParseTopExp(tail);
                break;
            }
            case T_VALUE:  AddTail(tail, ParseTypeDecl(true,  isprivate)); break;
            case T_STRUCT: AddTail(tail, ParseTypeDecl(false, isprivate)); break;
            case T_FUN: {
                lex.Next();
                AddTail(tail, ParseNamedFunctionDefinition(isprivate));
                break;
            }
            case T_ENUM: {
                lex.Next();
                bool incremental = IsNext(T_PLUS) || !IsNext(T_MULT);
                int cur = incremental ? 0 : 1;
                for (;;) {
                    ExpectId();
                    auto id = st.LookupDef(lastid, lex.errorline, lex, false, true);
                    id->constant = true;
                    if (isprivate) id->isprivate = true;
                    if (IsNext(T_ASSIGN)) {
                        cur = atoi(lex.sattr.c_str());
                        Expect(T_INT);
                    }
                    AddTail(tail,
                            new Node(lex, T_DEF, new Node(lex, id), new Node(lex, cur), nullptr));
                    if (lex.token != T_COMMA) break;
                    lex.Next();
                    if (incremental) cur++; else cur *= 2;
                }
                break;
            }
            case T_VAR:
            case T_CONST: {
                auto isconst = lex.token == T_CONST;
                lex.Next();
                ExpectId();
                Expect(T_ASSIGN);
                auto d = ParseSingleVarDecl(isprivate, isconst, false, false);
                AddTail(tail, d);
                break;
                break;
            }
            default: {
                if (IsNextId()) {
                    auto d = ParseVarDecl(isprivate);
                    if (d) {
                        AddTail(tail, d);
                        break;
                    }
                }
                if (isprivate)
                    Error("private only applies to declarations");
                AddTail(tail, ParseExpStat());
                break;
            }
        }
    }

    Node *ParseTypeDecl(bool isvalue, bool isprivate) {
        lex.Next();
        auto sname = ExpectId();
        Struct *struc = &st.StructDecl(lastid, lex);
        Struct *sup = nullptr;
        auto parse_sup = [&] () {
            ExpectId();
            sup = &st.StructUse(lastid, lex);
        };
        vector<pair<TypeRef, Node *>> spectypes;
        auto parse_specializers = [&] () {
            if (IsNext(T_LEFTPAREN)) {
                for (;;) {
                    spectypes.push_back(make_pair(nullptr, nullptr));
                    ParseType(spectypes.back().first, false);
                    spectypes.back().second = IsNext(T_ASSIGN) ? ParseExp(T_STRUCTDEF) : nullptr;
                    if (IsNext(T_RIGHTPAREN)) break;
                    Expect(T_COMMA);
                }
            }
        };
        size_t i = 0;
        auto specialize_field = [&] (Field &field) {
            if (field.flags == AF_ANYTYPE) {
                if (i >= spectypes.size()) Error("too many type specializers");
                auto &p = spectypes[i++];
                field.type = p.first;
                if (p.second) field.defaultval = p.second;
            }
        };
        if (IsNext(T_ASSIGN)) {
            // A specialization of an existing struct
            parse_sup();
            struc = sup->CloneInto(struc);
            struc->idx = (int)st.structtable.size() - 1;
            struc->name = sname;
            struc->generic = false;
            parse_specializers();
            if (!spectypes.size())
                Error("no specialization types specified");
            if (!sup->generic)
                Error("you can only specialize a generic struct/value");
            if (isvalue != sup->readonly)
                Error("specialization must use same struct/value keyword");
            if (isprivate != sup->isprivate)
                Error("specialization must have same privacy level");
            for (auto &field : struc->fields.v) {
                // We don't reset AF_ANYTYPE here, because its used to know which fields to select
                // a specialization on.
                specialize_field(field);
                struc->Resolve(field);
            }
            if (struc->superclass) {
                // This points to a generic version of the superclass of this class.
                // See if we can find a matching specialization instead.
                auto sti = struc->superclass->next;
                for (; sti; sti = sti->next) {
                    for (size_t i = 0; i < sti->fields.size(); i++)
                        if (sti->fields.v[i].type != struc->fields.v[i].type)
                            goto fail;
                    goto done;
                    fail:;
                }
                done:
                struc->superclass = sti;  // Either a match or nullptr.
            }
        } else if (lex.token == T_COLON || lex.token == T_LEFTCURLY) {
            // A regular struct declaration
            struc->readonly = isvalue;
            struc->isprivate = isprivate;
            if (IsNext(T_COLON)) {
                parse_sup();
                parse_specializers();
            }
            int fieldid = 0;
            if (sup) {
                struc->superclass = sup;
                for (auto &fld : sup->fields.v) {
                    struc->fields.v.push_back(fld);
                    auto &field = struc->fields.v.back();
                    // FIXME: must check if this type is a subtype if old type isn't V_ANY
                    if (spectypes.size()) specialize_field(field);
                    if (st.IsGeneric(field.type)) struc->generic = true;
                }
                fieldid = (int)sup->fields.size();
            }
            Expect(T_LEFTCURLY);
            ParseVector([this, &fieldid, &struc] () {
                ExpectId();
                auto &sfield = st.FieldDecl(lastid);
                TypeRef type;
                int fieldref = -1;
                if (IsNext(T_COLON)) {
                    fieldref = ParseType(type, false, struc);
                }
                bool generic = st.IsGeneric(type) && fieldref < 0;
                Node *defaultval = IsNext(T_ASSIGN) ? ParseExp(T_STRUCTDEF) : nullptr;
                struc->fields.v.push_back(Field(&sfield, type, generic, fieldref, defaultval));
                if (generic) struc->generic = true;
            }, T_RIGHTCURLY);
            // Loop thru a second time, because this type may have become generic just now, and may
            // refer to itself.
            for (auto &field : struc->fields.v) {
                if (st.IsGeneric(field.type) && field.fieldref < 0) field.flags = AF_ANYTYPE;
            }
        } else {
            // A pre-declaration.
            struc->predeclaration = true;
        }
        return new Node(lex, T_STRUCTDEF, new Node(lex, struc));
    }

    Node *ParseSingleVarDecl(bool isprivate, bool constant, bool dynscope, bool logvar) {
        auto idname = lastid;
        auto e = ParseExp(T_DEF);
        auto id = dynscope
            ? st.LookupDynScopeRedef(idname, lex)
            : st.LookupDef(idname, lex.errorline, lex, false, true);
        if (dynscope)  id->Assign(lex);
        if (constant)  id->constant = true;
        if (isprivate) id->isprivate = true;
        if (logvar)    st.MakeLogVar(id);
        return new Node(lex, T_DEF, new Node(lex, id), e, nullptr);
    }

    Node *ParseVarDecl(bool isprivate) {
        bool dynscope = lex.token == T_DYNASSIGN;
        bool constant = lex.token == T_DEFCONST;
        bool logvar = lex.token == T_LOGASSIGN;
        // codegen assumes these defs can only happen at toplevel
        if (lex.token == T_DEF || dynscope || constant || logvar) {
            lex.Next();
            return ParseSingleVarDecl(isprivate, constant, dynscope, logvar);
        }
        auto idname = lastid;
        bool withtype = lex.token == T_TYPEIN;
        if (lex.token == T_COLON || withtype) {
            lex.Next();
            auto tn = new Node(lex, T_TYPE, TypeRef());
            ParseType(tn->typenode(), withtype);
            Expect(T_ASSIGN);
            auto e = ParseExp(T_DEF);
            auto id = st.LookupDef(idname, lex.errorline, lex, false, true);
            if (isprivate) id->isprivate = true;
            return new Node(lex, T_DEF, new Node(lex, id), e, tn);
        }
        if (IsNext(T_COMMA)) {
            bool isdef = false;
            bool islogvar = false;
            auto e = RecMultiDef(idname, isprivate, 1, isdef, islogvar);
            if (e) return e;
        } else {
            lex.Undo(T_IDENT, idname);
        }
        return nullptr;
    }

    static size_t CountList(Node *n) {
        size_t c = 0;
        for (; n; n = n->tail()) c++;
        return c;
    }

    static Node *LastInList(Node *n) {
        Node *last = nullptr;
        for (auto l = n; l; l = l->tail()) last = l;
        assert(last);
        return last;
    }

    Node *ParseNamedFunctionDefinition(bool isprivate = false) {
        string idname = ExpectId();
        if (natreg.FindNative(idname))
            Error("cannot override built-in function: " + idname);
        return ParseFunction(&idname, isprivate, true, true, "");
    }

    Node *ParseFunction(const string *name,
                        bool isprivate, bool parens, bool parseargs,
                        const string &context,
                        bool expfunval = false, TType expfunvalparent = T_FUN) {
        auto sf = st.ScopeStart();
        if (parens) Expect(T_LEFTPAREN);
        int nargs = 0;
        if (lex.token != T_RIGHTPAREN && parseargs) {
            for (;;) {
                ExpectId();
                nargs++;
                auto id = st.LookupDef(lastid, lex.errorline, lex, false, false);
                TypeRef type;
                bool withtype = lex.token == T_TYPEIN;
                if (parens && (lex.token == T_COLON || withtype)) {
                    lex.Next();
                    ParseType(type, withtype);
                    if (withtype) st.AddWithStruct(type, id, lex);
                }
                sf->args.v.back().SetType(type, st.IsGeneric(type));
                if (!IsNext(T_COMMA)) break;
            }
        }
        if (parens) Expect(T_RIGHTPAREN);
        auto &f = name ? st.FunctionDecl(*name, nargs, lex) : st.CreateFunction("", context);
        sf->SetParent(f, f.subf);
        if (name && IsNext(T_DEFCONST)) {
            if (f.istype || f.subf->next)
                Error("redefinition of function type: " + *name);
            f.istype = true;
            sf->typechecked = true;
            // Any untyped args truely mean "any", they should not be specialized (we wouldn't know
            // which specialization that refers to).
            for (auto &arg : f.subf->args.v) {
                if (arg.flags == AF_ANYTYPE) arg.flags = AF_NONE;
            }
            ParseType(sf->returntypes[0], false, nullptr, true);
        } else {
            if (IsNext(T_CODOT)) {  // Return type decl.
                sf->fixedreturntype = true;
                ParseType(sf->returntypes[0], false, nullptr, true);
            }
            if (!expfunval) Expect(T_COLON);
        }
        if (name) {
            if (f.subf->next) {
                f.multimethod = true;
                if (isprivate != f.isprivate)
                    Error("inconsistent private annotation of multiple function implementations"
                          " for " + *name);
            }
            f.isprivate = isprivate;
            functionstack.push_back(f.idx);
        } else {
            f.anonymous = true;
        }
        // Parse the body.
        if (!f.istype) {
            if (expfunval) {
                sf->body = new Node(lex, T_LIST, ParseExp(expfunvalparent), nullptr);
            } else if (IsNext(T_INDENT)) {
                sf->body = ParseStatements();
                Expect(T_DEDENT);
            } else {
                sf->body = new Node(lex, T_LIST, ParseExpStat(), nullptr);
            }
        }
        for (auto &arg : sf->args.v) {
            if (arg.id->anonymous_arg) {
                if (name) Error("cannot use anonymous argument: " + arg.id->name +
                    " in named function: " + f.name, sf->body);
                if (nargs) Error("cannot mix anonymous argument: " + arg.id->name +
                    " with declared arguments in function", sf->body);
            }
        }
        st.ScopeCleanup();
        if (name) {
            functionstack.pop_back();

            if (!f.istype) {
                for (auto stat = sf->body; stat; stat = stat->tail())
                    if (!stat->tail() && (stat->head()->type != T_RETURN ||
                        stat->head()->return_function_idx()->integer() != f.idx /* return from */))
                        ReturnValues(f, 1);
                assert(f.retvals);
            }
        }
        // Keep copy or arg types from before specialization.
        f.orig_args = sf->args;  // not used for multimethods
        return new Node(lex, sf);
    }

    static Type *NewType() {
        // FIXME: this potentially generates quite a bit of "garbage".
        // Instead, we could hash these, or store common type variants inside Struct etc.
        // A quick test revealed that 10% of nodes cause one of these to be allocated, so probably
        // not worth optimizing.
        return parserpool->alloc_obj_small<Type>();
    }

    int ParseType(TypeRef &dest, bool withtype, Struct *fieldrefstruct = nullptr,
                  bool allowany = false) {
        switch(lex.token) {
            case T_INTTYPE:   dest = type_int;        lex.Next(); break;
            case T_FLOATTYPE: dest = type_float;      lex.Next(); break;
            case T_STRTYPE:   dest = type_string;     lex.Next(); break;
            case T_COROUTINE: dest = type_coroutine;  lex.Next(); break;
            case T_RESOURCE:  dest = type_resource;   lex.Next(); break;
            case T_VECTTYPE:  dest = type_vector_any; lex.Next(); break;  // FIXME: remove this one?
            case T_FUN: {
                lex.Next();
                auto idname = lex.sattr;
                if (IsNext(T_IDENT)) {
                    auto f = st.FindFunction(idname);
                    if (!f)
                        Error("unknown function type: " + idname);
                    if (!f->istype)
                        Error("function not declared as type: " + idname);
                    dest = &f->subf->thistype;
                } else {
                    dest = type_function_null;  // FIXME: in what cases does this make sense?
                }
                break;
            }
            case T_IDENT: {
                if (fieldrefstruct) {
                    for (auto &field : fieldrefstruct->fields.v) {
                        if (field.id->name == lex.sattr) {
                            if (field.flags != AF_ANYTYPE)
                                Error("field reference must be to generic field: " + lex.sattr);
                            lex.Next();
                            dest = field.type;
                            return int(&field - &fieldrefstruct->fields.v[0]);
                        }
                    }
                }
                auto &struc = st.StructUse(lex.sattr, lex);
                dest = &struc.thistype;
                lex.Next();
                break;
            }
            case T_LEFTBRACKET: {
                lex.Next();
                TypeRef elem;
                ParseType(elem, false);
                Expect(T_RIGHTBRACKET);
                dest = elem->Wrap(NewType());
                break;
            }
            case T_LEFTPAREN:
                if (allowany)  {
                    lex.Next();
                    Expect(T_RIGHTPAREN);
                    dest = type_any;
                    break;
                }
                // FALL-THRU:
            default:
                Error("illegal type syntax: " + lex.TokStr());
        }
        if (IsNext(T_QUESTIONMARK)) {
            if (dest->Numeric()) Error("numeric types can\'t be made nilable");
            dest = dest->Wrap(NewType(), V_NIL);
        }
        if (withtype && dest->t != V_STRUCT) Error(":: must be used with a struct type");
        return -1;
    }

    Node *ParseFunArgs(bool coroutine, Node *derefarg, const char *fname = "",
                       GenericArgs *args = nullptr, bool noparens = false) {
        if (derefarg) {
            CheckArg(args, 0, fname);
            if (IsNext(T_LEFTPAREN)) {
                return new Node(lex, T_LIST, derefarg,
                                ParseFunArgsRec(false, false, args, 1, fname, noparens));
            } else {
                return new Node(lex, T_LIST, derefarg, nullptr);
            }
        } else {
            if (!noparens) Expect(T_LEFTPAREN);
            return ParseFunArgsRec(coroutine, false, args, 0, fname, noparens);
        }
    }

    Node *ParseFunArgsRec(bool coroutine, bool needscomma, GenericArgs *args, size_t thisarg,
                          const char *fname, bool noparens) {
        if (!noparens && IsNext(T_RIGHTPAREN)) {
            if (expparents.size() && expparents.back() == T_CALL_NOPARENS) {
                // Don't unnecessarily parse funvals. Means "if f(x):" parses as expected.
                return nullptr;
            }
            return ParseTrailingFunctionValues(coroutine, args, thisarg, fname);
        }
        if (needscomma) Expect(T_COMMA);
        Node *arg = nullptr;
        CheckArg(args, thisarg, fname);
        auto parent = noparens ? T_CALL_NOPARENS : T_CALL;
        if (args && args->GetType(thisarg)->flags == NF_EXPFUNVAL) {
            arg = ParseFunction(nullptr, false, false, false, args->GetName(thisarg), true, parent);
        } else {
            arg = ParseExp(parent);
        }
        if (noparens)
            return new Node(lex, T_LIST, arg, lex.token == T_COLON
                ? ParseTrailingFunctionValues(coroutine, args, thisarg + 1, fname)
                : nullptr);
        else
            return new Node(lex, T_LIST, arg, ParseFunArgsRec(coroutine, !noparens, args,
                                                              thisarg + 1, fname, noparens));
    }

    void CheckArg(GenericArgs *args, size_t thisarg, const char *fname) {
        if (args && thisarg == args->size())
            Error("too many arguments passed to function " + string(fname));
    }

    Node *ParseTrailingFunctionValues(bool coroutine, GenericArgs *args, size_t thisarg,
                                      const char *fname) {
        if (args && thisarg + 1 < args->size())
            trailingkeywordedfunctionvaluestack.push_back(args->GetName(thisarg + 1));
        auto name = args && thisarg < args->size() ? args->GetName(thisarg) : "";
        Node *e = nullptr;
        switch (lex.token) {
            case T_COLON:
                e = ParseFunction(nullptr, false, false, false, name);
                break;
            case T_IDENT:
                // skip if this function value starts with an ID that's equal to the parents next
                // keyworded function val ID, e.g. "else" in: if(..): currentcall(..) else: ..
                if (trailingkeywordedfunctionvaluestack.empty() ||
                    trailingkeywordedfunctionvaluestack.back() != lex.sattr)
                    e = ParseFunction(nullptr, false, false, true, name);
                break;
            case T_LEFTPAREN:
                e = ParseFunction(nullptr, false, true, true, name);
                break;
        }
        if (args && thisarg + 1 < args->size()) trailingkeywordedfunctionvaluestack.pop_back();
        if (!e) {
            if (coroutine) {
                e = new Node(lex, T_COCLOSURE);
                coroutine = false;
            } else {
                return nullptr;
            }
        }
        CheckArg(args, thisarg, fname);
        thisarg++;
        Node *tail = nullptr;
        bool islf = lex.token == T_LINEFEED;
        if (args && thisarg < args->size() && (lex.token == T_IDENT || islf)) {
            if (islf) lex.Next();
            if (lex.token == T_IDENT && args->GetName(thisarg) == lex.sattr) {
                lex.Next();
                tail = ParseTrailingFunctionValues(coroutine, args, thisarg, fname);
            } else {
                lex.PushCur();
                if (islf) lex.Push(T_LINEFEED);
                lex.Next();
            }
        }
        return new Node(lex, T_LIST, e, tail);
    }

    Node *ParseMultiRet(Node *first, int &nrv) {
        nrv = 1;
        if (lex.token != T_COMMA) return first;
        auto list = new Node(lex, T_MULTIRET, first, nullptr);
        auto tail = &list->tailexps();
        while (IsNext(T_COMMA)) {
            *tail = new Node(lex, T_MULTIRET, ParseOpExp(), nullptr);
            tail = &(*tail)->tailexps();
            nrv++;
        }
        return list;
    }

    void ReturnValues(Function &f, int nrv) {
        if (f.retvals && f.retvals != nrv)
            Error(string("all return statements of this function must return the same number of"
                         " return values. previously: ") + to_string(f.retvals));
        f.retvals = nrv;
    }

    Node *ParseExpStat() {
        if (IsNext(T_RETURN)) {
            Node *rv = nullptr;
            int nrv = 0;
            if (!Either(T_LINEFEED, T_DEDENT, T_FROM)) {
                rv = ParseMultiRet(ParseOpExp(), nrv);
                if (rv->type == T_CALL)
                    nrv = max(nrv, rv->call_function()->sf()->parent->retvals);
            }
            int fid = -2;
            if (IsNext(T_FROM)) {
                if(!IsNext(T_PROGRAM)) {
                    if (!IsNextId())
                        Error("return from: must be followed by function identifier or"
                              " \"program\"");
                    auto f = st.FindFunction(lastid);
                    if (!f)
                        Error("return from: not a known function");
                    if (f->sibf || f->multimethod)
                        Error("return from: function must have single implementation");
                    fid = f->idx;
                }
            } else {
                if (functionstack.size())
                    fid = functionstack.back();
            }
            if (fid >= 0)
                ReturnValues(*st.functiontable[fid], nrv);
            else if (nrv > 1)
                Error("cannot return multiple values from top level");
            return new Node(lex, T_RETURN, rv, new Node(lex, fid));
        }
        auto e = ParseExp(T_LIST);
        while (IsNext(T_SEMICOLON)) {
            if (IsNext(T_LINEFEED)) {
                // specialized error for all the C-style language users
                Error("\';\' is not a statement terminator");
            }
            e = new Node(lex, T_SEQ, e, ParseExp(T_LIST));
        }
        return e;
    }

    void Modify(Node *e) {
        if (e->type == T_IDENT)
            e->ident()->Assign(lex);
    }

    Node *ParseExp(TType parent) {
        expparents.push_back(parent);
        auto e = ParseOpExp();
        switch (lex.token) {
            case T_ASSIGN:
            case T_PLUSEQ:
            case T_MINUSEQ:
            case T_MULTEQ:
            case T_DIVEQ:
            case T_MODEQ: {
                auto type = lex.token;
                if (e->type != T_IDENT && e->type != T_DOT &&
                    e->type != T_CODOT && e->type != T_INDEX)
                    Error("illegal left hand side of assignment");
                Modify(e);
                lex.Next();
                e = new Node(lex, type, e, ParseExp(type));
            }
        }
        expparents.pop_back();
        return e;
    }

    Node *ParseOpExp(uint level = 6) {
        static TType ops[][4] = {
            { T_MULT, T_DIV, T_MOD, T_NONE },
            { T_PLUS, T_MINUS, T_NONE, T_NONE },
            { T_ASL, T_ASR, T_NONE, T_NONE },
            { T_BINAND, T_BINOR, T_XOR, T_NONE },
            { T_LT, T_GT, T_LTEQ, T_GTEQ },
            { T_EQ, T_NEQ, T_NONE, T_NONE },
            { T_AND, T_OR, T_NONE, T_NONE },
        };
        Node *exp = level ? ParseOpExp(level - 1) : ParseUnary();
        TType *o = &ops[level][0];
        while (Either(o[0], o[1]) || Either(o[2], o[3])) {
            TType op = lex.token;
            lex.Next();
            exp = new Node(lex, op, exp, level ? ParseOpExp(level - 1) : ParseUnary());
        }
        return exp;
    }

    Node *ParseUnary() {
        switch (lex.token) {
            case T_MINUS:
            case T_NOT:
            case T_NEG:
            case T_INCR:
            case T_DECR: {
                auto t = lex.token;
                lex.Next();
                auto e = ParseUnary();
                if (t == T_INCR || t == T_DECR) Modify(e);
                if (t == T_MINUS) t = T_UMINUS;
                return new Node(lex, t, e);
            }
            default:
                return ParseDeref();
        }
    }

    Node *BuiltinControlClosure(Node *funval, int maxargs) {
        auto clnargs = 0;
        if (funval->type == T_FUN)
            clnargs = funval->sf()->parent->nargs();
        else if (funval->type != T_DEFAULTVAL)
            Error("illegal body", funval);
        if (clnargs > maxargs)
            Error("body has " + to_string(clnargs - maxargs) + " parameters too many", funval);
        if (funval->type == T_DEFAULTVAL) return funval;
        return new Node(lex, T_CALL, funval, clnargs > 0
            ? new Node(lex, T_LIST,
                new Node(lex, T_FORLOOPELEM),
                clnargs > 1 ? new Node(lex, T_LIST, new Node(lex, T_FORLOOPI), nullptr) : nullptr)
            : nullptr);
    }

    Node *ParseFunctionCall(Function *f, NativeFun *nf, const string &idname, Node *firstarg,
                            bool coroutine, bool noparens) {
        if (nf) {
            auto args = ParseFunArgs(coroutine, firstarg, idname.c_str(), &nf->args, noparens);
            Node **ai = &args;
            for (auto &arg : nf->args.v) {
                if (!*ai) {
                    auto &type = arg.type;
                    if (type->t == V_NIL) {
                        *ai = new Node(lex, T_LIST, new Node(lex, T_DEFAULTVAL), nullptr);
                    } else {
                        auto nargs = CountList(args);
                        for (auto ol = nf->overloads; ol; ol = ol->overloads) {
                            // Typechecker will deal with it.
                            if (ol->args.v.size() == nargs) goto argsok;
                        }
                        Error("missing arg to builtin function: " + idname);
                    }
                }
                ai = &(*ai)->tail();
            }
            argsok:
            // Special formats for these functions, for better type checking and performance
            // TODO: worth deleting the garbage list nodes this creates?
            if (nf->name == "if") {
                return new Node(lex, T_IF, args->head(),
                                           BuiltinControlClosure(args->tail()->head(), 0),
                                           BuiltinControlClosure(args->tail()->tail()->head(), 0));
            } else if (nf->name == "while") {
                return new Node(lex, T_WHILE, BuiltinControlClosure(args->head(), 0),
                                              BuiltinControlClosure(args->tail()->head(), 0));
            } else if (nf->name == "for") {
                return new Node(lex, T_FOR, args->head(),
                                BuiltinControlClosure(args->tail()->head(), 2));
            }
            return new Node(lex, T_NATCALL, new Node(lex, nf), args);
        }
        if (f) {
            if (f->istype) Error("can\'t call function type: " + f->name);
            auto bestf = f;
            for (auto fi = f->sibf; fi; fi = fi->sibf)
                if (fi->nargs() > bestf->nargs()) bestf = fi;
            auto args =
                ParseFunArgs(coroutine, firstarg, idname.c_str(), &bestf->subf->args, noparens);
            auto nargs = CountList(args);
            f = FindFunctionWithNargs(f, nargs, idname, nullptr);
            return new Node(lex, T_CALL, new Node(lex, f->subf), args);
        }
        auto args = ParseFunArgs(coroutine, firstarg);
        auto id = st.Lookup(idname);
        if (id)
            return new Node(lex, T_DYNCALL, new Node(lex, id),
                                            new Node(lex, (SubFunction *)nullptr),
                                            args);
        auto n = new Node(lex, T_CALL, new Node(lex, (SubFunction *)nullptr), args);
        ForwardFunctionCall ffc = { idname, st.scopelevels.size(), n };
        forwardfunctioncalls.push_back(ffc);
        return n;
    }

    Function *FindFunctionWithNargs(Function *f, int nargs, const string &idname, Node *errnode) {
        for (; f; f = f->sibf)
            if (f->nargs() == nargs)
                return f;
        Error("no version of function " + idname + " takes " + to_string(nargs) + " arguments",
              errnode);
        return nullptr;
    }

    void ResolveForwardFunctionCalls() {
        for (auto ffc = forwardfunctioncalls.begin(); ffc != forwardfunctioncalls.end(); ) {
            if (ffc->maxscopelevel >= st.scopelevels.size()) {
                auto f = st.FindFunction(ffc->idname);
                if (f) {
                    ffc->n->call_function()->sf() = FindFunctionWithNargs(f,
                        CountList(ffc->n->call_args()), ffc->idname, ffc->n)->subf;
                    ffc = forwardfunctioncalls.erase(ffc);
                    continue;
                } else {
                    if (st.scopelevels.size() == 1)
                        Error("call to unknown function: " + ffc->idname, ffc->n);
                    // Prevent it being found in sibling scopes.
                    ffc->maxscopelevel = st.scopelevels.size() - 1;
                }
            }
            ffc++;
        }
    }

    Node *ParseDeref() {
        auto n = ParseFactor();
        for (;;) switch (lex.token) {
            case T_DOT:
            case T_DOTMAYBE:
            case T_CODOT: {
                auto op = lex.token;
                lex.Next();
                auto idname = ExpectId();
                if (op == T_CODOT) {
                    // Here we just look up ANY var with this name, only in the typechecker can we
                    // know if it exists inside the coroutine. Can cause error if used before
                    // coroutine is defined, error hopefully hints at that.
                    auto id = st.LookupAny(idname);
                    if (!id)
                        Error("coroutines have no variable named: " + idname);
                    n = new Node(lex, T_CODOT, n, new Node(lex, id));
                } else {
                    SharedField *fld = st.FieldUse(idname);
                    if (fld)  {
                        n = new Node(lex, op, n, new Node(lex, fld));
                    } else {
                        auto f = st.FindFunction(idname);
                        auto nf = natreg.FindNative(idname);
                        if ((f || nf) && op == T_DOT) {
                            n = ParseFunctionCall(f, nf, idname, n, false, false);
                        } else {
                            Error("not a type member or function: " + idname);
                        }
                    }
                }
                break;
            }
            case T_LEFTBRACKET: {
                lex.Next();
                n = new Node(lex, T_INDEX, n, ParseExp(T_INDEX));
                Expect(T_RIGHTBRACKET);
                break;
            }
            case T_INCR:
            case T_DECR:
                Modify(n);
                n = new Node(lex, lex.token == T_INCR ? T_POSTINCR : T_POSTDECR, n);
                lex.Next();
                return n;
            case T_LEFTPAREN: {  // only for dyn calls
                auto args = ParseFunArgs(false, nullptr);
                n = new Node(lex, T_DYNCALL, n, new Node(lex, (SubFunction *)nullptr), args);
                break;
            }
            case T_IS: {
                lex.Next();
                auto tn = new Node(lex, T_TYPE, TypeRef());
                ParseType(tn->typenode(), false);
                return new Node(lex, T_IS, n, tn);
            }
            default:
                return n;
        }
    }

    Node *ParseFactor() {
        switch (lex.token) {
            case T_INT: {
                int i = atoi(lex.sattr.c_str());
                lex.Next();
                return new Node(lex, i);
            }
            case T_FLOAT: {
                double f = atof(lex.sattr.c_str());
                lex.Next();
                return new Node(lex, f);
            }
            case T_STR: {
                string s = lex.sattr;
                lex.Next();
                return new Node(lex, s);
            }
            case T_NIL: {
                lex.Next();
                auto n = new Node(lex, T_NIL, TypeRef());
                if (IsNext(T_COLON)) {
                    ParseType(n->typenode(), false);
                    n->typenode() = n->typenode()->Wrap(NewType(), V_NIL);
                }
                return n;
            }
            case T_LEFTPAREN: {
                lex.Next();
                auto n = ParseExp(T_LEFTPAREN);
                Expect(T_RIGHTPAREN);
                return n;
            }
            case T_LEFTBRACKET: {
                lex.Next();
                Node *list = nullptr;
                Node **tail = &list;
                ParseVector([this, &tail] () {
                    AddTail(tail, this->ParseExp(T_CONSTRUCTOR));
                }, T_RIGHTBRACKET);
                auto tn = new Node(lex, T_TYPE, TypeRef());
                tn->typenode() = type_vector_any;
                if (IsNext(T_TYPEIN)) {
                    ParseType(tn->typenode(), false);
                    tn->typenode() = tn->typenode()->Wrap(NewType());
                }
                return new Node(lex, T_CONSTRUCTOR, list, tn);
            }
            case T_FUN: {
                lex.Next();
                return ParseFunction(nullptr, false, true, true, "");
            }
            case T_COROUTINE: {
                lex.Next();
                string idname = ExpectId();
                return new Node(lex, T_COROUTINE, ParseFunctionCall(st.FindFunction(idname),
                                                      nullptr, idname, nullptr, true, false));
            }
            case T_FLOATTYPE:
            case T_INTTYPE:
            case T_STRTYPE: {
                // These are also used as built-in functions, so allow them to function as
                // identifier for calls.
                string idname = lex.sattr;
                lex.Next();
                if (lex.token != T_LEFTPAREN) Error("type used as expression");
                return IdentFactor(idname);
            }
            case T_TYPEOF: {  // "return", ident or type.
                lex.Next();
                if (lex.token == T_RETURN) {
                    lex.Next();
                    return new Node(lex, T_TYPEOF, nullptr);
                }
                if (lex.token == T_IDENT) {
                    auto id = st.Lookup(lex.sattr);
                    if (id) {
                        lex.Next();
                        return new Node(lex, T_TYPEOF, new Node(lex, id));
                    }
                }
                auto tn = new Node(lex, T_TYPE, TypeRef());
                ParseType(tn->typenode(), false);
                return new Node(lex, T_TYPEOF, tn);
            }
            case T_IDENT: {
                string idname = lex.sattr;
                lex.Next();
                return IdentFactor(idname);
            }
            default:
                Error("illegal start of expression: " + lex.TokStr());
                return nullptr;
        }
    }

    Node *IdentFactor(const string &idname) {
        if (IsNext(T_LEFTCURLY)) {
            auto &struc = st.StructUse(idname, lex);
            vector<Node *> exps(struc.fields.size(), nullptr);
            ParseVector([&] () {
                auto id = lex.sattr;
                if (IsNext(T_IDENT)) {
                    if (IsNext(T_COLON)) {
                        auto fld = st.FieldUse(id);
                        auto field = struc.Has(fld);
                        if (field < 0) Error("unknown field: " + id);
                        if (exps[field]) Error("field initialized twice: " + id);
                        exps[field] = ParseExp(T_CONSTRUCTOR);
                        return;
                    } else {  // Undo
                        lex.PushCur();
                        lex.Push(T_IDENT, id);
                        lex.Next();
                    }
                }
                // An initializer without a tag. Find first field without a default thats not
                // set yet.
                for (size_t i = 0; i < exps.size(); i++) {
                    if (!exps[i] && !struc.fields.v[i].defaultval) {
                        exps[i] = ParseExp(T_CONSTRUCTOR);
                        return;
                    }
                }
                Error("too many initializers for: " + struc.name);
            }, T_RIGHTCURLY);
            // Now fill in defaults, check for missing fields, and construct list.
            Node *list = nullptr;
            Node **tail = &list;
            for (size_t i = 0; i < exps.size(); i++) {
                if (!exps[i]) {
                    if (struc.fields.v[i].defaultval)
                        exps[i] = struc.fields.v[i].defaultval->Clone();
                    else
                        Error("field not initialized: " + struc.fields.v[i].id->name);
                }
                AddTail(tail, exps[i]);
            }
            auto tn = new Node(lex, T_TYPE, TypeRef());
            tn->typenode() = &struc.thistype;
            return new Node(lex, T_CONSTRUCTOR, list, tn);
        } else {
            // If we see "f(" the "(" is the start of an argument list, but for "f (", "(" is
            // part of an expression of a single argument with no extra "()".
            // This avoids things like "f (1 + 2) * 3" ("* 3" part of the single arg) being
            // interpreted as "f(1 + 2) * 3" (not part of the arg).
            // This is benign, since single arg calls with "()" work regardless of whitespace,
            // and multi-arg calls with whitespace will now error on the first "," (since we
            // don't have C's ","-operator.
            auto nf = natreg.FindNative(idname);
            auto f = st.FindFunction(idname);
            if (lex.token == T_LEFTPAREN && lex.whitespacebefore == 0) {
                return ParseFunctionCall(f, nf, idname, nullptr, false, false);
            }
            // Check for implicit variable.
            if (idname[0] == '_') {
                return new Node(lex, st.LookupDef(idname, lex.errorline, lex, true, false));
            }
            // Check for function call without ().
            auto id = st.Lookup(idname);
            if (!id && (nf || f) && lex.whitespacebefore > 0) {
                return ParseFunctionCall(f, nf, idname, nullptr, false, true);
            }
            // Check for field reference in function with :: arguments.
            id = nullptr;
            auto fld = st.LookupWithStruct(idname, lex, id);
            if (fld) {
                return new Node(lex, T_DOT, new Node(lex, id), new Node(lex, fld));
            }
            // It's a regular variable.
            return new Node(lex, st.LookupUse(idname, lex));
        }
    }

    bool IsNext(TType t) {
        bool isnext = lex.token == t;
        if (isnext) lex.Next();
        return isnext;
    }

    string lastid;

    bool IsNextId() {
        if (lex.token != T_IDENT) return false;
        lastid = lex.sattr;
        lex.Next();
        return true;
    }

    const string &ExpectId() {
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

    void Expect(TType t) {
        if (!IsNext(t))
            Error(lex.TokStr(t) + " expected, found: " + lex.TokStr());
    }

    string DumpAll(bool onlytypechecked = false) {
        string s;
        for (auto f : st.functiontable) {
            for (auto sf = f->subf; sf; sf = sf->next) {
                if (!onlytypechecked || sf->typechecked) {
                    s += "FUNCTION: " + f->name + "(";
                    for (auto &arg : sf->args.v) {
                        s += arg.id->name + ":" + TypeName(arg.type) + " ";
                    }
                    s += ")\n";
                    if (sf->body) s += Dump(*sf->body, 4);
                    s += "\n\n";
                }
            }
        }
        return s + "TOPLEVEL:\n" + Dump(*root, 0);
    }
};

}  // namespace lobster
