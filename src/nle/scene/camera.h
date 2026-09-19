/**
 * @file camera.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "../object/object_3d.h"
#include "../core/ray.h"

namespace nle
{

class camera : public object_3d
{
public:
    camera();
    virtual ~camera();

    void set_turn_speed(float turn_speed);
    float turn_speed() const;

    void set_free_roam(bool free_roam);
    bool free_roam() const;
    
    void set_field_of_view(float fov);
    float field_of_view() const;

    void set_near(float near);
    float near() const;

    void set_far(float far);
    float far() const;

    glm::mat4 view_matrix() const;

    /// Perspective projection for a viewport of the given aspect ratio.
    glm::mat4 projection_matrix(float aspect_ratio) const;

    /**
     * @brief The world-space ray under a pixel on screen.
     *
     * @p pixel uses the same convention as the mouse and the 2D overlay:
     * the origin is the top left corner, y grows downwards.
     *
     * This is what turns a click into a point on the ground -- intersect the
     * result with the terrain, or with intersect_horizontal_plane().
     */
    ray screen_ray(const glm::vec2& pixel, const glm::vec2& resolution) const;

    /**
     * @brief Projects a world position onto the screen, in pixels.
     *
     * Used to pin overlay elements -- a health bar over a monster's head -- to
     * something in the world.
     *
     * @return false when the point is behind the camera, in which case @p out
     *         is left alone and nothing should be drawn for it.
     */
    bool world_to_screen(const glm::vec3& world, const glm::vec2& resolution, glm::vec2& out) const;

    nlohmann::json to_json() const override;
    void from_json(const nlohmann::json& j) override;

private:
    float m_speed;
    float m_turn_speed;
    float m_field_of_view;
    float m_near;
    float m_far;
    bool m_free_roam;
};

} // namespace nle
