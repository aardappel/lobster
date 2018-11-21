// useful functions to implement common UI interaction and rendering

include vec

// This struct manages the scale & translation values for a zooming & scrolling view.
// For zoom it supports mousewheel and touch screen pinching.
// For translation it supports mouse button or finger dragging (with inertia).

struct scrollzoomview:
    // Mousewheel / zoom related:
    rate:float   // Zoom base rate, determines how rapid you want to zoom, try 1.2.
    mins:int     // The range of exponents, try e.g. (10, 15, 20) and tweak from there.
    defaults:int
    maxs:int
    // Scroll related:
    buttonname:string  // The button to do dragging with (e.g. "mouse1")
    // Internal state:
    panoffset:xy_f = xy_0
    touchvelocity:xy_f = xy_0
    viewsteps:int = 0
    pinchfactor:float = 1.0
    lastdistance:float = 0.0

    // mino/maxo: Bounding box of the scrollable area (in its own units)
    // viewsize: In pixels.
    def update(mino:xy_f, maxo:xy_f, viewsize:xy_i):
        // Update zooming:
        if viewsteps == 0: viewsteps = defaults
        oldscale := gl_window_size().x / pow(rate, viewsteps) * pinchfactor
        oldpanoffset := panoffset
        mousewheeldelta := gl_mousewheel_delta()
        viewsteps -= mousewheeldelta
        viewsteps = max(min(viewsteps, maxs), mins)
        viewscale := pow(rate, viewsteps)
        center := float(gl_mouse_pos(0))
        if gl_button("mouse1") >= 1 and gl_button("mouse11") >= 1:
            dist := magnitude(float(gl_mouse_pos(0) - gl_mouse_pos(1)))
            center = float(gl_mouse_pos(0) + gl_mouse_pos(1)) / 2.0
            if(lastdistance):
                pinchfactor *= dist / lastdistance
            lastdistance = dist
        else:
            lastdistance = 0
        scale := gl_window_size().x / viewscale * pinchfactor
        center /= oldscale
        panoffset -= center / (scale / oldscale) - center
        // Update panning:
        if gl_button(buttonname) >= 1:
            delta := float(gl_mouse_delta(0)) / scale
            panoffset -= delta
            // mild amount of dampening/smoothing from previous frames:
            touchvelocity = (touchvelocity + delta / gl_delta_time()) / 2
        else:
            panoffset -= touchvelocity * gl_delta_time()
            speed := touchvelocity.magnitude()
            if speed > 0:
                reduction := gl_delta_time() / scale * 1000
                if reduction < speed:
                    speed -= reduction
                    touchvelocity = touchvelocity.normalize() * speed
                else:
                    touchvelocity = xy_0
        if !equal(oldpanoffset, panoffset) and mousewheeldelta <= 0:
            maxo -= float(viewsize) / scale
            if(maxo.x < mino.x):
                mino = xy { (mino.x + maxo.x) / 2, mino.y }  // FIXME!!!
                maxo = xy { (mino.x + maxo.x) / 2, maxo.y }
            if(maxo.y < mino.y):
                mino = xy { mino.x, (mino.y + maxo.y) / 2 }
                maxo = xy { maxo.x, (mino.y + maxo.y) / 2 }
            panoffset = xy { max(min(maxo.x, panoffset.x), mino.x),        // FIXME: vector ops
                            max(min(maxo.y, panoffset.y), mino.y) }
        return scale, -panoffset

    def reset(startoffset:xy_f):
        panoffset = startoffset
        touchvelocity = xy_0
        viewsteps = 0
        pinchfactor = 1
        lastdistance = 0


// helper for rendering text at arbitrary scaled locations with maximum precision
// normal font rendering is specified in pixels, and is only rendered at precise pixel boundaries (which looks best)
// when gl_scale(1) and gl_translate in integer amounts
// if you use arbitrary scaling it will look more blurry than necessary
// using this function, it will find the appropriate font size given a size relative to the current scale (not pixels),
// clamp to pixel boundaries,
// then execute any text rendering contained in f

def ui_rendertextatlocalsize(localsize:float, f):
    oldfontsize := gl_get_font_size()
    oldoutlinesize := gl_getoutlinesize()
    sc := gl_scaling().x
    localfontsizef := sc * localsize
    localfontsize := round(localfontsizef)    // font size to nearest pixel
    gl_set_font_size(localfontsize, oldoutlinesize * (localfontsizef / oldfontsize))
    gl_scale xy_1 / gl_scaling():   // get back to pixel scale
        gl_translate(-fraction(gl_origin() + 0.5) + 0.5) // translate to nearest pixel
        f()
    gl_set_font_size(oldfontsize, oldoutlinesize)
