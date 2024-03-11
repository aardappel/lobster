import std
import vec
import color
import gl

fatal(gl.window("Shooter Tutorial", 640, 480))

let worldsize = 20.0

var playerpos = float2_0
let playerspeed = 10

class bullet:
    pos:float2
    dir:float2

let firerate = 0.1
let bulletspeed = 15
var bullets = []
var lastbullet = gl.time()

class enemy:
    pos:float2
    hp:int

var enemyrate = 1.0
let enemyspeed = 3
let enemymaxhp = 5
var enemies = []
var lastenemy = gl.time()

def renderpointytriangle(pos, dir):
    gl.translate pos:
        gl.rotate_z dir:
            gl.polygon([ float2 { -0.5, 0.5 }, float2_x, float2 { -0.5, -0.5 } ])

while gl.frame() and gl.button("escape") != 1:
    gl.clear(color_black)
    gl.color(color_white)

    gl.translate(float(gl.window_size()) / 2.0)
    let scale = gl.window_size().y / worldsize
    gl.scale(scale)

    let dir = float2 { (gl.button("d") >= 1) - (gl.button("a") >= 1),
                  (gl.button("s") >= 1) - (gl.button("w") >= 1) }
    let newpos = playerpos + normalize(dir) * gl.delta_time() * playerspeed
    if not any(abs(newpos) > float(gl.window_size()) / scale / 2.0):
        playerpos = newpos

    let tomouse = normalize(gl.local_mouse_pos(0) - playerpos)

    if lastenemy < gl.time():
        enemies.push(enemy { sincos(rnd(360)) * worldsize * 2.0, enemymaxhp })
        lastenemy += enemyrate
        enemyrate *= 0.999

    if lastbullet < gl.time():
        bullets.push(bullet { playerpos, tomouse })
        lastbullet += firerate

    for(enemies) e:
        let playerdir = normalize(playerpos - e.pos)
        e.pos += playerdir * gl.delta_time() * enemyspeed
        for(bullets) b:
            if magnitude(b.pos - e.pos) < 1.0:
                e.hp = max(e.hp - 1, 0)
                b.pos = float2_x * worldsize * 10.0
        gl.color lerp(color_red, color_blue, div(e.hp, enemymaxhp)):
            renderpointytriangle(e.pos, playerdir)

    enemies = filter(enemies): _.hp

    for(bullets) b:
        b.pos += b.dir * gl.delta_time() * bulletspeed
        gl.translate b.pos:
            gl.color color_yellow:
                gl.circle(0.2, 20)

    bullets = filter(bullets) b:
        magnitude(b.pos) < worldsize * 2.0

    gl.translate gl.local_mouse_pos(0):
        gl.line_mode 1:
            gl.color color_grey:
                gl.circle(0.5, 20)

    renderpointytriangle(playerpos, tomouse)
