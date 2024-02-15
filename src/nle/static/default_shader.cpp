#include "default_shader.h"

namespace nle
{

    const char *k_default_vertex_shader = 
R"(#version 330

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
})";

    const char *k_default_fragment_shader = 
R"(#version 330

in vec4 io_vertex_color;
in vec2 io_texture_coordinates;
in vec3 io_normal;
in vec3 io_frag_position;

out vec4 io_color;

struct DirectionalLight
{
    vec3 color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
};

struct Material
{
    float shininess;
    float dissolve;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Sky
{
    int distance_fog_enabled;
    float distance_fog_near;
    float distance_fog_far;
};

struct PointLight
{
    vec3 position;
    vec4 color;
    float distance_min;
    float distance_max;
    int enabled;
};

uniform int u_lighting_enabled = 1;
uniform int u_texture_enabled = 1;
uniform int u_fog_enabled = 1;
uniform sampler2D u_texture_0;
uniform DirectionalLight u_directional_light;
uniform Material u_material;
uniform Sky u_sky;
uniform vec3 u_eye_position;

float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

PointLight create_point_light(vec3 position, vec4 color)
{
    PointLight pl;
    pl.position = position;
    pl.color = color;
    pl.distance_min = 0.0;
    pl.distance_max = 5.0;
    pl.enabled = 1;
    return pl;
}

vec4 fog_factor() {
    vec3 dist = u_eye_position - io_frag_position;
    float fog_factor = (distance(u_eye_position, io_frag_position) - u_sky.distance_fog_near) / (u_sky.distance_fog_far - u_sky.distance_fog_near);
    fog_factor = clamp(fog_factor, 0.0, 1.0);
    return vec4(fog_factor);
}

vec4 point_light_factor(PointLight pl)
{
    vec4 plf;

    vec3 diff = pl.position - io_frag_position;
    float dist = length(diff);                                            
    float distance_factor = 1.0 / map(dist, pl.distance_min, pl.distance_max, 0.0, 1.0);
    float light_factor = max(dot(io_normal, normalize(diff)), 0.0); 
    plf += pl.color * light_factor * distance_factor;

    return plf;
}

void main() {

    vec4 light_factor = vec4(1.f);
    vec3 view_direction = normalize(u_eye_position - io_frag_position);
    vec3 refl = reflect(view_direction, normalize(io_normal));
    // io_color = vec4(texture(u_sky_sampler, refl).rgb, 1.0);

    if(u_lighting_enabled == 1)
    {
        vec3 norm = normalize(io_normal);
        // ambient
        vec3 ambient = u_directional_light.color * u_material.ambient;
        // diffuse
        float diffuse_factor = max(dot(norm, u_directional_light.direction), 0.0f);
        vec3 diffuse = u_material.diffuse * u_directional_light.diffuse * diffuse_factor;
        // specular
        vec3 reflect_direction = reflect(-u_directional_light.direction, norm);
        float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0f), u_material.shininess);
        vec3 specular = u_material.specular * u_directional_light.specular * specular_factor;
        

        light_factor = vec4((ambient + diffuse + specular), 1.0f);
    }

    if(u_texture_enabled == 1)
    {
        io_color = texture(u_texture_0, io_texture_coordinates) * light_factor;
    }
    else
    {
        io_color = io_vertex_color * light_factor;
    }

    // io_color = gl_FragColor * (1.0 - u_material.dissolve) + io_color * (u_material.dissolve);
    if(u_sky.distance_fog_enabled == 1)
    {
        io_color.a = u_material.dissolve;
        io_color += fog_factor();
    }

    PointLight pl0 = create_point_light(u_eye_position, vec4(1.0, 0.0, 0.0, 1.0));
    if(pl0.enabled == 1)
    {
        io_color += point_light_factor(pl0);
    }
})";

} // namespace nle
