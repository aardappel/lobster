#include "stdafx.h"

#include "openglsup.h"
#include "mctables.h"

struct vert
{
    vec3 pos;
    vec3 norm;
    unsigned char col[4];
};

int3 corners[8] =
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

struct ImplicitFunction;

struct fcell
{
    ImplicitFunction *last;
	unsigned char edges[3];
	bool solid;
};

INL int Sample(const int3 &v, const int3 &gridsize)
{
    return v.x * gridsize.y * gridsize.z +
           v.y * gridsize.z +
           v.z;
}

vec3 gridscale;
vec3 gridtrans;

struct ImplicitFunction
{
    vec3 orig;
    vec3 size;
    vec3 color;
    bool fill;

    virtual void ComputeSize() {};
    virtual void FillGrid(const int3 &start, const int3 &end, const int3 &gridsize, fcell *fcells) = 0;
};

static int3 axes[] = { int3(1, 0, 0), int3(0, 1, 0), int3(0, 0, 1) };

template<typename T> struct ImplicitFunctionImpl : ImplicitFunction
{
    void FillGrid(const int3 &start, const int3 &end, const int3 &gridsize, fcell *fcells)
    {
        assert(end <= gridsize && int3() <= start);

        for (int x = start.x; x < end.x; x++) for (int y = start.y; y < end.y; y++) for (int z = start.z; z < end.z; z++)
        {
            int3 ipos(x, y, z);
            vec3 pos(ipos.tovec3());
            pos -= gridtrans;
            pos /= gridscale;

            fcell &fc = fcells[Sample(ipos, gridsize)];
			if (T::Eval(pos))
            {
                fc.solid = fill;
                fc.last = this;
            }
            for (int i = 0; i < 3; i++)
            {
                int3 opos = ipos - axes[i];
                if (opos[i] >= 0)
                {
                    fcell &fo = fcells[Sample(opos, gridsize)];
                    if (fo.solid != fc.solid)
                    {
                        if (fo.last == this || fc.last == this)
                        {
                            auto solidpos = fc.solid ? ipos.tovec3() : opos.tovec3();
                            auto emptypos = fc.solid ? opos.tovec3() : ipos.tovec3();
                            auto p1f = solidpos;
                            auto p2f = emptypos;
                            float iso;
                            for (int j = 0; ; j++)
                            {
                                auto p = (p1f + p2f) / 2;
                                if (j == 10) { iso = dot(p - solidpos, emptypos - solidpos); break; }

                                auto p2 = p;
                                p2 -= gridtrans;
                                p2 /= gridscale;

                                if (T::Eval(p2) == fill) p1f = p;
                                else                     p2f = p;
                            }
                            auto e = int(iso * 256);
                            assert(iso >= 0 && iso <= 1);
                            e = max(1, min(255, e));
                            if (fc.edges[i])
                            {
                                int oe = fc.solid ? 256 - fc.edges[i] : fc.edges[i];
                                e = fill ? max(e, oe) : min(e, oe);
                            }
                            fc.edges[i] = fc.solid ? 256 - e : e;
                        }
                    }
                    else
                    {
                        fc.edges[i] = 0;
                    }
                }
            }
        }
    }
};



struct Sphere : ImplicitFunctionImpl<Sphere>
{
    static INL bool Eval(const vec3 &pos) { return dot(pos, pos) < 1.0f; }
};

struct Group : ImplicitFunctionImpl<Group>
{
    vector<ImplicitFunction *> children;

    static INL bool Eval(const vec3 &pos) { return false; }
    
    void ComputeSize()
    { 
        vec3 p1, p2;
        for (auto c : children)
        {
            c->ComputeSize();
            p1 = c == children[0] ? c->orig - c->size : min(p1, c->orig - c->size);
            p2 = c == children[0] ? c->orig + c->size : max(p2, c->orig + c->size);
        }
        size = (p2 - p1) / 2;
        orig = p1 + size;
    }

    void FillGrid(const int3 &start, const int3 &end, const int3 &gridsize, fcell *fcells)
    {
        auto scale = gridscale;
        auto trans = gridtrans;

        for (auto c : children)
        {
            gridtrans = trans + c->orig * scale;
            gridscale = scale * c->size;
            auto start = int3(gridtrans - c->size * scale - vec3(0.1f));
            auto end   = int3(gridtrans + c->size * scale + vec3(2.1f));
            auto bs    = end - start;
            if (bs.x && bs.y && bs.z) c->FillGrid(start, end, gridsize, fcells);
        }

        gridtrans = trans;
        gridscale = scale;
    }
};

