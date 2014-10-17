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

struct Parser
{
    Lex lex;
    Node *root;
    SymbolTable &st;
    vector<Node *> autoparstack;
    vector<Ident *> maybeundefined;
    vector<int> functionstack;
    vector<SubFunction *> subfunctionstack;
    vector<string> trailingkeywordedfunctionvaluestack;
    Struct *currentstruct;

    struct ForwardFunctionCall { string idname; size_t maxscopelevel; Node *n; };
    vector<ForwardFunctionCall> forwardfunctioncalls;

    Parser(const char *_src, SymbolTable &_st, char *_stringsource)
        : lex(_src, _st.filenames, _stringsource), root(nullptr), st(_st), currentstruct(nullptr)
    {
        assert(parserpool == nullptr);
        parserpool = new SlabAlloc();
    }
    
    ~Parser()
    {
        delete root;

        delete parserpool;
        parserpool = nullptr;
    }

    void Error(string err, const Node *what = nullptr)
    {
        if (what) lex.Error(err, what->fileidx, what->linenumber);
        else      lex.Error(err);
    }

    void Parse()
    {
        st.ScopeStart();

        root = ParseStatements();

        st.ScopeCleanup(lex);

        Expect(T_ENDOFFILE);

        if (!autoparstack.empty()) Error("implicit _ argument(s) used at top level");

        assert(forwardfunctioncalls.empty());
    }

    SubFunction *CurSF() { return subfunctionstack.empty() ? nullptr : subfunctionstack.back(); }

    void AddTail(Node **&tail, Node *a)
    {
        *tail = new Node(lex, T_LIST, a);
        tail = &(*tail)->tail();
    }

    Node *ParseStatements()
    {
        Node *list = nullptr;
        Node **tail = &list;

        for (;;)
        {
            AddTail(tail, ParseTopExp());
                
            if (lex.token == T_ENDOFINCLUDE)
            {
                st.EndOfInclude();
                lex.PopIncludeContinue();
            }
            else if (!IsNext(T_LINEFEED))
            {
                break;
            }
                
            if (Either(T_ENDOFFILE, T_DEDENT)) break;
        }

        ResolveForwardFunctionCalls();

        for (auto n = list; n; n = n->tail())
        {
            auto def = n->head();
            switch (def->type)
            {
                case T_STRUCTDEF:
                {
                    st.UnregisterStruct(def->struct_id()->st());
                    break;
                }
                case T_FUNDEF:
                {
                    st.UnregisterFun(def->function_def()->sf()->parent);
                    break;
                }
                case T_DEF:
                {
                    // FIXME: this never makes anything const if def->right() is another T_DEF
                    auto id = def->left()->ident();
                    id->static_constant = id->single_assignment && def->right()->IsConst();
                    break;
                }
            }
        }

        return list;
    }

    Node *ParseVector(int fieldid = -1)
    {
        if (IsNext(T_RIGHTBRACKET)) return nullptr;

        bool indented = IsNext(T_INDENT);

        Node *list = nullptr;
        Node **tail = &list;

        for (;;)
        {
            AddTail(tail, fieldid < 0 ? ParseExp() : ParseField(fieldid++));
            
            if (!IsNext(T_COMMA)) break;
        }

        if (indented)
        {
            IsNext(T_LINEFEED);
            Expect(T_DEDENT);
            Expect(T_LINEFEED);
        }

        Expect(T_RIGHTBRACKET);
        return list;
    }

    Node *ParseField(int idx)
    {
        string fname = lex.sattr;
        Expect(T_IDENT);
        auto n = new Node(lex, &st.FieldDecl(fname, idx, currentstruct, lex));
        if (IsNext(T_COLON)) ParseType(n->exptype, false);
        return n;
    }

