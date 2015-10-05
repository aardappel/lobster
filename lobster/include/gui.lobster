// a simple GUI system to auto layout text, images, borders and clickable buttons etc on top of a game
// no documentation yet, check out the test case at the end

include "vec.lobster"
include "color.lobster"

/*
TODO:

- need a select one of many mode (see totslike)
- need a "stretch" mode that automatically uses the maximum space inside the res

- other widgets... multiple select etc
- automatically scroll if doesn't fit in windowsize?

*/

private struct guielem { id:int, size:xy_i }

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

private actionsound := nil

private focus_id := -1

private def cache_size(id, f):
    if !render_pass:
        e := guielem { id, xy_0i }
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

private def advance(v:xy_i, f):
    if render_pass:
        f()
    else:
        cursize =
            if(vertical): xy { max(cursize.x, v.x), cursize.y + v.y + (cursize.y and spacing) }
            else:         xy { cursize.x + v.x + (cursize.x and spacing), max(cursize.y, v.y) }

private def max_space(v):
    if vertical: xy { parent_size.x, v.y } else: xy { v.x, parent_size.y }

private def position(v:xy_i, f):
    r := max_space(v) - v
    gl_translate(float(if(align == 1): r / 2 else: r * (align / 2)), f)

private def cursize_of(id, f):
    cache_size(id):
        cursize <- xy_0i
        f()
        cursize

private def element(size, f):
    if size != xy_0i:   // (note: object identity compare!) will be xy_0i if we're in the render pass and element was just deleted
        advance(size):
            position(size, f)
            if vertical:
                gl_translate(xy_y * (size.y + spacing))
            else:
                gl_translate(xy_x * (size.x + spacing))

private def block(al, sp, vert, id, f):
    spi := round(sp * gl_getfontsize())
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

def gui_text(text:string):
    element(cache_size(caller_id()): gl_textsize(text)):
        //print(gl_textsize(text))
        gl_text(text)

def gui_image(factors, tex):
    size := round(factors * gl_getfontsize())
    element(size):
        gl_setshader("textured")
        gl_setprimitivetexture(0, tex)
        gl_rect(float(size))
        gl_setshader("color")

def gui_divider(thickness:int):
    align <- 0
    element(xy_1i * thickness):
        gl_rect(float(if vertical: [ parent_size.x, thickness ] else: [ thickness, parent_size.y ]))

def gui_space(thickness:float):
    element(xy_1i * round(thickness * gl_getfontsize())): nil

def gui_start(size, alignh, alignv, interact, f):   // top level only
    id := caller_id()
    gl_setshader("color")
    gl_blend(1):
        parent_size <- size
        vertical <- false
        interactive <- interact

        render_pass = false
        size_list = []

        overalsize = cursize_of(id, f)
        assert(overalsize)

        //print(size_list)

        render_pass = true
        size_index = 1

        align <- alignv
        position(overalsize):
            align <- alignh
            vertical <- true
            f()

        //assert(size_index == length(size_list))
        //print(size_index)

def gui_hblock(al, sp, f): block(al, sp, false, caller_id(), f)
def gui_vblock(al, sp, f): block(al, sp, true,  caller_id(), f)

def gui_fontscale(factor, f):
    os := gl_getfontsize()
    gl_setfontsize(round(os * factor))
    f()
    gl_setfontsize(os)

def gui_color(col, f): nest_if(render_pass, def(): gl_color(col, _), f)
def gui_blend(bli, f): nest_if(render_pass, def(): gl_blend(bli, _), f)

private def around(m, draw, id, f):
    size := cursize_of(id, f)
    if size:
        size += m * 2
        advance(size):
            if draw:
                position(size): // FIXME: clean this up
                    gl_rect(float([ size.x, m.y ]))
                    gl_rect(float([ m.x, size.y ]))
                    gl_translate float([ size.x - m.x, 0]):
                        gl_rect(float([ m.x, size.y ]))
                    gl_translate [ 0.0, size.y - m.y]:
                        gl_rect(float([ size.x, m.y ]))
            def forward():
                gl_translate([ xy_x, xy_y ][vertical] * m[vertical])
            forward()
            f()
            forward()

def gui_margin(m, f): around(m, false, caller_id(), f)
def gui_border(m, f): around(m, true,  caller_id(), f)

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

