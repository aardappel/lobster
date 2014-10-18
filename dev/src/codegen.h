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
    F(CALL) F(CALLV) F(CALLVCOND) F(STORELOOPVAR) F(DUP) F(CONT1) F(CONT2) \
    F(FUNSTART) F(FUNEND) F(FUNMULTI) F(CALLMULTI) \
    F(JUMP) \
    F(NEWVEC) \
    F(POP) \
    F(EXIT) \
    F(IADD) F(ISUB) F(IMUL) F(IDIV) F(IMOD) F(ILT) F(IGT) F(ILE) F(IGE) F(IEQ) F(INE) \
    F(FADD) F(FSUB) F(FMUL) F(FDIV) F(FMOD) F(FLT) F(FGT) F(FLE) F(FGE) F(FEQ) F(FNE) \
    F(AADD) F(ASUB) F(AMUL) F(ADIV) F(AMOD) F(ALT) F(AGT) F(ALE) F(AGE) F(AEQ) F(ANE) \
    F(UMINUS) F(LOGNOT) F(I2F) F(A2S) F(JUMPFAIL) F(JUMPFAILR) F(JUMPNOFAIL) F(JUMPNOFAILR) F(RETURN) \
    F(PUSHONCE) F(PUSHPARENT) \
    F(TTSTRUCT) F(TT) F(TTFLT) F(TTSTR) F(ISTYPE) F(CORO) F(COCL) F(COEND) \
    F(FIELDTABLES) F(LOGREAD)

#define F(N) IL_##N,
enum { ILNAMES };
#undef F

