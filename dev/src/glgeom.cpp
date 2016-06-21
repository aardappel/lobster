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

uint GenBO(GLenum type, int elemsize, int count, const void *verts)
{
    uint bo;
    glGenBuffers(1, &bo);
    glBindBuffer(type, bo);
    glBufferData(type, elemsize * count, verts, GL_STATIC_DRAW);
    return bo;
}

void DeleteBO(uint id)
{
    glDeleteBuffers(1, &id);
}

size_t AttribsSize(const char *fmt)
{
    size_t size = 0;
    while (*fmt)
    {
        switch (*fmt++)
        {
            case 'P': case 'N':           size += 12; break;
            case 'p': case 'n': case 'T': size +=  8; break;
            case 'C': case 'W': case 'I': size +=  4; break;
            default: assert(0);
        }
    }
    return size;
}

void SetAttribs(uint vbo1, const char *fmt, int vertsize1, uint vbo2 = 0, int vertsize2 = 0)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo1);

    size_t offset = 0;
    int vertsize = vertsize1;

    while (*fmt)
    {
        switch (*fmt++)
        {
            case 'P': glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT,         false, vertsize, (void *)offset); offset += 12; break;
            case 'p': glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT,         false, vertsize, (void *)offset); offset +=  8; break;
            case 'N': glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT,         false, vertsize, (void *)offset); offset += 12; break;
            case 'n': glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT,         false, vertsize, (void *)offset); offset +=  8; break;
            case 'T': glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT,         false, vertsize, (void *)offset); offset +=  8; break;
            case 'C': glEnableVertexAttribArray(3); glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, true,  vertsize, (void *)offset); offset +=  4; break;
            case 'W': glEnableVertexAttribArray(4); glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, true,  vertsize, (void *)offset); offset +=  4; break;
            case 'I': glEnableVertexAttribArray(5); glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, false, vertsize, (void *)offset); offset +=  4; break;
            default: assert(0);
        }
        if (vbo2)
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo2);
            vertsize = vertsize2;
            offset = 0;
        }
    }
}

void UnSetAttribs(const char *fmt)
{
    while (*fmt) switch (*fmt++)
    {
        case 'P': case 'p': glDisableVertexAttribArray(0); break;
        case 'N': case 'n': glDisableVertexAttribArray(1); break;
        case 'T':           glDisableVertexAttribArray(2); break;
        case 'C':           glDisableVertexAttribArray(3); break;
        case 'W':           glDisableVertexAttribArray(4); break;
        case 'I':           glDisableVertexAttribArray(5); break;
        default: assert(0); 
    }
}

GLenum GetPrimitive(Primitive prim)
{
    switch (prim)
    {
        default: assert(0);

        case PRIM_TRIS:  return GL_TRIANGLES;
        case PRIM_FAN:   return GL_TRIANGLE_FAN;
        case PRIM_LOOP:  return GL_LINE_LOOP;
        case PRIM_POINT: return GL_POINTS;
    }
}

Textured::Textured()
{
    memset(textures, 0, sizeof(uint) * Shader::MAX_SAMPLERS);
}

Surface::Surface(const int *indices, size_t _nidx, Primitive _prim) : numidx(_nidx), prim(_prim)
{
    vboId = GenBO(GL_ELEMENT_ARRAY_BUFFER, sizeof(int), numidx, indices);
}

void Surface::Render(Shader *sh)
{
    sh->SetTextures(textures);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId);
    glDrawElements(GetPrimitive(prim), (GLsizei)numidx, GL_UNSIGNED_INT, 0);
}

Surface::~Surface()
{
    glDeleteBuffers(1, &vboId);
}

Geometry::Geometry(const void *verts, size_t _nverts, size_t _vertsize, const char *_fmt)
    : vertsize(_vertsize), fmt(_fmt), vbo(0), nverts(_nverts)
{
    vbo = GenBO(GL_ARRAY_BUFFER, vertsize, nverts, verts);
}

void Geometry::RenderSetup()
{
    /*
    if (glMapBufferRange)
    {
        auto mem = (float *)glMapBufferRange(GL_ARRAY_BUFFER, 0, vertsize * nverts, GL_MAP_READ_BIT);
        printf("%x\n", mem);
    }
    */

    SetAttribs(vbo, fmt.c_str(), (int)vertsize);
}

void Geometry::RenderDone()
{
    UnSetAttribs(fmt.c_str());
}

Geometry::~Geometry()
{
    glDeleteBuffers(1, &vbo);
}

void Geometry::BindAsSSBO(uint bind_point_index) { BindVBOAsSSBO(bind_point_index, vbo); }