    Node *DefineWith(const string &idname, Node *e, bool isprivate, bool isdef, bool islogvar)
    {
        auto id = isdef ? st.LookupLexDefOrDynScope(idname, lex.errorline, lex, false, CurSF()) 
                        : st.LookupLexUse(idname, lex);

        if (islogvar)
        {
            id->logvaridx = 0;
            st.uses_frame_state = true;
        }

        if (isprivate)
        {
            if (!isdef) Error("assignment cannot be made private");
            id->isprivate = true;
        }

        return new Node(lex, isdef ? T_DEF : T_ASSIGNLIST, new Node(lex, id), e);
    }

    Node *RecMultiDef(const string &idname, bool isprivate, int nids, bool &isdef, bool &islogvar)
    {
        Node *e = nullptr;
        if (IsNextId())
        {
            string id2 = lastid;
            nids++;
            if (Either(T_DEF, T_LOGASSIGN, T_ASSIGN))
            {
                isdef = lex.token != T_ASSIGN;
                islogvar = lex.token == T_LOGASSIGN;
                lex.Next();

                int nrv;
                e = DefineWith(id2, ParseMultiRet(ParseOpExp(), nrv), isprivate, isdef, islogvar);
                if (nrv > 1 && nrv != nids)
                    Error("number of values doesn't match number of variables");
            }
            else if (IsNext(T_COMMA))
            {
                e = RecMultiDef(id2, isprivate, nids, isdef, islogvar);
            }
            else
            {
                lex.Undo(T_IDENT, id2);
            }
        }
        if (e)
        {
            e = DefineWith(idname, e, isprivate, isdef, islogvar);
        }
        else
        {
            lex.Undo(T_COMMA);
            lex.Undo(T_IDENT, idname);
        }
        return e;
    }

    Node *ParseTopExp(bool isprivate = false)
    {   
        switch(lex.token)
        {
            case T_PRIVATE:
                if (st.scopelevels.size() != 1 || isprivate)
                    Error("private must be used at file scope");
                
                lex.Next();
                return ParseTopExp(true);
                
            case T_INCLUDE:
            {
                if (isprivate)
                    Error("include cannot be private");
                
                lex.Next();
                string fn = lex.sattr;
                Expect(T_STR);
                lex.Include((char *)fn.c_str());

                return ParseTopExp();
            }

            case T_VALUE:
            case T_STRUCT:
            {
                bool isvalue = lex.token == T_VALUE;
                lex.Next();
                string sname = lex.sattr;
                Expect(T_IDENT);

                Struct &struc = st.StructDecl(sname, lex);
                struc.readonly = isvalue;
                struc.isprivate = isprivate;

                currentstruct = &struc;

                Expect(T_COLON);

                int fieldid = 0;

                if (IsNextId())
                {
                    auto &base = st.StructUse(lastid, lex);
                    struc.superclass = &base;
                    struc.superclassidx = base.idx;

                    int off = 0;
                    for (auto &fld : base.fields)
                    {
                        struc.fields.push_back(fld);
                        fld.sf->NewFieldUse(FieldOffset(struc.idx, off++));
                    }

                    fieldid = base.fields.size();
                }

                Expect(T_LEFTBRACKET);

                auto v = ParseVector(fieldid);

                for (auto ids = v; ids; ids = ids->tail())
                {
                    assert(ids->head()->type == T_FIELD);
                    struc.fields.push_back(UniqueField(ids->head()->exptype, ids->head()->fld()));
                }

                currentstruct = nullptr;

                return new Node(lex, T_STRUCTDEF, new Node(lex, &struc), v);
            }

            case T_FUN:
            {
                lex.Next();
                return ParseFunctionDefinition(isprivate);
            }

            default:
            {
                if (IsNextId())
                {
                    auto idname = lastid;
                    bool dynscope = lex.token == T_DYNASSIGN;
                    bool constant = lex.token == T_DEFCONST;
                    bool logvar = lex.token == T_LOGASSIGN;
                    // codegen assumes these defs can only happen at toplevel
                    if (lex.token == T_DEF || dynscope || constant || logvar)
                    {
                        lex.Next();
                        auto e = ParseExp();
                        auto id = st.LookupLexDefOrDynScope(idname, lex.errorline, lex, dynscope, CurSF());
                        if (dynscope)  id->Assign(lex);
                        if (constant)  id->constant = true;
                        if (isprivate) id->isprivate = true;
                        if (logvar)  { id->logvaridx = 0; st.uses_frame_state = true; }
                        return new Node(lex, T_DEF, new Node(lex, id), e);
                    }
                    else if (IsNext(T_COMMA))
                    {
                        bool isdef = false;
                        bool islogvar = false;
                        auto e = RecMultiDef(idname, isprivate, 1, isdef, islogvar);
                        if (e) return e;
                    }
                    else
                    {
                        lex.Undo(T_IDENT, idname);
                    }
                }
                
                if (isprivate)
                    Error("private only applies to declarations");
                
                return ParseExpStat();
            }
        }
    }

