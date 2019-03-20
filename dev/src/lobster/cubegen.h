
const uchar transparant = 0;

struct Voxels {
    vector<byte4> palette;
    bool is_default_palette;
    Chunk3DGrid<uchar> grid;
    int idx = 0;

    Voxels(const int3 &dim) : is_default_palette(true), grid(dim, transparant) {}

    void Set(const int3 &p, const int3 &sz, uchar pi) {
        for (int x = max(0, p.x); x < min(p.x + sz.x, grid.dim.x); x++) {
            for (int y = max(0, p.y); y < min(p.y + sz.y, grid.dim.y); y++) {
                for (int z = max(0, p.z); z < min(p.z + sz.z, grid.dim.z); z++) {
                    grid.Get(int3(x, y, z)) = pi;
                }
            }
        }
    }

    void Copy(const int3 &p, const int3 &sz, const int3 &dest, const int3 &flip) {
        for (int x = max(0, p.x); x < min(p.x + sz.x, grid.dim.x); x++) {
            for (int y = max(0, p.y); y < min(p.y + sz.y, grid.dim.y); y++) {
                for (int z = max(0, p.z); z < min(p.z + sz.z, grid.dim.z); z++) {
                    auto pos = int3(x, y, z);
                    auto pi = grid.Get(pos);
                    auto d = (pos - p) * flip + dest;
                    if (d >= int3_0 && d < grid.dim) grid.Get(d) = pi;
                }
            }
        }
    }

    uchar Color2Palette(const float4 &color) const {
        uchar pi = transparant;
        if (color.w >= 0.5f) {
            if (is_default_palette) {  // Fast path.
                auto ic = byte4((int4(quantizec(color)) + (0x33 / 2)) / 0x33);
                pi = (5 - ic.x) * 36 +
                     (5 - ic.y) * 6 +
                     (5 - ic.z) + 1;
            } else {
                float error = 999999;
                for (size_t i = 1; i < palette.size(); i++) {
                    auto err = squaredlength(color2vec(palette[i]) - color);
                    if (err < error) { error = err; pi = (uchar)i; }
                }
            }
        }
        return pi;
    }
};

namespace lobster {

inline ResourceType *GetVoxelType() {
    static ResourceType voxel_type = { "voxels", [](void *v) { delete (Voxels *)v; } };
    return &voxel_type;
}

inline Voxels &GetVoxels(VM &vm, const Value &res) {
    return *GetResourceDec<Voxels *>(vm, res, GetVoxelType());
}

extern const unsigned int default_palette[256];

}

