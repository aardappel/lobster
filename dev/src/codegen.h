
#define ILNAMES \
    F(PUSHINT) \
    F(PUSHFLT) \
    F(PUSHSTR) \
    F(PUSHUNDEF) \
    F(PUSHNIL) \
    F(PUSHFUN) \
    F(PUSHVAR) F(LVALVAR) \
    F(PUSHIDX) F(LVALIDX) \
    F(PUSHFLDO) F(PUSHFLDC) F(PUSHFLDT) F(LVALFLDO) F(LVALFLDC) F(LVALFLDT) \
    F(PUSHLOC) F(LVALLOC) \
    F(BCALL) \
    F(CALL) F(CALLV) F(CALLVCOND) F(STORELOOPVAR) F(DUP) F(CONT1) F(CONT2) F(FUNSTART) F(FUNEND) F(FUNMULTI) F(CALLMULTI) \
    F(JUMP) \
    F(NEWVEC) \
    F(POP) \
    F(EXIT) \
    F(ADD) F(SUB) F(MUL) F(DIV) F(MOD) \
    F(LT) F(GT) F(LE) F(GE) F(EQ) F(NE) \
    F(UMINUS) F(LOGNOT) F(JUMPFAIL) F(JUMPFAILR) F(JUMPNOFAIL) F(JUMPNOFAILR) F(RETURN) \
    F(PUSHONCE) F(PUSHPARENT) \
    F(TTSTRUCT) F(TT) F(TTFLT) F(TTSTR) F(ISTYPE) F(CORO) F(COCL) F(COEND) \
    F(FIELDTABLES)

#define F(N) IL_##N,
enum { ILNAMES };
#undef F

#define LVALOPNAMES \
    F(WRITE) F(WRITER) F(WRITED) F(PLUS) F(PLUSR) F(MUL) F(MULR) F(SUB) F(SUBR) F(DIV) F(DIVR) F(MOD) F(MODR) F(PP) F(PPR) F(MM) F(MMR) F(PPP) F(PPPR) F(MMP) F(MMPR) 

#define F(N) LVO_##N,
enum { LVALOPNAMES };
#undef F

struct CodeGen 
{
    vector<int> &code;
    vector<LineInfo> &lineinfo;
    Lex &lex;
    Parser &parser;
    vector<Node *> linenumbernodes;
    SymbolTable &st;

    CodeGen(Parser &_p, SymbolTable &_st, vector<int> &_code, vector<LineInfo> &_lineinfo, bool verbose) : code(_code), lineinfo(_lineinfo), lex(_p.lex), parser(_p), st(_st)
    {
        linenumbernodes.push_back(parser.root);

        GenFieldTables(st, verbose);

        BodyGen(parser.root);
        Emit(IL_EXIT);

        for (;;)    // breadth first generation of functions that are actually used
        {
            int generated = 0;
            for (auto f : parser.st.functiontable) if (GenFunction(f)) generated++;
            if (!generated) break;
        }

        linenumbernodes.pop_back();
    }

    ~CodeGen()
    {
    }

    void Emit(int i)
    {
        int l = linenumbernodes.back()->linenumber;
        int f = linenumbernodes.back()->fileidx;
        if (lineinfo.empty() || l != lineinfo.back().line || f != lineinfo.back().fileidx)
            lineinfo.push_back(LineInfo(l, f, code.size()));
        code.push_back(i);
    }

    void Emit(int i, int j)        { Emit(i); Emit(j); }
    void Emit(int i, int j, int k) { Emit(i); Emit(j); Emit(k); }

    #define MARKL(name) auto name = (int)code.size();
    #define SETL(name) code[name - 1] = (int)code.size();

    void Dummy(int retval) { while (retval--) Emit(IL_PUSHUNDEF); }

    void BodyGen(Node *n)
    {
        for (; n; n = n->b)
        {
            assert(n->type == ',');
            Gen(n->a, !n->b);
        }
    }

    struct sfcompare
    {
        int nargs;
        CodeGen *cg;
        Function *f;
        bool operator()(SubFunction *a, SubFunction *b)
        {
            for (int i = 0; i < nargs; i++)
            {
                auto &ta = a->args[i].type;
                auto &tb = b->args[i].type;

                if (ta != tb) return ta < tb;
            }
            cg->parser.Error("function signature overlap for " + f->name, NULL);
            return false;
        }
    } sfcomparator;


