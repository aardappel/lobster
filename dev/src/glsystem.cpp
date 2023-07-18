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

#include "lobster/stdafx.h"

#include "lobster/vmdata.h"
#include "lobster/glincludes.h"
#include "lobster/glinterface.h"
#include "lobster/sdlincludes.h"
#include "lobster/sdlinterface.h"

#ifdef PLATFORM_WINNIX
#define GLEXT(type, name, needed) type name = nullptr;
GLBASEEXTS GLEXTS
#undef GLEXT
#endif

float4 curcolor = float4_0;
float4x4 view2clip(1);
objecttransforms otransforms;
vector<Light> lights;
float pointscale = 1.0f;
float custompointscale = 1.0f;
bool mode2d = true;
bool mode_srgb = false;
GeometryCache *geomcache = nullptr;
int max_ssbo = 0, max_ubo = 0;

BlendMode SetBlendMode(BlendMode mode) {
    static BlendMode curblendmode = BLEND_NONE;
    if (mode == curblendmode) return curblendmode;
    switch (mode) {
        case BLEND_NONE:
            GL_CALL(glDisable(GL_BLEND));
            break;
        case BLEND_ALPHA:
            GL_CALL(glEnable(GL_BLEND));
            GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            break;
        case BLEND_ADD:
            GL_CALL(glEnable(GL_BLEND));
            GL_CALL(glBlendFunc(GL_ONE, GL_ONE));
            break;
        case BLEND_ADDALPHA:
            GL_CALL(glEnable(GL_BLEND));
            GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
            break;
        case BLEND_MUL:
            GL_CALL(glEnable(GL_BLEND));
            GL_CALL(glBlendFunc(GL_DST_COLOR, GL_ZERO));
            break;
        case BLEND_PREMULALPHA:
            GL_CALL(glEnable(GL_BLEND));
            GL_CALL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
            break;
    }
    BlendMode old = curblendmode;
    curblendmode = mode;
    return old;
}

void ClearFrameBuffer(const float3 &c) {
    GL_CALL(glClearColor(c.x, c.y, c.z, 1.0));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void SetScissorRect(int2 topleft, int2 size, pair<int2, int2>& prev) {
    int2 scrnsz = GetFrameBufferSize(GetScreenSize());
    GLboolean enabled;

    GL_CALL(glGetBooleanv(GL_SCISSOR_TEST, &enabled));
    if (enabled) {
        GL_CALL(glGetIntegerv(GL_SCISSOR_BOX, (GLint*)prev.first.data())); 
    } else {
        prev = {int2_0, scrnsz};
    }

    if (topleft.x == 0 && topleft.y == 0 && size == scrnsz) {
        GL_CALL(glDisable(GL_SCISSOR_TEST));
    } else {
        // Always get bitten by this, glScissor x & y are BOTTOM left, not top left.
        GL_CALL(glScissor((GLint)topleft.x, (GLint)(scrnsz.y - (topleft.y + size.y)), 
                         (GLint)size.x, (GLint)size.y));
        GL_CALL(glEnable(GL_SCISSOR_TEST));
    }
}

void DepthTest(bool on) {
    if (on)
        GL_CALL(glEnable(GL_DEPTH_TEST));
    else
        GL_CALL(glDisable(GL_DEPTH_TEST));
}

void Set2DMode(const int2 &ssize, bool lh, bool depthtest) {
    DepthTest(depthtest);
    CullFace(false);
    glViewport(0, 0, ssize.x, ssize.y);
    otransforms = objecttransforms();
    auto y = (float)ssize.y;
    view2clip = ortho(0, (float)ssize.x, lh ? y : 0, lh ? 0 : y, 1, -1);
    mode2d = true;
}

void Set3DOrtho(const int2 &ssize, const float3 &center, const float3 &extends) {
    DepthTest(true);
    CullFace(true);
    glViewport(0, 0, ssize.x, ssize.y);
    otransforms = objecttransforms();
    auto p = center + extends;
    auto m = center - extends;
    view2clip = ortho(m.x, p.x, p.y, m.y, m.z, p.z); // left handed coordinate system
    mode2d = false;
}

void Set3DMode(float fovy, int2 fbo, int2 fbs, float znear, float zfar, bool nodepth) {
    DepthTest(!nodepth);
    CullFace(true);
    glViewport(fbo.x, fbo.y, fbs.x, fbs.y);
    otransforms = objecttransforms();
    float ratio = fbs.x / (float)fbs.y;
    view2clip = perspective(fovy, ratio, znear, zfar, 1);
    mode2d = false;
}

void CullFace(bool on) {
    GL_CALL(if (on) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE));
}

void CullFront(bool on) {
    GL_CALL(glCullFace(on ? GL_FRONT : GL_BACK));
}

bool Is2DMode() { return mode2d; }
bool IsSRGBMode() { return mode_srgb; }

uint8_t *ReadPixels(const int2 &pos, const int2 &size) {
    auto pixels = new uint8_t[size.x * size.y * 3];
    for (int y = 0; y < size.y; y++)
        GL_CALL(glReadPixels(pos.x, pos.y + size.y - y - 1, size.x, 1, GL_RGB, GL_UNSIGNED_BYTE,
                             pixels + y * (size.x * 3)));
    return pixels;
}

