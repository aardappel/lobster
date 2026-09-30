// Run from the Lobster root: bin/lobster tests/headless/headless.lobster
// Repeat with -- --display to exercise the same operations on a real GL context.
// For a stronger headless check, set SDL_VIDEO_DRIVER to a nonexistent driver.
import gl
import texture
import imgui

let args = command_line_arguments()
let display = args.length and args[0] == "--display"
assert not gl.has_display()
gl.require_version(4, 3)
fatal(gl.window("Headless/display regression", 320, 240,
               if display: gl.window_init_no_vsync else: gl.window_init_headless))
assert gl.has_display() == display
assert gl.visible()
assert gl.window_size().x > 0 and gl.window_size().y > 0
if not display:
    assert gl.window_size() == int2 { 320, 240 }
    gl.window_size(int2 { 400, 300 })
    assert gl.window_size() == int2 { 400, 300 }

let tex = gl.create_colored_texture(int3 { 32, 16, 0 }, color_blue, texture_format_nomipmap)
assert gl.texture_size(tex) == int2 { 32, 16 }
assert not gl.load_texture("nonexistent-headless-test-texture.png")
assert gl.switch_to_framebuffer(tex, true)
assert gl.framebuffer_size() == int2 { 32, 16 }
gl.clear(color_red)
if display:
    let pixels = assert gl.read_texture(tex)
    assert pixels.length == 32 * 16 * 4
    assert substring(pixels, 0, 4) == "\xFF\x00\x00\xFF"

let prev_pos, prev_size = gl.scissor(int2 { 2, 3 }, int2 { 10, 5 })
assert prev_pos == int2_0 and prev_size == int2 { 32, 16 }
let box_pos, box_size = gl.scissor(int2_0, int2 { 32, 16 })
// The current binding returns the GL (bottom-left) rectangle.
assert box_pos == int2 { 2, 8 } and box_size == int2 { 10, 5 }
assert gl.switch_to_framebuffer(nil)
assert gl.framebuffer_size() == gl.window_size()

fatal(gl.load_materials("""
COMPUTEFUNCTIONS
    layout(std430) buffer smoke_buffer { uint values[]; };
SHADER headless_smoke_compute
    COMPUTE
        LAYOUT 1 1 1
        values[0] += 1u;
""", true))
gl.set_shader("headless_smoke_compute")
var buf = assert gl.update_buffer_object("\x00\x00\x00\x00", true, -1, nil, true)
buf = assert gl.update_buffer_object("\x01\x00\x00\x00", true, 4, buf, true)
assert gl.bind_buffer_object("smoke_buffer", buf)
gl.dispatch_compute(int3_1)
let copy = assert gl.update_buffer_object("\x00\x00\x00\x00", true, -1, nil, false)
gl.copy_buffer_object(buf, copy, 4, 0, 4)
let tq = gl.create_time_query()

// Font replacement and complete ImGui recreation exercise its CPU texture lifecycle.
for(2):
    im.init(false, im.config_docking_enable, 3.0)
    assert im.add_font("data/fonts/Droid_Sans_Mono/DroidSansMono.ttf", 20.0)
    for(4) frame:
        assert gl.frame()
        assert gl.delta_time() > 0.0
        gl.start_time_query(tq)
        gl.clear(color_black)
        gl.set_shader("color")
        gl.color(color_green)
        gl.rect(float2 { 64.0, 64.0 })
        let elapsed = gl.stop_time_query(tq)
        if not display: assert elapsed == 0.0
        im.frame():
            im.window("CPU UI", im.window_none):
                im.text("Fonts, widgets, and frame state still run.")
                im.button("No input required")
        if frame == 1:
            im.clear_fonts()
            assert im.add_font("data/fonts/Droid_Sans_Mono/DroidSansMono.ttf", 24.0)
    im.cleanup()

print "headless/display regression passed (has_display: {gl.has_display()})"
