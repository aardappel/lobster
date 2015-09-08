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

struct CodeGen 
{
    vector<int> code;
    vector<bytecode::LineInfo> lineinfo;
    vector<bytecode::SpecIdent> sids;
    Parser &parser;
    vector<const Node *> linenumbernodes;
    vector<pair<int, const SubFunction *>> call_fixups;
    SymbolTable &st;
    
    vector<type_elem_t> type_table, vint_typeoffsets, vfloat_typeoffsets;
    map<vector<type_elem_t>, type_elem_t> type_lookup;  // Wasteful, but simple.

    int Pos() { return (int)code.size(); }

    void Emit(int i)
    {
        auto &ln = linenumbernodes.back()->line;
        if (lineinfo.empty() || ln.line != lineinfo.back().line() || ln.fileidx != lineinfo.back().fileidx())
            lineinfo.push_back(bytecode::LineInfo(ln.line, ln.fileidx, Pos()));
        code.push_back(i);
    }

    void Emit(int i, int j) { Emit(i); Emit(j); }
    void Emit(int i, int j, int k) { Emit(i); Emit(j); Emit(k); }
    void Emit(int i, int j, int k, int l) { Emit(i); Emit(j); Emit(k); Emit(l); }

    #define MARKL(name) auto name = Pos();
    #define SETL(name) code[name - 1] = Pos();

    // Make a table for use as VM runtime type.
    type_elem_t GetTypeTableOffset(TypeRef type)
    {
        vector<type_elem_t> tt;
        tt.push_back((type_elem_t)type->t);

        switch (type->t)
        {
            case V_NIL:
            case V_VECTOR:
                tt.push_back(GetTypeTableOffset(type->sub));
                break;

            case V_STRUCT:
                if (type->struc->typeinfo >= 0)
                    return type->struc->typeinfo;

                type->struc->typeinfo = (type_elem_t)type_table.size();
                tt.push_back((type_elem_t)type->struc->idx);
                // Reserve space, so other types can be added afterwards safely.
                type_table.insert(type_table.end(), type->struc->fields.size() + 2, (type_elem_t)0);

                for (auto &field : type->struc->fields.v)
                {
                    tt.push_back(GetTypeTableOffset(field.type));
                }

                std::copy(tt.begin(), tt.end(), type_table.begin() + type->struc->typeinfo);
                return type->struc->typeinfo;

            case V_VAR:
                // This can happen with an empty [] vector that was never bound to anything.
                // Should be benign to use any, since it is never accessed anywhere.
                // FIXME: would be even better to check this case before codegen, since this may mask bugs.
                return GetTypeTableOffset(type_any);

            default:
                assert(IsRuntime(type->t));
                break;
        }
        // For everything that's not a struct:
        auto it = type_lookup.find(tt);
        if (it != type_lookup.end()) return it->second;
        auto offset = (type_elem_t)type_table.size();
        type_lookup[tt] = offset;
        type_table.insert(type_table.end(), tt.begin(), tt.end());
        return offset;
    }

