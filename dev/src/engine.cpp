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

// Engine integration with Lobster VM.

#include "lobster/stdafx.h"

#include "lobster/compiler.h"  // For RegisterBuiltin().

#include "lobster/sdlinterface.h"

#include "lobster/glinterface.h"
#include "lobster/graphics.h"


// The code below allows for lightweight "plugins" to the
// engine, by seeing if there's a header file with this name
// in the "projects" directory of the Lobster repo.
// This header can define custom functions needed for your
// project, or include files that do, allowing you to extend
// Lobster without the needs for DLL/so files or whatever.
// "projects" is already in .gitignore so can even be a
// separate git repo.
// FIXME: Sadly this needs to be 0 when doing compiled_lobster builds
// (since it interferes with the builtin hashing)
#define ALLOW_PLUGINS 0
#if ALLOW_PLUGINS && defined(BUILD_CONTEXT_lobster) && \
         __has_include("../../projects/include/lobster_engine_plugins.h")
    // Which is expected to define its builtins into a BuiltinGroup plugin_builtins.
    #include "../../projects/include/lobster_engine_plugins.h"
    #define HAVE_PLUGINS
#endif


using namespace lobster;

extern BuiltinGroup graphics_builtins;
extern BuiltinGroup font_builtins;
extern BuiltinGroup sound_builtins;
extern BuiltinGroup physics_builtins;
extern BuiltinGroup meshgen_builtins;
extern BuiltinGroup cubegen_builtins;
extern BuiltinGroup vr_builtins;
extern BuiltinGroup steam_builtins;
extern BuiltinGroup imgui_builtins;
extern BuiltinGroup imguidebug_builtins;

namespace lobster {

FileLoader EnginePreInit(NativeRegistry &nfr) {
    RegisterBuiltin(nfr, "gl", "graphics", graphics_builtins);
    RegisterBuiltin(nfr, "gl", "font", font_builtins);
    RegisterBuiltin(nfr, "", "sound", sound_builtins);
    RegisterBuiltin(nfr, "ph", "physics", physics_builtins);
    RegisterBuiltin(nfr, "mg", "meshgen", meshgen_builtins);
    RegisterBuiltin(nfr, "cg", "cubegen", cubegen_builtins);
    RegisterBuiltin(nfr, "vr", "vr", vr_builtins);
    RegisterBuiltin(nfr, "steam", "steam", steam_builtins);
    RegisterBuiltin(nfr, "im", "imgui", imgui_builtins);
    RegisterBuiltin(nfr, "", "imguidebug", imguidebug_builtins);
    #ifdef HAVE_PLUGINS
        RegisterBuiltin(nfr, "", "plugin", plugin_builtins);
    #endif
    nfr.DoneRegistering();
    return SDLLoadFile;
}

}  // namespace lobster
