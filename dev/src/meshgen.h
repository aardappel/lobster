
#include "3dgrid.h"

struct fcell
{
    enum { MAXEDGE = 256 * 256 };

    int edgeidx[3];
    ushort edges[3];
    short material[3];

    fcell()
    {
        for (int i = 0; i < 3; i++)
        {
            edgeidx[i] = -1;
            edges[i] = 0;
            material[i] = 0;
        }
    }
};

class findex
{
    int i;

public:

    enum { MATBITS = 2, MATMASK = (1<<MATBITS) - 1 };

    findex() : i(0) {}

    int getm() { return i & MATMASK; }
    int geti() { return i >> MATBITS; }

    void setm(int m) { i = (i & ~MATMASK) | m; }
    void seti(int j) { i = (i & MATMASK) | (j << MATBITS); }

    bool operator==(const findex &o) { return i == o.i; }
    bool operator!=(const findex &o) { return i != o.i; }
};

// Turns out the way shapes overlap and are rasterized invidually below makes this not as efficient a data
// structure for this purpose as it at first seemed.
//typedef RLE3DGrid<findex> FIndexGrid;
//typedef RLE3DGrid<int> IntGrid;

// So use this instead:
typedef Chunk3DGrid<findex> FIndexGrid;
typedef Chunk3DGrid<int> IntGrid;

struct mgvert
{
    float3 pos;
    float3 norm;
    byte4 col;
};

inline void RecomputeNormals(vector<int> &triangles, vector<mgvert> &verts)
{
    normalize_mesh(triangles.data(), triangles.size(), verts.data(), verts.size(), sizeof(mgvert),
                   (uchar *)&verts.data()->norm - (uchar *)&verts.data()->pos, false);
};

extern int polygonize_mc(const int3 &gridsize, float gridscale, const float3 &gridtrans, const FIndexGrid *fcellindices,
                         vector<fcell> &fcells, const vector<float3> &materials, 
                         float3 (* grid_to_world)(const int3 &pos));
