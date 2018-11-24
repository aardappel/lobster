// a simple GUI system to auto layout text, images, borders and clickable buttons etc on top of a game
// no documentation yet, check out the test case at the end

include vec
include color
include texture

/*
TODO:

- need a select one of many mode (see totslike)
- need a "stretch" mode that automatically uses the maximum space inside the res

- other widgets... multiple select etc
- automatically scroll if doesn't fit in windowsize?

*/

namespace gui

private struct elem { id:int, size:xy_i }

private cursize := xy_0i

private parent_size := xy_0i

private vertical := true

private render_pass := false
private size_list := []
private size_index := 0

private align := 0
private spacing := 0

private hover_color := color_dark_grey
private select_color := color_grey

private overalsize := xy_0i

private interactive := true

private actionsfxr := nil
private actionwav := nil

private num_interactive_elements_hit := 0
private focus_id := -1

private button_key := "<none>"

def fun() :== ()
def funs(v:xy_i) :== ()

private def cache_size(id, f):
    if !render_pass:
        e := elem { id, xy_0i }
        // push first, just in case f() pushes elements also (keep pre-order)
        size_list.push(e)
        e.size = f()
    else:
        i := size_index
        // usually this lookup will hit the if on the first iteration, but if not, any skipped elements are old
        // elements just removed by an event handler
        while i < size_list.length:
            e := size_list[i++]
            if e.id == id:
                size_index = i
                return e.size
        xy_0i   // new element inserted by event handler, do not render yet

private def update_size(v:xy_i):
    cursize =
        if(vertical): xy { max(cursize.x, v.x), cursize.y + v.y + (cursize.y and spacing) }
        else:         xy { cursize.x + v.x + (cursize.x and spacing), max(cursize.y, v.y) }

private def advance(v:xy_i, f):
    if render_pass:
        f()
    else:
        update_size(v)

private def max_space(v:xy_i):
    if vertical: xy { parent_size.x, v.y } else: xy { v.x, parent_size.y }

private def position(v:xy_i, f):
    r := max_space(v) - v
    gl_translate(float(if(align == 1): r / 2 else: r * (align / 2)), f)

private def cursize_of(id, f):
    cache_size(id):
        cursize <- xy_0i
        f()
        cursize

private def element(size:xy_i, f):
    if size != xy_0i:   // (note: object identity compare!) will be xy_0i if we're in the render pass and element was just deleted
        advance(size):
            position(size, f)
            if vertical:
                gl_translate(xy_y * (size.y + spacing))
            else:
                gl_translate(xy_x * (size.x + spacing))

private def block(al, sp, vert, id, f):
    spi := round(sp * gl_get_font_size())
    size := cursize_of(id):
        spacing <- spi
        vertical <- vert
        f()
    element(size):
        vertical <- vert
        align <- al
        spacing <- spi
        parent_size <- size
        f()

def text(text:string):
    element(cache_size(hash(text)): gl_text_size(text)):
        gl_text(text)

def image(factors, tex):
    size := round(factors * gl_get_font_size())
    element(size):
        gl_set_shader("textured")
        gl_set_primitive_texture(0, tex)
        gl_rect(float(size))
        gl_set_shader("color")

def divider(thickness:int):
    align <- 0
    element(xy_1i * thickness):
        gl_rect(if vertical: xy_f { parent_size.x, thickness } else: xy_f { thickness, parent_size.y })

def space(thickness:float):
    element(xy_1i * round(thickness * gl_get_font_size())): nil

def start(size, alignh, alignv, interact, f:fun):   // top level only
    id := hash(f)
    gl_set_shader("color")
    gl_blend(blend_alpha):
        parent_size <- size
        vertical <- false
        interactive <- interact
        num_interactive_elements_hit = 0

        render_pass = false
        size_list = []

        overalsize = cursize_of(id, f)
        assert(overalsize)

        render_pass = true
        size_index = 1

        align <- alignv
        position(overalsize):
            align <- alignh
            vertical <- true
            f()

        if !num_interactive_elements_hit and !gl_touchscreen():
            focus_id = -1

def hblock(al, sp, f:fun): block(al, sp, false, hash(f), f)
def vblock(al, sp, f:fun): block(al, sp, true,  hash(f), f)

def fontscale(factor, f:fun):
    os := gl_get_font_size()
    ool := gl_getoutlinesize()
    gl_set_font_size(round(os * factor), ool * factor)
    f()
    gl_set_font_size(os, ool)

def color(col, f:fun): nest_if(render_pass, def(): gl_color(col, _), f)
def blend(bli, f:fun): nest_if(render_pass, def(): gl_blend(bli, _), f)

private def around(m, draw, id, f):
    size := cursize_of(id, f)
    if size:
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
                gl_translate([ xy_x, xy_y ][vertical] * m[vertical])
            forward()
            f()
            forward()

def margin(m, f:fun): around(m, false, hash(f), f)
def border(m, f:fun): around(m, true,  hash(f), f)

private def overlay(stretch, id, f, g):
    size := cursize_of(id, f)
    if size:
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
    button_key <- key_name
    f()

private def button(id, f, do, dofocus, focusf):
    overlay(false, id, f) size:
        if interactive:
            hit := gl_hit(float(size), 0)
            key := gl_button(button_key)
            active := hit or key >= 0
            if active or (focus_id == id and gl_touchscreen()):
                // Use the hover color both for mouse-hover, and touch screen buttons that have focus.
                gl_color hover_color:
                    gl_rect(float(size))
                if active:
                    num_interactive_elements_hit++
                    mouse1 := gl_button("mouse1")
                    if mouse1 >= 1 or key >= 1:
                        gl_color(select_color):
                            gl_rect(float(size))
                    if dofocus and focus_id != id and (mouse1 == 0 or !gl_touchscreen()):
                        // If we have a focus action and this button wasn't the focus, then on a touch screen
                        // we're going to give this button focus on touch, and on desktop merely on hover.
                        focus_id = id
                        focusf()
                    else: if mouse1 == 0 or key == 0:
                        if actionsfxr: play_sfxr(actionsfxr)
                        else: if actionwav: play_wav(actionwav)
                        do()

def button(f:fun, do:fun):                       button(hash(f), f, do, false): nil
def button(f:fun, do:fun, focus:fun):            button(hash(f), f, do, true, focus)
def button_id(id:int, f:fun, do:fun):            button(id,      f, do, false): nil
def button_id(id:int, f:fun, do:fun, focus:fun): button(id,      f, do, true, focus)

def button_text   (        msg:string, do:fun):            button(hash(msg), def(): text(msg), do, false): nil
def button_id_text(id:int, msg:string, do:fun):            button(id,        def(): text(msg), do, false): nil
def button_id_text(id:int, msg:string, do:fun, focus:fun): button(id,        def(): text(msg), do, true, focus)

def button_image   (        size, tex, do:fun):            button(hash(tex), def(): image(size, tex), do, false): nil
def button_id_image(id:int, size, tex, do:fun):            button(id,        def(): image(size, tex), do, false): nil
def button_id_image(id:int, size, tex, do:fun, focus:fun): button(id,        def(): image(size, tex), do, true, focus)

def custombackground(f:funs): if render_pass: f(overalsize)

def multiselect(labels, selected, mspacing):
    hblock(1, mspacing):
        for(labels) name, i:
            color(if selected == i: color_white else: color_grey):
                button_text(name):
                    selected = i
    selected

def setclickcolors(hover, sel):
    hover_color = hover
    select_color = sel

def setactionsfxr(snd): actionsfxr = snd
def setactionwav(snd): actionwav = snd


