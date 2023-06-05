import std
import vec
import color
import gl

let indentsize = 20.0
let column_margin = 10.0
let fontsize = 20

class node:
    name:string
    size:int
    parent:node?
    children:[node] = []

let root = node { "", -1, nil }
var selected = root
var entries = []

def fullpath(n::node) -> string:
    return if parent: "{parent.fullpath()}/{name}" else: name

def scan(n:node):
    entries = []
    let names, sizes = scan_folder(n.fullpath)
    if names and sizes:
        for(names) name, i:
            entries.push(node { name, sizes[i], nil })
    insertion_sort(entries) a, b:
        if (a.size < 0) == (b.size < 0): a.name < b.name else: a.size < b.size

def select(n:node):
    selected = n
    scan(n)

def unparent(n::node) -> void:
    parent = nil
    for children: _.unparent()

select(root)

def render_tree(n:node) -> float:
    var text = n.name
    if text == "": text = "(root)"
    let size = float(gl_text_size(text))
    var width = size.x
    gl_color(if n.size < 0: color_blue else: color_black):
        if gl_hit(size, 0):
            if gl_button("mouse1") == 1:
                select(n)
            gl_color color_light_grey:
                gl_rect(size)
        if selected == n:
            gl_color color_yellow:
                gl_rect(size)
        gl_text(text)
        gl_translate(xy_f { 0.0, fontsize })
        if n.size < 0:
            gl_translate xy_f { indentsize, 0.0 }
            for(n.children): width = max(width, indentsize + render_tree(_))
            gl_translate xy_f { -indentsize, 0.0 }
    return width

def render_entries():
    var newsel = nil
    for(entries) e:
        gl_color(if e.size < 0: color_blue else: color_black):
            let text = e.name
            let size = float(gl_text_size(text))
            if gl_hit(size, 0):
                if gl_button("mouse1") == 1:
                    let dups = filter selected.children: _.name == e.name
                    if dups.length:
                        newsel = dups[0]
                    else:
                        newsel = node { e.name, e.size, selected }
                        selected.children.push(newsel)
                gl_color color_light_grey:
                    gl_rect(size)
            gl_text(e.name)
            gl_translate xy_f { 0.0, fontsize }
    if newsel: select(newsel)

def render_sizes(old_width):
    var width = 0.0
    for(entries) e:
        gl_color color_dark_green:
            let text = if e.size < 0: "---" else: "{e.size}"
            let size = gl_text_size(text)
            width = max(width, size.x)
            gl_translate xy_f { old_width - size.x - column_margin, 0.0 }:
                gl_text(text)
            gl_translate xy_f { 0.0, fontsize }
    return width + column_margin

fatal(gl_window("dirbrowsing", 1024, 768))
check(gl_set_font_name("data/fonts/Droid_Sans_Mono/DroidSansMono.ttf") and
      gl_set_font_size(fontsize), "can\'t load font!")

var treesize = 400.0
var sizesize = 100.0
while(gl_frame() and gl_button("escape") != 1):
    gl_clear(color_white)
    gl_color(color_black)
    gl_color color_lightest_grey:
        gl_rect(xy_f { treesize, gl_window_size().y })
    gl_translate xy_f { column_margin, 0.0 }:
        treesize = render_tree(root) + column_margin * 2
    gl_translate xy_f { treesize, 0.0 }:
        sizesize = render_sizes(sizesize) + column_margin
    gl_translate xy_f { treesize + sizesize, 0.0 }:
        render_entries()

// the parent links cause cycles that our refcount VM can't handle. could GC it instead,
// but that could mask bugs:
unparent(root)
