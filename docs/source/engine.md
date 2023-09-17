---
title: Lobster built-in engine functionality
---

Lobster comes with "batteries included": a small (games/graphics) engine that allows you to start
drawing straight away. This doc collects some notes on the the built-in functionality.

Basics
------
The basic list of [builtin functions](builtin_functions_reference.html) is worth
browsing first, as well as the [shooter tutorial](shooter_tutorial.html)
for some basic explanation of how to render stuff. Most examples in the `samples` dir
are graphical and provide further examples.

While it easy to render stuff with simple functions like `gl.rect`, `gl.line`
and stuff, for anything more advanced some understanding of OpenGL style
rendering is required.

Default rendering environment
----------------------------
After you call `gl.window` you are left with a default black background,
a white drawing color, and the `color` shader set.

Shaders
-------
All basic shaders, including `color`, can be found in `data/shaders/default.materials`.

You can use one of the many useful shaders from there with `gl.set_shader`,
but more fun is to use a custom shader. You can define these in-line in
your Lobster program as well, an example of that can be found in
`samples/custom_shader_metaballs.lobster`.

Lobster uses glsl shaders compatible with OpenGL 3+ and OpenGL ES 3+,
and you'll have to look elsewhere to learn the details of this language.

Lobster wraps these shaders in a custom declaration language, to cut down on
some of the boilerplate of putting a vertex and pixel shader together.

Lets look at the `textured` shader as an example:

```
SHADER textured
    VERTEX
        INPUTS apos:4 atc:2
        UNIFORMS mvp
        gl_Position = mvp * apos;
        itc = atc;
    PIXEL
        INPUTS itc:2
        UNIFORMS tex0 col
        frag_color = texture(tex0, itc) * col;
```

In all caps we have keywords that help declare these shaders for you.
We have a `VERTEX` and `PIXEL` part (alternatively, you could have only
a `COMPUTE` part).

The inputs to the vertex shader must match your vertex attributes, specify the
number of components you care about after `:`, and come from a fixed set of:
* `apos` (position, 2 or 3 components in the vertex buffer, but usually 4 in
  the shader to work with matrix transforms).
* `anormal` (3 components).
* `atc` (texture coordinate, 2 components).
* `acolor` (4 components)
* `aweights` and `aindices` (used with character animation).

The outputs of the vertex shader are automatically the same as the pixel
shader inputs, here `itc` (interpolated texture coordinate).

Using `UNIFORMS` you declare variables automatically provided by the engine:
* `mvp`: the Model View Projection matrix composed of the various `gl.scale`,
  `gl.translate` and `gl.rotate` transforms (the `mv` part) and the `gl.ortho`
  or `gl.perspective` transforms (the `p` part).
* `col`: set by `gl.color`.
* `camera`: position of the camera relative to the primitive being rendered.
* `light1` and `lightparams1`: set by `gl.light`.
* `framebuffer_size`: size in pixels.
* `bones`: see character animation shaders.
* `pointscale`: used with point rendering.

In addition you can add any custom uniforms with a `UNIFORM` declaration, for
example by adding `UNIFORM float time` to the shader and
`gl.set_uniform("time", gl.time())` you can animate your shader to the
current time.

Following these is the raw glsl implementing the shader. This is the body
of the shader, if you want to add additional helper functions, write
a `VERTEXFUNCTIONS` or `PIXELFUNCTIONS` block before the shader. The
functions declared here will be available to all shaders following it.

Additionally `DEFINE name val` defines macros to be used in the shaders below.

For compute shaders, e.g. `COMPUTE 8 8` at the end of the uniforms declares
the dispatch size.

Any more details, see `glshader.cpp` ;)
