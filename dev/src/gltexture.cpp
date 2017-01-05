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
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable: 4244)
#endif
#include "stb/stb_image.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

const int nummultisamples = 4;

uint CreateTexture(const uchar *buf, const int *dim, int tf) {
    uint id;
    glGenTextures(1, &id);
    assert(id);
    GLenum textype =
        #if !defined(PLATFORM_ES2) && !defined(__APPLE__)
        tf & TF_MULTISAMPLE ? GL_TEXTURE_2D_MULTISAMPLE :
        #endif
        (tf & TF_3D ? GL_TEXTURE_3D : GL_TEXTURE_2D);
    GLenum teximagetype = textype;
    textype         = tf & TF_CUBEMAP ? GL_TEXTURE_CUBE_MAP            : textype;
    teximagetype    = tf & TF_CUBEMAP ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : teximagetype;
    int texnumfaces = tf & TF_CUBEMAP ? 6                              : 1;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(textype, id);
    auto wrap = tf & TF_CLAMP ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    glTexParameteri(textype, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(textype, GL_TEXTURE_WRAP_T, wrap);
    if(tf & TF_3D) glTexParameteri(textype, GL_TEXTURE_WRAP_R, wrap);
    glTexParameteri(textype, GL_TEXTURE_MAG_FILTER, tf & TF_NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(textype, GL_TEXTURE_MIN_FILTER,
        tf & TF_NOMIPMAP ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
    //if (mipmap) glTexParameteri(textype, GL_GENERATE_MIPMAP, GL_TRUE);
    auto internalformat = tf & TF_SINGLE_CHANNEL ? GL_LUMINANCE8 : GL_RGBA8;
    auto bufferformat = tf & TF_SINGLE_CHANNEL ? GL_LUMINANCE : GL_RGBA;
    auto buffercomponent = GL_UNSIGNED_BYTE;
    if ((tf & TF_SINGLE_CHANNEL) && (dim[0] & 0x3))
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // Defaults to 4.
    if (tf & TF_FLOAT) {
        #if !defined(PLATFORM_ES2) && !defined(__APPLE__)
            internalformat = tf & TF_SINGLE_CHANNEL ? GL_R32F : GL_RGBA32F;
            bufferformat = internalformat;
            buffercomponent = GL_FLOAT;
        #else
            assert(false);  // buf points to float data, which we don't support.
        #endif
    }
    if (tf & TF_MULTISAMPLE) {
        #if !defined(PLATFORM_ES2) && !defined(__APPLE__)
            glTexImage2DMultisample(teximagetype, nummultisamples, internalformat, dim[0], dim[1],
                                    true);
        #else
            assert(false);
        #endif
    } else if(tf & TF_3D) {
        GL_CALL(glTexImage3D(textype, 0, internalformat, dim[0], dim[1], dim[2], 0, bufferformat,
                             buffercomponent, buf));
    } else {
        for (int i = 0; i < texnumfaces; i++)
            GL_CALL(glTexImage2D(teximagetype + i, 0, internalformat, dim[0], dim[1], 0,
                                 bufferformat, buffercomponent, buf));
    }
    if (!(tf & TF_NOMIPMAP)) {
        glEnable(textype);  // required on ATI for mipmapping to work?
        #if !defined(PLATFORM_ES2) && !defined(__APPLE__)
        if (glGenerateMipmap)     // only exists in 3.0 contexts and up.
        #endif
            glGenerateMipmap(textype);
    }
    glBindTexture(textype, 0);
    return id;
}

uint CreateTextureFromFile(const char *name, int2 &dim, int tf) {
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
    uint id = CreateTexture(buf, dim.data(), tf);
    stbi_image_free(buf);
    return id;
}

uint CreateBlankTexture(const int2 &size, const float4 &color, int tf) {
    if (tf & TF_MULTISAMPLE) {
        return CreateTexture(nullptr, size.data(), tf);  // No buffer required.
    } else {
        auto sz = tf & TF_FLOAT ? sizeof(float4) : sizeof(byte4);
        auto buf = new uchar[size.x() * size.y() * sz];
        for (int y = 0; y < size.y(); y++) for (int x = 0; x < size.x(); x++) {
            auto idx = y * size.x() + x;
            if (tf & TF_FLOAT) ((float4 *)buf)[idx] = color;
            else               ((byte4  *)buf)[idx] = quantizec(color);
        }
        uint id = CreateTexture(buf, size.data(), tf);
        delete[] buf;
        return id;
    }
}

void DeleteTexture(uint &id) {
    if (id) glDeleteTextures(1, &id);
    id = 0;
}

void SetTexture(uint textureunit, uint id, int tf) {
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(tf & TF_CUBEMAP ? GL_TEXTURE_CUBE_MAP
                                  : (tf & TF_3D ? GL_TEXTURE_3D : GL_TEXTURE_2D), id);
}

int2 TextureSize(uint id) {
    glBindTexture(GL_TEXTURE_2D, id);
    int2 size(0);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &size.x_mut());
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &size.y_mut());
    return size;
}

uchar *ReadTexture(uint id, const int2 &size) {
    glBindTexture(GL_TEXTURE_2D, id);
    auto pixels = new uchar[size.x() * size.y() * 4];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    return pixels;
}

void SetImageTexture(uint textureunit, uint id, int tf) {
    #if !defined(PLATFORM_ES2) && !defined(__APPLE__)
        if (glBindImageTexture)
            glBindImageTexture(textureunit, id, 0, GL_TRUE, 0,
                               tf & TF_WRITEONLY
                                   ? GL_WRITE_ONLY
                                   : (tf & TF_READWRITE ? GL_READ_WRITE : GL_READ_ONLY),
                               tf & TF_FLOAT ? GL_RGBA32F : GL_RGBA8);
    #else
        assert(false);
    #endif
}

// from 2048 on older GLES2 devices and very old PCs to 16384 on the latest PC cards
int MaxTextureSize() { int mts = 0; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mts); return mts; }