    bool GenFunction(Function *f)
    {
        if (f->bytecodestart > 0 || !f->ncalls) return false;

        if (!f->subf->next) // single function
        {
            f->bytecodestart = (int)code.size();
            GenScope(f->subf->body);
            return true;
        }

        // do multi-dispatch
        vector<SubFunction *> sfs;

        for (auto sf = f->subf; sf; sf = sf->next)
        {
            sf->subbytecodestart = (int)code.size();
            sfs.push_back(sf);
            GenScope(sf->body);
        }

        sfcomparator.nargs = f->nargs;
        sfcomparator.cg = this;
        sfcomparator.f = f;
        sort(sfs.begin(), sfs.end(), sfcomparator);

        f->bytecodestart = (int)code.size();
        Emit(IL_FUNMULTI, sfs.size());

        for (auto sf : sfs)
        {
            for (int j = 0; j < f->nargs; j++)
            {
                auto arg = sf->args[j];
                Emit(arg.type.t, arg.type.idx);
            }
            Emit(sf->subbytecodestart);
        }

        return true;
    }

    void GenFunctionVal(Node *cl)
    {
        Emit(IL_PUSHFUN, 0);
        MARKL(funstart);
        GenScope(cl);
        SETL(funstart);
    }
                    
    void GenScope(Node *cl)
    {
        vector<Node *> scope;
        if (cl->a) for (auto ids = cl->a; ids; ids = ids->b)
        {
            scope.push_back(ids->a);
        }

        vector<Ident *> defs;
        for (auto topl = cl->b; topl; topl = topl->b)
        {
            for (auto dl = topl->a; dl->type == ':='; dl = dl->b)
            {
                auto id = dl->a->ident;
                defs.push_back(id);
            }
        }

        linenumbernodes.push_back(cl);

        Emit(IL_FUNSTART);
        Emit((int)scope.size()); 
        for (auto idn : scope) Emit(idn->ident->idx);
        Emit((int)defs.size());
        for (auto id : defs) Emit(id->idx);

        //for (auto idn : scope) GenTypeCheck(idn->ident->idx, idn->exptype);

        if (cl->b) BodyGen(cl->b);
        else Dummy(true);

        Emit(IL_FUNEND);

        linenumbernodes.pop_back();
    }

    void GenTypeCheck(Type &type)
    {
        switch(type.t)
        {
            case V_UNKNOWN: break;
            case V_FLOAT:   Emit(IL_TTFLT); break;
            case V_STRING:  Emit(IL_TTSTR); break;
            case V_VECTOR:  if(type.idx >= 0) { Emit(IL_TTSTRUCT, type.idx); break; } // else fall thru
            default:        Emit(IL_TT, type.t); break;
        }
    }

