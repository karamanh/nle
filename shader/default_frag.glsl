#version 330

in vec4 io_vertex_color;
in vec2 io_texture_coordinates;
in vec3 io_normal;
in vec3 io_frag_position;

out vec4 io_color;

struct DirectionalLight
{
    vec3 color;
    float ambient_intensity;
    vec3 direction;
    float diffuse_intensity;
};

struct Material
{
    float shininess;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform int u_lighting_enabled = 1;
uniform int u_texture_enabled = 1;
uniform int u_fog_enabled = 1;
uniform sampler2D u_texture_0;
uniform DirectionalLight u_directional_light;
uniform Material u_material;
uniform vec3 u_eye_position;

float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main() {

    vec4 light_factor = vec4(1.f);

    if(u_lighting_enabled == 1)
    {
        vec3 norm = normalize(io_normal);
        // ambient
        vec3 ambient = u_directional_light.color * u_material.ambient;
        // diffuse
        float diffuse_factor = max(dot(norm, u_directional_light.direction), 0.0f);
        vec3 diffuse = u_directional_light.color * (diffuse_factor * u_material.diffuse);
        // specular
        vec3 view_direction = normalize(u_eye_position - io_frag_position);
        vec3 reflect_direction = reflect(-u_directional_light.direction, norm);
        float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0f), u_material.shininess);
        vec3 specular = u_directional_light.color * (specular_factor * u_material.specular);
        
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
}