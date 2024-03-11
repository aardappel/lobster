import std
import vec
import color
import gl

fatal(gl.window("Shooter Tutorial", 640, 480))

check(gl.set_font_name("data/fonts/US101/US101.ttf") and gl.set_font_size(32), "can\'t load font!")

class Entity:
    pos:float2

    def renderpointytriangle(dir):
        gl.translate pos:
            gl.rotate_z dir:
                gl.polygon([ float2 { -0.5, 0.5 }, float2_x, float2 { -0.5, -0.5 } ])

class Player : Entity
    health = 100.0

    def update(bounds):
        let speed = 10
        let dir = float2 { (gl.button("d") >= 1) - (gl.button("a") >= 1),
                        (gl.button("s") >= 1) - (gl.button("w") >= 1) }
        let newpos = pos + normalize(dir) * gl.delta_time() * speed
        if not any(abs(newpos) > bounds):
            pos = newpos
        let tomouse = normalize(gl.local_mouse_pos(0) - pos)
        renderpointytriangle(tomouse)
        return tomouse

    def damage(amount):
        health -= amount
        if health <= 0.0:
            health = 0.0

class Bullet : Entity
    dir:float2
    alive = true

    def update(maxdist):
        let bulletspeed = 15
        pos += dir * gl.delta_time() * bulletspeed
        gl.translate pos:
            gl.color color_yellow:
                gl.circle(0.2, 20)
        if magnitude(pos) > maxdist:
            alive = false

class Enemy : Entity
    hp:int

    def update(player, bullets, maxhp):
        let enemyspeed = 3
        let playervec = player.pos - pos
        let playerdir = normalize(playervec)
        pos += playerdir * gl.delta_time() * enemyspeed
        for(bullets) b:
            if magnitude(b.pos - pos) < 1.0:
                hp = max(0, --hp)
                b.alive = false
        if magnitude(playervec) < 1.0:
            player.damage(gl.delta_time() * 50.0)
        gl.color lerp(color_red, color_blue, div(hp, maxhp)):
            renderpointytriangle(playerdir)

class World:
    player = Player { float2_0 }

    bullets = []
    lastbullet = gl.time()

    enemies = []

    score = 0

    def update():
        let worldsize = 20.0
        let firerate = 0.1
        let enemymaxhp = 5
        gl.translate gl.window_size() / 2:
            let scale = gl.window_size().y / worldsize
            gl.scale(scale)

            let tomouse = player.update(float(gl.window_size()) / scale / 2.0)

            member lastenemy = gl.time()
            if lastenemy < gl.time():
                enemies.push(Enemy { sincos(rnd(360)) * worldsize * 2.0, enemymaxhp })
                member enemyrate = 1.0
                lastenemy += enemyrate
                enemyrate *= 0.999

            if lastbullet < gl.time():
                bullets.push(Bullet { player.pos, tomouse })
                lastbullet += firerate

            enemies = filter(enemies) e:
                e.update(player, bullets, enemymaxhp)
                if not e.hp: score++
                e.hp

            bullets = filter(bullets) b:
                b.update(worldsize * 2.0)
                b.alive

            gl.translate gl.local_mouse_pos(0):
                gl.line_mode 1:
                    gl.color color_grey:
                        gl.circle(0.5, 20)

        return player.health

while gl.frame() and gl.button("escape") != 1:
    gl.clear(color_black)
    gl.color(color_white)

    static highscore = 0
    static w = nil::World

    if w:
        gl.text("health: {ceiling(w.player.health)} - score: {w.score} - highscore: {highscore}")
        if not w.update():
            highscore = max(highscore, w.score)
            w = nil
    else:
        gl.text("highscore: {highscore}")
        let msg = "press space to play!"
        gl.translate (gl.window_size() - gl.text_size(msg)) / 2:
            gl.text(msg)
        if gl.button("space") == 1:
            w = World {}