    CodeGen(Parser &_p, SymbolTable &_st) : parser(_p), st(_st)
    {
        // Pre-load some types into the table, must correspond to order of type_elem_t enums.
                                                    GetTypeTableOffset(type_int);
                                                    GetTypeTableOffset(type_float);
        Type type_boxedint(V_BOXEDINT);             GetTypeTableOffset(&type_boxedint);
        Type type_boxedfloat(V_BOXEDFLOAT);         GetTypeTableOffset(&type_boxedfloat);
                                                    GetTypeTableOffset(type_string);
                                                    GetTypeTableOffset(type_coroutine);
                                                    GetTypeTableOffset(type_any);
        Type type_cycledone(V_CYCLEDONE);           GetTypeTableOffset(&type_cycledone);
        Type type_valuebuf(V_VALUEBUF);             GetTypeTableOffset(&type_valuebuf);
        Type type_stackframebuf(V_STACKFRAMEBUF);   GetTypeTableOffset(&type_stackframebuf);
                                                    GetTypeTableOffset(type_vector_int);
                                                    GetTypeTableOffset(type_vector_float);
        Type type_vec_str(V_VECTOR, &*type_string); GetTypeTableOffset(&type_vec_str);
        assert(type_table.size() == TYPE_ELEM_FIXED_OFFSET_END);

        for (auto type : st.default_int_vector_types)
            vint_typeoffsets.push_back(!type.Null() ? GetTypeTableOffset(type) : (type_elem_t)-1);
        for (auto type : st.default_float_vector_types)
            vfloat_typeoffsets.push_back(!type.Null() ? GetTypeTableOffset(type) : (type_elem_t)-1);

        for (auto sid : st.specidents) sids.push_back(bytecode::SpecIdent(sid->id->idx, GetTypeTableOffset(sid->type)));

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
            if (f->subf && f->subf->typechecked)
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

    void Dummy(int retval) { while (retval--) Emit(IL_PUSHNIL); }

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

                if (ta != tb) return !(ta < tb);  // V_ANY must be last.
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
                auto gendispatch = [&] (int override_j, TypeRef override_type)
                {
                    Output(OUTPUT_DEBUG, "dispatch %s", f.name.c_str());
                    for (int j = 0; j < f.nargs(); j++)
                    {
                        auto type = j == override_j ? override_type : sf->args.v[j].type;
                        Output(OUTPUT_DEBUG, "arg %d: %s", j, TypeName(type).c_str());
                        Emit(GetTypeTableOffset(type));
                    }
                    Emit(sf->subbytecodestart);
                    numentries++;
                };
                // Generate regular dispatch entry.
                gendispatch(-1, nullptr);
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
                            gendispatch(j, &subs->thistype);
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

        vector<SpecIdent *> defs;
        vector<SpecIdent *> logvars;
        // FIXME: replace this with sf.locals and sf.dynscoperedefs, but be careful logvar order stays the same
        for (auto topl = sf.body; topl; topl = topl->tail())
        {
            size_t logmultiassignstart = logvars.size();
            for (auto dl = topl->head(); dl->type == T_DEF; dl = dl->right())
            {
                auto sid = dl->left()->sid();
                auto id = dl->left()->ident();
                if (id->logvaridx >= 0)
                {
                    id->logvaridx = (int)logvars.size();
                    logvars.push_back(sid);
                }
                else defs.push_back(sid);
            }
            // order of multi-assign initializers is reversed on the stack
            reverse(logvars.begin() + logmultiassignstart, logvars.end());
        }

        linenumbernodes.push_back(sf.body);

        Emit(IL_FUNSTART);
        Emit((int)sf.args.v.size()); 
        for (auto &arg : sf.args.v) Emit(arg.sid->idx);
        // FIXME: we now have sf.dynscoperedefs, so we could emit them seperately, and thus optimize function calls
        Emit((int)(defs.size() + logvars.size()));
        for (auto id : defs) Emit(id->idx);
        for (auto id : logvars) Emit(id->idx);
        Emit((int)logvars.size());

        if (sf.body) BodyGen(sf.body);
        else Dummy(true);

        Emit(IL_FUNEND);

        linenumbernodes.pop_back();
    }

    void GenFixup(const SubFunction *sf)
    {
        if (!sf->subbytecodestart) call_fixups.push_back(make_pair(Pos() - 1, sf));
    }

    const Node *GenArgs(const Node *list, int &nargs, const Node *parent = nullptr)
    {
        // Skip unused args, this may happen for dynamic calls.
        const Node *lastarg = nullptr;
        for (; list; list = list->tail())
        {
            Gen(list->head(), 1, parent);
            lastarg = list->head();
            nargs++;
        }
        return lastarg;
    };

