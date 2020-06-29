import vec
import color
import texture

// Example of how to do run a custom shader.
// In this case, a simple metaballs example adapted from
// https://www.shadertoy.com/view/XssSzN

fatal(gl_window("custom shader", 1024, 1024))

let mats =
"""
SHADER metaballs
    VERTEX
        INPUTS apos:4 atc:2
        UNIFORMS mvp
        gl_Position = mvp * apos;
        itc = atc;
    PIXEL
        INPUTS itc:2
        UNIFORM float time
        vec2 uv = itc * 2.0 - 1.0;
        float v = 0.0;
        for (int i = 0; i < 100; i++) {
            vec2 c = sin(time * (0.1 + float(i) / 300.0) + vec2(i, -i));
            v += 1.0 - smoothstep(0.0, 0.2, length(uv - c));
        }
        frag_color = vec4(mix(vec3(v), vec3(1.0), smoothstep(0.9, 0.9, v)), 1.0);
"""

fatal(gl_load_materials(mats, true))

while(gl_frame()):
    if gl_button("escape") == 1: return
    gl_clear(color_black)
    gl_blend(blend_none)
    gl_set_shader("metaballs")
    gl_set_uniform("time", gl_time())
    gl_rect(float(gl_window_size()))
