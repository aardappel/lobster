gl_window("graphical hello world", 800, 600)
gl_setfontname("data/fonts/Droid_Sans/DroidSans.ttf")
gl_setfontsize(30)

while gl_frame():
    gl_clear([ 0.0, 0.0, 0.0, 1.0 ])
    gl_text("Goodbye, World!")
