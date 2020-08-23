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
#include "lobster/glinterface.h"
#include "lobster/glincludes.h"
#include "lobster/sdlinterface.h"

map<string, Shader *, less<>> shadermap;

Shader *LookupShader(string_view name) {
    auto shi = shadermap.find(name);
    if (shi != shadermap.end()) return shi->second;
    return nullptr;
}

void ShaderShutDown() {
    for (auto &it : shadermap)
        delete it.second;
}

string GLSLError(int obj, bool isprogram, const char *source) {
    GLint length = 0;
    if (isprogram) GL_CALL(glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length));
    else           GL_CALL(glGetShaderiv (obj, GL_INFO_LOG_LENGTH, &length));
    if (length > 1) {
        GLchar *log = new GLchar[length];
        if (isprogram) GL_CALL(glGetProgramInfoLog(obj, length, &length, log));
        else           GL_CALL(glGetShaderInfoLog (obj, length, &length, log));
        string err = "GLSL ERROR: ";
        err += log;
        int i = 0;
        if (source) for (;;) {
            err += cat(++i, ": ");
            const char *next = strchr(source, '\n');
            if (next) { err += string_view(source, next - source + 1); source = next + 1; }
            else { err += string_view(source) + "\n"; break; }
        }
        delete[] log;
        return err;
    }
    return "";
}

int CompileGLSLShader(GLenum type, int program, const GLchar *source, string &err)  {
    int obj = glCreateShader(type);
    GL_CALL(glShaderSource(obj, 1, &source, nullptr));
    GL_CALL(glCompileShader(obj));
    GLint success;
    GL_CALL(glGetShaderiv(obj, GL_COMPILE_STATUS, &success));
    if (success) {
        GL_CALL(glAttachShader(program, obj));
        return obj;
    }
    err = GLSLError(obj, false, source);
    GL_CALL(glDeleteShader(obj));
    return 0;
}

