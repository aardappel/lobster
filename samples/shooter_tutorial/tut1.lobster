import vec
import color
import gl

fatal(gl.window("Shooter Tutorial", 640, 480))

while gl.frame() and gl.button("escape") != 1:
    gl.clear(color_black)


