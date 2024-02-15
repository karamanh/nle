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

#include <set>

namespace nle
{

class scene_3d : public object_3d
{
public:
    scene_3d();
    virtual ~scene_3d();

    void set_camera(ref<class camera> camera);
    ref<class camera> camera();
    ref<class camera> default_camera();

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

    std::set<ref<render_object_3d>> m_render_objects;

friend class renderer_3d;
};

} // namespace nle
