/**
 * @file sky.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief What is behind everything else: the sky, and the sun in it.
 * @version 0.2
 * @date 2024-04-11
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include "../mesh/mesh_instance_3d.h"
#include "../mesh/box_mesh.h"

#include <glm/glm.hpp>

namespace nle
{

/**
 * @brief How a sky looks.
 *
 * A gradient from the horizon up to the zenith, and down to the ground below
 * the horizon, with the sun drawn where the scene's directional light comes
 * from -- so moving the sun moves it in the sky too -- and stars for a night.
 */
struct sky_settings
{
    glm::vec3 zenith = glm::vec3(0.16f, 0.32f, 0.62f);
    glm::vec3 horizon = glm::vec3(0.62f, 0.72f, 0.82f);

    /// Below the horizon, seen past the edge of the world.
    glm::vec3 ground = glm::vec3(0.30f, 0.30f, 0.32f);

    /// How quickly the horizon gives way to the zenith going up: small is a
    /// deep band of horizon, large a thin one.
    float horizon_falloff = 0.45f;

    glm::vec3 sun_color = glm::vec3(1.0f, 0.93f, 0.78f);

    /// The sun's radius, in degrees of the sky.
    float sun_size = 1.6f;

    /// How far the glow round the sun reaches, nought for none.
    float sun_glow = 0.35f;

    /// How bright the stars are, nought for a day.
    float stars = 0.0f;
};

/**
 * @brief The sky, drawn round the camera behind everything else.
 *
 * Its own small shader, carried in the engine, so it does not depend on a
 * shader file being found; drawn first, writing no depth, so that whatever
 * is in the world is drawn over it.
 */
class sky : public mesh_instance_3d
{
public:
    sky(ref<mesh_3d> mesh = make_ref<boxmesh>());
    virtual ~sky();

    void set_settings(const sky_settings& settings);
    const sky_settings& settings() const;

    void render(render_command_buffer& command_buffer, const render_context& context) override;

private:
    sky_settings m_settings;
};

} // namespace nle
