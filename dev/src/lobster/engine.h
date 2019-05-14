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

#ifndef LOBSTER_ENGINE
#define LOBSTER_ENGINE

#include "lobster/natreg.h"

extern void RegisterCoreEngineBuiltins(lobster::NativeRegistry &natreg);
extern void EngineRunByteCode(lobster::NativeRegistry &natreg, const char *fn, string &bytecode,
                              const void *entry_point, const void *static_bytecode,
                              size_t static_size, const vector<string> &program_args,
                              const lobster::block_t *vtables);
extern "C" int EngineRunCompiledCodeMain(int argc, char *argv[], const void *entry_point,
                                         const void *bytecodefb, size_t static_size,
                                         const lobster::block_t *vtables);
extern void EngineSuspendIfNeeded();
extern void EngineExit(int code);

#ifdef __EMSCRIPTEN__
#define USE_MAIN_LOOP_CALLBACK
#endif

#endif  // LOBSTER_ENGINE
