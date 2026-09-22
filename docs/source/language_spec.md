---
title: Lobster Language Specification
---

Scope and conventions
---------------------

This document specifies the Lobster language as implemented by the reference
compiler and runtime (`dev/src`). It is written for implementors of Lobster
and for users who need an unambiguous answer about how a construct behaves. It
describes what programs are accepted and what they mean; it does not teach the
language, and it does not describe the builtin function library beyond the
rules the language imposes on calls to it.

The specification is normative for the reference implementation: where the two
disagree, one of them has a bug.

Terminology:

- *Error* means a compile-time diagnostic. Compilation stops after the pass
  that reported it (the parser, declaration checker, type checker or code
  generator), so no program with an error runs.
- *Warning* means a diagnostic that does not stop compilation.
- *Runtime error* means the program terminates with a message when the
  construct executes.
- *Unspecified* marks behavior the implementation inherits from the host C
  compiler or platform rather than defining itself.

Grammar fragments use an informal EBNF: `x*` zero or more, `x+` one or more,
`x?` optional, `|` alternatives, terminals in backticks, and the token names
`NL`, `INDENT`, `DEDENT` from the lexical section.


Lexical structure
-----------------

### Source files

A source file is a sequence of bytes. Bytes with the high bit set are accepted
as identifier characters and as string content without validation, so
identifiers and strings may contain UTF-8, but the compiler never decodes it
(the `\u` string escape encodes to UTF-8). A NUL byte ends the file. Lines end
in LF; CR is whitespace. If the main file starts with `#!`, its first line is
ignored.

### Whitespace and comments

Space, tab, CR and FF are whitespace. `//` starts a comment to the end of the
line. `/*` starts a comment that ends at the first `*/`; such comments do not
nest, and one that reaches the end of the file is an error. A comment counts as
whitespace, except that the indentation of the line on which a multi-line
comment ends is measured from the end of the comment.

`;` is an error ("isn't used as a statement terminator").

### Tokens

Tokens are matched longest-first. The operator and punctuation tokens are:

    +  -  *  /  %  ++  --  ==  !=  <  >  <=  >=  &  |  ^  ~  <<  >>
    =  +=  -=  *=  /=  %=  &=  |=  ^=  <<=  >>=
    .  ..  ->  (  )  [  ]  {  }  ?  ,  :  ::

`!` (other than in `!=`), `&&` and `||` are errors that direct the user to
`not`, `and` and `or`. Any other character outside a string or comment is an
error ("illegal token").

`>>` and `>>=` are only recognized outside a type argument list; inside `<`
`>` in a type, two `>` characters are two `>` tokens (so `dictionary<string,
dictionary<string, int>>` parses).

### Identifiers and keywords

    identifier = idstart idcont*
    idstart    = letter | `_` | any byte >= 0x80
    idcont     = idstart | digit

The following words are keywords and cannot be identifiers:

    abstract and any assert attribute break case class constructor continue
    def default elif else enum enum_flags float fn for from guard if import
    int is let member member_frame namespace nil nonlocal not operator or
    out_of_range pakfile private program resource return static static_frame
    string struct super switch typeof union var void while

`inf`, `infinity` and `nan` are float literals (see below) and therefore also
not usable as identifiers. `true`, `false` and `this` are not keywords: `true`
and `false` are the values of the enum `bool` declared by the prelude, and
`this` is an ordinary identifier that method declarations introduce (a
variable named `this` in an enclosing scope therefore makes every method in
that scope an error, see shadowing).

Namespaced identifiers: once a namespace `ns` has been declared (by a
`namespace` statement earlier in parse order, in any file, or by the host
program), the character sequence `ns` `.` `identifier` is lexed as a single
identifier token with the text `ns.identifier`, and this repeats for nested
namespaces (`a.b.c`). Consequently a `.` directly after a namespace name never
denotes field access, even if a variable of that name exists.

### Integer literals

    intlit = digit+ | (`0x` | `0X`) hexdigit+

A decimal literal must fit in a signed 64-bit integer; otherwise it is an
error ("int constant overflow"). Unary minus is a separate operator, so
`-9223372036854775808` is an error; the smallest integer is written as
`-9223372036854775807 - 1` or `0x8000000000000000`. A hex literal is parsed as
an unsigned 64-bit value and reinterpreted as signed, so `0xFFFFFFFFFFFFFFFF`
is `-1`; more than 64 bits is an error. There are no octal or binary
literals, digit separators or suffixes.

### Character literals

    charlit = `'` (char | escape)* `'`

The escapes are those of strings. The literal denotes an integer: the bytes of
its contents packed big-endian (the first byte most significant). Up to 8
bytes are allowed (more is an error); an empty literal is `0`. A raw `"` inside
is an error.

### Float literals

    floatlit = digit+ `.` digit* exponent?
             | `.` digit+ exponent?
             | `inf` | `infinity` | `nan`
    exponent = (`e` | `E`) (`+` | `-`)? digit+

`digit+ .` is only a float when the character after the `.` is neither `.`
nor a letter: `1..2` is `1` `..` `2`, and `1.abs` is `1` `.` `abs`. An
exponent is only part of the literal after a form containing `.`: `1e5` lexes
as `1` followed by the identifier `e5`. Literals are unsigned: `-inf` is the
unary minus operator applied to `inf`, so `a -inf` is `a - inf`. The value is
the nearest double as converted by the C library.

### String literals

    strlit = `"` (char | escape | interpolation)* `"`

A raw newline ends the literal with an error. Raw bytes below 0x20 and 0x7F
are errors; other bytes, including those above 0x7F, are content. A raw `'`
is an error (it must be written `\'`). The escapes are `\n`, `\t`, `\r`, `\\`,
`\"`, `\'`, `\{`, `\}`, `\x` followed by exactly two hex digits (one byte), and
`\u` followed by exactly four hex digits (the code point, encoded as UTF-8).
Any other `\` sequence is an error.

`{` starts an interpolation (below) unless it is doubled or written `\{`; `}`
outside an interpolation is an error unless it is doubled or written `\}`. A
doubled brace stands for a single brace.

Interpolation: `{` *expr* `}` inside a string embeds an expression, which is
an operator expression (an expression without assignment, see the expression
grammar). It may itself contain string literals and interpolations. The lexer
turns the string into a sequence of string-part and expression tokens; the
parser builds the expression described under String interpolation.

Multi-line strings: `"""` starts a literal that ends at the next `"""`. If the
opening is followed by a line end (`\n` or `\r\n`), that line end is skipped;
the number N of spaces at the start of the first content line is then removed
from the start of every following line (as many of its leading spaces as it
has, up to N; tabs are not stripped). CR bytes are dropped. No escapes and no
interpolation are recognized; `{`, `}`, `\` and single `"` are content.

### Lines, indentation and blocks

The lexer produces `NL` (linefeed), `INDENT` and `DEDENT` tokens from the
layout of the source:

1. Lines without tokens (empty, or comment only) are ignored.
2. An `NL` token is produced at the end of every line that has tokens, except:
   (a) inside an unclosed `(`, `[`, `{` or string interpolation, where line
   ends are ignored entirely; (b) when the line's last token is a
   *continuation token*: `,` `+` `-` `*` `/` `%` `<` `>` `<=` `>=` `<<` `>>`
   `==` `!=` `=` any compound assignment operator `&` `|` `^` `~` `::` `->`
   `and` `or` `operator` (`:` `.` `not` `is` are not); (c) when the next
   line's first token is `.` (this lets a method chain continue on the next
   line).
3. The indentation of a line is the number of whitespace characters before
   its first token; tabs and spaces each count as one. The leading whitespace
   of two consecutive token-bearing lines must agree character by character
   for the length of the shorter one, else it is an error.
4. A line indented more than the current block level opens a block: `INDENT`
   is produced. A line indented less closes blocks: for each level closed,
   `NL` `DEDENT` is produced; the line's indentation must then equal the level
   of an enclosing block (else error "inconsistent dedent"). A line at the
   current level produces `NL` before its tokens (which is the `NL` of the
   previous line under rule 2).
5. A line following a continuation token may be indented more than the
   current block; this does not open a block. It may not be indented less
   (error).
6. The end of the file closes all open blocks; an unclosed bracket at the end
   of a file is an error. No `NL` is produced after the last line.

The parser treats a block as `INDENT statements DEDENT`, or, for one-line
forms, as the statement(s) following `:` on the same line.


Programs, files, namespaces and privacy
---------------------------------------

### Program

A program consists of a main file and the files it imports transitively. The
compiler parses all files, then runs declaration checking, type checking,
optimization and code generation over the whole program; execution begins with
the first top-level statement of the main file. Top-level statements of all
files execute in the order in which they appear after import expansion.

The prelude `stdtype.lobster` is imported implicitly before the main file's
first statement. It declares `struct vec2<T>`, `vec3<T>` (: vec2), `vec4<T>`
(: vec3) with fields `x`, `y`, `z`, `w`, the specializations `float2`, `int2`,
`float3`, `int3`, `float4`, `int4`, and `enum bool: false, true`. The language
depends on these names existing: `bool` is the type of comparisons and `not`,
and the `intN` types are the results of struct comparisons.