string ParseMaterialFile(string_view mbuf) {
    auto p = mbuf;
    string err;
    string_view last;
    string defines;
    string vfunctions, pfunctions, cfunctions, vertex, pixel, compute, vdecl, pdecl, csdecl, shader;
    string *accum = nullptr;
    auto word = [&]() {
        p.remove_prefix(min(p.find_first_not_of(" \t\r"), p.size()));
        size_t len = min(p.find_first_of(" \t\r"), p.size());
        last = p.substr(0, len);
        p.remove_prefix(len);
    };
    auto finish = [&]() -> bool {
        if (!shader.empty()) {
            auto sh = new Shader();
            if (compute.length()) {
                #ifdef PLATFORM_WINNIX
                    extern string glslversion;
                    auto header = "#version " + glslversion + "\n" + defines;
                #else
                    auto header = "#version 430\n" + defines;
                #endif
                err = sh->Compile(shader.c_str(), (header + csdecl + cfunctions +
                                                   "void main()\n{\n" + compute + "}\n").c_str());
            } else {
                string header;
                #ifdef PLATFORM_ES3
                    #ifdef __EMSCRIPTEN__
                        header += "#version 300 es\n";
                    #endif
                    header += "#ifdef GL_ES\nprecision highp float;\n#endif\n";
                #else
                    #ifdef __APPLE__
                    auto supported = glGetString(GL_SHADING_LANGUAGE_VERSION);
                    // Apple randomly changes what it supports, so just ask for that.
                    header += string_view("#version ") + string_view((const char *)supported, 1) +
                              string_view((const char *)supported + 2, 2) + "\n";
                    #else
                    extern string glslversion;
                    header += "#version " + glslversion + "\n";
                    header += "#extension GL_EXT_gpu_shader4 : enable\n";
                    #endif
                #endif
                header += defines;
                pdecl = "out vec4 frag_color;\n" + pdecl;
                err = sh->Compile(shader.c_str(),
                                  (header + vdecl + vfunctions + "void main()\n{\n" + vertex +
                                   "}\n").c_str(),
                                  (header + pdecl + pfunctions + "void main()\n{\n" + pixel +
                                   "}\n").c_str());
            }
            if (!err.empty())
                return true;
            shadermap[shader] = sh;
            shader.clear();
        }
        return false;
    };
    for (;;) {
        auto start = p;
        p.remove_suffix(p.size() - min(p.find_first_of("\n"), p.size()));
        auto line = p;
        word();
        if (!last.empty()) {
            if (last == "VERTEXFUNCTIONS")  {
                if (finish()) return err;
                vfunctions.clear();
                accum = &vfunctions;
            } else if (last == "PIXELFUNCTIONS") {
                if (finish()) return err;
                pfunctions.clear();
                accum = &pfunctions;
            } else if (last == "COMPUTEFUNCTIONS") {
                if (finish()) return err;
                cfunctions.clear();
                accum = &cfunctions;
            } else if (last == "VERTEX") {
                vertex.clear();
                accum = &vertex;
            } else if (last == "PIXEL") {
                pixel.clear();
                accum = &pixel;
            } else if (last == "COMPUTE") {
                compute.clear();
                accum = &compute;
            } else if (last == "SHADER") {
                if (finish()) return err;
                word();
                shader = last;
                vdecl.clear();
                pdecl.clear();
                csdecl.clear();
                vertex.clear();
                pixel.clear();
                compute.clear();
                accum = nullptr;
            } else if (last == "UNIFORMS") {
                string &decl = accum == &compute ? csdecl : (accum == &vertex ? vdecl : pdecl);
                for (;;) {
                    word();
                    if (last.empty()) break;
                    else if (last == "mvp")              decl += "uniform mat4 mvp;\n";
                    else if (last == "col")              decl += "uniform vec4 col;\n";
                    else if (last == "camera")           decl += "uniform vec3 camera;\n";
                    else if (last == "light1")           decl += "uniform vec3 light1;\n";
                    else if (last == "lightparams1")     decl += "uniform vec2 lightparams1;\n";
                    else if (last == "framebuffer_size") decl += "uniform vec2 framebuffer_size;\n";
                    // FIXME: Make configurable.
                    else if (last == "bones")            decl += "uniform vec4 bones[230];\n";
                    else if (last == "pointscale")       decl += "uniform float pointscale;\n";
                    else if (last.substr(0, 3) == "tex") {
                        auto tp = last;
                        tp.remove_prefix(3);
                        bool cubemap = false;
                        bool floatingp = false;
                        bool d3 = false;
                        if (starts_with(tp, "cube")) {
                            tp.remove_prefix(4);
                            cubemap = true;
                        }
                        if (starts_with(tp, "3d")) {
                            tp.remove_prefix(2);
                            d3 = true;
                        }
                        if (starts_with(tp, "f")) {
                            tp.remove_prefix(1);
                            floatingp = true;
                        }
                        auto unit = parse_int<int>(tp);
                        if (accum == &compute) {
                            decl += cat("layout(binding = ", unit, ", ",
                                        (floatingp ? "rgba32f" : "rgba8"), ") ");
                        }
                        decl += "uniform ";
                        decl += accum == &compute ? (cubemap ? "imageCube" : "image2D")
                                                  : (cubemap ? "samplerCube" : (d3 ? "sampler3D" :
                                                                                     "sampler2D"));
                        decl += " " + last + ";\n";
                    } else return "unknown uniform: " + last;
                }
            } else if (last == "UNIFORM") {
                string &decl = accum == &compute ? csdecl : (accum == &vertex ? vdecl : pdecl);
                word();
                auto type = last;
                word();
                auto name = last;
                if (type.empty() || name.empty()) return "uniform decl must specify type and name";
                decl += "uniform " + type + " " + name + ";\n";
            } else if (last == "INPUTS") {
                string decl;
                for (;;) {
                    word();
                    if (last.empty()) break;
                    auto pos = last.find_first_of(":");
                    if (pos == string_view::npos) {
                        return "input " + last +
                               " doesn't specify number of components, e.g. anormal:3";
                    }
                    int comp = parse_int<int>(last.substr(pos + 1));
                    if (comp <= 0 || comp > 4) {
                        return "input " + last + " can only use 1..4 components";
                    }
                    last = last.substr(0, pos);
                    string d = cat(" vec", comp, " ", last, ";\n");
                    if (accum == &vertex) vdecl += "in" + d;
                    else { vdecl += "out" + d; pdecl += "in" + d; }
                }
            } else if (last == "LAYOUT") {
                word();
                auto xs = last;
                word();
                auto ys = last;
                csdecl += "layout(local_size_x = " + xs + ", local_size_y = " + ys + ") in;\n";
            } else if (last == "DEFINE") {
                word();
                auto def = last;
                word();
                auto val = last;
                defines += "#define " + (val.empty() ? def : def + " " + val) + "\n";
            } else {
                if (!accum)
                    return "GLSL code outside of FUNCTIONS/VERTEX/PIXEL block: " + line;
                *accum += line;
                *accum += "\n";
            }
        }
        if (line.size() == start.size()) break;
        start.remove_prefix(line.size() + 1);
        p = start;
    }
    finish();
    return err;
}

