

struct Parser
{
    Lex lex;
    Node *root;
    static Lex *nodelex;
    SymbolTable &st;
    vector<Ident *> autoparstack;
    vector<Ident *> maybeundefined;
    vector<Function *> functionstack;

    Parser(const char *_src, SymbolTable &_st) : st(_st), lex(_src, _st), root(NULL)
    {
        nodelex = &lex;
    }
    
    ~Parser()
    {
        nodelex = NULL;
        delete root;
    }

    void Parse()
    {
        st.ScopeStart();
        root = ParseList(true);
        st.ScopeCleanup(lex);
        Expect('EOF');
        if (autoparstack.size()) lex.Error("implicit _ argument(s) used at top level");
    }

    Node *ParseListRec(bool toplevel, int fieldid = -1)       // FIXME: make this non recursive in case of large files
    {
        Node *a = fieldid < 0 ? (toplevel ? ParseTopExp() : ParseExpStat())
                              : ParseField(fieldid++);
        Node *b = NULL;
        if (lex.token == ',')
        {
            lex.Next();
            b = ParseListRec(toplevel, fieldid);
        }
        if (lex.token == 'LF')
        {
            lex.Next();
            if (lex.token != 'EOF' && lex.token != 'DED' && lex.token != ']') b = ParseListRec(toplevel, fieldid);
        }
        st.UnregisterNode(a, lex);
        return new Node(',', a, b);
    }

    Node *ParseList(bool toplevel, int fieldid = -1)
    {
        if (lex.token == 'IND')
        {
            lex.Next();
            auto l = ParseListRec(toplevel, fieldid);
            Expect('DED');
            Expect('LF');
            return l;
        }
        return ParseListRec(toplevel, fieldid);
    }

    Node *ParseExpList(char term, int fieldid = -1)
    {
        if (term != ')' && lex.token == term) { lex.Next(); return NULL; }
        auto l = ParseList(false, fieldid);
        Expect(term);
        return l;
    }

    Node *ParseField(int idx)
    {
        string fname = lex.sattr;
        Expect('ID');
        return new Node(&st.FieldDecl(fname, idx, lex));
    }

