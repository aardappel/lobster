#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

#include "glinterface.h"

#include "meshgen.h"
#include "mctables.h"
#include "polyreduce.h"

using namespace lobster;


/* TODO:

- a function that takes a list of cylinder widths to be able to do wineglasses etc
- replace pow by something faster?
- proper groups, so it can help in early culling
- but big grids in compressed columns, so it can possibly run on larger dimension
- turn some of the push_back vector stuff into single allocations
- could work on sharp crease boolean version (see treesheets)
- crease texturing: verts either get a tc of (0, rnd) when they are on a crease, or (1, rnd) when they are flat
*/

extern float simplexNoise(const int octaves, const float persistence, const float scale, const float4 &v);

struct ImplicitFunction;

const int maxedge = 256 * 256;

struct fcell
{
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

const int MATBITS = 2;
const int MATMASK = (1<<MATBITS) - 1;

class findex
{
    int i;

    public:

    findex() : i(0) {}

    int getm() { return i & MATMASK; }
    int geti() { return i >> MATBITS; }

    void setm(int m) { i = (i & ~MATMASK) | m; }
    void seti(int j) { i = (i & MATMASK) | (j << MATBITS); }
};

inline int IndexGrid(const int3 &v, const int3 &gridsize)
{
    return v.x() * gridsize.y() * gridsize.z() +
           v.y() * gridsize.z() +
           v.z();
}

float3 rotated_size(const float3x3 &rot, const float3 &size)
{
    float3x3 absm = float3x3(abs(rot[0]), abs(rot[1]), abs(rot[2]));
    return absm * size;
}

struct ImplicitFunction
{
    float3 orig;
    float3 size;
    float3x3 rot;
    int material;

    virtual ~ImplicitFunction() {}

    inline bool Eval(const float3 & /*pos*/) { return false; }

    virtual float3 ComputeSize() { return size; };
    virtual void FillGrid(const int3 &start, const int3 &end, const int3 &gridsize, findex *fcellindices,
                          vector<fcell> &fcells, const float3 &gridscale, const float3 &gridtrans,
                          const float3x3 &gridrot) = 0;
};

static int3 axesi[] = { int3(1, 0, 0), int3(0, 1, 0), int3(0, 0, 1) };
static float3 axesf[] = { float3_x, float3_y, float3_z };

// use curiously recurring template pattern to allow implicit function to be inlined in rasterization loop

template<typename T> struct ImplicitFunctionImpl : ImplicitFunction
{
    void FillGrid(const int3 &start, const int3 &end, const int3 &gridsize, findex *fcellindices,
                  vector<fcell> &fcells, const float3 &gridscale, const float3 &gridtrans, const float3x3 &gridrot)
    {
        assert(end <= gridsize && int3(0) <= start);

        for (int x = start.x(); x < end.x(); x++)
            for (int y = start.y(); y < end.y(); y++)
                for (int z = start.z(); z < end.z(); z++)
        {
            int3 ipos(x, y, z);

            auto pos = float3(ipos);
            pos -= gridtrans;
            pos = pos * gridrot;
            pos /= gridscale;
            bool inside = static_cast<T *>(this)->Eval(pos);

            // bounding box is supposed to cover the entire shape with 1 empty cell surrounding in all directions
            assert(!inside || (ipos > start && ipos < end - 1));

            findex &fi = fcellindices[IndexGrid(ipos, gridsize)];
            int tmat = fi.getm();
            if (inside)
            {
                tmat = material;
                fi.setm(tmat);
            }

            for (int i = 0; i < 3; i++)
            {
                int3 opos = ipos - axesi[i];
                if (opos[i] >= 0)
                {
                    findex &fio = fcellindices[IndexGrid(opos, gridsize)];
                    int omat = fio.getm();

                    if ((omat != 0) != (tmat != 0))
                    {
                        // FIXME: factor this out, or make cell lookup
                        if (inside || static_cast<T *>(this)->Eval(pos - (axesf[i] * gridrot) / gridscale))
                        {
                            auto solidpos = float3(tmat ? ipos : opos);
                            auto emptypos = float3(tmat ? opos : ipos);
                            auto p1f = solidpos;
                            auto p2f = emptypos;
                            float iso;
                            for (int j = 0; ; j++)
                            {
                                auto p = (p1f + p2f) / 2;
                                if (j == 10) { iso = dot(p - solidpos, emptypos - solidpos); break; }

                                auto p2 = p;
                                p2 -= gridtrans;
                                p2 = p2 * gridrot;
                                p2 /= gridscale;

                                if (static_cast<T *>(this)->Eval(p2) == (material != 0)) p1f = p;
                                else                                                     p2f = p;
                            }

                            auto e = int(iso * maxedge);
                            assert(iso >= 0 && iso <= 1);
                            e = max(1, min(maxedge - 1, e));

                            if (!fi.geti())
                            {
                                fi.seti((int)fcells.size());
                                fcells.push_back(fcell());
                            }

                            fcell &fc = fcells[fi.geti()];

                            if (fc.edges[i])
                            {
                                int oe = tmat ? maxedge - fc.edges[i] : fc.edges[i];
                                e = material ? max(e, oe) : min(e, oe);
                            }
                            fc.edges[i] = ushort(tmat ? maxedge - e : e);
                            fc.material[i] = short(tmat ? tmat : omat);  // FIXME: make this depend on who is closer
                        }
                    }
                    else
                    {
                        fcells[fi.geti()].edges[i] = 0;
                    }
                }
            }
        }
    }
};

struct IFSphere : ImplicitFunctionImpl<IFSphere>
{
    inline bool Eval(const float3 &pos) { return dot(pos, pos) <= 1; }
};

struct IFCube : ImplicitFunctionImpl<IFCube>
{
    inline bool Eval(const float3 &pos) { return abs(pos) <= 1; }
};

struct IFCylinder : ImplicitFunctionImpl<IFCylinder>
{
    inline bool Eval(const float3 &pos)
    {
        return pos.z() <= 1 && pos.z() >= -1 && dot(pos.xy(), pos.xy()) <= 1;
    }
};

struct IFTaperedCylinder : ImplicitFunctionImpl<IFTaperedCylinder>
{
    float bot, top;