string LoadMaterialFile(string_view mfile) {
    string mbuf;
    if (LoadFile(mfile, &mbuf) < 0) return string_view("cannot load material file: ") + mfile;
    auto err = ParseMaterialFile(mbuf);
    return err;
}

string Shader::Compile(const char *name, const char *vscode, const char *pscode) {
    program = glCreateProgram();
    string err;
    vs = CompileGLSLShader(GL_VERTEX_SHADER,   program, vscode, err);
    if (!vs) return string_view("couldn't compile vertex shader: ") + name + "\n" + err;
    ps = CompileGLSLShader(GL_FRAGMENT_SHADER, program, pscode, err);
    if (!ps) return string_view("couldn't compile pixel shader: ") + name + "\n" + err;
    GL_CALL(glBindAttribLocation(program, 0, "apos"));
    GL_CALL(glBindAttribLocation(program, 1, "anormal"));
    GL_CALL(glBindAttribLocation(program, 2, "atc"));
    GL_CALL(glBindAttribLocation(program, 3, "acolor"));
    GL_CALL(glBindAttribLocation(program, 4, "aweights"));
    GL_CALL(glBindAttribLocation(program, 5, "aindices"));
    Link(name);
    return "";
}

string Shader::Compile(const char *name, const char *cscode) {
    #ifdef PLATFORM_WINNIX
        program = glCreateProgram();
        string err;
        cs = CompileGLSLShader(GL_COMPUTE_SHADER, program, cscode, err);
        if (!cs) return string_view("couldn't compile compute shader: ") + name + "\n" + err;
        Link(name);
        return "";
    #else
        return "compute shaders not supported";
    #endif
}

void Shader::Link(const char *name) {
    GL_CALL(glLinkProgram(program));
    GLint status;
    GL_CALL(glGetProgramiv(program, GL_LINK_STATUS, &status));
    if (status != GL_TRUE) {
        GLSLError(program, true, nullptr);
        THROW_OR_ABORT(string_view("linking failed for shader: ") + name);
    }
    mvp_i              = glGetUniformLocation(program, "mvp");
    col_i              = glGetUniformLocation(program, "col");
    camera_i           = glGetUniformLocation(program, "camera");
    light1_i           = glGetUniformLocation(program, "light1");
    lightparams1_i     = glGetUniformLocation(program, "lightparams1");
    framebuffer_size_i = glGetUniformLocation(program, "framebuffer_size");
    bones_i            = glGetUniformLocation(program, "bones");
    pointscale_i       = glGetUniformLocation(program, "pointscale");
    Activate();
    for (int i = 0; i < MAX_SAMPLERS; i++) {
        auto loc = glGetUniformLocation(program, cat("tex", i).c_str());
        if (loc < 0) {
            loc = glGetUniformLocation(program, cat("texcube", i).c_str());
            if (loc < 0) loc = glGetUniformLocation(program, cat("tex3d", i).c_str());
        }
        if (loc >= 0) {
            glUniform1i(loc, i);
            max_tex_defined = i + 1;
        }
    }
}

