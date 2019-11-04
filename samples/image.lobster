import gui
import texture

fatal(gl_window("Lobster", 400, 400))

let tex = gl_load_texture("data/textures/lobster.jpg")
assert tex

while gl_frame():
    gl_clear(color_black)

    gl_set_shader("textured")
    gl_set_primitive_texture(0, tex)

    // Lobster multiplies the texture by the color in context.
    // White preserves the original color.
    gl_color(color_white)

    // Center the image on the window.
    gl_translate(float(gl_window_size())/2)

    // Stretches the image vertically and horizontally to fill the window.
    gl_scale(float(gl_window_size())/2)

    // Draw a unit rectangle, as scaled and centered above, and filled with the
    // selected texture: the lobster image.
    // The second parameter renders the rectangle centered.
    gl_rect(xy_1, 1)
