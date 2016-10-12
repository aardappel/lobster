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

// simple interface for SDL (that doesn't depend on its headers)

extern string SDLInit(const char *title, const int2 &screensize, bool fullscreen, int vsync);
extern bool SDLFrame();
extern void SDLShutdown();
extern void SDLTitle(const char *title);
extern bool SDLIsMinimized();

extern const int2 &GetScreenSize();

extern const int2 &GetFinger(int i, bool delta);
extern TimeBool8 GetKS(const char *name);
extern double GetKeyTime(const char *name, int on);
extern int2 GetKeyPos(const char *name, int on);
extern float GetJoyAxis(int i);

extern double SDLTime();
extern double SDLDeltaTime();

extern int SDLWheelDelta();

extern bool SDLCursor(bool on);
extern bool SDLGrab(bool on);

extern bool SDLPlaySound(const char *filename, bool sfxr);
extern void SDLSoundClose();

extern uchar *SDLLoadFile(const char *absfilename, size_t *len);

extern bool ScreenShot(const char *filename);

extern void SDLTestMode();

extern int SDLScreenDPI(int screen);


extern void RegisterCoreEngineBuiltins();
extern bool GraphicsFrameStart();
extern void GraphicsShutDown();
extern void EngineExit(int code);
extern bool EngineRunByteCode(const char *fn, vector<uchar> &&bytecode, const void *entry_point, 
                              const void *static_bytecode);
extern int EngineRunCompiledCodeMain(int argc, const char *argv[], const void *entry_point, const void *bytecodefb);

#ifdef __EMSCRIPTEN__
#define USE_MAIN_LOOP_CALLBACK
#endif
