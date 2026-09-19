#version 330

in vec2 io_uv;
in vec4 io_color;

out vec4 io_fragment;

// Untextured quads sample a 1x1 white texture, so everything can share one
// batch instead of branching per quad.
uniform sampler2D u_texture_0;

void main() {
    io_fragment = texture(u_texture_0, io_uv) * io_color;
}
