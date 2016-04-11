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

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(disable: 4457)
#include "stb/stb_image.h"


uint CreateTexture(uchar *buf, const int2 &dim, int tf)
{
    uint id;
    glGenTextures(1, &id);
    assert(id);

    GLenum textype      = tf & TF_CUBEMAP ? GL_TEXTURE_CUBE_MAP            : GL_TEXTURE_2D;
    GLenum teximagetype = tf & TF_CUBEMAP ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D;
    int texnumfaces     = tf & TF_CUBEMAP ? 6                              : 1;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(textype, id);

    glTexParameteri(textype, GL_TEXTURE_WRAP_S, tf & TF_CLAMP ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(textype, GL_TEXTURE_WRAP_T, tf & TF_CLAMP ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(textype, GL_TEXTURE_MAG_FILTER, tf & TF_NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(textype, GL_TEXTURE_MIN_FILTER, tf & TF_NOMIPMAP ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);

    //if (mipmap) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    auto format = GL_RGBA;
    auto component = GL_UNSIGNED_BYTE;
    if (tf & TF_FLOAT)
    {
        #if !defined(PLATFORM_ES2) && !defined(__APPLE__)
            format = GL_RGBA32F;
            component = GL_FLOAT;
        #else
            assert(false);  // buf points to float data, which we don't support.
        #endif
    }

    for (int i = 0; i < texnumfaces; i++)
        glTexImage2D(teximagetype + i, 0, format, dim.x(), dim.y(), 0, GL_RGBA, component, buf);

    if (tf & TF_NOMIPMAP) return id;

    glEnable(textype);  // required on ATI for mipmapping to work?
    #if !defined(PLATFORM_ES2) && !defined(__APPLE__)
    if (glGenerateMipmap)     // only exists in 3.0 contexts and up.
    #endif
        glGenerateMipmap(textype);

    return id;
}

uint CreateTextureFromFile(const char *name, int2 &dim, int tf)
{
    tf &= ~TF_FLOAT;  // Not supported yet.

    size_t len = 0;
    auto fbuf = LoadFile(name, &len);
    if (!fbuf)
        return 0;

    int x, y, comp;
    auto buf = stbi_load_from_memory(fbuf, (int)len, &x, &y, &comp, 4);
    dim = int2(x, y);

    free(fbuf);

    if (!buf)
        return 0;

    uint id = CreateTexture(buf, dim, tf);

    stbi_image_free(buf);
    return id;
}

void DeleteTexture(uint id) { glDeleteTextures(1, &id); }

void SetTexture(uint textureunit, uint id, int tf)
{
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(tf & TF_CUBEMAP ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, id);
}

void SetImageTexture(uint textureunit, uint id, int tf)
{
    #if !defined(PLATFORM_ES2) && !defined(__APPLE__)
        if (glBindImageTexture)
            glBindImageTexture(textureunit, id, 0, GL_TRUE, 0,
                               tf & TF_WRITEONLY ? GL_WRITE_ONLY : (tf & TF_READWRITE ? GL_READ_WRITE : GL_READ_ONLY),
                               tf & TF_FLOAT ? GL_RGBA32F : GL_RGBA8);
    #else
        assert(false);
    #endif
}

// from 2048 on older GLES2 devices and very old PCs to 16384 on the latest PC cards
int MaxTextureSize() { int mts = 0; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mts); return mts; }

uint fb = 0;
uint rb = 0;
bool SwitchToFrameBuffer(uint texture, const int2 &depthsize)
{
    if (rb)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers(1, &rb);
        rb = 0;
    }
    if (fb)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fb);
        fb = 0;
    }
    if (!texture) return true;
    
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    if (depthsize.x())
    {
        glGenRenderbuffers(1, &rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, depthsize.x(), depthsize.y());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
    }
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}
