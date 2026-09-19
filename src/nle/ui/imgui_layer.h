/**
 * @file imgui_layer.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Dear ImGui, set up against the engine's window and render loop.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "../core/ref.h"
#include "../window/window_glfw.h"

namespace nle
{

/**
 * @brief Owns the imgui context and runs one frame of it.
 *
 * Tools built on the engine need a real interface -- panels, fields, text --
 * and imgui is already linked. This is the whole integration: construct one,
 * draw between begin_frame() and end_frame() from window::render_ui(), and
 * let it go out of scope to shut down.
 *
 *     nle::imgui_layer ui(app.window());
 *
 *     app.window()->render_ui() = [&]() {
 *         ui.begin_frame();
 *         ImGui::Begin("Tools");
 *         ...
 *         ImGui::End();
 *         ui.end_frame();
 *     };
 *
 * Only one of these may exist at a time, since the imgui context is global.
 *
 * wants_mouse() and wants_keyboard() are what keep a tool from acting on input
 * that the interface has already taken: a click that lands on a panel must not
 * also place something in the world behind it.
 */
class imgui_layer
{
public:
    /// @param glsl_version the version to compile imgui's shaders against.
    explicit imgui_layer(ref<window_glfw> window, unsigned int glsl_version = 330);
    ~imgui_layer();

    imgui_layer(const imgui_layer&) = delete;
    imgui_layer& operator=(const imgui_layer&) = delete;

    void begin_frame();
    void end_frame();

    /// True when the interface is using the mouse, so the world should not.
    bool wants_mouse() const;

    /// True when the interface has keyboard focus, such as a text field.
    bool wants_keyboard() const;

private:
    ref<window_glfw> m_window;
    bool m_frame_open = false;
};

} // namespace nle
