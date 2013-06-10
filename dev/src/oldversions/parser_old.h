

struct Parser
{
    Lex lex;
    Node *root;
    static Lex *nodelex;
    SymbolTable &st;
    vector<Ident *> autoparstack;
    vector<Ident *> maybeundefined;
    vector<Function *> functionstack;
    vector<string> trailingkeywordedclosurestack;

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
        Node *a = fieldid < 0 ? (toplevel ? ParseTopExp() : ParseExp())
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

    Node *ParseBody(Node *args)
    {
        int autoparlevel = autoparstack.size();
        Node *body = NULL;

        if (lex.token == '{')
        {
            lex.Next();
            if (lex.token != '}') body = ParseList(true);
            Expect('}');
        }
        else if (lex.token == 'IND')
        {
            lex.Next();
            body = ParseListRec(true);
            Expect('DED');
        }
        else
        {
            body = new Node(',', ParseExp());
        }

        auto ap = &args;
        for (int i = autoparlevel; i < (int)autoparstack.size(); i++) 
        {
            for (int j = autoparlevel; j < i; j++) if (autoparstack[i] == autoparstack[j]) goto twice;
            *ap = new Node(';', new Node(autoparstack[i]));
            ap = &(*ap)->b;
            twice:;
        }
        while ((int)autoparstack.size() > autoparlevel) autoparstack.pop_back();

        return args ? new Node('{}', args, body)
                    : new Node('{}', body);
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
        if (lex.token == 'INCL')
        {
            lex.Next();
            string fn = lex.sattr;
            Expect('STR');
            lex.Include((char *)fn.c_str());

            return ParseTopExp();
        }
        else if (lex.token == 'TYPE')
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
        else
        {
            return ParseExp();
        }
    }