#define LVALOPNAMES \
    F(WRITE) F(WRITER) F(WRITED) F(PLUS) F(PLUSR) F(MUL) F(MULR) F(SUB) F(SUBR) F(DIV) F(DIVR) F(MOD) F(MODR) \
    F(PP) F(PPR) F(MM) F(MMR) F(PPP) F(PPPR) F(MMP) F(MMPR) 

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
    vector<pair<int, SubFunction *>> call_fixups;
    SymbolTable &st;

    CodeGen(Parser &_p, SymbolTable &_st, vector<int> &_code, vector<LineInfo> &_lineinfo, bool verbose)
        : code(_code), lineinfo(_lineinfo), lex(_p.lex), parser(_p), st(_st)
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

        for (auto &fixup : call_fixups)
        {
            auto &sf = *fixup.second;
            auto &f = *sf.parent;
            auto bytecodestart = f.multimethod ? f.bytecodestart : sf.subbytecodestart;
            assert(bytecodestart);
            assert(!code[fixup.first]);
            code[fixup.first] = bytecodestart;
        }
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

    void Emit(int i, int j)               { Emit(i); Emit(j); }
    void Emit(int i, int j, int k)        { Emit(i); Emit(j); Emit(k); }
    void Emit(int i, int j, int k, int l) { Emit(i); Emit(j); Emit(k); Emit(l); }

    #define MARKL(name) auto name = (int)code.size();
    #define SETL(name) code[name - 1] = (int)code.size();

    void Dummy(int retval) { while (retval--) Emit(IL_PUSHUNDEF); }

    void BodyGen(Node *n)
    {
        for (; n; n = n->tail()) Gen(n->head(), !n->tail());
    }

    struct sfcompare
    {
        int nargs;
        CodeGen *cg;
        Function *f;
        bool operator() (SubFunction *a, SubFunction *b)
        {
            for (int i = 0; i < nargs; i++)
            {
                auto &ta = a->args[i].type;
                auto &tb = b->args[i].type;

                if (ta != tb) return ta < tb;
            }
            cg->parser.Error("function signature overlap for " + f->name, nullptr);
            return false;
        }
    } sfcomparator;


    bool GenFunction(Function *f)
    {
        if (f->bytecodestart > 0 || !f->ncalls) return false;

        if (!f->multimethod)
        {
            f->bytecodestart = (int)code.size();
            for (auto sf = f->subf; sf; sf = sf->next)
            {
                sf->subbytecodestart = (int)code.size();
                GenScope(sf->body);
            }
        }
        else
        {
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
            Emit(IL_FUNMULTI, sfs.size(), f->nargs);

            for (auto sf : sfs)
            {
                for (int j = 0; j < f->nargs; j++)
                {
                    auto arg = sf->args[j];
                    Emit(arg.type.t == V_STRUCT ? V_VECTOR : arg.type.t, arg.type.idx);
                }
                Emit(sf->subbytecodestart);
            }
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
        if (cl->parameters()) for (auto ids = cl->parameters(); ids; ids = ids->tail())
        {
            scope.push_back(ids->head());
        }

        vector<Ident *> defs;
        vector<Ident *> logvars;
        for (auto topl = cl->body(); topl; topl = topl->tail())
        {
            size_t logmultiassignstart = logvars.size();
            for (auto dl = topl->head(); dl->type == T_DEF; dl = dl->right())
            {
                auto id = dl->left()->ident();
                if (id->logvaridx >= 0)
                {
                    id->logvaridx = logvars.size();
                    logvars.push_back(id);
                }
                else defs.push_back(id);
            }
            // order of multi-assign initializers is reversed on the stack
            reverse(logvars.begin() + logmultiassignstart, logvars.end());
        }

        linenumbernodes.push_back(cl);

        Emit(IL_FUNSTART);
        Emit((int)scope.size()); 
        for (auto idn : scope) Emit(idn->ident()->idx);
        Emit((int)(defs.size() + logvars.size()));
        for (auto id : defs) Emit(id->idx);
        for (auto id : logvars) Emit(id->idx);
        Emit((int)logvars.size());

        //for (auto idn : scope) GenTypeCheck(idn->ident->idx, idn->exptype);

        if (cl->body()) BodyGen(cl->body());
        else Dummy(true);

        Emit(IL_FUNEND);

        linenumbernodes.pop_back();
    }

    void GenTypeCheck(Type &given, Type &type)
    {
        if (given == type) return;
        switch(type.t)
        {
            case V_ANY:     break;
            case V_FLOAT:   Emit(IL_TTFLT); break;
            case V_STRING:  Emit(IL_TTSTR); break;
            case V_STRUCT:  Emit(IL_TTSTRUCT, type.idx); break;
            default:        Emit(IL_TT, type.t); break;
        }
    }

    void Gen(Node *n, int retval)
    {
        linenumbernodes.push_back(n);
        // by default, the cases below only deal with 0 or 1 retvals,
        // set this to > 1 to indicate extra values on the stack need to be dealt with
        int maxretvalsupplied = 1;
        int opc = 0;

        switch(n->type)
        {
            case T_INT:   if (retval) { Emit(IL_PUSHINT, n->integer()); }; break;
            case T_FLOAT: if (retval) { Emit(IL_PUSHFLT); int2float i2f; i2f.f = (float)n->flt(); Emit(i2f.i); }; break; 
            case T_STR:   if (retval) { Emit(IL_PUSHSTR); for (const char *p = n->str(); *p; p++) Emit(*p); Emit(0); }; break;
            case T_NIL:   if (retval) { Emit(IL_PUSHNIL); break; }

            case T_IDENT:  if (retval) { Emit(IL_PUSHVAR, n->ident()->idx); }; break;

            case T_DOT:   
                Gen(n->left(), retval);
                if (retval) GenFieldAccess(n->right()->fld(), -1);
                break;

            case T_INDEX:
                Gen(n->left(), retval);
                Gen(n->right(), retval);
                if (retval) Emit(IL_PUSHIDX);
                break;

            case T_CO_AT:
                Gen(n->coroutine_at(), retval);
                if (retval) Emit(IL_PUSHLOC, n->coroutine_var()->ident()->idx);
                break;

            case T_DEF:
            case T_ASSIGNLIST:
            {
                int num = 0;
                auto dl = n;
                vector<Ident *> ids;
                for (; dl->type == T_DEF || dl->type == T_ASSIGNLIST; dl = dl->right())
                {
                    ids.push_back(dl->left()->ident());
                    num++;
                }
                Gen(dl, num);
                dl = n;
                for (int i = ids.size() - 1; i >= 0; i--)
                {
                    if (n->type == T_DEF)
                    {
                        if (ids[i]->logvaridx >= 0) Emit(IL_LOGREAD, ids[i]->logvaridx);
                        Emit(IL_LVALVAR, LVO_WRITED, ids[i]->idx);
                    }
                    else
                    {
                        Emit(IL_LVALVAR, LVO_WRITE, ids[i]->idx);
                    }
                }
                // currently can only happen with def on last line of body, which is nonsensical
                if (retval) Dummy(retval);
                break;
            }

            case T_ASSIGN: GenAssign(n->left(), LVO_WRITE, retval, n->right()); break;

            case T_PLUSEQ:  GenAssign(n->left(), LVO_PLUS, retval, n->right()); break;
            case T_MULTEQ:  GenAssign(n->left(), LVO_MUL,  retval, n->right()); break;
            case T_MINUSEQ: GenAssign(n->left(), LVO_SUB, retval, n->right()); break;
            case T_DIVEQ:   GenAssign(n->left(), LVO_DIV, retval, n->right()); break;
            case T_MODEQ:   GenAssign(n->left(), LVO_MOD,  retval, n->right()); break;

            case T_POSTDECR: GenAssign(n->child(), LVO_MMP, retval); break;
            case T_POSTINCR: GenAssign(n->child(), LVO_PPP, retval); break;
            case T_DECR:     GenAssign(n->child(), LVO_MM,  retval); break;
            case T_INCR:     GenAssign(n->child(), LVO_PP,  retval); break;

            case T_NEQ:   opc++;
            case T_EQ:    opc++;
            case T_GTEQ:  opc++;
            case T_LTEQ:  opc++;
            case T_GT:    opc++;
            case T_LT:    opc++;
            case T_MOD:   opc++;
            case T_DIV:   opc++;
            case T_MULT:  opc++;
            case T_MINUS: opc++;
            case T_PLUS:
                Gen(n->left(), retval);
                Gen(n->right(), retval);
                if (retval)
                {
                    if (n->exptype.t == V_INT) Emit(IL_IADD + opc);
                    else if (n->exptype.t == V_FLOAT) Emit(IL_FADD + opc);
                    else Emit(IL_AADD + opc);
                }
                break;

            case T_UMINUS:
                Gen(n->child(), retval);
                if (retval) Emit(IL_UMINUS);
                break;

            case T_I2F:
                Gen(n->child(), retval);
                if (retval) Emit(IL_I2F);
                break;

            case T_A2S:
                Gen(n->child(), retval);
                if (retval) Emit(IL_A2S);
                break;

            case T_CLOSURE: if (retval) GenFunctionVal(n); break; 

            case T_STRUCTDEF:
                Dummy(retval);
                break;
            
            case T_NATCALL:
            case T_CALL:
            case T_DYNCALL:     
            {
                int nargs = 0;
                Node *lastarg = nullptr;
                auto genargs = [&](Node *list, Arg *args, int checkargs)
                {
                    for (; list; list = list->tail())
                    {
                        Gen(list->head(), 1);
                        if (nargs < checkargs) GenTypeCheck(list->head()->exptype, args[nargs].type);
                        lastarg = list->head();
                        nargs++;
                    }
                };
                if (n->type == T_NATCALL)
                {
                    auto nf = n->ncall_id()->nf();
                    if (nf->ncm == NCM_LOOP)
                    {
                        Emit(IL_PUSHINT, -1);
                    }
                    else if (nf->ncm == NCM_WHILE)
                    {
                        Emit(IL_PUSHUNDEF);
                    }
                    // TODO: could pass arg types in here if most exps have types, cheaper than doing it all in call
                    // instruction?
                    genargs(n->ncall_args(), nullptr, 0);
                    switch(nf->ncm)
                    {
                        case NCM_CONTINUATION:  // if()
                            Emit(IL_BCALL, nf->idx, nargs);
                            Emit(IL_CALLVCOND, 0); 
                            break;

                        case NCM_CONT_EXIT:     // graphics.h
                            Emit(IL_BCALL, nf->idx, nargs);
                            if (lastarg->type != T_NIL) // FIXME: this will not work if its a var with nil value
                            {
                                Emit(IL_CALLVCOND, 0);
                                Emit(IL_CONT1, nf->idx);
                            }
                            break;

                        case NCM_LOOP:      // for() filter() exists() map()
                        {
                            assert(lastarg->type == T_CLOSURE || lastarg->type == T_COCLOSURE);
                            int clnargs = 0; 
                            if (lastarg->type == T_COCLOSURE) clnargs = 1;
                            else if (lastarg->parameters()) for (auto ids = lastarg->parameters(); ids; ids = ids->tail()) clnargs++;
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
                        // can't make this an error since these functions are often called as the last thing in a
                        // function, requiring a return value
                        //Error("builtin function call returns nothing", n); 
                    }
                }
                else if (n->type == T_CALL)
                {
                    auto &sf = *n->call_function()->sf();
                    auto &f = *sf.parent;
                    genargs(n->call_args(), sf.args, f.multimethod ? 0 : f.nargs);
                    if (f.nargs != nargs)
                        parser.Error("call to function " + f.name + " needs " + string(inttoa(f.nargs)) +
                                     " arguments, " + string(inttoa(nargs)) + " given", n->call_function());
                    f.ncalls++;
                    auto bytecodestart = f.multimethod ? f.bytecodestart : sf.subbytecodestart;
                    Emit(f.multimethod ? IL_CALLMULTI : IL_CALL, nargs, f.idx, bytecodestart);
                    if (!bytecodestart) call_fixups.push_back(make_pair((int)code.size() - 1, &sf));
                    if (f.retvals > 1)
                    {
                        maxretvalsupplied = f.retvals;
                    }
                }
                else
                {
                    genargs(n->dcall_args(), nullptr, 0);
                    Gen(n->dcall_var(), 1);
                    Emit(IL_CALLV, nargs);
                }
                if (!retval) Emit(IL_POP);
                break;
            }

            case T_FUNDEF:
                Dummy(retval);
                break;   

            case T_LIST:
                assert(0);  // handled by individual parents: EXPS {} [] ADT
                break;

            case T_SEQ:
                Gen(n->left(), false);
                Gen(n->right(), retval);
                break;

            case T_MULTIRET:
                maxretvalsupplied = 0;
                assert(retval);
                for (; n; n = n->tailexps())
                {
                    assert(n->type == T_MULTIRET);
                    Gen(n->headexp(), true);
                    maxretvalsupplied++;
                }
                break;

            case T_AND:
            {
                Gen(n->left(), 1);
                Emit(retval ? IL_JUMPFAILR : IL_JUMPFAIL, 0);
                MARKL(loc);
                Gen(n->right(), retval);
                SETL(loc);
                break;
            }

            case T_OR:
            {
                Gen(n->left(), 1);
                Emit(retval ? IL_JUMPNOFAILR : IL_JUMPNOFAIL, 0);
                MARKL(loc);
                Gen(n->right(), retval);
                SETL(loc);
                break;
            }

            case T_NOT:
            {
                Gen(n->child(), retval);
                if (retval) Emit(IL_LOGNOT);
                break;
            }

            case T_CONSTRUCTOR:
            {
                Struct *superclass = nullptr;
                Struct *struc = nullptr;

                auto vtype = n->constructor_type()->typenode();
                if (vtype->t == V_STRUCT)
                {
                    struc = st.structtable[vtype->idx];
                    Emit(IL_NEWVEC, struc->idx, struc->fields.size());
                    superclass = struc->superclass;
                }
                else
                {
                    int nargs = 0;
                    for (Node *it = n->constructor_args(); it; it = it->tail()) nargs++;
                    Emit(IL_NEWVEC, V_VECTOR, nargs);
                }

                int i = 0;
                for (auto cn = n->constructor_args(); cn; cn = cn->tail())
                {
                    assert(cn->type == T_LIST);
                    if (cn->head()->type == T_SUPER)
                    {
                        if (!superclass) parser.Error("super used in object without superclass", cn->head());
                        Gen(cn->head()->child(), 1);
                        Emit(IL_PUSHPARENT, superclass ? superclass->idx : -1);
                        i += superclass->fields.size();  // FIXME: not typechecking these .. 
                    }
                    else
                    {
                        Gen(cn->head(), 1);

                        auto &type = cn->head()->exptype;
                        if (struc) GenTypeCheck(type, struc->fields[i].type);
                        else GenTypeCheck(type, vtype->Element());

                        Emit(IL_PUSHONCE);
                        i++;
                    }

                    superclass = nullptr;
                }
                if (!retval) Emit(IL_POP);
                break;
            }

            case T_IS:
            {
                Gen(n->left(), retval);
                auto t = n->right()->typenode()->t;
                if (retval) Emit(IL_ISTYPE, t == V_STRUCT ? V_VECTOR : t, n->right()->typenode()->idx);
                break;
            }

            case T_RETURN:
            {
                int fid = n->return_function_idx()->integer();
                if (n->return_value()) Gen(n->return_value(), fid >= 0 ? st.functiontable[fid]->retvals : 1);
                else Emit(IL_PUSHUNDEF);
                Emit(IL_RETURN, fid);
                // retval==true is nonsensical here, but can't enforce
                break;
            }
            case T_COCLOSURE:
                if (retval) Emit(IL_COCL);
                break;

            case T_COROUTINE:
            {
                // TODO: maybe more correct to generate call regardless of retval?
                if (retval) Emit(IL_CORO, 0);
                MARKL(loc);

                if (retval)
                {
                    bool found = false;
                    Emit(0); // count
                    // TODO: we shouldn't need to compute and store this table for each call, instead do it once for
                    // each function / builtin function
                    auto err = n->child()->FindIdentsUpToYield([&](vector<Ident *> &istack)
                    {
                        found = true;
                        for (auto id : istack)
                        {
                            for (size_t i = loc + 1; i < code.size(); i++) if (code[i] == id->idx) continue;
                            // FIXME: merging of variables from all yield sites is potentially incorrect, we might end
                            // up restoring variables that are not actually in use
                            Emit(id->idx);
                            //printf("cor: %s\n", id->name.c_str());
                        }
                    });

                    if (err)
                        parser.Error(string("coroutine construction error: ") + err, n->child());

                    // this guarantees FindIdentsUpToYield has done an accurate job finding all ids, since if it can
                    // reach the yield, it must also have found the whole callchain leading up to it
                    // if people start storing the yield function inside data structures or doing other weird things to
                    // confuse the algorithm, they'll at least get this error
                    if (!found)
                        parser.Error("coroutine construction error: cannot find yield call", n->child());
                    code[loc] = code.size() - loc - 1;
                }

                Gen(n->child(), retval);

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

    void GenAssign(Node *lval, int lvalop, int retval, Node *rhs = nullptr)
    {
        if (retval) lvalop++;
        if (rhs) Gen(rhs, 1);
        switch (lval->type)
        {
            case T_IDENT: Emit(IL_LVALVAR, lvalop, lval->ident()->idx); break;
            case T_DOT:   Gen(lval->left(), 1); GenFieldAccess(lval->right()->fld(), lvalop); break;
            case T_CO_AT: Gen(lval->coroutine_at(), 1); Emit(IL_LVALLOC, lvalop, lval->coroutine_var()->ident()->idx); break;
            case T_INDEX: Gen(lval->left(), 1); Gen(lval->right(), 1); Emit(IL_LVALIDX, lvalop); break;
            default:    parser.Error("lvalue required", lval);
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

            // see if of the two offsets, one of them only is used with one type, then we can use a simple if-check
            // encoding
            if (f->numunique == 2)
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
            if (condfields.length())
                printf("performance warning: conditionals generated for fields:%s\n", condfields.c_str());
            if (tablefields.length())
                printf("performance warning: table lookups generated for fields:%s (in %ld types)\n",
                       tablefields.c_str(), long(st.structtable.size()));
        }
    }

    #undef MARKL
    #undef SETL
};