    void Gen(Node *n, int retval)
    {
        linenumbernodes.push_back(n);
        int maxretvalsupplied = 1;  // by default, the cases below only deal with 0 or 1 retvals, set this to > 1 to indicate extra values on the stack need to be dealt with
        int opc = 0;

        switch(n->type)
        {
            case 'INT': if (retval) { Emit(IL_PUSHINT, n->integer); }; break;
            case 'FLT': if (retval) { Emit(IL_PUSHFLT); int2float i2f; i2f.f = (float)n->flt; Emit(i2f.i); }; break; 
            case 'STR': if (retval) { Emit(IL_PUSHSTR); for (const char *p = n->str; *p; p++) Emit(*p); Emit(0); }; break;

            case 'ID':  if (retval) { Emit(IL_PUSHVAR, n->ident->idx); }; break;

            case 'NIL': if (retval)   Emit(IL_PUSHNIL); break;

            case '.':   
                Gen(n->a, retval);
                if (retval) GenFieldAccess(n->b->fld, -1);
                break;

            case 'IDX':
                Gen(n->a, retval);
                Gen(n->b, retval);
                if (retval) Emit(IL_PUSHIDX);
                break;

            case '.@':
                Gen(n->a, retval);
                if (retval) Emit(IL_PUSHLOC, n->b->ident->idx);
                break;

            case ':=':
            case ',=':
            {
                int num = 0;
                auto dl = n;
                vector<Ident *> ids;
                for (; dl->type == ':=' || dl->type == ',='; dl = dl->b)
                {
                    assert(dl->a->type == 'ID');
                    ids.push_back(dl->a->ident);
                    num++;
                }
                Gen(dl, num);
                dl = n;
                for (int i = ids.size() - 1; i >= 0; i--)
                {
                    Emit(IL_LVALVAR, n->type == ':=' ? LVO_WRITED : LVO_WRITE, ids[i]->idx); 
                }
                if (retval) Dummy(retval);  // currently can only happen with def on last line of body, which is nonsensical
                break;
            }

            case '=': GenAssign(n, LVO_WRITE, retval); break;

            case '+=': GenAssign(n, LVO_PLUS, retval); break;
            case '*=': GenAssign(n, LVO_MUL,  retval); break;
            case '-=': GenAssign(n, LVO_SUB,  retval); break;
            case '/=': GenAssign(n, LVO_DIV,  retval); break;
            case '%=': GenAssign(n, LVO_MOD,  retval); break;

            case '---': GenAssign(n, LVO_MMP, retval); break;
            case '+++': GenAssign(n, LVO_PPP, retval); break;
            case '--':  GenAssign(n, LVO_MM,  retval); break;
            case '++':  GenAssign(n, LVO_PP,  retval); break;

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

            case '{}': if (retval) GenFunctionVal(n); break; 

            case 'ADT':
                Dummy(retval);
                break;
            
            case 'NATC':
            case 'CALL':
            case '->':     
            {
                int nargs = 0;
                Node *lastarg = NULL;
                auto genargs = [&](Arg *args, int checkargs)
                {
                    for (Node *list = n->b; list; list = list->b)
                    {
                        Gen(list->a, 1);
                        if (nargs < checkargs) GenTypeCheck(args[nargs].type);
                        lastarg = list->a;
                        nargs++;
                    }
                };
                if (n->type == 'NATC')
                {
                    auto nf = n->a->nf;
                    if (nf->ncm == NCM_LOOP)
                    {
                        Emit(IL_PUSHINT, -1);
                    }
                    else if (nf->ncm == NCM_WHILE)
                    {
                        Emit(IL_PUSHUNDEF);
                    }
                    genargs(NULL, 0); // TODO: could pass arg types in here if most exps have types, cheaper than doing it all in call instruction?
                    switch(nf->ncm)
                    {
                        case NCM_CONTINUATION:  // if()
                            Emit(IL_BCALL, nf->idx, nargs);
                            Emit(IL_CALLVCOND, 0); 
                            break;

                        case NCM_CONT_EXIT:     // graphics.h
                            Emit(IL_BCALL, nf->idx, nargs);
                            if (lastarg->type != 'NIL') // FIXME: this will not work if its a var with nil value
                            {
                                Emit(IL_CALLVCOND, 0);
                                Emit(IL_CONT1, nf->idx);
                            }
                            break;

                        case NCM_LOOP:      // for() filter() exists() map()
                        {
                            assert(lastarg->type == '{}' || lastarg->type == 'COCL');
                            int clnargs = 0; 
                            if (lastarg->type == 'COCL') clnargs = 1;
                            else if (lastarg->a) for (auto ids = lastarg->a; ids; ids = ids->b) clnargs++;
                            Emit(IL_JUMP, 0);
                            MARKL(pos);
                            Emit(IL_CALLV, min(2, clnargs));
                            Emit(IL_STORELOOPVAR, 0);
                            SETL(pos);
                            Emit(IL_BCALL, nf->idx, nargs);
                            Emit(IL_JUMPNOFAIL, pos);
                            break;
                        }

                        case NCM_WHILE:     // while() collectwhile() // FIXME: reduce instructions
                        {
                            MARKL(start);
                            Emit(IL_DUP, 1);
                            Emit(IL_CALLV, 0);      
                            Emit(IL_STORELOOPVAR, 0);
                            Emit(IL_BCALL, nf->idx, nargs);
                            Emit(IL_JUMPFAIL, 0);
                            MARKL(pos);
                            Emit(IL_DUP, 0);
                            Emit(IL_CALLV, 0);  
                            Emit(IL_STORELOOPVAR, 1);
                            Emit(IL_JUMP, start);
                            SETL(pos);
                            break;
                        }

                        default:
                            Emit(IL_BCALL, nf->idx, nargs);
                            break;
                    }
                    if (nf->nretvalues > 1)
                    {
                        maxretvalsupplied = nf->nretvalues;
                    }
                    else if (!nf->nretvalues && retval)
                    { 
                        // can't make this an error since these functions are often called as the last thing in a function, requiring a return value
                        //Error("builtin function call returns nothing", n); 
                    }
                }
                else if (n->type == 'CALL')
                {
                    auto &f = *n->a->f;
                    genargs(f.subf->args, f.subf->next ? 0 : f.nargs);
                    if (f.nargs != nargs) parser.Error("call to function " + f.name + " needs " + string(inttoa(f.nargs)) + " arguments, " + string(inttoa(nargs)) + " given", n->a);
                    f.ncalls++;
                    Emit(f.subf->next ? IL_CALLMULTI : IL_CALL, nargs, f.idx);
                    if (f.retvals > 1)
                    {
                        maxretvalsupplied = f.retvals;
                    }
                }
                else
                {
                    genargs(NULL, 0);
                    Gen(n->a, 1);
                    Emit(IL_CALLV, nargs);
                }
                if (!retval) Emit(IL_POP);
                break;
            }

            case 'FUN':
                Dummy(retval);
                break;   

            case ':':   // FIXME
                Dummy(retval);
                break;

            case ',':
                assert(0);  // handled by individual parents: EXPS {} [] ADT
                break;

            case 'BUT':
                Gen(n->a, false);
                Gen(n->b, retval);
                break;

                /*
            case 'EXPS':
                for (n = n->a; n; n = n->b)
                {
                    assert(n->type == ',');
                    Gen(n->a, retval && !n->b);
                }
                break;
                */

            case 'MR':
                maxretvalsupplied = 0;
                assert(retval);
                for (; n; n = n->b)
                {
                    assert(n->type == 'MR');
                    Gen(n->a, true);
                    maxretvalsupplied++;
                }
                break;

            case '&':
            {
                Gen(n->a, 1);
                Emit(retval ? IL_JUMPFAILR : IL_JUMPFAIL, 0);
                MARKL(loc);
                Gen(n->b, retval);
                SETL(loc);
                break;
            }

            case '|':
            {
                Gen(n->a, 1);
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
                Struct *struc = NULL;

                if (n->exptype.idx >= 0)
                {
                    struc = st.structtable[n->exptype.idx];
                    Emit(IL_NEWVEC, struc->idx, struc->fields.size());
                    superclass = struc->superclass;
                }
                else
                {
                    int nargs = 0;
                    for (Node *it = n->a; it; it = it->b) nargs++;
                    Emit(IL_NEWVEC, V_VECTOR, nargs);
                }

                int i = 0;
                for (auto cn = n->a; cn; cn = cn->b)
                {
                    assert(cn->type == ',');
                    if (cn->a->type == 'SUP')
                    {
                        Gen(cn->a->a, 1);
                        Emit(IL_PUSHPARENT, superclass ? superclass->idx : -1);
                        i += superclass->fields.size();  // FIXME: not typechecking these .. 
                    }
                    else
                    {
                        Gen(cn->a, 1);

                        if (struc) GenTypeCheck(struc->fields[i].first.type);
                        else GenTypeCheck(n->exptype);

                        Emit(IL_PUSHONCE);
                        i++;
                    }

                    superclass = NULL;
                }
                if (!retval) Emit(IL_POP);
                break;
            }

            case 'IS':
            {
                Gen(n->a, retval);
                if (retval) Emit(IL_ISTYPE, n->a->exptype.t, n->a->exptype.idx);
                break;
            }

            case 'RET':
            {
                int fid = n->b->integer;
                if (n->a) Gen(n->a, fid >= 0 ? st.functiontable[fid]->retvals : 1);
                else Emit(IL_PUSHUNDEF);
                Emit(IL_RETURN, fid);
                // retval==true is nonsensical here, but can't enforce
                break;
            }
            case 'COCL':
                if (retval) Emit(IL_COCL);
                break;

            case 'CORO':
            {
                // TODO: maybe more correct to generate call regardless of retval?
                if (retval) Emit(IL_CORO, 0);
                MARKL(loc);

                if (retval)
                {
                    bool found = false;
                    Emit(0); // count
                    // TODO: we shouldn't need to compute and store this table for each call, instead do it once for each function / builtin function
                    auto err = n->a->FindIdentsUpToYield([&](vector<Ident *> &istack)
                    {
                        found = true;
                        for (auto id : istack)
                        {
                            for (size_t i = loc + 1; i < code.size(); i++) if (code[i] == id->idx) continue;
                            // FIXME: merging of variables from all yield sites is potentially incorrect, we might end up restoring variables that are not actually in use
                            Emit(id->idx);
                            //printf("cor: %s\n", id->name.c_str());
                        }
                    });

                    if (err)
                        parser.Error(string("coroutine construction error: ") + err, n->a);

                    // this guarantees FindIdentsUpToYield has done an accurate job finding all ids, since if it can reach the yield, it must also have found the whole callchain leading up to it
                    // if people start storing the yield function inside data structures or doing other weird things to confuse the algorithm, they'll at least get this error
                    if (!found)
                        parser.Error("coroutine construction error: cannot find yield call", n->a);
                    code[loc] = code.size() - loc - 1;
                }

                Gen(n->a, retval);

                if (retval)
                {
                    Emit(IL_COEND);
                    SETL(loc);
                }
                break;
            }

            default:
                assert(0);
        }


        if (maxretvalsupplied == 1) // if we generate just 0/1 value, it can be copied into multiple vars if needed
        {
            while (retval > 1) { Emit(IL_DUP, 0); retval--; }
        }
        else if(retval < maxretvalsupplied) // if the caller doesn't want all return values, just pop em
        {
            while (maxretvalsupplied > retval && retval > 0) { Emit(IL_POP); maxretvalsupplied--; }
        }
        else if (retval > maxretvalsupplied)    // only happens if both are > 1
        {
            parser.Error("expression does not supply that many return values", n);
        }

        linenumbernodes.pop_back();
    }

