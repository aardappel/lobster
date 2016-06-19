#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

#include "glinterface.h"

#include "3dgrid.h"

using namespace lobster;

Chunk3DGrid<byte4> *grid = nullptr;

void CubeGenClear()
{
    if (grid) delete grid;
    grid = nullptr;
}

void AddCubeGen()
{
    STARTDECL(mg_init_cubes) (Value &size)
    {
        CubeGenClear();
        grid = new Chunk3DGrid<byte4>(ValueDecToI<3>(size), byte4_0);
        return Value();
    }
    ENDDECL1(mg_init_cubes, "size", "I]:3", "",
        "initializes a new, empty 3D cube world. 4 bytes per cell, careful with big sizes :)");

    STARTDECL(mg_set_cubes) (Value &pos, Value &size, Value &color)
    {
        auto p = ValueDecToI<3>(pos);
        auto sz = ValueDecToI<3>(size);
        auto col = quantizec(ValueDecToF<4>(color));
        if (!grid) return Value();
        for (int x = max(0, p.x()); x < min(p.x() + sz.x(), grid->dim.x()); x++)
        {
            for (int y = max(0, p.y()); y < min(p.y() + sz.y(), grid->dim.y()); y++)
            {
                for (int z = max(0, p.z()); z < min(p.z() + sz.z(), grid->dim.z()); z++)
                {
                    grid->Set(int3(x, y, z), col);
                }
            }
        }
        return Value();
    }
    ENDDECL3(mg_set_cubes, "pos,size,color", "I]:3I]:3F]:4", "",
        "sets a range of cubes to a certain color. alpha < 0.5 is considered empty space");

    STARTDECL(mg_create_cube_mesh) ()
    {
        if (!grid) return Value(0);

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

        // Woah nested loops!
        for (int x = 0; x < grid->dim.x(); x++)
        {
            for (int y = 0; y < grid->dim.y(); y++)
            {
                for (int z = 0; z < grid->dim.z(); z++)
                {
                    auto pos = int3(x, y, z);
                    auto c = grid->Get(pos);
                    if (c.w() >= 128)
                    {
                        for (int n = 0; n < 6; n++)
                        {
                            auto npos = pos + neighbors[n];
                            auto nc = npos >= 0 && npos < grid->dim ? grid->Get(npos) : byte4_0;
                            if (nc.w() < 128)
                            {
                                auto face = faces[n];
                                int istart = (int)verts.size();
                                triangles.push_back(istart + 0);
                                triangles.push_back(istart + 1);
                                triangles.push_back(istart + 3);
                                triangles.push_back(istart + 1);
                                triangles.push_back(istart + 2);
                                triangles.push_back(istart + 3);
                                for (int vn = 0; vn < 4; vn++)
                                {
                                    cvert vert;
                                    for (int d = 0; d < 3; d++)
                                    {
                                        vert.pos.set(d, float((face[vn] & (1 << (2 - d))) != 0));
                                    }
                                    vert.pos += float3(pos);
                                    vert.normal = float3(neighbors[n]);
                                    vert.color = c;
                                    verts.push_back(vert);  // FIXME: duplication!
                                }
                            }
                        }
                    }
                }
            }
        }

        Output(OUTPUT_DEBUG, "cubegen verts = %lu, tris = %lu\n", verts.size(), triangles.size() / 3);

        auto m = new Mesh(new Geometry(&verts[0], verts.size(), sizeof(cvert), "PNC"), PRIM_TRIS);
        m->surfs.push_back(new Surface(&triangles[0], triangles.size(), PRIM_TRIS));

        extern IntResourceManagerCompact<Mesh> *meshes;
        return (int)meshes->Add(m);
    }
    ENDDECL0(mg_create_cube_mesh, "", "", "I",
        "converts world to a mesh");
}
