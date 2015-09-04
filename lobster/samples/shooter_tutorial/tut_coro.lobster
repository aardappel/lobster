include "std.lobster"
include "vec.lobster"
include "color.lobster"

mode := coroutine menu(0, 0)

def menu(lastscore, highscore, frame):
    forever():
        frame()
        msg := "last score: " + lastscore + " - highscore: " + highscore + " - press space to play!"
        gl_translate float(gl_windowsize() - gl_textsize(msg)) / 2:
            gl_text(msg)
        if gl_wentdown("space"):
            mode = coroutine level(highscore)

def level(highscore, frame):
    score := 0

    worldsize :== 20.0

    playerhealth := 100.0
    playerpos := xy_0
    playerspeed :== 10

    bs := coroutine bulletset()
    es := coroutine enemyset()

    def renderpointytriangle(pos, dir):
        gl_translate pos:
            gl_rotate_z dir:
                gl_polygon([ [ -0.5, 0.5 ], xy_x, [ -0.5, -0.5 ] ])

    def bullet(pos, dir, frame):
        bulletspeed :== 15
        while magnitude(pos) < worldsize * 2:
            frame()
            pos += dir * gl_deltatime() * bulletspeed
            gl_translate pos:
                gl_color color_yellow:
                    gl_circle(0.2, 20)

    def bulletset(frame):
        firerate :== 0.1
        lastbullet := gl_time()
        bullets := []
        forever():
            dir := frame() | xy_0
            if lastbullet < gl_time():
                bullets.push(coroutine bullet(playerpos, dir))
                lastbullet += firerate

            bullets = filter(bullets) b:
                b.resume()
                b.active

    def enemy(pos, frame):
        speed :== 3
        maxhp :== 5
        hp := maxhp
        forever():
            frame()
            playervec := playerpos - pos
            playerdir := normalize(playervec)
            pos += playerdir * gl_deltatime() * speed
            for(bs->bullets) b:
                if magnitude(b->pos - pos) < 1:
                    hp--
                    b->pos = xy_x * worldsize * 10
                    if hp <= 0:
                        score++
                        return
            if magnitude(playervec) < 1:
                playerhealth -= gl_deltatime() * 50
            gl_color(lerp(color_red, color_blue, div(hp, maxhp))):
                renderpointytriangle(pos, playerdir)

    def enemyset(frame):
        enemies := []
        enemyrate := 1.0
        lastenemy := gl_time()
        forever():
            frame()
            if lastenemy < gl_time():
                enemies.push(coroutine enemy(sincos(rnd(360)).xy * worldsize * 2))
                lastenemy += enemyrate
                enemyrate *= 0.999
            enemies = filter(enemies) e:
                e.resume()
                e.active

    while playerhealth > 0:
        frame()
        gl_translate float(gl_windowsize()) / 2.0:
            scale := gl_windowsize().y / worldsize
            gl_scale(scale)

            dir := xy_f { gl_isdown("d") - gl_isdown("a"),
                          gl_isdown("s") - gl_isdown("w") }
            newpos := playerpos + normalize(dir) * gl_deltatime() * playerspeed
            if !any(abs(newpos) > float(gl_windowsize()) / scale / 2):
                playerpos = newpos

            tomouse := normalize(gl_localmousepos(0) - playerpos)

            es.resume()
            bs.resume(tomouse)

            gl_translate gl_localmousepos(0):
                gl_linemode 1:
                    gl_color color_grey:
                        gl_circle(0.5, 20)

            renderpointytriangle(playerpos, tomouse)
        gl_text("health: " + ceiling(playerhealth) + " - score: " + score + " - highscore: " + highscore)

    mode = coroutine menu(score, max(highscore, score))

fatal(gl_window("Shooter Tutorial", 640, 480))
check(gl_setfontname("data/fonts/US101/US101.TTF") & gl_setfontsize(32), "can\'t load font!")

while gl_frame() & !gl_wentdown("escape"):
    gl_clear(color_black)
    gl_color(color_white)
    mode.resume()