int polygonize_mc()
{
    Sphere sp;
    sp.orig = vec3(0);
    sp.size = vec3(1);
    sp.fill = true;
    sp.color = vec3(1, 0, 0);
	
    Sphere sp2;
    sp2.orig = vec3(0.5f);
    sp2.size = vec3(0.5f, 2, 0.5f); 
    sp2.fill = true; 
    sp2.color = vec3(1, 1, 0);

    Sphere sp3;
    sp3.orig = vec3(-0.5f, 0, 0);
    sp3.size = vec3(0.5f); 
    sp3.fill = false; 

    Group grp;
    grp.children.push_back(&sp);
    grp.children.push_back(&sp2);
    grp.children.push_back(&sp3);
	
    ImplicitFunction *root = &grp;
    
    root->ComputeSize();
    auto scenesize = root->size * 2;

    float biggestdim = max(scenesize.x, max(scenesize.y, scenesize.z));

    const int targetgridsize = 32;

    gridscale = vec3(targetgridsize / biggestdim);

    auto gridsize = int3(scenesize * gridscale + vec3(2.5f));
    auto gridcount = gridsize.x * gridsize.y * gridsize.z;

    gridtrans = (gridsize.tovec3() - vec3(1)) / 2 - root->orig * gridscale;

    auto fcells = new fcell[gridcount];
    memset(fcells, 0, gridcount * sizeof(fcell));

    /////////// SAMPLE GRID 

    root->FillGrid(int3(0, 0, 0), gridsize, gridsize, fcells);

	/////////// APPLY MC

    struct edge
    {
        vec3 mid, col;

        edge() {}
        edge(const vec3 &_mid, const vec3 &_col) : mid(_mid), col(_col) {}
    };

    int3 *edgehashes = new int3[gridcount];
    memset(edgehashes, -1, gridcount * sizeof(int3));

    vector<int> triangles;
    vector<edge> edges;

    for (int x = 0; x < gridsize.x - 1; x++) for (int y = 0; y < gridsize.y - 1; y++) for (int z = 0; z < gridsize.z - 1; z++)
    {
        static int3 gridpos[8];
        static fcell fc[8];

        int3 pos(x, y, z);

        int ci = 0;

        for (int i = 0; i < 8; i++)
        {
            gridpos[i] = pos + corners[i];
            fc[i] = fcells[Sample(gridpos[i], gridsize)];

            ci |= (fc[i].solid != false) << i;
        }

        if (mc_edge_table[ci] == 0) continue;

        static int vertlist[12];

        for (int i = 0; i < 12; i++) if (mc_edge_table[ci] & (1<<i))
        {
            int i1 = mc_edge_to_vert[i][0];
            int i2 = mc_edge_to_vert[i][1];
            int3 &p1 = gridpos[i1];
            int3 &p2 = gridpos[i2];
            int dir = p1.x < p2.x ? 0 : p1.y < p2.y ? 1 : 2;
            int &idx = *((&edgehashes[Sample(p1, gridsize)].x) + dir);
            if (idx < 0)
            {
                idx = edges.size();
                auto e = fc[i2].edges[dir];
                assert(e);
                auto mid = mix(p1.tovec3(), p2.tovec3(), e / 256.0f);
                auto col = fc[fc[i1].last && fc[i1].last->fill ? i1 : i2].last->color;  // FIXME: keeping track of color per edge would be more accurate
                assert(check(col));
                edges.push_back(edge(mid, col));
            }
            vertlist[i] = idx;
        }

        for (int i = 0; mc_tri_table[ci][i] != -1; i++)
        {
            triangles.push_back(vertlist[mc_tri_table[ci][i]]);
        }
    }

    delete[] fcells;
    delete[] edgehashes;

    vector<int> triangles2;
    vector<vert> verts;

    const bool mesh_displacent = true;
    const bool flat_triangles_opt = true;
    const bool simple_occlusion = false;

    if (mesh_displacent)
    {
        /////////// MESH DISPLACEMENT

        // "Mesh Displacement: An Improved Contouring Method for Trivariate Data"
        // http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.49.5214
        // reduces tris by half, and makes for more regular proportioned tris
        // from: http://chrishecker.com/My_liner_notes_for_spore

        struct cell
        {
            vec3 accum;
            vec3 col;
            int n;

            cell() : n(0), accum(0), col(0) {}
        };

        int *idxs = new int[gridcount];
        memset(idxs, -1, gridcount * sizeof(int));

        vector<int> iverts;
        vector<cell> cells;

        for (edge &e : edges)
        {
            int3 ipos(e.mid + vec3(0.5f));
            int &idx = idxs[Sample(ipos, gridsize)];

            if (idx < 0)
            {
                idx = cells.size();
                cells.push_back(cell());
            }

            cell &c = cells[idx];
            c.accum += e.mid;
            c.n--;
            c.col += e.col;
            iverts.push_back(&idx - idxs);
        }

        for (size_t t = 0; t < triangles.size(); t += 3)
        {
            int i[3];
            i[0] = iverts[triangles[t + 0]];
            i[1] = iverts[triangles[t + 1]];
            i[2] = iverts[triangles[t + 2]];

            if (i[0] != i[1] && i[0] != i[2] && i[1] != i[2])
            {
                for (int j = 0; j < 3; j++)
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
                        quantizec(c.col, v.col, 1.0f);
                    }

                    triangles2.push_back(c.n);
                }
            }
        }

        delete[] idxs;
    }
    else
    {
        for (edge &e : edges)
        {
            verts.push_back(vert());
            vert &v = verts.back();
            v.pos = e.mid;
            quantizec(e.col, v.col, 1.0f);
            v.norm = vec3(0);
        }

        triangles2.assign(triangles.begin(), triangles.end());
    }

    /////////// CALCULATE NORMALS & CULL FLAT TRIANGLES

    bool *problemvert = new bool[verts.size()];
    memset(problemvert, 0, sizeof(bool) * verts.size());

    for (size_t t = 0; t < triangles2.size(); t += 3)
    {
        vert &v1 = verts[triangles2[t + 0]];
        vert &v2 = verts[triangles2[t + 1]];
        vert &v3 = verts[triangles2[t + 2]];

        assert(v1.pos != v2.pos && v1.pos != v3.pos && v2.pos != v3.pos);

        vec3 v12 = normalize(v2.pos - v1.pos);
        vec3 v13 = normalize(v3.pos - v1.pos);
        vec3 v23 = normalize(v3.pos - v2.pos);
        vec3 d3  = normalize(cross(v13, v12));
        
        if (flat_triangles_opt)
        {
            const float threshold = -0.98f;
            // if a plane normal points away near 180 from past normals, its likely part of triangle with no volume behind it (special case in mesh displacement)
            if (v1.norm != vec3(0) && dot(d3, normalize(v1.norm)) < threshold) problemvert[triangles2[t + 0]] = true;
            if (v2.norm != vec3(0) && dot(d3, normalize(v2.norm)) < threshold) problemvert[triangles2[t + 1]] = true;
            if (v3.norm != vec3(0) && dot(d3, normalize(v3.norm)) < threshold) problemvert[triangles2[t + 2]] = true;
        }
        
        v1.norm += d3 * (1 - dot( v12, v13));
        v2.norm += d3 * (1 - dot(-v12, v23));
        v3.norm += d3 * (1 - dot(-v23,-v13));
    }

    if (flat_triangles_opt)
    {
        for (size_t t = 0; t < triangles2.size(); t += 3)
        {
            // if all corners have screwey normals, the triangle should be culled
            // could also cull associated verts, but generally so few of them its not worth it
            if ((problemvert[triangles2[t + 0]] && problemvert[triangles2[t + 1]] && problemvert[triangles2[t + 2]]))
            {
                triangles2.erase(triangles2.begin() + t, triangles2.begin() + t + 3);
                t -= 3;            
            }
        }
    }

    delete[] problemvert;

    for (auto &v : verts)
    {
        if (v.norm != vec3(0))  // mesh displacement can generate bad normals
            v.norm.normalize();
    }

    /////////// MODULATE LIGHTING BY CREASE FACTOR

    if (simple_occlusion)
    {
        float *cfactor = new float[verts.size()];
        memset(cfactor, 0, sizeof(float) * verts.size());

        for (size_t t = 0; t < triangles2.size(); t += 3)
        {
            vert &v1 = verts[triangles2[t + 0]];
            vert &v2 = verts[triangles2[t + 1]];
            vert &v3 = verts[triangles2[t + 2]];

            vec3 v12 = normalize(v2.pos - v1.pos);
            vec3 v13 = normalize(v3.pos - v1.pos);
            vec3 v23 = normalize(v3.pos - v2.pos);

            auto centroid = (v1.pos + v2.pos + v3.pos) / 3;
            cfactor[triangles2[t + 0]] += dot(v1.norm, normalize(centroid - v1.pos)) * (1 - dot( v12, v13));
            cfactor[triangles2[t + 1]] += dot(v1.norm, normalize(centroid - v1.pos)) * (1 - dot(-v12, v23));
            cfactor[triangles2[t + 2]] += dot(v1.norm, normalize(centroid - v1.pos)) * (1 - dot(-v23,-v13));
        }

        for (auto &v : verts)
        {
            float f = cfactor[&v - &verts[0]] + 1;
            for (int i = 0; i < 3; i++) v.col[i] = min(255, max(0, int(v.col[i] - 64 * f)));
        }

        delete[] cfactor;
    }

    printf("verts = %lu, tris = %lu, GS = %d\n", verts.size(), triangles2.size()/3, targetgridsize);

    extern IntResourceManager<Mesh> *meshes;
    return meshes->Add(new Mesh(new Geometry(&verts[0], verts.size(), sizeof(vert), "PNC"), new Surface(&triangles2[0], triangles2.size())));
}