    int CountList(Node *n)
    {
        int c = 0;
        for (; n; n = n->tail()) c++;
        return c;
    }

    Node *ParseFunctionDefinition(bool isprivate = false)
    {
        string idname = lex.sattr;
        Expect(T_IDENT);

        if (natreg.FindNative(idname))
            Error("cannot override built-in function: " + idname);

        auto args = ParseFunDefArgs();
        auto nargs = CountList(args);
        auto &f = st.FunctionDecl(idname, nargs, lex);
        
        if (f.subf)
        {
            f.multimethod = true;
            if (isprivate != f.isprivate)
                Error("inconsistent private annotation of multiple function implementations for " + idname);
        }
        f.isprivate = isprivate;
        
        SubFunction *sf = new SubFunction(&f);
        sf->next = f.subf;
        f.subf = sf;

        functionstack.push_back(f.idx);
        subfunctionstack.push_back(sf);
        for (auto n = args; n; n = n->tail()) n->head()->ident()->sf = sf;
        sf->body = ParseFunDefBody(args, &f);
        subfunctionstack.pop_back();
        functionstack.pop_back();

        for (auto stat = sf->body->body(); stat; stat = stat->tail())
            if (!stat->tail() && (stat->head()->type != T_RETURN ||
                                  stat->head()->return_function_idx()->integer() != f.idx /* return from */))
                ReturnValues(f, 1);
        assert(f.retvals);

        sf->args = new Arg[nargs];
        int i = 0;
        for (auto a = args; a; a = a->tail())
        {
            Arg &arg = sf->args[i++];
            arg.type = a->head()->exptype;
            arg.flags = arg.type.t == V_ANY ? AF_ANYTYPE : AF_NONE;
            arg.id = a->head()->ident()->name;
        }

        return new Node(lex, T_FUNDEF, new Node(lex, sf), sf->body);
    }

    void ParseType(Type &dest, bool withtype)
    {
        switch(lex.token)
        {
            case T_INTTYPE:   dest.t = V_INT;       lex.Next(); break;
            case T_FLOATTYPE: dest.t = V_FLOAT;     lex.Next(); break;
            case T_STRTYPE:   dest.t = V_STRING;    lex.Next(); break;
            case T_COROUTINE: dest.t = V_COROUTINE; lex.Next(); break;
            case T_NIL:       dest.t = V_NIL;       lex.Next(); break;
            case T_FUN:       dest.t = V_FUNCTION;  lex.Next(); break;
            case T_VECTTYPE:  dest.t = V_VECTOR;    lex.Next(); break;

            case T_IDENT:
            {
                dest.t = V_STRUCT;
                dest.idx = st.StructUse(lex.sattr, lex).idx;
                lex.Next();
                break;
            }
            default:
                Error("illegal type syntax: " + lex.TokStr());
        }

        if (withtype && dest.idx < 0) Error(":: must be used with a struct type");
    }

