// example of how to play wav files

include "std.lobster"
include "vec.lobster"
include "color.lobster"

fatal(gl_window("wav sound test", 1024, 768))
check(gl_setfontname("data/fonts/Carrois_Gothic_SC/CarroisGothicSC-Regular.ttf") and
      gl_setfontsize(30),
      "can\'t load font!")

while gl_frame():
    if gl_button("escape") == 1: return
    gl_clear(color_black)
    gl_color(color_white)
    gl_text("left mouse to shoot rifle, right mouse for shotgun")
    if gl_button("mouse1") == 1:
        check(play_wav("data/sounds/awesund/rifle.wav"), "can\'t load sound!")
    if gl_button("mouse3") == 1:
        check(play_wav("data/sounds/awesund/shotgun.wav"), "can\'t load sound!")