void OpenGLFrameStart(const int2 &ssize) {
    GL_CALL(glDisable(GL_SCISSOR_TEST));
    GL_CALL(glViewport(0, 0, ssize.x, ssize.y));
    SetBlendMode(BLEND_ALPHA);
    curcolor = float4(1);
    lights.clear();
}

void OpenGLFrameEnd() {
    //uint8_t pixel[4];
    //glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    //glFlush();
    //glFinish();
    #if LOBSTER_FRAME_PROFILER
        FrameMark
    #endif
}

void OpenGLPostSwapBuffers() {
    #if LOBSTER_FRAME_PROFILER
        TracyGpuCollect;
    #endif
}

#ifdef PLATFORM_WINNIX
void DebugCallBack(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                   const GLchar *message, const void *) {
    (void)source;
    (void)type;
    (void)id;
    auto ll = OUTPUT_INFO;
    if (severity == GL_DEBUG_SEVERITY_HIGH) ll = OUTPUT_ERROR;
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM) ll = OUTPUT_WARN;
    else if (severity == GL_DEBUG_SEVERITY_LOW) ll = OUTPUT_INFO;
    // Suppress notifications in general
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
    if (ll < min_output_level) return;
    LogOutput(ll, "GLDEBUG: ", string_view(message, length));
}
#endif

string OpenGLInit(int samples, bool srgb) {
    GL_CHECK("before_init");
    LOG_INFO((const char *)glGetString(GL_VENDOR), " - ",
             (const char *)glGetString(GL_RENDERER), " - ",
             (const char *)glGetString(GL_VERSION));
    // If not called, flashes red framebuffer on OS X before first gl_clear() is called.
    ClearFrameBuffer(float3_0);
    #ifdef PLATFORM_WINNIX
        #define GLEXT(type, name, needed) { \
                union { void *proc; type fun; } funcast; /* regular cast causes gcc warning */ \
                funcast.proc = SDL_GL_GetProcAddress(#name); \
                name = funcast.fun; \
                if (!name && needed) return "no " #name; \
            }
        GLBASEEXTS GLEXTS
        #undef GLEXT
        GL_CALL(glEnable(GL_DEBUG_OUTPUT));
        if (glDebugMessageCallback && glDebugMessageInsert && glDebugMessageControl) {
            GL_CALL(glDebugMessageCallback(DebugCallBack, nullptr));
            // Turn off all notification messages, which are largely spam.
            GL_CALL(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
                                          0, nullptr, false));
            GL_CALL(glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 'TEST',
                                         GL_DEBUG_SEVERITY_LOW, 2, "on"));
            #ifndef NDEBUG
                // This allows breakpoints in DebugCallBack above that show the caller.
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            #else
                // Disabled only in Release to reduce spam.
                // FIXME: are the ids below NVidia specific?
                // "Buffer name does not refer to an buffer object generated by OpenGL"
                // Seems to originate from SDL_GL_SwapWindow and its not clear why.
                unsigned api_error_ids[] = { 0x00000502 };
                GL_CALL(glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR,
                                              GL_DONT_CARE, 1, api_error_ids, false));
                // "Vertex shader in program N is being recompiled based on GL state"
                // Seems impossible to track down which sequence of Lobster calls
                // produces this warning.
                unsigned api_perf_ids[] = { 0x00020092 };
                GL_CALL(glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PERFORMANCE,
                                              GL_DONT_CARE, 1, api_perf_ids, false));
            #endif
        }
        // FIXME: call glGetInteger64v if we ever want buffers >2GB.
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_ssbo);
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_ubo);
    #endif
    #ifndef PLATFORM_ES3
        GL_CALL(glEnable(GL_LINE_SMOOTH));
        GL_CALL(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
        GL_CALL(glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST));
        if (samples > 1) GL_CALL(glEnable(GL_MULTISAMPLE));
        if (srgb) {
            GL_CALL(glEnable(GL_FRAMEBUFFER_SRGB));
            mode_srgb = true;
        }
    #endif
    CullFront(true);
    if (!geomcache) geomcache = new GeometryCache();
    #if LOBSTER_FRAME_PROFILER
        #undef new
        TracyGpuContext;
        #if defined(_MSC_VER) && !defined(NDEBUG)
            #define new DEBUG_NEW
        #endif
    #endif
    return {};
}

void OpenGLCleanup() {
    if (geomcache) delete geomcache;
    geomcache = nullptr;
}

void LogGLError(const char *file, int line, const char *call) {
    auto err = glGetError();
    if (err == GL_NO_ERROR) return;
    const char *err_str = "<unknown error enum>";
    switch (err) {
        case GL_INVALID_ENUM:
            err_str = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            err_str = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            err_str = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            err_str = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            err_str = "GL_OUT_OF_MEMORY";
            break;
    }
    LOG_ERROR(file, "(", line, "): OpenGL Error: ", err_str, " from ", call);
    assert(false);
}

#ifdef _WIN32
// This magically forces GL to pick dedicated over integrated?
extern "C" {
_declspec(dllexport) DWORD NvOptimusEnablement = 1;
_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
