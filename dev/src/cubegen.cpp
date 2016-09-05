#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

#include "glinterface.h"

#include "3dgrid.h"

using namespace lobster;

const unsigned int default_palette[256] = {
    0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
    0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff, 0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
    0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
    0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
    0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc, 0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
    0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
    0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
    0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66, 0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
    0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
    0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
    0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633, 0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
    0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
    0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
    0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000, 0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
    0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
    0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111
};

const uchar transparant = 0; 

struct Voxels
{
    vector<byte4> palette;
    Chunk3DGrid<uchar> grid;
    int i;

    Voxels(const int3 &dim) : grid(dim, transparant), i(0) {}

    void Set(const int3 &p, const int3 &sz, uchar pi)
    {
        for (int x = max(0, p.x()); x < min(p.x() + sz.x(), grid.dim.x()); x++)
        {
            for (int y = max(0, p.y()); y < min(p.y() + sz.y(), grid.dim.y()); y++)
            {
                for (int z = max(0, p.z()); z < min(p.z() + sz.z(), grid.dim.z()); z++)
                {
                    grid.Set(int3(x, y, z), pi);
                }
            }
        }
    }
};

IntResourceManagerCompact<Voxels> *voxels_set = nullptr;

Voxels &GetVoxels(Value &i)
{
    auto v = voxels_set->Get(i.ival());
    if (!v) g_vm->BuiltinError("illegal voxel grid id: " + to_string(i.ival()));
    return *v;
}

void CubeGenClear()
{
    if (voxels_set) delete voxels_set;
    voxels_set = nullptr;
}

Voxels &NewWorld(const int3 &size)
{
    if (!voxels_set) voxels_set = new IntResourceManagerCompact<Voxels>([](Voxels *v) { delete v; });
    auto v = new Voxels(size);
    v->i = (int)voxels_set->Add(v);
    v->palette.insert(v->palette.end(), (byte4 *)default_palette, ((byte4 *)default_palette) + 256);
    return *v;
}