### import

    importstmt = `import` ident (`.` ident)*
               | `import` `.` ident (`.` ident)*
               | `import` strlit
               | `import` `from` strlit

`import a.b` names the file `a/b.lobster`; `import .a.b` names it relative to
the directory of the importing file; a string literal is a path used
verbatim. A file is searched for as `modules/<path>` relative to the
executable's directory first (not for relative imports), then as `<path>` via
the program's file loader (the main file's directory and data directories).
`import from "dir"` adds a data directory and imports nothing. The statement
must be the only thing on its line, and at file scope (not inside a block or
a function; error otherwise). Files are identified by the resulting path
string: a file that was imported before (anywhere in the program) is not
imported again, and produces nothing; an import cycle is an error.

An import parses the imported file's statements as if they were written at
the point of the import, with two differences: the current namespace is
empty while parsing the imported file and restored afterwards, and `private`
declarations of the file become inaccessible after it ends. The imported
file's declarations are visible from the import point on.

### namespace

    nsstmt = `namespace` ident (`.` ident)* | `namespace`

Only at file scope. Sets the current namespace for the rest of the file (or
until the next `namespace` statement); `namespace` alone clears it. The
namespace applies to declarations at the file's top scope only, and only to:
variables, functions (including methods declared with `::` outside a class,
but not methods declared inside a class body), classes, structs and unions
(and their members), enums and enum values. Fields are never namespaced, and
neither are declarations inside blocks or functions. Such a declaration's
name is `ns.name`. Looking up an unqualified name while
the current namespace is `ns` tries `ns.name` first, then `name`; a qualified
name `ns.name` (a single identifier token, see Lexical structure) works from
anywhere. The lexer only recognizes a namespace from the statement declaring
it onward.

### private

`private` may prefix a file-scope `let`/`var`, `def`, `constructor`, `class`,
`struct`, `union`, `abstract` and `enum` declaration, and a field or method
inside a class body. Everything private is fully usable within its file. After
the file has been parsed to its end, private variables, types, enums (with
their values) are no longer reachable by name from other files, and a call
from another file that selects an overload declared private is an error
("call to (partially) private function"); a private field read or written from
another file is an error. A `static` variable is always private; a `member`
field is method-scoped (see below). A named specialization must have the same
privacy as its generic type.


Declarations and scope
----------------------

### Scopes

There are file scope, function scopes (each `def`, `fn`, and trailing block
body), and block scopes (the bodies of `if`/`elif`/`else`/`guard`, `while`,
`for`, and each `switch` case; the condition of an `if` is in a scope of its
own). A block is either an indented sequence of statements or a single
statement after `:`.

Variables (from `let`, `var`, parameters, loop variables, pattern variables,
`static`) are visible from their declaration to the end of the innermost
enclosing function or block scope, including inside nested function bodies
(where they are free variables, see Function values).

Named functions are visible in the whole block that declares them, before as
well as after the declaration, and in everything lexically nested in it; an
inner declaration hides an outer function of the same name for the nested
code only. Two `def`s of the same name in the same scope declare overloads
(same arity) or arity variants (different arity) of one function.

Types (`class`, `struct`, `union`, `enum`) are visible from their declaration
to the end of the block that declares them, and inside nested code. A type
must be declared, or pre-declared, before (in parse order) any use of its
name.

Shadowing: declaring a variable whose name is visible as a variable of an
enclosing scope is an error ("identifier shadowing"); a second declaration in
the same scope is an error ("identifier redefinition"). This applies to
parameters, `fn` parameters, loop variables, pattern variables and the
implicit `this`. A variable may not have the name of a field that is in scope
through `::` (error). Variables, functions, types and fields are otherwise
separate namespaces: a variable may share its name with a function, a type or
a field. When an identifier could be an enum value or a variable, the enum
value wins (see Names in expressions).

### Variable declarations

    vardecl = (`let` | `var`) binding (`,` binding)* (`=` exprlist)?
    binding = ident (`:` type)?
    exprlist = opexpr (`,` opexpr)*

A `let` variable is single-assignment: any assignment, compound assignment or
`++`/`--` to it is an error ("variable is constant"), and if its type is a
struct, assigning any of its fields (at any nesting, including indexed
elements of nested numeric structs) is an error; the contents of a class
instance or vector it refers to may be modified. A `var` may be reassigned.

With several bindings the initializer must produce at least that many values
(a multiple-value call, or an expression list); extra values are discarded,
fewer is an error. Each binding takes the corresponding value.

The variable's type is its annotation, to which the initializer must convert
(with coercions, see Conversions); without an annotation it is the
initializer's type. An initializer of `nil` without annotation gives a
nilable type whose element type is a type variable bound by later use (an
error if that use binds it to a type that cannot be nilable). A value of type
`void` cannot be stored (error). An initializer that never completes (a
`return`, or a call to a function whose every return is a non-local one, on
every path) is an error ("initializer of x never completes"), as is such an
expression on the right of an assignment; in a branch of an `if` or `switch`
that is used as a value it is fine, since the other branches provide the value.

Pre-declaration: at file scope only, `let x:T` or `var x:T` without an
initializer declares `x` for later definition. The definition is a later
`let`/`var x:T = e` in the same scope with an identical type annotation,
the same keyword and the same privacy; one that never comes is an error, and
one in a nested scope or a condition is an error. Reading the variable in code
that the compiler type checks before it reaches the definition is an error
("access of x before being initialized"); the check follows the type checker's
traversal (call graph order from the top of the main file), not execution.

A declaration produces no value and may not be the last statement of a block
(except at the end of a file). A local variable that is never read gets a
warning (unless its name starts with `_` and it is one of several bindings);
a `var` that is never reassigned gets a warning suggesting `let`.

### static and static_frame

    staticdecl = (`static` | `static_frame`) ident (`:` type)? `=` expr

Allowed in any nested scope (a function body, or a block at file scope; not
directly at file scope). Declares a variable with program lifetime, private
to its file, initialized once, immediately before
the top-level statement that (lexically) contains the declaring function
executes; the initializer is evaluated in the top-level context. Its type is
the given type or the initializer's type. `static_frame` additionally re-evaluates the initializer
and stores its value each time the statement is executed after a frame has
passed since its previous execution (frames are counted by the engine's frame
builtin; without frames it behaves like `static`).

### member and member_frame

    memberdecl = (`member` | `member_frame`) ident (`:` type)? `=` expr

Allowed inside a method (a `def` in a `class` body) of a class that is not a
struct and has no subclasses yet. Declares an additional field of the class,
with the initializer as its default value (evaluated like any field default:
at every construction site, so it may not refer to fields, locals of the
method, or anything declared by the method), and the annotation or the
initializer's type as its type. The field is accessible by its unqualified
name only lexically inside the declaring method (including nested functions),
and only after the `member` statement in the method body; it is not
accessible through `obj.name`. `member_frame` re-runs the initializer and
assigns the field, per object, each time the statement executes after a frame
has passed since it last executed for that object.

### nonlocal

`nonlocal var/let x = e` declares `x` normally and marks it as preferred when
binding explicit free variables (see Function values). `nonlocal from a, b,
...` restricts, for the whole program, the names allowed in explicit free
variable lists to those listed.

### Function declarations

    fundecl  = (`def` | `constructor`) funname generics? `(` params? `)`
               freevars? (`->` rettypes)? (`:` body)?
    funname  = ident | `operator` op
    generics = `<` ident (`,` ident)* `>`
    params   = param (`,` param)*
    param    = ident ((`:` | `::`) type)? (`=` opexpr)?
    freevars = `[` ident (`:` type)? (`,` ident (`:` type)?)* `]`
    rettypes = type (`,` type)* | `void`
    body     = INDENT statements DEDENT | statement

A `def` declares a named function. The body is a block. A `def` at the file's
top scope gets the current namespace prefixed to its name.

Parameters: a parameter without a type is generic: it gets an implicit type
variable (named `A`, `B`, ... in order of appearance), so the function is
specialized per argument type. `name::T` puts the fields of `T` (a class or
struct type) in scope in the body by their unqualified names, referring to
that parameter, and makes methods of `T` callable without a receiver. A
default value `= e` makes the parameter optional; defaults must be trailing;
the expression is parsed in the scope enclosing the function (it can refer to
variables visible there; referring to a parameter of the function is an
error) and is evaluated at each call site in the caller's context. All overloads of one arity variant must
declare the same number of defaults, with structurally identical default
expressions.

Explicit generics `def f<T, U>(...)` declare type variables usable in the
parameter types, return type and body. Without an explicit list, type
variables are implicit as above.

Explicit free variables `[a, b:T]` name variables that the function reads or
writes from the environment of its callers rather than its lexical scope (see
Function values).

Return types: `-> T` a single value, `-> T1, T2` several, `-> void` none. Without
a return type it is inferred (see Return).

A `def` without `:` and body declares a function type: it must be at the
file's top scope, have no generics, all parameters typed with concrete types,
and a return type. The name then denotes that function type (see Types).

