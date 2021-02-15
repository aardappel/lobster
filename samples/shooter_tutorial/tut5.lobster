import std
import vec
import color
import gl

fatal(gl_window("Shooter Tutorial", 640, 480))

let worldsize = 20.0

var playerpos = xy_0
let playerspeed = 10

class bullet:
    pos:xy_f
    dir:xy_f

let firerate = 0.1
let bulletspeed = 15
var bullets = []
var lastbullet = gl_time()

class enemy:
    pos:xy_f
    hp:int

var enemyrate = 1.0
let enemyspeed = 3
let enemymaxhp = 5
var enemies = []
var lastenemy = gl_time()

def renderpointytriangle(pos, dir):
    gl_translate pos:
        gl_rotate_z dir:
            gl_polygon([ xy { -0.5, 0.5 }, xy_x, xy { -0.5, -0.5 } ])

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)
    gl_color(color_white)

    gl_translate(float(gl_window_size()) / 2.0)
    let scale = gl_window_size().y / worldsize
    gl_scale(scale)

    let dir = xy_f { (gl_button("d") >= 1) - (gl_button("a") >= 1),
                  (gl_button("s") >= 1) - (gl_button("w") >= 1) }
    let newpos = playerpos + normalize(dir) * gl_delta_time() * playerspeed
    if not any(abs(newpos) > float(gl_window_size()) / scale / 2):
        playerpos = newpos

    let tomouse = normalize(gl_local_mouse_pos(0) - playerpos)

    if lastenemy < gl_time():
        enemies.push(enemy { sincos(rnd(360)) * worldsize * 2, enemymaxhp })
        lastenemy += enemyrate
        enemyrate *= 0.999

    if lastbullet < gl_time():
        bullets.push(bullet { playerpos, tomouse })
        lastbullet += firerate

    for(enemies) e:
        let playerdir = normalize(playerpos - e.pos)
        e.pos += playerdir * gl_delta_time() * enemyspeed
        for(bullets) b:
            if magnitude(b.pos - e.pos) < 1:
                e.hp = max(e.hp - 1, 0)
                b.pos = xy_x * worldsize * 10
        gl_color lerp(color_red, color_blue, div(e.hp, enemymaxhp)):
            renderpointytriangle(e.pos, playerdir)

    enemies = filter(enemies): _.hp

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

    renderpointytriangle(playerpos, tomouse)
