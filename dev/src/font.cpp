#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

#include "glinterface.h"
#include "ftinterface.h"

map<string, BitmapFont *> fontcache;
BitmapFont *curfont = NULL;
int curfontsize = -1;
int maxfontsize = 128;

map<string, OutlineFont *> loadedfaces;
OutlineFont *curface = NULL;
string curfacename;

Shader *texturedshader = NULL;

void CullFonts()
{
    for(auto it = fontcache.begin(); it != fontcache.end(); )
    {
        if (it->second->usedcount)
        {
            it->second->usedcount = 0;
            it++;
        }
        else
        {
            if (curfont == it->second) curfont = NULL;
            delete it->second;
            fontcache.erase(it++);
        }
    }
}

void FontCleanup()
{
    for (auto e : fontcache) delete e.second;
    fontcache.clear();
    curfont = NULL;

    for (auto e : loadedfaces) delete e.second;
    loadedfaces.clear();
    curface = NULL;

    FTClosedown();
}

void AddFont()
{
    STARTDECL(gl_setfontname) (Value &fname)   
    {       
        extern void TestGL(); TestGL();

        string piname = SanitizePath(fname.sval->str());
        fname.DEC();

        auto faceit = loadedfaces.find(piname);
        if (faceit != loadedfaces.end())
        {
            curface = faceit->second;
            curfacename = piname;
            return Value(true);
        }

        texturedshader = LookupShader("textured");
        assert(texturedshader);

        curface = LoadFont(piname.c_str());
        if (curface) 
        { 
            curfacename = piname;
            loadedfaces[piname] = curface;
            return Value(true);
        }
        else
        {
            return Value(false);
        }
    }
    ENDDECL1(gl_setfontname, "filename", "S", "I", "sets a freetype/OTF/TTF font as current (and loads it from disk the first time). returns true if success.");

    STARTDECL(gl_setfontsize) (Value &fontsize) 
    {
        if (!curface) g_vm->BuiltinError("gl_setfontsize: no current font set with gl_setfontname");
        
        int size = max(1, fontsize.ival);
        int csize = min(size, maxfontsize);

        string fontname = curfacename;
        fontname += inttoa(csize);

        auto fontelem = fontcache.find(fontname);
        if (fontelem != fontcache.end())
        {
            curfont = fontelem->second;
            curfontsize = size;
            return Value(true);
        }

        curfont = new BitmapFont(curface, csize);
                 
        fontcache.insert(make_pair(fontname, curfont));
        curfontsize = size;

        return Value(true);
    }
    ENDDECL1(gl_setfontsize, "size", "I", "I", "sets the font for rendering into this fontsize (in pixels). caches into a texture first time this size is used, flushes from cache if this size is not used an entire frame. font rendering will look best if using 1:1 pixels (careful with gl_scale/gl_translate). returns true if success");

    STARTDECL(gl_setmaxfontsize) (Value &fontsize) { maxfontsize = fontsize.ival; return Value(0); } ENDDECL1(gl_setmaxfontsize, "size", "I", "", "sets the max font size to render to bitmaps. any sizes specified over that by setfontsize will still work but cause scaled rendering. default 128");

    STARTDECL(gl_getfontsize) () { return Value(curfontsize); } ENDDECL0(gl_getfontsize, "", "", "I", "the current font size");

    STARTDECL(gl_text) (Value &s)
    {
        auto f = curfont;
        if (!f) { s.DEC(); return g_vm->BuiltinError("gl_text: no font size set"); }

        if (!s.sval->len) return s;

        float4x4 oldobject2view;
        if (curfontsize > maxfontsize) { oldobject2view = object2view; object2view *= scaling(curfontsize / float(maxfontsize)); }

        SetTexture(0, f->texid);
		texturedshader->Set();
        f->RenderText(s.sval->str());

        if (curfontsize > maxfontsize) object2view = oldobject2view;

        return s;
    }
    ENDDECL1(gl_text, "text", "S", "S", "renders a text with the current font (at the current coordinate origin)");

    STARTDECL(gl_textsize) (Value &s)
    {
        auto f = curfont;
        if (!f) { s.DEC(); return g_vm->BuiltinError("gl_textsize: no font size set"); }

        auto size = f->TextSize(s.sval->str());
        s.DEC();
        
        if (curfontsize > maxfontsize)
        {
            size = int2(ceilf(float2(size) * float(curfontsize) / float(maxfontsize)));
        }

        return ToValue(size);
    }
    ENDDECL1(gl_textsize, "text", "S", "V", "the x/y size in pixels the given text would need");
}

AutoRegister __af("font", AddFont);
