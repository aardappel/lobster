// example of how to play wav files

import std
import vec
import color
import gl

fatal(gl.window("wav sound test", 1024, 768))
check(gl.set_font_name("data/fonts/Carrois_Gothic_SC/CarroisGothicSC-Regular.ttf") and
      gl.set_font_size(30),
      "can\'t load font!")

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    gl.color(color_white)
    gl.text("left mouse to shoot rifle, right mouse for shotgun")
    if gl.button("mouse1") == 1:
        check(play_wav("data/sounds/awesund/rifle.wav"), "can\'t load sound!")
    if gl.button("mouse3") == 1:
        check(play_wav("data/sounds/awesund/shotgun.wav"), "can\'t load sound!")



