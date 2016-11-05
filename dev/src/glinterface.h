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

enum BlendMode {
    BLEND_NONE = 0,
    BLEND_ALPHA,
    BLEND_ADD,
    BLEND_ADDALPHA,
    BLEND_MUL,
    BLEND_PREMULALPHA
};

enum Primitive { PRIM_TRIS, PRIM_FAN, PRIM_LOOP, PRIM_POINT };

struct Shader {
    enum { MAX_SAMPLERS = 4 };
    uint vs, ps, cs, program;
    int mvp_i, tex_i[MAX_SAMPLERS], col_i, camera_i, light1_i, lightparams1_i, bones_i,
        pointscale_i;

    Shader() : vs(0), ps(0), cs(0), program(0) {}
    ~Shader();

    string Compile(const char *name, const char *vscode, const char *pscode);
    string Compile(const char *name, const char *comcode);
    void Link(const char *name);
    void Activate();                         // Makes shader current;
    void Set();                              // Activate + sets common uniforms.
    void SetAnim(float3x4 *bones, int num);  // Optionally, after Activate().
    void SetTextures(uint *textures);        // Optionally, after Activate().
    bool SetUniform(const char *name,        // Optionally, after Activate().
                    const float *val,
                    int components, int elements = 1);
};

struct Textured {
    uint textures[Shader::MAX_SAMPLERS];

	Textured();
};

struct Surface : Textured {
    size_t numidx;
    uint ibo;
    string name;
    Primitive prim;

    Surface(const int *indices, size_t _nidx, Primitive _prim = PRIM_TRIS);
    ~Surface();

    void Render(Shader *sh);
    void WritePLY(string &s);
};

struct BasicVert {   // common generic format: "PNTC"
    float3 pos;
    float3 norm;
    float2 tc;
    byte4 col;
};

struct AnimVert : BasicVert { // "PNTCWI"
    byte4 weights;
    byte4 indices;
};

class Geometry  {
    const size_t vertsize;
    string fmt;
    uint vbo;

    public:
    const size_t nverts;

    Geometry(const void *verts, size_t _nverts, size_t _vertsize, const char *_fmt);
    ~Geometry();

    void RenderSetup();
    void RenderDone();
    void BindAsSSBO(uint bind_point_index);
    bool WritePLY(string &s, int nindices);
};

struct Mesh {
    Geometry *geom;
    vector<Surface *> surfs;
    Primitive prim;  // If surfs is empty, this determines how to draw the verts.
    float pointsize;  // if prim == PRIM_POINT
    int numframes, numbones;
    float3x4 *mats;
    float curanim;

    Mesh(Geometry *_g, Primitive _prim = PRIM_FAN)
        : geom(_g), prim(_prim), pointsize(1.0f), numframes(0), numbones(0), mats(nullptr),
          curanim(0) {}
    ~Mesh();

    void Render(Shader *sh);
    bool SaveAsPLY(const char *filename);
};

struct Light {
    float4 pos;
    float2 params;
};


extern void OpenGLInit();
extern void OpenGLFrameStart(const int2 &ssize);
extern void Set2DMode(const int2 &ssize, bool lh);
extern void Set3DMode(float fovy, float ratio, float znear, float zfar);
extern void Set3DOrtho(const float3 &center, const float3 &extends);
extern bool Is2DMode();
extern void ClearFrameBuffer(const float3 &c);
extern int SetBlendMode(BlendMode mode);
extern void SetPointSprite(float size);

extern void AppendTransform(const float4x4 &forward, const float4x4 &backward);

extern string LoadMaterialFile(const char *mfile);
extern string ParseMaterialFile(char *mfile);
extern Shader *LookupShader(const char *name);
extern void ShaderShutDown();

extern void DispatchCompute(const int3 &groups);
extern void SetImageTexture(uint textureunit, uint id, int tf);
extern uint UniformBufferObject(Shader *sh, const float *data, size_t len,
                                const char *uniformblockname, bool ssbo);
extern void BindVBOAsSSBO(uint bind_point_index, uint vbo);

// These must correspond to the constants in color.lobster
enum TextureFlag {
    TF_NONE = 0,
    TF_CLAMP = 1, TF_NOMIPMAP = 2, TF_NEAREST = 4,
    TF_FLOAT = 8,                           // rgba32f instead of rgba8
    TF_WRITEONLY = 16, TF_READWRITE = 32,   // Default is readonly.
    TF_CUBEMAP = 64,
    TF_MULTISAMPLE = 128,
};

extern uint CreateTexture(const uchar *buf, const int2 &dim, int tf = TF_NONE);
extern uint CreateTextureFromFile(const char *name, int2 &dim, int tf = TF_NONE);
extern uint CreateBlankTexture(const int2 &size, const float4 &color, int tf = TF_NONE);
extern void DeleteTexture(uint &id);
extern void SetTexture(uint textureunit, uint id, int tf = TF_NONE);
extern int2 TextureSize(uint id);
extern uchar *ReadTexture(uint id, const int2 &size);
extern int MaxTextureSize();
extern bool SwitchToFrameBuffer(uint texture, const int2 &fbsize, bool depth, int tf,
                                uint resolvetex);

extern uchar *ReadPixels(const int2 &pos, const int2 &size);

extern void DeleteBO(uint id);
extern void RenderArraySlow(Primitive prim, int tcount, int vcount, const char *fmt, int vertsize,
                            void *vbuf1, int *ibuf = nullptr, int vertsize2 = 0,
                            void *vbuf2 = nullptr);
extern void RenderLine2D(Shader *sh, Primitive prim, const float3 &v1, const float3 &v2,
                         float thickness);
extern void RenderLine3D(Shader *sh, const float3 &v1, const float3 &v2, const float3 &campos,
                         float thickness);
extern void RenderUnitSquare(Shader *sh, Primitive prim, bool centered);
extern void RenderQuad(Shader *sh, Primitive prim, bool centered, const float4x4 &trans);
extern void RenderUnitCube(Shader *sh);
extern void RenderCircle(Shader *sh, Primitive prim, int segments, float radius);
extern void RenderOpenCircle(Shader *sh, int segments, float radius, float thickness);

extern size_t AttribsSize(const char *fmt);

extern Mesh *LoadIQM(const char *filename);

extern float4x4 view2clip;

struct objecttransforms {
    float4x4 view2object;
    float4x4 object2view;

    objecttransforms() : view2object(1), object2view(1) {}
};

extern objecttransforms otransforms;

extern vector<Light> lights;

extern float4 curcolor;

extern float pointscale, custompointscale;

// 2D, since this skips view2object needed for lighting.
template<typename F> void Transform2D(const float4x4 &mat, F body) {
    auto oldobject2view = otransforms.object2view;
    otransforms.object2view *= mat;
    body();
    otransforms.object2view = oldobject2view;
}

extern bool VRInit();
extern void VRShutDown();
