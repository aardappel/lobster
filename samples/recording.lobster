import std
import color
import gl
import vec

fatal(gl.window("sound recording test", 1024, 768))
check(gl.set_font_name("data/fonts/US101/US101.ttf") and gl.set_font_size(32), "can\'t load font!")

let ids, names = mic_devices()
check(ids.length, "No audio recording devices!")
let freq = 44100
let id = mic_start(ids[0], freq)

var frac = 0.0
let samples = []

while gl.frame():
    if gl.button("escape") == 1: return
    gl.clear(color_black)
    gl.text("Recording \"{names[0]}\" freq={freq}")

    let new_samples = mic_get(id)
    samples.append_into(new_samples)
    if samples.length >= freq:
        frac += gl.delta_time()
        if frac >= 1.0:
            frac -= 1.0
            samples.remove_range(0, freq)

    let size = gl.window_size()

    def map_x(x:int):
        let x_offset = frac * -float(size.x)
        return x_offset + lerp(0.0, float(size.x), float(x) / float(freq))
    def map_y(y:float):
        let vpad = size.y * 0.1
        let y_scale = 0.3
        return lerp(float(size.y) - vpad, vpad, ((y / y_scale + 1.0) * 0.5))

    gl.color(color_white)
    let zero = map_y(0.0)
    var last_x = 0
    var mn = zero
    var mx = zero
    for(samples) v, i:
        let x = int(map_x(i))
        let y = map_y(v)
        guard x.in_range(size.x)
        if y > mx: mx = y
        if y < mn: mn = y
        if last_x != x:
            gl.line(float2 { float(x), mn }, float2 { float(x), mx }, 1.5)
            last_x = x
            mn = zero
            mx = zero

