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

template <typename T> struct SlabAllocated
{
    #undef new
    void *operator new(size_t size)                         { return parserpool->alloc_small(size); }
    void *operator new(size_t size, int, const char *, int) { return parserpool->alloc_small(size); }
    void operator delete(void *p)                           { parserpool->dealloc_small(p); };
    void operator delete(void *p, int, const char *, int)   { parserpool->dealloc_small(p); }
    #ifdef WIN32
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #endif
    #endif
};

struct Node : SlabAllocated<Node>
{
    int type;

    union
    {
        struct { Node *a, *b;      }; // default
        int integer;                  // 'INT'
        double flt;                   // 'FLT'
        struct { char *str;        }; // 'STR'

        struct { Name *n;          }; // 'ID' | 'ST' | 'FLD' | 'FN'
        struct { Ident *ident;     }; // 'ID'
        struct { Struct *st;       }; // 'ST'
        struct { SharedField *fld; }; // 'FLD'
        struct { Function *f;      }; // 'FN'
        struct { NativeFun *nf;    }; // 'NF'
    };

    Type exptype;

    int linenumber;
    int fileidx;

    Node(Lex &lex, int _t)                     : type(_t),    a(NULL), b(NULL)                    { L(lex); };
    Node(Lex &lex, int _t, Node *_a)           : type(_t),    a(_a),   b(NULL)                    { L(lex); };
    Node(Lex &lex, int _t, Node *_a, Node *_b) : type(_t),    a(_a),   b(_b)                      { L(lex); };
    Node(Lex &lex, int _t, int _i)             : type(_t),    integer(_i)                         { L(lex); };
    Node(Lex &lex, int _t, double _f)          : type(_t),    flt(_f)                             { L(lex); };
    Node(Lex &lex, int _t, string &_s)         : type(_t),    str(parserpool->
                                                                  alloc_string_sized(_s.c_str())) { L(lex); };
    Node(Lex &lex, Ident *_id)                 : type('ID'),  ident(_id)                          { L(lex); };
    Node(Lex &lex, Struct *_st)                : type('ST'),  st(_st)                             { L(lex); };
    Node(Lex &lex, SharedField *_fld)          : type('FLD'), fld(_fld)                           { L(lex); };
    Node(Lex &lex, Function *_f)               : type('FN'),  f(_f)                               { L(lex); };
    Node(Lex &lex, NativeFun *_nf)             : type('NF'),  nf(_nf)                             { L(lex); };

    void L(Lex &lex)
    {
        linenumber = lex.errorline;
        fileidx = lex.fileidx;
    }

    bool HasChildren()
    {
        switch (type)
        {
            case 'INT':
            case 'FLT':
            case 'ID':
            case 'STR': 
            case 'ST':
            case 'FLD':
            case 'FN':
            case 'NF':
                return false;

            default:
                return true;
        }
    }

    ~Node()
    {
        if (type == 'STR')
        {
            parserpool->dealloc_sized(str);
        }
        else if (HasChildren())
        {
            if (a) delete a;
            if (b) delete b;
        }
    }
    
    bool IsConst()      // used to see if a var is worth outputting in a stacktrace
    {
        switch (type)
        {
            case 'INT':
            case 'FLT':
            case 'STR':
            case '{}':
                return true;
                
            case 'ID':
                return ident->static_constant;
                
            case '[]':
            {
                for (Node *n = a; n; n = n->b)
                {
                    if (!n->a->IsConst()) return false;
                }
                return true;
            }
                
            // TODO: support more types of exps?
                
            default:
                return false;
        }
    }

    /*
    void Iterate(const function<bool (Node *)> &f)
    {
        if (!f(this)) return;
        if (!HasChildren()) return;
        if (a) a->Iterate(f);
        if (b) b->Iterate(f);
    }
    */

    /*
    Node *Clone()
    {
        auto o = new Node(type);
        switch (type)
        {
            case 'INT': o->integer = integer; break;
            case 'FLT': o->flt = flt; break;
            case 'STR': o->str = pool->alloc_string(str); break;

            case 'ID':  
            case 'ST':  
            case 'FLD':
            case 'FN': 
            case 'NF':
                o->n = n;
                break;

            default:
                if (a) o->a = a->Clone();
                if (b) o->b = b->Clone();
                break;
        }
        return o;
    }
    */

