// useful functions to implement common UI interaction and rendering

import vec

// This class manages the scale & translation values for a zooming & scrolling view.
// For zoom it supports mousewheel and touch screen pinching.
// For translation it supports mouse button or finger dragging (with inertia).

class scrollzoomview:
    // Mousewheel / zoom related:
    rate:float   // Zoom base rate, determines how rapid you want to zoom, try 1.2.
    mins:int     // The range of exponents, try e.g. (10, 15, 20) and tweak from there.
    defaults:int
    maxs:int
    // Scroll related:
    buttonname:string  // The button to do dragging with (e.g. "mouse1")
    // Internal state:
    panoffset = float2_0
    touchvelocity = float2_0
    viewsteps = 0
    pinchfactor = 1.0
    lastdistance = 0.0

    // mino/maxo: Bounding box of the scrollable area (in its own units)
    // viewsize: In pixels.
    def update(mino:float2, maxo:float2, viewsize:int2):
        // Update zooming:
        if viewsteps == 0: viewsteps = defaults
        let oldscale = gl.framebuffer_size().x / pow(rate, viewsteps) * pinchfactor
        let oldpanoffset = panoffset
        let mousewheeldelta = gl.mousewheel_delta()
        viewsteps -= mousewheeldelta
        viewsteps = max(min(viewsteps, maxs), mins)
        let viewscale = pow(rate, viewsteps)
        var center = float(gl.mouse_pos(0))
        if gl.button("mouse1") and gl.button("mouse11"):
            let dist = magnitude(float(gl.mouse_pos(0) - gl.mouse_pos(1)))
            center = float(gl.mouse_pos(0) + gl.mouse_pos(1)) / 2.0
            if(lastdistance):
                pinchfactor *= dist / lastdistance
            lastdistance = dist
        else:
            lastdistance = 0.0
        let scale = gl.framebuffer_size().x / viewscale * pinchfactor
        center /= oldscale
        panoffset -= center / (scale / oldscale) - center
        // Update panning:
        if gl.button(buttonname):
            let delta = float(gl.mouse_delta(0)) / scale
            panoffset -= delta
            // mild amount of dampening/smoothing from previous frames:
            touchvelocity = (touchvelocity + delta / gl.delta_time()) / 2.0
        else:
            panoffset -= touchvelocity * gl.delta_time()
            var speed = touchvelocity.magnitude()
            if speed > 0.0:
                let reduction = gl.delta_time() / scale * 1000.0
                if reduction < speed:
                    speed -= reduction
                    touchvelocity = touchvelocity.normalize() * speed
                else:
                    touchvelocity = float2_0
        if oldpanoffset != panoffset and mousewheeldelta <= 0:
            maxo -= float(viewsize) / scale
            if(maxo.x < mino.x):
                mino = float2 { (mino.x + maxo.x) / 2.0, mino.y }  // FIXME!!!
                maxo = float2 { (mino.x + maxo.x) / 2.0, maxo.y }
            if(maxo.y < mino.y):
                mino = float2 { mino.x, (mino.y + maxo.y) / 2.0 }
                maxo = float2 { maxo.x, (mino.y + maxo.y) / 2.0 }
            panoffset = float2 { max(min(maxo.x, panoffset.x), mino.x),        // FIXME: vector ops
                            max(min(maxo.y, panoffset.y), mino.y) }
        return scale, -panoffset

    def reset(startoffset:float2):
        panoffset = startoffset
        touchvelocity = float2_0
        viewsteps = 0
        pinchfactor = 1.0
        lastdistance = 0.0


// helper for rendering text at arbitrary scaled locations with maximum precision
// normal font rendering is specified in pixels, and is only rendered at precise pixel boundaries (which looks best)
// when gl.scale(1) and gl.translate in integer amounts
// if you use arbitrary scaling it will look more blurry than necessary
// using this function, it will find the appropriate font size given a size relative to the current scale (not pixels),
// clamp to pixel boundaries,
// then execute any text rendering contained in f

def ui_rendertextatpixelsize(f):
    // Translate to nearest pixel:
    gl.translate(-fraction(gl.origin() + 0.5) + 0.5):
        f()

def ui_rendertextatlocalsize(localsize:float, f):
    let oldfontsize = gl.get_font_size()
    let oldoutlinesize = gl.get_outline_size()
    let sc = gl.scaling().x
    let localfontsizef = sc * localsize
    let localfontsize = round(localfontsizef)    // font size to nearest pixel
    gl.set_font_size(localfontsize, oldoutlinesize * (localfontsizef / oldfontsize))
    gl.scale float2_1 / gl.scaling():   // get back to pixel scale
        ui_rendertextatpixelsize(f)
    gl.set_font_size(oldfontsize, oldoutlinesize)

def ui_text_pixel(t):
    ui_rendertextatpixelsize():
        gl.text(t)

def ui_text_local(t):
    ui_rendertextatlocalsize():
        gl.text(t)