Shader::~Shader() {
    if (program) GL_CALL(glDeleteProgram(program));
    if (ps) GL_CALL(glDeleteShader(ps));
    if (vs) GL_CALL(glDeleteShader(vs));
    if (cs) GL_CALL(glDeleteShader(cs));
}

// FIXME: unlikely to cause ABA problem, but still better to reset once per frame just in case.
static int last_program = 0;

void Shader::Activate() {
    if (program != last_program) {
        GL_CALL(glUseProgram(program));
        last_program = program;
    }
}

void Shader::Set() {
    Activate();
    if (mvp_i >= 0) GL_CALL(glUniformMatrix4fv(mvp_i, 1, false,
                                               (view2clip * otransforms.object2view).data()));
    if (col_i >= 0) GL_CALL(glUniform4fv(col_i, 1, curcolor.begin()));
    if (camera_i >= 0) GL_CALL(glUniform3fv(camera_i, 1, otransforms.view2object[3].begin()));
    if (pointscale_i >= 0) GL_CALL(glUniform1f(pointscale_i, pointscale));
    if (lights.size() > 0) {
        if (light1_i >= 0)
            GL_CALL(glUniform3fv(light1_i, 1, (otransforms.view2object * lights[0].pos).begin()));
        if (lightparams1_i >= 0)
            GL_CALL(glUniform2fv(lightparams1_i, 1, lights[0].params.begin()));
    }
    if (framebuffer_size_i >= 0)
        GL_CALL(glUniform2fv(framebuffer_size_i, 1,
                             float2(GetFrameBufferSize(GetScreenSize())).begin()));
}

void Shader::SetAnim(float3x4 *bones, int num) {
    // FIXME: Check if num fits with shader def.
    if (bones_i >= 0) GL_CALL(glUniform4fv(bones_i, num * 3, (float *)bones));
}

void Shader::SetTextures(const vector<Texture> &textures) {
    for (int i = 0; i < min(max_tex_defined, (int)textures.size()); i++) {
        SetTexture(i, textures[i]);
    }
}

bool Shader::SetUniform(string_view name, const float *val, int components, int elements) {
    auto loc = glGetUniformLocation(program, null_terminated(name));
    if (loc < 0) return false;
    switch (components) {
        case 1: GL_CALL(glUniform1fv(loc, elements, val)); return true;
        case 2: GL_CALL(glUniform2fv(loc, elements, val)); return true;
        case 3: GL_CALL(glUniform3fv(loc, elements, val)); return true;
        case 4: GL_CALL(glUniform4fv(loc, elements, val)); return true;
        default: return false;
    }
}

bool Shader::SetUniformMatrix(string_view name, const float *val, int components, int elements) {
    auto loc = glGetUniformLocation(program, null_terminated(name));
    if (loc < 0) return false;
    switch (components) {
        case 4:  GL_CALL(glUniformMatrix2fv(loc, elements, false, val)); return true;
        case 9:  GL_CALL(glUniformMatrix3fv(loc, elements, false, val)); return true;
        case 12: GL_CALL(glUniformMatrix3x4fv(loc, elements, false, val)); return true;
        case 16: GL_CALL(glUniformMatrix4fv(loc, elements, false, val)); return true;
        default: return false;
    }
}

void DispatchCompute(const int3 &groups) {
    #ifdef PLATFORM_WINNIX
        if (glDispatchCompute) GL_CALL(glDispatchCompute(groups.x, groups.y, groups.z));
        // Make sure any imageStore/VBOasSSBO operations have completed.
        // Would be better to decouple this from DispatchCompute.
        if (glMemoryBarrier)
            GL_CALL(glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT |
                                    GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT));
    #else
        assert(false);
    #endif
}

