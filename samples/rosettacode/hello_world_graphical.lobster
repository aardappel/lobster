import color
import gl

gl.window("graphical hello world", 800, 600)
gl.set_font_name("data/fonts/Droid_Sans/DroidSans.ttf")
gl.set_font_size(30)

while gl.frame():
    gl.clear(color_black)
    gl.text("Goodbye, World!")
