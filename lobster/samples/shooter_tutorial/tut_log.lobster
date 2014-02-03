include "std.lobster"
include "vec.lobster"
include "color.lobster"


function log(def, f):
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
    
    

fatal(gl_window("Shooter Tutorial", 640, 480))

check(gl_setfontname("data/fonts/US101/US101.TTF") & gl_setfontsize(32), "can\'t load font!")

worldsize :== 20.0

playerpos := nil
playerspeed :== 10
playerhealth := 0.0

struct bullet: [ pos, dir ]

firerate :== 0.1
bulletspeed :== 15
bullets := nil
lastbullet := nil

enemyrate := nil
enemyspeed :== 3
enemymaxhp :== 5
enemies := nil
lastenemy := nil

highscore := 0


while(gl_frame() & !gl_wentdown("escape")):

    log_frame()

    gl_clear(color_black)
    gl_color(color_white)
    
    log(false) playing:
    
        if(playing):
            log(0) score:
                function renderpointytriangle(pos, dir):
                    gl_translate(pos):
                        gl_rotate_z(dir):
                            gl_polygon([ [ -0.5, 0.5 ], vec_x, [ -0.5, -0.5 ] ])
                gl_text("health: " + ceiling(playerhealth) + " - score: " + score + " - highscore: " + highscore)
                gl_translate(gl_windowsize() / 2.0)
                scale := gl_windowsize().y / worldsize
                gl_scale(scale)
                
                dir := [ gl_isdown("d") - gl_isdown("a"),
                         gl_isdown("s") - gl_isdown("w") ]
                newpos := playerpos + normalize(dir) * gl_deltatime() * playerspeed
                if(!any(abs(newpos) > gl_windowsize() / scale / 2)):
                    playerpos = newpos
                    
                tomouse := normalize(gl_localmousepos(0) - playerpos)
                
                struct enemy: [ pos, hp ]

                if(lastenemy < gl_time()):
                    enemies.push([ sincos(rnd(360)) * worldsize * 2, enemymaxhp ]:enemy)
                    lastenemy += enemyrate
                    enemyrate *= 0.999
                    
                for(enemies) e:
                    playervec := playerpos - e.pos
                    playerdir := normalize(playervec)
                    e.pos += playerdir * gl_deltatime() * enemyspeed
                    for(bullets) b:
                        if(magnitude(b.pos - e.pos) < 1):
                            e.hp--
                            if(e.hp <= 0):
                                e.hp = 0
                                score++
                            b.pos = vec_x * worldsize * 10
                    if(magnitude(playervec) < 1):
                        playerhealth -= gl_deltatime() * 50
                        if(playerhealth <= 0):
                            playerhealth = 0
                            highscore = max(highscore, score)
                            playing = false
                    gl_color(lerp(color_red, color_blue, div(e.hp, enemymaxhp))):
                        renderpointytriangle(e.pos, playerdir)
                        
                enemies = filter(enemies): _.hp 
                
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
                
                renderpointytriangle(playerpos, tomouse)
        else:
            msg := "press space to play!"
            gl_translate((gl_windowsize() - gl_textsize(msg)) / 2):
                gl_text(msg)
            if(gl_wentdown("space")):
                playerhealth = 100.0
                playerpos = vec_0
                bullets = []
                lastbullet = gl_time()
                enemyrate = 1.0
                enemies = []
                lastenemy = gl_time()
                playing = true
            gl_text("highscore: " + highscore)

    
    