    void GenAssign(Node *n, int lvalop, int retval)
    {
        if (retval) lvalop++;
        if (n->b) Gen(n->b, 1);
        switch (n->a->type)
        {
            case 'ID':  Emit(IL_LVALVAR, lvalop, n->a->ident->idx); break;
            case '.':   Gen(n->a->a, 1); GenFieldAccess(n->a->b->fld, lvalop); break;
            case '.@':  Gen(n->a->a, 1); Emit(IL_LVALLOC, lvalop, n->a->b->ident->idx); break;
            case 'IDX': Gen(n->a->a, 1); Gen(n->a->b, 1); Emit(IL_LVALIDX, lvalop); break;
            default:    parser.Error("lvalue required", n->a);
        }
    }

    void GenFieldAccess(SharedField *f, int lvalop)
    {
        int om = f->numunique == 1 ? 0 : f->offsettable >= 0 ? 2 : 1;

        if (lvalop >= 0) Emit(IL_LVALFLDO + om, lvalop);
        else Emit(IL_PUSHFLDO + om);

        switch (om)
        {
            case 0: Emit(f->offsets[0].offset); break;
            case 1: Emit(f->fo1.structidx, f->fo1.offset, f->foN.offset); break;
            case 2: Emit(f->offsettable); break;
        }
    }

