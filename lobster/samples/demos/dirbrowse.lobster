include "std.lobster"
include "vec.lobster"
include "color.lobster"

include "unittest.lobster"

indentsize := 20
fontsize := 20

struct node { name:string, size:int, parent:node?, children:[node] }

root := node { "", -1, nil, [] }
selected := root
entries := []

def fullpath(n::node):
    if parent: fullpath(parent) + "/" + name else: name

def scan(n:node):
    entries = []
    names, sizes := scan_folder(n.fullpath, 1024)
    if names and sizes:
        for(names) name, i:
            entries.push(node { name, sizes[i], nil, [] })
    insertion_sort(entries) a, b:
        if ((a.size < 0) == (b.size < 0)): a.name < b.name else: a.size < b.size

def select(n:node):
    selected = n
    scan(n)

def unparent(n::node):
    parent = nil
    for(children): unparent(_)

select(root)

def render_tree(n:node):
    gl_color(if n.size < 0: color_blue else: color_black):
        text := n.name
        if text == "": text = "(root)"
        // if(n.size >= 0) text += ": " + n.size
        size := float(gl_textsize(text))
        if gl_hit(size, 0):
            if gl_button("mouse1") == 1:
                select(n)
            gl_color color_light_grey:
                gl_rect(size)
        if selected == n:
            gl_color color_yellow:
                gl_rect(size)
        gl_text(text)
        gl_translate([ 0.0, fontsize ])
        if n.size < 0:
            gl_translate([ indentsize, 0.0 ])
            for(n.children): render_tree(_)
            gl_translate([ -indentsize, 0.0 ])

def render_entries():
    for(entries) e:
        gl_color(if e.size < 0: color_blue else: color_black):
            text := e.name
            size := float(gl_textsize(text))
            if gl_hit(size, 0):
                if gl_button("mouse1") == 1:
                    dups := filter selected.children: _.name == e.name
                    if dups.length:
                        select(dups[0])
                    else:
                        n := node { e.name, e.size, selected, [] }
                        selected.children.push(n)
                        select(n)
                gl_color color_light_grey:
                    gl_rect(size)
            gl_text(e.name)
            gl_translate([ 0.0, fontsize ])

def render_sizes():
    for(entries) e:
        gl_color color_dark_green:
            text := if e.size < 0: "---" else: e.size + 1 + "K"
            size := gl_textsize(text)
            gl_translate [ 90.0 - size.x, 0.0 ]:
                gl_text(text)
            gl_translate([ 0.0, fontsize ])

fatal(gl_window("dirbrowsing", 1024, 768))
check(gl_setfontname("data/fonts/Droid_Sans_Mono/DroidSansMono.ttf") and gl_setfontsize(fontsize), "can\'t load font!")

while(gl_frame() and gl_button("escape") != 1):
    gl_clear(color_white)
    gl_color(color_black)
    gl_color color_lightest_grey:
        gl_rect([ 400.0, gl_windowsize().y ])
    gl_translate xy_0:
        render_tree(root)
    gl_translate [ 400.0, 0.0 ]:
        render_sizes()
    gl_translate [ 500.0, 0.0 ]:
        render_entries()

// the parent links cause cycles that our refcount VM can't handle. could GC it instead, but that could mask bugs:
unparent(root)

