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

// simple rendering interface for OpenGL (ES) (that doesn't depend on its headers)

struct Shader
{
    enum { MAX_SAMPLERS = 3 };

    uint vs, ps, program;

    int mvp_i, tex_i[MAX_SAMPLERS], col_i, camera_i, light1_i, bones_i;

    Shader() : vs(0), ps(0), program(0) {}

    ~Shader();

    string Compile(const char *name, const char *vscode, const char *pscode);
    void Set();
    void SetAnim(float3x4 *bones, int num);   // optionally, after Set()
    void SetTextures(uint *textures); // optionally, after Set()
};

struct Textured
{
    uint textures[Shader::MAX_SAMPLERS];

	Textured();
};

struct Surface : Textured
{
    int numidx;
    uint vboId;
    string name;

    Surface(int *indices, int _nidx);
    ~Surface();

    void Render(Shader *sh);
};

struct BasicVert    // common generic format: "PNTC"
{
    float3 pos;
    float3 norm;
    float2 tc;
    byte4 col;
};

struct AnimVert : BasicVert // "PNTCWI"
{
    byte4 weights;
    byte4 indices;
};

class Geometry 
{
    const uint vertsize;
    const char *fmt;
    uint vboId;

    public:

    const uint nverts;

    Geometry(void *verts, int _nverts, int _vertsize, const char *_fmt);
    ~Geometry();

    void RenderSetup();
    void RenderDone();
};

struct Mesh
{
    Geometry *geom;
    vector<Surface *> surfs;

    int numframes, numbones;
    float3x4 *mats;
    float curanim;

    Mesh(Geometry *_g) : geom(_g), numframes(0), numbones(0), mats(nullptr), curanim(0) {}
    ~Mesh();

    void Render(Shader *sh);
};

struct Light
{
    float4 pos;
};


enum BlendMode { BLEND_NONE = 0, BLEND_ALPHA, BLEND_ADD, BLEND_ADDALPHA, BLEND_MUL };
enum Primitive { PRIM_TRIS, PRIM_FAN, PRIM_LOOP, PRIM_POINT };


extern void OpenGLInit();
extern void OpenGLFrameStart(const int2 &screensize);
extern void Set2DMode(const int2 &screensize);
extern void Set3DMode(float fovy, float ratio, float znear, float zfar);
extern void ClearFrameBuffer(const float3 &c);
extern int SetBlendMode(BlendMode mode);
extern void SetPointSprite(float size);

extern string LoadMaterialFile(const char *mfile);
extern Shader *LookupShader(const char *name);
extern void ShaderShutDown();

extern uint CreateTexture(uchar *buf, int x, int y, bool clamp = false, bool mipmap = true);
extern uint CreateTextureFromFile(const char *name);
extern void DeleteTexture(uint id);
extern void SetTexture(uint textureunit, uint id);
extern int MaxTextureSize();

extern void RenderArray(Primitive prim, int count, const char *fmt, 
                        int vertsize, void *vbuf1, int *ibuf = nullptr, int vertsize2 = 0, void *vbuf2 = nullptr);
extern void RenderLine(Primitive prim, const float3 &v1, const float3 &v2, const float3 &side);
extern void RenderLine3D(const float3 &v1, const float3 &v2, const float3 &campos, float thickness);

extern Mesh *LoadIQM(const char *filename);

extern float4x4 view2clip;
extern float4x4 object2view;
extern float4x4 view2object;

extern vector<Light> lights;

extern float4 curcolor;