    void GenFieldTables(SymbolTable &st, bool verbose)
    {
        string condfields, tablefields;

        Emit(IL_FIELDTABLES, 0);
        MARKL(loc);

        for (auto f : st.fieldtable)
        {
            if (f->numunique == 1) continue;

            if (f->numunique == 2) // see if of the two offsets, one of them only is used with one type, then we can use a simple if-check encoding
            {
                FieldOffset fo1, fo2;
                int n1 = 1, n2 = 0;
                fo1 = f->offsets[0];
                for (auto &fo : f->offsets)
                {
                    if (fo.offset == fo1.offset) n1++;
                    else { fo2 = fo; n2++; } 
                }
                if (n1 == 1) { f->fo1 = fo1; f->foN = fo2; condfields += " " + f->name; continue; }
                if (n2 == 1) { f->fo1 = fo2; f->foN = fo1; condfields += " " + f->name; continue; }
            }
            
            // we have a more complex distribution, encode as a lookup table

            f->offsettable = code.size();

            // TODO: could optimize this by cutting off leading and trailing 0's.
            for (size_t i = 0; i < st.structtable.size(); i++) Emit(0);
            for (auto &fo : f->offsets) code[f->offsettable + fo.structidx] = fo.offset; 

            tablefields += " " + f->name;
        }

        SETL(loc);

        if (verbose)
        {
            if (condfields.length())  printf("performance warning: conditionals generated for fields:%s\n", condfields.c_str());
            if (tablefields.length()) printf("performance warning: table lookups generated for fields:%s (in %ld types)\n", tablefields.c_str(), long(st.structtable.size()));
        }
    }

    #undef MARKL
    #undef SETL
};
