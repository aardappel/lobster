import gui
import texture

fatal(gl.window("Lobster", 400, 400))

let tex = gl.load_texture("data/textures/lobster.jpg")
assert tex

while gl.frame():
    gl.clear(color_black)

    gl.set_shader("textured")
    gl.set_primitive_texture(0, tex)

    // Lobster multiplies the texture by the color in context.
    // White preserves the original color.
    gl.color(color_white)

    // Center the image on the window.
    gl.translate(float(gl.window_size()) / 2.0)

    // Stretches the image vertically and horizontally to fill the window.
    gl.scale(float(gl.window_size()) / 2.0)

    // Draw a unit rectangle, as scaled and centered above, and filled with the
    // selected texture: the lobster image.
    // The second parameter renders the rectangle centered.
    gl.rect(float2_1, 1)
