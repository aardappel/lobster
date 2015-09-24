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

uint GenBO(GLenum type, int elemsize, int count, void *verts)
{
    uint bo;
    glGenBuffers(1, &bo);
    glBindBuffer(type, bo);
    glBufferData(type, elemsize * count, verts, GL_STATIC_DRAW);
    return bo;
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

Surface::Surface(int *indices, size_t _nidx, Primitive _prim) : numidx(_nidx), prim(_prim)
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

Geometry::Geometry(void *verts, size_t _nverts, size_t _vertsize, const char *_fmt)
    : vertsize(_vertsize), fmt(_fmt), vboId(0), nverts(_nverts)
{
    vboId = GenBO(GL_ARRAY_BUFFER, vertsize, nverts, verts);
}

void Geometry::RenderSetup() { SetAttribs(vboId, fmt, (int)vertsize); }
void Geometry::RenderDone()  { UnSetAttribs(fmt); }

Geometry::~Geometry()
{
    glDeleteBuffers(1, &vboId);
}

void Mesh::Render(Shader *sh)
{
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

void SetPointSprite(float size)
{
    #ifdef PLATFORM_ES2
        // glEnable(GL_POINT_SPRITE_OES);
        // glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
        //glPointSize(size); // FIXME: set thru uniform
    #else
        glEnable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glPointSize(size);
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

uint quadvbo = 0;

void RenderQuad(Shader *sh, Primitive prim, const float4x4 &trans)
{
    int quadsize = sizeof(float) * 5;
    if (!quadvbo)
    {
        static float tempquad_rect[20] =
        {
            0, 0, 0, 0, 0,
            0, 1, 0, 0, 1,
            1, 1, 0, 1, 1,
            1, 0, 0, 1, 0,
        };
        quadvbo = GenBO(GL_ARRAY_BUFFER, quadsize, 4, tempquad_rect);
    }
    Transform2D(trans, [&]()
    {
        sh->Set();
        RenderArray(prim, 4, 4, "PT", quadsize, quadvbo);
    });
}

void RenderLine2D(Shader *sh, Primitive prim, const float2 &v1, const float2 &v2, float thickness)
{
    auto v = v2 - v1;
    auto len = length(v);
    auto vnorm = v / len;
    auto side = float2(vnorm.y(), -vnorm.x()) * thickness / 2;
    auto trans = translation(float3(v1 + side, 0)) * 
                 rotationZ(vnorm) *
                 float4x4(float4(len, thickness, 1, 1));
    RenderQuad(sh, prim, trans);
}

void RenderLine3D(Shader *sh, const float3 &v1, const float3 &v2, const float3 &campos, float thickness)
{
    glDisable(GL_CULL_FACE);  // An exception in 3d mode.
    auto side = normalize(cross(v2 - v1, campos - (v1 + v2) / 2)) * thickness;
    auto v = v2 - v1;
    auto trans = translation(v1 + side) * 
                 float3x3to4x4(rotation(quatfromtwovectors(normalize(v), float3_x))) *  // FIXME: cheaper?
                 float4x4(float4(length(v), length(side) * 2, 1, 1));
    RenderQuad(sh, PRIM_FAN, trans);
    glEnable(GL_CULL_FACE); 
}

map<int, uint> circlevbos;  // FIXME: not global;

void RenderCircle(Shader *sh, Primitive prim, int segments, float radius)
{
    assert(segments >= 3);

    auto vbo = circlevbos[segments];

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
        circlevbos[segments] = vbo;
        delete[] vbuf;
    }

    Transform2D(float4x4(float4(float2_1 * radius, 1)), [&]()
    {
        sh->Set();
        RenderArray(prim, segments, segments, "P", sizeof(float3), vbo);
    });
}