    Node *ParseFunArgs(bool coroutine, Node *derefarg, const char *fname = "", Arg *args = nullptr, int nargs = -1)
    {
        if (derefarg)
        {
            CheckArg(nargs, 0, fname);
            if (IsNext(T_LEFTPAREN))
            {
                return new Node(lex, T_LIST, derefarg, ParseFunArgsRec(false, false, args, nargs, 1, fname));
            }
            else
            {
                return new Node(lex, T_LIST, derefarg);
            }
        }
        else
        {
            Expect(T_LEFTPAREN);
            return ParseFunArgsRec(coroutine, false, args, nargs, 0, fname);
        }
    }

    Node *ParseFunArgsRec(bool coroutine, bool needscomma, Arg *argdecls, int nargs, int thisarg, const char *fname)
    {
        if (IsNext(T_RIGHTPAREN))
        {
            return ParseTrailingFunctionValues(coroutine, argdecls, nargs, thisarg, fname);
        }
        if (needscomma) Expect(T_COMMA);
        Node *arg = ParseExp();

        CheckArg(nargs, thisarg, fname);
        if (argdecls && argdecls[thisarg].flags == NF_EXPFUNVAL)
            arg = new Node(lex, T_CLOSURE, nullptr, new Node(lex, T_LIST, arg));

        return new Node(lex, T_LIST, arg, ParseFunArgsRec(coroutine, true, argdecls, nargs, thisarg + 1, fname));
    }

    void CheckArg(int nargs, int thisarg, const char *fname)
    {
        if (thisarg == nargs) Error("too many arguments passed to function " + string(fname));
    }

    Node *ParseTrailingFunctionValues(bool coroutine, Arg *argdecls, int nargs, int thisarg, const char *fname)
    {
        if (argdecls && thisarg + 1 < nargs) trailingkeywordedfunctionvaluestack.push_back(argdecls[thisarg + 1].id);

        Node *e = nullptr;
        switch (lex.token)
        {
            case T_COLON:
                e = ParseFunDefBody(ParseFunDefArgs(false, false));
                break;

            case T_IDENT:
                // skip if this function value starts with an ID that's equal to the parents next
                // keyworded function val ID, e.g. "else" in: if(..): currentcall(..) else: ..
                if (trailingkeywordedfunctionvaluestack.empty() || 
                    trailingkeywordedfunctionvaluestack.back() != lex.sattr)
                    e = ParseFunDefBody(ParseFunDefArgs(false));
                break;
            
            case T_LEFTPAREN:
                e = ParseFunDefBody(ParseFunDefArgs());
                break;
        }

        if (argdecls && thisarg + 1 < nargs) trailingkeywordedfunctionvaluestack.pop_back();

        if (!e)
        {
            if (coroutine) { e = new Node(lex, T_COCLOSURE); coroutine = false; }
            else return nullptr;
        }

        CheckArg(nargs, thisarg, fname);

        thisarg++;

        Node *tail = nullptr;

        bool islf = lex.token == T_LINEFEED;
        if (argdecls && thisarg < nargs && (lex.token == T_IDENT || islf))
        {
            if (islf) lex.Next();
            if (lex.token == T_IDENT && argdecls[thisarg].id == lex.sattr)
            {
                lex.Next();
                tail = ParseTrailingFunctionValues(coroutine, argdecls, nargs, thisarg, fname);
            }
            else
            {
                lex.PushCur();
                if (islf) lex.Push(T_LINEFEED);
                lex.Next();
            }
        }

        return new Node(lex, T_LIST, e, tail);
    }

