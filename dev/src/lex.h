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

struct LoadedFile
{
    char *p, *linestart, *tokenstart, *source, *stringsource;
    int fileidx;
    int token;
    int line;
    int errorline;  // line before, if current token crossed a line
    bool islf;
    bool cont;
    string sattr;

    vector<pair<int, bool>> indentstack;
    //char prevlineindenttype;
    const char *prevline, *prevlinetok;

    struct Tok { int t; string a; };

    vector<Tok> gentokens;

    LoadedFile(const char *fn, vector<string> &fns, char *_ss)
        : tokenstart(NULL), stringsource(_ss), fileidx(fns.size()), token('?'), line(1), errorline(1), islf(false),
          cont(false), prevline(NULL), prevlinetok(NULL) /* prevlineindenttype(0) */
    {
        source = stringsource;
        if (!source) source = (char *)LoadFile((string("include/") + fn).c_str());
        if (!source) source = (char *)LoadFile(fn);
        if (!source) throw string("can't open file: ") + fn;

        linestart = p = source;
        
        indentstack.push_back(make_pair(0, false));

        fns.push_back(fn);
    }

    void Clean()
    {
        if (source && source != stringsource) free(source);
    }
};

struct Lex : LoadedFile
{
    vector<LoadedFile> parentfiles;
    set<string, less<string>> allfiles;

    vector<string> &filenames;

    Lex(const char *fn, vector<string> &fns, char *_ss = NULL) : LoadedFile(fn, fns, _ss), filenames(fns)
    {
        FirstToken();
    }

    ~Lex()
    {
        Clean();
    }

    void FirstToken()
    {
        Next();
        if (token == 'LF') Next();
    }

    void Include(char *_fn)
    {
        if (allfiles.find(_fn) != allfiles.end())
        {
            Next();
            return;
        }

        allfiles.insert(_fn);
        parentfiles.push_back(*this);

        *((LoadedFile *)this) = LoadedFile(_fn, filenames, NULL);

        FirstToken();
    }

    void PopIncludeStart()
    {
        if (parentfiles.empty()) return;
        
        token = 'EOI';
    }

    void PopIncludeContinue()
    {
        Clean();

        *((LoadedFile *)this) = parentfiles.back();
        parentfiles.pop_back();

        Next();
    }
    
    void Push(int t, const string &a = string())
    {
        Tok tok;
        tok.t = t;
        tok.a = a;
        gentokens.push_back(tok);
    }

    void PushCur() { Push(token, sattr); }
    
    void Undo(int t, const string &a = string())
    {
        PushCur();
        Push(t, a);
        Next();
    }

    void Next()
    {
        if (gentokens.size())
        {
            token = gentokens.back().t;
            sattr = gentokens.back().a;
            gentokens.pop_back();
            return;
        }

        bool lastcont = cont;
        cont = false;

        NextToken();

        if (islf && token != 'EOF' && token != 'EOI')
        {
            int indent = (int)(tokenstart - linestart);
            if (indent > 0)
            {
                if (prevline)
                    for (const char *p = linestart; p < tokenstart && prevline < prevlinetok; p++, prevline++)
                        if (*p != *prevline)
                            Error("adjacent lines do not start with the same sequence of spaces and/or tabs");

                prevline = linestart;
                prevlinetok = tokenstart;
            }
            else
            {
                //prevlineindenttype = 0;
                prevlinetok = prevline = NULL;
            }

            if (lastcont)
            {
                if (indent < indentstack.back().first)
                    Error("line continuation can't indent less than the previous line");
                if (indent > indentstack.back().first)
                    indentstack.push_back(make_pair(indent, true));
                return;
            }

            PushCur();

            tryagain:
            if (indent != indentstack.back().first)
            {
                if (indent > indentstack.back().first)
                {
                    indentstack.push_back(make_pair(indent, false));
                    Push('IND');
                }
                else
                {
                    bool iscont = false;
                    while (indentstack.back().first > indent)
                    {
                        iscont = indentstack.back().second;
                        indentstack.pop_back();
                        if (!iscont)
                        {
                            Push('LF');
                            Push('DED');
                        }
                    }
                    if (iscont) goto tryagain;
                    if (indent != indentstack.back().first /*&& !iscont*/) Error("inconsistent dedent");
                    //if (iscont) Push('LF');
                }
            }
            else
            {
                Push('LF');
            }
            
            Next();
        }
    }

