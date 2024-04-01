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

#include "../object/render_object_3d.h"
#include "../renderer/camera.h"
#include "../renderer/light.h"

#include <set>

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

    void set_light(ref<class light> light);
    ref<class light> light();

    glm::vec2 target_resolution() const;
    
    void render();
    
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

    std::set<ref<render_object_3d>> m_render_objects;

    glm::vec2 m_target_resolution;

    void set_target_resolution(glm::vec2 resolution);

friend class renderer_3d;
};

} // namespace nle
