import std
import vec
import color
import gl

fatal(gl_window("Shooter Tutorial", 640, 480))

check(gl_set_font_name("data/fonts/US101/US101.ttf") and gl_set_font_size(32), "can\'t load font!")

class Entity:
    pos:xy_f

    def renderpointytriangle(dir):
        gl_translate pos:
            gl_rotate_z dir:
                gl_polygon([ xy { -0.5, 0.5 }, xy_x, xy { -0.5, -0.5 } ])

class Player : Entity
    health = 100.0

    def update(bounds):
        let speed = 10
        let dir = xy_f { (gl_button("d") >= 1) - (gl_button("a") >= 1),
                        (gl_button("s") >= 1) - (gl_button("w") >= 1) }
        let newpos = pos + normalize(dir) * gl_delta_time() * speed
        if not any(abs(newpos) > bounds):
            pos = newpos
        let tomouse = normalize(gl_local_mouse_pos(0) - pos)
        renderpointytriangle(tomouse)
        return tomouse

    def damage(amount):
        health -= amount
        if health <= 0:
            health = 0

class Bullet : Entity
    dir:xy_f
    alive = true

    def update(maxdist):
        let bulletspeed = 15
        pos += dir * gl_delta_time() * bulletspeed
        gl_translate pos:
            gl_color color_yellow:
                gl_circle(0.2, 20)
        if magnitude(pos) > maxdist:
            alive = false

class Enemy : Entity
    hp:int

    def update(player, bullets, maxhp):
        let enemyspeed = 3
        let playervec = player.pos - pos
        let playerdir = normalize(playervec)
        pos += playerdir * gl_delta_time() * enemyspeed
        for(bullets) b:
            if magnitude(b.pos - pos) < 1:
                hp = max(0, --hp)
                b.alive = false
        if magnitude(playervec) < 1:
            player.damage(gl_delta_time() * 50)
        gl_color lerp(color_red, color_blue, div(hp, maxhp)):
            renderpointytriangle(playerdir)

class World:
    player = Player { xy_0 }

    bullets = []
    lastbullet = gl_time()

    enemies = []

    score = 0

    def update():
        let worldsize = 20.0
        let firerate = 0.1
        let enemymaxhp = 5
        gl_translate float(gl_window_size()) / 2.0:
            let scale = gl_window_size().y / worldsize
            gl_scale(scale)

            let tomouse = player.update(float(gl_window_size()) / scale / 2)

            member lastenemy = gl_time()
            if lastenemy < gl_time():
                enemies.push(Enemy { sincos(rnd(360)) * worldsize * 2, enemymaxhp })
                member enemyrate = 1.0
                lastenemy += enemyrate
                enemyrate *= 0.999

            if lastbullet < gl_time():
                bullets.push(Bullet { player.pos, tomouse })
                lastbullet += firerate

            enemies = filter(enemies) e:
                e.update(player, bullets, enemymaxhp)
                if not e.hp: score++
                e.hp

            bullets = filter(bullets) b:
                b.update(worldsize * 2)
                b.alive

            gl_translate gl_local_mouse_pos(0):
                gl_line_mode 1:
                    gl_color color_grey:
                        gl_circle(0.5, 20)

        return player.health

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)
    gl_color(color_white)

    static highscore = 0
    static w = nil::World

    if w:
        gl_text("health: {ceiling(w.player.health)} - score: {w.score} - highscore: {highscore}")
        if not w.update():
            highscore = max(highscore, w.score)
            w = nil
    else:
        gl_text("highscore: {highscore}")
        let msg = "press space to play!"
        gl_translate float(gl_window_size() - gl_text_size(msg)) / 2:
            gl_text(msg)
        if gl_button("space") == 1:
            w = World {}