    Node *ParseFunDefArgs(bool full = true, bool parseargs = true)
    {
        st.ScopeStart();
        if (full) Expect(T_LEFTPAREN);

        Node *n = nullptr;

        if (lex.token != T_RIGHTPAREN && parseargs)
        {
            Node **tail = &n;
            for (;;)
            {
                string a = lex.sattr;
                Expect(T_IDENT);

                auto id = new Node(lex, st.LookupLexDefOrDynScope(a, lex.errorline, lex, false,
                    CurSF()  // this is good for function vals, but bad for named functions, which get corrected later
                ));

                bool withtype = lex.token == T_TYPEIN;
                if (full && (lex.token == T_COLON || withtype))
                {
                    lex.Next(); 

                    ParseType(id->exptype, withtype);

                    if (withtype) st.AddWithStruct(id->exptype, id->ident(), lex);
                }

                *tail = new Node(lex, T_LIST, id);
                tail = &(*tail)->tail();

                if (!IsNext(T_COMMA)) break;
            }
        }

        if (full) Expect(T_RIGHTPAREN);
        Expect(T_COLON);

        return n;
    }

    Node *ParseFunDefBody(Node *n, Function *f = nullptr)  // continuation of ParseFunDefArgs(), n is its return value
    {
        size_t autoparlevel = autoparstack.size();
        Node *body = nullptr;

        if (IsNext(T_INDENT))
        {
            body = ParseStatements();
            Expect(T_DEDENT);
        }
        else
        {
            body = new Node(lex, T_LIST, ParseExpStat());
        }

        if (autoparlevel < autoparstack.size())
        {
            if (f) Error("cannot use anonymous argument: " + autoparstack[autoparlevel]->ident()->name + 
                         ", in named function: " + f->name, autoparstack[autoparlevel]);
            if (n) Error("cannot mix anonymous argument: " + autoparstack[autoparlevel]->ident()->name +
                         ", with declared arguments in function", autoparstack[autoparlevel]);

            auto ap = &n;
            for (size_t i = autoparlevel; i < autoparstack.size(); i++) 
            {
                for (size_t j = autoparlevel; j < i; j++)
                    if (autoparstack[i]->ident() == autoparstack[j]->ident())
                        goto twice;
                *ap = new Node(lex, T_LIST, new Node(lex, autoparstack[i]->ident()));
                ap = &(*ap)->tail();
                twice:;
            }
            while (autoparstack.size() > autoparlevel) autoparstack.pop_back();
        }

        n = new Node(lex, T_CLOSURE, n, body);

        st.ScopeCleanup(lex);
        return n;
    }

    Node *ParseMultiRet(Node *first, int &nrv)
    {
        nrv = 1;
        if (lex.token != T_COMMA) return first;

        auto list = new Node(lex, T_MULTIRET, first, nullptr);
        auto tail = &list->tailexps();
        while (IsNext(T_COMMA))
        {
            *tail = new Node(lex, T_MULTIRET, ParseOpExp(), nullptr);
            tail = &(*tail)->tailexps();
            nrv++;
        }

        return list;
    }

    void ReturnValues(Function &f, int nrv)
    {
        if (f.retvals && f.retvals != nrv)
            Error(string("all return statements of this function must return the same amount of return values."
                         " previously: ") + inttoa(f.retvals));
        f.retvals = nrv;
    }

    Node *ParseExpStat()
    {
        if (IsNext(T_RETURN))
        {
            Node *rv = nullptr;
            int nrv = 0;
            if (!Either(T_LINEFEED, T_DEDENT, T_FROM))
            {
                rv = ParseMultiRet(ParseOpExp(), nrv);
                if (rv->type == T_CALL)
                    nrv = max(nrv, rv->call_function()->sf()->parent->retvals);
            }

            int fid = -2;
            if (IsNext(T_FROM))
            {
                if(!IsNext(T_PROGRAM))
                {
                    if (!IsNextId()) Error("return from: must be followed by function identifier or \"program\"");
                    auto f = st.FindFunction(lastid);
                    if (!f) Error("return from: not a known function");
                    if (f->sibf) Error("return from: function must have single implementation");
                    fid = f->idx;
                }
            }
            else
            {
                if (functionstack.size())
                    fid = functionstack.back();
            }

            if (fid >= 0)
                ReturnValues(*st.functiontable[fid], nrv);
            else if (nrv > 1)
                Error("cannot return multiple values from top level");

            return new Node(lex, T_RETURN, rv, new Node(lex, T_INT, fid));
        }

        auto e = ParseExp();

        while (IsNext(T_SEMICOLON))
        {
            if (IsNext(T_LINEFEED))
                Error("\';\' is not a statement terminator");  // specialized error for all the C-style language users
            e = new Node(lex, T_SEQ, e, ParseExp());
        }

        return e;
    }
    
