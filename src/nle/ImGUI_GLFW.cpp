#include "ImGUI_GLFW.h"
#include "Utils.h"

#include <string>

namespace nle
{

    ImGUI_GLFW::ImGUI_GLFW(GLFWwindow *handle, unsigned int glsl_version)
        : m_handle(handle)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        ImGui_ImplGlfw_InitForOpenGL(m_handle, true);

        std::string version = "#version " + std::to_string(glsl_version);

        ImGui_ImplOpenGL3_Init(version.c_str());
        ImGui::StyleColorsDark();
    }

    ImGUI_GLFW::~ImGUI_GLFW()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGUI_GLFW::render()
    {
        if(!m_visible)
            return;

        // create frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        m_draw_callback();

        // render
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    const std::function<void()> &ImGUI_GLFW::draw_callback() const
    {
        return m_draw_callback;
    }

    void ImGUI_GLFW::set_draw_callback(std::function<void()> draw_callback)
    {
        m_draw_callback = draw_callback;
    }

    bool ImGUI_GLFW::visible() const
    {
        return m_visible;
    }

    void ImGUI_GLFW::set_visible(bool visible)
    {
        m_visible = visible;
    }

} // namespace nle
