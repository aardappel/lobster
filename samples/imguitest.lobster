// simple demo showing integration of https://github.com/ocornut/imgui

import vec
import color
import gl
import imgui
import gl

fatal(gl_window("imgui demo", 1280, 1024))

im_init(false, im_config_docking_enable, 3.0)
assert im_add_font("data/fonts/Droid_Sans/DroidSans.ttf", 20.0)

// The data we're editing:
class Thingy:
    on = true
    i = 1
    ratio = 0.5
    name = "Fred"
    col = color_red
    pos = xy_1
    fruits = ["apple", "banana", "pear"]
    others:[Thingy]
    windowflag = im_window_always_autoresize

var d = Thingy { map(3): Thingy { [] } }
var demo = false
let foo = "bar"  // Will show up in debug view.

while gl_frame():
    gl_clear(color_grey)

    // <- Render game here.

    im_frame():
        // Maximum convenience: turn any data structure into editable UI!
        im_window("Automatically created from a class", d.windowflag):
            d = im_edit_anything(d)

        // Manually created widgets, so you can choose how to represent and layout data:
        im_window("Created by code", im_window_no_collapse | im_window_always_autoresize):
            im_text("A text!")
            d.on = im_checkbox("Check it out!", d.on)
            im_tooltip("This is worth checking out, really!")
            d.ratio = im_sliderfloat("Float", d.ratio, 0.0, 1.0)
            d.i = im_sliderint("Int", d.i, 0, 10)
            d.name = im_input_text("Name", d.name)
            d.i = im_radio(d.fruits, d.i, true)
            d.i = im_combo("Combo", d.fruits, d.i)
            d.i = im_listbox("ListBox", d.fruits, d.i, d.fruits.length)
            d.col = im_coloredit("Color", d.col)
            for(2) g: im_graph("Sin", map(20): sin(_ * 20.0) + 1.0, g)
            im_separator()
            if not demo and im_button("show demo window"): demo = true
            if im_button("Cause breakpoint() hit"):
                // The debugger should have popped up!
                // The program is frozen until you hit continue.
                // Try putting these breakpoints anywhere in your own code.
                breakpoint()

        // Show the built-in imgui demo window, for testing.
        if demo: demo = im_window_demo()

        // Realtime debugging! Show all lobster variables as editable UI.
        // Also show some engine stats, like e.g. a framerate graph.
        // These are shown in realtime, if you want to pause and see local
        // variables in functions, try breakpoint() above
        im_window("Realtime Debugger", im_window_always_autoresize):
            im_show_engine_stats()
            im_treenode("Memory Usage"):
                im_text(get_memory_usage(20))
            im_show_vars()