    void Modify(Node *e)
    {
        if (e->type == T_IDENT)
            e->ident()->Assign(lex);
    }

    Node *ParseExp()
    {
        auto e = ParseOpExp();

        switch (lex.token)
        {
            case T_ASSIGN:  
            case T_PLUSEQ: 
            case T_MINUSEQ: 
            case T_MULTEQ: 
            case T_DIVEQ:
            case T_MODEQ:
            {
                auto type = lex.token;
                if (e->type != T_IDENT && e->type != T_DOT && e->type != T_CO_AT && e->type != T_INDEX)
                    Error("illegal left hand side of assignment");
                Modify(e);
                lex.Next();
                e = new Node(lex, type, e, ParseExp());
            }
        }

        return e;
    }

    Node *ParseOpExp(uint level = 4)
    {
        static TType ops[][4] =
        {
            { T_MULT, T_DIV, T_MOD, T_NONE },
            { T_PLUS, T_MINUS, T_NONE, T_NONE },
            { T_LT, T_GT, T_LTEQ, T_GTEQ },
            { T_EQ, T_NEQ, T_NONE, T_NONE },
            { T_AND, T_OR, T_NONE, T_NONE },
        };

        Node *exp = level ? ParseOpExp(level - 1) : ParseUnary();

        TType *o = &ops[level][0];
        while (Either(o[0], o[1]) || Either(o[2], o[3]))
        {
            TType op = lex.token;
            lex.Next();
            exp = new Node(lex, op, exp, level ? ParseOpExp(level - 1) : ParseUnary());
        }

        return exp;
    }

