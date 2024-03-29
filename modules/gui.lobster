// a simple GUI system to auto layout text, images, borders and clickable buttons etc on top of a game
// no documentation yet, check out the test case at the end

import vec
import color
import gl
import texture

/*
TODO:

- need a select one of many mode (see totslike)
- need a "stretch" mode that automatically uses the maximum space inside the res

- other widgets... multiple select etc
- automatically scroll if doesn't fit in windowsize?

*/

namespace gui

private class elem:
    id:int
    size:int2

private let cursize = [ int2_0 ]

private let parent_size = [ int2_0 ]

private let vertical = [ true ]

private var render_pass = false
private var size_list = []
private var size_index = 0

private let align = [ 0 ]
private let spacing = [ 0 ]

private var hover_color = color_dark_grey
private var select_color = color_grey

private var overalsize = int2_0

private let interactive = [ true ]

private var actionsfxr = nil
private var actionwav = nil

private var num_interactive_elements_hit = 0
private var focus_id = -1

private let button_key = [ "<none>" ]

def fun() -> void
def funs(v:int2) -> void

private def cache_size(id, f):
    return if not render_pass:
        let e = elem { id, int2_0 }
        // push first, just in case f() pushes elements also (keep pre-order)
        size_list.push(e)
        e.size = f()
    else:
        var i = size_index
        // usually this lookup will hit the if on the first iteration, but if not,
        // any skipped elements are old elements just removed by an event handler
        while i < size_list.length:
            let e = size_list[i++]
            if e.id == id:
                size_index = i
                return e.size
        int2_0   // new element inserted by event handler, do not render yet

private def update_size(v:int2):
    let cs = cursize.pop()
    cursize.push(
        if vertical.top: int2 { max(cs.x, v.x), cs.y + v.y + (cs.y and spacing.top) }
        else:            int2 { cs.x + v.x + (cs.x and spacing.top), max(cs.y, v.y) })

private def advance(v:int2, f):
    if render_pass:
        f()
    else:
        update_size(v)

private def max_space(v:int2):
    return if vertical.top:
        int2 { parent_size.top.x, v.y }
    else:
        int2 { v.x, parent_size.top.y }

private def position(v:int2, f):
    let r = max_space(v) - v
    gl.translate(float(if(align.top == 1): r / 2 else: r * (align.top / 2)), f)

private def cursize_of(id, f):
    return cache_size(id):
        cursize.push(int2_0)
        f()
        cursize.pop()

private def element(size:int2, f):
    // (note: object identity compare!) will be int2_0 if we're in the render pass and
    // element was just deleted
    if size != int2_0:
        advance(size):
            position(size, f)
            if vertical.top:
                gl.translate(float2_y * (size.y + spacing.top))
            else:
                gl.translate(float2_x * (size.x + spacing.top))

private def block(al, sp, vert, id, f):
    let spi = round(sp * gl.get_font_size())
    let size = cursize_of(id):
        spacing.push(spi)
        vertical.push(vert)
        f()
        vertical.pop()
        spacing.pop()
    element(size):
        spacing.push(spi)
        vertical.push(vert)
        align.push(al)
        parent_size.push(size)
        f()
        parent_size.pop()
        align.pop()
        vertical.pop()
        spacing.pop()

def text(text:string):
    element(cache_size(hash(text)): gl.text_size(text)):
        gl.text(text)

def image(factors, tex):
    let size = round(factors * gl.get_font_size())
    element(size):
        gl.set_shader("textured")
        gl.set_primitive_texture(0, tex)
        gl.rect(float(size))
        gl.set_shader("color")

def divider(thickness:int):
    align.push(0)
    element(int2_1 * thickness):
        gl.rect(if vertical.top: float2 { parent_size.top.x, thickness }
                else:            float2 { thickness, parent_size.top.y })
    align.pop()

def space(thickness:float):
    element(int2_1 * round(thickness * gl.get_font_size())): nil

def start(size, alignh, alignv, interact, f:fun):   // top level only
    let id = hash(f)
    gl.set_shader("color")
    gl.blend(blend_alpha):
        parent_size.push(size)
        vertical.push(false)
        interactive.push(interact)
        num_interactive_elements_hit = 0

        render_pass = false
        size_list = []

        overalsize = cursize_of(id, f)

        render_pass = true
        size_index = 1

        align.push(alignv)
        position(overalsize):
            align.push(alignh)
            vertical.push(true)
            f()
            vertical.pop()
            align.pop()

        if not num_interactive_elements_hit and not gl.touchscreen():
            focus_id = -1

        align.pop()
        interactive.pop()
        vertical.pop()
        parent_size.pop()

def hblock(al, sp, f:fun): block(al, sp, false, hash(f), f)
def vblock(al, sp, f:fun): block(al, sp, true,  hash(f), f)

