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

void SetAttribs(uint vbo, const char *fmt, int vertsize1, char *buf1 = NULL, int vertsize2 = 0, char *buf2 = NULL)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    size_t offset = 0;
    char *buf = buf1;
    int vertsize = vertsize1;

    while (*fmt)
    {
        switch (*fmt++)
        {
            case 'P': glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT,         false, vertsize, buf + offset); offset += 12; break;
            case 'p': glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT,         false, vertsize, buf + offset); offset +=  8; break;
            case 'N': glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT,         false, vertsize, buf + offset); offset += 12; break;
            case 'n': glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT,         false, vertsize, buf + offset); offset +=  8; break;
            case 'T': glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT,         false, vertsize, buf + offset); offset +=  8; break;
            case 'C': glEnableVertexAttribArray(3); glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, true,  vertsize, buf + offset); offset +=  4; break;
            case 'W': glEnableVertexAttribArray(4); glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, true,  vertsize, buf + offset); offset +=  4; break;
            case 'I': glEnableVertexAttribArray(5); glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, false, vertsize, buf + offset); offset +=  4; break;
            default: assert(0);
        }
        if (buf2)
        {
            buf = buf2;
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

Textured::Textured()
{
    memset(textures, 0, sizeof(uint) * Shader::MAX_SAMPLERS);
}

Surface::Surface(int *indices, int _nidx) : numidx(_nidx)
{
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numidx * sizeof(int), indices, GL_STATIC_DRAW);
}

void Surface::Render(Shader *sh)
{
    sh->SetTextures(textures);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId);
    glDrawElements(GL_TRIANGLES, numidx, GL_UNSIGNED_INT, 0);
}

Surface::~Surface()
{
    glDeleteBuffers(1, &vboId);
}

Geometry::Geometry(void *verts, int _nverts, int _vertsize, const char *_fmt)
    : vertsize(_vertsize), nverts(_nverts), fmt(_fmt)
{
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, nverts * vertsize, verts, GL_STATIC_DRAW);
}

void Geometry::RenderSetup() { SetAttribs(vboId, fmt, vertsize); }
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
    for (auto s : surfs) s->Render(sh);
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
    #ifdef PLATFORM_MOBILE
        // glEnable(GL_POINT_SPRITE_OES);
        // glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
        //glPointSize(size); // FIXME: set thru uniform
    #else
        glEnable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glPointSize(size);
    #endif
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

void RenderArray(Primitive prim, int count, const char *fmt,
                 int vertsize1, void *vbuf1, int *ibuf, int vertsize2, void *vbuf2)
{
    GLenum glprim = GetPrimitive(prim);
    SetAttribs(0, fmt, vertsize1, (char *)vbuf1, vertsize2, (char *)vbuf2);
    if (ibuf)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDrawElements(glprim, count, GL_UNSIGNED_INT, ibuf);
    }
    else glDrawArrays(glprim, 0, count);
    UnSetAttribs(fmt);
}

void RenderLine(Primitive prim, const float3 &v1, const float3 &v2, const float3 &side)
{
    float3 templine[4] = { v1 + side, v1 - side, v2 - side, v2 + side };
    RenderArray(prim, 4, "P", sizeof(float3), templine);
}

void RenderLine3D(const float3 &v1, const float3 &v2, const float3 &campos, float thickness)
{
    auto side = normalize(cross(v2 - v1, campos - (v1 + v2) / 2)) * thickness;
    RenderLine(PRIM_FAN, v1, v2, side);
}