    Node *ParseTopExp()
    {   
        switch(lex.token)
        {
            case 'INCL':
            {
                lex.Next();
                string fn = lex.sattr;
                Expect('STR');
                lex.Include((char *)fn.c_str());

                return ParseTopExp();
            }

            case 'TYPE':
            {
                lex.Next();
                string sname = lex.sattr;
                Expect('ID');

                Struct &struc = st.StructDecl(sname, lex);

                Expect(':');

                int fieldid = 0;

                if (lex.token == 'ID')
                {
                    auto &base = st.StructUse(lex.sattr, lex);
                    struc.superclass = &base;
                    struc.superclassidx = base.idx;

                    lex.Next();

                    for_all(fld, base.fields, struc.fields.push_back(fld));

                    fieldid = base.fields.size();
                }

                Expect('[');

                auto v = ParseExpList(']', fieldid);

                for (auto ids = v; ids; ids = ids->b)
                {
                    assert (ids->a->type == 'FLD');
                    struc.fields.push_back(ids->a->fld);
                }
                return new Node('ADT', new Node(&struc), v);
            }

            case 'FUNC':
            {
                lex.Next();
                return ParseFunctionDefinition();
            }

            default:
            {
                if (lex.token == 'ID')
                {
                    auto idname = lex.sattr;
                    lex.Next();
                    bool dynscope = lex.token == '<-';
                    if (lex.token == ':=' || dynscope)
                    {
                        lex.Next();
                        auto e = ParseExp();
                        auto id = st.LookupLexDefOrDynScope(idname, lex.errorline, lex, dynscope);
                        return new Node(':=', new Node(id), e);
                    }
                    else
                    {
                        lex.Undo('ID', idname);
                    }
                }
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

    Node *ParseFunctionDefinition()
    {
        string idname = lex.sattr;
        Expect('ID');

        if (st.FindNative(idname))
            lex.Error("cannot override built-in function: " + idname);

        auto f = st.FindFunction(idname);

        if (f)  lex.Error("double declaration of function: " + idname); // FIXME test, also double of what happens in FunctionDecl below

        f = &st.FunctionDecl(idname, lex);

        functionstack.push_back(f);
        f->body = ParseFunRest(f);
        functionstack.pop_back();

        auto args = f->body->a;

        f->args = new Arg[f->nargs];
        int i = 0;
        for (auto a = args; a; a = a->b)
        {
            Arg &arg = f->args[i++];
            arg.type = a->a->exptype;
            arg.flags = AF_NONE;
            arg.id = a->a->ident->name;
        }

        return new Node('FUN', new Node(f), f->body);
    }

    void ParseType(Type &dest, bool withtype)
    {
        switch(lex.token)
        {
            case 'INTT': dest.t = T_INT;    lex.Next(); break;
            case 'FLTT': dest.t = T_FLOAT;  lex.Next(); break;
            case 'STRT': dest.t = T_STRING; lex.Next(); break;

            case 'ID':
            {
                dest.t = T_STRUCT;
                dest.udt = &st.StructUse(lex.sattr, lex);
                lex.Next();
                break;
            }
            // FIXME more types
            default:
                lex.Error("illegal type syntax: " + lex.TokStr());
        }

        if (withtype && dest.t != T_STRUCT) lex.Error(":: must be used with a struct type");
    }

    Node *ParseFunArgs(Node *derefarg, const char *fname = "", Arg *args = NULL, int nargs = -1)
    {
        if (derefarg)
        {
            CheckArg(nargs, 0, fname);
            if (lex.token == '(')
            {
                lex.Next();
                return new Node(';', derefarg, ParseFunArgsRec(false, args, nargs, 1, fname));
            }
            else
            {
                return new Node(';', derefarg);
            }
        }
        else
        {
            Expect('(');
            return ParseFunArgsRec(false, args, nargs, 0, fname);
        }
    }

    Node *ParseFunArgsRec(bool needscomma, Arg *argdecls, int nargs, int thisarg, const char *fname)
    {
        if (lex.token == ')')
        {
            lex.Next();
            return ParseTrailingClosures(argdecls, nargs, thisarg, fname);
        }
        if (needscomma) Expect(',');
        Node *arg = ParseExp();

        CheckArg(nargs, thisarg, fname);
        if (argdecls && argdecls[thisarg].flags == NF_EXPCLOSURE) arg = new Node('{}', NULL, new Node(',', arg));

        return new Node(';', arg, ParseFunArgsRec(true, argdecls, nargs, thisarg + 1, fname));
    }

    void CheckArg(int nargs, int thisarg, const char *fname)
    {
        if (thisarg == nargs) lex.Error("too many arguments passed to function " + string(fname));
    }

    Node *ParseTrailingClosures(Arg *argdecls, int nargs, int thisarg, const char *fname)
    {
        Node *e = NULL;
        switch (lex.token)
        {
            case ':':
                e = ParseFunRest(NULL, false, false);
                break;

            case 'ID':
                e = ParseFunRest(NULL, false);
                break;
            
            case '(':
                e = ParseFunRest(NULL);
                break;

            default:
                return NULL;
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
                tail = ParseTrailingClosures(argdecls, nargs, thisarg, fname);
            }
            else
            {
                lex.PushCur();
                if (islf) lex.Push('LF');
                lex.Next();
            }
        }

        return new Node(';', e, tail);
    }

    Node *ParseFunRest(Function *f = NULL, bool full = true, bool parseargs = true)
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

                auto id = new Node(st.LookupLexDefOrDynScope(a, lex.errorline, lex, false));

                bool withtype = lex.token == '::';
                if (full && (lex.token == ':' || withtype))
                {
                    lex.Next(); 

                    ParseType(id->exptype, withtype);

                    if (withtype) st.AddWithStruct(id, lex);
                }

                *tail = new Node(';', id);
                tail = &(*tail)->b;

                if (lex.token != ',') break;

                lex.Next();
            }
        }

        if (full) Expect(')');
        Expect(':');

        if (f) f->nargs = CountList(n);

        int autoparlevel = autoparstack.size();
        Node *body = NULL;

        if (lex.token == 'IND')
        {
            lex.Next();
            body = ParseListRec(true);
            Expect('DED');
        }
        else
        {
            body = new Node(',', ParseExpStat());
        }

        auto ap = &n;
        for (int i = autoparlevel; i < (int)autoparstack.size(); i++) 
        {
            for (int j = autoparlevel; j < i; j++) if (autoparstack[i] == autoparstack[j]) goto twice;
            *ap = new Node(';', new Node(autoparstack[i]));
            ap = &(*ap)->b;
            twice:;
        }
        while ((int)autoparstack.size() > autoparlevel) autoparstack.pop_back();

        n = new Node('{}', n, body);

