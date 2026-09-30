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

// Do not redirect the native symbols used to initialize the legacy dispatch table.
#define LOBSTER_GL_NATIVE_IMPORTS
#include "lobster/glincludes.h"

#ifdef PLATFORM_WINNIX

#define GLIMPORT(name) decltype(&::name) lobster_##name = &::name;
GLIMPORTS(GLIMPORT) GLLINUXIMPORTS(GLIMPORT)
#undef GLIMPORT

namespace {

// No buffers, pixels, shader binaries, or driver objects are stored here. The engine
// still owns its normal CPU resources and updates their metadata/dirty state.
GLuint next_id = 1;
GLboolean scissor_enabled = GL_FALSE;
GLint scissor_box[4] = {};
GLint viewport[4] = {};

[[noreturn]] void Unsupported() {
    THROW_OR_ABORT("Unsupported OpenGL operation in headless mode (readback/mapping or query)");
}

// Match both the exact signature and GL calling convention. Unclassified functions
// fail instead of leaving output parameters unwritten or leaking through to a driver.
template<typename> struct HeadlessFunction;
template<typename R, typename... Args> struct HeadlessFunction<R (APIENTRY *)(Args...)> {
    static R APIENTRY Fail(Args...) { Unsupported(); }
    static void APIENTRY Noop(Args...) { static_assert(std::is_void_v<R>); }
};

void APIENTRY GenObjects(GLsizei count, GLuint *ids) {
    for (GLsizei i = 0; i < count; i++) {
        if (next_id == 0 || next_id > GLuint(INT_MAX)) Unsupported();
        ids[i] = next_id++;
    }
}

GLuint APIENTRY CreateProgram() { GLuint id; GenObjects(1, &id); return id; }
GLuint APIENTRY CreateShader(GLenum) { return CreateProgram(); }
GLboolean APIENTRY IsProgram(GLuint id) { return id != 0; }
GLenum APIENTRY GetError() { return GL_NO_ERROR; }
GLenum APIENTRY CheckFramebufferStatus(GLenum) { return GL_FRAMEBUFFER_COMPLETE; }

const GLubyte *APIENTRY GetString(GLenum name) {
    switch (name) {
        case GL_VENDOR: case GL_RENDERER: return (const GLubyte *)"Lobster headless";
        case GL_VERSION: return (const GLubyte *)"4.3 headless";
        case GL_SHADING_LANGUAGE_VERSION: return (const GLubyte *)"4.30 headless";
        case GL_EXTENSIONS: return (const GLubyte *)"";
        default: Unsupported();
    }
}

void APIENTRY GetIntegerv(GLenum name, GLint *value) {
    switch (name) {
        case GL_MAX_SHADER_STORAGE_BLOCK_SIZE: *value = INT_MAX; break;
        case GL_MAX_UNIFORM_BLOCK_SIZE: *value = 65536; break;
        case GL_MAX_TEXTURE_SIZE: *value = 16384; break;
        case GL_NUM_EXTENSIONS: *value = 0; break;
        case GL_MAJOR_VERSION: *value = 4; break;
        case GL_MINOR_VERSION: *value = 3; break;
        case GL_SCISSOR_BOX: memcpy(value, scissor_box, sizeof(scissor_box)); break;
        case GL_VIEWPORT: memcpy(value, viewport, sizeof(viewport)); break;
        default: Unsupported();
    }
}

void APIENTRY GetBooleanv(GLenum name, GLboolean *value) {
    if (name != GL_SCISSOR_TEST) Unsupported();
    *value = scissor_enabled;
}
void APIENTRY Enable(GLenum cap) { if (cap == GL_SCISSOR_TEST) scissor_enabled = GL_TRUE; }
void APIENTRY Disable(GLenum cap) { if (cap == GL_SCISSOR_TEST) scissor_enabled = GL_FALSE; }
void APIENTRY Scissor(GLint x, GLint y, GLsizei w, GLsizei h) {
    scissor_box[0] = x; scissor_box[1] = y; scissor_box[2] = w; scissor_box[3] = h;
}
void APIENTRY Viewport(GLint x, GLint y, GLsizei w, GLsizei h) {
    viewport[0] = x; viewport[1] = y; viewport[2] = w; viewport[3] = h;
}

void APIENTRY GetShaderiv(GLuint, GLenum name, GLint *value) {
    switch (name) {
        case GL_COMPILE_STATUS: *value = GL_TRUE; break;
        case GL_INFO_LOG_LENGTH: *value = 0; break;
        default: Unsupported();
    }
}
void APIENTRY GetProgramiv(GLuint, GLenum name, GLint *value) {
    switch (name) {
        case GL_LINK_STATUS: case GL_VALIDATE_STATUS: *value = GL_TRUE; break;
        case GL_INFO_LOG_LENGTH: case GL_ACTIVE_UNIFORMS: case GL_ACTIVE_ATTRIBUTES:
            *value = 0; break;
        default: Unsupported();
    }
}
void APIENTRY GetInfoLog(GLuint, GLsizei size, GLsizei *length, GLchar *log) {
    if (length) *length = 0;
    if (size > 0 && log) *log = '\0';
}

// Uniform writes are discarded, so no GLSL reflection or per-program storage is needed.
GLint APIENTRY GetUniformLocation(GLuint, const GLchar *) { return 0; }
GLuint APIENTRY GetUniformBlockIndex(GLuint, const GLchar *) { return 0; }
GLuint APIENTRY GetProgramResourceIndex(GLuint, GLenum, const GLchar *) { return 0; }
void APIENTRY GetQueryObjectiv(GLuint, GLenum name, GLint *value) {
    if (name != GL_QUERY_RESULT && name != GL_QUERY_RESULT_AVAILABLE) Unsupported();
    *value = name == GL_QUERY_RESULT_AVAILABLE ? GL_TRUE : 0;
}
void APIENTRY GetQueryObjectui64v(GLuint, GLenum name, GLuint64 *value) {
    if (name != GL_QUERY_RESULT && name != GL_QUERY_RESULT_AVAILABLE) Unsupported();
    *value = name == GL_QUERY_RESULT_AVAILABLE ? GL_TRUE : 0;
}

}  // namespace

