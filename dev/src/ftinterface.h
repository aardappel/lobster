// simple interface for FreeType (that doesn't depend on its headers)

struct BitmapFont;

struct OutlineFont
{
    void *fthandle;
    void *fbuf;

	map<int, int> unicodemap;
	vector<int> unicodetable;

    OutlineFont(void *fth, void *fb) : fthandle(fth), fbuf(fb) {}
    ~OutlineFont();

	bool EnsureCharsPresent(const char *utf8str);
};

struct BitmapFont
{
    uint texid;
    vector<int3> positions;  
    int height;
    size_t texh, texw;
    int usedcount;

	int size;

	OutlineFont *font;

    ~BitmapFont();
    BitmapFont(OutlineFont *_font, int _size);

    void RenderText(Shader *sh, const char *text);
    const int2 TextSize(const char *text);

	bool CacheChars(const char *text);
};

extern OutlineFont *LoadFont(const char *name);

extern void FTClosedown();
