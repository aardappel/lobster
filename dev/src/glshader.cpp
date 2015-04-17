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

#include "stdafx.h"
#include "glinterface.h"
#include "glincludes.h"

map<string, Shader *> shadermap;

Shader *LookupShader(const char *name)
{
    auto shi = shadermap.find(name);
    if (shi != shadermap.end()) return shi->second;
    return nullptr;
}

void ShaderShutDown()
{
    for (auto &it : shadermap)
        delete it.second;
}

string GLSLError(uint obj, bool isprogram, const char *source)
{
    GLint length = 0;
    if (isprogram) glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
    else           glGetShaderiv (obj, GL_INFO_LOG_LENGTH, &length);
    if (length > 1)
    {
        GLchar *log = new GLchar[length];
        if (isprogram) glGetProgramInfoLog(obj, length, &length, log);
        else           glGetShaderInfoLog (obj, length, &length, log);
        string err = "GLSL ERROR: ";
        err += log;
        int i = 0;
        if (source) for (;;)
        {
            err += inttoa(++i);
            err += ": ";
            const char *next = strchr(source, '\n');
            if (next) { err += string(source, next - source + 1); source = next + 1; }
            else { err += string(source) + "\n"; break; }
        } 
        delete[] log;
        return err;
    }
    return "";
}

uint CompileGLSLShader(GLenum type, uint program, const GLchar *source, string &err) 
{
    uint obj = glCreateShader(type);
    glShaderSource(obj, 1, &source, nullptr);
    glCompileShader(obj);
    GLint success;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &success);
    if (success)
    {
        glAttachShader(program, obj);
        return obj; 
    }
    err = GLSLError(obj, false, source);
    glDeleteShader(obj);
    return 0;
}

string LoadMaterialFile(const char *mfile)
{
    auto mbuf = (char *)LoadFile(mfile);
    if (!mbuf) return string("cannot load material file: ") + mfile;

    auto p = mbuf;

    string err;
    string last;
    string vfunctions, pfunctions, vertex, pixel, vdecl, pdecl, shader;
    string *accum = nullptr;

    string header;
    #ifdef PLATFORM_MOBILE
    header += "#ifdef GL_ES\nprecision highp float;\n#endif\n";
    #else
    //#ifdef __APPLE__
    header += "#version 120\n";
    //#else
    //header += "#version 130\n";
    //#endif
    #endif

    auto word = [&]()
    {
        p += strspn(p, " \t\r");
        size_t len = strcspn(p, " \t\r\0");
        last = string(p, len);
        p += len;
    };

    auto finish = [&]() -> bool
    {
        if (!shader.empty())
        {
            auto sh = new Shader();
            err = sh->Compile(shader.c_str(),
                              (header + vdecl + vfunctions + "void main()\n{\n" + vertex + "}\n").c_str(),
                              (header + pdecl + pfunctions + "void main()\n{\n" + pixel  + "}\n").c_str());
            if (!err.empty())
                return true;
            shadermap[shader] = sh;
            shader.clear();
        }
        return false;
    };

    for (;;)
    {
        auto start = p;
        auto end = p + strcspn(p, "\n\0");
        bool eof = !*end;
        *end = 0;
        
        word();

        if (!last.empty())
        {
            if      (last == "VERTEXFUNCTIONS") { if (finish()) goto out; vfunctions.clear(); accum = &vfunctions; }
            else if (last == "PIXELFUNCTIONS")  { if (finish()) goto out; pfunctions.clear(); accum = &pfunctions; }
            else if (last == "VERTEX")          {                         vertex.clear();     accum = &vertex;     }
            else if (last == "PIXEL")           {                         pixel.clear();      accum = &pixel;      }
            else if (last == "SHADER")
            {
                if (finish()) goto out;
                word();
                shader = last;
                vdecl.clear();
                pdecl.clear();
                accum = nullptr;
            }
            else if (last == "UNIFORMS")
            {
                string &decl = accum == &vertex ? vdecl : pdecl;
                for (;;)
                {
                    word();
                    if (last.empty()) break;
                    else if (last == "mvp")    decl += "uniform mat4 mvp;\n";
                    else if (last == "col")    decl += "uniform vec4 col;\n";
                    else if (last == "camera") decl += "uniform vec3 camera;\n";
                    else if (last == "light1") decl += "uniform vec3 light1;\n";
                    else if (last == "bones")  decl += "uniform vec4 bones[240];\n";   // FIXME: configurable
                    else if (strstr(last.c_str(), "tex")) decl += "uniform sampler2D " + last + ";\n";
                    else { err = "unknown uniform: " + last; goto out; }
                }
            }
            else if (last == "UNIFORM")
            {
                string &decl = accum == &vertex ? vdecl : pdecl;
                word();
                auto type = last;
                word();
                auto name = last;
                if (type.empty() || name.empty()) { err = "uniform decl must specify type and name"; goto out; }
                decl += "uniform " + type + " " + name + ";\n";
            }
            else if (last == "INPUTS")
            {
                string decl;
                for (;;)
                {
                    word();
                    if (last.empty()) break;
                    auto pos = strstr(last.c_str(), ":");
                    if (!pos)
                    {
                        err = "input " + last + " doesn't specify number of components, e.g. anormal:3";
                        goto out;
                    }
                    int comp = atoi(pos + 1);
                    if (comp <= 0 || comp > 4)
                    {
                        err = "input " + last + " can only use 1..4 components";
                        goto out;
                    }
                    last = last.substr(0, pos - last.c_str());
                    string d = string(" vec") + inttoa(comp) + " " + last + ";\n";
                    if (accum == &vertex) vdecl += "attribute" + d;
                    else { d = "varying" + d; vdecl += d; pdecl += d; }
                }
            }
            else
            {
                if (!accum) { err = "GLSL code outside of FUNCTIONS/VERTEX/PIXEL block: " + string(start); goto out; }
                *accum += start;
                *accum += "\n";
            }
        }

        if (eof) break;

        p = end + 1;
    }

    finish();

    out:
    free(mbuf);

    return err;
}

