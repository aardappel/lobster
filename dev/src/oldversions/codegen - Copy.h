
#define ILNAMES \
    F(PUSHINT) \
    F(PUSHFLT) \
    F(PUSHSTR) \
    F(PUSHUNDEF) \
    F(PUSHNIL) \
    F(PUSHFUN) \
    F(PUSHVAR) F(PLUSVAR) F(PLUSVARR) F(MULVAR) F(MULVARR) F(SUBVAR) F(SUBVARR) F(DIVVAR) F(DIVVARR) F(WRITEVAR) F(WRITEVARR) F(PPVAR) F(PPVARR) F(MMVAR) F(MMVARR) F(PPPVAR) F(PPPVARR) F(MMPVAR) F(MMPVARR) \
    F(PUSHIDX) F(PLUSIDX) F(PLUSIDXR) F(MULIDX) F(MULIDXR) F(SUBIDX) F(SUBIDXR) F(DIVIDX) F(DIVIDXR) F(WRITEIDX) F(WRITEIDXR) F(PPIDX) F(PPIDXR) F(MMIDX) F(MMIDXR) F(PPPIDX) F(PPPIDXR) F(MMPIDX) F(MMPIDXR) \
    F(PUSHFLD) F(PLUSFLD) F(PLUSFLDR) F(MULFLD) F(MULFLDR) F(SUBFLD) F(SUBFLDR) F(DIVFLD) F(DIVFLDR) F(WRITEFLD) F(WRITEFLDR) F(PPFLD) F(PPFLDR) F(MMFLD) F(MMFLDR) F(PPPFLD) F(PPPFLDR) F(MMPFLD) F(MMPFLDR) \
    F(BCALL) \
    F(CALL) F(CALLV) F(FUNSTART) F(FUNEND) \
    F(JUMP) \
    F(NEWVEC) \
    F(POP) \
    F(EXIT) \
    F(ADD) F(SUB) F(MUL) F(DIV) F(MOD) \
    F(LT) F(GT) F(LE) F(GE) F(EQ) F(NE) \
    F(UMINUS) F(LOGNOT) F(JUMPFAIL) F(JUMPFAILR) F(JUMPNOFAIL) F(JUMPNOFAILR) F(RETURN) \
    F(PUSHONCE) F(PUSHPARENT) \
    F(TTSTRUCT) F(TTINT) F(TTFLT) F(TTSTR)

#define F(N) IL_##N,
enum { ILNAMES };
#undef F

struct CodeGen 
{
    vector<int> &code;
    vector<LineInfo> &lineinfo;
    Lex &lex;
    vector<Node *> linenumbernodes;

    CodeGen(Parser &parser, vector<int> &_code, vector<LineInfo> &_lineinfo) : code(_code), lineinfo(_lineinfo), lex(parser.lex)
    {
        linenumbernodes.push_back(parser.root);

        BodyGen(parser.root);
        Emit(IL_EXIT);

        for_all(f, parser.st.functiontable, GenScope(f->subf->body, f));

        linenumbernodes.pop_back();
    }

    ~CodeGen()
    {
        assert(code.size() == lineinfo.size());
    }

    void Error(string err, Node *what)
    {
        lex.Error(err, what->linenumber, lex.st.filenames[what->fileidx].c_str());
    }

    void Emit(int i)
    {
        lineinfo.push_back(LineInfo(linenumbernodes.back()->linenumber,
                                    linenumbernodes.back()->fileidx));
        code.push_back(i);
    }

    void Emit(int i, int j)        { Emit(i); Emit(j); }
    void Emit(int i, int j, int k) { Emit(i); Emit(j); Emit(k); }

    #define MARKL(name) auto name = (int)code.size();
    #define SETL(name) code[name - 1] = (int)code.size();

    void Dummy(int retval) { if (retval) Emit(IL_PUSHUNDEF); }

    void BodyGen(Node *n)
    {
        for (; n; n = n->b)
        {
            assert(n->type == ',');
            Gen(n->a, !n->b);
        }
    }
    
