#pragma once

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <functional>

namespace nle
{
    class ImGUI_GLFW
    {
    private:
        GLFWwindow *m_handle;
        std::function<void()> m_draw_callback;
        bool m_visible = true;

    public:
        ImGUI_GLFW(GLFWwindow *handle, unsigned int glsl_version = 330);
        ~ImGUI_GLFW();
        void render();

        const std::function<void()> &draw_callback() const;
        void set_draw_callback(std::function<void()> draw_callback);

        bool visible() const;
        void set_visible(bool visible);
    };

} // namespace nle