`constructor T(params): body` declares a constructor function for the type
`T`: it must return a value of type `T`, and once any constructor function is
declared for `T`, `T { ... }` construction is only allowed inside a
constructor of `T` (or in anonymous functions called from one); all overloads
of a name must agree on being constructors.

Operator overloads: `def operator+(a:T, b:U): ...` at the file's top scope,
either as a function or as a method in the body of a class or struct declared
there; one declared anywhere else (inside a function or a block, including a
class declared inside one) is an error. The operator may be any of `+ - * / % ++ -- == != < > <= >= & | ^ ~ << >> =` and
the compound assignments `+= -= *= /= %= &= |= ^= <<= >>=`, or `[]` (written
`operator[]`). Unary `-`, `~`, `++`, `--` take one parameter, the rest two.
See Operator overloading for when they apply.

A function of the same name as a builtin, whose arity the builtin also
accepts, is an error at the declaration.

### Class and struct declarations

    typedecl = `abstract`? (`class` | `struct`) tname generics? `:` super?
               (INDENT members DEDENT)?
             | (`class` | `struct`) tname                           (pre-declaration)
             | (`class` | `struct`) tname `=` tname `<` types `>`   (named specialization)
    super    = tname (`<` types `>`)?
    members  = (attribute | field | `private`? `def` ...)*
    field    = `private`? ident (`:` type (`<` intlit `>`)?)? (`=` opexpr)?

A `struct` is a value type; a `class` is a reference type (see Types). The
name gets the namespace prefix at file scope. Generics `<T, U>` declare type
variables usable in field types, defaults, and the superclass arguments.

Fields come before methods. A field has a declared type, a default value, or
both; with only a default, the type is derived from it: from its syntactic
form where that is decidable without type checking (literals, constructors,
typed nil, arithmetic on such values, string interpolation, some builtin
calls), otherwise by type checking the default when the type is first
completed. A field default is evaluated anew at each construction that does
not supply the field, in the scope of the class declaration (it may refer to
globals and to variables visible at the declaration, not to other fields or
`this`). A default that constructs its own type (directly or through other
types' defaults) is an error. Field names are program-wide: the same name
used in several types refers to one field identity, which is what `x.name`
resolves against. A type may not declare a field twice, nor redeclare an
inherited one.

Inheritance: `class Sub : Super` gives `Sub` all fields of `Super` first (in
order), then its own; a struct may only inherit from a struct and a class
from a class. A generic superclass takes type arguments, which may mention the
subclass's type variables. Inheriting from a pre-declared type is allowed;
its fields are added when it is defined. An inheritance cycle is an error.
`abstract` types cannot be constructed.

A struct with no fields is an error unless it is (in) an abstract struct
family (below). A struct may not contain itself, directly or through nested
structs.

Pre-declaration: `class Name` (no `:`) declares the name so it can be used in
types (typically nilable references to it) before its definition; the
definition must follow in the same block, with the same kind, and may not add
generics. Until the definition, the type has no fields.

Named specialization: `class Name = Generic<T1, ...>` declares `Name` as the
specialization of a fully defined generic type with exactly its number of type
arguments. It is the same type as `Generic<T1, ...>` written anywhere.

Bit-width fields: `name:int<N>` with N a multiple of 8 up to 64, or
`name:float<32>`/`float<64>`, or an enum type with `<N>`, gives the field a
storage width. Only the storage differs: reads produce the whole `int`/`float`
(ints sign-extended, floats widened from single precision), writes store the
low N bits (ints) or the value rounded to single precision (floats), and
consecutive such fields share a 64-bit slot while they fit (so
compound assignment and `++` on them read and write the bits). A struct with
such a field is not a numeric struct (no vector arithmetic, indexing or use
as a numeric-struct builtin argument). `<64>` is the same as no width.

Attributes: `attribute name` or `attribute name = value` inside a class body
records metadata. `attribute serializable` (optionally `= id`, 0..65536,
otherwise the previous id plus one) assigns the serialization id used by the
binary serialization builtins; it is only allowed on non-generic,
non-abstract classes and on structs that belong to an abstract struct family,
and ids must be unique. Inside a function body, `attribute name [= value]`
records an attribute of that function (`profile` is used by the profiler).

Methods: a `def` inside a class body declares a function whose first
parameter is `this::Type` (implicit; it is named `this`, is a `::` parameter,
and is not counted in the written parameter list), so inside the body the
fields of the class are in scope by name and other methods of the class can
be called without a receiver (see Call resolution). A generic class's type
variables are in scope in its methods, but not in the methods of a subclass
of one of its specializations. Such a method takes no namespace prefix.
`constructor` inside a class body is an error. Any function whose first
parameter has a class or struct type is a method of that type for the
purposes of dispatch and `x.f()` syntax, whether declared inside or outside
the class body, with `:` or `::`.

### union declarations

    uniondecl = `union` (`class` | `struct`) tname `:`
                INDENT (tname (`(` fields `)`)? NL)+ DEDENT

`union class U:` with members `A(fields)`, `B` declares `abstract class U` and,
for each member, `class A : U` with the given fields (fields as in a class
body, comma separated, `private` allowed; parentheses optional for a member
without fields). `union struct` does the same with structs, declaring an
abstract struct family. The members are ordinary types in every respect.

### Abstract structs (families)

A struct that has an abstract struct among its superclasses (itself included)
is a member of the *family* rooted at the topmost such abstract struct. All
members of a family have the same size, and a value of any member type
carries its dynamic type, so a value of a supertype in the family may hold any
member: conversions between family types follow the class rules, and `is`,
`switch` on the value and dynamic dispatch see the dynamic type. A family
member is a struct of references if any member of the family (over all
declarations, including generic ones) can hold a reference. A family member
may not (transitively) contain a value of its own family. A family has at
most 256 members. Members of a family are never numeric structs. A member
that is never used and whose field types could not be inferred is an error
when any member is completed.

### Enum declarations

    enumdecl = (`enum` | `enum_flags`) tname `:` INDENT
               (ident (`=` opexpr)? ((`,` | NL) ident (`=` opexpr)?)*) DEDENT

Declares an enum type and its values (both namespaced at file scope; the
value names are program-wide identifiers, not qualified by the type). An
explicit value must be a compile-time integer constant expression (integer
literals, enum values, and the arithmetic, comparison, bitwise and logical
operators on them). Without one, the value is the previous value plus 1 for
`enum` (0 for the first), and for `enum_flags` the smallest power of two
greater than the previous value (1 for the first, or after a previous value
that is not positive); an explicit value resets the sequence. A value equal to
an earlier value of the same enum is an error, as is redeclaring an enum name
or a value name. The prelude's `bool` is `enum bool` with `false = 0`,
`true = 1`.


Types
-----

### Type syntax

    type   = `int` | `float` | `string` | tname | tname `<` types `>`
           | `[` type `]` | `resource` `<` ident `>` | typevar | ftname
           | type `?`
    types  = type (`,` type)*

`tname` names a class, struct or enum (with namespace lookup); a generic type
needs its type arguments except where a generic reference is explicitly
allowed (a generic `::` parameter, the constructor forms below); `typevar` is
a type variable in scope; `ftname` is the name of a function type declared by
a bodiless `def`. `[T]` is a vector of `T`. `T?` is the nilable version of `T`,
allowed only for reference types (string, vector, class, resource, function
types are not, structs are not) and type variables. `void` is allowed only as
a function's return type. `any` is not a type in source (only the builtin
function `any`).

### Kinds of types

- `int`: 64-bit two's complement integer. Enum types are distinct int types.
- `float`: 64-bit IEEE 754 binary float.
- `string`: byte sequence, reference type, compared by value; immutable except
  thru the byte-writing builtins (see Operators on strings).
- `[T]`: vector, a resizable sequence of `T`, reference type.
- class types: reference types with named fields, single inheritance,
  identity comparison.
- struct types: value types with named fields (a struct value is the tuple of
  its fields; nested structs are inline), single inheritance; a struct is
  either a struct of scalars or a struct of references (it has a field, at
  any nesting, holding a reference or nilable value).
- function types: the type of a function value, either the signature of a
  specific anonymous function (not nameable) or a declared function type.
- `resource<name>`: opaque reference type provided by the host; compared by
  identity, nilable.
- typeid: the type of `typeof` expressions, represented as an `int` at run
  time.
- `T?`: `T` or `nil`. `nil` is the null reference; there is one nil value for
  all nilable types.
- `void`: no value; the type of statements and of calls to functions that
  return nothing.
- tuples: the type of an expression producing several values; only in
  multiple-value contexts (returns, multi-declarations, multi-assignments).

Type variables (from generic functions and types) stand for a type to be
bound per specialization; implementation-internal inference variables stand
for a type not yet known (e.g. the element type of `[]` or `nil`), and are
bound by unification with the first type they meet.

### Numeric structs

A struct is *numeric* when it is not in an abstract struct family, has no
bit-width fields, and all its fields, flattened through nested struct fields,
have the same numeric type (the same `int` type, i.e. the same enum or plain
`int`, or `float`). Numeric structs support vector arithmetic, comparisons,
indexing, and use as vector arguments of builtins (see the respective
sections). The prelude's `int2`, `int3`, `int4`, `float2`, `float3`, `float4`
are the canonical numeric structs of arities 2 to 4.

