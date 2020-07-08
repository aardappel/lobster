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

// Engine integration with Lobster VM and frame management.

#include "lobster/stdafx.h"

#include "lobster/compiler.h"  // For RegisterBuiltin().
#ifdef __EMSCRIPTEN__
    #include "emscripten.h"
#endif

#include "lobster/sdlinterface.h"

#include "lobster/engine.h"

#include "lobster/compiler.h"

using namespace lobster;

void RegisterCoreEngineBuiltins(NativeRegistry &nfr) {
    RegisterCoreLanguageBuiltins(nfr);

    extern void AddGraphics(NativeRegistry &nfr); RegisterBuiltin(nfr, "graphics",  AddGraphics);
    extern void AddFont(NativeRegistry &nfr);     RegisterBuiltin(nfr, "font",      AddFont);
    extern void AddSound(NativeRegistry &nfr);    RegisterBuiltin(nfr, "sound",     AddSound);
    extern void AddPhysics(NativeRegistry &nfr);  RegisterBuiltin(nfr, "physics",   AddPhysics);
    extern void AddNoise(NativeRegistry &nfr);    RegisterBuiltin(nfr, "noise",     AddNoise);
    extern void AddMeshGen(NativeRegistry &nfr);  RegisterBuiltin(nfr, "meshgen",   AddMeshGen);
    extern void AddCubeGen(NativeRegistry &nfr);  RegisterBuiltin(nfr, "cubegen",   AddCubeGen);
    extern void AddOcTree(NativeRegistry &nfr);   RegisterBuiltin(nfr, "octree",    AddOcTree);
    extern void AddVR(NativeRegistry &nfr);       RegisterBuiltin(nfr, "vr",        AddVR);
    extern void AddSteam(NativeRegistry &nfr);    RegisterBuiltin(nfr, "steam",     AddSteam);
    extern void AddIMGUI(NativeRegistry &nfr);    RegisterBuiltin(nfr, "imgui",     AddIMGUI);
}

void EngineExit(int code) {
    GraphicsShutDown();
    exit(code); // Needed at least on iOS to forcibly shut down the wrapper main()
}

void EngineRunByteCode(VMArgs &&vmargs) {
    lobster::VMAllocator vma(std::move(vmargs));
    vma.vm->EvalProgram();
}

extern "C" int RunCompiledCodeMain(int argc, char *argv[],
                                   const void *bytecodefb, size_t static_size,
                                   const lobster::block_base_t *vtables) {
    #ifdef USE_EXCEPTION_HANDLING
    try
    #endif
    {
        NativeRegistry nfr;
        RegisterCoreEngineBuiltins(nfr);
        EngineRunByteCode(CompiledInit(argc, argv, bytecodefb, static_size, vtables,
                                       SDLLoadFile, nfr));
    }
    #ifdef USE_EXCEPTION_HANDLING
    catch (string &s) {
        LOG_ERROR(s);
        EngineExit(1);
    }
    #endif
    EngineExit(0);
    return 0;
}
