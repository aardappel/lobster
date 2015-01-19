// useful functions to implement common UI interaction and rendering

include "vec.lobster"

// inertia scroll: call this once a frame to receive a panning offset for any 2D scrolling item,
// as it has been dragged by the user using "button" (e.g. "mouse1")
// scale: the size in pixels of one unit in the scrollable area
// size: of the scrollable area (in its own units)
// viewsize: in pixels

private panoffset := xy_0
private touchvelocity := xy_0

function ui_inertiascroll(buttonname, scale, mino, maxo, viewsize):
    if(gl_isdown(buttonname)):
        delta := gl_mousedelta(0) / scale
        panoffset -= delta
        // mild amount of dampening/smoothing from previous frames:
        touchvelocity = (touchvelocity + delta / gl_deltatime()) / 2
    else:
        panoffset -= touchvelocity * gl_deltatime()
        speed := touchvelocity.magnitude()
        if(speed > 0):
            reduction := gl_deltatime() / scale * 1000
            if(reduction < speed):
                speed -= reduction
                touchvelocity = touchvelocity.normalize() * speed
            else:
                touchvelocity = xy_0
    maxo -= viewsize / scale
    if(maxo.x < mino.x):
        mino = mino.replace(0, (mino.x + maxo.x) / 2)   // FIXME!!!
        maxo = maxo.replace(0, (mino.x + maxo.x) / 2)
    if(maxo.y < mino.y):
        mino = mino.replace(1, (mino.y + maxo.y) / 2)
        maxo = maxo.replace(1, (mino.y + maxo.y) / 2)

    panoffset = [ max(min(maxo.x, panoffset.x), mino.x),        // FIXME: vector ops
                  max(min(maxo.y, panoffset.y), mino.y) ]:xy
    panoffset

function ui_inertiascroll_reset(startoffset):
    panoffset = startoffset
    touchvelocity = xy_0


// Mouse wheel or pinch zoom: call each frame to receive a current zoom factor depending on the mousewheel of pinch
// on touch devices.
// Zooms centered around the mouse cursor or the middle of the two fingers, so affects the panning offset returned
// by ui_inertiascroll()
// rate: determines how rapid you want to zoom, try 1.2
// mins/default/maxs: the range of exponents, try e.g. (10, 15, 20) and tweak from there
// origin is the translation that has been set so far, if gl_translate hasn't been called yet, pass in xy_0

private viewsteps := 0
private pinchfactor := 1.0
private lastdistance := 0.0

function ui_mousewheelorpinchzoom(rate, mins, default, maxs, origin):
    if(viewsteps == 0): viewsteps = default
    oldscale := gl_windowsize().x / pow(rate, viewsteps) * pinchfactor
    viewsteps -= gl_mousewheeldelta()
    viewsteps = max(min(viewsteps, maxs), mins)
    viewscale := pow(rate, viewsteps)
    center := float(gl_mousepos(0))
    if(gl_isdown("mouse1") & gl_isdown("mouse11")):
        dist := magnitude(gl_mousepos(0) - gl_mousepos(1))
        center = (gl_mousepos(0) + gl_mousepos(1)) / 2.0
        if(lastdistance):
            pinchfactor *= dist / lastdistance
        lastdistance = dist
    else:
        lastdistance = 0
    scale := gl_windowsize().x / viewscale * pinchfactor
    center -= origin
    center /= oldscale
    panoffset -= center / (scale / oldscale) - center
    //if(scale != oldscale): print(oldscale + " " + scale + " " + center + " " + panoffset)
    scale

function ui_mousewheelzoom_reset():
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

function ui_rendertextatlocalsize(localsize:float, f):
    oldfontsize := gl_getfontsize()
    localfontsize := round(gl_scaling().x * localsize)    // font size to nearest pixel
    gl_setfontsize(localfontsize)
    gl_scale(1 / gl_scaling()):   // get back to pixel scale
        gl_translate(-fraction(gl_origin() + 0.5) + 0.5) // translate to nearest pixel
        f()
    gl_setfontsize(oldfontsize)