### Generics

A generic function (explicit `<T>` list or untyped parameters) and a generic
type (`class G<T>`) are templates; every distinct binding of their type
variables is a separate specialization, created on demand. A type variable of
a function is bound, per call, from the explicit type arguments (in order;
fewer than declared may be given), else from the argument types by matching
the parameter's given type structurally against the argument's type: through
`[..]` and `..?` wrappers, and through the type arguments of a generic type
(where the argument may be a subtype whose superclass is the generic type
instantiated). A type variable that no argument determines is an error
("cannot implicitly bind type variable"). Inside the specialization the
variable is the bound type: usable in `nil::T`, `[]::T`, `T {}`, `x is T`,
annotations and type arguments. A generic type's variables are in scope in
its field types and defaults, its superclass arguments and its methods.
`G<A, B>` written anywhere with the same arguments is one type; a named
specialization `class N = G<A, B>` is that same type under a second name. A
generic type may be subclassed with its type arguments passed on (`class
S<T> : G<T>`) or fixed (`class S : G<int>`).

### Conversions

A value of type S *converts* to a type T (is accepted where T is expected)
under these rules; "with coercion" marks conversions that change the value's
representation and are only applied where stated:

1. S and T are the same type.
2. T is `float` and S is `int` (with coercion; an integer literal converted
   this way gets a warning). Not any enum type.
3. T is plain `int` and S is an enum type (an enum converts to `int`, not the
   reverse, and not to another enum). T is `int` and S is a typeid (with
   coercion).
4. T is `E?`: S is `F?` with F converting to E without coercion; or S is a
   reference type (not `void`, not numeric, not a struct) converting to E
   without coercion. Numeric S converts to `E?` only as a builtin argument.
5. T is `[E]`: S is `[F]` with F the same type as E (vectors are invariant),
   except that a vector literal or object constructor expression (a fresh,
   unaliased value) converts covariantly (F a subtype of E under these rules
   without coercion).
6. T is a class: S is the same class or a subclass; as a vector element
   type or where an exact type is required, only the same class.
7. T is a struct: S is the same struct or a subclass of the same kind (both
   structs of scalars or both of references); in an abstract struct family
   any subtype converts; outside a family only a subclass that adds no fields
   converts (a struct is exactly its fields, so it is never truncated).
8. T is a declared function type: see Function values.
9. T is a tuple: S is a tuple of the same length converting element-wise.
10. T is `void`: any S, where coercion is allowed (a value is discarded).
11. Either side is an unbound inference variable: it is bound to the other.
    Binding the element type variable of a `nil` (see Type syntax for which
    types can be nilable) to a type that cannot be nilable is an error, so a
    type such as `int?` never exists.

A vector or vector literal argument to a builtin parameter declared read-only
also converts covariantly. `nil` converts to every nilable type.

The *union* (common supertype) of two types A and B, used by `if`/`else`
branches, `switch` cases, vector literal elements, `and`/`or`, and untyped
function returns: B if A converts to B, else A if B converts to A (with
coercion where the construct allows it), else for two vectors the vector of
the union of their elements (without coercion), for nilables the nilable of
the union of the elements (both must be reference types), for two classes
their nearest common superclass, for two structs of the same family their
nearest common superclass. Otherwise there is no union (an error, or a
fallback, as each construct states).


Expressions
-----------

### Grammar and precedence

    expr     = opexpr (assignop expr)?                  (right associative)
    assignop = `=` | `+=` | `-=` | `*=` | `/=` | `%=` | `&=` | `|=` | `^=`
             | `<<=` | `>>=`
    opexpr   = binary expression over operands, levels below
    operand  = unary (`is` type)?
    unary    = `-` unary | `~` unary | `++` unary | `--` unary
             | `not` opexpr(level 5)
             | postfix
    postfix  = factor ( `.` ident callargs? | `[` expr `]` | `++` | `--` )*

Binary operators, from tightest to loosest, all left associative:

    0: `*`  `/`  `%`
    1: `+`  `-`
    2: `<<` `>>`
    3: `&`  `|`  `^`
    4: `<`  `>`  `<=` `>=`
    5: `==` `!=`
    6: `and` `or`

`not` takes as its operand an expression of level 5 (so `not a == b` is
`not (a == b)`, while `not a and b` is `(not a) and b`). The prefix operators
`-` `~` `++` `--` bind tighter than all binary operators and apply to a unary
expression, so `-a * b` is `(-a) * b`, but they bind looser than postfix
operators: `-x.y` is `-(x.y)`, `-x[0]` is `-(x[0])`. `is` binds looser than
the prefix operators and tighter than every binary operator: `-x is int` is
`(-x) is int`, `a + b is int` is `a + (b is int)`, and `not x is int` is
`not (x is int)`. Postfix `++`/`--` end a postfix chain (nothing can follow
them). Comparisons chain as ordinary left-associative binary operators:
`1 < 2 < 3` is `(1 < 2) < 3`. `&`, `|` and `^` share one level: `a | b & c` is
`(a | b) & c`. `and` and `or` share one level: `a or b and c` is
`(a or b) and c`.

Assignment is only parsed at the top of an expression (`expr`), i.e. in a
statement, a parenthesized expression, a call argument, an initializer, or
the right side of another assignment; it is not an operand of any operator.

### Factors

    factor = intlit | floatlit | strlit | charlit | `nil` (`::` type)?
           | `(` expr (`,` opexpr)* `)`
           | `[` (expr (`,` expr)*)? `]` (`::` type)?
           | `{` initializers `}`
           | tname (`<` types `>`)? `{` initializers `}`
           | typevar `{` initializers `}`
           | `fn` ...                                   (function value)
           | `typeof` (ident | type)
           | `super` ident callargs
           | `pakfile` strlit
           | `assert` expr
           | `if` ... | `switch` ...
           | ident callargs? | ident `<` types `>` callargs
           | (`int` | `float` | `string` | `any`) callargs

`(e)` groups; `(e1, e2)` is a multiple-value expression (only meaningful where
several values are consumed). `pakfile "path"` is the string literal `"path"`
and additionally records the file for inclusion in a pak file.

### Names in expressions

An identifier in expression position is resolved, in this order:

1. If it starts with `_`, it is an implicit argument (see Function values).
2. A namespace-qualified or unqualified variable visible here, and any field
   in scope through `::` or a pattern variable, are looked up. If neither
   exists, and a function or builtin of the name exists, and the identifier is
   followed by whitespace and then a token other than `NL`, the identifier is
   a call without parentheses taking a single argument (`print x`); the
   argument is an `expr` (in which a nested call cannot take trailing
   function values), optionally followed by trailing function values of the
   `: body` form.
3. If the name is an enum value, the expression is that value (even if a
   variable of the same name exists).
4. If the name is a field in scope through `::` or a pattern variable, the
   expression reads that field of the corresponding object.
5. If the name is an explicit free variable of an enclosing function, it
   refers to that.
6. Otherwise it must be a variable; a name that is only a function or builtin
   is an error ("can't use named function as value"), anything else is
   "unknown identifier".

`ident(` with no whitespace before `(` is a call with parenthesized arguments;
`ident (` is a call without parentheses whose single argument is the
parenthesized expression (`f (1) * 3` is `f((1) * 3)`). `ident<` with no
whitespace before `<`, where `ident` is a function that is not also a builtin
or enum, starts a type argument list of a call; otherwise `<` is the
comparison operator. `E(e)` where `E` is an enum type (and not a function)
converts an `int` expression to the enum (no range check).

### Constructors