void AddCubeGen()
{
    STARTDECL(cg_init) (Value &size)
    {
        auto &v = NewWorld(ValueDecToI<3>(size));
        return Value(v.i);
    }
    ENDDECL1(cg_init, "size", "I]:3", "I",
        "initializes a new, empty 3D cube world. 4 bytes per cell, careful with big sizes :)"
        " returns the world id");

    STARTDECL(cg_size) (Value &wid)
    {
        return Value(ToValueI(GetVoxels(wid).grid.dim));
    }
    ENDDECL1(cg_size, "worldid", "I", "I]:3",
        "returns the current world size");

    STARTDECL(cg_set) (Value &wid, Value &pos, Value &size, Value &color)
    {
        auto p = ValueDecToI<3>(pos);
        auto sz = ValueDecToI<3>(size);
        GetVoxels(wid).Set(p, sz, (uchar)color.ival());
        return Value();
    }
    ENDDECL4(cg_set, "worldid,pos,size,paletteindex", "II]:3I]:3I", "",
        "sets a range of cubes to palette index. index 0 is considered empty space");

    STARTDECL(cg_color_to_default_palette) (Value &color)
    {
        auto col = quantizec(ValueDecToF<4>(color));
        uchar pi = transparant;
        if (col.w() >= 128)
        {
            auto ic = byte4((int4(col) + (0x33 / 2)) / 0x33);
            pi = (5 - ic.x()) * 36 +
                (5 - ic.y()) * 6 +
                (5 - ic.z()) + 1;
        }
        return Value(pi);
    }
    ENDDECL1(cg_color_to_default_palette, "color", "F]:4", "I",
        "converts a color to a palette index. alpha < 0.5 is considered empty space."
        " note: only works for the default palette");

    STARTDECL(cg_palette_to_color) (Value &wid, Value &pal)
    {
        auto p = uchar(pal.ival());
        return Value(ToValueF(color2vec(GetVoxels(wid).palette[p])));
    }
    ENDDECL2(cg_palette_to_color, "worldid,paletteindex", "II", "F]:4",
        "converts a palette index to a color. empty space (index 0) will have 0 alpha");

    STARTDECL(cg_copy_palette) (Value &fromworld, Value &toworld)
    {
        auto &w1 = GetVoxels(fromworld);
        auto &w2 = GetVoxels(toworld);
        w2.palette.clear();
        w2.palette.insert(w2.palette.end(), w1.palette.begin(), w1.palette.end());
        return Value();
    }
    ENDDECL2(cg_copy_palette, "fromworld,toworld", "II", "",
        "");

    STARTDECL(cg_create_mesh) (Value &wid)
    {
        auto &v = GetVoxels(wid);

        static int3 neighbors[] =
        {
            int3(1, 0, 0), int3(-1,  0,  0),
            int3(0, 1, 0), int3( 0, -1,  0),
            int3(0, 0, 1), int3( 0,  0, -1),
        };

        // FIXME: normal can be byte4, pos can short4
        struct cvert { float3 pos; float3 normal; byte4 color; };
        vector<cvert> verts;
        vector<int> triangles;

        static const char *faces[6] = { "4576", "0231", "2673", "0154", "1375", "0462" };
        static int indices[6] = { 0, 1, 3, 1, 2, 3 };

        // off by default because slow, maybe try: https://github.com/greg7mdp/sparsepp
        bool optimize_verts = false;

        struct VKey
        {
            vec<short, 3> pos;
            uchar pal, dir;
            bool operator==(const VKey &o) const { return pos == o.pos && pal == o.pal && dir == o.dir; };
        };
        auto hasher = [](const VKey &k)
        {
            return k.pos.x() ^ (k.pos.y() << 3) ^ (k.pos.z() << 6) ^ (k.pal << 3) ^ k.dir;
        };
        unordered_map<VKey, int, decltype(hasher)> vertlookup(optimize_verts ? 100000 : 10, hasher);

        RandomNumberGenerator<PCG32> rnd;
        vector<float> rnd_offset(1024);
        for (auto &f : rnd_offset) { f = (rnd.rndfloat() - 0.5f) * 0.15f; }

        // Woah nested loops!
        for (int x = 0; x < v.grid.dim.x(); x++)
        {
            for (int y = 0; y < v.grid.dim.y(); y++)
            {
                for (int z = 0; z < v.grid.dim.z(); z++)
                {
                    auto pos = int3(x, y, z);
                    auto c = v.grid.Get(pos);
                    if (c != transparant)
                    {
                        for (int n = 0; n < 6; n++)
                        {
                            auto npos = pos + neighbors[n];
                            auto nc = npos >= 0 && npos < v.grid.dim ? v.grid.Get(npos) : transparant;
                            if (nc == transparant)
                            {
                                auto face = faces[n];
                                int vindices[4];
                                for (int vn = 0; vn < 4; vn++)
                                {
                                    int3 vpos;
                                    for (int d = 0; d < 3; d++)
                                    {
                                        vpos.set(d, (face[vn] & (1 << (2 - d))) != 0);
                                    }
                                    vpos += pos;

                                    VKey vkey { vec<short, 3>(vpos), c, (uchar)n };
                                    if (optimize_verts)
                                    { 
                                        auto it = vertlookup.find(vkey);
                                        if (it != vertlookup.end())
                                        {
                                            vindices[vn] = it->second;
                                            continue;
                                        }
                                    }
                                    cvert vert;
                                    auto oi = ((vpos.z() << 8) ^ (vpos.y() << 4) ^ vpos.x()) % (rnd_offset.size() - 2);
                                    auto offset = float3(&rnd_offset[oi]);
                                    vert.pos = float3(vpos) + offset;
                                    vert.normal = float3(neighbors[n]);
                                    vert.color = v.palette[c];
                                    vindices[vn] = (int)verts.size();
                                    verts.push_back(vert);

                                    if (optimize_verts) vertlookup[vkey] = vindices[vn];
                                }
                                for (int i = 0; i < 6; i++) triangles.push_back(vindices[indices[i]]);
                            }
                        }
                    }
                }
            }
        }

        normalize_mesh(triangles.data(), triangles.size(), verts.data(), verts.size(), sizeof(cvert),
                        (uchar *)&verts.data()->normal - (uchar *)&verts.data()->pos, false);

        Output(OUTPUT_INFO, "cubegen verts = %lu, tris = %lu\n", verts.size(), triangles.size() / 3);

        auto m = new Mesh(new Geometry(verts.data(), verts.size(), sizeof(cvert), "PNC"), PRIM_TRIS);
        m->surfs.push_back(new Surface(triangles.data(), triangles.size(), PRIM_TRIS));

        extern IntResourceManagerCompact<Mesh> *meshes;
        return (int)meshes->Add(m);
    }
    ENDDECL1(cg_create_mesh, "worldid", "I", "I",
        "converts world to a mesh");

    STARTDECL(cg_load_vox) (Value &name)
    {

        size_t len = 0;
        auto buf = LoadFile(name.sval()->str(), &len);
        name.DECRT();
        if (!buf) return Value(0);
        
        if (strncmp((char *)buf, "VOX ", 4))
        {
            free(buf);
            return Value(0);
        }

        int3 size = int3_0;
        Voxels *voxels = nullptr;

        auto p = buf + 8;
        while (p < buf + len)
        {
            auto id = p;
            p += 4;
            auto contentlen = *((int *)p);
            p += 8;
            if (!strncmp((char *)id, "SIZE", 4))
            {
                size = int3((int *)p);
            }
            else if (!strncmp((char *)id, "RGBA", 4))
            {
                assert(voxels);
                voxels->palette.clear();
                voxels->palette.push_back(byte4_0);
                voxels->palette.insert(voxels->palette.end(), (byte4 *)p, ((byte4 *)p) + 255);
            }
            else if (!strncmp((char *)id, "XYZI", 4))
            {
                assert(size.x());
                voxels = &NewWorld(size);
                auto numvoxels = *((int *)p);
                for (int i = 0; i < numvoxels; i++)
                {
                    auto vox = byte4((uchar *)(p + i * 4 + 4));
                    voxels->grid.Set(int3(vox.xyz()), vox.w());  // FIXME: check bounds.
                }
            }
            p += contentlen;
        }

        free(buf);
        return Value(voxels->i);
    }
    ENDDECL1(cg_load_vox, "name", "S", "I",
        "loads a file in the .vox format (MagicaVoxel). returns world id or 0 if file failed to load");

    STARTDECL(cg_save_vox) (Value &wid, Value &name)
    {
        auto &v = GetVoxels(wid);

        if (!(v.grid.dim < 256))
        {
            name.DECRT();
            return Value(false);
        }

        vector<byte4> voxels;
        for (int x = 0; x < v.grid.dim.x(); x++)
        {
            for (int y = 0; y < v.grid.dim.y(); y++)
            {
                for (int z = 0; z < v.grid.dim.z(); z++)
                {
                    auto pos = int3(x, y, z);
                    auto i = v.grid.Get(pos);
                    if (i) voxels.push_back(byte4(int4(pos, i)));
                }
            }
        }

        FILE *f = OpenForWriting(name.sval()->str(), true);
        name.DECRT();
        if (!f) return Value(false);

        auto wint = [&](int i) { fwrite(&i, 4, 1, f); };
        auto wstr = [&](const char *s) { fwrite(s, 4, 1, f); };

        wstr("VOX ");
        wint(150);

        wstr("MAIN");
        wint(0);
        wint(24 /* SIZE */ + 12 + 1024 /* RGBA */ + 16 + (int)voxels.size() * 4 /* XYZI */);

        wstr("SIZE");
        wint(12);
        wint(0);
        wint(v.grid.dim.x());
        wint(v.grid.dim.y());
        wint(v.grid.dim.z());

        wstr("RGBA");
        wint(256 * 4);
        wint(0);
        fwrite(v.palette.data() + 1, 4, 255, f);
        wint(0);

        wstr("XYZI");
        wint((int)voxels.size() * 4 + 4);
        wint(0);
        wint((int)voxels.size());
        fwrite(voxels.data(), 4, voxels.size(), f);

        fclose(f);
        return Value(true);
    }
    ENDDECL2(cg_save_vox, "worldid,name", "IS", "I",
        "saves a file in the .vox format (MagicaVoxel). returns false if file failed to save."
        " this format can only save worlds < 256^3, will fail if bigger");

    STARTDECL(cg_delete) (Value &wid)
    {
        voxels_set->Delete(wid.ival());
        return Value();
    }
    ENDDECL1(cg_delete, "worldid", "I", "",
        "deletes a world");


}
