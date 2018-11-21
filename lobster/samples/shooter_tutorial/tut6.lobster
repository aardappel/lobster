include std
include vec
include color

fatal(gl_window("Shooter Tutorial", 640, 480))

check(gl_set_font_name("data/fonts/US101/US101.TTF") and gl_set_font_size(32), "can\'t load font!")

worldsize :== 20.0

playerpos := xy_0
playerspeed :== 10
playerhealth := 0.0

struct bullet { pos:xy_f, dir:xy_f }

firerate :== 0.1
bulletspeed :== 15
bullets := []
lastbullet := gl_time()

struct enemy { pos:xy_f, hp:int }

enemyrate := 1.0
enemyspeed :== 3
enemymaxhp :== 5
enemies := []
lastenemy := gl_time()

score := 0
highscore := 0
playing := false

def renderpointytriangle(pos, dir):
    gl_translate pos:
        gl_rotate_z dir:
            gl_polygon([ xy { -0.5, 0.5 }, xy_x, xy { -0.5, -0.5 } ])

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)
    gl_color(color_white)

    if playing:
        gl_translate float(gl_window_size()) / 2.0:
            scale := gl_window_size().y / worldsize
            gl_scale(scale)

            dir := xy_f { (gl_button("d") >= 1) - (gl_button("a") >= 1),
                          (gl_button("s") >= 1) - (gl_button("w") >= 1) }
            newpos := playerpos + normalize(dir) * gl_delta_time() * playerspeed
            if !any(abs(newpos) > float(gl_window_size()) / scale / 2):
                playerpos = newpos

            tomouse := normalize(gl_local_mouse_pos(0) - playerpos)

            if lastenemy < gl_time():
                enemies.push(enemy { sincos(rnd(360)) * worldsize * 2, enemymaxhp })
                lastenemy += enemyrate
                enemyrate *= 0.999

            if lastbullet < gl_time():
                bullets.push(bullet { playerpos, tomouse })
                lastbullet += firerate

            for(enemies) e:
                playervec := playerpos - e.pos
                playerdir := normalize(playervec)
                e.pos += playerdir * gl_delta_time() * enemyspeed
                for(bullets) b:
                    if magnitude(b.pos - e.pos) < 1:
                        e.hp--
                        if(e.hp <= 0):
                            e.hp = 0
                            score++
                        b.pos = xy_x * worldsize * 10
                if magnitude(playervec) < 1:
                    playerhealth -= gl_delta_time() * 50
                    if playerhealth <= 0:
                        playerhealth = 0
                        highscore = max(highscore, score)
                        playing = false
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
    else:
        msg := "press space to play!"
        gl_translate float(gl_window_size() - gl_text_size(msg)) / 2:
            gl_text(msg)
        if gl_button("space") == 1:
            score = 0
            playerhealth = 100.0
            playerpos = xy_0
            bullets = []
            lastbullet = gl_time()
            enemyrate = 1.0
            enemies = []
            lastenemy = gl_time()
            playing = true

    gl_text("health: " + ceiling(playerhealth) + " - score: " + score + " - highscore: " + highscore)