void SetGLHeadless(bool headless) {
    #define GLIMPORT(name) lobster_##name = headless ? HeadlessFunction<decltype(&::name)>::Fail : &::name;
    GLIMPORTS(GLIMPORT) GLLINUXIMPORTS(GLIMPORT)
    #undef GLIMPORT
    if (!headless) return;  // Extension pointers are loaded normally by OpenGLInit.

    next_id = 1;
    scissor_enabled = GL_FALSE;
    memset(scissor_box, 0, sizeof(scissor_box));
    memset(viewport, 0, sizeof(viewport));
    #define GLEXT(type, name, needed) name = HeadlessFunction<type>::Fail;
    GLBASEEXTS GLEXTS
    #undef GLEXT

    #define NOOP(name) name = HeadlessFunction<decltype(name)>::Noop
    NOOP(lobster_glBindTexture);
    NOOP(lobster_glBlendFunc);
    NOOP(lobster_glClear);
    NOOP(lobster_glClearColor);
    NOOP(lobster_glCullFace);
    NOOP(lobster_glDeleteTextures);
    NOOP(lobster_glDrawArrays);
    NOOP(lobster_glDrawElements);
    NOOP(lobster_glFinish);
    NOOP(lobster_glFlush);
    NOOP(lobster_glHint);
    NOOP(lobster_glPixelStorei);
    NOOP(lobster_glTexImage2D);
    NOOP(lobster_glTexParameteri);
    #ifdef _WIN32
        NOOP(glActiveTexture);
        NOOP(glTexImage3D);
        NOOP(glBlendEquation);
    #else
        NOOP(lobster_glActiveTexture);
        NOOP(lobster_glTexImage3D);
        NOOP(lobster_glBlendEquation);
    #endif
    NOOP(glBindBuffer);
    NOOP(glBufferData);
    NOOP(glBufferSubData);
    NOOP(glDeleteBuffers);
    NOOP(glVertexAttribPointer);
    NOOP(glEnableVertexAttribArray);
    NOOP(glDisableVertexAttribArray);
    NOOP(glBindVertexArray);
    NOOP(glDeleteVertexArrays);
    NOOP(glDeleteProgram);
    NOOP(glDeleteShader);
    NOOP(glUseProgram);
    NOOP(glShaderSource);
    NOOP(glCompileShader);
    NOOP(glAttachShader);
    NOOP(glDetachShader);
    NOOP(glLinkProgram);
    NOOP(glUniform1f);
    NOOP(glUniform2f);
    NOOP(glUniform3f);
    NOOP(glUniform4f);
    NOOP(glUniform1fv);
    NOOP(glUniform2fv);
    NOOP(glUniform3fv);
    NOOP(glUniform4fv);
    NOOP(glUniform1iv);
    NOOP(glUniform2iv);
    NOOP(glUniform3iv);
    NOOP(glUniform4iv);
    NOOP(glUniform1i);
    NOOP(glUniformMatrix2fv);
    NOOP(glUniformMatrix3x2fv);
    NOOP(glUniformMatrix2x3fv);
    NOOP(glUniformMatrix3fv);
    NOOP(glUniformMatrix4fv);
    NOOP(glUniformMatrix3x4fv);
    NOOP(glUniformMatrix4x3fv);
    NOOP(glBindAttribLocation);
    NOOP(glBlendEquationSeparate);
    NOOP(glBlendFuncSeparate);
    NOOP(glBindSampler);
    NOOP(glDrawElementsBaseVertex);
    NOOP(glBindRenderbuffer);
    NOOP(glDeleteRenderbuffers);
    NOOP(glBindFramebuffer);
    NOOP(glDeleteFramebuffers);
    NOOP(glFramebufferTexture2D);
    NOOP(glRenderbufferStorage);
    NOOP(glFramebufferRenderbuffer);
    NOOP(glTexImage2DMultisample);
    NOOP(glRenderbufferStorageMultisample);
    NOOP(glBlitFramebuffer);
    NOOP(glGenerateMipmap);
    NOOP(glDispatchCompute);
    NOOP(glBindImageTexture);
    NOOP(glShaderStorageBlockBinding);
    NOOP(glUniformBlockBinding);
    NOOP(glBindBufferBase);
    NOOP(glMemoryBarrier);
    NOOP(glCopyBufferSubData);
    NOOP(glDeleteQueries);
    NOOP(glBeginQuery);
    NOOP(glEndQuery);
    NOOP(glQueryCounter);
    NOOP(glObjectLabel);
    NOOP(glDebugMessageCallback);
    NOOP(glDebugMessageControl);
    NOOP(glDebugMessageInsert);
    NOOP(glTexStorage2D);
    NOOP(glTexStorage3D);
    #undef NOOP

    lobster_glGenTextures = glGenBuffers = glGenVertexArrays = glGenFramebuffers =
        glGenRenderbuffers = glGenQueries = GenObjects;
    lobster_glEnable = Enable;
    lobster_glDisable = Disable;
    lobster_glScissor = Scissor;
    lobster_glViewport = Viewport;
    lobster_glGetError = GetError;
    lobster_glGetString = GetString;
    lobster_glGetIntegerv = GetIntegerv;
    lobster_glGetBooleanv = GetBooleanv;
    glCreateProgram = CreateProgram;
    glCreateShader = CreateShader;
    glIsProgram = IsProgram;
    glGetShaderiv = GetShaderiv;
    glGetProgramiv = GetProgramiv;
    glGetProgramInfoLog = glGetShaderInfoLog = GetInfoLog;
    glGetUniformLocation = GetUniformLocation;
    glGetUniformBlockIndex = GetUniformBlockIndex;
    glGetProgramResourceIndex = GetProgramResourceIndex;
    glCheckFramebufferStatus = CheckFramebufferStatus;
    glGetQueryObjectiv = GetQueryObjectiv;
    glGetQueryObjectui64v = GetQueryObjectui64v;
}

#endif  // PLATFORM_WINNIX