    inline bool Eval(const float3 &pos)
    {
        auto xy = pos.xy();
        auto r = mix(bot, top, pos.z() / 2 + 0.5f);
        return pos.z() <= 1 && pos.z() >= -1 && dot(xy, xy) <= r * r;
    }

    float3 ComputeSize()
    {
        auto rad = max(top, bot);
        return size * float3(rad, rad, 1);
    }
};

// TODO: pow is rather slow...

struct IFSuperQuadric : ImplicitFunctionImpl<IFSuperQuadric>
{
    float3 exp;

    inline bool Eval(const float3 &pos)
    {
        return dot(pow(abs(pos), exp), float3_1) <= 1;
    }
};

struct IFSuperToroid : ImplicitFunctionImpl<IFSuperToroid>
{
    float r;
    float3 exp;

    inline bool Eval(const float3 &pos)
    {
        auto p = pow(abs(pos), exp);
        auto xy = r - sqrtf(p.x() + p.y());
        return powf(fabsf(xy), exp.z()) + p.z() <= 1;
    }

    float3 ComputeSize() { return size * float3(r * 2 + 1, r * 2 + 1, 1); }
};

struct IFSuperQuadricNonUniform : ImplicitFunctionImpl<IFSuperQuadricNonUniform>
{
    float3 exppos, expneg;
    float3 scalepos, scaleneg;

    inline bool Eval(const float3 &pos)
    {
        auto d = pos.iflt(0, scaleneg, scalepos);
        auto e = pos.iflt(0, expneg, exppos);
        auto p = abs(pos) / d;
        return p < 1 && dot(pow(p, e), float3_1) <= 1;
    }

    float3 ComputeSize() { return size * max(scalepos, scaleneg); }
};

struct IFLandscape : ImplicitFunctionImpl<IFLandscape>
{
    float zscale, xyscale;

    inline bool Eval(const float3 &pos)
    {
        if (abs(pos) > 1) return false;
        auto f = simplexNoise(8, 0.5f, xyscale, float4(pos.xy(), 0)) * zscale;
        return pos.z() < f;
    }
};

struct Group : ImplicitFunctionImpl<Group>
{
    vector<ImplicitFunction *> children;

    ~Group()
    {
        for (auto c : children) delete c;
    }

    static inline bool Eval(const float3 & /*pos*/) { return false; }

    float3 ComputeSize()
    {
        float3 p1, p2;
        for (auto c : children)
        {
            auto csz = c->ComputeSize();
            csz = rotated_size(c->rot, csz);
            p1 = c == children[0] ? c->orig - csz : min(p1, c->orig - csz);
            p2 = c == children[0] ? c->orig + csz : max(p2, c->orig + csz);
        }
        auto sz = (p2 - p1) / 2;
        orig = p1 + sz;
        return sz;
    }

