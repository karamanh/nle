/**
 * @file scene_editor.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-08-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "../window/input_handler_glfw.h"
#include "ui.hpp"

// #include <imgui/imgui.h>
// #include <imgui/backends/imgui_impl_glfw.h>
// #include <imgui/backends/imgui_impl_opengl3.h>

#include "../../../vendor/imgui/imgui.h"
#include "../../../vendor/imgui/backends/imgui_impl_glfw.h"
#include "../../../vendor/imgui/backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

namespace nle
{

class scene_editor : public ui<GLFWwindow*>
{
public:
    scene_editor(GLFWwindow *handle, unsigned int glsl_version = 330);
    ~scene_editor();
    void render() override;
};

} // namespace nle
