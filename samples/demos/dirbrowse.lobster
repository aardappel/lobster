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
    let size = float(gl.text_size(text))
    var width = size.x
    gl.color(if n.size < 0: color_blue else: color_black):
        if gl.hit(size, 0):
            if gl.button("mouse1") == 1:
                select(n)
            gl.color color_light_grey:
                gl.rect(size)
        if selected == n:
            gl.color color_yellow:
                gl.rect(size)
        gl.text(text)
        gl.translate(float2 { 0.0, fontsize })
        if n.size < 0:
            gl.translate float2 { indentsize, 0.0 }
            for(n.children): width = max(width, indentsize + render_tree(_))
            gl.translate float2 { -indentsize, 0.0 }
    return width

def render_entries():
    var newsel = nil
    for(entries) e:
        gl.color(if e.size < 0: color_blue else: color_black):
            let text = e.name
            let size = float(gl.text_size(text))
            if gl.hit(size, 0):
                if gl.button("mouse1") == 1:
                    let dups = filter selected.children: _.name == e.name
                    if dups.length:
                        newsel = dups[0]
                    else:
                        newsel = node { e.name, e.size, selected }
                        selected.children.push(newsel)
                gl.color color_light_grey:
                    gl.rect(size)
            gl.text(e.name)
            gl.translate float2 { 0.0, fontsize }
    if newsel: select(newsel)

def render_sizes(old_width):
    var width = 0.0
    for(entries) e:
        gl.color color_dark_green:
            let text = if e.size < 0: "---" else: "{e.size}"
            let size = gl.text_size(text)
            width = max(width, size.x)
            gl.translate float2 { old_width - size.x - column_margin, 0.0 }:
                gl.text(text)
            gl.translate float2 { 0.0, fontsize }
    return width + column_margin

fatal(gl.window("dirbrowsing", 1024, 768))
check(gl.set_font_name("data/fonts/Droid_Sans_Mono/DroidSansMono.ttf") and
      gl.set_font_size(fontsize), "can\'t load font!")

var treesize = 400.0
var sizesize = 100.0
while(gl.frame() and gl.button("escape") != 1):
    gl.clear(color_white)
    gl.color(color_black)
    gl.color color_lightest_grey:
        gl.rect(float2 { treesize, gl.window_size().y })
    gl.translate float2 { column_margin, 0.0 }:
        treesize = render_tree(root) + column_margin * 2
    gl.translate float2 { treesize, 0.0 }:
        sizesize = render_sizes(sizesize) + column_margin
    gl.translate float2 { treesize + sizesize, 0.0 }:
        render_entries()

// the parent links cause cycles that our refcount VM can't handle. could GC it instead,
// but that could mask bugs:
unparent(root)
