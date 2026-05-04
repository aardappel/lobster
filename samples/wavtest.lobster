// example of how to play wav files

import std
import vec
import color
import gl

fatal(gl.window("wav sound test", 1024, 768))
check(gl.set_font_name("data/fonts/Carrois_Gothic_SC/CarroisGothicSC-Regular.ttf") and
      gl.set_font_size(30),
      "can\'t load font!")

let audio_freq = 44100
let sine = play_audio_stream(audio_freq, 1, 10)
let sine_freq = 440.0
let sine_vol = 0.5
var sine_t = 0.0
var stream_avail = 0.0
def sine_queue_secs(time):
    guard stream_avail < time
    let remaining = time - stream_avail
    let sample_count = int(audio_freq * remaining)
    let samples = map(sample_count):
        sine_t += 1.0 / float(audio_freq)
        sine_vol * sin(sine_t * sine_freq * 360.0)
    put_audio_stream(sine, samples)
    stream_avail += remaining

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    gl.color(color_white)
    gl.text("left mouse to shoot rifle, right mouse for shotgun, space for sine wave")
    if gl.button("mouse1") == 1:
        check(play_wav("data/sounds/awesund/rifle.wav"), "can\'t load sound!")
    if gl.button("mouse3") == 1:
        check(play_wav("data/sounds/awesund/shotgun.wav"), "can\'t load sound!")

    stream_avail = max(stream_avail - gl.delta_time(), 0.0)
    if gl.button("space") >= 1:
        sine_queue_secs(0.2)

