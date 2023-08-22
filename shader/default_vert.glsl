#version 330

out vec4 io_vertex_color;
out vec2 io_texture_coordinates;
out vec3 io_normal;
out vec3 io_frag_position;

uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 texture;

void main() {
    gl_Position = u_projection * u_view * u_model * vec4(position, 1);
    io_texture_coordinates = texture;
    io_normal = mat3(transpose(inverse(u_model))) * normal;
    io_frag_position = (u_model * vec4(position, 1.0)).xyz;
    io_vertex_color = vec4(color, 1.0f);
}