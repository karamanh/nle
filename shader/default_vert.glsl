#version 330

#define NLE_MAX_JOINTS 64

out vec4 io_vertex_color;
out vec2 io_texture_coordinates;
out vec3 io_normal;
out vec3 io_frag_position;

uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;

// Skinning. u_joint_matrices holds, for the skin this draw belongs to,
// joint_world_matrix * inverse_bind_matrix for every joint.
uniform int u_skinning_enabled;
uniform mat4 u_joint_matrices[NLE_MAX_JOINTS];

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 texture;
layout (location = 4) in vec4 joints;
layout (location = 5) in vec4 weights;

int joint_index(float raw)
{
    // Clamped so malformed weights can never index outside the array.
    return int(clamp(raw, 0.0, float(NLE_MAX_JOINTS - 1)));
}

mat4 skin_matrix()
{
    float total = weights.x + weights.y + weights.z + weights.w;

    if (total <= 0.0)
    {
        return mat4(1.0);
    }

    mat4 skin = weights.x * u_joint_matrices[joint_index(joints.x)]
              + weights.y * u_joint_matrices[joint_index(joints.y)]
              + weights.z * u_joint_matrices[joint_index(joints.z)]
              + weights.w * u_joint_matrices[joint_index(joints.w)];

    return skin / total;
}

void main() {
    mat4 model = u_model;

    if (u_skinning_enabled == 1)
    {
        model = model * skin_matrix();
    }

    vec4 world_position = model * vec4(position, 1.0);

    gl_Position = u_projection * u_view * world_position;
    io_texture_coordinates = texture;
    io_normal = mat3(transpose(inverse(model))) * normal;
    io_frag_position = world_position.xyz;
    io_vertex_color = vec4(color, 1.0f);
}
