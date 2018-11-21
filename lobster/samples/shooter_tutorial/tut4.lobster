include std
include vec
include color

fatal(gl_window("Shooter Tutorial", 640, 480))

worldsize :== 20.0

playerpos := xy_0
playerspeed :== 10

struct bullet { pos:xy_f, dir:xy_f }

firerate :== 0.1
bulletspeed :== 15
bullets := []
lastbullet := gl_time()

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)
    gl_color(color_white)

    gl_translate(float(gl_window_size()) / 2.0)
    scale := gl_window_size().y / worldsize
    gl_scale(scale)

    dir := xy_f { (gl_button("d") >= 1) - (gl_button("a") >= 1),
                  (gl_button("s") >= 1) - (gl_button("w") >= 1) }
    newpos := playerpos + normalize(dir) * gl_delta_time() * playerspeed
    if !any(abs(newpos) > float(gl_window_size()) / scale / 2):
        playerpos = newpos

    tomouse := normalize(gl_local_mouse_pos(0) - playerpos)

    if lastbullet < gl_time():
        bullets.push(bullet { playerpos, tomouse })
        lastbullet += firerate

    for(bullets) b:
        b.pos += b.dir * gl_delta_time() * bulletspeed
        gl_translate b.pos:
            gl_color color_yellow:
                gl_circle(0.2, 20)

    bullets = filter(bullets) b:
        magnitude(b.pos) < worldsize * 2

    gl_translate gl_local_mouse_pos(0):
        gl_line_mode 1:
            gl_color color_grey:
                gl_circle(0.5, 20)

    gl_translate playerpos:
        gl_rotate_z tomouse:
            gl_polygon([ xy { -0.5, 0.5 }, xy_x, xy { -0.5, -0.5 } ])
