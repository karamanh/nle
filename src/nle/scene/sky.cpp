#include "sky.h"

#include "../renderer/shader.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace nle
{

namespace
{

const char* SKY_VERTEX = R"(
#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 u_view_rotation;
uniform mat4 u_projection;

out vec3 v_direction;

void main()
{
    v_direction = position;

    // Drawn first and writing no depth, so everything drawn after it is in
    // front of it wherever it happens to sit.
    gl_Position = u_projection * u_view_rotation * vec4(position, 1.0);
}
)";

const char* SKY_FRAGMENT = R"(
#version 330 core

in vec3 v_direction;
out vec4 frag_color;

uniform vec3 u_zenith;
uniform vec3 u_horizon;
uniform vec3 u_ground;
uniform float u_horizon_falloff;

uniform vec3 u_sun_direction;
uniform vec3 u_sun_color;
uniform float u_sun_cos_outer;
uniform float u_sun_cos_inner;
uniform float u_sun_glow;

uniform float u_stars;
uniform float u_time;

float hash(vec3 p)
{
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

void main()
{
    vec3 dir = normalize(v_direction);
    float up = dir.y;

    vec3 colour;

    if (up >= 0.0)
    {
        colour = mix(u_horizon, u_zenith, pow(up, u_horizon_falloff));
    }
    else
    {
        // Quickly to the ground colour below the horizon, so the seam where
        // the world ends is soft rather than a line.
        colour = mix(u_horizon, u_ground, smoothstep(0.0, 0.25, -up));
    }

    // Stars, where it is dark enough to see them and above the horizon.
    if (u_stars > 0.0 && up > 0.0)
    {
        vec3 cell = floor(dir * 300.0);
        float h = hash(cell);

        if (h > 0.9975)
        {
            float twinkle = 0.75 + 0.25 * sin(u_time * (1.0 + h * 3.0) + h * 40.0);
            colour += vec3(u_stars * twinkle) * smoothstep(0.0, 0.2, up);
        }
    }

    // The sun: a disc with a soft edge, and a glow round it that warms the
    // sky it is in.
    float toward = dot(dir, u_sun_direction);

    float disc = smoothstep(u_sun_cos_outer, u_sun_cos_inner, toward);
    // More glow is a wider glow as well as a brighter one.
    float exponent = mix(600.0, 18.0, clamp(u_sun_glow, 0.0, 1.0));
    float glow = u_sun_glow * pow(max(toward, 0.0), exponent);
    float halo = u_sun_glow * 0.35 * pow(max(toward, 0.0), 3.0) * (1.0 - clamp(up, 0.0, 1.0));

    colour += u_sun_color * (disc * 2.5 + glow + halo);

    frag_color = vec4(colour, 1.0);
}
)";

ref<shader> g_sky_shader;

ref<shader> sky_shader()
{
    if(!g_sky_shader)
    {
        g_sky_shader = make_ref<shader>(SKY_VERTEX, SKY_FRAGMENT, shader_source::memory);

        // Kept as source until asked to load: a shader never loaded has
        // program zero and draws nothing, silently.
        g_sky_shader->load();
    }

    return g_sky_shader;
}

} // namespace

sky::sky(ref<mesh_3d> mesh)
    : mesh_instance_3d(mesh)
{
}

sky::~sky()
{
}

void sky::set_settings(const sky_settings& settings)
{
    m_settings = settings;
}

const sky_settings& sky::settings() const
{
    return m_settings;
}

void sky::render(render_command_buffer& command_buffer, const render_context& context)
{
    if(!mesh())
    {
        return;
    }

    command_buffer.use_shader(sky_shader());

    // The camera's turning and nothing of its moving: the sky is infinitely
    // far off, so walking never brings any of it closer.
    command_buffer.set_uniform("u_view_rotation", glm::mat4(glm::mat3(context.view)));
    command_buffer.set_uniform("u_projection", context.projection);

    command_buffer.set_uniform("u_zenith", m_settings.zenith);
    command_buffer.set_uniform("u_horizon", m_settings.horizon);
    command_buffer.set_uniform("u_ground", m_settings.ground);
    command_buffer.set_uniform("u_horizon_falloff", std::max(m_settings.horizon_falloff, 0.01f));

    // Where the light comes from. The light's direction already points at
    // it -- the lit shader dots a surface's normal straight with it -- so the
    // sun sits where the shading says it is.
    const glm::vec3 toward = context.directional_light.direction;
    const glm::vec3 toward_sun = glm::length(toward) > 0.0f ? glm::normalize(toward)
                                                            : glm::vec3(0.0f, 1.0f, 0.0f);

    const float radius = glm::radians(std::max(m_settings.sun_size, 0.05f));

    command_buffer.set_uniform("u_sun_direction", toward_sun);
    command_buffer.set_uniform("u_sun_color", context.directional_light.enabled ? m_settings.sun_color
                                                                              : glm::vec3(0.0f));
    command_buffer.set_uniform("u_sun_cos_outer", std::cos(radius));
    command_buffer.set_uniform("u_sun_cos_inner", std::cos(radius * 0.75f));
    command_buffer.set_uniform("u_sun_glow", std::max(m_settings.sun_glow, 0.0f));

    command_buffer.set_uniform("u_stars", std::max(m_settings.stars, 0.0f));
    command_buffer.set_uniform("u_time", context.time);

    command_buffer.set_depth_mask(false);
    command_buffer.draw_elements(GL_TRIANGLES, mesh()->vao(), mesh()->ebo(), mesh()->indices().size());
    command_buffer.set_depth_mask(true);
}

} // namespace nle
