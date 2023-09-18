// Extension of shooter tutorial tut6 with background music, enemy sounds and "hot" pause

import std
import vec
import color
import gl

fatal(gl.window("Shooter Tutorial", 640, 480))

check(gl.set_font_name("data/fonts/US101/US101.ttf") and gl.set_font_size(32), "can\'t load font!")

// load music and sound files (for sound/music authorship informations, see AUTHORS.TXT)
let bgmusic = play_ogg("data/sounds/tutorial/Cyber-Dream-Loop short.ogg", -1) // by Eric Matyas from https://soundimage.org/looping-music/
if bgmusic: sound_volume(bgmusic, 0.5) else: print "can\'t load music!"
let ufobuzz = play_ogg("data/sounds/tutorial/alien_buzz2.ogg", -1) // https://freesound.org/people/CommanderDerp/sounds/167921/ (Slenderman static..)
if ufobuzz: sound_volume(ufobuzz, 0.0) else: print "can\'t load sound!"

let worldsize = 20.0

var playerpos = float2_0
let playerspeed = 10.0
var playerhealth = 0.0

class bullet:
    pos:float2
    dir:float2

let firerate = 0.1
let bulletspeed = 15.0
var bullets = []
var lastbullet = 0.0

class enemy:
    pos:float2
    hp:int

var enemyrate = 1.0
let enemyspeed = 3.0
let enemymaxhp = 5
var enemies = []
var lastenemy = 0.0
var closest_enemy_dist = 9999.0

var score = 0
var highscore = 0
var playing = false
var paused = false
var game_time = 0.0
var delta_time = 0.0

def renderpointytriangle(pos, dir):
    gl.translate pos:
        gl.rotate_z dir:
            gl.polygon([ float2 { -0.5, 0.5 }, float2_x, float2 { -0.5, -0.5 } ])

while gl.frame() and gl.button("escape") != 1:
    gl.clear(color_black)
    gl.color(color_white)
    if playing:
        // status banner
        gl.text("health: {ceiling(playerhealth)} - score: {score} - highscore: {highscore}")
        // check if user pauses
        if gl.button("space") == 1:
            paused = not paused
            if paused:
                sound_pause(bgmusic)
                sound_pause(ufobuzz)
            else:
                sound_resume(bgmusic)
                sound_resume(ufobuzz)
        // pause time management
        if not paused:
            delta_time = gl.delta_time()
            game_time += delta_time
        else:
            delta_time = 0.0

        gl.translate gl.window_size() / 2:
            let scale = gl.window_size().y / worldsize
            gl.scale(scale)

            let dir = float2 { (gl.button("d") >= 1) - (gl.button("a") >= 1),
                          (gl.button("s") >= 1) - (gl.button("w") >= 1) }
            let newpos = playerpos + normalize(dir) * delta_time * playerspeed
            if not any(abs(newpos) > float(gl.window_size()) / scale / 2.0):
                playerpos = newpos

            let tomouse = normalize(gl.local_mouse_pos(0) - playerpos)

            if lastenemy < game_time:
                enemies.push(enemy { sincos(rnd(360)) * worldsize * 2.0, enemymaxhp })
                lastenemy += enemyrate
                enemyrate *= 0.999

            if lastbullet < game_time:
                bullets.push(bullet { playerpos, tomouse })
                lastbullet += firerate

            closest_enemy_dist = 9999.0
            for(enemies) e:
                let playervec = playerpos - e.pos
                let playerdist = magnitude(playervec)
                let playerdir = normalize(playervec)
                if playerdist < closest_enemy_dist:
                    closest_enemy_dist = playerdist
                e.pos += playerdir * delta_time * enemyspeed
                for(bullets) b:
                    if magnitude(b.pos - e.pos) < 1.0:
                        sound_volume(play_ogg("data/sounds/tutorial/ufo hit.ogg"), clamp(1.0 / (playerdist + 0.001), 0.1, 1.0)) // distance-dependent volume of bullet hit
                        e.hp--
                        if(e.hp <= 0):
                            e.hp = 0
                            score++
                        b.pos = float2_x * worldsize * 10.0
                if playerdist < 1.0:
                    playerhealth -= delta_time * 50.0
                    if playerhealth <= 0.0:
                        playerhealth = 0.0
                        highscore = max(highscore, score)
                        sound_pause(ufobuzz)
                        playing = false
                gl.color lerp(color_red, color_blue, div(e.hp, enemymaxhp)):
                    renderpointytriangle(e.pos, playerdir)
            if not paused:
                // distance-dependent volume of the scary UFO buzz sound
                let enemy_volume = clamp(1.0 / (closest_enemy_dist * closest_enemy_dist + 0.001), 0.0, 1.0)
                sound_volume(ufobuzz, enemy_volume)

            enemies = filter(enemies): _.hp

            for(bullets) b:
                b.pos += b.dir * delta_time * bulletspeed
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
    else:
        let msg = "press space to play!"
        gl.translate (gl.window_size() - gl.text_size(msg)) / 2:
            gl.text(msg)
        if gl.button("space") == 1:
            score = 0
            playerhealth = 100.0
            playerpos = float2_0
            bullets = []
            enemyrate = 1.0
            enemies = []
            playing = true
            paused = false
            sound_resume(bgmusic)
            sound_resume(ufobuzz)
            sound_volume(ufobuzz, 0.0)