`T { ... }` constructs a value of class or struct type `T`, `T<A> { ... }`
of a specialization, `{ ... }` of the type expected by the context, and
`V { ... }` of the type a type variable `V` is bound to. The context type of
`{ ... }` (ignoring nilability) is: the annotation of a single-binding
variable declaration it initializes; the type of the field it initializes in
another constructor; the element type of a vector literal that has one
(`::T`, or a vector context); the parameter type of a function with exactly
one definition (no overloads or arity variants) whose parameter type is
concrete, or of a builtin with a single overload. Elsewhere (including
`return`) it is an error ("class/struct type for auto constructor unknown in
this context"). Initializers are `expr` or
`field: expr`. Tagged initializers assign the named field (each at most once;
unknown field is an error). Untagged initializers assign, in order, the fields
that have no default value (an untagged initializer left over is an error
"too many initializers"). Every field not initialized takes its default;
one without a default is an error. Each initializer must convert (with
coercion) to the field type. An abstract type cannot be constructed. A generic
type without type arguments, `G { ... }`, requires the initializers to have
exactly one per field and picks the named specialization of `G` whose field
types the most initializers convert to without coercion (error if none). If a
type variable is bound to a non-class type, `V {}` is the default value of
that type (`0`, `0.0`, `""`, `[]`, `nil`) and initializers are not allowed.
Once a constructor function exists for `T`, `T { ... }` is only allowed inside
one (error otherwise).

A vector literal `[e1, e2]` has element type the union (with coercion) of
the element types, each element converted to it; `[...]::T` makes the element
type `T` with each element converted (with coercion); `[]` has an inference
variable as element type. Elements that are structs are stored inline.

### Operators on numbers

For `+ - * / %` with both operands numeric: if either is `float`, the other
is converted and the result is `float`; otherwise the result is `int`. Enum
operands convert to `int` (any enum, or two different enums, may be mixed)
and the result is plain `int`. Integer `+`, `-`, `*` wrap modulo 2^64 (two's
complement). Integer `/` truncates toward
zero and `%` has the sign of the dividend, with `a == (a / b) * b + a % b`; a
divisor of `0` is a runtime error ("division by zero"), and `INT_MIN / -1` and
`INT_MIN % -1` are runtime errors ("integer overflow"). Float `/` by zero gives
`inf`, `-inf` or `nan` per IEEE 754; float `%` is C `fmod` (result has the
sign of the dividend). Unary `-` negates (integer `-INT_MIN` is `INT_MIN`; on
an enum the result is `int`).

Bitwise `& | ^` and `~` require `int` operands (enums convert) and produce
`int`, except that `& | ^` on two operands of the same `enum_flags` type, and
`~` on one, produce that `enum_flags` type. `<<` and `>>` require `int` and
produce `int`; the shift count is taken modulo 64 (`x << 64 == x`, `x << -1
== x << 63`); `<<` shifts the 64-bit pattern, `>>` is arithmetic (sign
propagating).

Comparisons `< > <= >=` on numbers follow the same operand conversion as
arithmetic (enums compare as `int`, so values of different enums may be
compared) and produce `bool`; `==`/`!=` likewise, and also accept: two
strings (by value), two vectors, two class references (identity; the operand
types must be related by conversion: comparing unrelated classes is an
error), two nilables, `nil` against a reference (a non-nilable reference is
simply never nil), two function values (identity), two resources (identity),
two typeids, two structs (see below). Floats compare per IEEE 754 (`nan` is
not equal to itself, `-0.0 == 0.0`).

`++`/`--` prefix and postfix apply to an lvalue of type `int` or `float`;
prefix yields the new value, postfix the old.

### Operators on strings

`s + x` where either operand is a string: the other operand, if not a string,
is converted to a string (as by the builtin `string`, see String
conversion) and the result is the concatenation. `s += x` converts `x`. `+` is
the only arithmetic operator on strings. Comparisons `< > <= >=` order strings
lexicographically by unsigned byte, a prefix ordering before the longer string;
`==` compares bytes. Strings are immutable: indexed assignment is an error.
The one exception is the byte-writing builtins (`ensure_size`, the `write_*_le`
family, `write_substring` and the imgui text inputs), which write into the
string they are given in place, visible thru every reference to that string,
and return it. A write past the end grows the string to end right after what
was written (bytes skipped over are 0): in place as well when the string's
allocation has room to spare (`string_with_capacity` makes an empty string
with room, and a write that had to move a string leaves it with as much room
again), otherwise into a larger copy, and other references keep the string as
it was. The `_back` variants, which write relative to the end, instead grow
the string to twice the length they need, with 0 bytes at the front, so what
was written keeps its place from the end. A string constant (a literal, or a
field default that is one) is never written: such a builtin copies it first
and returns the copy, so every evaluation of the literal yields its original
bytes.

### Operators on structs

For a numeric struct `S` with element type `N`: `S op S` for `+ - * / %`
applies the operator element-wise and yields `S`; `S op N` and `N op S` (a
scalar on either side) apply it to every element (an `int` scalar with a
`float` struct is converted; a `float` scalar with an `int` struct is an
error). Unary `-` negates element-wise. Comparisons `< > <= >=` between two
numeric structs, or a struct and a scalar, yield an `int` struct of the same
arity (`int2`, `int3` or `int4`, so arity 2 to 4 only) with `1`/`0` per
element. `==` between two structs of the same type (any struct, whatever the
mix of its field types) yields one `bool`: `a == b` is true when every field
of `a` equals the corresponding field of `b` under the `==` of that field's
type (`int` and `float` by value, strings by contents, class, vector and
resource references by identity, function values by identity, nested structs
by this same rule), and, for family structs, the dynamic types are equal and
the fields the static type does not declare (those of the dynamic type) are
equal slot by slot, numbers by value and references by identity;
`a != b` is `not (a == b)`. Indexing `s[i]` reads element
`i` (0-based, field order flattened) of a numeric struct; out of range is a
runtime error; `s[i] = v` assigns it. Compound assignments `s += x` etc. apply
the corresponding operator. Integer division and modulo per element check
their divisors like scalars. Any struct that is not numeric supports none of
these except `==`/`!=` as stated.

### Operator overloading

For a binary operator whose left operand has a class or struct type, unary
`-`, `~`, `++`, `--` whose operand does, `[` `]` whose object does, and `=` and
the compound assignments whose left operand does, the compiler looks for a
top-level function named `operator<op>` whose arity matches and one of whose
overloads takes a first parameter of a type related to the operand's type.
If found, the expression becomes a call to that function with the operands as
arguments, resolved as any call (so other overloads may decide on the second
operand); its result is the function's result. Otherwise the built-in meaning
applies. An assigning operator overload on a struct type is an error (structs
cannot model side effects). `operator=` on a class replaces the assignment
with a call (the variable is not rebound). Only the left (or sole) operand
selects an overload: `2 * x` never calls `operator*`.

### Logical operators and truth

A value is *true* when its 64-bit representation is non-zero: an `int` other
than 0, a `float` other than positive zero (so `-0.0` and `nan` are true), a
non-nil reference; function values are always true; `nil` and `0` are false.
Struct values cannot be tested (error in `if`, `while`, `not`, `and`/`or`,
`assert`, and the conditional `guard`).

`a and b`: evaluates `a`; if false, the value is `a`, else `b`. `a or b`:
evaluates `a`; if true, the value is `a`, else `b`. The type is the union
(without coercion) of the operand types, where a nilable left operand of `or`
contributes its non-nil type, and an `and` that is itself the left operand of
an `or` contributes only its right operand's type (`(a and b) or c` is typed
from `b` and `c`); if there is no union, or one operand is an enum and the
other a plain `int` or a different enum, both operands are converted to
`bool` (`1`/`0` by truth) and the result is `bool`. Thus `n or 1` with
`n:string?` is `true` of type `bool`, as is `1 or nil`, and `0 or 2.5` is
`true`. A bare `nil` (or a variable initialized from an untyped `nil`) has an
unbound element type, which unifies with the other operand's type; when that
type cannot be nilable this is an error (`nil or 1`, see Conversions).
`not a` yields `bool`: `true` when `a` is false. A condition whose type is a
non-nilable reference type gets a warning (it is always true).

### Field access and indexing

`e.f` reads field `f` of a class or struct value `e` (an error if `e`'s type
has no such field, if it is nilable, or if it is not a class/struct); `e.f =
v` writes it. A private field is only accessible in its file; a `member` field
only in its method. Reading a field through a nilable expression is an error
("dereferencing nillable type").

`v[i]` with `v:[T]` and `i:int` reads element `i`; `i` outside `0 .. length-1`
(negative included) is a runtime error. `v[p]` with `p` an `int` numeric
struct of arity N indexes N levels of nested vectors, last component
outermost: `m[int2 { x, y }]` is `m[y][x]`. `s[i]` with `s:string` reads byte
`i` as an `int` in `0..255`; `i` may equal the length (the result is `0`);
beyond that is a runtime error. Elements of vectors, and fields and elements
of structs stored in them, are lvalues (`v[i] = e`, `v[i].x = e`, `v[i] += e`).

### Calls

    callargs = `(` (expr (`,` expr)*)? `)` trailing*
             | trailing+                                 (no parentheses)
    trailing = `:` body
             | ident (`,` ident)* `:` body
             | `(` params `)` (`->` type)? `:` body
             | NL? `fn` ...

Call forms:

- `f(args)`: call of function or builtin `f` (namespace lookup applies).
- `x.f(args)`: `f(x, args)`, for any `x` (the receiver is the first argument;
  this is the only way to call a method-like function on a receiver, and
  works for any function whose first parameter accepts `x`, including
  builtins). `x.f` without `(` is a field access when the type of `x` has a
  field `f`, and otherwise the call `f(x)`.
- `f x` (see Names in expressions): a call with one argument.
- `f<T1, T2>(args)`: a call with explicit type arguments for the function's
  generics (fewer than declared is allowed; the rest are inferred).
- `super f(args)`: inside a method, a call of the superclass's implementation
  of `f` with `this` as receiver (static, no dispatch); an error if there is
  none.
- Trailing function values: after the argument list (or after the single
  argument of a call without parentheses), `: body` passes an anonymous
  function with implicit parameters, `x, y: body` one with named untyped
  parameters (not after a call without parentheses), `(x:T, y): body` one
  with typed parameters, and further function values follow on their own
  lines as `fn ...`. Each such value is one more argument. In a one-line
  trailing body a `,` after the expression ends the body (it separates the
  arguments of the enclosing call), so multiple assignment is not available
  there. An identifier after a call's closing `)` only starts a trailing
  function when followed by `,` or `:`.

