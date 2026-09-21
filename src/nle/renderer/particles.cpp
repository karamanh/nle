#include "particles.h"

#include "render_command.h"
#include "shader.h"

#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>

namespace nle
{

namespace
{

/**
 * @brief The whole simulation, as a function of an index and the clock.
 *
 * Every particle works out where it is from gl_InstanceID and u_time and
 * nothing else. There is no buffer of positions to update, no ping-pong, no
 * read-back -- which is what makes an emitter cost one draw call and no CPU
 * time, and is also why a particle cannot be told anything: it has no memory
 * to tell it into.
 *
 * The randomness is a hash of the index, so it is the same every frame and
 * the same on every machine. A particle that jittered between frames because
 * its "random" direction was re-rolled would be a firefly, not a flame.
 */
const char* PARTICLE_VERTEX = R"(
#version 330 core

layout (location = 0) in vec2 a_corner;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform float u_time;
uniform float u_lifetime;
uniform float u_speed;
uniform float u_size;
uniform float u_radius;
uniform vec3  u_span;
uniform int   u_count;
uniform int   u_style;

out float v_age;
out vec2  v_corner;

/// Three uncorrelated numbers in [0,1) from one index. Cheap and good enough
/// that no two particles of a few hundred visibly share a path.
vec3 hash3(float n)
{
    return fract(sin(vec3(n, n + 17.13, n + 41.71)) * vec3(43758.5453, 22578.1459, 19642.3491));
}

void main()
{
    float index = float(gl_InstanceID);
    vec3 r = hash3(index);

    // Spread the births evenly through the cycle, so the stream is steady
    // rather than pulsing all together.
    float phase = float(gl_InstanceID) / float(max(u_count, 1));
    float age = fract(u_time / u_lifetime + phase);

    float seconds = age * u_lifetime;

    // A direction that does not change for the life of this particle.
    float angle = r.x * 6.2831853;
    vec2 around = vec2(cos(angle), sin(angle));

    vec3 local = vec3(0.0);
    float size = u_size;

    if(u_style == 0)
    {
        // Flame: up, wavering, narrowing to nothing.
        float wobble = sin(u_time * 3.1 + index) * 0.12 * age;

        local = vec3(around.x * u_radius * (0.35 + r.y * 0.65) + wobble,
                     seconds * u_speed,
                     around.y * u_radius * (0.35 + r.y * 0.65));

        // Wide at the base, pinched at the top, which is what reads as fire
        // rather than as a column of dots.
        size *= (0.45 + r.z * 0.55) * (1.0 - age * 0.75);
    }
    else if(u_style == 1)
    {
        // Cloud: outward and gently down, keeping its size.
        float out_by = u_radius + seconds * u_speed * 0.5;

        local = vec3(around.x * out_by,
                     (r.y - 0.65) * seconds * u_speed * 0.6,
                     around.y * out_by);

        size *= (0.6 + r.z * 0.8) * (1.0 - age * 0.3);
    }
    else
    {
        // Spark: straight out, fast, gone.
        float reach = seconds * u_speed * 2.2;
        float up = (r.y - 0.4) * 1.6;

        local = normalize(vec3(around.x, up, around.y)) * (u_radius + reach);

        size *= (0.25 + r.z * 0.4) * (1.0 - age) * (1.0 - age);
    }

    vec3 centre = (u_model * vec4(local, 1.0)).xyz;

    // Born along a line rather than at a point, for a thing that glows down
    // its length. Added after the model transform because the span is given
    // in world terms: only the birthplace moves, so a flame spread along a
    // blade still rises rather than running along it.
    centre += u_span * fract(r.z + phase);

    // Turned to face the camera by building the quad in view space, which is
    // the cheapest billboard there is: no per-particle matrix, no cross
    // products, just the two axes the view already has.
    vec3 right = vec3(u_view[0][0], u_view[1][0], u_view[2][0]);
    vec3 up    = vec3(u_view[0][1], u_view[1][1], u_view[2][1]);

    vec3 world = centre + (right * a_corner.x + up * a_corner.y) * size;

    gl_Position = u_projection * u_view * vec4(world, 1.0);

    v_age = age;
    v_corner = a_corner;
}
)";

/**
 * @brief A soft round dot, faded at the edge and over its life.
 *
 * No texture: a circle with a soft edge is two instructions and always the
 * right resolution, and an emitter that needs no files is an emitter that
 * cannot be missing one.
 */
const char* PARTICLE_FRAGMENT = R"(
#version 330 core

in float v_age;
in vec2  v_corner;

uniform vec3  u_born;
uniform vec3  u_dying;
uniform float u_intensity;

out vec4 frag_color;

void main()
{
    // Round, and soft at the rim. Squared so the falloff is gentler in the
    // middle than at the edge.
    float r = length(v_corner) * 2.0;
    float fall = clamp(1.0 - r, 0.0, 1.0);
    fall *= fall;

    if(fall <= 0.001)
    {
        discard;
    }

    // Bright at birth, guttering out. The fade is on alpha rather than on
    // colour, because these are drawn additively and a colour fading to black
    // is already invisible.
    float life = 1.0 - v_age;

    vec3 colour = mix(u_born, u_dying, v_age);

    frag_color = vec4(colour * u_intensity, fall * life * life);
}
)";

