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

// Included after the platform GL declarations, on Windows/Linux only.
// Keep native imports available to glheadless.cpp when defining the dispatch pointers.
#define GLIMPORTS(F) \
    F(glBindTexture) \
    F(glBlendFunc) \
    F(glClear) \
    F(glClearColor) \
    F(glCullFace) \
    F(glDeleteTextures) \
    F(glDisable) \
    F(glDrawArrays) \
    F(glDrawElements) \
    F(glEnable) \
    F(glFinish) \
    F(glFlush) \
    F(glGenTextures) \
    F(glGetBooleanv) \
    F(glGetError) \
    F(glGetIntegerv) \
    F(glGetString) \
    F(glGetTexImage) \
    F(glHint) \
    F(glPixelStorei) \
    F(glReadPixels) \
    F(glScissor) \
    F(glTexImage2D) \
    F(glTexParameteri) \
    F(glViewport)
#ifndef _WIN32
    #define GLLINUXIMPORTS(F) \
        F(glActiveTexture) \
        F(glTexImage3D) \
        F(glBlendEquation)
#else
    #define GLLINUXIMPORTS(F)
#endif

#define GLIMPORT(name) extern decltype(&::name) lobster_##name;
GLIMPORTS(GLIMPORT) GLLINUXIMPORTS(GLIMPORT)
#undef GLIMPORT

extern void SetGLHeadless(bool headless);

#ifndef LOBSTER_GL_NATIVE_IMPORTS
#define glBindTexture lobster_glBindTexture
#define glBlendFunc lobster_glBlendFunc
#define glClear lobster_glClear
#define glClearColor lobster_glClearColor
#define glCullFace lobster_glCullFace
#define glDeleteTextures lobster_glDeleteTextures
#define glDisable lobster_glDisable
#define glDrawArrays lobster_glDrawArrays
#define glDrawElements lobster_glDrawElements
#define glEnable lobster_glEnable
#define glFinish lobster_glFinish
#define glFlush lobster_glFlush
#define glGenTextures lobster_glGenTextures
#define glGetBooleanv lobster_glGetBooleanv
#define glGetError lobster_glGetError
#define glGetIntegerv lobster_glGetIntegerv
#define glGetString lobster_glGetString
#define glGetTexImage lobster_glGetTexImage
#define glHint lobster_glHint
#define glPixelStorei lobster_glPixelStorei
#define glReadPixels lobster_glReadPixels
#define glScissor lobster_glScissor
#define glTexImage2D lobster_glTexImage2D
#define glTexParameteri lobster_glTexParameteri
#define glViewport lobster_glViewport
#ifndef _WIN32
#define glActiveTexture lobster_glActiveTexture
#define glTexImage3D lobster_glTexImage3D
#define glBlendEquation lobster_glBlendEquation
#endif
#endif
