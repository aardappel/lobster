COMPUTEFUNCTIONS

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
        Sphere(lrad, vec3(50.0, lrad + 81.6 - 1.0, 81.6), vec3(12.0), vec3(0), DIFF), // Lite
        Sphere(16.5, vec3(73.0, 16.5, 78.0), vec3(0.0), vec3(0.999), REFR),            // Glas
        Sphere(16.5, vec3(27.0, 16.5, 47.0), vec3(0.0), vec3(0.999), SPEC),            // Mirr
        Sphere(bigrad, vec3(50.0, -bigrad + 81.6, 81.6), vec3(0.0), vec3(0.75), DIFF), // Top
        Sphere(bigrad, vec3(50.0, bigrad, 81.6), vec3(0.0), vec3(0.75), DIFF),         // Botm
        Sphere(bigrad, vec3(50.0, 40.8, -bigrad + 170.0), vec3(0.0), vec3(0), DIFF), // Frnt
        Sphere(bigrad, vec3(50.0, 40.8, bigrad), vec3(0.0), vec3(0.75), DIFF),         // Back
        Sphere(bigrad, vec3(-bigrad + 99.0, 40.8, 81.6), vec3(0.0), vec3(0.25, 0.25, 0.75), DIFF), // Rght
        Sphere(bigrad, vec3(bigrad + 1.0, 40.8, 81.6), vec3(0.0), vec3(0.75, 0.25, 0.25), DIFF)    // Left
    );


    vec3 radiance(Ray initial_ray, uint initial_depth, inout uint seed) {
        vec3 result = vec3(0.0); // Final accumulated color
        vec3 attenuation = vec3(1.0); // Tracks the accumulated reflectance along the path
        uint depth = initial_depth;

        Ray ray = initial_ray;

        while (true) {
            float t = 1000000000000.0; // Distance to intersection
            int id = -1; // ID of the intersected object

            // Find the closest sphere intersection
            for (int i = 0; i < num_spheres; ++i) {
                float d = intersect(spheres[i], ray);
                if (d > 0.0 && d < t) {
                    t = d;
                    id = i;
                }
            }

            // If no intersection, terminate and return the accumulated color
            if (id == -1) {
                result += attenuation * vec3(0.0); // Add black if no hit
                break;
            }

            // Intersection found
            Sphere s = spheres[id];
            vec3 x = ray.o + ray.d * t;  // Hit position
            vec3 n = normalize(x - s.p);  // Surface normal
            vec3 nl = dot(n, ray.d) < 0.0 ? n : -n;  // Properly oriented normal
            vec3 f = s.c; // Object color
            result += attenuation * s.e; // Add emitted light

            // Russian Roulette for termination
            float p = max(max(f.x, f.y), f.z); // Maximum reflectance
            if (++depth > 5) {
                if (random_float(seed) >= p) {
                    break; // Terminate if not reflected
                }
                f *= 1.0 / p; // Scale reflectance
            }

            // Update attenuation
            attenuation *= f;

            // Determine next ray direction based on material type
            if (s.refl == DIFF) { // Diffuse reflection
                float r1 = 360.0* random_float(seed);
                float r2 = random_float(seed);
                float r2s = sqrt(r2);
                vec3 w = nl;
                vec3 u = normalize(abs(w.x) > 0.1 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0));
                vec3 v = cross(w, u);
                vec3 d = normalize(u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0 - r2));
                ray = Ray(x, d); // Update the ray
            } else if (s.refl == SPEC) { // Specular reflection
                ray = Ray(x, ray.d - n * 2.0 * dot(n, ray.d)); // Reflect ray
            } else { // Dielectric refraction
                Ray reflRay = Ray(x, ray.d - n * 2.0 * dot(n, ray.d));
                bool into = dot(n, nl) > 0.0; // Ray entering?
                float nc = 1.0;
                float nt = 1.5;
                float nnt = into ? nc / nt : nt / nc;
                float ddn = dot(ray.d, nl);
                float cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

                if (cos2t < 0.0) { // Total internal reflection
                    ray = reflRay; // Reflect ray
                    continue;
                }

                vec3 tdir = normalize(ray.d * nnt - n * ((into ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))));
                float a = nt - nc;
                float b = nt + nc;
                float R0 = a * a / (b * b);
                float c = 1.0 - (into ? -ddn : dot(tdir, n));
                float Re = R0 + (1.0 - R0) * c * c * c * c * c;
                float Tr = 1.0 - Re;
                float P = 0.25 + 0.5 * Re;
                float RP = Re / P;
                float TP = Tr / (1.0 - P);

                if (random_float(seed) < P) {
                    attenuation *= RP;
                    ray = reflRay;
                } else {
                    attenuation *= TP;
                    ray = Ray(x, tdir);
                }
            }

            ray.o += ray.d * 0.001; // Offset the origin to avoid self-intersection
        }

        return result;
    }
    
SHADER pathtracer 
    COMPUTE 
        LAYOUT 8 8 1
        // texture unit 0: UAV albedo
        UNIFORMS texfuav0 
        // texture unit 0: UAV albedo
        UNIFORMS texfuavw1 
        UNIFORM int frame_count 
        UNIFORM int frame_accum 
        UNIFORM mat2x3 m_ray
        UNIFORM mat2x3 lens
        UNIFORM ivec2 window_size

        ivec2 iuv = ivec2(gl_GlobalInvocationID.xy);
        uvec2 uv = gl_GlobalInvocationID.xy;
        Ray cam = Ray(m_ray[0], m_ray[1]);
        uint seed = init_random_seed(uv, uint(frame_count));

        vec3 cx = lens[0];
        vec3 cy = lens[1];

        float r1 = clamp(2.0 * random_float(seed), 0.0, 2.0);
        float dx = r1<1.0 ? sqrt(r1)-1.0 : 1.0-sqrt(2.0-r1);
        float r2 = clamp(2.0 * random_float(seed), 0.0, 2.0);
        float dy = r2<1.0 ? sqrt(r2)-1.0 : 1.0-sqrt(2.0-r2);
        vec3 d = cx*( ( (1.0 + dx)/2.0 + uv.x)/window_size.x - .5) +
                cy*( ( (1.0 + dy)/2.0 + uv.y)/window_size.y - .5) + cam.d;
        d = normalize(d);

        vec3 r = radiance(Ray(cam.o+d*140.0, d), 0, seed);
        iuv.y = window_size.y - iuv.y - 1;
        vec4 prev = imageLoad(texfuav0, iuv);
        vec4 average = vec4((prev.xyz * float(frame_accum) + r) / float(frame_accum + 1), 1.0);
        imageStore(texfuavw1, iuv, average);