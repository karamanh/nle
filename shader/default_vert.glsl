#version 330

out vec4 vertexColor;
out vec2 textureCoordinates;
out vec3 normal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;

void main() {
    gl_Position = projection * view * model * vec4(pos, 1);
    // vertexColor = vec4(clamp(pos, 0.f, 1.f), 1.f);
    vertexColor = vec4(1.0f);
    textureCoordinates = tex;
    normal = mat3(transpose(inverse(model))) * norm;
    fragPos = (model * vec4(pos, 1.0)).xyz;
}