void Mesh::Render(Shader *sh)
{
    if (prim == PRIM_POINT) SetPointSprite(pointsize);

    sh->Set();

    if (numbones && numframes)
    {
        int frame1 = (int)floor(curanim);
        int frame2 = frame1 + 1;
        float frameoffset = curanim - frame1;

        float3x4 *mat1 = &mats[(frame1 % numframes) * numbones],
                 *mat2 = &mats[(frame2 % numframes) * numbones];

        auto outframe = new float3x4[numbones];

        for(int i = 0; i < numbones; i++) outframe[i] = mix(mat1[i], mat2[i], frameoffset);

        sh->SetAnim(outframe, numbones);

        delete[] outframe;
    }

    geom->RenderSetup();

    if (surfs.size())
    {
        for (auto s : surfs) s->Render(sh);
    }
    else
    {
        glDrawArrays(GetPrimitive(prim), 0, geom->nverts);
    }
    geom->RenderDone();
}

Mesh::~Mesh()
{
    delete geom;
    for (auto s : surfs) delete s;
    if (mats) delete[] mats;
}

void SetPointSprite(float scale)
{
    pointscale = scale * custompointscale;
    #ifdef PLATFORM_ES2
        // glEnable(GL_POINT_SPRITE_OES);
        // glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
    #else
        glEnable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    #endif
}

void RenderArray(Primitive prim, int tcount, int vcount, const char *fmt,
                 int vertsize1, uint vbo, uint ibo = 0, int vertsize2 = 0, uint vbo2 = 0)
{
    GLenum glprim = GetPrimitive(prim);
    SetAttribs(vbo, fmt, vertsize1, vbo2, vertsize2);
    if (ibo)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(glprim, tcount, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(glprim, 0, vcount);
    }
    UnSetAttribs(fmt);

}

void RenderArraySlow(Primitive prim, int tcount, int vcount, const char *fmt,
                     int vertsize1, void *vbuf1, int *ibuf, int vertsize2, void *vbuf2)
{
    uint vbo = GenBO(GL_ARRAY_BUFFER, vertsize1, vcount, vbuf1);
    uint vbo2 = vbuf2 ? GenBO(GL_ARRAY_BUFFER, vertsize2, vcount, vbuf2) : 0;
    uint ibo = ibuf ? GenBO(GL_ELEMENT_ARRAY_BUFFER, sizeof(int), tcount, ibuf) : 0;

    RenderArray(prim, tcount, vcount, fmt, vertsize1, vbo, ibo, vertsize2, vbo2);

    if (ibuf) glDeleteBuffers(1, &ibo);
    if (vbuf2) glDeleteBuffers(1, &vbo2);
    glDeleteBuffers(1, &vbo);
}

uint quadvbo[2] = { 0, 0 };

void RenderUnitSquare(Shader *sh, Primitive prim, bool centered)
{
    int quadsize = sizeof(float) * 5;
    if (!quadvbo[centered])
    {
        static float vb_square[20] =
        {
            0, 0, 0, 0, 0,
            0, 1, 0, 0, 1,
            1, 1, 0, 1, 1,
            1, 0, 0, 1, 0,
        };
        static float vb_square_centered[20] =
        {
            -1, -1, 0, 0, 0,
            -1,  1, 0, 0, 1,
             1,  1, 0, 1, 1,
             1, -1, 0, 1, 0,
        };
        quadvbo[centered] = GenBO(GL_ARRAY_BUFFER, quadsize, 4, centered ? vb_square_centered : vb_square);
    }
    sh->Set();
    RenderArray(prim, 4, 4, "PT", quadsize, quadvbo[centered]);
}

void RenderQuad(Shader *sh, Primitive prim, bool centered, const float4x4 &trans)
{
    Transform2D(trans, [&]() { RenderUnitSquare(sh, prim, centered); });
}

void RenderLine2D(Shader *sh, Primitive prim, const float3 &v1, const float3 &v2, float thickness)
{
    auto v = (v2 - v1) / 2;
    auto len = length(v);
    auto vnorm = v / len;
    auto trans = translation(v1 + v) * 
                 rotationZ(vnorm.xy()) *
                 float4x4(float4(len, thickness / 2, 1, 1));
    RenderQuad(sh, prim, true, trans);
}

void RenderLine3D(Shader *sh, const float3 &v1, const float3 &v2, const float3 &/*campos*/, float thickness)
{
    glDisable(GL_CULL_FACE);  // An exception in 3d mode.
    // FIXME: need to rotate the line also to make it face the camera.
    //auto camvec = normalize(campos - (v1 + v2) / 2);
    auto v = v2 - v1;
    auto vq = quatfromtwovectors(normalize(v), float3_x);
    //auto sq = quatfromtwovectors(camvec, float3_z);
    auto trans = translation((v1 + v2) / 2) * 
                 float3x3to4x4(rotation(vq)) *  // FIXME: cheaper?
                 float4x4(float4(length(v) / 2, thickness, 1, 1));
    RenderQuad(sh, PRIM_FAN, true, trans);
    glEnable(GL_CULL_FACE); 
}