    // FIXME: can do this more elegantly now that we don't have redef problems anymore
    void FindDefsAndCheckRedefRec(Node *n, vector<Ident *> &defs)
    {
        switch (n->type)
        {
            case '{}':
            case 'FUN':
                return;

            case ':=':
                assert(n->a->type == 'ID');
                defs.push_back(n->a->ident);
                FindDefsAndCheckRedefRec(n->b, defs);
                return;
        }

        if (!n->HasChildren()) return;

        if (n->a) FindDefsAndCheckRedefRec(n->a, defs);
        if (n->b) FindDefsAndCheckRedefRec(n->b, defs);
    }
    
    void GenScope(Node *cl, Function *f = NULL)
    {
        if (!f) Emit(IL_PUSHFUN, 0);
        MARKL(funstart);

        if (f) f->bytecodestart = (int)code.size();

        vector<Node *> scope;
        if (cl->a) for (auto ids = cl->a; ids; ids = ids->b)
        {
            scope.push_back(ids->a);
        }

        vector<Ident *> defs1, defs;
        if (cl->b) FindDefsAndCheckRedefRec(cl->b, defs1);
        for (auto topl = cl->b; topl; topl = topl->b)
        {
            assert(topl->type == ',');
            if (topl->a->type == ':=')
                defs.push_back(topl->a->a->ident);
        }
        assert(defs1.size() == defs.size());

        Emit(IL_FUNSTART);
        Emit((int)scope.size()); 
        for_all(idn, scope, Emit(idn->ident->idx));
        Emit((int)defs.size());
        for_all(id, defs, Emit(id->idx));

        for_all(idn, scope, GenTypeCheck(idn->ident->idx, idn->exptype));

        if (cl->b) BodyGen(cl->b);
        else Dummy(true);

        Emit(IL_FUNEND);

        if (!f) SETL(funstart);
    }

    void GenTypeCheck(int idx, Type &type)
    {
        switch(type.t)
        {
            case T_ANY:    break;
            case T_STRUCT: Emit(IL_TTSTRUCT, idx, type.udt->idx); break;
            case T_INT:    Emit(IL_TTINT, idx); break;
            case T_FLOAT:  Emit(IL_TTFLT, idx); break;
            case T_STRING: Emit(IL_TTSTR, idx); break;
            default:       assert(0);
        }
    }