        st.ScopeCleanup(lex);
        return n;
    }

    Node *ParseExpStat()
    {
        if (lex.token == 'RET')
        {
            lex.Next();
            return new Node('RET', lex.token == 'LF' || lex.token == 'DED' ? NULL : ParseOpExp(), new Node(functionstack.size() ? functionstack.back() : NULL));
        }

        return ParseExp();
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
            {
                auto type = lex.token;
                if (e->type != 'ID' && e->type != '.' && e->type != 'IDX') lex.Error("illegal left hand side of assignment");
                lex.Next();
                return new Node(type, e, ParseExp());
            }

            default:
                return e;
        }
    }

    Node *ParseOpExp(unsigned int level = 4)
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
        while (lex.token == o[0] || lex.token == o[1] || lex.token == o[2] || lex.token == o[3])
        {
            int op = lex.token;
            lex.Next();
            exp = new Node(op, exp, level ? ParseOpExp(level - 1) : ParseUnary());
        }

        return exp;
    }

    Node *ParseUnary()
    {
        switch (lex.token)
        {
            case '-': 
            case '~': 
            case '!':
            case '++':
            case '--':
            {
                auto t = lex.token;
                lex.Next();
                return new Node(t, ParseDeref());
            }

            default:
                return ParseDeref();
        }
    }

    Node *ParseFunctionCall(Function *f, NativeFun *nf, string &idname, Node *firstarg)
    {
        auto args = nf ?      ParseFunArgs(firstarg, idname.c_str(), nf->args, nf->nargs)
                       : (f ? ParseFunArgs(firstarg, idname.c_str(),  f->args,  f->nargs)
                            : ParseFunArgs(firstarg));

        if (nf)
        {
            Node **ai = &args;
            for(int i = 0; i < nf->nargs; i++)
            {
                if (!*ai)
                {
                    if (nf->args[i].flags == NF_OPTIONAL)
                    {
                        *ai = new Node(';', new Node('NIL'));   
                    }
                    else
                    {
                        // FIXME: here we can look if "ids" exists in this scope, and substitute it
                        lex.Error("missing arg to builtin function: " + idname); 
                    }
                }
                ai = &(*ai)->b;
            }
            return new Node('NATC', new Node(nf), args);
        }

        if (f) return new Node('CALL', new Node(f), args);

        auto id = st.LookupLexUse(idname, lex);  // FIXME: it could be calling a function yet to be declared
        return new Node('->', new Node(id), args);
    }

    Node *ParseDeref()
    {
        auto n = ParseFactor();

        for(;;) switch (lex.token)
        {
            case '.':
            {
                lex.Next();
                string idname = lex.sattr;
                Expect('ID');
                Field *fld = st.FieldUse(idname);
                if (fld) 
                {
                    n = new Node('.', n, new Node(fld));
                }
                else
                {
                    auto f = st.FindFunction(idname);
                    auto nf = st.FindNative(idname);
                    if (f || nf)
                    {
                        n = ParseFunctionCall(f, nf, idname, n);
                    }
                    else
                    {
                        lex.Error("not a type member or function: " + idname);
                    }
                }
                break;
            }

            case '[':
            {
                lex.Next();
                n = new Node('IDX', n, ParseExp());
                Expect(']');
                break;
            }

            case '++':
            case '--':
                n = new Node(lex.token == '++' ? '+++' : '---', n);
                lex.Next();
                return n;

            case '(':   // only for dyn calls
            {
                auto args = ParseFunArgs(NULL);
                n = new Node('->', n, args);
                break;
            }

            default:
                return n;
        }
    }

    Node *ParseFactor()
    {
        switch (lex.token)
        {
            case 'INT': { int i    = atoi(lex.sattr.c_str()); lex.Next(); return new Node('INT', i); }
            case 'FLT': { double f = atof(lex.sattr.c_str()); lex.Next(); return new Node('FLT', f); }
            case 'STR': { string s = lex.sattr;               lex.Next(); return new Node('STR', s); }     
            case 'NIL': {                                     lex.Next(); return new Node('NIL');    }

            case '(':
            {
                lex.Next();
                auto n = ParseExpList(')');
                if (!n->b) { auto na = n->a; n->a = NULL; delete n; return na; };     // reduce node clutter
                return new Node('EXPS', n);
            }

            case '[':
            {
                lex.Next();
                auto n = new Node('[]', ParseExpList(']'));
                if(lex.token == ':')
                {
                    lex.Next();
                    string sname = lex.sattr;
                    Expect('ID');
                    Struct &struc = st.StructUse(sname, lex);
                    n->b = new Node(&struc);
                    int nargs = 0;
                    for (auto nl = n->a; nl; nl = nl->b) nargs++;
                    int reqargs = struc.fields.size();
                    if (struc.superclass) reqargs -= struc.superclass->fields.size() - 1;
                    if (nargs != reqargs) lex.Error("constructor requires " + string(inttoa(reqargs)) + " arguments, not " + string(inttoa(nargs)));
                }
                return n;
            }

            case 'FUNC':
            {
                lex.Next();
                return ParseFunRest(NULL);
            }

            case 'ID':
            {
                string idname = lex.sattr;
                lex.Next();

                switch (lex.token)
                {
                    case '(':
                        return ParseFunctionCall(st.FindFunction(idname), st.FindNative(idname), idname, NULL);

                    default:
                        if (idname[0] == '_')
                        {
                            auto dest = new Node(st.LookupLexDefOrDynScope(idname, lex.errorline, lex, true));
                            autoparstack.push_back(dest->ident);
                            return dest;
                        }
                        else
                        {
                            Ident *id = NULL;
                            auto fld = st.LookupWithStruct(idname, lex, id);
                            if (fld)
                            {
                                return new Node('.', new Node(id), new Node(fld));
                            }

                            return new Node(st.LookupLexUse(idname, lex));
                        }
                    }
            }

            default:
                lex.Error("illegal start of expression: " + lex.TokStr());
                return NULL;
        }
    }

    void Expect(int t)
    {
        if (lex.token != t)
            lex.Error(lex.TokStr(t) + " expected, found: " + lex.TokStr());

        lex.Next();
    }

    string Dump() { return root->Dump(0, lex); }
};