    void NextToken()
    {
        errorline = line;
        islf = false;
        for (;;) switch (tokenstart = p, token = *p++)
        {
            case '\0':
                p--;
                if (indentstack.size() > 1)
                {
                    bool iscont = indentstack.back().second;
                    indentstack.pop_back();
                    if (iscont) return NextToken();
                    islf = false; // avoid indents being generated because of this dedent
                    token = 'DED';
                }
                else
                {
                    token = 'EOF';
                    PopIncludeStart();
                }
                return;

            case '\n': line++; islf = true; linestart = p; break;
            case ' ': case '\t': case '\r': case '\f': break;
            
            case '(': case ')':
            case '[': case ']':
            //case '{': case '}':

            //case '`':
            //case '^':
            //case '~':
            //case '#':
                 return;

            case ';':
                return;

            case '@':
            case ',':
            case '&':
            case '|':
                cont = true;
                return;

            #define secondb(s, t, b) if (*p == s) { p++; token = t; b; return; }
            #define second(s, t) secondb(s, t, {})

            case '+': second('+', '++'); cont = true; second('=', '+='); return;
            case '-': second('-', '--'); cont = true; second('=', '-='); return;
            case '*':                    cont = true; second('=', '*='); return;
            case '%':                    cont = true; second('=', '%='); return;

            case '<': cont = true; second('=', '<='); second('-', '<-'); return;
            case '=': cont = true; second('=', '=='); return;
            case '!': cont = true; second('=', '!='); cont = false; return;
            case '>': cont = true; second('=', '>='); return;

            case '?': cont = true; second('=', '?='); Error("illegal token: ?");

            case ':':
                cont = true;
                secondb('=', ':=', second('=', ':=='));
                if (*p == ':')
                {
                    p++;
                    second('=', '::=');
                    token = '::';
                    return;
                };
                cont = false;
                return;

            case '/':
                cont = true;
                second('=', '/=');
                cont = false; 
                if (*p == '/')
                {
                    while (*p != '\n' && *p != '\0') p++;
                    break;
                }
                else if (*p == '*')
                {
                    for (;;)
                    {
                        p++;
                        if (*p == '\0') Error("end of file in multi-line comment");
                        if (*p == '\n') line++;
                        if (*p == '*' && *(p + 1) == '/') { p += 2; break; }
                    }
                    linestart = p;  // not entirely correct, but best we can do
                    break;
                }
                else
                {
                    cont = true;
                    return;
                }

            case '\"':
            case '\'':
                LexString();
                return;

            default:
            {
                if (isalpha(token) || token == '_' || token < 0)
                {
                    while (isalnum(*p) || *p == '_' || *p < 0) p++;
                    sattr = string(tokenstart, p);
                    // add any new keywords also to TokStr below
                    if      (sattr == "nil")       token = 'NIL';
                    else if (sattr == "true")    { token = 'INT'; sattr = "1"; }
                    else if (sattr == "false")   { token = 'INT'; sattr = "0"; }
                    else if (sattr == "return")    token = 'RET'; 
                    else if (sattr == "struct")    token = 'TYPE'; 
                    else if (sattr == "value")     token = 'VALU';
                    else if (sattr == "include")   token = 'INCL';
                    else if (sattr == "int")       token = 'INTT';
                    else if (sattr == "float")     token = 'FLTT';
                    else if (sattr == "string")    token = 'STRT';
                    else if (sattr == "vector")    token = 'VECT';
                    else if (sattr == "function")  token = 'FUNC';
                    else if (sattr == "super")     token = 'SUP';
                    else if (sattr == "is")        token = 'IS';
                    else if (sattr == "from")      token = 'FROM';
                    else if (sattr == "program")   token = 'PROG';
                    else if (sattr == "private")   token = 'PRIV';
                    else if (sattr == "coroutine") token = 'CORO';
                    else token = 'ID';
                    return;
                }

                if (isdigit(token) || (token == '.' && isdigit(*p)))
                {
                    if (token == '0' && *p == 'x')
                    {
                        p++;
                        int val = 0;
                        while (isxdigit(*p)) val = (val << 4) | HexDigit(*p++);
                        sattr = inttoa(val);
                        token = 'INT';
                        return;
                    }
                    while (isdigit(*p) || (*p=='.' && !isalpha(*(p + 1)))) p++;
                    sattr = string(tokenstart, p);
                    token = strchr(sattr.c_str(), '.') ? 'FLT' : 'INT';
                    return;
                }

                if (token == '.') return;

                Error("illegal token: " + TokStr());
            }
        }
    }

