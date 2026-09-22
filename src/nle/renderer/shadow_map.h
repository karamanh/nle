/**
 * @file shadow_map.h
 * @brief What the sun can see, so that everything else is in shadow.
 *
 * One directional light, one depth texture, and an orthographic box that
 * follows the camera. The scene is drawn once more from the light's point of
 * view, keeping only how far away things were; anything the lit pass finds
 * further from the light than that is standing behind something.
 *
 * Deliberately one cascade rather than several. A cascade is the answer to
 * wanting crisp shadows at your feet and shadows at all on the horizon at
 * the same time, and the cost is three or four more scene passes. This box
 * is sized to the distance things are actually drawn at, which is a setting
 * here, so for a game that draws a couple of hundred units it is enough.
 */

#pragma once

#include "render_context.h"

#include <glm/glm.hpp>

namespace nle
{

class shadow_map
{
public:
    shadow_map();
    ~shadow_map();

    shadow_map(const shadow_map&) = delete;
    shadow_map& operator=(const shadow_map&) = delete;

    /// Off by default: a game that does not want them pays nothing at all.
    void set_enabled(bool enabled);
    bool enabled() const;

    /**
     * @brief How big the depth texture is, per side.
     *
     * The one dial worth having. Bigger is sharper and costs memory and
     * fill; anything past four thousand is usually wasted on a box this
     * size.
     */
    void set_resolution(int pixels);
    int resolution() const;

    /// How far from the eye shadows are cast. The box is built around this.
    void set_distance(float units);
    float distance() const;

    /**
     * @brief Points the box at what the camera is looking at.
     *
     * @return the light-space matrix, which the lit pass needs to ask where
     *         a fragment falls in the depth texture.
     */
    glm::mat4 aim(const glm::vec3& eye, const glm::vec3& light_direction);

    /// Starts the depth pass. False when there is nothing to draw into, so
    /// the caller can skip it entirely rather than drawing into nowhere.
    bool begin();

    /// Ends it, putting back both the viewport and whatever framebuffer was
    /// bound before -- which is not always the window.
    void end(int window_width, int window_height);

    /// Binds the depth texture for the lit pass to sample.
    void bind_for_reading(int texture_unit) const;

    unsigned int texture() const;

private:
    bool ensure_resources();
    void drop_resources();

    bool m_enabled = false;

    int m_resolution = 2048;
    float m_distance = 120.0f;

    unsigned int m_framebuffer = 0;
    unsigned int m_depth = 0;

    /// What was bound when the pass started. Bloom draws the scene into a
    /// target of its own, so assuming the window here blanked the world.
    int m_was_bound = 0;

    int m_built_at = 0;

    glm::mat4 m_light_space = glm::mat4(1.0f);
};

} // namespace nle