// Simple function for getting some uniform / shader storage attached to a shader. Should ideally
// be split up for more flexibility.
// Use this for reusing BO's for now:
struct BOEntry { int bo; int bpi; size_t size; };
map<string, BOEntry, less<>> ubomap;
// Note that bo_binding_point_index is assigned automatically based on unique block names.
// You can also specify these in the shader using `binding=`, but GL doesn't seem to have a way
// to retrieve these programmatically.
// If data is nullptr, bo is used instead.
// If offset < 0 then its a buffer replacement/creation.
int UniformBufferObject(Shader *sh, const void *data, size_t len, ptrdiff_t offset,
                         string_view uniformblockname, bool ssbo, int bo) {
    #ifdef PLATFORM_WINNIX
        if (sh && glGetProgramResourceIndex && glShaderStorageBlockBinding && glBindBufferBase &&
                  glUniformBlockBinding && glGetUniformBlockIndex) {
            sh->Activate();
            auto idx = ssbo
                ? glGetProgramResourceIndex(sh->program, GL_SHADER_STORAGE_BLOCK,
                                            null_terminated(uniformblockname))
                : glGetUniformBlockIndex(sh->program, null_terminated(uniformblockname));

            GLint maxsize = 0;
            // FIXME: call glGetInteger64v if we ever want buffers >2GB.
            if (ssbo) glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxsize);
            else glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxsize);
            if (idx != GL_INVALID_INDEX && len <= size_t(maxsize)) {
                auto type = ssbo ? GL_SHADER_STORAGE_BUFFER : GL_UNIFORM_BUFFER;
                static int binding_point_index_alloc = 0;
                auto it = ubomap.find(uniformblockname);
                int bo_binding_point_index = 0;
                if (it == ubomap.end()) {
                    assert(offset < 0);
                    if (data) bo = GenBO_(type, len, data);
                    bo_binding_point_index = binding_point_index_alloc++;
                    ubomap[string(uniformblockname)] = { bo, bo_binding_point_index, len };
				} else {
                    if (data) bo = it->second.bo;
                    bo_binding_point_index = it->second.bpi;
                    GL_CALL(glBindBuffer(type, bo));
                    if (data) {
                        // We're going to re-upload the buffer.
                        // See this for what is fast:
                        // https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
                        if (offset < 0) {
                            // Whole buffer.
                            if (false && len == it->second.size) {
                                // Is this faster than glBufferData if same size?
                                // Actually, this might cause *more* sync issues than glBufferData.
                                GL_CALL(glBufferSubData(type, 0, len, data));
                            } else {
                                // We can "orphan" the buffer before uploading, that way if a draw
                                // call is still using it, we won't have to sync.
                                // TODO: this doesn't actually seem faster in testing sofar.
                                //glBufferData(type, it->second.size, nullptr, GL_STATIC_DRAW);
                                GL_CALL(glBufferData(type, len, data, GL_STATIC_DRAW));
                                it->second.size = len;
                            }
                        } else {
                            // Partial buffer.
                            GL_CALL(glBufferSubData(type, offset, len, data));
                        }
                    }
                }
                GL_CALL(glBindBuffer(type, 0));  // Support for unbinding this way removed in GL 3.1?
                GL_CALL(glBindBufferBase(type, bo_binding_point_index, bo));
                if (ssbo) GL_CALL(glShaderStorageBlockBinding(sh->program, idx,
                                                              bo_binding_point_index));
                else GL_CALL(glUniformBlockBinding(sh->program, idx, bo_binding_point_index));
            }
        }
    #else
        // UBO's are in ES 3.0, not sure why OS X doesn't have them
    #endif
    return bo;
}

bool Shader::Dump(string_view filename, bool stripnonascii) {
  #ifdef PLATFORM_WINNIX
    if (!glGetProgramBinary) return false;
  #endif
  #ifndef __EMSCRIPTEN__
    int len = 0;
    GL_CALL(glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &len));
    string buf;
    buf.resize(len);
    GLenum format = 0;
    GL_CALL(glGetProgramBinary(program, len, nullptr, &format, buf.data()));
    if (stripnonascii) {
      buf.erase(remove_if(buf.begin(), buf.end(), [](char c) {
        return (c < ' ' || c > '~') && c != '\n' && c != '\t';
      }), buf.end());
    }
    return WriteFile(filename, true, buf);
  #else
    return false;
  #endif
}
