#version 330 core

layout (location=0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 eyePosition;

out vec3 io_position;
out float io_render_scale;

void main()
{
    // float height =  distance(eyePosition, position);
    float height =  distance(eyePosition, vec3(eyePosition.x, 0.0, eyePosition.z));
    io_render_scale = height;
    io_position = position * io_render_scale;
    gl_Position = projection * view * model * vec4(io_position, 1.0);
}