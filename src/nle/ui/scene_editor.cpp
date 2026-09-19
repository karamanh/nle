#include "scene_editor.h"

namespace nle
{

scene_editor::scene_editor(GLFWwindow *handle, unsigned int glsl_version)
    : ui(handle)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui_ImplGlfw_InitForOpenGL(m_render_target, true);

    std::string version = "#version " + std::to_string(glsl_version);

    ImGui_ImplOpenGL3_Init(version.c_str());
    ImGui::StyleColorsDark();
}

scene_editor::~scene_editor()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void scene_editor::render()
{
    if(ImGui::BeginMainMenuBar())
    {
    }
}

} // namespace nle
