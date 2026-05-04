import std
import color
import gl
import vec

fatal(gl.window("sound recording test", 1024, 768))
check(gl.set_font_name("data/fonts/US101/US101.ttf") and gl.set_font_size(32), "can\'t load font!")

let ids, names = mic_devices()
check(ids.length, "No audio recording devices!")
let freq = 48000
let channels = 1
let frame_size = 480

let encoder = opus.encoder_create(freq, 1)
let decoder = opus.decoder_create(freq, 1)

var state = 0
let state_names = ["Idle", "Recording", "Playback"]
let packets = []
var mic_id = 0
var stream_id = 0

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    gl.text("{state_names[state]} - \"{names[0]}\"")

    let space = gl.button("space")
    switch state:
        case 0:
            if space == 1:
                mic_id = mic_start(ids[0], freq)
                state = 1
        case 1:
            if space > 1:
                let new_samples = mic_get(mic_id)
                let new_packets = opus.encode_float(encoder, new_samples, frame_size)
                packets.append_into(new_packets)
            else:
                mic_stop(mic_id)
                mic_id = 0
                state = 2
                let lens = (map(packets):_.length)
                print "Recorded into {packets.length} packets:{lens} (total={sum(lens)})"
        case 2:
            stream_id = play_audio_stream(freq, 1, 10)
            let out_samples = opus.decode_float(decoder, packets, frame_size)
            put_audio_stream(stream_id, out_samples)
            packets.truncate(0)
            state = 0

