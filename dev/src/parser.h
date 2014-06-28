
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

    Parser(const char *_src, SymbolTable &_st, char *_stringsource) : lex(_src, _st.filenames, _stringsource), root(NULL), st(_st), currentstruct(NULL)
    {
        assert(parserpool == NULL);
        parserpool = new SlabAlloc();
    }
    
    ~Parser()
    {
        delete root;

        delete parserpool;
        parserpool = NULL;
    }

    void Error(string err, Node *what = NULL)
    {
        if (what) lex.Error(err, what->fileidx, what->linenumber);
        else      lex.Error(err);
    }

    void Parse()
    {
        st.ScopeStart();

        root = ParseStatements();

        st.ScopeCleanup(lex);

        Expect('EOF');

        if (!autoparstack.empty()) Error("implicit _ argument(s) used at top level");

        assert(forwardfunctioncalls.empty());
    }

    SubFunction *CurSF() { return subfunctionstack.empty() ? NULL : subfunctionstack.back(); }

    void AddTail(Node **&tail, Node *a)
    {
        *tail = new Node(lex, ',', a);
        tail = &(*tail)->b;
    }

    Node *ParseStatements()
    {
        Node *list = NULL;
        Node **tail = &list;

        for (;;)
        {
            AddTail(tail, ParseTopExp());
                
            if (lex.token == 'EOI')
            {
                st.EndOfInclude();
                lex.PopIncludeContinue();
            }
            else if (!IsNext('LF'))
            {
                break;
            }
                
            if (Either('EOF', 'DED')) break;
        }

        ResolveForwardFunctionCalls();

        for (auto n = list; n; n = n->b)
        {
            switch (n->a->type)
            {
                case 'ADT':
                {
                    st.UnregisterStruct(n->a->a->st);
                    break;
                }
                case 'FUN':
                {
                    st.UnregisterFun(n->a->a->f);
                    break;
                }
                case ':=':
                {
                    // FIXME: this never makes anything const if n->a->b is another ':='
                    auto id = n->a->a->ident;
                    id->static_constant = id->single_assignment && n->a->b->IsConst();
                    break;
                }
            }
        }

        return list;
    }

    Node *ParseVector(int fieldid = -1)
    {
        if (IsNext(']')) return NULL;

        bool indented = IsNext('IND');

        Node *list = NULL;
        Node **tail = &list;

        for (;;)
        {
            AddTail(tail, fieldid < 0 ? ParseExp() : ParseField(fieldid++));
            
            if (!IsNext(',')) break;
        }

        if (indented)
        {
            IsNext('LF');
            Expect('DED');
            Expect('LF');
        }

        Expect(']');
        return list;
    }

    Node *ParseField(int idx)
    {
        string fname = lex.sattr;
        Expect('ID');
        auto n = new Node(lex, &st.FieldDecl(fname, idx, currentstruct, lex));
        if (IsNext(':')) ParseType(n->exptype, false);
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

        return new Node(lex, isdef ? ':=' : ',=', new Node(lex, id), e);
    }

    Node *RecMultiDef(const string &idname, bool isprivate, int nids, bool &isdef, bool &islogvar)
    {
        Node *e = NULL;
        if (IsNextId())
        {
            string id2 = lastid;
            nids++;
            if (Either(':=', '?=', '='))
            {
                isdef = lex.token != '=';
                islogvar = lex.token == '?=';
                lex.Next();

                int nrv;
                e = DefineWith(id2, ParseMultiRet(ParseOpExp(), nrv), isprivate, isdef, islogvar);
                if (nrv > 1 && nrv != nids)
                    Error("number of values doesn't match number of variables");
            }
            else if (IsNext(','))
            {
                e = RecMultiDef(id2, isprivate, nids, isdef, islogvar);
            }
            else
            {
                lex.Undo('ID', id2);
            }
        }
        if (e)
        {
            e = DefineWith(idname, e, isprivate, isdef, islogvar);
        }
        else
        {
            lex.Undo(',');
            lex.Undo('ID', idname);
        }
        return e;
    }

    Node *ParseTopExp(bool isprivate = false)
    {   
        switch(lex.token)
        {
            case 'PRIV':
                if (st.scopelevels.size() != 1 || isprivate)
                    Error("private must be used at file scope");
                
                lex.Next();
                return ParseTopExp(true);
                
            case 'INCL':
            {
                if (isprivate)
                    Error("include cannot be private");
                
                lex.Next();
                string fn = lex.sattr;
                Expect('STR');
                lex.Include((char *)fn.c_str());

                return ParseTopExp();
            }

            case 'VALU':
            case 'TYPE':
            {
                bool isvalue = lex.token == 'VALU';
                lex.Next();
                string sname = lex.sattr;
                Expect('ID');

                Struct &struc = st.StructDecl(sname, lex);
                struc.readonly = isvalue;
                struc.isprivate = isprivate;

                currentstruct = &struc;

                Expect(':');

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

                Expect('[');

                auto v = ParseVector(fieldid);

                for (auto ids = v; ids; ids = ids->b)
                {
                    assert (ids->a->type == 'FLD');
                    struc.fields.push_back(UniqueField(ids->a->exptype, ids->a->fld));
                }

                currentstruct = NULL;

                return new Node(lex, 'ADT', new Node(lex, &struc), v);
            }

            case 'FUNC':
            {
                lex.Next();
                return ParseFunctionDefinition(isprivate);
            }

            default:
            {
                if (IsNextId())
                {
                    auto idname = lastid;
                    bool dynscope = lex.token == '<-';
                    bool constant = lex.token == ':==';
                    bool logvar = lex.token == '?=';
                    if (lex.token == ':=' || dynscope || constant || logvar)  // codegen assumes these defs can only happen at toplevel
                    {
                        lex.Next();
                        auto e = ParseExp();
                        auto id = st.LookupLexDefOrDynScope(idname, lex.errorline, lex, dynscope, CurSF());
                        if (dynscope)  id->Assign(lex);
                        if (constant)  id->constant = true;
                        if (isprivate) id->isprivate = true;
                        if (logvar)  { id->logvaridx = 0; st.uses_frame_state = true; }
                        return new Node(lex, ':=', new Node(lex, id), e);
                    }
                    else if (IsNext(','))
                    {
                        bool isdef = false;
                        bool islogvar = false;
                        auto e = RecMultiDef(idname, isprivate, 1, isdef, islogvar);
                        if (e) return e;
                    }
                    else
                    {
                        lex.Undo('ID', idname);
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
        for (; n; n = n->b) c++;
        return c;
    }

    Node *ParseFunctionDefinition(bool isprivate = false)
    {
        string idname = lex.sattr;
        Expect('ID');

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
        for (auto n = args; n; n = n->b) n->a->ident->sf = sf;
        sf->body = ParseFunDefBody(args, &f);
        subfunctionstack.pop_back();
        functionstack.pop_back();

        for (auto stat = sf->body; stat; stat = stat->b)
            if (!stat->b && (stat->a->type != 'RET' || stat->a->b->integer != f.idx /* return from */))
                ReturnValues(f, 1);
        assert(f.retvals);

        sf->args = new Arg[nargs];
        int i = 0;
        for (auto a = args; a; a = a->b)
        {
            Arg &arg = sf->args[i++];
            arg.type = a->a->exptype;
            arg.flags = AF_NONE;
            arg.id = a->a->ident->name;
        }

        return new Node(lex, 'FUN', new Node(lex, &f), sf->body);
    }

    void ParseType(Type &dest, bool withtype)
    {
        switch(lex.token)
        {
            case 'INTT': dest.t = V_INT;       lex.Next(); break;
            case 'FLTT': dest.t = V_FLOAT;     lex.Next(); break;
            case 'STRT': dest.t = V_STRING;    lex.Next(); break;
            case 'CORO': dest.t = V_COROUTINE; lex.Next(); break;
            case 'NIL':  dest.t = V_NIL;       lex.Next(); break;
            case 'FUNC': dest.t = V_FUNCTION;  lex.Next(); break;
            case 'VECT': dest.t = V_VECTOR;    lex.Next(); break;

            case 'ID':
            {
                dest.t = V_VECTOR;
                dest.idx = st.StructUse(lex.sattr, lex).idx;
                lex.Next();
                break;
            }
            // FIXME more types
            default:
                Error("illegal type syntax: " + lex.TokStr());
        }

        if (withtype && dest.idx < 0) Error(":: must be used with a struct type");
    }

    Node *ParseFunArgs(bool coroutine, Node *derefarg, const char *fname = "", Arg *args = NULL, int nargs = -1)
    {
        if (derefarg)
        {
            CheckArg(nargs, 0, fname);
            if (IsNext('('))
            {
                return new Node(lex, ';', derefarg, ParseFunArgsRec(false, false, args, nargs, 1, fname));
            }
            else
            {
                return new Node(lex, ';', derefarg);
            }
        }
        else
        {
            Expect('(');
            return ParseFunArgsRec(coroutine, false, args, nargs, 0, fname);
        }
    }

    Node *ParseFunArgsRec(bool coroutine, bool needscomma, Arg *argdecls, int nargs, int thisarg, const char *fname)
    {
        if (IsNext(')'))
        {
            return ParseTrailingFunctionValues(coroutine, argdecls, nargs, thisarg, fname);
        }
        if (needscomma) Expect(',');
        Node *arg = ParseExp();

        CheckArg(nargs, thisarg, fname);
        if (argdecls && argdecls[thisarg].flags == NF_EXPFUNVAL)
            arg = new Node(lex, '{}', NULL, new Node(lex, ',', arg));

        return new Node(lex, ';', arg, ParseFunArgsRec(coroutine, true, argdecls, nargs, thisarg + 1, fname));
    }

    void CheckArg(int nargs, int thisarg, const char *fname)
    {
        if (thisarg == nargs) Error("too many arguments passed to function " + string(fname));
    }

    Node *ParseTrailingFunctionValues(bool coroutine, Arg *argdecls, int nargs, int thisarg, const char *fname)
    {
        if (argdecls && thisarg + 1 < nargs) trailingkeywordedfunctionvaluestack.push_back(argdecls[thisarg + 1].id);

        Node *e = NULL;
        switch (lex.token)
        {
            case ':':
                e = ParseFunDefBody(ParseFunDefArgs(false, false));
                break;

            case 'ID':
                if (trailingkeywordedfunctionvaluestack.empty() || trailingkeywordedfunctionvaluestack.back() != lex.sattr) // skip if this function value starts with an ID that's equal to the parents next keyworded function val ID, e.g. "else" in: if(..): currentcall(..) else: ..
                    e = ParseFunDefBody(ParseFunDefArgs(false));
                break;
            
            case '(':
                e = ParseFunDefBody(ParseFunDefArgs());
                break;
        }

        if (argdecls && thisarg + 1 < nargs) trailingkeywordedfunctionvaluestack.pop_back();

        if (!e)
        {
            if (coroutine) { e = new Node(lex, 'COCL'); coroutine = false; }
            else return NULL;
        }

        CheckArg(nargs, thisarg, fname);

        thisarg++;

        Node *tail = NULL;

        bool islf = lex.token == 'LF';
        if (argdecls && thisarg < nargs && (lex.token == 'ID' || islf))
        {
            if (islf) lex.Next();
            if (lex.token == 'ID' && argdecls[thisarg].id == lex.sattr)
            {
                lex.Next();
                tail = ParseTrailingFunctionValues(coroutine, argdecls, nargs, thisarg, fname);
            }
            else
            {
                lex.PushCur();
                if (islf) lex.Push('LF');
                lex.Next();
            }
        }

        return new Node(lex, ';', e, tail);
    }

    Node *ParseFunDefArgs(bool full = true, bool parseargs = true)
    {
        st.ScopeStart();
        if (full) Expect('(');

        Node *n = NULL;

        if (lex.token != ')' && parseargs)
        {
            Node **tail = &n;
            for (;;)
            {
                string a = lex.sattr;
                Expect('ID');

                auto id = new Node(lex, st.LookupLexDefOrDynScope(a, lex.errorline, lex, false, CurSF() /* this is good for function vals, but bad for named functions, which get corrected later */));

                bool withtype = lex.token == '::';
                if (full && (lex.token == ':' || withtype))
                {
                    lex.Next(); 

                    ParseType(id->exptype, withtype);

                    if (withtype) st.AddWithStruct(id->exptype, id->ident, lex);
                }

                *tail = new Node(lex, ';', id);
                tail = &(*tail)->b;

                if (!IsNext(',')) break;
            }
        }

        if (full) Expect(')');
        Expect(':');

        return n;
    }

    Node *ParseFunDefBody(Node *n, Function *f = NULL)  // continuation of ParseFunDefArgs(), n is its return value
    {
        size_t autoparlevel = autoparstack.size();
        Node *body = NULL;

        if (IsNext('IND'))
        {
            body = ParseStatements();
            Expect('DED');
        }
        else
        {
            body = new Node(lex, ',', ParseExpStat());
        }

        if (autoparlevel < autoparstack.size())
        {
            if (f) Error("cannot use anonymous argument: " + autoparstack[autoparlevel]->ident->name + ", in named function: " + f->name,       autoparstack[autoparlevel]);
            if (n) Error("cannot mix anonymous argument: " + autoparstack[autoparlevel]->ident->name + ", with declared arguments in function", autoparstack[autoparlevel]);

            auto ap = &n;
            for (size_t i = autoparlevel; i < autoparstack.size(); i++) 
            {
                for (size_t j = autoparlevel; j < i; j++) if (autoparstack[i]->ident == autoparstack[j]->ident) goto twice;
                *ap = new Node(lex, ';', new Node(lex, autoparstack[i]->ident));
                ap = &(*ap)->b;
                twice:;
            }
            while (autoparstack.size() > autoparlevel) autoparstack.pop_back();
        }

        n = new Node(lex, '{}', n, body);

        st.ScopeCleanup(lex);
        return n;
    }

    Node *ParseMultiRet(Node *first, int &nrv)
    {
        nrv = 1;
        if (lex.token != ',') return first;

        auto list = new Node(lex, 'MR', first, NULL);
        auto tail = &list->b;
        while (IsNext(','))
        {
            *tail = new Node(lex, 'MR', ParseOpExp(), NULL);
            tail = &(*tail)->b;
            nrv++;
        }

        return list;
    }

    void ReturnValues(Function &f, int nrv)
    {
        if (f.retvals && f.retvals != nrv)
            Error(string("all return statements of this function must return the same amount of return values. previously: ") + inttoa(f.retvals));
        f.retvals = nrv;
    }

    Node *ParseExpStat()
    {
        if (IsNext('RET'))
        {
            Node *rv = NULL;
            int nrv = 0;
            if (!Either('LF', 'DED', 'FROM'))
            {
                rv = ParseMultiRet(ParseOpExp(), nrv);
                if (rv->type == 'CALL')
                    nrv = max(nrv, rv->a->f->retvals);
            }

            int fid = -2;
            if (IsNext('FROM'))
            {
                if(!IsNext('PROG'))
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

            return new Node(lex, 'RET', rv, new Node(lex, 'INT', fid));
        }

        auto e = ParseExp();

        while (IsNext(';'))
        {
            if (IsNext('LF'))
                Error("\';\' is not a statement terminator"); // specialized error for all the C-style language users
            e = new Node(lex, 'BUT', e, ParseExp());
        }

        return e;
    }
    
    void Modify(Node *e)
    {
        if (e->type == 'ID')
            e->ident->Assign(lex);
    }

    Node *ParseExp()
    {
        auto e = ParseOpExp();

        switch (lex.token)
        {
            case '=':  
            case '+=': 
            case '-=': 
            case '*=': 
            case '/=':
            case '%=':
            {
                auto type = lex.token;
                if (e->type != 'ID' && e->type != '.' && e->type != '.@' && e->type != 'IDX') Error("illegal left hand side of assignment");
                Modify(e);
                lex.Next();
                e = new Node(lex, type, e, ParseExp());
            }
        }

        return e;
    }

    Node *ParseOpExp(uint level = 4)
    {
        static int ops[][4] =
        {
            { '*',  '/',  '%',  0    },
            { '+',  '-',  0,    0    },
            { '<',  '>',  '>=', '<=' },
            { '==', '!=', 0,    0    },
            { '&',  '|',  0,    0    },
        };

        Node *exp = level ? ParseOpExp(level - 1) : ParseUnary();

        int *o = &ops[level][0];
        while (Either(o[0], o[1]) || Either(o[2], o[3]))
        {
            int op = lex.token;
            lex.Next();
            exp = new Node(lex, op, exp, level ? ParseOpExp(level - 1) : ParseUnary());
        }

        return exp;
    }

    Node *ParseUnary()
    {
        switch (lex.token)
        {
            case '-': 
            //case '~': 
            case '!':
            case '++':
            case '--':
            {
                auto t = lex.token;
                lex.Next();
                auto e = ParseUnary();
                if (t == '++' || t == '--') Modify(e);
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
                        *ai = new Node(lex, ';', new Node(lex, 'NIL'));   
                    }
                    else
                    {
                        // FIXME: here we can look if "ids" exists in this scope, and substitute it
                        Error("missing arg to builtin function: " + idname); 
                    }
                }
                ai = &(*ai)->b;
            }
            return new Node(lex, 'NATC', new Node(lex, nf), args);
        }

        if (f)
        {
            auto bestf = f;
            for (auto fi = f->sibf; fi; fi = fi->sibf) 
                if (fi->nargs > bestf->nargs) bestf = fi;

            auto args = ParseFunArgs(coroutine, firstarg, idname.c_str(),  bestf->subf->args, bestf->nargs);
            auto nargs = CountList(args);

            f = FindFunctionWithNargs(f, nargs, idname, NULL);

            return new Node(lex, 'CALL', new Node(lex, f), args);
        }

        auto args = ParseFunArgs(coroutine, firstarg);
        auto id = st.LookupLexMaybe(idname);
        if (id)
            return new Node(lex, '->', new Node(lex, id), args);

        auto n = new Node(lex, 'CALL', new Node(lex, (Function *)NULL), args);
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
        return NULL;
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
                    ffc->n->a->f = FindFunctionWithNargs(f, CountList(ffc->n->b), ffc->idname, ffc->n);
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
            case '.':
            {
                lex.Next();
                string idname = lex.sattr;
                Expect('ID');
                if (IsNext('@'))
                {
                    string fname = lex.sattr;
                    Expect('ID');
                    if (!st.FindFunction(fname)) // TODO: this is here because currently these functions have to be declared before use
                        Error(string("function ") + fname + " hasn't been declared yet");
                    auto id = st.LookupIdentInFun(idname, fname);
                    if (!id)    
                        Error(string("no unique local variable ") + idname + " in function with name " + fname);
                    n = new Node(lex, '.@', n, new Node(lex, id));
                }
                else
                {
                    SharedField *fld = st.FieldUse(idname);
                    if (fld) 
                    {
                        n = new Node(lex, '.', n, new Node(lex, fld));
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

            case '[':
            {
                lex.Next();
                n = new Node(lex, 'IDX', n, ParseExp());
                Expect(']');
                break;
            }

            case '++':
            case '--':
                Modify(n);
                n = new Node(lex, lex.token == '++' ? '+++' : '---', n);
                lex.Next();
                return n;

            case '(':   // only for dyn calls
            {
                auto args = ParseFunArgs(false, NULL);
                n = new Node(lex, '->', n, args);
                break;
            }

            case 'IS':
            {
                lex.Next();
                ParseType(n->exptype, false);
                return new Node(lex, 'IS', n);
            }

            default:
                return n;
        }
    }

    Node *ParseFactor()
    {
        switch (lex.token)
        {
            case 'INT': { int i    = atoi(lex.sattr.c_str()); lex.Next(); return new Node(lex, 'INT', i); }
            case 'FLT': { double f = atof(lex.sattr.c_str()); lex.Next(); return new Node(lex, 'FLT', f); }
            case 'STR': { string s = lex.sattr;               lex.Next(); return new Node(lex, 'STR', s); }     
            case 'NIL': {                                     lex.Next(); return new Node(lex, 'NIL');    }

            case '(':
            {
                lex.Next();
                auto n = ParseExp();
                Expect(')');
                return n;
            }

            case '[':
            {
                lex.Next();

                Node *n;
                if (IsNext('SUP'))
                {
                    n = new Node(lex, ',', new Node(lex, 'SUP', ParseExp()));
                    if (IsNext(',')) n->b = ParseVector();
                    else Expect(']');
                }
                else
                {
                    n = ParseVector();
                }

                n = new Node(lex, '[]', n);

                if (IsNext(':'))
                {
                    ParseType(n->exptype, false);
                    if (n->exptype.idx >= 0)
                    {
                        auto struc = st.structtable[n->exptype.idx];
                        int nargs = CountList(n->a);
                        int reqargs = struc->fields.size();
                        if (n->a && n->a->a->type == 'SUP') reqargs -= struc->superclass->fields.size() - 1;
                        if (nargs != reqargs) Error("constructor requires " + string(inttoa(reqargs)) + " arguments, not " + string(inttoa(nargs)));
                    }
                    else
                    {
                        if (n->exptype.t == V_VECTOR) Error("vector type annotation superfluous");
                    }
                }
                return n;
            }

            case 'FUNC':
            {
                lex.Next();
                return ParseFunDefBody(ParseFunDefArgs());
            }

            case 'CORO':
            {
                lex.Next();
                string idname = lex.sattr;
                Expect('ID');
                return new Node(lex, 'CORO', ParseFunctionCall(st.FindFunction(idname), natreg.FindNative(idname), idname, NULL, true));
            }

            case 'ID':
            {
                string idname = lex.sattr;
                lex.Next();

                switch (lex.token)
                {
                    case '(':
                        return ParseFunctionCall(st.FindFunction(idname), natreg.FindNative(idname), idname, NULL, false);

                    default:
                        if (idname[0] == '_')
                        {
                            auto dest = new Node(lex, st.LookupLexDefOrDynScope(idname, lex.errorline, lex, true, CurSF()));
                            autoparstack.push_back(dest);
                            return dest;
                        }
                        else
                        {
                            Ident *id = NULL;
                            auto fld = st.LookupWithStruct(idname, lex, id);
                            if (fld)
                            {
                                return new Node(lex, '.', new Node(lex, id), new Node(lex, fld));
                            }

                            return new Node(lex, st.LookupLexUse(idname, lex));
                        }
                    }
            }

            default:
                Error("illegal start of expression: " + lex.TokStr());
                return NULL;
        }
    }

    bool IsNext(int t)
    {
        bool isnext = lex.token == t;
        if (isnext) lex.Next();
        return isnext;
    }

    string lastid;

    bool IsNextId()
    {
        if (lex.token != 'ID') return false;
        lastid = lex.sattr;
        lex.Next();
        return true;
    }

    bool Either(int t1, int t2)         { return lex.token == t1 || lex.token == t2; }
    bool Either(int t1, int t2, int t3) { return lex.token == t1 || lex.token == t2 || lex.token == t3; }

    void Expect(int t)
    {
        if (!IsNext(t))
            Error(lex.TokStr(t) + " expected, found: " + lex.TokStr());
    }

    string Dump() { return root->Dump(0, lex); }
};