def fontscale(factor, f:fun):
    let os = gl.get_font_size()
    let ool = gl.get_outline_size()
    gl.set_font_size(round(os * factor), ool * factor)
    f()
    gl.set_font_size(os, ool)

def color(col, f:fun): nest_if(render_pass, fn: gl.color(col, _), f)
def blend(bli, f:fun): nest_if(render_pass, fn: gl.blend(bli, _), f)

private def around(m, draw, id, f):
    var size = cursize_of(id, f)
    size += m * 2
    advance(size):
        if draw:
            position(size): // FIXME: clean this up
                gl.rect(float2{ size.x, m.y })
                gl.rect(float2 { m.x, size.y })
                gl.translate float2 { size.x - m.x, 0.0 }:
                    gl.rect(float2 { m.x, size.y })
                gl.translate float2 { 0.0, size.y - m.y }:
                    gl.rect(float2 { size.x, m.y })
        def forward():
            gl.translate([ float2_x, float2_y ][vertical.top] * m[vertical.top])
        forward()
        f()
        forward()

def margin(m, f:fun): around(m, false, hash(f), f)
def border(m, f:fun): around(m, true,  hash(f), f)

private def overlay(stretch, id, f, g):
    let size = cursize_of(id, f)
    advance(size):
        if stretch:
            g(max_space(size))
        else:
            position(size):
                g(size)
        f()

def backgroundf(stretch, f:fun, background:funs):
    overlay(stretch, hash(f), f) size:
        background(size)
        gl.hit(float(size), 0)    // stop clicks from reaching whatevers beneath this

def background(col, stretch, f:fun):
    backgroundf(stretch, f) size:
        gl.color col:
            gl.rect(float(size))

def key(key_name, f):  // Surround a button with this to also allow keyboard activation.
    button_key.push(key_name)
    f()
    button_key.pop()

private def btn(id, render, do, dofocus:bool, focusf):
    overlay(false, id, render) size:
        if interactive.top:
            let hit = gl.hit(float(size), 0)
            let keydown, keyup = gl.button(button_key.top)
            let active = hit or keydown or keyup == 1
            if active or (focus_id == id and gl.touchscreen()):
                // Use the hover color both for mouse-hover, and touch screen buttons that have focus.
                gl.color hover_color:
                    gl.rect(float(size))
                if active:
                    num_interactive_elements_hit++
                    let mouse1down, mouse1up = gl.button("mouse1")
                    if mouse1down or keydown:
                        gl.color(select_color):
                            gl.rect(float(size))
                    if dofocus and focus_id != id and (mouse1up == 1 or not gl.touchscreen()):
                        // If we have a focus action and this button wasn't the focus, then on a touch screen
                        // we're going to give this button focus on touch, and on desktop merely on hover.
                        focus_id = id
                        focusf()
                    elif mouse1up == 1 or keyup == 1:
                        if actionsfxr: play_sfxr(actionsfxr)
                        elif actionwav: play_wav(actionwav)
                        do()

def button(f:fun, do:fun):
    btn(hash(f), f, do, false): nil

def button(f:fun, do:fun, focus:fun):
    btn(hash(f), f, do, true, focus)

def button_id(id:int, f:fun, do:fun):
    btn(id, f, do, false): nil

def button_id(id:int, f:fun, do:fun, focus:fun):
    btn(id, f, do, true, focus)

def button_text(msg:string, do:fun):
    btn(hash(msg), fn: text(msg), do, false): nil

def button_id_text(id:int, msg:string, do:fun):
    btn(id, fn: text(msg), do, false): nil

def button_id_text(id:int, msg:string, do:fun, focus:fun):
    btn(id, fn: text(msg), do, true, focus)

def button_text_color(msg:string, col:color, do:fun):
    color(col):
        btn(hash(msg), fn: text(msg), do, false): nil

def button_image(size, tex, do:fun):
    btn(hash(tex), fn: image(size, tex), do, false): nil

def button_id_image(id:int, size, tex, do:fun):
    btn(id, fn: image(size, tex), do, false): nil

def button_id_image(id:int, size, tex, do:fun, focus:fun):
    btn(id, fn: image(size, tex), do, true, focus)

def button_image_color(size, tex, col:color, do:fun):
    color(col):
        btn(hash(tex), fn: image(size, tex), do, false): nil

def custombackground(f:funs): if render_pass: f(overalsize)

def multiselect(labels, selected:int, mspacing):
    hblock(1, mspacing):
        for(labels) name, i:
            color(if selected == i: color_white else: color_grey):
                button_text(name):
                    // FIXME: we force "selected" to int, because otherwise this assign will
                    // fail when selected is an enum. We currently have no way to coerce
                    // i to "whatever type selected is" (e.g. template parameters).
                    // Problem is that the caller of this function needs to now coerce.
                    selected = i
    return selected

def setclickcolors(hover, sel):
    hover_color = hover
    select_color = sel

def setactionsfxr(snd): actionsfxr = snd
def setactionwav(snd): actionwav = snd
