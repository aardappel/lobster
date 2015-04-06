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

namespace lobster
{

#define ILNAMES \
    F(PUSHINT) \
    F(PUSHFLT) \
    F(PUSHSTR) \
    F(PUSHUNDEF) \
    F(PUSHNIL) \
    F(PUSHFUN) \
    F(PUSHVAR) F(LVALVAR) \
    F(PUSHIDX) F(LVALIDX) \
    F(PUSHFLD) F(PUSHFLDM) F(LVALFLD) \
    F(PUSHLOC) F(LVALLOC) \
    F(BCALL) \
    F(CALL) F(CALLV) F(CALLVCOND) F(YIELD) F(DUP) F(CONT1) \
    F(FUNSTART) F(FUNEND) F(FUNMULTI) F(CALLMULTI) \
    F(JUMP) \
    F(NEWVEC) \
    F(POP) \
    F(EXIT) \
    F(IADD) F(ISUB) F(IMUL) F(IDIV) F(IMOD) F(ILT) F(IGT) F(ILE) F(IGE) F(IEQ) F(INE) \
    F(FADD) F(FSUB) F(FMUL) F(FDIV) F(FMOD) F(FLT) F(FGT) F(FLE) F(FGE) F(FEQ) F(FNE) \
    F(AADD) F(ASUB) F(AMUL) F(ADIV) F(AMOD) F(ALT) F(AGT) F(ALE) F(AGE) F(AEQ) F(ANE) \
    F(UMINUS) F(LOGNOT) F(I2F) F(A2S) F(JUMPFAIL) F(JUMPFAILR) F(JUMPNOFAIL) F(JUMPNOFAILR) F(RETURN) F(FOR) \
    F(PUSHONCE) \
    F(ISTYPE) F(CORO) F(COCL) F(COEND) \
    F(LOGREAD)

#define F(N) IL_##N,
enum { ILNAMES };
#undef F

#define LVALOPNAMES \
    F(WRITE) F(WRITER) F(WRITED) \
    F(AADD) F(AADDR) F(ASUB) F(ASUBR) F(AMUL) F(AMULR) F(ADIV) F(ADIVR) F(AMOD) F(AMODR) \
    F(IADD) F(IADDR) F(ISUB) F(ISUBR) F(IMUL) F(IMULR) F(IDIV) F(IDIVR) F(IMOD) F(IMODR) \
    F(FADD) F(FADDR) F(FSUB) F(FSUBR) F(FMUL) F(FMULR) F(FDIV) F(FDIVR) \
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
    vector<const Node *> linenumbernodes;
    vector<pair<int, const SubFunction *>> call_fixups;
    SymbolTable &st;

    void Emit(int i)
    {
        int l = linenumbernodes.back()->linenumber;
        int f = linenumbernodes.back()->fileidx;
        if (lineinfo.empty() || l != lineinfo.back().line || f != lineinfo.back().fileidx)
            lineinfo.push_back(LineInfo(l, f, code.size()));
        code.push_back(i);
    }

    void Emit(int i, int j) { Emit(i); Emit(j); }
    void Emit(int i, int j, int k) { Emit(i); Emit(j); Emit(k); }
    void Emit(int i, int j, int k, int l) { Emit(i); Emit(j); Emit(k); Emit(l); }

    int Pos() { return (int)code.size(); }

    #define MARKL(name) auto name = Pos();
    #define SETL(name) code[name - 1] = Pos();