def gui_background(col, stretch, f):
    overlay(stretch, caller_id(), f) size:
        gl_color col:
            gl_rect(float(size))
            gl_hit(float(size), 0)    // stop clicks from reaching whatevers beneath this

private def button(id, f, do, dofocus, focusf):
    overlay(false, id, f) size:
        if interactive:
            hit := gl_hit(float(size), 0)
            if hit or (focus_id == id and gl_touchscreen()):
                // Use the hover color both for mouse-hover, and touch screen buttons that have focus.
                gl_color hover_color:
                    gl_rect(float(size))
                if hit:
                    if gl_isdown("mouse1"):
                        gl_color(select_color):
                            gl_rect(float(size))
                    up := gl_wentup("mouse1")
                    if dofocus and focus_id != id and (up or !gl_touchscreen()):
                        // If we have a focus action and this button wasn't the focus, then on a touch screen
                        // we're going to give this button focus on touch, and on desktop merely on hover.
                        focus_id = id
                        focusf()
                    else: if up:
                        if actionsound: play_sfxr(actionsound) // FIXME: wavs too
                        do()

def gui_button(f, do):                   button(caller_id(), f, do, false): nil
def gui_button(f, do, focus):            button(caller_id(), f, do, true, focus)
def gui_button_id(id:int, f, do):        button(id,          f, do, false): nil
def gui_button_id(id:int, f, do, focus): button(id,          f, do, true, focus)

def gui_button_text   (        text, do):         button(caller_id(), def(): gui_text(text), do, false): nil
def gui_button_id_text(id:int, text, do):         button(id,          def(): gui_text(text), do, false): nil
def gui_button_id_text(id:int, text, do, focus):  button(id,          def(): gui_text(text), do, true, focus)

def gui_button_image   (        size, tex, do):         button(caller_id(), def(): gui_image(size, tex), do, false): nil
def gui_button_id_image(id:int, size, tex, do):         button(id,          def(): gui_image(size, tex), do, false): nil
def gui_button_id_image(id:int, size, tex, do, focus):  button(id,          def(): gui_image(size, tex), do, true, focus)


def gui_custombackground(f): if render_pass: f(overalsize)

def gui_setclickcolors(hover, sel):
    hover_color = hover
    select_color = sel

def gui_setactionsound(snd):
    actionsound = snd

if program_name() == "gui.lobster": // run example

    fatal(gl_window("gui test", 960, 640))
    check(gl_setfontname("data/fonts/US101/US101.ttf"), "can\'t load font!")
    //check(gl_setfontname("C:\\Windows\\Fonts\\arialuni.ttf"), "can\'t load font!")

    ghosttex := gl_loadtexture("data/textures/lobster.jpg")

    clicked := false
    focused := false

    while gl_frame():
        if gl_wentdown("escape"):
            return

        gl_setfontsize(gl_windowsize().y / 10)
        //gl_setmaxfontsize(512)

        gl_clear(color_black)
        gl_color(color_white)

        gui_setclickcolors(color_dark_green, color_green)

        gui_start(gl_windowsize(), 1, 1, true):
            gui_hblock(1, 0):
                //gui_fontscale(0.3):
                 //   gui_text(unicode2string(map(500): _))

                gui_text("A")
                gui_fontscale(5):
                    gui_text("B")
                gui_vblock(2, 0):
                    gui_text("1")
                    gui_hblock(1, 0.1):
                        gui_text("2")
                        gui_text("2")
                        gui_text("2")
                    gui_divider(2)
                    gui_text("3")
                    gui_text("4")
                gui_space(0.2)
                gui_divider(4)
                gui_space(0.2)
                gui_text("C")
                gui_button():
                    gui_border(xy_1i * 2):
                        gui_fontscale(2):
                            gui_text("clickme")
                do:
                    clicked = !clicked
                focus:
                    focused = !focused
                if clicked:
                    gui_color(color_red):
                        //gui_text(unicode2string([0x30E6, 0x30FC, 0x30B6, 0x30FC, 0x5225, 0x30B5, 0x30A4, 0x30C8]))
                        gui_text("ON")
                gui_text(focused and "C" or "E")
                gui_background(color_blue, false):
                    gui_margin(xy_1i * 4):
                        gui_text("D")
                        if(clicked):
                            gui_blend(2):
                                gui_image(xy_1, ghosttex)
                gui_text("E")
                gui_text("E")