    int GenCall(const SubFunction &sf, const Node *args, const Node *errnode, int &nargs)
    {
        auto &f = *sf.parent;
        GenArgs(args, nargs);
        if (f.nargs() != nargs)
            parser.Error("call to function " + f.name + " needs " + to_string(f.nargs()) +
                         " arguments, " + to_string(nargs) + " given", errnode);
        Emit(f.multimethod ? IL_CALLMULTI : IL_CALL,
             nargs,
             f.idx,
             f.multimethod ? f.bytecodestart : sf.subbytecodestart);
        GenFixup(&sf);
        return max(f.retvals, 1);
    };

    void GenFloat(float f) { Emit(IL_PUSHFLT); int2float i2f; i2f.f = f; Emit(i2f.i); }

    void Gen(const Node *n, int retval, const Node *parent = nullptr)
    {
        linenumbernodes.push_back(n);
        // by default, the cases below only deal with 0 or 1 retvals,
        // set this to > 1 to indicate extra values on the stack need to be dealt with
        int maxretvalsupplied = 1;
        int opc = 0;

        switch(n->type)
        {
            case T_INT:   if (retval) { Emit(IL_PUSHINT, n->integer()); }; break;
            case T_FLOAT: if (retval) { GenFloat((float)n->flt()); }; break;
            case T_STR:   if (retval) { Emit(IL_PUSHSTR); for (const char *p = n->str(); *p; p++) Emit(*p); Emit(0); }; break;
            case T_NIL:   if (retval) { Emit(IL_PUSHNIL); break; }

            case T_DEFAULTVAL:
            {
                assert(n->exptype->t == V_NIL);  // Optional args are indicated by being nillable.
                if (retval) switch (n->exptype->sub->t)
                {
                    case V_INT: Emit(IL_PUSHINT, 0); break;
                    case V_FLOAT: GenFloat(0); break;
                    default: Emit(IL_PUSHNIL); break;
                }
                break;
            }

            case T_IDENT:  if (retval) { Emit(IL_PUSHVAR, n->sid()->idx); }; break;

            case T_DOT:
            case T_DOTMAYBE:
                Gen(n->left(), retval);
                if (retval) GenFieldAccess(n->right(), -1, n->type == T_DOTMAYBE);
                break;

            case T_INDEX:
                Gen(n->left(), retval);
                Gen(n->right(), retval);
                if (retval) Emit(n->right()->exptype->t == V_INT ? IL_PUSHIDXI : IL_PUSHIDXV);
                break;

            case T_CODOT:
                Gen(n->left(), retval);
                if (retval) Emit(IL_PUSHLOC, n->right()->sid()->idx);
                break;

            case T_DEF:
            case T_ASSIGNLIST:
            {
                auto dl = n;
                vector<Node *> defs;
                for (; dl->type == T_DEF || dl->type == T_ASSIGNLIST; dl = dl->right())
                {
                    defs.push_back(dl->left());
                }
                Gen(dl, (int)defs.size());
                dl = n;
                for (int i = (int)defs.size() - 1; i >= 0; i--)
                {
                    if (n->type == T_DEF)
                    {
                        if (defs[i]->ident()->logvaridx >= 0) Emit(IL_LOGREAD, defs[i]->ident()->logvaridx);
                    }
                    Emit(IL_LVALVAR, IsRefNil(defs[i]->exptype->t) ? LVO_WRITEREF : LVO_WRITE, defs[i]->sid()->idx);
                }
                // currently can only happen with def on last line of body, which is nonsensical
                Dummy(retval);
                break;
            }

            case T_ASSIGN:
                GenAssign(n->left(),
                          IsRefNil(n->left()->exptype->t) ? LVO_WRITEREF : LVO_WRITE, retval,
                          nullptr,
                          n->right());
                break;

            case T_PLUSEQ:  GenAssign(n->left(), LVO_IADD, retval, n->exptype, n->right()); break;
            case T_MINUSEQ: GenAssign(n->left(), LVO_ISUB, retval, n->exptype, n->right()); break;
            case T_MULTEQ:  GenAssign(n->left(), LVO_IMUL, retval, n->exptype, n->right()); break;
            case T_DIVEQ:   GenAssign(n->left(), LVO_IDIV, retval, n->exptype, n->right()); break;
            case T_MODEQ:   GenAssign(n->left(), LVO_IMOD, retval, n->exptype, n->right()); break;

            case T_POSTDECR: GenAssign(n->child(), LVO_IMMP, retval, n->exptype); break;
            case T_POSTINCR: GenAssign(n->child(), LVO_IPPP, retval, n->exptype); break;
            case T_DECR:     GenAssign(n->child(), LVO_IMM,  retval, n->exptype); break;
            case T_INCR:     GenAssign(n->child(), LVO_IPP,  retval, n->exptype); break;

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
                    // Have to check right and left because comparison ops generate ints for node overall.
                    if      (n->right()->exptype->t == V_INT    && n->left()->exptype->t == V_INT)    Emit(IL_IADD + opc);
                    else if (n->right()->exptype->t == V_FLOAT  && n->left()->exptype->t == V_FLOAT)  Emit(IL_FADD + opc);
                    else if (n->right()->exptype->t == V_STRING && n->left()->exptype->t == V_STRING) Emit(IL_SADD + opc);
                    else
                    {
                        if (opc >= 9)  // EQ/NEQ
                        {
                            Emit(IL_AEQ + opc - 9);
                        }
                        else
                        {
                            // If this is a comparison op, be sure to use the child type.
                            TypeRef vectype = opc >= 5 ? n->left()->exptype : n->exptype;
                            TypeRef sub;
                            if      (vectype->t == V_VECTOR) sub = vectype->sub;
                            else if (vectype->t == V_STRUCT) sub = vectype->struc->vectortype->sub;
                            else assert(false);
                            bool withscalar = IsScalar(n->right()->exptype->t);
                            if      (sub->t == V_INT)    Emit((withscalar ? IL_IVSADD : IL_IVVADD) + opc);
                            else if (sub->t == V_FLOAT)  Emit((withscalar ? IL_FVSADD : IL_FVVADD) + opc);
                            else assert(false);
                        }
                    }
                }
                break;

            case T_UMINUS:
                Gen(n->child(), retval);
                if (retval)
                {
                    auto type = n->child()->exptype;
                    switch (type->t)
                    {
                        case V_INT: Emit(IL_IUMINUS); break;
                        case V_FLOAT: Emit(IL_FUMINUS); break;
                        case V_STRUCT: 
                        case V_VECTOR:
                        {
                            auto elem = type->t == V_VECTOR
                                        ? type->Element()->t 
                                        : type->struc->vectortype->Element()->t;
                            Emit(elem == V_INT ? IL_IVUMINUS : IL_FVUMINUS);
                            break;
                        }
                        default: assert(false);
                    }
                }
                break;

            case T_I2F:
                Gen(n->child(), retval);
                if (retval) Emit(IL_I2F);
                break;

            case T_A2S:
                Gen(n->child(), retval);
                if (retval) Emit(IL_A2S);
                break;

            case T_A2A:
                Gen(n->child(), retval);
                if (retval)
                {
                    switch (n->child()->exptype->t)
                    {
                        case V_INT:   Emit(IL_I2A); break;
                        case V_FLOAT: Emit(IL_F2A); break;
                        default: assert(0);
                    }
                }
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
                    auto lastarg = GenArgs(n->ncall_args(), nargs, n);
                    assert(nargs == (int)nf->args.size());
                    if (nf->ncm == NCM_CONT_EXIT)  // graphics.h
                    {   
                        Emit(IL_BCALL, nf->idx);
                        if (lastarg->type != T_DEFAULTVAL) // FIXME: this will not work if its a var with nil value
                        {
                            Emit(IL_CALLVCOND, 0);
                            Emit(IL_CONT1, nf->idx);
                        }
                    }
                    else
                    {
                        Emit(IL_BCALL, nf->idx);
                    }
                    if (nf->retvals.v.size() > 1)
                    {
                        maxretvalsupplied = (int)nf->retvals.v.size();
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
                        if (n->dcall_args())
                        {
                            GenArgs(n->dcall_args(), nargs);
                            assert(nargs == 1);
                        }
                        else
                        {
                            Emit(IL_PUSHNIL);
                        }
                        Emit(IL_YIELD);
                    }
                    else
                    {
                        auto sf = n->dcall_fval()->exptype->sf;
                        auto spec_sf = n->dcall_function()->sf();
                        (void)spec_sf;
                        assert(sf && sf == spec_sf);
                        // FIXME: in the future, we can make a special case for istype calls.
                        if (!sf->parent->istype)
                        {
                            // We statically know which function this is calling, which means that we don't have
                            // to need function value, but we generate code for it for the rare case it contains a
                            // side effect, usually it is an ident which will result in no code (retval = 0).
                            Gen(n->dcall_fval(), 0);
                            // We can now turn this into a normal call.
                            maxretvalsupplied = GenCall(*sf, n->dcall_args(), n, nargs);
                        }
                        else
                        {
                            GenArgs(n->dcall_args(), nargs);
                            assert(nargs == (int)sf->args.size());
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
                bool has_else = n->if_else()->type != T_DEFAULTVAL;
                Emit(!has_else && retval ? IL_JUMPFAILN : IL_JUMPFAIL, 0);
                MARKL(loc);
                if (has_else)
                {
                    Gen(n->if_then(), retval);
                    Emit(IL_JUMP, 0);
                    MARKL(loc2);
                    SETL(loc);
                    Gen(n->if_else(), retval);
                    SETL(loc2);
                }
                else
                {
                    Gen(n->if_then(), false);
                    Dummy(retval);  // This potentially generates a pop/push combo, but important we return nil.
                    SETL(loc);
                }
                break;
            }

            case T_WHILE:
            {
                MARKL(loopback);
                Gen(n->while_condition(), 1);
                Emit(IL_JUMPFAIL, 0);
                MARKL(jumpout);
                Gen(n->while_body(), 0);
                Emit(IL_JUMP, loopback);
                SETL(jumpout);
                Dummy(retval);
                break;
            }

            case T_FOR:
            {
                Emit(IL_PUSHINT, -1);   // i
                Gen(n->for_iter(), 1);
                Gen(n->for_body()->call_function(), 1);  // FIXME: inline this somehow.
                Emit(IL_PUSHNIL);     // body retval
                auto type = n->for_iter()->exptype;
                switch (type->t)
                {
                    case V_INT: Emit(IL_IFOR); break;
                    case V_STRING: Emit(IL_SFOR); break;
                    case V_VECTOR: Emit(IL_VFOR); break;
                    case V_STRUCT: assert(type->struc->vectortype->t == V_VECTOR); Emit(IL_VFOR); break;
                    default: assert(false);
                }
                Dummy(retval);
                break;
            }

            case T_CONSTRUCTOR:
            {
                int i = 0;
                // FIXME: a malicious script can exploit this for a stack overflow.
                for (auto cn = n->constructor_args(); cn; cn = cn->tail())
                {
                    assert(cn->type == T_LIST);
                    Gen(cn->head(), 1);
                    i++;
                }
                auto vtype = n->exptype;
                auto offset = GetTypeTableOffset(vtype);
                if (vtype->t == V_STRUCT)
                {
                    assert((int)vtype->struc->fields.size() == i);
                }
                else
                {
                    assert(vtype->t == V_VECTOR);
                }
                Emit(IL_NEWVEC, offset, i);
                if (!retval) Emit(IL_POP);
                break;
            }

            case T_IS:
            {
                Gen(n->left(), retval);
                // If the value was a scalar, then it always results in a compile time type check, which means
                // this T_IS would have been optimized out. Which means from here on we can assume its a ref.
                assert(!IsScalar(n->left()->exptype->t));
                if (retval)
                {
                    Emit(IL_ISTYPE, GetTypeTableOffset(n->right()->typenode()));
                }
                break;
            }

            case T_RETURN:
            {
                int fid = n->return_function_idx()->integer();
                if (n->return_value()) Gen(n->return_value(), fid >= 0 ? st.functiontable[fid]->retvals : 1);
                else Emit(IL_PUSHNIL);
                Emit(IL_RETURN, fid, fid >= 0 ? st.functiontable[fid]->retvals : 1);
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

                // TODO: we shouldn't need to store this table for each call, instead do it once for each function
                Emit(0); // count
                for (auto &arg : sf->coyieldsave.v) Emit(arg.sid->idx);

                code[loc] = Pos() - loc - 1;

                Gen(n->child(), retval);

                Emit(IL_COEND);
                SETL(loc);

                if (!retval) Emit(IL_POP);
                break;
            }

            case T_TYPEOF:
            {
                if (n->child())
                {
                    Emit(IL_PUSHINT, GetTypeTableOffset(n->child()->type == T_IDENT
                                                        ? n->child()->exptype
                                                        : n->child()->typenode()));
                }
                else
                {
                    if (!parent || parent->type != T_NATCALL) parser.Error("typeof return out of call context", n);
                    Emit(IL_PUSHINT, GetTypeTableOffset(parent->exptype));
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

    void GenAssign(const Node *lval, int lvalop, int retval, TypeRef type, const Node *rhs = nullptr)
    {
        if (lvalop >= LVO_IADD && lvalop <= LVO_IMOD)
        {
            if      (type->t == V_INT)    {                                                            }
            else if (type->t == V_FLOAT)  { assert(lvalop != LVO_IMOD); lvalop += LVO_FADD - LVO_IADD; }
            else if (type->t == V_STRING) { assert(lvalop == LVO_IADD); lvalop = LVO_SADD; }
            else
            {
                TypeRef sub;
                if      (type->t == V_VECTOR) sub = type->sub;
                else if (type->t == V_STRUCT) sub = type->struc->vectortype->sub;
                else assert(false);
                bool withscalar = IsScalar(rhs->exptype->t);
                if      (sub->t == V_INT)    {                             lvalop += (withscalar ? LVO_IVSADD : LVO_IVVADD) - LVO_IADD; }
                else if (sub->t == V_FLOAT)  { assert(lvalop != LVO_IMOD); lvalop += (withscalar ? LVO_FVSADD : LVO_FVVADD) - LVO_IADD; }
                else assert(false);
            }
        }
        else if (lvalop >= LVO_IPP && lvalop <= LVO_IMMP)
        {
            if (type->t == V_FLOAT) lvalop += LVO_FPP - LVO_IPP;
            else assert(type->t == V_INT);
        }
        if (retval) lvalop++;
        if (rhs) Gen(rhs, 1);
        switch (lval->type)
        {
            case T_IDENT: Emit(IL_LVALVAR, lvalop, lval->sid()->idx); break;
            case T_DOT:   Gen(lval->left(), 1); GenFieldAccess(lval->right(), lvalop, false); break;
            case T_CODOT: Gen(lval->left(), 1); Emit(IL_LVALLOC, lvalop, lval->right()->sid()->idx); break;
            case T_INDEX: Gen(lval->left(), 1); Gen(lval->right(), 1);
                          Emit(lval->right()->exptype->t == V_INT ? IL_LVALIDXI : IL_LVALIDXV, lvalop);
                          break;
            default:      parser.Error("lvalue required", lval);
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