    CodeGen(Parser &_p, SymbolTable &_st, vector<int> &_code, vector<LineInfo> &_lineinfo)
        : code(_code), lineinfo(_lineinfo), lex(_p.lex), parser(_p), st(_st)
    {
        // Create list of subclasses, to help in creation of dispatch tables.
        for (auto struc : st.structtable)
        {
            if (struc->superclass)
            {
                struc->nextsubclass = struc->superclass->firstsubclass;
                struc->superclass->firstsubclass = struc;
            }
        }

        linenumbernodes.push_back(parser.root);

        Emit(IL_JUMP, 0);
        MARKL(fundefjump);
        for (auto f : parser.st.functiontable)
            if (f->subf->typechecked)
                GenFunction(*f);
        SETL(fundefjump);

        BodyGen(parser.root);
        Emit(IL_EXIT);

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
                auto ta = a->args.v[i].type;
                auto tb = b->args.v[i].type;

                if (ta != tb) return ta < tb;
            }
            cg->parser.Error("function signature overlap for " + f->name, nullptr);
            return false;
        }
    } sfcomparator;


    bool GenFunction(Function &f)
    {
        if (f.bytecodestart > 0 || f.istype) return false;

        if (!f.multimethod)
        {
            f.bytecodestart = Pos();
            for (auto sf = f.subf; sf; sf = sf->next)
            {
                GenScope(*sf);
            }
        }
        else
        {
            // do multi-dispatch
            vector<SubFunction *> sfs;

            for (auto sf = f.subf; sf; sf = sf->next)
            {
                sfs.push_back(sf);
                GenScope(*sf);
            }

            sfcomparator.nargs = f.nargs();
            sfcomparator.cg = this;
            sfcomparator.f = &f;
            sort(sfs.begin(), sfs.end(), sfcomparator);

            f.bytecodestart = Pos();
            int numentries = 0;
            MARKL(multistart);
            Emit(IL_FUNMULTI, 0, f.nargs());

            // FIXME: invent a much faster, more robust multi-dispatch mechanic.
            for (auto sf : sfs)
            {
                auto gendispatch = [&] (int override_j, int override_idx)
                {
                    for (int j = 0; j < f.nargs(); j++)
                    {
                        if (j == override_j)
                        {
                            Emit(V_VECTOR, override_idx);
                        }
                        else
                        {
                            auto arg = sf->args.v[j];
                            // FIXME: this probably doesn't cover all cases anymore..
                            if (arg.type->t == V_STRUCT) Emit(V_VECTOR, arg.type->struc->idx); 
                            else Emit(arg.type->t, -1);
                        }
                    }
                    Emit(sf->subbytecodestart);
                    numentries++;
                };
                // Generate regular dispatch entry.
                gendispatch(-1, -1);
                // See if this entry contains super-types and generate additional entries.
                for (int j = 0; j < f.nargs(); j++)
                {
                    auto arg = sf->args.v[j];
                    if (arg.type->t == V_STRUCT)
                    {
                        auto struc = arg.type->struc;
                        for (auto subs = struc->firstsubclass; subs; subs = subs->nextsubclass)
                        {
                            // See if this instance already exists:
                            for (auto osf : sfs)
                            {
                                // Only check this arg, not all arg, which is reasonable.
                                if (*osf->args.v[j].type == subs->thistype) goto skip;
                            }
                            gendispatch(j, subs->idx);
                            // FIXME: We should also call it on subtypes of subs.
                            skip:;
                        }
                    }
                }
            }
            code[multistart + 1] = numentries;
        }
        return true;
    }

    void GenScope(SubFunction &sf)
    {
        if (sf.subbytecodestart > 0) return;
        sf.subbytecodestart = Pos();

        if (!sf.typechecked)
        {
            auto s = Dump(*sf.body, 0);
            Output(OUTPUT_DEBUG, "untypechecked: %s : %s", sf.parent->name.c_str(), s.c_str());
            assert(0);
        }

        vector<Ident *> defs;
        vector<Ident *> logvars;
        // FIXME: replace this with sf.locals and sf.dynscoperedefs, but be careful logvar order stays the same
        for (auto topl = sf.body; topl; topl = topl->tail())
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

        linenumbernodes.push_back(sf.body);

        Emit(IL_FUNSTART);
        Emit((int)sf.args.v.size()); 
        for (auto arg : sf.args.v) Emit(arg.id->idx);
        // FIXME: we now have sf.dynscoperedefs, so we could emit them seperately, and thus optimize function calls
        Emit((int)(defs.size() + logvars.size()));
        for (auto id : defs) Emit(id->idx);
        for (auto id : logvars) Emit(id->idx);
        Emit((int)logvars.size());

        //for (auto idn : scope) GenTypeCheck(idn->ident->idx, idn->exptype);

        if (sf.body) BodyGen(sf.body);
        else Dummy(true);

        Emit(IL_FUNEND);

        linenumbernodes.pop_back();
    }

    void GenInlineScope(const Node *cl, int retval)
    {
        // FIXME: should NOT need a call here, vars need to be moved to outer scope
        // FIXME: is it guaranteed that someone can't call if(a,b,c) ? do we want to allow it?
        assert(cl->type == T_FUN);
        auto sf = cl->sf();
        int nargs = 0;
        auto returned = GenCall(*sf, nullptr, sf->body, nargs);
        assert(returned == 1);
        (void)returned;
        if (!retval) Emit(IL_POP);  // FIXME: always the case with while body
    }

    void GenFixup(const SubFunction *sf)
    {
        if (!sf->subbytecodestart) call_fixups.push_back(make_pair(Pos() - 1, sf));
    }

    const Node *GenArgs(const Node *list, const ArgVector *args, int &nargs)
    {
        // Skip unused args, this may happen for dynamic calls.
        const Node *lastarg = nullptr;
        for (; list && (!args || nargs < (int)args->v.size()); list = list->tail())
        {
            Gen(list->head(), 1);
            lastarg = list->head();
            nargs++;
        }
        return lastarg;
    };

    int GenCall(const SubFunction &sf, const Node *args, const Node *errnode, int &nargs)
    {
        auto &f = *sf.parent;
        GenArgs(args, &sf.args, nargs);
        if (f.nargs() != nargs)
            parser.Error("call to function " + f.name + " needs " + string(inttoa(f.nargs())) +
            " arguments, " + string(inttoa(nargs)) + " given", errnode);
        Emit(f.multimethod ? IL_CALLMULTI : IL_CALL,
             nargs,
             f.idx,
             f.multimethod ? f.bytecodestart : sf.subbytecodestart);
        GenFixup(&sf);
        return max(f.retvals, 1);
    };

    void Gen(const Node *n, int retval)
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
            case T_DOTMAYBE:
                Gen(n->left(), retval);
                if (retval) GenFieldAccess(n->right(), -1, n->type == T_DOTMAYBE);
                break;

            case T_INDEX:
                Gen(n->left(), retval);
                Gen(n->right(), retval);
                if (retval) Emit(IL_PUSHIDX);
                break;

            case T_CODOT:
                Gen(n->left(), retval);
                if (retval) Emit(IL_PUSHLOC, n->right()->ident()->idx);
                break;

            case T_DEF:
            case T_ASSIGNLIST:
            {
                auto dl = n;
                vector<Ident *> ids;
                for (; dl->type == T_DEF || dl->type == T_ASSIGNLIST; dl = dl->right())
                {
                    ids.push_back(dl->left()->ident());
                }
                Gen(dl, ids.size());
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
                Dummy(retval);
                break;
            }

            case T_ASSIGN: GenAssign(n->left(), LVO_WRITE, retval, nullptr, n->right()); break;

            case T_PLUSEQ:  GenAssign(n->left(), LVO_AADD, retval, n->exptype, n->right()); break;
            case T_MINUSEQ: GenAssign(n->left(), LVO_ASUB, retval, n->exptype, n->right()); break;
            case T_MULTEQ:  GenAssign(n->left(), LVO_AMUL, retval, n->exptype, n->right()); break;
            case T_DIVEQ:   GenAssign(n->left(), LVO_ADIV, retval, n->exptype, n->right()); break;
            case T_MODEQ:   GenAssign(n->left(), LVO_AMOD, retval, n->exptype, n->right()); break;

            case T_POSTDECR: GenAssign(n->child(), LVO_MMP, retval, nullptr); break;
            case T_POSTINCR: GenAssign(n->child(), LVO_PPP, retval, nullptr); break;
            case T_DECR:     GenAssign(n->child(), LVO_MM,  retval, nullptr); break;
            case T_INCR:     GenAssign(n->child(), LVO_PP,  retval, nullptr); break;

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
                    // Have to check node and left because comparison ops generate ints
                    if      (n->right()->exptype->t == V_INT   && n->left()->exptype->t == V_INT)   Emit(IL_IADD + opc);
                    else if (n->right()->exptype->t == V_FLOAT && n->left()->exptype->t == V_FLOAT) Emit(IL_FADD + opc);
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

            case T_FUN:
                if (retval) 
                {
                    if (n->sf()->parent->anonymous)
                    {
                        Emit(IL_PUSHFUN, n->sf()->subbytecodestart);
                        GenFixup(n->sf());
                    }
                    else
                    {
                        Dummy(retval);
                    }
                }
                break; 

            case T_STRUCTDEF:
                Dummy(retval);
                break;
            
            case T_NATCALL:
            case T_CALL:
            case T_DYNCALL:     
            {
                int nargs = 0;
                if (n->type == T_NATCALL)
                {
                    auto nf = n->ncall_id()->nf();
                    // TODO: could pass arg types in here if most exps have types, cheaper than doing it all in call
                    // instruction?
                    auto lastarg = GenArgs(n->ncall_args(), nullptr, nargs);
                    if (nf->ncm == NCM_CONT_EXIT)  // graphics.h
                    {   
                        Emit(IL_BCALL, nf->idx, nargs);
                        if (lastarg->type != T_NIL) // FIXME: this will not work if its a var with nil value
                        {
                            Emit(IL_CALLVCOND, 0);
                            Emit(IL_CONT1, nf->idx);
                        }
                    }
                    else
                    {
                        Emit(IL_BCALL, nf->idx, nargs);
                    }
                    if (nf->retvals.v.size() > 1)
                    {
                        maxretvalsupplied = nf->retvals.v.size();
                    }
                    else if (!nf->retvals.v.size() && retval)
                    { 
                        // can't make this an error since these functions are often called as the last thing in a
                        // function, requiring a return value
                        //Error("builtin function call returns nothing", n); 
                    }
                }
                else if (n->type == T_CALL)
                {
                    maxretvalsupplied = GenCall(*n->call_function()->sf(), n->call_args(), n->call_function(), nargs);
                }
                else
                {
                    assert(n->type == T_DYNCALL);

                    if (n->dcall_fval()->exptype->t == V_YIELD)
                    {
                        GenArgs(n->dcall_info()->dcall_args(), nullptr, nargs);
                        if (!nargs) Emit(IL_PUSHNIL);
                        Emit(IL_YIELD);
                    }
                    else
                    {
                        auto sf = n->dcall_fval()->exptype->sf;
                        auto spec_sf = n->dcall_info()->dcall_function()->sf();
                        // FIXME: in the future, we can make a special case for istype calls.
                        if (sf && !sf->parent->istype)
                        {
                            assert(sf == spec_sf);
                            // We statically know which function this is calling, which means that we don't have
                            // to need function value, but we generate code for it for the rare case it contains a
                            // side effect, usually it is an ident which will result in no code (retval = 0).
                            Gen(n->dcall_fval(), 0);
                            // We can now turn this into a normal call.
                            maxretvalsupplied = GenCall(*sf, n->dcall_info()->dcall_args(), n, nargs);
                        }
                        else
                        {
                            // Fully dynamic call.
                            if (!sf)
                            {
                                // Don't support these in typechecked mode
                                if (!spec_sf)   // FIXME: if spec_sf is set, this is a call to nil function value.
                                    // e.g. focus in gui.lobster
                                {
                                    Output(OUTPUT_DEBUG, "dyncall: %s", Dump(*n, 0).c_str());
                                    assert(0);
                                }
                            }

                            GenArgs(n->dcall_info()->dcall_args(), nullptr, nargs);
                            Gen(n->dcall_fval(), 1);
                            Emit(IL_CALLV, nargs);
                        }
                    }
                }
                if (!retval) Emit(IL_POP);
                break;
            }  

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

            case T_IF:
            {
                Gen(n->if_condition(), 1);
                bool has_else = n->if_branches()->right()->type != T_NIL;
                // FIXME: if we need a dummy return value, it needs to be type compatible, otherwise refcount issues
                Emit(!has_else && retval ? IL_JUMPFAILR : IL_JUMPFAIL, 0);
                MARKL(loc);
                GenInlineScope(n->if_branches()->left(), retval);
                if (has_else)
                {
                    Emit(IL_JUMP, 0);
                    MARKL(loc2);
                    SETL(loc);
                    GenInlineScope(n->if_branches()->right(), retval);
                    SETL(loc2);
                }
                else
                {
                    SETL(loc);
                }
                break;
            }

            case T_WHILE:
            {
                MARKL(loopback);
                GenInlineScope(n->while_condition(), 1);
                Emit(IL_JUMPFAIL, 0);
                MARKL(jumpout);
                GenInlineScope(n->while_body(), 0);
                Emit(IL_JUMP, loopback);
                SETL(jumpout);
                Dummy(retval);
                break;
            }

            case T_FOR:
            {
                Emit(IL_PUSHINT, -1);   // i
                Gen(n->for_iter(), 1);
                Gen(n->for_body(), 1);  // FIXME: inline this somehow.
                Emit(IL_PUSHUNDEF);     // body retval
                Emit(IL_FOR);
                Dummy(retval);
                break;
            }

            case T_CONSTRUCTOR:
            {
                Struct *superclass = nullptr;
                Struct *struc = nullptr;

                auto vtype = n->constructor_type()->typenode();
                if (vtype->t == V_STRUCT)
                {
                    struc = vtype->struc;
                    Emit(IL_NEWVEC, struc->idx, struc->fields.size());
                    superclass = struc->superclass;
                }
                else
                {
                    int nargs = 0;
                    for (const Node *it = n->constructor_args(); it; it = it->tail()) nargs++;
                    Emit(IL_NEWVEC, V_VECTOR, nargs);
                }

                int i = 0;
                for (auto cn = n->constructor_args(); cn; cn = cn->tail())
                {
                    assert(cn->type == T_LIST);

                    Gen(cn->head(), 1);

                    Emit(IL_PUSHONCE);
                    i++;

                    superclass = nullptr;
                }
                if (!retval) Emit(IL_POP);
                break;
            }

            case T_IS:
            {
                Gen(n->left(), retval);
                auto t = n->right()->typenode()->t;
                // FIXME: this probably dpesn't cover all cases anymore
                if (retval)
                {
                    if (t == V_STRUCT) Emit(IL_ISTYPE, V_VECTOR, n->right()->typenode()->struc->idx);
                    else Emit(IL_ISTYPE, t, -1);
                }
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
                Emit(IL_CORO, 0);
                MARKL(loc);

                assert(n->exptype->t == V_COROUTINE);
                auto sf = n->exptype->sf;

                Emit(0); // count
                // TODO: we shouldn't need to compute and store this table for each call, instead do it once for
                // each function

                for (auto &arg : sf->coyieldsave.v) Emit(arg.id->idx);

                code[loc] = code.size() - loc - 1;

                Gen(n->child(), retval);

                Emit(IL_COEND);
                SETL(loc);

                if (!retval) Emit(IL_POP);
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

    void GenAssign(const Node *lval, int lvalop, int retval, TypeRef type, const Node *rhs = nullptr)
    {
        if (lvalop >= LVO_AADD && lvalop <= LVO_AMOD)
        {
            if      (type->t == V_INT)   {                             lvalop += LVO_IADD - LVO_AADD; }
            else if (type->t == V_FLOAT) { assert(lvalop != LVO_AMOD); lvalop += LVO_FADD - LVO_AADD; }
        }
        if (retval) lvalop++;
        if (rhs) Gen(rhs, 1);
        switch (lval->type)
        {
            case T_IDENT: Emit(IL_LVALVAR, lvalop, lval->ident()->idx); break;
            case T_DOT:   Gen(lval->left(), 1); GenFieldAccess(lval->right(), lvalop, false); break;
            case T_CODOT: Gen(lval->left(), 1); Emit(IL_LVALLOC, lvalop, lval->right()->ident()->idx); break;
            case T_INDEX: Gen(lval->left(), 1); Gen(lval->right(), 1); Emit(IL_LVALIDX, lvalop); break;
            default:    parser.Error("lvalue required", lval);
        }
    }

    void GenFieldAccess(Node *sfieldnode, int lvalop, bool maybe)
    {
        auto f = sfieldnode->fld();
        auto stype = sfieldnode->exptype;
        assert(stype->t == V_STRUCT);  // Ensured by typechecker.

        if (lvalop >= 0) Emit(IL_LVALFLD, lvalop);
        else Emit(IL_PUSHFLD + (int)maybe);
        
        auto idx = stype->struc->Has(f);
        assert(idx >= 0);
        Emit(idx);
    }

    #undef MARKL
    #undef SETL
};

}  // namespace lobster
