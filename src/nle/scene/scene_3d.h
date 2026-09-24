/**
 * @file scene_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "../renderer/render_context.h"
#include "../renderer/render_object_3d.h"
#include "camera.h"
#include "light.h"
#include "sky.h"

#include <set>
#include <vector>

namespace nle
{

class scene_3d : public render_object_3d, public std::enable_shared_from_this<scene_3d>
{
public:
    scene_3d();
    virtual ~scene_3d();

    void set_camera(ref<class camera> camera);
    ref<class camera> camera();
    ref<class camera> default_camera();

    /// The scene's directional light. Always non-null; a default one is created
    /// with the scene.
    void set_light(ref<class light> light);
    ref<class light> light();

    /**
     * @brief Registers a point light with the scene.
     *
     * Point lights are not render objects, so they are tracked separately from
     * the object tree. add_child() also picks them up, which is what you want
     * when parenting a light to a moving object; this method is for lights that
     * belong to the scene itself.
     */
    void add_point_light(ref<class point_light> light);
    void remove_point_light(ref<class point_light> light);
    const std::vector<ref<class point_light>>& point_lights() const;

    /**
     * @brief The point lights that actually matter for a viewer at @p eye.
     *
     * A light reaches only as far as its range, so one whose reach falls
     * entirely outside @p view cannot light anything anybody can see, and is
     * dropped -- along with the ones switched off. What is left is sorted
     * nearest-first and truncated to MAX_POINT_LIGHTS, so the shader's
     * fixed-size array is never overrun and the nearest lights are the ones
     * that fill it.
     *
     * Dropping by reach rather than by distance alone is what stops a
     * campfire on the other side of the map taking a slot from the one at
     * your feet -- and what stops the shader testing eight lights a fragment
     * when seven of them were never going to reach it.
     */
    std::vector<point_light_data> collect_point_lights(const glm::vec3& eye,
                                                       const frustum& view) const;

    void set_sky(ref<class sky> sky);
    ref<class sky> sky();

    /**
     * @brief Distance fog: how far you can see, and what the air looks like.
     *
     * Belongs to the scene rather than to the sky, so that a level can fade
     * into the distance without also having a skybox -- and so that the fog
     * colour can be matched to whatever the background actually is.
     *
     * Setting it to the same colour as the background is what makes a render
     * distance stop looking like things winking out of existence.
     */
    void set_fog(const fog_data& fog);
    const fog_data& fog() const;

    glm::vec2 target_resolution() const;
    
    void render(render_command_buffer& command_buffer, const render_context& context) override;
    
    /// check if added child is render object, if so, add it to set of render objects.
    void add_child(ref<object_3d> child) override;

    /// check if deleted child is render object, if so, remove it from render objects.
    void delete_child(ref<object_3d> child) override;

    nlohmann::json to_json() const override;
    void from_json(const nlohmann::json& j) override;
    
private:
    // this will be created with every scene.
    ref<class camera> m_default_camera;
    // this will be set by the user.
    ref<class camera> m_camera;

    ref<class light> m_default_light;

    ref<class light> m_light;

    std::vector<ref<class point_light>> m_point_lights;

    ref<class sky> m_sky;

    fog_data m_fog;

    glm::vec2 m_target_resolution;

    void set_target_resolution(glm::vec2 resolution);

friend class renderer_3d;
};

} // namespace nle
