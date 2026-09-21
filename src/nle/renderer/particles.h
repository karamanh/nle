/**
 * @file particles.h
 * @brief A great many small bright things, simulated entirely on the GPU.
 *
 * Stateless, which is the whole design: a particle's position, size and
 * colour are a pure function of its index and the clock. Nothing is stored
 * between frames, nothing is read back, and the CPU's entire contribution is
 * one uniform and one instanced draw call. Ten emitters of two hundred
 * particles each cost ten draw calls and no per-particle work at all.
 *
 * The cost of statelessness is that particles cannot collide, cannot be
 * spawned in bursts and cannot outlive their emitter. For fire on the end of
 * a staff, none of that is a loss: it is a loop that has always been burning
 * and always will be.
 *
 * The geometry is one quad, turned to face the camera in the vertex shader.
 * Drawn additively with depth writes off, so overlapping particles brighten
 * each other and nothing is hidden behind the ones in front of it.
 */

#pragma once

#include "render_object_3d.h"

#include <glm/glm.hpp>

namespace nle
{

/// What a puff of particles behaves like.
enum class particle_style
{
    /// Rises, narrows and fades. Fire, and anything that burns.
    flame,

    /// Drifts outward and downward, slowly, and stays soft. Frost, mist.
    cloud,

    /// Darts out and dies almost at once. Sparks, lightning.
    spark
};

/**
 * @brief One emitter: a cloud of particles around a point.
 *
 * Placed like anything else, including by matrix, so it can be hung off a
 * bone. Its own transform is where the particles come from; they are
 * simulated in its space and so follow it without any of them being moved.
 */
class particle_emitter : public render_object_3d
{
public:
    particle_emitter();
    ~particle_emitter() override;

    /// How many at once. Cheap -- this is one number in one draw call -- but
    /// not free, since each one is still a quad's worth of overdraw.
    void set_count(size_t count);
    size_t count() const;

    void set_style(particle_style style);
    particle_style style() const;

    /// What colour they start and end. Ending darker and redder is what makes
    /// a flame look like one.
    void set_colours(const glm::vec3& born, const glm::vec3& dying);

    /// How long one lives, in seconds, and how far it gets.
    void set_lifetime(float seconds);
    void set_speed(float units_per_second);

    /// How big one is at birth, in world units.
    void set_size(float size);

    /// How far from the middle they are born.
    void set_radius(float radius);

    /**
     * @brief Spreads where they are born along a line rather than a point.
     *
     * For a thing that glows along its length -- a sword's blade -- rather
     * than at one end of it. @p span is a world-space direction times how far
     * it reaches; zero puts every particle at the emitter, which is where
     * they were before this existed.
     *
     * Only where they are *born* moves. How they then travel is unchanged, so
     * a flame spread along a blade still rises rather than running down it.
     */
    void set_span(const glm::vec3& span);
    const glm::vec3& span() const;

    /// Overall brightness. Everything else being equal, this is the dial to
    /// turn when a thing should read as more enchanted.
    void set_intensity(float intensity);
    float intensity() const;

    void render(render_command_buffer& command_buffer, const render_context& context) override;

private:
    /// Builds the one quad and the shader, once for the whole process.
    static void ensure_resources();

    size_t m_count = 48;
    particle_style m_style = particle_style::flame;

    glm::vec3 m_born = glm::vec3(1.0f, 0.75f, 0.25f);
    glm::vec3 m_dying = glm::vec3(0.9f, 0.15f, 0.05f);

    float m_lifetime = 1.1f;
    float m_speed = 1.0f;
    float m_size = 0.18f;
    float m_radius = 0.1f;

    /// Where they are born, spread along this from the emitter. Zero for a
    /// point, which is the ordinary case.
    glm::vec3 m_span = glm::vec3(0.0f);
    float m_intensity = 1.0f;

    /// Counts up, and is what the whole simulation is a function of.
    float m_clock = 0.0f;
};

} // namespace nle
