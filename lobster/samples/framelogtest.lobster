
include "vec.lobster"
include "color.lobster"

fatal(gl_window("logtest", 1024, 1024))

function log1(def, f):
    x, xi := log_read(def)
    log_write(f(x), xi)
    x

function log2(def1, def2, f):
    x, xi := log_read(def1)
    y, yi := log_read(def2)
    xn, yn := f(x, y)
    log_write(xn, xi)
    log_write(yn, yi)
    return x, y

while(gl_frame()):
    
    log_frame()
    
    if(gl_wentdown("escape")): return
    
    gl_clear(color_black)
    gl_color(color_white)
    
    function a():
        log1(0): _ + 1
        
    function b():
        if(a() % 4):
            log1(0): _ + 1

    b()
    