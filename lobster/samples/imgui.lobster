// simple demo showing integration of https://github.com/ocornut/imgui

include vec
include color

fatal(gl_window("imgui demo", 1280, 1024))

im_init(false)
assert im_add_font("data/fonts/Droid_Sans/DroidSans.ttf", 20.0)

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_grey)

    im_frame():
        // Show the built-in demo window, for testing.
        demo ?= false
        if demo: demo = im_window_demo()

        // A window with some widgets.
        im_window("Created by lobster code"):
            im_text("A text!")
            check ?= true
            check = im_checkbox("Check it out!", check)
            im_tooltip("This is worth checking out, really!")
            f ?= 0.5
            f = im_sliderfloat("Float", f, 0.0, 1.0)
            i ?= 3
            i = im_sliderint("Int", i, 0, 10)
            s ?= "edit me"
            s = im_input_text("Edit", s)
            r ?= 1
            items ?= ["apple", "banana", "pear"]
            r = im_radio(items, r, true)
            r = im_combo("Combo", items, r)
            r = im_listbox("ListBox", items, r)
            col ?= color_red
            col = im_coloredit("Color", col)
            im_separator()
            if !demo:
                im_button("show demo window"): demo = true
