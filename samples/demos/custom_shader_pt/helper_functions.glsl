
const uint DIFF = 0;
const uint SPEC = 1;
const uint REFR = 2;

struct Ray {
    vec3 o;
    vec3 d;
};

struct Sphere {
    float rad;   // radius
    vec3 p;     // position
    vec3 e;     // emission
    vec3 c;     // color
    uint refl; // Material
};

// Wang Hash Function
uint wang_hash(uint seed)
{
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9u;
    seed = seed ^ (seed >> 4u);
    seed *= 0x27d4eb2du;
    seed = seed ^ (seed >> 15u);
    return seed;
}

// Function to initialize the random seed
uint init_random_seed(uvec2 pixel_coords, uint frame_number)
{
    uint seed = pixel_coords.x + pixel_coords.y * 12345u + frame_number * 6789u;
    return wang_hash(seed);
}

// XorShift RNG
uint xorshift32(inout uint state)
{
    if (state == 0) state = 1;
    state ^= state << 13u;
    state ^= state >> 17u;
    state ^= state << 5u;
    return state;
}

// Function to generate a random float between 0 and 1
float random_float(inout uint state)
{
    uint rnd = xorshift32(state);
    return rnd * 2.3283064365386963e-10f; // 1/2^32
}

float intersect(Sphere s, Ray r) {
    vec3 op = s.p - r.o;   // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
    float eps = 0.0001;
    float b = dot(op, r.d);
    float det = b * b - dot(op, op) + s.rad * s.rad;
    if (det < 0.0) return 0.0;
    det = sqrt(det);
    float t = b - det;
    return t > eps ? t : (t = b + det) > eps ? t : 0.0;
}

const float bigrad = 1000.0;
const float lrad = 100.0;

const uint num_spheres = 9;
const Sphere spheres[num_spheres] = Sphere[](
    Sphere(50.0, vec3(50.0, lrad + 81.6 - 1.0, 81.6), vec3(12.0), vec3(0), DIFF), // Lite
    Sphere(16.5, vec3(73.0, 16.5, 78.0), vec3(0.0), vec3(0.999), REFR),            // Glas
    Sphere(16.5, vec3(27.0, 16.5, 47.0), vec3(0.0), vec3(0.999), SPEC),            // Mirr
    Sphere(bigrad, vec3(50.0, -bigrad + 81.6, 81.6), vec3(0.0), vec3(0.75), DIFF), // Top
    Sphere(bigrad, vec3(50.0, bigrad, 81.6), vec3(0.0), vec3(0.75), DIFF),         // Botm
    Sphere(bigrad, vec3(50.0, 40.8, -bigrad + 170.0), vec3(0.0), vec3(0), DIFF), // Frnt
    Sphere(bigrad, vec3(50.0, 40.8, bigrad), vec3(0.0), vec3(0.75), DIFF),         // Back
    Sphere(bigrad, vec3(-bigrad + 99.0, 40.8, 81.6), vec3(0.0), vec3(0.25, 0.25, 0.75), DIFF), // Rght
    Sphere(bigrad, vec3(bigrad + 1.0, 40.8, 81.6), vec3(0.0), vec3(0.75, 0.25, 0.25), DIFF)    // Left
);


vec3 radiance(Ray r, uint depth) {
    float t = 1000000000000.0; // distance to intersection
    int id = -1; // id of intersected object
    for (int i = 0; i < num_spheres; ++i) {
        float d = intersect(spheres[i], r);
        if (d > 0.0 && d < t) {
            t = d;
            id = i;
        }
    }
    if (id == -1) return vec3(0.0); // if miss, return black
    return spheres[id].c; // the hit object
    // return vec3(1.0); // the hit object
}
