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
    size:xy_i

private let cursize = [ xy_0i ]

private let parent_size = [ xy_0i ]

private let vertical = [ true ]

private var render_pass = false
private var size_list = []
private var size_index = 0

private let align = [ 0 ]
private let spacing = [ 0 ]

private var hover_color = color_dark_grey
private var select_color = color_grey

private var overalsize = xy_0i

private let interactive = [ true ]

private var actionsfxr = nil
private var actionwav = nil

private var num_interactive_elements_hit = 0
private var focus_id = -1

private let button_key = [ "<none>" ]

def fun() -> void
def funs(v:xy_i) -> void

private def cache_size(id, f):
    return if not render_pass:
        let e = elem { id, xy_0i }
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
        xy_0i   // new element inserted by event handler, do not render yet

private def update_size(v:xy_i):
    let cs = cursize.pop()
    cursize.push(
        if vertical.top: xy { max(cs.x, v.x), cs.y + v.y + (cs.y and spacing.top) }
        else:            xy { cs.x + v.x + (cs.x and spacing.top), max(cs.y, v.y) })

private def advance(v:xy_i, f):
    if render_pass:
        f()
    else:
        update_size(v)

private def max_space(v:xy_i):
    return if vertical.top:
        xy { parent_size.top.x, v.y }
    else:
        xy { v.x, parent_size.top.y }

private def position(v:xy_i, f):
    let r = max_space(v) - v
    gl_translate(float(if(align.top == 1): r / 2 else: r * (align.top / 2)), f)

private def cursize_of(id, f):
    return cache_size(id):
        cursize.push(xy_0i)
        f()
        cursize.pop()

private def element(size:xy_i, f):
    // (note: object identity compare!) will be xy_0i if we're in the render pass and
    // element was just deleted
    if size != xy_0i:
        advance(size):
            position(size, f)
            if vertical.top:
                gl_translate(xy_y * (size.y + spacing.top))
            else:
                gl_translate(xy_x * (size.x + spacing.top))

private def block(al, sp, vert, id, f):
    let spi = round(sp * gl_get_font_size())
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
    element(cache_size(hash(text)): gl_text_size(text)):
        gl_text(text)

def image(factors, tex):
    let size = round(factors * gl_get_font_size())
    element(size):
        gl_set_shader("textured")
        gl_set_primitive_texture(0, tex)
        gl_rect(float(size))
        gl_set_shader("color")

def divider(thickness:int):
    align.push(0)
    element(xy_1i * thickness):
        gl_rect(if vertical.top: xy_f { parent_size.top.x, thickness }
                else:            xy_f { thickness, parent_size.top.y })
    align.pop()

def space(thickness:float):
    element(xy_1i * round(thickness * gl_get_font_size())): nil

def start(size, alignh, alignv, interact, f:fun):   // top level only
    let id = hash(f)
    gl_set_shader("color")
    gl_blend(blend_alpha):
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

        if not num_interactive_elements_hit and not gl_touchscreen():
            focus_id = -1

        align.pop()
        interactive.pop()
        vertical.pop()
        parent_size.pop()

def hblock(al, sp, f:fun): block(al, sp, false, hash(f), f)
def vblock(al, sp, f:fun): block(al, sp, true,  hash(f), f)

def fontscale(factor, f:fun):
    let os = gl_get_font_size()
    let ool = gl_get_outline_size()
    gl_set_font_size(round(os * factor), ool * factor)
    f()
    gl_set_font_size(os, ool)

def color(col, f:fun): nest_if(render_pass, fn: gl_color(col, _), f)
def blend(bli, f:fun): nest_if(render_pass, fn: gl_blend(bli, _), f)

private def around(m, draw, id, f):
    var size = cursize_of(id, f)
    size += m * 2
    advance(size):
        if draw:
            position(size): // FIXME: clean this up
                gl_rect(xy_f{ size.x, m.y })
                gl_rect(xy_f { m.x, size.y })
                gl_translate xy_f { size.x - m.x, 0 }:
                    gl_rect(xy_f { m.x, size.y })
                gl_translate xy_f { 0.0, size.y - m.y }:
                    gl_rect(xy_f { size.x, m.y })
        def forward():
            gl_translate([ xy_x, xy_y ][vertical.top] * m[vertical.top])
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
        gl_hit(float(size), 0)    // stop clicks from reaching whatevers beneath this

def background(col, stretch, f:fun):
    backgroundf(stretch, f) size:
        gl_color col:
            gl_rect(float(size))

def key(key_name, f):  // Surround a button with this to also allow keyboard activation.
    button_key.push(key_name)
    f()
    button_key.pop()

private def btn(id, render, do, dofocus:bool, focusf):
    overlay(false, id, render) size:
        if interactive.top:
            let hit = gl_hit(float(size), 0)
            let keydown, keyup = gl_button(button_key.top)
            let active = hit or keydown or keyup == 1
            if active or (focus_id == id and gl_touchscreen()):
                // Use the hover color both for mouse-hover, and touch screen buttons that have focus.
                gl_color hover_color:
                    gl_rect(float(size))
                if active:
                    num_interactive_elements_hit++
                    let mouse1down, mouse1up = gl_button("mouse1")
                    if mouse1down or keydown:
                        gl_color(select_color):
                            gl_rect(float(size))
                    if dofocus and focus_id != id and (mouse1up == 1 or not gl_touchscreen()):
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
