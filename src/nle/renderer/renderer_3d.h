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

#include "../core/clock.h"
#include "../window/window_glfw.h"
#include "../scene/scene_3d.h"
#include "render_command.h"
#include "render_context.h"
#include "opengl_backend.h"

namespace nle
{

struct render_layer_attribute
{
    bool visible = true;
    float render_distance = 500000.0f;
};

/**
 * @brief Draws a scene_3d into a window.
 *
 * Each frame the renderer builds one render_context describing the camera and
 * the lights, hands it to the backend, and then walks the scene asking every
 * visible object to record its draw. Objects never touch GL and never upload
 * frame constants themselves.
 */
class renderer_3d
{
public:
    renderer_3d(ref<window_glfw> render_target);
    virtual ~renderer_3d();

    void set_current_scene(ref<scene_3d> scene);
    ref<scene_3d> current_scene();

    ref<window_glfw> render_target();

    void set_render_layer_attribute(enum render_layer layer, const render_layer_attribute& attribute);
    render_layer_attribute render_layer_attribute_of(enum render_layer layer);

    /// Command/draw counts for the frame that was just submitted.
    const opengl_backend::frame_statistics& statistics() const;

    /// Seconds spent on the previous frame.
    float delta_time() const;

    void set_clear_color(const glm::vec3& color);
    glm::vec3 clear_color() const;

private:
    ref<window_glfw> m_render_target;

    ref<scene_3d> m_current_scene;

    std::unordered_map<render_layer, render_layer_attribute> m_render_layer_attributes;

    render_command_buffer m_command_buffer;
    opengl_backend m_opengl_backend;

    clock m_clock;
    int64_t m_last_frame_us = 0;
    float m_delta_time = 0.0f;
    float m_time = 0.0f;

    glm::vec3 m_clear_color = glm::vec3(0.5f);

    /// Gathers the camera, lights and fog for this frame.
    render_context build_render_context(ref<scene_3d> scene);

    bool is_visible(const ref<render_object_3d>& ro, const glm::vec3& eye);

    void render_scene(ref<scene_3d> scene);

    void main_routine();
};

} // namespace nle
