// This is an example Lobster file to demonstrate syntax highlighting.
//
// This file isn't expected to do anything in particular, it attempts to use all
// the language grammar.
//
// For example, this is a single line comment.

/* Any this is a multi-line nested
   comment.
*/

namespace syntax
import threads
import from "shooter_tutorial/tut1.lobster"

// Numbers
let integer1 = 1234
let integer2 = 1
let float1 = 1.234
let float2 = .234
let float3 = 1.
let float4 = 1000.0
let hex = 0xDEADBEEF
let hex_mix = 0xDeAdBeEf

// Strings
let string1 = "I am a string"
let string2 = "I am a string with 'internal quotes'"
let string3 = """I'm a super string
   that can contain escape codes \n \t 
   and embedded strings: "like this"
   and code segements like:
      
      let integer1 = 1234    
       
"""

// Constant Characters
let const_char1 = 'a'
let const_char2 = '\n'
let const_char3 = '\t'
let const_char4 = '\r'
let const_char5 = '\"'
let const_char6 = '\''
let const_char7 = '\'
let const_char8 = '\xFF'
let const_char9 = '5'

// Types
var int_type = int
var float_type = float
var string_type = string
var vector_type = [float]
var void_type = void
var resouce_type = resource
// bool is actually a built-in enum, but is treated as a first-class type.
var bool_type = bool

// Constants
var nil_literal = nil
var nil_with_type_literal = nil::float
var true_literal = true
var false_literal = false

// Identifiers
var random_ident1 = "test"
var random_ident2 = random_ident1
var _indent_with_underscore = true
var a = "single character identifier"

// Statements

// Assignment
let using_let = 1
var using_var = 1
using_var = 2

let using_let_no_space=1234
var using_var_no_space=1234
using_let_no_space=4321

// Multiple Assignment
var a,b = 1,2

// Functions
def somefunction1(arg1, arg2):
   return arg1 + arg2

def somefunction2(arg1, with_type::float, with_single_colon:int):
   return arg1 + with_type

def noParameterFunction():
   return true

def withReturn() -> string:
   return "a string"

def withCustomReturnType() -> xyz:
   return xzy {}

def withReturnNospace()->int:
   return 1234

// Enums
enum someEnum:
   foo = 1
   bar
   baz

enum_flags flagEnum:
   foo = 1
   bar
   baz

// Struct
struct xy:
   x:int
   y:int

struct xyz : xy
   z:int

// Keywords

// Control
if a < 0:
    print "negativity not allowed!"
elif a < 10:
    print "single digit!"
elif a < 100:
    print "double digit!"
else:
    print "way too big!"

