include "std.lobster"
include "vec.lobster"
include "color.lobster"

fatal(gl_window("Shooter Tutorial", 640, 480))

worldsize :== 20.0

playerpos := xy_0
playerspeed :== 10

struct bullet: [ pos, dir ]

firerate :== 0.1
bulletspeed :== 15
bullets := []
lastbullet := gl_time()

while(gl_frame() & !gl_wentdown("escape")):
    gl_clear(color_black)
    gl_color(color_white)

    gl_translate(gl_windowsize() / 2.0)
    scale := gl_windowsize().y / worldsize
    gl_scale(scale)

    dir := [ gl_isdown("d") - gl_isdown("a"),
             gl_isdown("s") - gl_isdown("w") ]:xy_f
    newpos := playerpos + normalize(dir) * gl_deltatime() * playerspeed
    if(!any(abs(newpos) > gl_windowsize() / scale / 2)):
        playerpos = newpos

    tomouse := normalize(gl_localmousepos(0) - playerpos)

    if(lastbullet < gl_time()):
        bullets.push([ playerpos, tomouse ]:bullet)
        lastbullet += firerate

    for(bullets) b:
        b.pos += b.dir * gl_deltatime() * bulletspeed
        gl_translate(b.pos):
            gl_color(color_yellow):
                gl_circle(0.2, 20)

    bullets = filter(bullets) b:
        magnitude(b.pos) < worldsize * 2

    gl_translate(gl_localmousepos(0)):
        gl_linemode(1):
            gl_color(color_grey):
                gl_circle(0.5, 20)

    gl_translate(playerpos):
        gl_rotate_z(tomouse):
            gl_polygon([ [ -0.5, 0.5 ], xy_x, [ -0.5, -0.5 ] ])
