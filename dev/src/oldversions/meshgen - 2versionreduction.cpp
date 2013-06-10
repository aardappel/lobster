#include "stdafx.h"

#include "openglsup.h"
#include "mctables.h"

#include "vmdata.h"
#include "natreg.h"

/* TODO:

- proper groups, so it can help in early culling

*/

struct ImplicitFunction;

const int maxedge = 256 * 256;

struct fcell
{
    int edgeidx[3];
	unsigned short edges[3];
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

INL int IndexGrid(const int3 &v, const int3 &gridsize)
{
    return v.x() * gridsize.y() * gridsize.z() +
           v.y() * gridsize.z() +
           v.z();
}

vec3 rotated_size(const mat3 &rot, const vec3 &size)
{
    mat3 absm = mat3(abs(rot[0]), abs(rot[1]), abs(rot[2]));
    return absm * size;
}

struct ImplicitFunction
{
    vec3 orig;
    vec3 size;
    mat3 rot;
    int material;
    
    virtual ~ImplicitFunction() {}

    INL bool Eval(const vec3 &pos) { return false; }

    virtual vec3 ComputeSize() { return size; };
    virtual void FillGrid(const int3 &start, const int3 &end, const int3 &gridsize, findex *fcellindices, vector<fcell> &fcells, const vec3 &gridscale, const vec3 &gridtrans, const mat3 &gridrot) = 0;
};

static int3 axesi[] = { int3(1, 0, 0), int3(0, 1, 0), int3(0, 0, 1) };
static vec3 axesf[] = { vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1) };

// use curiously recurring template pattern to allow implicit function to be inlined in rasterization loop