string Shader::Compile(const char *name, const char *vscode, const char *pscode)
{
    program = glCreateProgram();

    string err;
    vs = CompileGLSLShader(GL_VERTEX_SHADER,   program, vscode, err);
    if (!vs) return string("couldn't compile vertex shader: ") + name + "\n" + err;
    ps = CompileGLSLShader(GL_FRAGMENT_SHADER, program, pscode, err);
    if (!ps) return string("couldn't compile pixel shader: ") + name + "\n" + err;

    glBindAttribLocation(program, 0, "apos");
    glBindAttribLocation(program, 1, "anormal");
    glBindAttribLocation(program, 2, "atc");
    glBindAttribLocation(program, 3, "acolor");
    glBindAttribLocation(program, 4, "aweights");
    glBindAttribLocation(program, 5, "aindices");

    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLSLError(program, true, nullptr);
        throw string("linking failed for shader: ") + name;
    }

    mvp_i    = glGetUniformLocation(program, "mvp");
    col_i    = glGetUniformLocation(program, "col");
    camera_i = glGetUniformLocation(program, "camera");
    light1_i = glGetUniformLocation(program, "light1");
    bones_i  = glGetUniformLocation(program, "bones");

    glUseProgram(program);

    for (int i = 0; i < MAX_SAMPLERS; i++)
    {
        tex_i[i] = glGetUniformLocation(program, (string("tex") + inttoa(i)).c_str());
        if (tex_i[i] >= 0) glUniform1i(tex_i[i], i); 
    }

    return "";
}

Shader::~Shader()
{
    if (program) glDeleteProgram(program);
    if (ps) glDeleteShader(ps);
    if (vs) glDeleteShader(vs);
}

void Shader::Activate()
{
    glUseProgram(program);
}

void Shader::Set()
{
    Activate();

    if (mvp_i >= 0) glUniformMatrix4fv(mvp_i, 1, false, view2clip * object2view);
    if (col_i >= 0) glUniform4fv(col_i, 1, curcolor.begin());

    if (light1_i >= 0 && lights.size() > 0) glUniform3fv(light1_i, 1, (view2object * lights[0].pos).begin());
    if (camera_i >= 0) glUniform3fv(camera_i, 1, view2object[3].begin());
}

void Shader::SetAnim(float3x4 *bones, int num)
{
    if (bones_i >= 0) glUniform4fv(bones_i, num * 3, (float *)bones);    // FIXME: check if num fits with shader def
}

void Shader::SetTextures(uint *textures)
{
    for (int i = 0; i < MAX_SAMPLERS; i++)
        if (tex_i[i] >= 0)
            SetTexture(i, textures[i]);
}

bool Shader::SetUniform(const char *name, const float *val, size_t count)
{
    auto loc = glGetUniformLocation(program, name);
    if (loc < 0) return false;
    switch (count)
    {
        case 1: glUniform1fv(loc, 1, val);
        case 2: glUniform2fv(loc, 1, val);
        case 3: glUniform3fv(loc, 1, val);
        case 4: glUniform4fv(loc, 1, val);
        default: return false;
    }
    return true;
}
