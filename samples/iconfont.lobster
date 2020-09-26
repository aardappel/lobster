// Use the font from https://seiyria.com/gameicons-font/
// Files: https://github.com/seiyria/gameicons-font
// Original SVGs: https://game-icons.net/

import std
import vec
import color
import gl

gl_window("testing icons", 1200, 600)
gl_set_font_name("data/fonts/Game_Icons/game-icons.ttf")
gl_set_font_size(100)

// If you wanted to see what names are all in this font.
// for(filter(map(10000): gl_get_glyph_name(_)): _.length): print _

// pick some icons we like:
let names = [ "cactus", "ghost", "hydra", "leg-armor", "lever", "lion", "linden-leaf", "mac-10",
              "monkey", "poison-bottle", "refinery", "rose", "shambling-zombie" ]

let indices = map(names) name: assert gl_get_char_code(name)
let str = unicode_to_string(indices)

while gl_frame():
    gl_clear(color_black)
    gl_translate xy_1 * 50
    gl_text(str)
