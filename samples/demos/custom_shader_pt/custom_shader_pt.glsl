COMPUTEFUNCTIONS
    INCLUDE helper_functions.glsl
    
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