uint cube_vbo = 0, cube_ibo = 0;

void RenderUnitCube(Shader *sh)
{
    struct cvert { float3 pos; float3 normal; };

    if (!cube_vbo)
    {
        static int3 normals[] =
        {
            int3(1, 0, 0), int3(-1,  0,  0),
            int3(0, 1, 0), int3( 0, -1,  0),
            int3(0, 0, 1), int3( 0,  0, -1),
        };

        static const char *faces[6] = { "4576", "0231", "2673", "0154", "1375", "0462" };
        static int indices[6] = { 0, 1, 3, 1, 2, 3 };

        vector<cvert> verts;
        vector<int> triangles;

        for (int n = 0; n < 6; n++)
        {
            auto face = faces[n];
            for (int i = 0; i < 6; i++) triangles.push_back(indices[i] + (int)verts.size());
            for (int vn = 0; vn < 4; vn++)
            {
                cvert vert;
                for (int d = 0; d < 3; d++)
                {
                    vert.pos.set(d, float((face[vn] & (1 << (2 - d))) != 0));
                }
                vert.normal = float3(normals[n]);
                verts.push_back(vert);
            }
        }

        cube_vbo = GenBO(GL_ARRAY_BUFFER, sizeof(cvert), 24, verts.data());
        cube_ibo = GenBO(GL_ELEMENT_ARRAY_BUFFER, sizeof(int), 36, triangles.data());
    }

    sh->Set();
    RenderArray(PRIM_TRIS, 36, 24, "PN", sizeof(cvert), cube_vbo, cube_ibo);
}
    

map<int, uint> circlevbos;  // FIXME: not global;

void RenderCircle(Shader *sh, Primitive prim, int segments, float radius)
{
    assert(segments >= 3);

    auto &vbo = circlevbos[segments];

    if (!vbo)
    {
        auto vbuf = new float3[segments];

        float step = PI * 2 / segments;
        for (int i = 0; i < segments; i++)
        {
            // + 1 to reduce "aliasing" from exact 0 / 90 degrees points
            vbuf[i] = float3(sinf(i * step + 1),
                             cosf(i * step + 1), 0);
        }

        vbo = GenBO(GL_ARRAY_BUFFER, sizeof(float3), segments, vbuf);
        delete[] vbuf;
    }

    Transform2D(float4x4(float4(float2_1 * radius, 1)), [&]()
    {
        sh->Set();
        RenderArray(prim, segments, segments, "P", sizeof(float3), vbo);
    });
}

map<pair<int, float>, pair<uint, uint>> opencirclevbos;  // FIXME: not global;

void RenderOpenCircle(Shader *sh, int segments, float radius, float thickness)
{
    assert(segments >= 3);

    auto vbo_type = make_pair(segments, thickness);
    auto &vibo = opencirclevbos[vbo_type];

    auto nverts = segments * 2;
    auto nindices = segments * 6;

    if (!vibo.first)
    {
        auto vbuf = new float3[nverts];
        auto ibuf = new int[nindices];

        float step = PI * 2 / segments;
        float inner = 1 - thickness;
        for (int i = 0; i < segments; i++)
        {
            // + 1 to reduce "aliasing" from exact 0 / 90 degrees points
            float x = sinf(i * step + 1);
            float y = cosf(i * step + 1);
            vbuf[i * 2 + 0] = float3(x, y, 0);
            vbuf[i * 2 + 1] = float3(x * inner, y * inner, 0);
            ibuf[i * 6 + 0] = i * 2 + 0;
            ibuf[i * 6 + 1] = ((i + 1) * 2 + 0) % nverts;
            ibuf[i * 6 + 2] = i * 2 + 1;
            ibuf[i * 6 + 3] = i * 2 + 1;
            ibuf[i * 6 + 4] = ((i + 1) * 2 + 1) % nverts;
            ibuf[i * 6 + 5] = ((i + 1) * 2 + 0) % nverts;
        }

        vibo.first = GenBO(GL_ARRAY_BUFFER, sizeof(float3), nverts, vbuf);
        vibo.second = GenBO(GL_ELEMENT_ARRAY_BUFFER, sizeof(int), nindices, ibuf);
        delete[] vbuf;
        delete[] ibuf;
    }

    Transform2D(float4x4(float4(float2_1 * radius, 1)), [&]()
    {
        sh->Set();
        RenderArray(PRIM_TRIS, nindices, nverts, "P", sizeof(float3), vibo.first, vibo.second);
    });
}