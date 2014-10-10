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

// Enum used both for token and node types

enum TType
{
    T_NONE = 0,

    // These are used in both.
    T_PLUS,
    T_MINUS,
    T_MULT,
    T_DIV,
    T_MOD,
    T_PLUSEQ,
    T_MINUSEQ,
    T_MULTEQ,
    T_DIVEQ,
    T_MODEQ,
    T_AND,
    T_OR,
    T_NOT,
    T_INCR,
    T_DECR,
    T_EQ,
    T_NEQ,
    T_LT,
    T_GT,
    T_LTEQ,
    T_GTEQ,
    T_ASSIGN,
    T_DYNASSIGN,
    T_LOGASSIGN,
    T_DEF,
    T_DOT,
    T_INT,
    T_FLOAT,
    T_STR,
    T_NIL,
    T_IDENT,
    T_STRUCT,
    T_FUN,
    T_RETURN,
    T_IS,
    T_SUPER,
    T_COROUTINE,

    // These are used ONLY as node types:
    T_STRUCTDEF,
    T_FIELD,
    T_FUNDEF,
    T_NATIVE,
    T_NATCALL,
    T_CALL,
    T_DYNCALL,
    T_LIST,
    T_ARGLIST,
    T_DEFLIST,
    T_CLOSURE,
    T_COCLOSURE,
    T_MULTIRET,
    T_SEQ,
    T_CO_AT,
    T_INDEX,
    T_POSTINCR,
    T_POSTDECR,
    T_CONSTRUCTOR,

    // These are used ONLY as tokens:
    T_LINEFEED,
    T_ENDOFINCLUDE,
    T_ENDOFFILE,
    T_INDENT,
    T_DEDENT,
    T_LEFTPAREN,
    T_RIGHTPAREN,
    T_LEFTBRACKET,
    T_RIGHTBRACKET,
    T_SEMICOLON,
    T_AT,
    T_COMMA,
    T_COLON,

    T_DEFCONST,
    T_DEFTYPEIN,
    T_TYPEIN,

    T_VALUE,
    T_INCLUDE,
    T_INTTYPE,
    T_FLOATTYPE,
    T_STRTYPE,
    T_VECTTYPE,
    T_FROM,
    T_PROGRAM,
    T_PRIVATE,

};

inline const char *TName(TType t)
{
    static const char *names[] =
    {
        "invalid_token",

        "+", "-", "*", "/", "%",
        "+=", "-=", "*=", "/=", "%=",
        "&", "|", "~",
        "++", "--",
        "==", "!=", "<", ">", "<=", ">=",
        "=", "<-", "?=", ":=", ".",
        "integer literal",
        "floating point literal",
        "string literal",
        "nil",
        "identifier",
        "struct",
        "function",
        "return",
        "is",
        "super",
        "coroutine",

        "structdef",
        "field",
        "fundef",
        "native",
        "natcall",
        "call",
        "dyncall",
        "list",
        "#",
        "deflist",
        "closure",
        "coclosure",
        "multiret",
        "seq",
        "co at",
        "index",
        "+++", "---",
        "construct",

        "linefeed",
        "end of include",
        "end of file",
        "indentation",
        "de-indentation",
        "(", ")", "[", "]", ";", "@", ",", ":",
        ":==", "::=", "::", 
        "value",
        "include",
        "int",
        "float",
        "string",
        "vector",
        "from",
        "program",
        "private",
    };
    return names[t];
}