uint CreateFrameBuffer(uint texture, int tf) {
    if (!glGenFramebuffers)
        return 0;
    uint fb = 0;
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        #if !defined(PLATFORM_ES2) && !defined(__APPLE__)
        tf & TF_MULTISAMPLE ? GL_TEXTURE_2D_MULTISAMPLE :
        #endif
        GL_TEXTURE_2D, texture, 0);
    return fb;
}

static uint fb = 0;
static uint rb = 0;
static uint retex = 0;  // Texture to resolve to at the end when fb refers to a multisample texture.
static int retf = 0;
static int2 resize = int2_0;
bool SwitchToFrameBuffer(uint texture, const int2 &fbsize, bool depth, int tf, uint resolvetex) {
    if (!glGenRenderbuffers)
        return false;
    if (rb) {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers(1, &rb);
        rb = 0;
    }
    if (fb) {
        if (retex) {
            uint refb = CreateFrameBuffer(retex, retf);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, refb);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
            glBlitFramebuffer(0, 0, resize.x(), resize.y(),
                              0, 0, resize.x(), resize.y(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glDeleteFramebuffers(1, &refb);
            retex = 0;
            retf = 0;
            resize = int2_0;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fb);
        fb = 0;
    }
    if (!texture) {
        OpenGLFrameStart(fbsize);
        Set2DMode(fbsize, true);
        return true;
    }
    fb = CreateFrameBuffer(texture, tf);
    if (depth) {
        glGenRenderbuffers(1, &rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        if (tf & TF_MULTISAMPLE) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, nummultisamples, GL_DEPTH_COMPONENT24,
                                             fbsize.x(), fbsize.y());
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbsize.x(), fbsize.y());
        }
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
    }
    retex = resolvetex;
    retf = tf & ~TF_MULTISAMPLE;
    resize = fbsize;
    OpenGLFrameStart(fbsize);
    Set2DMode(fbsize, false);  // Have to use rh mode here, otherwise texture is filled flipped.
    auto ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    assert(ok);
    return ok;
}

