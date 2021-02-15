// Extension of shooter tutorial tut6 with background music, enemy sounds and "hot" pause

import std
import vec
import color
import gl

fatal(gl_window("Shooter Tutorial", 640, 480))

check(gl_set_font_name("data/fonts/US101/US101.ttf") and gl_set_font_size(32), "can\'t load font!")

// load music and sound files (for sound/music authorship informations, see AUTHORS.TXT)
let bgmusic = play_wav("data/sounds/Cyber-Dream-Loop short.wav", -1) // by Eric Matyas from https://soundimage.org/looping-music/
if bgmusic: sound_volume(bgmusic, 0.5) else: print "can\'t load music!"
let ufobuzz = play_wav("data/sounds/Alien_buzz2.wav", -1) // https://freesound.org/people/CommanderDerp/sounds/167921/ (Slenderman static..)
if ufobuzz: sound_volume(ufobuzz, 0.0) else: print "can\'t load sound!"

let worldsize = 20.0

var playerpos = xy_0
let playerspeed = 10
var playerhealth = 0.0

class bullet:
    pos:xy_f
    dir:xy_f

let firerate = 0.1
let bulletspeed = 15
var bullets = []
var lastbullet = 0.0

class enemy:
    pos:xy_f
    hp:int

var enemyrate = 1.0
let enemyspeed = 3
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
    gl_translate pos:
        gl_rotate_z dir:
            gl_polygon([ xy { -0.5, 0.5 }, xy_x, xy { -0.5, -0.5 } ])

while gl_frame() and gl_button("escape") != 1:
    gl_clear(color_black)
    gl_color(color_white)
    if playing:
        // status banner
        gl_text("health: " + ceiling(playerhealth) + " - score: " + score + " - highscore: " + highscore + " - space: un(pause)")
        // check if user pauses
        if gl_button("space") == 1:
            paused = not paused
            if paused:
                sound_pause(bgmusic)
                sound_pause(ufobuzz)
            else:
                sound_resume(bgmusic)
                sound_resume(ufobuzz)
        // pause time management
        if not paused:
            delta_time = gl_delta_time()
            game_time += delta_time
        else:
            delta_time = 0

        gl_translate float(gl_window_size()) / 2.0:
            let scale = gl_window_size().y / worldsize
            gl_scale(scale)

            let dir = xy_f { (gl_button("d") >= 1) - (gl_button("a") >= 1),
                          (gl_button("s") >= 1) - (gl_button("w") >= 1) }
            let newpos = playerpos + normalize(dir) * delta_time * playerspeed
            if not any(abs(newpos) > float(gl_window_size()) / scale / 2):
                playerpos = newpos

            let tomouse = normalize(gl_local_mouse_pos(0) - playerpos)

            if lastenemy < game_time:
                enemies.push(enemy { sincos(rnd(360)) * worldsize * 2, enemymaxhp })
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
                    if magnitude(b.pos - e.pos) < 1:
                        sound_volume(play_wav("data/sounds/ufo hit.wav"), clamp(1.0 / (playerdist + 0.001), 0.1, 1.0)) // distance-dependent volume of bullet hit
                        e.hp--
                        if(e.hp <= 0):
                            e.hp = 0
                            score++
                        b.pos = xy_x * worldsize * 10
                if playerdist < 1:
                    playerhealth -= delta_time * 50
                    if playerhealth <= 0:
                        playerhealth = 0
                        highscore = max(highscore, score)
                        sound_pause(ufobuzz)
                        playing = false
                gl_color lerp(color_red, color_blue, div(e.hp, enemymaxhp)):
                    renderpointytriangle(e.pos, playerdir)
            if not paused:
                // distance-dependent volume of the scary UFO buzz sound
                let enemy_volume = clamp(1.0 / (closest_enemy_dist * closest_enemy_dist + 0.001), 0.0, 1.0)
                sound_volume(ufobuzz, enemy_volume)

            enemies = filter(enemies): _.hp

            for(bullets) b:
                b.pos += b.dir * delta_time * bulletspeed
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
        let msg = "press space to play!"
        gl_translate float(gl_window_size() - gl_text_size(msg)) / 2:
            gl_text(msg)
        if gl_button("space") == 1:
            score = 0
            playerhealth = 100.0
            playerpos = xy_0
            bullets = []
            enemyrate = 1.0
            enemies = []
            playing = true
            paused = false
            sound_resume(bgmusic)
            sound_resume(ufobuzz)
            sound_volume(ufobuzz, 0)
