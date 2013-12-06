
include "vec.lobster"
include "color.lobster"

fatal(gl_window("logtest", 1024, 1024))

lines := []

while(gl_frame()):
    
    log_frame()
    
    if(gl_wentdown("escape")): return
    
    gl_clear(color_black)
    gl_color(color_white)
    
    function a():
        x := log_read(0) + 1
        log_write(x)
        //log_write(123)
        x
        
    function b():
        if(a() % 4):
            log_write(log_read(0) + 1)

    b()
    