    void FillGrid(const int3 & /*start*/, const int3 & /*end*/, const int3 &gridsize, findex *fcellindices,
                  vector<fcell> &fcells, const float3 &gridscale, const float3 &gridtrans, const float3x3 & /*gridrot*/)
    {
        for (auto c : children)
        {
            if (dot(c->size, gridscale) > 3)
            {
                auto trans = gridtrans + c->orig * gridscale;
                auto scale = gridscale * c->size;

                //auto _start = int3(trans - c->size * gridscale - 0.01f);
                //auto _end   = int3(trans + c->size * gridscale + 2.01f);


                auto rsize = rotated_size(c->rot, c->ComputeSize());

                auto start = int3(trans - rsize * gridscale - 0.01f);
                auto end   = int3(trans + rsize * gridscale + 2.01f);

                auto bs    = end - start;

                if (bs > 1) c->FillGrid(start, end, gridsize, fcellindices, fcells, scale, trans, c->rot);
            }
        }
    }
};

float noisestretch = 1;
float noiseintensity = 0;
float randomizeverts = 0;

bool pointmode = false;

int polygonize_mc(ImplicitFunction *root, const int targetgridsize, vector<float3> &materials)
{
    auto scenesize = root->ComputeSize() * 2;

    float biggestdim = max(scenesize.x(), max(scenesize.y(), scenesize.z()));

    auto gridscale = targetgridsize / biggestdim;

    auto gridsize = int3(scenesize * gridscale + float3(2.5f));
    auto gridcount = gridsize.x() * gridsize.y() * gridsize.z();

    auto gridtrans = (float3(gridsize) - 1) / 2 - root->orig * gridscale;

    auto fcellindices = new findex[gridcount];
    memset(fcellindices, 0, gridcount * sizeof(findex));

    vector<fcell> fcells;
    fcells.push_back(fcell());  // index 0 means no per cell data

    /////////// SAMPLE GRID

    root->FillGrid(int3(0), gridsize, gridsize, fcellindices, fcells, float3(gridscale), gridtrans, float3x3_1);

    /////////// APPLY MC

    struct edge
    {
        float3 mid;
        int material;

        edge(const float3 &_mid, int _m) : mid(_mid), material(_m) {}
    };

    vector<int> mctriangles;
    vector<edge> edges;

    bool mesh_displacent = true;
    bool flat_triangles_opt = true;
    bool simple_occlusion = false;

    bool marching_cubes = true;

    if (marching_cubes)
    {
        int3 gridpos[8];
        findex celli[8];
        int vertlist[12];

        const int3 corners[8] =
        {
            int3(0, 0, 0),
            int3(1, 0, 0),
            int3(1, 1, 0),
            int3(0, 1, 0),
            int3(0, 0, 1),
            int3(1, 0, 1),
            int3(1, 1, 1),
            int3(0, 1, 1),
        };

        for (int x = 0; x < gridsize.x() - 1; x++)
            for (int y = 0; y < gridsize.y() - 1; y++)
                for (int z = 0; z < gridsize.z() - 1; z++)
        {
            int3 pos(x, y, z);

            int ci = 0;
            for (int i = 0; i < 8; i++)
            {
                gridpos[i] = pos + corners[i];
                celli[i] = fcellindices[IndexGrid(gridpos[i], gridsize)];

                ci |= (celli[i].getm() != 0) << i;
            }

            if (mc_edge_table[ci] == 0) continue;

            for (int i = 0; i < 12; i++) if (mc_edge_table[ci] & (1<<i))
            {
                int i1 = mc_edge_to_vert[i][0];
                int i2 = mc_edge_to_vert[i][1];
                int3 &p1 = gridpos[i1];
                int3 &p2 = gridpos[i2];

                int cidx = celli[i2].geti();
                assert(cidx);  // FIXME: this triggers at certain resolutions?

                int dir = p1.x() < p2.x() ? 0 : p1.y() < p2.y() ? 1 : 2;
                fcell &fc = fcells[cidx];
                int &idx = fc.edgeidx[dir];
                if (idx < 0)
                {
                    idx = (int)edges.size();

                    auto e = fc.edges[dir];
                    assert(e);
                    auto mid = mix(float3(p1), float3(p2), float(e) / maxedge);

                    assert(fc.material[dir]);
                    auto material = min(fc.material[dir] - 1, (int)materials.size() - 1);

                    edges.push_back(edge(mid, material));
                }
                vertlist[i] = idx;
            }

            for (int i = 0; mc_tri_table[ci][i] != -1; i++)
            {
                mctriangles.push_back(vertlist[mc_tri_table[ci][i]]);
            }
        }
    }
    else
    {
        // Experimental marching squares slices mode, unfinished and unoptimized.
        mesh_displacent = false;
        flat_triangles_opt = false;
        polyreductionpasses = 0;

        int3 gridpos[3][4];
        findex celli[3][4];
        float3 edgev[4];

        int3 corners[4] =
        {
            int3(0, 0, 0),
            int3(1, 0, 0),
            int3(1, 1, 0),
            int3(0, 1, 0),
        };

        int linelist[16][5] =
        {
            { -1 },
            { 0, 3, -1 },
            { 1, 0, -1 },
            { 1, 3, -1 },
            { 2, 1, -1 },
            { 0, 3, 2, 1, -1 },
            { 2, 0, -1 },
            { 2, 3, -1 },
            { 3, 2, -1 },
            { 0, 2, -1 },
            { 1, 0, 3, 2, -1 },
            { 1, 2, -1 },
            { 3, 1, -1 },
            { 0, 1, -1 },
            { 3, 0, -1 },
            { -1 },
        };

        // Both ambiguous cases use the minimal version since that's less polygons overal.
        int trilist[16][10] =
        {
            { -1 },
            { 0, 7, 1, -1 },
            { 1, 3, 2, -1 },
            { 0, 3, 2, 3, 0, 7, -1 },
            { 3, 5, 4, -1 },
            { 3, 5, 4, 0, 7, 1, -1 },
            { 1, 4, 2, 4, 1, 5, -1 },
            { 0, 7, 2, 7, 5, 2, 5, 4, 2, -1 },
            { 5, 7, 6, -1 },
            { 0, 5, 1, 5, 0, 6, -1 },
            { 1, 3, 2, 5, 7, 6, -1 },
            { 0, 6, 5, 0, 5, 3, 0, 3, 2, -1 },
            { 7, 4, 3, 4, 7, 6, -1 },
            { 6, 4, 3, 6, 3, 1, 6, 1, 0, -1 },
            { 4, 3, 1, 4, 1, 7, 4, 7, 6, -1 },
            { 0, 4, 2, 4, 0, 6, -1 },
        };

        int edgestartcell[4] = { 1, 2, 2, 3 };

        for (int z = 1; z < gridsize.z() - 1; z++)
            for (int x = 0; x < gridsize.x() - 1; x++)
                for (int y = 0; y < gridsize.y() - 1; y++)
        {
            int3 pos(x, y, z);

            int ci[3] = { 0, 0, 0 };
            for (int lz = 0; lz < 3; lz++)
            {
                for (int i = 0; i < 4; i++)
                {
                    gridpos[lz][i] = pos + corners[i] + int3(0, 0, lz - 1);
                    celli[lz][i] = fcellindices[IndexGrid(gridpos[lz][i], gridsize)];
                    ci[lz] |= (celli[lz][i].getm() != 0) << i;
                }
            }

            if (linelist[ci[1]][0] < 0 && ci[1] == ci[0] && ci[1] == ci[2]) continue;
            
            for (int i = 0; i < 4; i++)
            {
                auto &fc = fcells[celli[1][edgestartcell[i]].geti()];
                auto e = fc.edges[i & 1];
                auto mid = e / float(maxedge) - ((i & 2) / 2);
                edgev[i] = float3(gridpos[1][i]).add(i & 1, mid);
            }

            float3 zup(0, 0,  0.5f);
            float3 zdn(0, 0, -0.5f);

            // Side polys.
            for (int i = 0; linelist[ci[1]][i] >= 0; i += 2)
            {
                // FIXME: disambiguate saddles?
                auto a = linelist[ci[1]][i];
                auto b = linelist[ci[1]][i + 1];
                // FIXME: duplicate verts. reuse thru edgeidx.
                mctriangles.push_back(edges.size()); edges.push_back(edge(edgev[a] + zup, 0));
                mctriangles.push_back(edges.size()); edges.push_back(edge(edgev[b] + zup, 0));
                mctriangles.push_back(edges.size()); edges.push_back(edge(edgev[b] + zdn, 0));

                mctriangles.push_back(edges.size()); edges.push_back(edge(edgev[b] + zdn, 0));
                mctriangles.push_back(edges.size()); edges.push_back(edge(edgev[a] + zdn, 0));
                mctriangles.push_back(edges.size()); edges.push_back(edge(edgev[a] + zup, 0));
            }

            // Top polys.
            if (ci[1] != ci[2] || linelist[ci[1]][0] >= 0)
            {
                // FIXME: clip against adjacent level.
                for (int i = 0; trilist[ci[1]][i] >= 0; i += 3)
                {
                    auto a = trilist[ci[1]][i];
                    auto b = trilist[ci[1]][i + 1];
                    auto c = trilist[ci[1]][i + 2];
                    // FIXME: duplicate verts.
                    mctriangles.push_back(edges.size()); edges.push_back(edge((a & 1 ? edgev[a / 2] : float3(gridpos[1][a / 2])) + zup, 1));
                    mctriangles.push_back(edges.size()); edges.push_back(edge((b & 1 ? edgev[b / 2] : float3(gridpos[1][b / 2])) + zup, 1));
                    mctriangles.push_back(edges.size()); edges.push_back(edge((c & 1 ? edgev[c / 2] : float3(gridpos[1][c / 2])) + zup, 1));
                }
            }

            // FIXME: bottom polys missing.
        }
    }

    delete[] fcellindices;

    vector<int> triangles;
    vector<mgvert> verts;

    RandomNumberGenerator<PCG32> r;

    if (mesh_displacent)
    {
        /////////// MESH DISPLACEMENT

        // "Mesh Displacement: An Improved Contouring Method for Trivariate Data"
        // http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.49.5214
        // reduces tris by half, and makes for more regular proportioned tris
        // from: http://chrishecker.com/My_liner_notes_for_spore

        struct dcell
        {
            float3 accum;
            float3 col;
            int n;

            dcell() : accum(float3_0), col(float3_0), n(0) {}
        };

        auto dcellindices = new int[gridcount];
        memset(dcellindices, -1, gridcount * sizeof(int));

        vector<int> iverts;
        vector<dcell> cells;

        for (edge &e : edges)
        {
            int3 ipos(e.mid + 0.5f);
            int &idx = dcellindices[IndexGrid(ipos, gridsize)];

            if (idx < 0)
            {
                idx = (int)cells.size();
                cells.push_back(dcell());
            }

            dcell &c = cells[idx];
            c.accum += e.mid;
            c.col += materials[e.material];
            c.n--;
            iverts.push_back(int(&idx - dcellindices));
        }

        for (size_t t = 0; t < mctriangles.size(); t += 3)
        {
            int i[3];
            i[0] = iverts[mctriangles[t + 0]];
            i[1] = iverts[mctriangles[t + 1]];
            i[2] = iverts[mctriangles[t + 2]];

            if (i[0] != i[1] && i[0] != i[2] && i[1] != i[2])
            {
                for (int j = 0; j < 3; j++)
                {
                    dcell &c = cells[dcellindices[i[j]]];
                    if (c.n < 0)
                    {
                        c.accum /= (float)-c.n;
                        c.col /= (float)-c.n;
                        c.n = (int)verts.size();
                        verts.push_back(mgvert());
                        auto &v = verts.back();
                        v.pos = c.accum;
                        v.norm = float3_0;
                        v.col = quantizec(c.col);
                    }

                    triangles.push_back(c.n);
                }
            }
        }

        delete[] dcellindices;
    }
    else
    {
        for (edge &e : edges)
        {
            verts.push_back(mgvert());
            auto &v = verts.back();
            v.pos = e.mid;
            v.col = quantizec(materials[e.material]);
            v.norm = float3_0;
        }

        triangles.assign(mctriangles.begin(), mctriangles.end());
    }

    /////////// CULL FLAT TRIANGLES

    // TODO: can bad tris simply be culled based on bad normals?
    if (flat_triangles_opt)
    {
        bool *problemvert = new bool[verts.size()];
        memset(problemvert, 0, sizeof(bool) * verts.size());

        const float threshold = -0.98f;

        for (size_t t = 0; t < triangles.size(); t += 3)
        {
            auto &v1 = verts[triangles[t + 0]];
            auto &v2 = verts[triangles[t + 1]];
            auto &v3 = verts[triangles[t + 2]];

            assert(v1.pos != v2.pos && v1.pos != v3.pos && v2.pos != v3.pos);

            float3 d3  = normalize(cross(v3.pos - v1.pos, v2.pos - v1.pos));

            // if a plane normal points away near 180 from past normals, its likely part of triangle with no volume
            // behind it (special case in mesh displacement)
            if (v1.norm != float3_0 && dot(d3, normalize(v1.norm)) < threshold) problemvert[triangles[t + 0]] = true;
            if (v2.norm != float3_0 && dot(d3, normalize(v2.norm)) < threshold) problemvert[triangles[t + 1]] = true;
            if (v3.norm != float3_0 && dot(d3, normalize(v3.norm)) < threshold) problemvert[triangles[t + 2]] = true;
        }

        for (size_t t = 0; t < triangles.size(); t += 3)
        {
            // if all corners have screwey normals, the triangle should be culled
            // could also cull associated verts, but generally so few of them its not worth it
            if ((problemvert[triangles[t + 0]] && problemvert[triangles[t + 1]] && problemvert[triangles[t + 2]]))
            {
                triangles.erase(triangles.begin() + t, triangles.begin() + t + 3);
                t -= 3;
            }
        }

        delete[] problemvert;
    }

    /////////// RANDOMIZE

    if (randomizeverts > 0)
    {
        for (auto &v : verts)
        {
            v.pos += random_point_in_sphere(r) * randomizeverts;
        }
    }

    /////////// ORIGIN/SCALE

    for (auto &v : verts)
    {
        v.pos -= gridtrans;
        v.pos /= gridscale;
    }

    /////////// CALCULATE NORMALS

    RecomputeNormals(triangles, verts);

    /////////// POLYGON REDUCTION

    if (polyreductionpasses)
    {
        PolyReduce(triangles, verts);
    }

    /////////// APPLY NOISE TO COLOR

    if (noiseintensity > 0)
    {
        float scale = noisestretch;
        int octaves = 8;
        float persistence = 0.5f;

        for (auto &v : verts)
        {
            auto n = float3(simplexNoise(octaves, persistence, scale, float4(v.pos, 0.0f / scale)),
                            simplexNoise(octaves, persistence, scale, float4(v.pos, 0.3f / scale)),
                            simplexNoise(octaves, persistence, scale, float4(v.pos, 0.6f / scale)));
            v.col = quantizec(color2vec(v.col).xyz() * (float3_1 - (n + float3_1) / 2 * noiseintensity));
        }
    }

    /////////// MODULATE LIGHTING BY CREASE FACTOR

    if (simple_occlusion)
    {
        float *cfactor = new float[verts.size()];
        memset(cfactor, 0, sizeof(float) * verts.size());

        for (size_t t = 0; t < triangles.size(); t += 3)
        {
            auto &v1 = verts[triangles[t + 0]];
            auto &v2 = verts[triangles[t + 1]];
            auto &v3 = verts[triangles[t + 2]];

            float3 v12 = normalize(v2.pos - v1.pos);
            float3 v13 = normalize(v3.pos - v1.pos);
            float3 v23 = normalize(v3.pos - v2.pos);

            auto centroid = (v1.pos + v2.pos + v3.pos) / 3;
            cfactor[triangles[t + 0]] += dot(v1.norm, normalize(centroid - v1.pos)) * (1 - dot( v12, v13));
            cfactor[triangles[t + 1]] += dot(v1.norm, normalize(centroid - v1.pos)) * (1 - dot(-v12, v23));
            cfactor[triangles[t + 2]] += dot(v1.norm, normalize(centroid - v1.pos)) * (1 - dot(-v23,-v13));
        }

        for (auto &v : verts)
        {
            float f = cfactor[&v - &verts[0]] + 1;
            v.col = byte4(float4(min(float3(255), max(float3_0, float3(v.col.xyz()) - 64 * f)), 255));
        }

        delete[] cfactor;
    }

    if (verts.empty())
        return -1;

    Output(OUTPUT_DEBUG, "verts = %lu, edgeverts = %lu, tris = %lu, mctris = %lu, fcells = %lu, GS = %d, scale = %f\n",
           verts.size(), edges.size(), triangles.size() / 3, mctriangles.size() / 3, fcells.size(), targetgridsize,
           gridscale);

    extern IntResourceManagerCompact<Mesh> *meshes;
    auto m = new Mesh(new Geometry(&verts[0], verts.size(), sizeof(mgvert), "PNC"), pointmode ? PRIM_POINT : PRIM_TRIS);
    if (pointmode)
    {
        m->pointsize = 1000 / gridscale;
    }
    else
    {
        m->surfs.push_back(new Surface(&triangles[0], triangles.size(), PRIM_TRIS));
    }
    return (int)meshes->Add(m);
}


Group *root = nullptr;
Group *curgroup = nullptr;

float3 cursize = float3_1;
float3 curorig = float3_0;
float3x3 currot  = float3x3_1;
int curcol   = 1;

void MeshGenClear()
{
    if (root) delete root;
    root = curgroup = nullptr;

    cursize = float3_1;
    curorig = float3_0;
    currot  = float3x3_1;
    curcol  = 1;
}

Group *GetGroup()
{
    if (!curgroup)
    {
        assert(!root);
        root = new Group();
        curgroup = root;
    }
    return curgroup;
}

Value AddShape(ImplicitFunction *f)
{
    f->size = cursize;
    f->orig = curorig;
    f->rot  = currot;

    f->material = curcol;

    GetGroup()->children.push_back(f);

    return Value();
}

void AddMeshGen()
{
    STARTDECL(mg_sphere)   () { return AddShape(new IFSphere());   } ENDDECL0(mg_sphere,   "", "", "",
        "a unit sphere");
    STARTDECL(mg_cube)     () { return AddShape(new IFCube());     } ENDDECL0(mg_cube,     "", "", "",
        "a unit cube (fits around unit sphere)");
    STARTDECL(mg_cylinder) () { return AddShape(new IFCylinder()); } ENDDECL0(mg_cylinder, "", "", "",
        "a unit cylinder (fits around unit sphere)");

    STARTDECL(mg_tapered_cylinder) (Value &bot, Value &top)
    {
        auto tc = new IFTaperedCylinder();
        tc->bot = bot.fval();
        tc->top = top.fval();
        return AddShape(tc);
    }
    ENDDECL2(mg_tapered_cylinder, "bot,top", "FF", "",
        "a cyclinder where you specify the top and bottom radius, height still 2");

    STARTDECL(mg_superquadric) (Value &exps)
    {
        auto sq = new IFSuperQuadric();
        sq->exp = ValueDecToF<3>(exps);
        return AddShape(sq);
    }
    ENDDECL1(mg_superquadric, "exponents", "F]", "",
        "a super quadric. specify an exponent of 2 for spherical, higher values for rounded squares");

    STARTDECL(mg_supertoroid) (Value &r, Value &exps)
    {
        auto t = new IFSuperToroid();
        t->r = r.fval();
        t->exp = ValueDecToF<3>(exps);
        return AddShape(t);
    }
    ENDDECL2(mg_supertoroid, "R,exponents", "FF]", "",
        "a super toroid. R is the distance from the origin to the center of the ring.");

    STARTDECL(mg_superquadric_non_uniform) (Value &posexps, Value &negexps, Value &posscale, Value &negscale)
    {
        auto sq = new IFSuperQuadricNonUniform();
        sq->exppos   = ValueDecToF<3>(posexps);
        sq->expneg   = ValueDecToF<3>(negexps);
        sq->scalepos = max(float3(0.01f), ValueDecToF<3>(posscale));
        sq->scaleneg = max(float3(0.01f), ValueDecToF<3>(negscale));

        return AddShape(sq);
    }
    ENDDECL4(mg_superquadric_non_uniform, "posexponents,negexponents,posscale,negscale", "F]F]F]F]", "",
        "a superquadric that allows you to specify exponents and sizes in all 6 directions independently for maximum"
        " modelling possibilities");

    STARTDECL(mg_landscape) (Value &zscale, Value &xyscale)
    {
        auto ls = new IFLandscape();
        ls->zscale = zscale.fval();
        ls->xyscale = xyscale.fval();
        return AddShape(ls); 
    } ENDDECL2(mg_landscape, "zscale,xyscale", "FF", "",
        "a simplex landscape of unit size");

    STARTDECL(mg_set_polygonreduction) (Value &_polyreductionpasses, Value &_epsilon, Value &_maxtricornerdot)
    {
        polyreductionpasses = _polyreductionpasses.ival();
        epsilon = _epsilon.fval();
        maxtricornerdot = _maxtricornerdot.fval();
        return Value();
    }
    ENDDECL3(mg_set_polygonreduction, "polyreductionpasses,epsilon,maxtricornerdot", "IFF", "",
        "controls the polygon reduction algorithm. set polyreductionpasses to 0 for off, 100 for max compression,"
        " or low values for generation speed or to keep the mesh uniform. epsilon determines how flat adjacent"
        " triangles must be to be reduced, use 0.98 as a good tradeoff, lower to get more compression. maxtricornerdot"
        " avoid very thin triangles, use 0.95 as a good tradeoff, up to 0.99 to get more compression");

    STARTDECL(mg_set_colornoise) (Value &_noiseintensity, Value &_noisestretch)
    {
        noisestretch = _noisestretch.fval();
        noiseintensity = _noiseintensity.fval();
        return Value();
    }
    ENDDECL2(mg_set_colornoise, "noiseintensity,noisestretch", "FF", "",
        "applies simplex noise to the colors of the model. try 0.3 for intensity."
        " stretch scales the pattern over the model");

    STARTDECL(mg_set_vertrandomize) (Value &factor)
    {
        randomizeverts = factor.fval();
        return Value();
    }
    ENDDECL1(mg_set_vertrandomize, "factor", "F", "",
        "randomizes all verts produced to give a more organic look and to hide the inherent messy polygons produced"
        " by the algorithm. try 0.15. note that any setting other than 0 will likely counteract the polygon"
        " reduction algorithm");

    STARTDECL(mg_set_pointmode) (Value &aspoints)
    {
        pointmode = aspoints.True();
        return Value();
    }
    ENDDECL1(mg_set_pointmode, "on", "I", "",
             "generates a point mesh instead of polygons");

    STARTDECL(mg_polygonize) (Value &subdiv, Value &color)
    {
        vector<float3> materials;
        for (int i = 0; i < color.eval()->Len(); i++) materials.push_back(ValueToF<3>(color.eval()->At(i)));
        color.DECRT();
        int mesh = polygonize_mc(root, subdiv.ival(), materials);
        MeshGenClear();
        return Value(mesh);
    }
    ENDDECL2(mg_polygonize, "subdiv,colors", "IF]]", "I",
        "returns a generated mesh (id 1..) from past mg_ commands."
        " subdiv determines detail and amount of polygons (relative to the largest dimension of the model),"
        " try 30.. 300 depending on the subject."
        " values much higher than that will likely make you run out of memory (or take very long)."
        " colors is a list of colors to be used with mg_fill()");

    STARTDECL(mg_translate) (Value &vec, Value &body)
    {
        if (body.True()) g_vm->Push(ToValueF(curorig));
        auto v = ValueDecToF<3>(vec);
        // FIXME: not good enough if non-uniform scale, might as well forbid that before any trans
        curorig += currot * (v * cursize);
        return body;
    }
    MIDDECL(mg_translate) ()
    {
        curorig = ValueDecToF<3>(g_vm->Pop());
    }
    ENDDECL2CONTEXIT(mg_translate, "vec,body", "F]C?", "",
        "translates the current coordinate system along a vector. when a body is given,"
        " restores the previous transform afterwards");

    STARTDECL(mg_scalevec) (Value &vec, Value &body)
    {
        if (body.True()) g_vm->Push(ToValueF(cursize));
        auto v = ValueDecToF<3>(vec);
        cursize *= v;
        return body;
    }
    MIDDECL(mg_scalevec) ()
    {
        cursize = ValueDecToF<3>(g_vm->Pop());
    }
    ENDDECL2CONTEXIT(mg_scalevec, "vec,body", "F]C?", "",
        "non-unimformly scales the current coordinate system using individual factors per axis. when a body is given,"
        " restores the previous transform afterwards");

    STARTDECL(mg_rotate) (Value &axis, Value &angle, Value &body)
    {
        if (body.True()) g_vm->Push(Value(g_vm->NewString((char *)&currot, sizeof(float3x3))));
        auto v = ValueDecToF<3>(axis);
        currot *= float3x3(angle.fval()*RAD, v);
        return body;
    }
    MIDDECL(mg_rotate) ()
    {
        auto s = g_vm->Pop();
        TYPE_ASSERT(s.type == V_STRING);
        assert(s.sval()->len == sizeof(float3x3));
        currot = *(float3x3 *)s.sval()->str();
        s.DECRT();
    }
    ENDDECL3CONTEXIT(mg_rotate, "axis,angle,body", "F]FC?", "",
        "rotates using axis/angle. when a body is given, restores the previous transform afterwards");

    STARTDECL(mg_fill) (Value &fill, Value &body)
    {
        if (body.True()) g_vm->Push(Value(curcol));
        curcol = fill.ival() & MATMASK;   // FIXME: error if doesn't fit?
        return body;
    }
    MIDDECL(mg_fill) ()
    {
        auto fill = g_vm->Pop();
        TYPE_ASSERT(fill.type == V_INT);
        curcol = fill.ival();
    }
    ENDDECL2CONTEXIT(mg_fill, "fill,body", "IC?", "",
        "sets the fill mode, where 1.. means to add shapes to the scene (union), and 0 substracts them (carves)."
        " when a body is given, restores the previous fill mode afterwards."
        " Values 1.. refer to colors 0.. of the colors passed to mg_polygonize");
}

AutoRegister __amg("meshgen", AddMeshGen);
