
struct mgvert
{
    float3 pos;
    float3 norm;
    byte4 col;
};

inline void RecomputeNormals(vector<int> &triangles, vector<mgvert> &verts)
{
    normalize_mesh(&triangles[0], triangles.size(), &verts[0], verts.size(), false);
};