    // this "evaluates" an exp, by iterating thru all subexps and thru function calls, ignoring recursive calls,
    // and tracking the value of idents as the value nodes they refer to
    const char *FindIdentsUpToYield(const function<void (vector<Ident *> &istack)> &customf)
    {
        vector<Ident *> istack;
        vector<Node *> vstack; // current value of each ident
        vector<Function *> fstack;

        const char *err = NULL;

        auto lookup = [&](Node *n) -> Node *
        {
            if (n->type == 'ID')
                for (size_t i = 0; i < istack.size(); i++) 
                    if (n->ident == istack[i])
                        return vstack[i];

            return n;
        };

        std::function<void(Node *)> eval;
        std::function<void(Node *, Node *)> evalblock;

        eval = [&](Node *n)
        {
            if (!n) return;

            //customf(n, istack);

            if (n->type == '{}')
                return;

            if (n->type == ',' && n->a->type == ':=')
            {
                eval(n->a);
                for (auto dl = n->a; dl->type == ':='; dl = dl->b)
                {
                    // FIXME: this is incorrect in the multiple assignments case, though not harmful
                    auto val = lookup(dl->b);
                    istack.push_back(dl->a->ident);
                    vstack.push_back(val);
                }
                eval (n->b);
                for (auto dl = n->a; dl->type == ':='; dl = dl->b)
                {
                    istack.pop_back();
                    vstack.pop_back();
                }
                return;
            }

            if (n->HasChildren())
            {
                eval(n->a);
                eval(n->b);
            }

            if (n->type == 'CALL')
            {
                for (auto f : fstack) if (f == n->a->f) return;    // ignore recursive call
                for (auto args = n->b; args; args = args->b)
                    if (args->a->type == 'COCL' && n != this) return;  // coroutine constructor, don't enter
                fstack.push_back(n->a->f);
                if (n->a->f->multimethod) err = "multi-method call";
                evalblock(n->a->f->subf->body, n->b);
                fstack.pop_back();
            }
            else if (n->type == '->')
            {
                auto f = lookup(n->a);
                if (f->type == 'COCL') { customf(istack); return; }
                // ignore dynamic calls to non-function-vals, could make this an error?
                if (f->type != '{}') { assert(0); return; }
                evalblock(f, n->b);
            }
            else if (n->type == 'NATC')
            {
                for (Node *list = n->b; list; list = list->b)
                {
                    auto a = lookup(list->a);
                    if (a->type == 'COCL') customf(istack);
                    // a builtin calling a function, we don't know what values will be supplied for the args,
                    // so we define them in terms of themselves
                    if (a->type == '{}') evalblock(a, a->a);
                }
            }
        };

        evalblock = [&](Node *cl, Node *args)
        {
            Node *a = args;
            for (Node *pars = cl->a; pars; pars = pars->b)
            {
                if (a)  // if not, this is a _ var that's referring to a past version, ok to ignore
                {
                    assert(pars->a->type == 'ID');
                    auto val = lookup(a->a);
                    istack.push_back(pars->a->ident);
                    vstack.push_back(val);
                    a = a->b;
                }
            }

            eval(cl->b);

            a = args;
            for (Node *pars = cl->a; pars; pars = pars->b) if (a) 
            {
                istack.pop_back();
                vstack.pop_back();
                a = a->b;
            }
        };

        eval(this);

        return err;
    }

    string Dump(int indent, Lex &lex)
    {
        switch (type)
        {
            case 'INT': return inttoa(integer);
            case 'FLT': return flttoa(flt);
            case 'STR': return string("\"") + str + "\"";
            case 'NIL': return "nil";
            case 'TRU': return "true";

            case 'ID': 
            case 'ST':  
            case 'FLD': 
            case 'FN':
            case 'NF':
                return n->name;

            default:
            {
                string s = lex.TokStr(type);

                string as, bs;
                bool ml = false;
                auto indenb = indent - (type == ',') * 2;

                if (a) { as = a->Dump(indent + 2, lex); DumpType(a, as); if (as[0] == ' ') ml = true; }
                if (b) { bs = b->Dump(indenb + 2, lex); DumpType(b, bs); if (bs[0] == ' ') ml = true; }

                if (as.size() + bs.size() > 60) ml = true;

                if (ml)
                {
                    if (a) { if (as[0] != ' ') as = string(indent + 2, ' ') + as; }
                    if (b) { if (bs[0] != ' ') bs = string(indenb + 2, ' ') + bs; }
                    if (type == ',')
                    {
                        s = "";
                    }
                    else
                    {
                        s = string(indent, ' ') + s;
                        if (a) s += "\n";
                    }
                    if (a) s += as;
                    if (b) s += "\n" + bs;
                    return s;
                }
                else
                {
                    if (b) return "(" + s + " " + as + " " + bs + ")";
                    else return "(" + s + " " + as + ")";
                }
            }
        }
    }

    void DumpType(Node *n, string &ns)
    {
        if (n->exptype.t != V_UNKNOWN)
        {
            ns += ":";
            ns += TypeName(n->exptype.t);
        }
    }
};
