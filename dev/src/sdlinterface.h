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

extern string SDLInit(const char *title, int2 &screensize, bool fullscreen);
extern bool SDLFrame(int2 &screensize);
extern void SDLShutdown();
extern void SDLTitle(const char *title);
extern bool SDLIsMinimized();


struct UpDown
{
    bool isdown;
    bool wentdown;
    bool wentup;

    UpDown() : isdown(false), wentdown(false), wentup(false) {}
};


extern const int2 &GetFinger(int i, bool delta);
extern UpDown GetKS(const char *name);
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

extern bool ScreenShot(const char *filename, const int2 &screensize);

extern void SDLTestMode();
