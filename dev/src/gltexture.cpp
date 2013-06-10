#include "stdafx.h"

#include "glinterface.h"
#include "glincludes.h"

#include "stb_image.h"


uint CreateTexture(uchar *buf, int x, int y, bool clamp, bool mipmap)
{
    uint id;
    glGenTextures(1, &id);
    assert(id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    //if (mipmap) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);

    if (mipmap)
    {
        glEnable(GL_TEXTURE_2D);// required on ATI for mipmapping to work?
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    return id;
}

uint CreateTextureFromFile(const char *name)
{
    size_t len = 0;
    auto fbuf = LoadFile(name, &len);
    if (!fbuf)
        return 0;

    int x, y, comp;
    auto buf = stbi_load_from_memory(fbuf, len, &x, &y, &comp, 4);

    free(fbuf);

    if (!buf)
        return 0;

    uint id = CreateTexture(buf, x, y);

    stbi_image_free(buf);
    return id;
}

void DeleteTexture(uint id) { glDeleteTextures(1, &id); }

void SetTexture(uint textureunit, uint id)
{
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, id);
}

int MaxTextureSize() { int mts = 0; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mts); return mts; }   // from 2048 on older GLES2 devices and very old PCs to 16384 on the latest PC cards


