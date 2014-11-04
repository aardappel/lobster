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
#include "ftinterface.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "unicode.h"

BitmapFont::~BitmapFont()
{
    if (texid) DeleteTexture(texid);
}

BitmapFont::BitmapFont(OutlineFont *_font, int _size) : texid(0), usedcount(1), font(_font), size(_size)
{
}

bool BitmapFont::CacheChars(const char *text)
{
    usedcount++;

    font->EnsureCharsPresent(text);

    if (positions.size() == font->unicodetable.size())
        return true;

    if (texid) DeleteTexture(texid);
    positions.clear();

    if (FT_Set_Pixel_Sizes((FT_Face)font->fthandle, 0, size))
        return false;

    auto face = (FT_Face)font->fthandle;
    const size_t margin = 3;
    texh = 0;
    texw = MaxTextureSize();

    int max_descent = 0, max_ascent = 0;
    size_t space_on_line = texw - margin, lines = 1;

    for (int i : font->unicodetable)
    {
        size_t char_index = FT_Get_Char_Index(face, i);

        FT_Load_Glyph(face, char_index, FT_LOAD_DEFAULT);
        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);    //FIXME: can we avoid doing this twice?

        size_t advance = (face->glyph->metrics.horiAdvance >> 6) + margin;

        if (advance > space_on_line)
        {
            space_on_line = texw - margin;
            ++lines;
        }
        space_on_line -= advance;

        max_ascent = max(face->glyph->bitmap_top, max_ascent);
        max_descent = max(face->glyph->bitmap.rows - face->glyph->bitmap_top, max_descent);
    }

    height = max_ascent + max_descent;

    size_t needed_image_height = (max_ascent + max_descent + margin) * lines + margin;
    texh = 1;
    while (texh < needed_image_height)
        texh *= 2;

    uchar* image = new uchar[texh * texw * 4];
    memset(image, 0, texh * texw * 4);

    size_t x = margin, y = margin + max_ascent;

    for (int i : font->unicodetable)
    {
        size_t char_index = FT_Get_Char_Index(face, i);

        FT_Load_Glyph(face, char_index, FT_LOAD_DEFAULT);
        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

        size_t advance = (face->glyph->metrics.horiAdvance >> 6) + margin;
        if (advance > texw - x)
        {
            x = margin;
            y += (max_ascent + max_descent + margin);
        }

        positions.push_back(int3(x, y - max_ascent, advance - margin));

        for (int row = 0; row < face->glyph->bitmap.rows; ++row)
        {
            for (int pixel = 0; pixel < face->glyph->bitmap.width; ++pixel)
            {
                auto p = image + ((x + face->glyph->bitmap_left + pixel) +
                                    (y - face->glyph->bitmap_top + row) * texw) * 4;
                *p++ = 0xFF;    // FIXME: wastefull
                *p++ = 0xFF;
                *p++ = 0xFF;
                *p++ = face->glyph->bitmap.buffer[pixel + row * face->glyph->bitmap.pitch];
            }
        }

        x += advance;
    }

    texid = CreateTexture(image, texw, texh, true, false);

    delete[] image;

    return true;
}

void BitmapFont::RenderText(const char *text)
{
    if (!CacheChars(text))
        return;

    struct PT { float3 p; float2 t; };

    int len = StrLenUTF8(text);
    if (len <= 0)
        return;

    auto vbuf = new PT[len * 4];
    auto ibuf = new int[len * 6];

    auto x = 0.0f;
    auto y = 0.0f;
    float fontheighttex = height / float(texh);

    auto idx = ibuf;

    for (int i = 0; i < len; i++)
    {
        int c = FromUTF8(text);
        int3 &pos = positions[font->unicodemap[c]];
        float x1 = pos.x() / float(texw);
        float x2 = (pos.x() + pos.z()) / float(texw);
        float y1 = pos.y() / float(texh);
        float advance = float(pos.z());
        int j = i * 4;
        auto &v0 = vbuf[j + 0]; v0.t = float2(x1, y1                ); v0.p = float3(x,           y,          0);
        auto &v1 = vbuf[j + 1]; v1.t = float2(x1, y1 + fontheighttex); v1.p = float3(x,           y + height, 0);
        auto &v2 = vbuf[j + 2]; v2.t = float2(x2, y1 + fontheighttex); v2.p = float3(x + advance, y + height, 0);
        auto &v3 = vbuf[j + 3]; v3.t = float2(x2, y1                ); v3.p = float3(x + advance, y,          0);
        *idx++ = j + 0;
        *idx++ = j + 1;
        *idx++ = j + 2;
        *idx++ = j + 2;
        *idx++ = j + 3;
        *idx++ = j + 0;
        x += advance;
    }

    RenderArray(PRIM_TRIS, len * 6, "PT", sizeof(PT), vbuf, ibuf);

    delete[] ibuf;
    delete[] vbuf;
}

const int2 BitmapFont::TextSize(const char *text)
{
    if (!CacheChars(text))
        return int2_0;

    auto x = 0;
    for (;;)
    {
        int c = FromUTF8(text);
        if (c <= 0) return int2(x, height);
        x += positions[font->unicodemap[c]].z();

    }
}


FT_Library library = nullptr;

OutlineFont *LoadFont(const char *name)
{
    FT_Error err = 0;

    if (!library) err = FT_Init_FreeType(&library);
    if (!err)
    {
        size_t len = 0;
        auto fbuf = LoadFile(name, &len);
        if (fbuf)
        {
            FT_Face face;
            err = FT_New_Memory_Face(library, fbuf, len, 0, &face);
            if (!err) return new OutlineFont(face, fbuf);
            free(fbuf);
        }
    }
    return nullptr;
}

OutlineFont::~OutlineFont()
{
    FT_Done_Face((FT_Face)fthandle);
    free(fbuf);
}

bool OutlineFont::EnsureCharsPresent(const char *utf8str)
{
    bool anynew = false;
    for (;;)
    {
        int uc = FromUTF8(utf8str);
        if (uc <= 0)
            break;
        auto it = unicodemap.find(uc);
        if (it == unicodemap.end())
        {
            anynew = true;
            unicodemap[uc] = unicodetable.size();
            unicodetable.push_back(uc);
        }
    }
    return anynew;
}

void FTClosedown()
{
    if (library) FT_Done_FreeType(library);
    library = nullptr;
}

