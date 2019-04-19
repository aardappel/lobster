---
title: The Lobster WASM code generator implementation
---

NOTE: this document contains detail on how code generation for the
WebAssembly backend works internally, if you just want to USE this backend,
please read the sections on WebAssembly [here](implementation.html).

Structure of the backend
------------------------
Lobster is structured such that the front-end outputs Lobster bytecode,
which sits in a FlatBuffer file together with all its metadata. That file
is stand-alone (does not need any further information from the front-end
or source code) and can be interpreted by the VM directly, or passed to
any of the native code backends, currently C++ and WebAssembly.

The backend is layered in these 3 files:

* `tonative.cpp` is shared between all native code backends. It parses the
   Lobster bytecode and tracks information needed by any backend, then calls
   into the specific backend through an interface defined in `tonative.h`.
* `towasm.cpp` implements that interface, and does all the Lobster-specific
  work of generating wasm code.
* `wasm_binary_writer.h` is a utility class that does all the heavy lifting
  of actually generating a valid wasm module. It is written such that it is
  entirely independent of the rest of the Lobster code base, and depends only
  on a few STL containers, to ensure it can easily be adopted by others
  wishing to emit wasm. It contains functionality for low level encoding
  (LEBs etc), emitting sections, individual instructions, and importantly,
  linking and relocation information.

Design of the binary writer
---------------------------
Most languages, like Lobster, come with a runtime in C or C++ that you wouldn't
want to translate to wasm by hand (or worse, emit instruction by instruction
from a backend), so it is imporant that generated wasm code can be linked
correctly against a runtime compiled to object files by the C/C++ compiler.
In this case we're piggy-backing on Clang, LLD and Binaryen as used in the
Emscripten toolchain.

This means generating a wasm module with linking information, and supplying
the right relocations. Many instructions in wasm refer to functions and other
things by index, but the linker has to merge many such index spaces into
the final module, which means renumbering all of these indices. Luckily, the
binary writer takes care of all this automatically, and allows us the generate
code as-if we're the only ones.

### Streaming interface.
The binary writer is a "streaming" API (as opposed to an object
API), meaning that it immediately writes out the wasm module data with no
or few intermediary data structures. This has the advantage that it is
low in memory usage and is very fast, with the downside being that most
things have to be supplied to it in order. This should not be very difficult
for most code generators, however.

A wasm module should contain sections in order, and the API enforces
this same order:

~~~~
Type     1    Function signature declarations
Import   2    Import declarations
Function 3    Function declarations
Table    4    Indirect function table and other tables
Memory   5    Memory attributes
Global   6    Global declarations
Export   7    Exports
Start    8    Start function declaration
Element  9    Elements section
Code    10    Function bodies (code)
Data    11    Data segments
~~~~

What is not shown here is custom sections, which may appear before and
after each of these sections. One such custom section is the linking
section, which appears at the end of all of the above. Its contents is
generated automatically by the binary writer as a consequence of all the
things emitted in the above section (primarily the Code section).

### Simple example.

A minimal usage of the API, where we generate code for "hello world",
assuming the printing function is in the runtime.

Note that this assumes a minimum familiarity with wasm in general.
I am not going to explain the "why" of this design, if you end up using
this API then reading up on the wasm module format would be useful.

~~~~cpp
vector<uint8_t> bytes;
WASM::BinaryWriter bw(bytes);

// Write a (function) type section, to be referred to by functions below.
// For any of these sections, if you write them out of order, or don't match
// begin/end, you'll get an assert.
// As with everything, to refer to things in wasm, use a 0 based index.
bw.BeginSection(WASM::Section::Type);
// A list of arguments followed by a list of return values.
// You don't have to use the return value, but it may make referring to this
// type easier.
auto type_ii_i = bw.AddType({ WASM::I32, WASM::I32 }, { WASM::I32 });  // 0
auto type_i_v = bw.AddType({ WASM::I32 }, {});  // 1
bw.EndSection(WASM::Section::Type);

// Import some functions, from the runtime compiled in other modules.
// For our example that will just be the printing function.
// Note: we assume this function has been declared with: extern "C"
// You can link against C++ functions as well if you don't mind dealing
// with name mangling.
bw.BeginSection(WASM::Section::Import);
auto import_print = bw.AddImportLinkFunction("print", type_i_v);  // 0
bw.EndSection(WASM::Section::Import);

// Declare all the functions we will generate. Note this is just the type,
// the body of the code will follow below.
bw.BeginSection(WASM::Section::Function);
bw.AddFunction(type_ii_i);  // main()
bw.EndSection(WASM::Section::Function);

// Declare the linear memory we want to use, with 1 initial page.
bw.BeginSection(WASM::Section::Memory);
bw.AddMemory(1);
bw.EndSection(WASM::Section::Memory);

// Here we'd normally declare a "Start" section, but the linker will
// take care for that for us.

// Now the exciting part: emitting function bodies.
bw.BeginSection(WASM::Section::Code);

// A list of 0 local types,
bw.AddCode({}, "main", false);
// Refers to data segment 0 at offset 0 below. This emits an i32.const
// instruction, whose immediate value will get relocated to refer to the
// data correctly.
bw.EmitI32ConstDataRef(0, 0);
bw.EmitCall(import_print);
bw.EmitEndFunction();

// Here, call AddCode..EmitEndFunction for more functions.

bw.EndSection(WASM::Section::Code);

// Add all our static data.
bw.BeginSection(WASM::Section::Data);
// This is our first segment, we referred to this above as 0.
auto hello = "Hello, World\n\0"sv;
// Data, name, and alignment.
bw.AddData(hello, "hello", 0);
bw.EndSection(WASM::Section::Data);

// This call does all the remaining work of generating the linking
// information, and wrapping up the file.
bw.Finish();

// Here, we can write the contents of "bytes" above to a file.
// Then, using emcc to link print.c and generated.o should actually
// produce a valid module!
~~~~

### The Lobster generator.

A more complex example using the binary writer is the Lobster generator
in `towasm.cpp`, which follows the same pattern as the above simple example.
Because it is being driven by calls from `tonative.cpp` it is in separate
functions, but these functions are in the order of being called, so should
be easy to follow.

In terms of imports, it imports one function for each of Lobsters bytecodes,
which means a Lobster bytecode can be directly mapped to a function id.
Then there are a couple of additional functions in the runtime it needs,
most importantly `EngineRunCompiledCodeMain` which it calls with the starting
compiled function, such that runtime can initialize itself and then call
back into the Lobster compiled code. This structure was chosen such that
the maximum amount of code can live in the runtime, and the minimum needs
to be explicitly generated.

It generates Lobster code not on a per function basis, but per basic block.
This is because Lobster has more complicated control flow features (such
as non-local returns and co-routines) which (at the time of writing)
wasm can't support natively. So it uses a "trampoline" system where each
basic block returns the function address of the next basic block it wants
to go to. Some of these transitions can be optimized, but in the most
general case this has to be an indirect jump.

The actual instructions generated are pretty simple, with it mostly
emitting a call per VM instruction, with the exception of control flow
operations which have more special purpose implementation.

(to be expanded)
