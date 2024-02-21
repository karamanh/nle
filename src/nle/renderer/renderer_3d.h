/**
 * @file renderer_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "../window/window_glfw.h"
#include "../scene/scene_3d.h"

namespace nle
{

struct render_layer_attribute
{
    bool visible = true;
    float render_distance = 100.0f;
};

class renderer_3d
{
public:
    renderer_3d(ref<window_glfw> render_target);
    virtual ~renderer_3d();

    void set_current_scene(ref<scene_3d> scene);
    ref<scene_3d> current_scene();

    ref<window_glfw> render_target();

private:
    ref<window_glfw> m_render_target;

    ref<scene_3d> m_current_scene;

    std::unordered_map<render_layer, render_layer_attribute> m_render_layer_attributes;

    void render(ref<render_object_3d> ro);

    void render_scene(ref<scene_3d> scene);
    
    void load_scene(ref<scene_3d> scene);

    void main_routine();
};

} // namespace nle
