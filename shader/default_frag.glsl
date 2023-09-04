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
    float specular_intensity;
    float shininess;
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
        vec4 ambient_color = vec4(u_directional_light.color, 1.f) * u_directional_light.ambient_intensity;
        float diffuse_factor = max(dot(normalize(io_normal), normalize(u_directional_light.direction)), 0.f);
        vec4 diffuse_color = vec4(u_directional_light.color, 1.f) * u_directional_light.diffuse_intensity * diffuse_factor * 1.f;
        light_factor = ambient_color + diffuse_color;

        vec4 specular_color = vec4(0.0f);
        if(diffuse_factor > 0.0f)
        {
            vec3 frag_to_eye = normalize(u_eye_position -  io_frag_position);
            vec3 reflected_vertex = normalize(reflect(u_directional_light.direction, normalize(io_normal)));
            float specular_factor = dot(frag_to_eye, reflected_vertex);
            
            if(specular_factor > 0.0f)
            {
                specular_factor = pow(specular_factor, u_material.shininess);
                specular_color = vec4(u_directional_light.color * u_material.specular_intensity * specular_factor, 1.0f);
                light_factor += specular_color;
            }
        }
    }

    if(u_texture_enabled == 1)
    {
        io_color = texture(u_texture_0, io_texture_coordinates) * light_factor;
    }
    else
    {
        io_color = io_vertex_color * light_factor;
    }

    float dist = length(u_eye_position - io_frag_position); 

    float fog_factor = 0.25f;
    float fog_inner = 5.f;
    float fog_outer = 10000.f;

    if(dist >= fog_inner && dist <= fog_outer)
    {
        float mapped = map(dist, fog_inner, fog_outer, 0.0f, 1.0f);
        vec4 fog = vec4(mapped);
        fog.a = 0.f;
        io_color += fog;
    }
    // io_color += (clamp(dist, fog_inner, fog_outer));
}