#version 330

// Screen-space overlay. Positions are in pixels with the origin at the top
// left, which is what u_projection (an inverted-Y ortho) expects.

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;

uniform mat4 u_projection;

out vec2 io_uv;
out vec4 io_color;

void main() {
    gl_Position = u_projection * vec4(position, 0.0, 1.0);
    io_uv = uv;
    io_color = color;
}