Arguments are evaluated left to right, before the call. Which function a call
invokes is decided at compile time (see Call resolution), except that dynamic
dispatch may select an implementation at run time.

### Function values

    fnexpr = `fn` `(` params `)` (`->` type)? `:` body
           | `fn` ident (`,` ident)* `:` body
           | `fn` `:` body

An anonymous function is a value of a function type specific to that
function. Its parameters are typed or generic like a `def`'s; `-> T` coerces
the result. Its body's last statement's value is its return value (unlike a
named function); `return` inside it returns from the nearest enclosing *named*
function (or the program at file scope), not from the anonymous function.

Implicit parameters: inside an anonymous function whose parameters are not
declared (`fn: ...`, `f(): ...`), each distinct identifier starting with `_`
that is not a visible variable declares a parameter, in order of first
appearance (`_`, `_a`, `_b`). They cannot be mixed with declared parameters
and cannot be used in a named function or at file scope. In a `for` body with
no declared loop variables, `_` and `_name` declare the loop variables in the
same way (the element, then the index).

Free variables: function values carry no state (they are not closures). An
anonymous or nested named function may use variables of enclosing functions;
such a variable becomes a *free variable*: it gets a single storage location
for the whole program, like a global, which the declaring function saves on
entry and restores on exit. The nested function reads and writes that
location, so writes are visible both ways, and what it sees is the value of
the most recent activation of the declaring function that is still active,
whichever activation created the function value. A function value may only
be called while every function whose variables it uses is active (on the
call stack); a call that the compiler can see violates this is an error
("free variable not in scope").
Explicit free variables (`def f() [x]`) are bound at each call of `f`, at type
checking time, to the nearest variable named `x` among: variables declared
`nonlocal`, then the locals and parameters of the functions active at the
call (innermost first, including fields of `::` parameters); no such variable
is an error.

