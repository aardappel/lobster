#include "stdafx.h"
#include "geom.h"
#include "tools.h"
#include "openglsup.h"
#include "mctables.h"

Surface *surf;
Geometry *geom;

#define Sample(x, y, z) ((x)*GS*GS + (y)*GS + (z))
#define Fun(pos) (dot(pos, pos) < 1.0f)

struct edge
{
    vec4 a, b;

    edge() {}
    edge(vec3 &_a, vec3 &_b) : a(_a, 0), b(_b, 0) {}
};

struct edgehash
{
    int x, y, z;    // edge index along this axis
};

struct cell
{
    vec3 accum;
    vec3 col;
    int n;

    cell() : n(0), accum(0), col(0) {}
};

struct vert
{
    vec3 pos;
    vec3 norm;
    unsigned char col[4];
};

void mesh_do()
{
    int GS = 32; 

    char *issolid = new char[GS*GS*GS];
    memset(issolid, 0, GS*GS*GS*sizeof(char));

    for (int x = 0; x < GS; x++) for (int y = 0; y < GS; y++) for (int z = 0; z < GS; z++)
    {
        vec3 pos(int3(x, y, z).tovec3());
        pos -= 16.0f;
        pos /= 14.0f;

        issolid[Sample(x, y, z)] = Fun(pos);
    }

    edgehash *edgehashes = new edgehash[GS*GS*GS];
    memset(edgehashes, -1, GS*GS*GS*sizeof(edgehash));

    vector<int> triangles;
    vector_sse<edge> edges;

    for (int x = 0; x < GS-1; x++) for (int y = 0; y < GS-1; y++) for (int z = 0; z < GS-1; z++)
    {
        struct svec
        {
            unsigned short x, y, z;  // coordinates of the MC grid
            char solid;
        };

        svec grid[8] =
        {
            #define SV(X, Y, Z) { x + X, y + Y, z + Z, issolid[Sample(x + X, y + Y, z + Z)] },
            SV(0, 0, 0)
            SV(1, 0, 0)
            SV(1, 1, 0)
            SV(0, 1, 0)
            SV(0, 0, 1)
            SV(1, 0, 1)
            SV(1, 1, 1)
            SV(0, 1, 1)
            #undef SV
        };

        int ci = 0;
        for (int i = 0; i < 8; i++) ci |= grid[i].solid << i;
        //assert(ci < 256 && ci >= 0);

        if (mc_edge_table[ci] == 0) continue;

        static int vertlist[12];

        for (int i = 0; i < 12; i++) if (mc_edge_table[ci] & (1<<i))
        {
            svec &p1 = grid[mc_edge_to_vert[i][0]];
            svec &p2 = grid[mc_edge_to_vert[i][1]];
            int dir = p1.x < p2.x ? 0 : p1.y < p2.y ? 1 : 2;
            int &idx = *((&edgehashes[Sample(p1.x, p1.y, p1.z)].x) + dir);
            if (idx < 0)
            {
                edges.push_back(p1.solid ? edge(vec3(p1.x, p1.y, p1.z), vec3(p2.x, p2.y, p2.z))
                    : edge(vec3(p2.x, p2.y, p2.z), vec3(p1.x, p1.y, p1.z)));
                idx = edges.size() - 1;
            }
            vertlist[i] = idx;
        }

        for (int i = 0; mc_tri_table[ci][i] != -1; i++)
        {
            triangles.push_back(vertlist[mc_tri_table[ci][i]]);
        }
    }

    delete[] issolid;
    delete[] edgehashes;

    printf("e = %d\n", edges.size());

    for (size_t i = 0; i < edges.size(); i++)
    {
        auto &e = edges[i];
        vec3 a = e.a.xyz();
        vec3 b = e.b.xyz();

        vec3 pos = (a + b) / 2;
        vec3 col(1, 1, 1);

        e.a = vec4(pos, 0);
        e.b = vec4(col, 0);
    }

    int *idxs = new int[GS*GS*GS];
    memset(idxs, -1, GS*GS*GS*sizeof(int));

    vector<int> iverts;
    vector<cell> cells;

    for(size_t i = 0; i < edges.size(); i++)
    {
        edge &e = edges[i];

        int &idx = idxs[Sample(int(e.a.x + 0.5f), int(e.a.y + 0.5f), int(e.a.z + 0.5f))];

        if (idx < 0)
        {
            idx = cells.size();
            cells.push_back(cell());
        }

        cell &c = cells[idx];
        c.accum += e.a.xyz();
        c.n--;
        c.col += e.b.xyz();
        iverts.push_back(&idx - idxs);
    }

    vector<int> triangles2;
    vector<vert> verts;

    for (size_t t = 0; t < triangles.size(); t += 3)
    {
        int i[3];
        i[0] = iverts[triangles[t + 0]];
        i[1] = iverts[triangles[t + 1]];
        i[2] = iverts[triangles[t + 2]];

        if (i[0] != i[1] && i[0] != i[2] && i[1] != i[2]) for (int j = 0; j < 3; j++)
        {
            cell &c = cells[idxs[i[j]]];
            if (c.n < 0)
            {
                c.accum /= (float)-c.n;
                c.col /= (float)-c.n;
                c.n = verts.size();
                verts.push_back(vert());
                vert &v = verts.back();
                v.pos = c.accum;
                v.norm = vec3(0, 0, 0);
                quantizec(c.col, v.col);
            }
            triangles2.push_back(c.n);
        }
    }

    delete[] idxs;

    for (size_t t = 0; t < triangles2.size(); t += 3)
    {
        vert &v1 = verts[triangles2[t + 0]];
        vert &v2 = verts[triangles2[t + 1]];
        vert &v3 = verts[triangles2[t + 2]];

        vec3 v12 = normalize(v2.pos - v1.pos);
        vec3 v13 = normalize(v3.pos - v1.pos);
        vec3 v23 = normalize(v3.pos - v2.pos);
        vec3 d3  = normalize(cross(v13, v12));

        v1.norm += d3 * (1 - dot( v12, v13));
        v2.norm += d3 * (1 - dot(-v12, v23));
        v3.norm += d3 * (1 - dot(-v23,-v13));
    }

    for (size_t i = 0; i < verts.size(); i++)
    {
        verts[i].norm.normalize();
    }

    printf("verts = %d, tris = %d, GS = %d\n", verts.size(), triangles2.size()/3, GS);

    surf = new Surface(&triangles2[0], triangles2.size());  // FIXME no dealloc
    geom = new Geometry(&verts[0], verts.size(), sizeof(vert), "PNC");
}

void mesh_render(mat4 &mvp, vec4 &curcolor)
{
    glDisable(GL_BLEND);
    phongshader.set(mvp, curcolor);
    geom->Render(surf);
    glEnable(GL_BLEND);
}