/// One quad, shared by every emitter in the process. Two triangles is all the
/// geometry a particle system needs; everything else is the shader.
GLuint g_vao = 0;
GLuint g_vbo = 0;
GLuint g_ebo = 0;

ref<::nle::shader> g_shader;

} // namespace

void particle_emitter::ensure_resources()
{
    if(g_vao != 0)
    {
        return;
    }

    const float corners[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f,
    };

    const unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glGenBuffers(1, &g_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

    glGenBuffers(1, &g_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    g_shader = make_ref<::nle::shader>(PARTICLE_VERTEX, PARTICLE_FRAGMENT,
                                      shader_source::memory);

    // The constructor only keeps the source; nothing is compiled or linked
    // until it is asked for. A shader that was never loaded has program zero
    // and draws nothing at all, perfectly silently.
    g_shader->load();
}

particle_emitter::particle_emitter()
{
}

particle_emitter::~particle_emitter()
{
}

void particle_emitter::set_count(size_t count)
{
    m_count = count;
}

size_t particle_emitter::count() const
{
    return m_count;
}

void particle_emitter::set_style(particle_style style)
{
    m_style = style;
}

particle_style particle_emitter::style() const
{
    return m_style;
}

void particle_emitter::set_colours(const glm::vec3& born, const glm::vec3& dying)
{
    m_born = born;
    m_dying = dying;
}

void particle_emitter::set_lifetime(float seconds)
{
    m_lifetime = seconds > 0.0f ? seconds : 0.001f;
}

void particle_emitter::set_speed(float units_per_second)
{
    m_speed = units_per_second;
}

void particle_emitter::set_size(float size)
{
    m_size = size;
}

float particle_emitter::size() const
{
    return m_size;
}

void particle_emitter::set_radius(float radius)
{
    m_radius = radius;
}

void particle_emitter::set_span(const glm::vec3& span)
{
    m_span = span;
}

const glm::vec3& particle_emitter::span() const
{
    return m_span;
}

void particle_emitter::set_intensity(float intensity)
{
    m_intensity = intensity;
}

float particle_emitter::intensity() const
{
    return m_intensity;
}

void particle_emitter::render(render_command_buffer& command_buffer,
                              const render_context& context)
{
    if(m_count == 0 || !visible())
    {
        return;
    }

    ensure_resources();

    m_clock += context.delta_time;

    command_buffer.use_shader(g_shader);

    command_buffer.set_uniform("u_model", transform_matrix());
    command_buffer.set_uniform("u_view", context.view);
    command_buffer.set_uniform("u_projection", context.projection);

    command_buffer.set_uniform("u_time", m_clock);
    command_buffer.set_uniform("u_lifetime", m_lifetime);
    command_buffer.set_uniform("u_speed", m_speed);
    command_buffer.set_uniform("u_size", m_size);
    command_buffer.set_uniform("u_radius", m_radius);
    command_buffer.set_uniform("u_span", m_span);
    command_buffer.set_uniform("u_count", static_cast<int>(m_count));
    command_buffer.set_uniform("u_style", static_cast<int>(m_style));

    command_buffer.set_uniform("u_born", m_born);
    command_buffer.set_uniform("u_dying", m_dying);
    command_buffer.set_uniform("u_intensity", m_intensity);

    // Added to what is already there, and writing no depth: overlapping
    // particles should brighten each other rather than the nearest one
    // hiding the rest, and a cloud of them should not stop the world behind
    // it being drawn.
    command_buffer.set_blending(blend_mode::additive);
    command_buffer.set_depth_mask(false);

    command_buffer.draw_elements_instanced(GL_TRIANGLES, g_vao, g_ebo, 6, m_count);

    // Put back, or everything drawn after this is a glow too.
    command_buffer.set_depth_mask(true);
    command_buffer.set_blending(blend_mode::none);
}

} // namespace nle