    int HexDigit(char c)
    {
        if (isdigit(c)) return c - '0';
        if (isxdigit(c)) return c - (c < 'a' ? 'A' : 'a') + 10;
        return -1;
    }

    void LexString()
    {
        int c = 0;
        sattr = "";

        while ((c = *p++) != token) switch(c)
        {
            case 0:
            case '\n':
                p--;
                Error("end of line found in string constant");

            case '\'':
            case '\"':
                Error("\' and \" should be prefixed with a \\ in a string constant");

            case '\\':
                switch(c = *p++)
                {
                    case 'n': c = '\n'; break;
                    case 't': c = '\t'; break;
                    case 'r': c = '\r'; break;
                    case '\\':
                    case '\"':
                    case '\'': break;
                    case 'x':
                        if (!isxdigit(*p) || !isxdigit(p[1]))
                            Error("illegal hexadecimal escape code in string constant");
                        c = HexDigit(*p++) << 4;
                        c |= HexDigit(*p++);
                        break;
                    default:
                        p--;
                        Error("unknown control code in string constant");
                };
                sattr += c;
                break;

            default:
                if (c<' ') Error("unprintable character in string constant");
                sattr += c;
        };

        if (token == '\"')
        {
            token = 'STR';
        }
        else
        {
            if (sattr.size() > 4) Error("character constant too long");
            int ival = 0;
            for (auto c : sattr) ival = (ival << 8) + c;
            sattr = inttoa(ival);
            token = 'INT';
        };
    };

    string TokStr(int t = 0)
    {
        bool attr = !t;
        if (attr) t = token;
        switch (t)
        {
            case 'EOF':  return "end of source file";
            case 'EOI':  return "end of include file";
            case 'ID':   return attr ? sattr : "identifier";
            case 'FLT':  return attr ? sattr : "floating point literal";
            case 'INT':  return attr ? sattr : "integer literal";
            case 'STR':  // FIXME: will not deal with other escape codes, use ToString code
                         return attr ? "\"" + sattr + "\"" : "string literal";
            case 'IND':  return "indentation";
            case 'DED':  return "de-indentation";
            case 'LF':   return "end of line";
            case 'NIL':  return "nil";
            case 'RET':  return "return";
            case 'TYPE': return "struct";
            case 'VALU': return "value";
            case 'INCL': return "include";
            case 'INTT': return "int";
            case 'FLTT': return "float";
            case 'STRT': return "string";
            case 'VECT': return "vector";
            case 'FUNC': return "function";
            case 'SUP':  return "super";
            case 'IS':   return "is";
            case 'FROM': return "from";
            case 'PROG': return "program";
            case 'PRIV': return "private";
            case 'CORO': return "coroutine";

            default:
            {
                if (t <= ' ') return string("[ascii ") + inttoa(t) + "]";

                string s;
                while (t) { s = (char)t + s; t >>= 8; }
                return s;
            }
        }
    }

    void Error(string err, int fidx = -1, int line = -1)
    {
        throw (fidx >= 0 ? filenames[fidx] : filenames[fileidx]) +
              "(" + inttoa(line >= 0 ? line : errorline) + "): error: " + err;
    } 
};