    Node *ParseUnary()
    {
        switch (lex.token)
        {
            case T_MINUS: 
            case T_NOT:
            case T_INCR:
            case T_DECR:
            {
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

    Node *ParseFunctionCall(Function *f, NativeFun *nf, string &idname, Node *firstarg, bool coroutine)
    {
        if (nf)
        {
            auto args = ParseFunArgs(coroutine, firstarg, idname.c_str(), nf->args, nf->nargs);

            Node **ai = &args;
            for (int i = 0; i < nf->nargs; i++)
            {
                if (!*ai)
                {
                    if (nf->args[i].flags == NF_OPTIONAL)
                    {
                        *ai = new Node(lex, T_LIST, new Node(lex, T_NIL));   
                    }
                    else
                    {
                        // FIXME: here we can look if "ids" exists in this scope, and substitute it
                        Error("missing arg to builtin function: " + idname); 
                    }
                }
                ai = &(*ai)->tail();
            }
            return new Node(lex, T_NATCALL, new Node(lex, nf), args);
        }

        if (f)
        {
            auto bestf = f;
            for (auto fi = f->sibf; fi; fi = fi->sibf) 
                if (fi->nargs > bestf->nargs) bestf = fi;

            auto args = ParseFunArgs(coroutine, firstarg, idname.c_str(),  bestf->subf->args, bestf->nargs);
            auto nargs = CountList(args);

            f = FindFunctionWithNargs(f, nargs, idname, nullptr);

            return new Node(lex, T_CALL, new Node(lex, f->subf), args);
        }

        auto args = ParseFunArgs(coroutine, firstarg);
        auto id = st.LookupLexMaybe(idname);
        if (id)
            return new Node(lex, T_DYNCALL, new Node(lex, id), args);

        auto n = new Node(lex, T_CALL, new Node(lex, (SubFunction *)nullptr), args);
        ForwardFunctionCall ffc = { idname, st.scopelevels.size(), n };
        forwardfunctioncalls.push_back(ffc);
        return n;
    }

    Function *FindFunctionWithNargs(Function *f, int nargs, const string &idname, Node *errnode)
    {
        for (; f; f = f->sibf) 
            if (f->nargs == nargs) 
                return f;

        Error("no version of function " + idname + " takes " + inttoa(nargs) + " arguments", errnode);
        return nullptr;
    }

    void ResolveForwardFunctionCalls()
    {
        for (auto ffc = forwardfunctioncalls.begin(); ffc != forwardfunctioncalls.end(); )
        {
            if (ffc->maxscopelevel >= st.scopelevels.size())
            {
                auto f = st.FindFunction(ffc->idname);
                if (f)
                {
                    ffc->n->call_function()->sf() = FindFunctionWithNargs(f, CountList(ffc->n->call_args()), ffc->idname, ffc->n)->subf;
                    ffc = forwardfunctioncalls.erase(ffc);
                    continue;
                }
                else
                {
                    if (st.scopelevels.size() == 1)
                        Error("call to unknown function: " + ffc->idname, ffc->n);

                    ffc->maxscopelevel = st.scopelevels.size() - 1; // prevent it being found in sibling scopes
                }
            }
            ffc++;
        }
    }

    Node *ParseDeref()
    {
        auto n = ParseFactor();

        for (;;) switch (lex.token)
        {
            case T_DOT:
            {
                lex.Next();
                string idname = lex.sattr;
                Expect(T_IDENT);
                if (IsNext(T_AT))
                {
                    string fname = lex.sattr;
                    Expect(T_IDENT);
                    // TODO: this is here because currently these functions have to be declared before use
                    if (!st.FindFunction(fname))
                        Error(string("function ") + fname + " hasn't been declared yet");
                    auto id = st.LookupIdentInFun(idname, fname);
                    if (!id)    
                        Error(string("no unique local variable ") + idname + " in function with name " + fname);
                    n = new Node(lex, T_CO_AT, n, new Node(lex, id));
                }
                else
                {
                    SharedField *fld = st.FieldUse(idname);
                    if (fld) 
                    {
                        n = new Node(lex, T_DOT, n, new Node(lex, fld));
                    }
                    else
                    {
                        auto f = st.FindFunction(idname);
                        auto nf = natreg.FindNative(idname);
                        if (f || nf)
                        {
                            n = ParseFunctionCall(f, nf, idname, n, false);
                        }
                        else
                        {
                            Error("not a type member or function: " + idname);
                        }
                    }
                }
                break;
            }

            case T_LEFTBRACKET:
            {
                lex.Next();
                n = new Node(lex, T_INDEX, n, ParseExp());
                Expect(T_RIGHTBRACKET);
                break;
            }

            case T_INCR:
            case T_DECR:
                Modify(n);
                n = new Node(lex, lex.token == T_INCR ? T_POSTINCR : T_POSTDECR, n);
                lex.Next();
                return n;

            case T_LEFTPAREN:   // only for dyn calls
            {
                auto args = ParseFunArgs(false, nullptr);
                n = new Node(lex, T_DYNCALL, n, args);
                break;
            }

            case T_IS:
            {
                lex.Next();
                Type type;
                ParseType(type, false);
                return new Node(lex, T_IS, n, new Node(lex, type));
            }

            default:
                return n;
        }
    }

    Node *ParseFactor()
    {
        switch (lex.token)
        {
            case T_INT:   { int i    = atoi(lex.sattr.c_str()); lex.Next(); return new Node(lex, T_INT, i); }
            case T_FLOAT: { double f = atof(lex.sattr.c_str()); lex.Next(); return new Node(lex, T_FLOAT, f); }
            case T_STR:   { string s = lex.sattr;               lex.Next(); return new Node(lex, T_STR, s); }     
            case T_NIL:   {                                     lex.Next(); return new Node(lex, T_NIL); }

            case T_LEFTPAREN:
            {
                lex.Next();
                auto n = ParseExp();
                Expect(T_RIGHTPAREN);
                return n;
            }

            case T_LEFTBRACKET:
            {
                lex.Next();

                Node *n;
                if (IsNext(T_SUPER))
                {
                    n = new Node(lex, T_LIST, new Node(lex, T_SUPER, ParseExp()));
                    if (IsNext(T_COMMA)) n->b() = ParseVector();
                    else Expect(T_RIGHTBRACKET);
                }
                else
                {
                    n = ParseVector();
                }

                // FIXME: this type is not in line with other types: any non-struct type means a vector of it.
                Type type;

                if (IsNext(T_COLON))
                {
                    ParseType(type, false);
                    if (type.t == V_STRUCT)
                    {
                        auto struc = st.structtable[type.idx];
                        int nargs = CountList(n);
                        int reqargs = struc->fields.size();
                        if (n && n->head()->type == T_SUPER)
                            reqargs -= struc->superclass->fields.size() - 1;
                        if (nargs != reqargs)
                            Error("constructor requires " + string(inttoa(reqargs)) +
                                  " arguments, not " + string(inttoa(nargs)));
                    }
                    else
                    {
                        if (type.t == V_VECTOR) Error("vector type annotation superfluous");
                    }
                }
                return new Node(lex, T_CONSTRUCTOR, n, new Node(lex, type));
            }

            case T_FUN:
            {
                lex.Next();
                return ParseFunDefBody(ParseFunDefArgs());
            }

            case T_COROUTINE:
            {
                lex.Next();
                string idname = lex.sattr;
                Expect(T_IDENT);
                return new Node(lex, T_COROUTINE, ParseFunctionCall(st.FindFunction(idname), natreg.FindNative(idname),
                                idname, nullptr, true));
            }

            case T_IDENT:
            {
                string idname = lex.sattr;
                lex.Next();

                switch (lex.token)
                {
                    case T_LEFTPAREN:
                        return ParseFunctionCall(st.FindFunction(idname), natreg.FindNative(idname),
                                                 idname, nullptr, false);

                    default:
                        if (idname[0] == '_')
                        {
                            auto dest = new Node(lex, st.LookupLexDefOrDynScope(idname, lex.errorline, lex, 
                                                                                true, CurSF()));
                            autoparstack.push_back(dest);
                            return dest;
                        }
                        else
                        {
                            Ident *id = nullptr;
                            auto fld = st.LookupWithStruct(idname, lex, id);
                            if (fld)
                            {
                                return new Node(lex, T_DOT, new Node(lex, id), new Node(lex, fld));
                            }

                            return new Node(lex, st.LookupLexUse(idname, lex));
                        }
                    }
            }

            default:
                Error("illegal start of expression: " + lex.TokStr());
                return nullptr;
        }
    }

    bool IsNext(TType t)
    {
        bool isnext = lex.token == t;
        if (isnext) lex.Next();
        return isnext;
    }

    string lastid;

    bool IsNextId()
    {
        if (lex.token != T_IDENT) return false;
        lastid = lex.sattr;
        lex.Next();
        return true;
    }

    bool Either(TType t1, TType t2)           { return lex.token == t1 || lex.token == t2; }
    bool Either(TType t1, TType t2, TType t3) { return lex.token == t1 || lex.token == t2 || lex.token == t3; }

    void Expect(TType t)
    {
        if (!IsNext(t))
            Error(lex.TokStr(t) + " expected, found: " + lex.TokStr());
    }

    string Dump()
    { 
        string s;
        for (auto f : st.functiontable)
        {
            for (auto sf = f->subf; sf; sf = sf->next)
            {
                s += "FUNCTION: " + f->name + " ";
                if (sf->body->parameters()) s += sf->body->parameters()->Dump(0, st);
                s += "\n";
                s += sf->body->body()->Dump(4, st);
                s += "\n\n";
            }
        }
        return s + "TOPLEVEL:\n" + root->Dump(0, st); 
    }
};