    void Gen(Node *n, bool retval)
    {
        linenumbernodes.push_back(n);
        int opc = 0;

        switch(n->type)
        {
            case 'INT': if (retval) { Emit(IL_PUSHINT, n->integer); }; break;
            case 'FLT': if (retval) { Emit(IL_PUSHFLT); float f = (float)n->flt; Emit(*(int *)&f); }; break; 
            case 'STR': if (retval) { Emit(IL_PUSHSTR); for (const char *p = n->str; *p; p++) Emit(*p); Emit(0); }; break;

            case 'ID':  if (retval) { Emit(IL_PUSHVAR, n->ident->idx); }; break;

            case 'NIL': if (retval)   Emit(IL_PUSHNIL); break;

            case '.':   
                Gen(n->a, retval);
                if (retval) Emit(IL_PUSHFLD, n->b->fld->idx);
                break;

            case 'IDX':
                Gen(n->a, retval);
                Gen(n->b, retval);
                if (retval) Emit(IL_PUSHIDX);
                break;

            case ':=': assert(n->a->type == 'ID');
            case '=':  opc++;
            case '/=': opc++;
            case '-=': opc++;
            case '*=': opc++;
            case '+=':
                GenAssign(n, opc * 2 + (int)retval);
                break;

            case '---': opc++;
            case '+++': opc++;
            case '--':  opc++;
            case '++':  
                GenAssign(n, opc * 2 + (int)retval + 10);
                break;

            case '!=': opc++;
            case '==': opc++;
            case '>=': opc++;
            case '<=': opc++;
            case '>':  opc++;
            case '<':  opc++;
            case '%':  opc++;
            case '/':  opc++;
            case '*':  opc++;
            case '-':  opc++;
            case '+':
                Gen(n->a, retval);
                if (n->b)
                {
                    Gen(n->b, retval);
                    if (retval) Emit(IL_ADD + opc);
                }
                else
                {
                    assert(n->type == '-');
                    if (retval) Emit(IL_UMINUS);
                }
                break;

            case '{}': if (retval) GenScope(n); break; 

            case 'ADT':
                Dummy(retval);
                break;
            
            case 'NATC':
            case 'CALL':
            case '->':
            {
                int nargs = 0;
                for(Node *list = n->b; list; list = list->b)
                {
                    Gen(list->a, true);
                    nargs++;
                }
                if (n->type == 'NATC')
                {
                    Emit(IL_BCALL, n->a->nf->idx, nargs);
                }
                else if (n->type == 'CALL')
                {
                    Emit(IL_CALL, nargs, n->a->f->idx);
                    if (n->a->f->nargs != nargs) Error("call to function " + n->a->f->name + " needs " + string(inttoa(n->a->f->nargs)) + " arguments, " + string(inttoa(nargs)) + " given", n->a);
                }
                else
                {
                    Gen(n->a, true);
                    Emit(IL_CALLV, nargs);
                }
                if (!retval) Emit(IL_POP);
                break;
            }

            case 'FUN':
                Dummy(retval);
                break;   

            case 'TYPE':
            case ':':   // FIXME
                Dummy(retval);
                break;

            case ',':
                assert(0);  // handled by individual parents: EXPS {} [] ADT
                break;

            case 'EXPS':
                for (n = n->a; n; n = n->b)
                {
                    assert(n->type == ',');
                    Gen(n->a, retval && !n->b);
                }
                break;

            case '&':
            {
                Gen(n->a, true);
                Emit(retval ? IL_JUMPFAILR : IL_JUMPFAIL, 0);
                MARKL(loc);
                Gen(n->b, retval);
                SETL(loc);
                break;
            }

            case '|':
            {
                Gen(n->a, true);
                Emit(retval ? IL_JUMPNOFAILR : IL_JUMPNOFAIL, 0);
                MARKL(loc);
                Gen(n->b, retval);
                SETL(loc);
                break;
            }

            case '!':
            {
                Gen(n->a, retval);
                if (retval) Emit(IL_LOGNOT);
                break;
            }

            case '[]':
            {
                Struct *superclass = NULL;

                if (n->b)
                {
                    Emit(IL_NEWVEC, n->b->st->idx, n->b->st->fields.size());
                    superclass = n->b->st->superclass;
                }
                else
                {
                    int nargs = 0;
                    for (Node *it = n->a; it; it = it->b) nargs++;
                    Emit(IL_NEWVEC, V_VECTOR, nargs);
                }

                for (n = n->a; n; n = n->b)
                {
                    assert(n->type == ',');
                    Gen(n->a, true);

                    if (superclass) Emit(IL_PUSHPARENT, superclass->idx);
                    else Emit(IL_PUSHONCE);

                    superclass = NULL;
                }
                if (!retval) Emit(IL_POP);
                break;
            }

            case 'RET':
                if (n->a) Gen(n->a, true);
                else Emit(IL_PUSHUNDEF);
                Emit(IL_RETURN, n->b->integer);
                // retval==true is nonsensical here, but can't enforce
                break;

            default:
                assert(0);
        }

        linenumbernodes.pop_back();
    }

    void GenAssign(Node *n, int offset)
    {
        if (n->b) Gen(n->b, true);
        switch (n->a->type)
        {
            case 'ID':  Emit(IL_PLUSVAR + offset, n->a->ident->idx); break;
            case '.':   Gen(n->a->a, true); Emit(IL_PLUSFLD + offset, n->a->b->fld->idx); break;
            case 'IDX': Gen(n->a->a, true); Gen(n->a->b, true); Emit(IL_PLUSIDX + offset); break;
            default:    Error("lvalue required", n->a);
        }
    }

    #undef MARKL
    #undef SETL
};