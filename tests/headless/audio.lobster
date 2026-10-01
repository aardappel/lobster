// Run from the Lobster root with SDL_AUDIO_DRIVER=dummy.
// Default: headless. -- --display: normal audio with a display.
// -- --preinit: audio started before headless gl.window must be shut down too.
// Use nonexistent SDL_VIDEO_DRIVER and SDL_AUDIO_DRIVER values for an additional
// default-mode run: headless must need neither device backend.
import gl

let args = command_line_arguments()
let mode = if args.length: args[0] else: ""
let display = mode == "--display"
// Asset paths are relative to this test file.
let wav = "../../data/sounds/awesund/shotgun.wav"
let ogg = "../../data/sounds/tutorial/ufo hit.ogg"
assert exists_file(wav) and exists_file(ogg)

if mode == "--preinit":
    assert play_wav(wav, -1) > 0
    assert play_music(ogg, -1) > 0
    assert play_audio_stream(48000, 1) > 0
    // This queued text must be discarded when headless mode is selected.
    text_to_speech("This must not be spoken.")

fatal(gl.window("Headless audio regression", 320, 240,
               if display: gl.window_init_no_vsync else: gl.window_init_headless))
assert gl.has_display() == display

if display:
    assert load_wav(wav)
    assert load_ogg(ogg)
    let channel = play_wav(wav, -1)
    assert channel > 0
    assert sound_status(channel) == 1
    assert sound_time_length(channel) > 0.0
    sound_halt(channel)
    assert sound_status(channel) == 0
    let music = play_music(ogg, -1)
    assert music > 0 and music_is_playing(music)
    music_halt(music)
    let stream = play_audio_stream(48000, 1)
    assert stream > 0 and has_audio_stream(stream)
    assert put_audio_stream(stream, map(480): 0.0)
    sound_halt(stream)
else:
    for(2):
        assert not load_wav(wav)
        assert not load_ogg(ogg)
        assert not load_sfxr("unused-headless.sfs")
        assert play_wav(wav) == 0
        assert play_ogg(ogg) == 0
        assert play_sfxr("unused-headless.sfs") == 0
        assert play_music(ogg) == 0
        assert play_music_fade_in(ogg, 100) == 0
        assert play_music_cross_fade(1, ogg, 100) == 0
        assert play_audio_stream(48000, 1) == 0
        assert not put_audio_stream(1, [0.0])
        assert not has_audio_stream(1)
        assert audio_stream_available(1) == 0
        assert sound_status(1) == -1
        assert sound_time_length(1) == 0.0
        assert not music_is_playing(1)
        sound_pause(1)
        sound_resume(1)
        sound_volume(1, 0.5)
        sound_position(1, float3 { 0.0, 0.0, 0.0 }, float3 { 0.0, 1.0, 0.0 }, 1.0)
        sound_no_position(1)
        sound_halt(1)
        music_pause(1)
        music_resume(1)
        music_volume(1, 0.5)
        music_fade_out(1, 100)
        music_halt(1)
        music_set_general_volume(0.5)
        let ids, names = mic_devices()
        assert not ids.length and not names.length
        assert mic_start(1, 48000) == 0
        assert not mic_get(1).length
        assert not mic_stop(1)
        text_to_speech("This must not be spoken either.")
        assert gl.frame()
    text_to_speech_stop()

print "audio regression passed (mode: {mode}, has_display: {gl.has_display()})"