    Node *ParseExp()
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
            default:   return e;
        }
    }

    Node *ParseFunArgs(Node *derefarg, const char *fname = "", Arg *args = NULL, int nargs = -1)
    {
        if (derefarg)
        {
            derefarg = ProcessArg(derefarg, args, nargs, 0, fname);
            if (lex.token == '(')
            {
                lex.Next();
                return new Node(';', derefarg, ParseFunArgsRec(false, args, nargs, 1, fname, false));
            }
            else
            {
                return new Node(';', derefarg);
                //return new Node(';', derefarg, ParseTrailingClosures(true, args, nargs, 1, fname, false));
            }
        }
        else
        {
            Expect('(');
            return ParseFunArgsRec(false, args, nargs, 0, fname, !args);
        }
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

    Node *ParseFunArgsRec(bool needscomma, Arg *argdecls, int nargs, int thisarg, const char *fname, bool canstillbefunctiondef)
    {
        if (lex.token == ')')
        {
            lex.Next();
            return ParseTrailingClosures(true, argdecls, nargs, thisarg, fname, false);
        }
        if (needscomma) Expect(',');
        Node *arg = NULL;
        if (canstillbefunctiondef && lex.token == 'ID')
        {
            auto idname = lex.sattr;
            lex.Next();
            bool withtype = lex.token == '::'; 
            if (lex.token == ':' || withtype)
            {
                arg = new Node('ID?', idname);
                lex.Next();
                ParseType(arg->exptype, withtype);
                arg->extra1 = withtype;
            }
            else if (lex.token == ',' || lex.token == ')')
            {
                arg = new Node('ID?', idname);
            }
            else
            {
                canstillbefunctiondef = false;
                lex.Undo('ID', idname);
            }
        }
        if (!arg) arg = ParseExp();
        auto e = ProcessArg(arg, argdecls, nargs, thisarg, fname);
        return new Node(';', e, ParseFunArgsRec(true, argdecls, nargs, thisarg + 1, fname, canstillbefunctiondef));
    }

    Node *ProcessArg(Node *e, Arg *argdecls, int nargs, int thisarg, const char *fname)
    {
        CheckArg(nargs, thisarg, fname);
        if (argdecls && argdecls[thisarg].flags == NF_EXPCLOSURE) e = new Node('{}', new Node(',', e));
        return e;
    }

    void CheckArg(int nargs, int thisarg, const char *fname)
    {
        if (thisarg == nargs) lex.Error("too many arguments passed to function " + string(fname));
    }

    Node *ParseTrailingClosures(bool firstclosure, Arg *argdecls, int nargs, int thisarg, const char *fname, bool keyworded)
    {
        st.ScopeStart();
        Node *args = NULL;
        switch (lex.token)
        {
            case '{':
            case 'IND':
            {
                CheckArg(nargs, thisarg, fname);
                break;
            }
            case 'ID':
            {
                ClosureParameters(&args);
                if (args)
                {
                    CheckArg(nargs, thisarg, fname);
                    break;
                }
                if (!keyworded && !firstclosure && argdecls && thisarg < nargs && argdecls[thisarg].id == lex.sattr)
                {
                    lex.Next();
                    st.ScopeCleanup(lex);
                    return ParseTrailingClosures(false, argdecls, nargs, thisarg, fname, true);
                }
                if (trailingkeywordedclosurestack.size() && trailingkeywordedclosurestack.back() == lex.sattr)
                {
                    // this exp starts with an ID that's equal to the parents next keyworded closure ID, do not regard it as the start of an exp
                    // e.g. "else" in: if(..) currentcall(..) else ..
                    st.ScopeCleanup(lex);
                    return NULL;
                }
                // fall thru (!)
            }
            case '[':       // FIXME: not a great solution, needs to be extended with other tokens that can start an exp, but never appear directly after ')'
            case '(':
            case 'INT':
            case 'FLT':
            case 'STR':
            case 'NIL':
            case 'RET':
            case '!':
            {
                CheckArg(nargs, thisarg, fname);
                break;
            }
            /*
            case '-':   // this is a problematic case :(
            {
                if (keyworded || argdecls[thisarg].type.t == T_CLOSURE)     // this helps with builtins and typed functions only
                {
                    CheckArg(nargs, thisarg, fname);
                    break;
                }
                else
                {
                    st.ScopeCleanup(lex);
                    return NULL;
                }
            }
            */
            case 'LF':
                if (!keyworded && !firstclosure && argdecls && thisarg < nargs)
                {
                    lex.Next();
                    if (lex.token != 'ID' || argdecls[thisarg].id != lex.sattr)
                    {
                        lex.Undo('LF');
                    }
                    else
                    {
                        lex.Next();
                        st.ScopeCleanup(lex);
                        return ParseTrailingClosures(false, argdecls, nargs, thisarg, fname, true);
                    }
                }
                // fall thru (!)
            default:
                // this helps with builtins and typed functions only, for '-' and any other tokens that are not in the exp start tokens list above
                if (keyworded || (argdecls && thisarg < nargs && argdecls[thisarg].type.t == T_CLOSURE && argdecls[thisarg].flags != NF_OPTIONAL))
                {
                    CheckArg(nargs, thisarg, fname);
                    break;
                }
                else
                {
                    st.ScopeCleanup(lex);
                    return NULL;
                }
                /*
                if (keyworded) lex.Error(string("block expected after block identifier: ") + argdecls[thisarg].id);
                st.ScopeCleanup(lex);
                return NULL;
                */

        }

        if (argdecls && thisarg + 1 < nargs) trailingkeywordedclosurestack.push_back(argdecls[thisarg + 1].id); // see above

        auto e = ParseBody(args);

        if (argdecls && thisarg + 1 < nargs) trailingkeywordedclosurestack.pop_back();

        st.ScopeCleanup(lex);
        return new Node(';', e, ParseTrailingClosures(false, argdecls, nargs, thisarg + 1, fname, false));
    }

    void ClosureParameters(Node **tail)
    {
        for (;;)
        {
            if (lex.token != 'ID') break;

            string a = lex.sattr;
            lex.Next();
            bool withtype = lex.token == '::';
            if (lex.token != ':' && !withtype)
            {
                lex.Undo('ID', a);
                break;
            }
            lex.Next(); 

            auto id = new Node(st.LookupLexDefOrDynScope(a, lex.errorline, lex, false));

            if (withtype)
            {
                ParseType(id->exptype, true);
                st.AddWithStruct(id, lex);
            }

            *tail = new Node(';', id);
            tail = &(*tail)->b;
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
    
    void DefineFunArgs(Node *args)
    {
        for (auto a = args; a; a = a->b)
        {
            if (a->a->type == 'ID?')
            {
                const char *name = a->a->str;
                MakeIdNode(name, a->a, lex.token == ':');
                pool->dealloc_string(name);
            }
        }
    }

    Node *ParseFunctionCall(Function *f, NativeFun *nf, string &idname, Node *firstarg)
    {
        auto args = nf ?      ParseFunArgs(firstarg, idname.c_str(), nf->args, nf->nargs)
                       : (f ? ParseFunArgs(firstarg, idname.c_str(),  f->args,  f->nargs)
                            : ParseFunArgs(firstarg));
        
        st.ScopeStart();
        DefineFunArgs(args);

        if (lex.token == ':')
        {		
            if (nf) lex.Error("cannot override built-in function: " + idname);
            if (f)  lex.Error("double declaration of function: " + idname); // FIXME test
            int nargs = 0;
            for (auto a = args; a; a = a->b)
            {
                nargs++;
                a->type = ',';
                if (a->a->type != 'ID') lex.Error("illegal function declaration parameter type");
            }
            lex.Next();
            f = &st.FunctionDecl(idname, lex);
            f->nargs = nargs;
            f->args = new Arg[nargs];
            int i = 0;
            for (auto a = args; a; a = a->b)
            {
                Arg &arg = f->args[i++];
                arg.type = a->a->exptype;
                arg.flags = AF_NONE;
                if (a->a->extra1)
                {
                    //arg.flags = AF_WITHTYPE;
                    a->a->extra1 = 0;
                    st.AddWithStruct(a->a, lex);
                }
                arg.id = a->a->ident->name;
            }
            functionstack.push_back(f);
            auto n = new Node('FUN', new Node(f), ParseBody(args));
            functionstack.pop_back();
            st.ScopeCleanup(lex);
            return n;
        }
        
        st.ScopeCleanup(lex);

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
            return ParseDeref(new Node('NATC', new Node(nf), args));
        }

        if (f) return ParseDeref(new Node('CALL', new Node(f), args));

        auto id = st.LookupLexUse(idname, lex);  // FIXME: it could be calling a function yet to be declared
        return ParseDeref(new Node('->', new Node(id), args));
    }

    Node *ParseDeref(Node *n)
    {
        switch (lex.token)
        {
            case '.':
            {
                lex.Next();
                string idname = lex.sattr;
                Expect('ID');
                Field *fld = st.FieldUse(idname);
                if (fld) 
                {
                    n = ParseDeref(new Node('.', n, new Node(fld)));
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
                n = ParseDeref(n);
                break;
            }
            case '++':
            case '--':
                n = new Node(lex.token == '++' ? '+++' : '---', n);
                lex.Next();
                break;
            case '(':   // only for dyn calls
            {
                auto args = ParseFunArgs(NULL);
                DefineFunArgs(args);
                n = new Node('->', n, args);
                break;
            }
        }
        return n;
    }

    Node *ParseDeref()
    {
        if (lex.token != 'ID') return ParseDeref(ParseFactor());

        string idname = lex.sattr;
        lex.Next();

        switch (lex.token)
        {
            case ':':
            {
                st.ScopeStart();
                auto n = new Node(st.LookupLexDefOrDynScope(idname, lex.errorline, lex, false));
                lex.Next();
                n = new Node(';', n);
                ClosureParameters(&n->b);

                auto b = ParseBody(n);

                st.ScopeCleanup(lex);
                return b;
            }
            case '(':
                return ParseFunctionCall(st.FindFunction(idname), st.FindNative(idname), idname, NULL);

            default:
                return ParseDeref(MakeIdNode(idname, new Node(0)));
        }
    }
    
    Node *MakeIdNode(const string &idname, Node *dest, bool forcedefine = false)
    {
        Ident *id = NULL;
        if (forcedefine)
        {
            id = st.LookupLexDefOrDynScope(idname, dest->linenumber, lex, false);
        }
        else if (idname[0] == '_')
        {
            id = st.LookupLexDefOrDynScope(idname, lex.errorline, lex, true);
            autoparstack.push_back(id);
        }
        else
        {
            auto fld = st.LookupWithStruct(idname, lex, id);
            if (fld)
            {
                dest->type = '.';
                dest->a = new Node(id);
                dest->b = new Node(fld);
                return dest;
            }

            id = st.LookupLexUse(idname, lex);
        }
        dest->ident = id;
        dest->type = 'ID';
        return dest;
    }

    Node *ParseFactor()
    {
        switch (lex.token)
        {
            case 'INT': { int i    = atoi(lex.sattr.c_str()); lex.Next(); return new Node('INT', i); }
            case 'FLT': { double f = atof(lex.sattr.c_str()); lex.Next(); return new Node('FLT', f); }
            case 'STR': { string s = lex.sattr;               lex.Next(); return new Node('STR', s); }     
            case 'NIL': {                                     lex.Next(); return new Node('NIL');    }

            case '{':
                return ParseBody(NULL);

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

            case 'RET':     // FIXME: not a great location
                lex.Next();
                return new Node('RET', ParseOpExp(), new Node(functionstack.size() ? functionstack.back() : NULL));

            case 'ID':
                assert(0);

            default:
                lex.Error("illegal start of expression: " + lex.TokStr());
                return NULL;
        }
    }

    void Expect(int t)
    {
        if (lex.token != t) lex.Error(lex.TokStr(t) + " expected, found: " + lex.TokStr());
        lex.Next();
    }

    string Dump() { return root->Dump(0, lex); }


};