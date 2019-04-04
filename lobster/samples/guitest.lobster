import gui

fatal(gl_window("gui test", 960, 640))
check(gl_set_font_name("data/fonts/US101/US101.ttf"), "can\'t load font!")
//check(gl_set_font_name("C:\\Windows\\Fonts\\arialuni.ttf"), "can\'t load font!")

let ghosttex = gl_load_texture("data/textures/lobster.jpg")
assert ghosttex

var clicked = false
var focused = false

while gl_frame():
    if gl_button("escape") == 1:
        return

    let fontsize = gl_window_size().y / 10
    gl_set_font_size(fontsize, fontsize / 40.0)
    //gl_set_max_font_size(512)

    gl_clear(color_grey)
    gl_color(color_white)

    gui_setclickcolors(color_dark_green, color_green)

    gui_start(gl_window_size(), 1, 1, true):
        gui_hblock(1, 0):
            //gui_fontscale(0.3):
                //   gui_text(unicode_to_string(map(500): _))

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
                    //gui_text(unicode_to_string([0x30E6, 0x30FC, 0x30B6, 0x30FC, 0x5225, 0x30B5, 0x30A4, 0x30C8]))
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