template<typename T> struct ImplicitFunctionImpl : ImplicitFunction
{
    void FillGrid(const int3 &start, const int3 &end, const int3 &gridsize, findex *fcellindices, vector<fcell> &fcells, const vec3 &gridscale, const vec3 &gridtrans, const mat3 &gridrot)
    {
        assert(end <= gridsize && int3() <= start);

        for (int x = start.x(); x < end.x(); x++) for (int y = start.y(); y < end.y(); y++) for (int z = start.z(); z < end.z(); z++)
        {
            int3 ipos(x, y, z);

            auto pos = vec3(ipos);
            pos -= gridtrans;
            pos = pos * gridrot;
            pos /= gridscale;
            bool inside = static_cast<T *>(this)->Eval(pos);

            assert(!inside || (ipos > start && ipos < end - 1));   // bounding box is supposed to cover the entire shape with 1 empty cell surrounding in all directions

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
                        if (inside || static_cast<T *>(this)->Eval(pos - (axesf[i] * gridrot) / gridscale)) // FIXME: factor this out, or make cell lookup
                        {
                            auto solidpos = vec3(tmat ? ipos : opos);
                            auto emptypos = vec3(tmat ? opos : ipos);
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
                                else                                                 p2f = p;
                            }

                            auto e = int(iso * maxedge);
                            assert(iso >= 0 && iso <= 1);
                            e = max(1, min(maxedge - 1, e));

                            if (!fi.geti())
                            {
                                fi.seti(fcells.size());
                                fcells.push_back(fcell());
                            }

                            fcell &fc = fcells[fi.geti()];

                            if (fc.edges[i])
                            {
                                int oe = tmat ? maxedge - fc.edges[i] : fc.edges[i];
                                e = material ? max(e, oe) : min(e, oe);
                            }
                            fc.edges[i] = tmat ? maxedge - e : e;
                            fc.material[i] = tmat ? tmat : omat;  // FIXME: make this depend on who is closer
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
    INL bool Eval(const vec3 &pos) { return dot(pos, pos) <= 1; }
};

struct IFCube : ImplicitFunctionImpl<IFCube>
{
    INL bool Eval(const vec3 &pos) { return abs(pos) <= 1; }
};

struct IFCylinder : ImplicitFunctionImpl<IFCylinder>
{
    INL bool Eval(const vec3 &pos) { return pos.z() <= 1 && pos.z() >= -1 && dot(pos.xy(), pos.xy()) <= 1; }
};

struct IFTaperedCylinder : ImplicitFunctionImpl<IFTaperedCylinder>
{
    float bot, top;

    INL bool Eval(const vec3 &pos) { auto xy = pos.xy(); auto r = mix(bot, top, pos.z() / 2 + 0.5f); return pos.z() <= 1 && pos.z() >= -1 && dot(xy, xy) <= r * r; }

    vec3 ComputeSize()
    {
        auto rad = max(top, bot);
        return size * vec3(rad, rad, 1);
    }
};


struct IFSuperQuadric : ImplicitFunctionImpl<IFSuperQuadric>
{
    vec3 exp;

    INL bool Eval(const vec3 &pos)
    {
        auto p = abs(pos);
        return powf(p.x(), exp.x()) + 
               powf(p.y(), exp.y()) + 
               powf(p.z(), exp.z()) <= 1; 
    }
};

struct IFSuperToroid : ImplicitFunctionImpl<IFSuperToroid>
{
    float r;
    vec3 exp;

    INL bool Eval(const vec3 &pos)
    {
        auto p = abs(pos);
        auto xy = r - sqrtf(powf(p.x(), exp.x()) + 
                            powf(p.y(), exp.y()));
        return powf(fabs(xy), exp.z()) + powf(p.z(), exp.z()) <= 1;
    }

    vec3 ComputeSize() { return size * vec3(r * 2 + 1, r * 2 + 1, 1); }
};

struct IFSuperQuadricNonUniform : ImplicitFunctionImpl<IFSuperQuadricNonUniform>
{
    vec3 exppos, expneg;
    vec3 scalepos, scaleneg;

    INL bool Eval(const vec3 &pos)
    {
        auto p = abs(vec3(pos.x() / (pos.x() < 0 ? scaleneg.x() : scalepos.x()),
                         pos.y() / (pos.y() < 0 ? scaleneg.y() : scalepos.y()),
                         pos.z() / (pos.z() < 0 ? scaleneg.z() : scalepos.z())));
        return powf(p.x(), (pos.x() < 0 ? expneg.x() : exppos.x())) + 
               powf(p.y(), (pos.y() < 0 ? expneg.y() : exppos.y())) + 
               powf(p.z(), (pos.z() < 0 ? expneg.z() : exppos.z())) <= 1; 
    }

    vec3 ComputeSize() { return size * max(scalepos, scaleneg); }
};


struct Group : ImplicitFunctionImpl<Group>
{
    vector<ImplicitFunction *> children;

    ~Group()
    {
        for (auto c : children) delete c;
    }

    static INL bool Eval(const vec3 &pos) { return false; }
    
    vec3 ComputeSize()
    { 
        vec3 p1, p2;
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

    void FillGrid(const int3 &start, const int3 &end, const int3 &gridsize, findex *fcellindices, vector<fcell> &fcells, const vec3 &gridscale, const vec3 &gridtrans, const mat3 &_gridrot)
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

int polygonize_mc(ImplicitFunction *root, const int targetgridsize, vector<vec3> &materials, float randomfactor, float noisestretch, float noiseintensity)
{    
    auto scenesize = root->ComputeSize() * 2;

    float biggestdim = max(scenesize.x(), max(scenesize.y(), scenesize.z()));

    auto gridscale = targetgridsize / biggestdim;

    auto gridsize = int3(scenesize * gridscale + vec3(2.5f));
    auto gridcount = gridsize.x() * gridsize.y() * gridsize.z();

    auto gridtrans = (vec3(gridsize) - 1) / 2 - root->orig * gridscale;

    auto fcellindices = new findex[gridcount];
    memset(fcellindices, 0, gridcount * sizeof(findex));

    vector<fcell> fcells;
    fcells.push_back(fcell());  // index 0 means no per cell data 

    /////////// SAMPLE GRID 

    root->FillGrid(int3(0), gridsize, gridsize, fcellindices, fcells, vec3(gridscale), gridtrans, mat3_1);

	/////////// APPLY MC

    struct edge
    {
        vec3 mid;
        int material;

        edge(const vec3 &_mid, int _m) : mid(_mid), material(_m) {}
    };

    vector<int> mctriangles;
    vector<edge> edges;

    for (int x = 0; x < gridsize.x() - 1; x++) for (int y = 0; y < gridsize.y() - 1; y++) for (int z = 0; z < gridsize.z() - 1; z++)
    {
        static int3 gridpos[8];
        static findex celli[8];

        int3 pos(x, y, z);

        int ci = 0;

        static int3 corners[8] =
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

        for (int i = 0; i < 8; i++)
        {
            gridpos[i] = pos + corners[i];
            celli[i] = fcellindices[IndexGrid(gridpos[i], gridsize)];

            ci |= (celli[i].getm() != 0) << i;
        }

        if (mc_edge_table[ci] == 0) continue;

        static int vertlist[12];

        for (int i = 0; i < 12; i++) if (mc_edge_table[ci] & (1<<i))
        {
            int i1 = mc_edge_to_vert[i][0];
            int i2 = mc_edge_to_vert[i][1];
            int3 &p1 = gridpos[i1];
            int3 &p2 = gridpos[i2];

            int cidx = celli[i2].geti();
            assert(cidx);

            int dir = p1.x() < p2.x() ? 0 : p1.y() < p2.y() ? 1 : 2;
            fcell &fc = fcells[cidx];
            int &idx = fc.edgeidx[dir];
            if (idx < 0)
            {
                idx = edges.size();

                auto e = fc.edges[dir];
                assert(e);
                auto mid = mix(vec3(p1), vec3(p2), float(e) / maxedge);

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

    delete[] fcellindices;

    struct vert
    {
        vec3 pos;
        vec3 norm;
        byte4 col;
    };

    vector<int> triangles;
    vector<vert> verts;

    const bool mesh_displacent = true;
    const bool flat_triangles_opt = true;
    const bool simple_occlusion = false;

    MTRnd r;

    if (mesh_displacent)
    {
        /////////// MESH DISPLACEMENT

        // "Mesh Displacement: An Improved Contouring Method for Trivariate Data"
        // http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.49.5214
        // reduces tris by half, and makes for more regular proportioned tris
        // from: http://chrishecker.com/My_liner_notes_for_spore

        struct dcell
        {
            vec3 accum;
            vec3 col;
            int n;

            dcell() : n(0), accum(vec3_0), col(vec3_0) {}
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
                idx = cells.size();
                cells.push_back(dcell());
            }

            dcell &c = cells[idx];
            c.accum += e.mid;
            c.col += materials[e.material];
            c.n--;
            iverts.push_back(&idx - dcellindices);
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
                        c.n = verts.size();
                        verts.push_back(vert());
                        vert &v = verts.back();
                        v.pos = c.accum;
                        v.norm = vec3_0;
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
            verts.push_back(vert());
            vert &v = verts.back();
            v.pos = e.mid;
            v.col = quantizec(materials[e.material]);
            v.norm = vec3_0;
        }

        triangles.assign(mctriangles.begin(), mctriangles.end());
    }

    /////////// CULL FLAT TRIANGLES

    if (flat_triangles_opt)
    {
        bool *problemvert = new bool[verts.size()];
        memset(problemvert, 0, sizeof(bool) * verts.size());

        const float threshold = -0.98f;

        for (size_t t = 0; t < triangles.size(); t += 3)
        {
            vert &v1 = verts[triangles[t + 0]];
            vert &v2 = verts[triangles[t + 1]];
            vert &v3 = verts[triangles[t + 2]];

            assert(v1.pos != v2.pos && v1.pos != v3.pos && v2.pos != v3.pos);

            vec3 v12 = normalize(v2.pos - v1.pos);  // FIXME: factor this out, used in 3 spots!
            vec3 v13 = normalize(v3.pos - v1.pos);
            vec3 d3  = normalize(cross(v13, v12));
        
            // if a plane normal points away near 180 from past normals, its likely part of triangle with no volume behind it (special case in mesh displacement)
            if (v1.norm != vec3_0 && dot(d3, normalize(v1.norm)) < threshold) problemvert[triangles[t + 0]] = true;
            if (v2.norm != vec3_0 && dot(d3, normalize(v2.norm)) < threshold) problemvert[triangles[t + 1]] = true;
            if (v3.norm != vec3_0 && dot(d3, normalize(v3.norm)) < threshold) problemvert[triangles[t + 2]] = true;
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

    randomfactor = 0;   // FIXME

    if (randomfactor > 0)
    {
        for (auto &v : verts)
        {
            v.pos += random_point_in_sphere(r) * randomfactor;
        }
    }

    /////////// ORIGIN/SCALE

    for (auto &v : verts)
    {
        v.pos -= gridtrans;
        v.pos /= gridscale;
    }

    /////////// CALCULATE NORMALS

    auto recompute_normals = [&]()
    {
        for (auto &v : verts)
        {
            v.norm = vec3_0;
        }

        for (size_t t = 0; t < triangles.size(); t += 3)
        {
            vert &v1 = verts[triangles[t + 0]];
            vert &v2 = verts[triangles[t + 1]];
            vert &v3 = verts[triangles[t + 2]];

            assert(v1.pos != v2.pos && v1.pos != v3.pos && v2.pos != v3.pos);

            vec3 v12 = normalize(v2.pos - v1.pos);
            vec3 v13 = normalize(v3.pos - v1.pos);
            vec3 v23 = normalize(v3.pos - v2.pos);
            vec3 d3  = normalize(cross(v13, v12));
                
            v1.norm += d3 * (1 - dot( v12, v13));
            v2.norm += d3 * (1 - dot(-v12, v23));
            v3.norm += d3 * (1 - dot(-v23,-v13));
        }

        for (auto &v : verts)
        {
            if (v.norm != vec3_0)  // mesh displacement can theoretically still generate bad normals
                v.norm = normalize(v.norm);
        }
    };

    recompute_normals();

    /////////// POLYGON REDUCTION

    if (false)
    {
        const float epsilon = 0.98f;
        const float maxtricornerdot = 0.95f;

        int *vertmap = new int[verts.size()];

        for (;;)
        {
            memset(vertmap, -1, verts.size() * sizeof(int));

            for (size_t t = 0; t < triangles.size(); t += 3)
            {
                vert &v1 = verts[triangles[t + 0]];
                vert &v2 = verts[triangles[t + 1]];
                vert &v3 = verts[triangles[t + 2]];

                vec3 v12u = v2.pos - v1.pos;
                vec3 v13u = v3.pos - v1.pos;
                vec3 v12 = normalize(v12u);
                vec3 v13 = normalize(v13u);
                vec3 d3  = normalize(cross(v13, v12));

                int idx[3];
                auto l12 = length(v12u);
                auto l13 = length(v13u);
                auto l23 = length(v2.pos - v3.pos);

                if (l13 < l12 && l13 < l23) // pick 13
                {
                    idx[0] = 0;
                    idx[1] = 2;
                    idx[2] = 1;
                }
                else if (l23 < l12 && l23 < l13)    // pick 23
                {
                    idx[0] = 1;
                    idx[1] = 2;
                    idx[2] = 0;
                }
                else   // pick 12
                {
                    idx[0] = 0;
                    idx[1] = 1;
                    idx[2] = 2;
                }

                for (int i = 0; i < 2; i++)
                {
                    int is = triangles[t + idx[i]];
                    vert &v = verts[is];
                    // FIXME: can also cancel any where < epsilon
                    if (dot(v.norm, d3) > epsilon      // can also be the case if face normals cancel eachother out, but this is rare (simplest case: 6 tris meeting, 2 opposite ones flat, the other 4 creased downwards along their 2 shared edges)
                        && vertmap[is] == -1)
                    {
                        int ia = idx[(i + 1) % 3];
                        int ib = idx[(i + 2) % 3];
                        int via = triangles[t + ia];
                        int vib = triangles[t + ib];
                        vert &va = verts[via];
                        vert &vb = verts[vib];
                        auto target = length(va.pos - v.pos) < length(vb.pos - v.pos) && vertmap[via] == -1 && vertmap[vib] < 0 ? via : (vertmap[vib] == -1 && vertmap[via] < 0 ? vib : -1);
                        if (target >= 0)
                        {
                            vertmap[is] = target;
                            vertmap[target] = -2;

                            //verts[target].col = quantizec(vec3(1, 0, 0));

                            // FIXME: average verts for better quality
                            // can't average unless target vert also within epsilon
                            //verts[target].pos = (verts[target].pos + v1.pos) / 2;
                            // FIXME: same for color

                            break;
                        }
                    }
                }
            }

            int flipped = 0;
            for (size_t t = 0; t < triangles.size(); t += 3)
            {
                for (int i = 0; i < 3; i++)
                {
                    int vi1 = triangles[t + i];
                    int vi2 = triangles[t + (i + 1) % 3];
                    int vi3 = triangles[t + (i + 2) % 3];
                    if (vertmap[vi1] >= 0 && vertmap[vi1] != vi2 && vertmap[vi1] != vi3)
                    {
                        vert &v1 = verts[vi1];
                        vert &v2 = verts[vi2];
                        vert &v3 = verts[vi3];

                        vec3 v12 = normalize(v2.pos - v1.pos);
                        vec3 v13 = normalize(v3.pos - v1.pos);
                        vec3 d3  = normalize(cross(v13, v12));
                        
                        vert &vm = verts[vertmap[vi1]];
                        vec3 v12m = normalize(v2.pos - vm.pos);
                        vec3 v13m = normalize(v3.pos - vm.pos);
                        vec3 v23m = normalize(v3.pos - v2.pos);
                        vec3 d3m  = normalize(cross(v13m, v12m));

                        if (dot(d3, d3m) < epsilon ||
                            dot(v12m, v13m) > maxtricornerdot || 
                            dot(-v12m, v23m) > maxtricornerdot || 
                            dot(-v23m, -v13m) > maxtricornerdot)
                        {
                            //vm.col = quantizec(vec3(1, 0, 1));
                            vertmap[vi1] = -4;
                            flipped++;
                        }
                    }
                }
            }
            printf("flipped tris: %d\n", flipped);

            for (size_t t = 0; t < triangles.size(); t += 3)
            {
                int keep = -1;
                for (int i = 0; i < 3; i++) if (vertmap[triangles[t + i]] >= 0)
                {
                    if (keep >= 0)
                    { 
                        if (length(verts[triangles[t + i   ]].pos - verts[vertmap[triangles[t + i   ]]].pos) <
                            length(verts[triangles[t + keep]].pos - verts[vertmap[triangles[t + keep]]].pos))
                        {
                            vertmap[triangles[t + keep]] = -3;
                            keep = i;
                        }
                        else
                        {
                            vertmap[triangles[t + i]] = -3; 
                        }

                    
                        //verts[triangles[t + i]].col = quantizec(vec3(1, 1, 0));
                    }
                    else keep = i;
                }
            }

            int polysreduced = 0;
            for (size_t t = 0; t < triangles.size(); t += 3)
            {
                for (int i = 0; i < 3; i++)
                {
                    int target = vertmap[triangles[t + i]];
                    if (target >= 0)
                    {
                        if (triangles[t + (i + 1) % 3] == target || triangles[t + (i + 2) % 3] == target)
                        {
                            triangles.erase(triangles.begin() + t, triangles.begin() + t + 3);      // FIXME this can't be fast
                            t -= 3;  
                            polysreduced++;          
                            break;
                        }
                    }
                }
            }
            printf("reduced tris: %d\n", polysreduced);

            for (size_t t = 0; t < triangles.size(); t++)
            {
                if (vertmap[triangles[t]] >= 0)
                    triangles[t] = vertmap[triangles[t]];
            }
            /*
            for (size_t i = 0; i < verts.size(); i++) if (vertmap[i] >= 0)
            {
                vert &v1 = verts[i];
                vert &v2 = verts[vertmap[i]];

                if (dot(v1.norm, v2.norm) > 1 - epsilon)
                {
                    v2.pos = (v1.pos + v2.pos) / 2;
                    // FIXME color
                }
            }   
            */         

            recompute_normals();

            if (polysreduced < 100)
                break;

        }

        // FIXME: delete verts also, and the ones from bad tri culling
        // can bad tris simply be culled based on bad normals?

        delete[] vertmap;
    }

    if (true)
    {
        const float epsilon = 0.98f;
        const float maxtricornerdot = 0.95f;

        int *vertmap = new int[verts.size()];

        for (;;)
        {
            memset(vertmap, -1, verts.size() * sizeof(int));

            for (size_t t = 0; t < triangles.size(); t += 3)
            {
                vert &v1 = verts[triangles[t + 0]];
                vert &v2 = verts[triangles[t + 1]];
                vert &v3 = verts[triangles[t + 2]];

                vec3 v12u = v2.pos - v1.pos;
                vec3 v13u = v3.pos - v1.pos;
                vec3 v12 = normalize(v12u);
                vec3 v13 = normalize(v13u);
                vec3 d3  = normalize(cross(v13, v12));
                
                int i1 = -1, i2 = -1, i3 = -1;
                for (int i = 0; i < 3; i++)
                {
                    // FIXME: can also cancel any where < epsilon
                    if (dot(verts[triangles[t + i]].norm, d3) > epsilon && vertmap[triangles[t + i]] < 0)
                    {
                        if (i2 < 0)
                        {
                            if (i1 >= 0) i2 = i;
                            else i1 = i;
                        }
                    }
                    else i3 = i;
                }

                if (i3 < 0)     // all 3 flat, pick the shortest edge
                {
                    auto l12 = length(v12u);
                    auto l13 = length(v13u);
                    auto l23 = length(v2.pos - v3.pos);

                    if (l13 < l12 && l13 < l23) // pick 13
                    {
                        i2 = 2;   
                        i3 = 1;
                    }
                    else if (l23 < l12 && l23 < l13)    // pick 23
                    {
                        i1 = 1;
                        i2 = 2;
                        i3 = 0;
                    }
                    else   // pick 12
                    {
                        i3 = 2;
                    }
                }

                if (i1 >= 0 && i2 >= 0
                && vertmap[triangles[t + i3]] < 0  // FIXME: wtf is this so important???
                )  // can also be the case if face normals cancel eachother out, but this is rare (simplest case: 6 tris meeting, 2 opposite ones flat, the other 4 creased downwards along their 2 shared edges)
                {
                    int vi = triangles[t + i1];
                    int ovi = triangles[t + i2];
                    vertmap[vi] = ovi;
                    vertmap[ovi] = vi;
                }
            }

            int flipped = 0;
            for (size_t t = 0; t < triangles.size(); t += 3)
            {
                for (int i = 0; i < 3; i++)
                {
                    int vi1 = triangles[t + i];
                    int vi2 = triangles[t + (i + 1) % 3];
                    int vi3 = triangles[t + (i + 2) % 3];
                    if (vertmap[vi1] >= 0 && vertmap[vi1] != vi2 && vertmap[vi1] != vi3)
                    {
                        vert &v1 = verts[vi1];
                        vert &v2 = verts[vi2];
                        vert &v3 = verts[vi3];

                        vec3 v12 = normalize(v2.pos - v1.pos);
                        vec3 v13 = normalize(v3.pos - v1.pos);
                        vec3 d3  = normalize(cross(v13, v12));
                        
                        vec3 &vm = (verts[vertmap[vi1]].pos + v1.pos) / 2;
                        vec3 v12m = normalize(v2.pos - vm);
                        vec3 v13m = normalize(v3.pos - vm);
                        vec3 v23m = normalize(v3.pos - v2.pos);
                        vec3 d3m  = normalize(cross(v13m, v12m));

                        if (dot(d3, d3m) < epsilon ||
                            dot(v12m, v13m) > maxtricornerdot || 
                            dot(-v12m, v23m) > maxtricornerdot || 
                            dot(-v23m, -v13m) > maxtricornerdot)
                        {
                            //vm.col = quantizec(vec3(1, 0, 1));
                            vertmap[vertmap[vi1]] = -1;
                            vertmap[vi1] = -1;
                            flipped++;
                        }
                    }
                }
            }
            printf("flipped tris: %d\n", flipped);

            for (size_t t = 0; t < triangles.size(); t += 3)
            {
                int keep = -1;                
                for (int i = 0; i < 3; i++)
                {
                    int vi = triangles[t + i];
                    if (vertmap[vi] >= 0)
                    {
                        if (keep >= 0)
                        { 
                            int kvi = triangles[t + keep];
                            if (vertmap[vi] != kvi)
                            {
                                if (length(verts[ vi].pos - verts[vertmap[ vi]].pos) <
                                    length(verts[kvi].pos - verts[vertmap[kvi]].pos))
                                {
                                    vertmap[vertmap[kvi]] = -1;
                                    vertmap[kvi] = -1;
                                    keep = i;
                                }
                                else
                                {
                                    vertmap[vertmap[vi]] = -1;
                                    vertmap[vi] = -1; 
                                }
                            }
                        }
                        else keep = i;
                    }
                }
            }

            size_t writep = 0;
            for (size_t t = 0; t < triangles.size(); t += 3)
            {
                for (int i = 0; i < 3; i++)
                {
                    int target = vertmap[triangles[t + i]];
                    if (target >= 0)
                    {
                        if (triangles[t + (i + 1) % 3] == target || triangles[t + (i + 2) % 3] == target)
                        {
                            writep -= i;
                            break;
                        }
                    }

                    triangles[writep++] = triangles[t + i];
                }
            }
            int polysreduced = (triangles.size() - writep) / 3;
            printf("reduced tris: %d\n", polysreduced);
            triangles.erase(triangles.begin() + writep, triangles.end());

            for (size_t t = 0; t < triangles.size(); t++)
            {
                if (vertmap[triangles[t]] >= 0 && vertmap[triangles[t]] < triangles[t])
                    triangles[t] = vertmap[triangles[t]];
            }
            
            for (size_t i = 0; i < verts.size(); i++) if (vertmap[i] >= 0 && vertmap[i] < (int)i)
            {
                vert &v1 = verts[i];
                vert &v2 = verts[vertmap[i]];

                v2.pos = (v1.pos + v2.pos) / 2;
                // FIXME color
            }           

            recompute_normals();

            if (polysreduced < 100)
                break;
        }

        // FIXME: delete verts also, and the ones from bad tri culling
        // can bad tris simply be culled based on bad normals?

        delete[] vertmap;
    }


    /////////// APPLY NOISE TO COLOR

    if (noiseintensity > 0)
    {
        extern float simplexNoise(const int octaves, const float persistence, const float scale, const vec4 &v);

        float scale = noisestretch;
        int octaves = 8;
        float persistence = 0.5f;

        for (auto &v : verts)
        {
            auto n = vec3(simplexNoise(octaves, persistence, scale, vec4(v.pos, 0.0f / scale)),
                          simplexNoise(octaves, persistence, scale, vec4(v.pos, 0.3f / scale)),
                          simplexNoise(octaves, persistence, scale, vec4(v.pos, 0.6f / scale)));
            v.col = quantizec(color2vec(v.col).xyz() * (vec3_1 - (n + vec3_1) / 2 * noiseintensity));
        }
    }

    /////////// MODULATE LIGHTING BY CREASE FACTOR

    if (simple_occlusion)
    {
        float *cfactor = new float[verts.size()];
        memset(cfactor, 0, sizeof(float) * verts.size());

        for (size_t t = 0; t < triangles.size(); t += 3)
        {
            vert &v1 = verts[triangles[t + 0]];
            vert &v2 = verts[triangles[t + 1]];
            vert &v3 = verts[triangles[t + 2]];

            vec3 v12 = normalize(v2.pos - v1.pos);
            vec3 v13 = normalize(v3.pos - v1.pos);
            vec3 v23 = normalize(v3.pos - v2.pos);

            auto centroid = (v1.pos + v2.pos + v3.pos) / 3;
            cfactor[triangles[t + 0]] += dot(v1.norm, normalize(centroid - v1.pos)) * (1 - dot( v12, v13));
            cfactor[triangles[t + 1]] += dot(v1.norm, normalize(centroid - v1.pos)) * (1 - dot(-v12, v23));
            cfactor[triangles[t + 2]] += dot(v1.norm, normalize(centroid - v1.pos)) * (1 - dot(-v23,-v13));
        }

        for (auto &v : verts)
        {
            float f = cfactor[&v - &verts[0]] + 1;
            v.col = byte4(vec4(min(vec3(255), max(vec3_0, vec3(v.col.xyz()) - 64 * f)), 255));
        }

        delete[] cfactor;
    }

    printf("verts = %lu, edgeverts = %lu, tris = %lu, mctris = %lu, fcells = %lu, GS = %d\n", verts.size(), edges.size(), triangles.size()/3, mctriangles.size()/3, fcells.size(), targetgridsize);

    if (verts.empty())
        return -1;

    extern IntResourceManager<Mesh> *meshes;
    return meshes->Add(new Mesh(new Geometry(&verts[0], verts.size(), sizeof(vert), "PNC"), new Surface(&triangles[0], triangles.size())));
}


Group *root = NULL;
Group *curgroup = NULL;

vec3 cursize = vec3_1;
vec3 curorig = vec3_0;
mat3 currot  = mat3_1;
int curcol   = 1;

void MeshGenClear()
{
    if (root) delete root;
    root = curgroup = NULL;

    cursize = vec3_1;
    curorig = vec3_0;
    currot  = mat3_1;
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
    STARTDECL(mg_sphere)   () { return AddShape(new IFSphere());   } ENDDECL0(mg_sphere,   "", "", "");
    STARTDECL(mg_cube)     () { return AddShape(new IFCube());     } ENDDECL0(mg_cube,     "", "", "");
    STARTDECL(mg_cylinder) () { return AddShape(new IFCylinder()); } ENDDECL0(mg_cylinder, "", "", "");

    STARTDECL(mg_tapered_cylinder) (Value &bot, Value &top)
    {
        auto tc = new IFTaperedCylinder();
        tc->bot = bot.fval;
        tc->top = top.fval;
        return AddShape(tc);
    } ENDDECL2(mg_tapered_cylinder, "bot/0top", "FF", "");

    STARTDECL(mg_superquadric) (Value &exps)
    {
        auto sq = new IFSuperQuadric();
        sq->exp = _val_to_vec3_dec(exps);
        return AddShape(sq);
    } ENDDECL1(mg_superquadric, "exponents", "V", "");

    STARTDECL(mg_supertoroid) (Value &r, Value &exps)
    {
        auto t = new IFSuperToroid();
        t->r = r.fval;
        t->exp = _val_to_vec3_dec(exps);
        return AddShape(t);
    } ENDDECL2(mg_supertoroid, "R/0exponents", "FV", "");

    STARTDECL(mg_superquadric_non_uniform) (Value &posexps, Value &negexps, Value &posscale, Value &negscale)
    {
        auto sq = new IFSuperQuadricNonUniform();
        sq->exppos   = _val_to_vec3_dec(posexps);
        sq->expneg   = _val_to_vec3_dec(negexps);
        sq->scalepos = max(vec3(0.01f), _val_to_vec3_dec(posscale));
        sq->scaleneg = max(vec3(0.01f), _val_to_vec3_dec(negscale));
        
        return AddShape(sq);
    } ENDDECL4(mg_superquadric_non_uniform, "posexponents/0negexponents/0posscale/0negscale", "VVVV", "");

    STARTDECL(mg_polygonize) (Value &subdiv, Value &color, Value &noisestretch, Value &noiseintensity, Value &randomizeverts)
    {
        vector<vec3> materials;
        for (int i = 0; i < color.vval->len; i++) materials.push_back(_val_to_vec3(color.vval->at(i)));
        color.DECRT();
        int mesh = polygonize_mc(root, subdiv.ival, materials, randomizeverts.fval, noisestretch.fval, noiseintensity.fval);
        MeshGenClear();
        return Value(mesh);
    }
    ENDDECL5(mg_polygonize, "subdiv\0color\0noisestretch\0noiseintensity\0randomizeverts", "IVFFF", "");

    STARTDECL(mg_translate) (Value &vec, Value &body)
    {
        if (body.type != V_NIL) g_vm->Push(Value(_vec_to_val3d(curorig)));
        auto v = _val_to_vec3_dec(vec);
        curorig += currot * (v * cursize);       // FIXME: not good enough if non-uniform scale, might as well forbid that before any trans
        return body;
    }
    MIDDECL(mg_translate) (Value &ret)
    {
        curorig = _val_to_vec3_dec(g_vm->Pop());
        return ret;
    }
    ENDDECL2CONTEXIT(mg_translate, "vec\0body", "Vc", "translates the current coordinate system along a vector. when a body is given, restores the previous transform afterwards");

    STARTDECL(mg_scalevec) (Value &vec, Value &body)
    {
        if (body.type != V_NIL) g_vm->Push(Value(_vec_to_val3d(cursize)));
        auto v = _val_to_vec3_dec(vec);
        cursize *= v;
        return body;
    }
    MIDDECL(mg_scalevec) (Value &ret)
    {
        cursize = _val_to_vec3_dec(g_vm->Pop());
        return ret;
    }
    ENDDECL2CONTEXIT(mg_scalevec, "vec\0body", "Vc", "non-unimformly scales the current coordinate system using individual factors per axis. when a body is given, restores the previous transform afterwards");

    STARTDECL(mg_rotate) (Value &axis, Value &angle, Value &body)
    {
        if (body.type != V_NIL) g_vm->Push(Value(g_vm->NewString((char *)&currot, sizeof(mat3))));
        auto v = _val_to_vec3_dec(axis);
        currot *= mat3(angle.fval*RAD, v);
        return body;
    }
    MIDDECL(mg_rotate) (Value &ret)
    {
        auto s = g_vm->Pop();
        assert(s.type == V_STRING && s.sval->len == sizeof(mat3));
        currot = *(mat3 *)s.sval->str();
        s.DECRT();
        return ret;
    }
    ENDDECL3CONTEXIT(mg_rotate, "axis\0angle\0body", "VFc", "rotates using axis/angle. when a body is given, restores the previous transform afterwards");

    STARTDECL(mg_fill) (Value &fill, Value &body)
    {
        if (body.type != V_NIL) g_vm->Push(Value(curcol));
        curcol = fill.ival & MATMASK;   // FIXME: error if doesn't fit?
        return body;
    }
    MIDDECL(mg_fill) (Value &ret)
    {
        auto fill = g_vm->Pop();
        assert(fill.type == V_INT);
        curcol = fill.ival;
        return ret;
    }
    ENDDECL2CONTEXIT(mg_fill, "fill\0body", "Ic", "sets the fill mode, where 1.. means to add shapes to the scene (union), and 0 substracts them (carves). when a body is given, restores the previous fill mode afterwards");
};