Function values are typed and called statically. A value of a specific
anonymous function type can be stored in a variable and called through it, and
passed to a parameter whose type is a declared function type `ft` (a bodiless
`def`): it must have the same number of parameters, each of `ft`'s parameter
types must convert to the corresponding parameter type, and its result must
convert to `ft`'s return type (or both be void). Only declared function types
can be the type of a field, vector element or explicitly typed variable;
values of one anonymous function type do not convert to another, so a vector
of distinct lambdas needs `::ft`. Calling a variable of function type passes
exactly the parameters; extra arguments to a function value whose function
takes fewer are dropped without being evaluated, fewer is an error. A
function value may assign to its parameters like any function, also when it
is called thru a declared function type (see Borrowing). `x()` calls the
function value held by `x` only when `x` is a variable (and no function or
builtin takes precedence, see Call resolution); calling any other expression
(`f()()`, `v[0]()`) is an error. Named functions are not values ("can't use
named function as value").

Functions as environments: a named function declared inside a function `g`
may also be called by name from code outside `g` while an activation of `g` is
on the call stack (the innermost active declaring function wins), provided no
lexically visible function of that name exists at the call and the call is
not lexically inside `g` in a block that does not contain the declaration.

### Conditional and switch expressions

`if c: a else: b` (and with `elif`) is an expression whose type is the union
(with coercion) of the branch types; each branch is a block whose last
statement provides the value. `if c: a` without `else` has no value. `switch`
is an expression whose type is the union (with coercion) of its cases' values;
on `int`, `float` and `string` values it must have a `default` when its value
is used. Cases ending in a terminal statement (a `return`, `break`,
`continue`, a call that never returns) do not contribute. See Statements for
the full forms.

A condition that is a compile-time constant selects its branch at compile
time; the other branch is discarded without type checking, which is what makes
`if x is int: ... else: ...` in a generic function specialize per type.

### is, typeof, assert, nil

`e is T` yields `bool`: whether the value of `e` has type `T`, where a value
of a subtype counts as having each of its supertypes. `e is T?` is true when
`e` is `nil` or `e is T`; the `?` is accepted here for any `T`, including
types that cannot otherwise be made nilable (so `1.5 is float?` is `1.5 is
float`). For values of scalar, string, vector and (non-family) struct types
the result is decided at compile time from the static types (`x is T` with
`x:T?` being a nil check at run time). A scalar is of its static type only:
an enum converts to `int` but `e is int` is false for an enum value `e`, and
`e is E` for an enum `E` is false for an `int` value `e` (such as `a | b` on
a plain `enum`) and for a value of another enum. For class and family struct
values it is a runtime test against the dynamic type: true if that type is
`T` or a subclass of `T`. If the static type makes the result certain, the
test folds to a constant (side effects of `e` are kept). `e` is not
evaluated more than once.

Unlike a conversion (see Conversions), the test never binds an inference
variable, which would make it true by construction. Where the static type of
`e`, or `T`, still has one, the result is decided as above only if no type
the variable could be bound to later changes it, and is an error ("`is`
cannot test a value whose type is not known yet") otherwise. So after `let
n = nil`, before any use determines its type, `n is int` is false and `n is
int?` is a nil check, since `int?` never exists, but `n is string` is an
error, as is `v[0] is int` after `let v = []`.

`typeof x` for a variable `x`, or `typeof T` for a type, is an `int` typeid
identifying the type in the program's type table (`typeof int` is always
`0`). Typeids convert to `int`; comparing typeids of different static types
is an error.

`assert e` evaluates `e`; if false, it is a runtime error ("assertion
failed: <expression>") unless assertions are compiled out (`--runtime-no-asserts`,
in which case `e` is still evaluated for its side effects and the result is
`e` itself). Its value and type are those of `e`, except that a nilable type
becomes non-nilable, and `e` is promoted for the following code (see
Flow-sensitive typing). A constant-true or non-nilable-reference assertion gets
a warning. `assert let x = e` declares `x` (see Conditional declarations).

`nil` is the nil value; its type is `T?` for a `T` inferred from use, or given
by `nil::T` (`T` must be nilable).

### String interpolation

A string literal with interpolations `"s0{e1}s1{e2}s2"` denotes the
expression `"s0" + e1 + "s1" + e2 + "s2"` (each `+` with a string left operand
converting the right operand, see Operators on strings), with empty pieces
omitted; when the literal starts with an interpolation, the first expression
is wrapped in the builtin `string()` to make it a string. Interpolated
expressions are `opexpr`s and may not contain assignments. Nested strings and
interpolations are allowed.

### Evaluation order

Operands of binary operators and arguments of calls are evaluated left to
right; `and`/`or` short-circuit. In an assignment `lhs = rhs` (and the
compound forms), `rhs` is evaluated first, then the subexpressions of `lhs`
(the object and index of `a[i]`, the object of `a.b`), each once. In `a, b =
e`, `e` is evaluated, then the targets are assigned right to left. The
initializers of a constructor and the elements of a vector literal are
evaluated in order. A `for` loop's iterable is evaluated once, before the
loop. A `switch` value is evaluated once.


Statements
----------

    statement = vardecl | staticdecl | memberdecl | fundecl | typedecl
              | uniondecl | enumdecl | importstmt | nsstmt | `private` statement
              | `attribute` ...
              | `if` ... | `guard` ... | `while` ... | `for` ... | `switch` ...
              | `return` ... | `break` | `continue` | `assert` expr
              | multiassign | expr

The value of a block is the value of its last statement (only a value where
the block is used as one). Statements produce `void` except expressions. A
`return` must be the last statement of its block. A declaration may not be
the last statement of a block (except at file end).

An expression statement whose expression has no side effects and is not the
last statement of its block gets a warning ("expression statement has no
effect"); calls always count as side effects. A `switch` or `if` used as a
statement discards its value.

### if, elif, else

    ifstmt = `if` cond `:` body (NL `elif` cond `:` body)* (NL `else` `:` body)?
    cond   = expr | (`let` | `var`) ident (`:` type)? `=` opexpr
             (`and` (`let` | `var`) ident ... `=` opexpr)*

The condition is any expression except a struct value; it is tested for truth.
`elif`/`else` must start a line at the same indentation as the `if` (or follow
the one-line body on the same line). A conditional declaration in the
condition declares the variable(s) in a scope that ends with the `if`
statement; the condition is the truth of the (last) declared variable, and
with `and` chains each subsequent initializer is only evaluated if the
previous variable is true. A conditional declaration is not allowed in an
`elif`. Any branch may be a block or a one-line statement. As a statement, the
branches need not agree in type.

### guard

    guardstmt = `guard` cond (`:` body)?

`guard c: exit` followed by the rest of the block is `if c: rest else: exit`;
`guard c` without a body is `if c: rest`. The exit body typically ends in
`return`, `break` or `continue` but need not. A `guard let x = e` declares `x`
in the enclosing block (visible in the rest of it, and holding the false value
inside the exit body).

### while

    whilestmt = `while` expr `:` body

Evaluates the condition before each iteration; runs the body while it is true.
No declaration is allowed in the condition. Promotions established by the
condition hold in the body.

### for

    forstmt = `for` `(` expr `)` loopvars? `:` body
            | `for` expr `:` body
    loopvars = ident (`,` ident)?
             | `(` ident ((`:` | `::`) type)? (`,` ident (`:` type)?)? `)`

The iterable is evaluated once. For an `int` `n`, the body runs for `i = 0, 1,
..., n-1` (not at all for `n <= 0`), the element and the index both being
`i`. For a string, the body runs once per byte, with the byte as an `int`
element. For a vector, the body runs for each index from `0` while the index
is less than the vector's current length (re-read every iteration, so
elements appended during the loop are visited and removed elements skipped),
with the element at that index (a copy of a struct element, a reference for a
reference element). The first loop variable is the element, the second the
index; at most two, each assignable in the body (a fresh binding per
iteration); `(x::T)` puts the element's fields in scope. With no loop
variables, `_` and `_name` in the body declare them (see Function values);
the form without parentheses around the iterable takes no loop variables.
The body is a block scope.

### break, continue

Only inside a `while` or `for` body of the same function; `break` leaves the
innermost enclosing loop, `continue` proceeds to its next iteration (the
condition or index increment). Both pass through any enclosing `switch` or
`if`.

### switch

    switchstmt = `switch` expr `:` INDENT (case NL)+ DEDENT
    case   = `case` pattern (`,` pattern)* (`:` body | `::` body)
           | `case` tname `(` (ident (`,` ident)*)? `)` `:` body
           | `default` `:` body
           | `out_of_range` `:` body
    pattern = unary | unary `..` unary | tname

The value must be `int` (including enums), `float`, `string`, a class, or a
family struct; a struct outside a family is an error. Patterns are unary
expressions (an operator expression needs parentheses); `a..b` is an
inclusive range. Cases are tried in order; the first whose pattern matches
runs, then the switch ends (no fall-through). For `int` values with all
patterns constant and dense enough, matching is by table; either way the
behavior is that of sequential matching except that a constant integer (or
integer range member) appearing in two cases is an error. Strings match by
value, floats by `==` and inclusive ranges. `default` runs when no case
matches; at most one.

Enums: a switch on an enum value without `default` must have a case for every
declared value of the enum (error "not tested in switch"); at run time a value
that matches no case (a value outside the enum's declared ones) is a runtime
error ("out of range of possible values in switch"), unless an `out_of_range`
case is present, which then runs. `out_of_range` is only allowed for enum
values, at most once, not together with `default`, and does not relieve the
exhaustiveness requirement. Inside an `out_of_range` body, the switch value
(if a variable or field path) has type `int`.

Types: on a class or family struct value, every pattern is a type name (a
class or struct); one type per case. The case whose type is the closest
supertype (or the type itself) of the value's dynamic type runs; `default`
covers the rest. Without `default`, every concrete (non-abstract) subtype of
the value's static type, as declared anywhere in the program, must be covered
(error "no case applies"); two cases equally close for some type is an error.
Inside the case, the switch value (if a variable or field path) has the case's
type. `case T:: body` additionally brings the fields of `T` into scope by
their names, and `T`'s methods callable without receiver, referring to the
switch value, which must then be a variable or a field path from one (`x`,
`x.a.b`). `case T(n1, n2): body` declares pattern variables `n1`, `n2` for the
first fields of `T` in declaration order (fewer than all is fine; `_` skips a
field; a name starting with `_` is an error); each is another name for that
field of the switch value (assignable, promotable). Pattern variables follow
the shadowing rules of variables. A type used twice in `::` scope for
different objects, or a field name that two objects in `::` scope both have,
is an error at use ("access to ambiguous field").

As an expression the value type is the union (with coercion) of the values of
the cases that can complete normally; a non-exhaustive `int`/`float`/`string`
switch used as a value must have `default`.

### return

    returnstmt = `return` (opexpr (`,` opexpr)*)? (`from` (ident | `program`))?

Without `from`, returns from the nearest lexically enclosing named function
(passing through any anonymous functions and blocks), or ends the program at
file scope. `return e from f` returns from the innermost active call of the
named function `f` (which must have a single overload and arity variant), and
`from program` ends the program; the function returned from must be on the
call stack: the compiler verifies that `f` is active in every calling context
the return is type checked in, and a return that would have to pass through a
call of a function value is an error ("cannot return through dynamic function
value").

The values: none (`return` alone, only valid in a function that returns no
value; error "returns 0 values, 1 needed" otherwise), one, or several (a
tuple). The function's return type is its declared one, to which each value
must convert (with coercion); with more declared values than the caller
requires, the extra are dropped; with fewer, an error. Without a declared
return type, the type is the union, without coercion, of the values of all
`return` statements the function contains (in the specialization), including
those in nested anonymous functions that return from it; a function whose
every path ends in a non-local return to another function has type `void`. A
function that is called recursively must declare its return type (error
otherwise). A named function's body that does not end in `return` returns
nothing (its last expression is evaluated and discarded); if the function
returns a value elsewhere (declared or inferred), reaching the end of the
body is an error ("control reaches end of non-void function"), and a declared
non-void function without any `return` is an error ("missing return
statement").

The number of values a call yields is the number the context consumes, up to
the number the function returns: in a single-value context the first value, in
a statement context none (the values are discarded), in `let a, b = f()`
two.

### assert (statement)

`assert e` as a statement is the assert expression with its value discarded.

### Multiple assignment

    multiassign = target (`,` target)+ `=` opexpr (`,` opexpr)*
    target = ident | postfix-with-field-access

Only as a statement (also as a one-line body). The right side is evaluated
first; it must produce at least as many values as there are targets (extra
are dropped). Targets are variables or fields (not indexed elements). Each
value must convert (without coercion) to its target's type.


Functions: resolution, dispatch and specialization
--------------------------------------------------

### Specialization

A function's body is type checked once per *specialization*: a combination of
the types bound to its generic parameters and type variables, the number of
return values the caller requires, the types (as promoted) of free variables it
uses, and the ownership convention chosen for its parameters. Each call site
either reuses a compatible specialization or creates a new one; a function
that returns a type still containing an inference variable is re-specialized
per call. Specialization is not observable except through the diagnostics it
produces and the "compile-time if" behavior of `is`.

A top-level function that is never called but whose parameters all have
concrete types is type checked anyway, for its errors (uses that need an
active caller, such as free variables, make the rest of the body skip). One
with generic parameters is not checked unless called.

### Call resolution

For a call of name `f` with arguments `a0, a1, ...` (after the receiver of a
`.` call has been placed first), the candidates are: the lexically visible
function `f` (with its arity variants), the builtin `f`, the field `f` (for a
receiver call without parentheses), and a variable `f` (for a non-receiver
call). The choice:

1. A `.f` without parentheses where the receiver's type has field `f` is a
   field access.
2. A variable `f` that holds a function value is called (as a dynamic call),
   unless a builtin `f` exists, or a function `f` is declared in a scope deeper
   than the variable's (a function declared in the same block as the
   variable counts as deeper).
3. Otherwise, among the arity variants of function `f`: a variant is viable if
   its parameter count equals the argument count, or exceeds it by parameters
   that have defaults, or (for a call without receiver, inside a method or a
   function with a `::` parameter) exceeds it by one that an implicit `this`
   argument can fill (the variant has an overload whose first parameter is
   `::` of a type the in-scope `this` converts to; the innermost such `this`
   is inserted as first argument). Among viable variants, if the first
   argument is a class or struct, the variant with an overload whose first
   parameter type is the closest supertype of it wins (a generic first
   parameter counts as a match farther than any concrete one); otherwise, and
   on ties, the variant with the most parameters wins. If a builtin `f` also
   exists, the builtin is called unless the chosen variant matched the
   receiver's class concretely, or has exactly the argument count and more
   parameters than the builtin. Missing arguments are filled with defaults
   (evaluated at the call). No viable variant is an error listing the declared
   ones.
4. Among the overloads of the chosen variant, one is selected by the argument
   types, argument by argument from the first: keep the overloads whose
   parameter type matches the argument exactly; if none, those whose
   parameter is the argument's generic type (any instantiation); if none, for
   a class/family argument, those whose parameter is a supertype (the closest
   only); if none, those the argument converts to without coercion (closest
   class supertype preferred); if none, those with a fully generic
   parameter; if none, those matching after applying the explicit type
   arguments; if none, those the argument converts to with coercion; if none,
   an error ("no overloads apply"). If one remains, it is chosen; if several
   remain after the last argument, and explicit type arguments select one by
   generic count, it is chosen, else the call is ambiguous (error).
5. Before step 4, if the first argument's static type is a class or family
   struct and any overload's first parameter type is a strict subtype of it,
   the call is a dynamic dispatch (below) instead.

Builtin calls: a builtin may have several overloads, tried with the one
declared first and then the rest in reverse declaration order; the first whose
parameters all accept the arguments (with the builtin conversion rules: `int`
to `float` coercion, numeric to nilable, a numeric struct argument for a
struct parameter of the right element type and arity, any vector for a generic
vector parameter, any reference type for a generic reference parameter, any
value for a parameter declared as converting to string (as `print` and
`string` do), and enum conversions) is called; missing optional arguments take
the builtin's default. Some builtins are typed by their
arguments (the return type is that of an argument, or the element type of a
vector argument, or the result of a function argument). The arity of a struct
argument is the slots it takes, so a struct field counts for as many as it has
and not for one. A builtin's numeric struct parameter of a specified arity is
one of 1 to 4, so a wider struct fits no overload; one of unspecified arity
takes a struct of any arity that is one of the prelude's `intN`/`floatN` types
or has the same element type ("cannot deduce struct type" otherwise).

### Dynamic dispatch

When a call dispatches dynamically on its first argument (a class or family
struct value with static type `C`), the implementation run is chosen at run
time by the argument's dynamic type `D`: the overload whose first parameter
type is the closest supertype of `D` (including `D` itself) among the
overloads of the function, with the remaining arguments resolved statically
against it. Every concrete (non-abstract) type in the program that is `C` or a
subtype of `C` must have such an implementation (error "no implementation for
D.f"); an ambiguity (two equally close) is an error. All implementations in a
dispatch are type checked for the call, must agree on the types of the
parameters other than the first (unless generic), and must agree on their
return type without coercion (the dispatch's return type is their union;
recursive dispatch requires declared return types). The set of subtypes is
that of the whole program as declared; a specialization of a generic class
created (by first use) after a dispatch on its superclass was type checked is
an error ("already used in dynamic dispatch ... before it has been declared"),
except where only a function that is never called creates it (see
Specialization).
A `super f(...)` call resolves statically to the implementation for the
superclass of the enclosing method's class. Methods of related types declared
in different scopes cannot dispatch together (error).

### Default arguments

A default is evaluated at the call site each time it is used, after the
explicit arguments, in the caller's context (it may read variables that were
visible at the function's declaration).

### Multiple return values and tuples

A function may return several values (`return a, b`, declared `-> A, B`).
They are consumed by `let a, b = f()`, `a, b = f()`, another `return f()`
(pass-through), or a builtin taking several; in an `if`/`switch` expression
branch several values need parentheses `(a, b)`. Any context consuming fewer
values drops the rest; one consuming more is an error.


Flow-sensitive typing
---------------------

The type checker tracks *promotions* of variables and field paths (a variable
followed by field accesses; indexed elements are never promoted) within a
function:

- After `if x:` (and inside `x and ...`), a nilable `x` has its non-nil type
  in the true branch; after `if not x:` or `if x == nil:` in the false branch
  (`x != nil` the true branch). Chains of `and` promote for later operands and
  the true branch; `or` promotes for the false branch. Only chains of one
  operator kind are analyzed.
- After `if x is T:`, `x` has type `T` in the true branch (for a nilable `x`,
  `x is T` with non-nilable `T` also excludes nil).
- `assert x` promotes `x` for the following code (nil excluded, `is` applied).
- Assigning a non-nil value to a nilable variable or field path promotes it for
  the following code (the promotion is the assigned type's non-nil form). A
  variable declared with a nilable annotation and a non-nil initializer is
  promoted likewise.
- In a `switch` case on a type, and in `out_of_range`, the switch value (if a
  path) is promoted.

A promotion is dropped by an assignment to the path or to any prefix of it,
including writes performed by called functions through parameters that alias
the path (the compiler tracks parameter aliases per call), writes through a
variable that was assigned the path's object from a variable path (`var y =
x` or `y = x.f`, a parameter, a `for` loop element: the fields of `y` are then
also those of `x` for every path `y` was ever assigned from, whether or not it
still holds that object), by `and`/`or` operands' assignments, and at the end
of the branch that established it. An alias that arrives any other way (a
function result, an element or field the object was stored in, the reference
fields of a copied struct) is not tracked: a write through it does not drop
the promotion, and neither does a write through an alias assigned later in a
loop body, on the next iteration. After
an `if`/`elif`/`else` or `switch`, a promotion survives only if every branch
that can fall through establishes it (with a common type); a branch that ends
in `return`/`break`/`continue` or a never-returning call is not one that
falls through. Demotions (assignments) in any branch, including ones followed
by `break`/`continue`, apply after the statement. A loop body's promotions do
not survive the loop; its demotions apply around it. Promotions of free
variables are visible inside nested functions called while they hold, and
functions are specialized on them.

Dereferencing a nilable value (`x.f` where `x:T?`, passing it where `T` is
required) is an error unless promoted.


Values, ownership and borrowing
-------------------------------

### Value and reference semantics

`int`, `float`, function values, typeids and structs are values: assignment
and passing copy them (a struct copy copies all its fields, including
reference fields, which are then shared). Strings, vectors, class instances
and resources are references: assignment and passing share the object;
`copy` and `deepcopy` (builtins) duplicate. Strings are immutable, except thru
the byte-writing builtins (see Operators on strings). Memory is
reclaimed by reference counting when the last reference disappears;
reference cycles are not reclaimed (they are reported as leaks when the program
ends).

### Borrowing

The compiler decides per parameter, variable and temporary whether it owns a
reference or borrows one, which is not observable except for one class of
errors. A *borrow* is taken of a value read from a variable, field path or
vector element (path `x`, `x.a.b`, `v[i]`, `v[..]`) when it is used as a
function argument, as the object of a field access or index, in the
initializer of a single-assignment variable, or as a `for` loop element. While
a borrow is live (until the enclosing expression or statement completes, or
for the variable's scope), a write to that location (or one that may alias
it: any element of the same vector counts as one location unless both indices
are different constants; builtins that remove elements count as writes; a
write through a variable that was assigned the object from a variable path
counts as a write to that path, with the same limits as for promotions, see
Flow typing) is
detected: if the write happens in a called function, after the borrowing
variable was initialized, or in a later argument of the same call (or a
later value of the same `return` list), e.g. `f(v[0], v.pop())`, the compiler
instead makes the borrower own a reference and the program is accepted; a
write elsewhere in the expression holding the borrow (e.g. `v[0] + v.pop()`,
where an operand borrows) is an error ("cannot modify `v[..]` while `v[0]`
borrows it").
A parameter that the function assigns to always owns its own reference; the
caller's variable is unaffected. This includes the parameters of an anonymous
function, whichever way it is called: a call thru a declared function type
passes every argument borrowed, and such a parameter copies it on entry.


Runtime semantics
-----------------

### Program execution and termination

Execution proceeds statement by statement from the start of the expanded
main file. `return e` at file scope (or `return e from program` anywhere)
ends the program with `e` as its result value (converted to a string for
`compile_run_code`; ignored by the main program, whose exit code is that set by
the exit code builtin, `0` by default). A runtime error prints its message
(with a source location and stack trace when compiled with those enabled) and
terminates the program with a non-zero exit code. Exceeding the host stack in
deep recursion is unspecified (a crash).

### Runtime errors defined by the language

- integer division or modulo by zero: "division by zero";
- `INT_MIN / -1`, `INT_MIN % -1`: "integer overflow";
- vector, string or struct index out of range;
- an enum `switch` value matching no case, without `out_of_range`;
- a failed `assert`;
- a `return from` passing through a function value call.

Builtins define their own errors (empty vector `pop`, etc.).

### String conversion

Values convert to strings (`string()`, `print`, interpolation, string `+`) as:
`int` in decimal; enum values by name (for `enum_flags`, the names of the set
bits joined by `|`), or in decimal when the value is not a declared value (for
flags: any undeclared bit set); `float` in fixed (non-exponent) decimal
notation with the shortest digits that read back to the same value, always
with a fractional part (`1.0`, `100000000000000000000.0`, `0.0000001`), `inf`,
`-inf`, `-0.0`, and `nan` in the host's spelling (unspecified, e.g.
`-nan(ind)`); strings as themselves at top level, and quoted with the escapes
`\n \t \r \\ \" \'` and `\xHH` for other bytes outside `0x20..0x7E` when
nested in a vector or object; vectors as `[e1, e2]`; class and struct values
as `Name{f1, f2}` with the dynamic type's name and its fields in order (a
family struct prints as the member it holds); `nil` as `nil`; function values
as `<FUNCTION:n>`; resources as `(resource:name)`; typeids as `int`. Nesting
deeper than the print depth (default 10) prints `..`, and output beyond the
print length budget is cut with `....`; both, the number of float decimals,
top-level string quoting and indentation are settable by builtins.

### Equality and hashing

`==` is as specified under operators. The builtin `equal` compares
structurally (vectors and objects element by element, recursively); `hash`
hashes ints, floats, enums and function values by value, strings by
contents, and vectors, objects and resources by identity.

### Numeric conversions

`float(i)` and the implicit `int` to `float` coercion convert to the nearest
double. `int(f)` truncates toward zero; for values outside the `int` range,
`nan` and infinities the result is unspecified (the host conversion; on the
reference platform `INT_MIN`).

### Threads

Worker threads run copies of the program with their own memory; the only
communication is by copying objects through the worker builtins. Nothing else
in the language is concurrent.
