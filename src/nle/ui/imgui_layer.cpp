#include "imgui_layer.h"

#include "../core/utils.h"

// From the system imgui, which is also the one linked. See the note in
// CMakeLists.txt about vendor/imgui.
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <string>

namespace nle
{

imgui_layer::imgui_layer(ref<window_glfw> window, unsigned int glsl_version)
    : m_window(window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // imgui installs its own GLFW callbacks and chains to any already there,
    // which is what lets it see input without the engine forwarding anything.
    ImGui_ImplGlfw_InitForOpenGL(m_window ? m_window->handle() : nullptr, true);

    const std::string version = "#version " + std::to_string(glsl_version);
    ImGui_ImplOpenGL3_Init(version.c_str());

    ImGui::StyleColorsDark();
}

imgui_layer::~imgui_layer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void imgui_layer::begin_frame()
{
    if(m_frame_open)
    {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_frame_open = true;
}

void imgui_layer::end_frame()
{
    if(!m_frame_open)
    {
        return;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    m_frame_open = false;
}

void imgui_layer::set_layout_file(const std::string& path)
{
    m_layout_file = path;

    // imgui holds the pointer rather than copying it, which is why the string
    // is a member: handing it a temporary is a use-after-free that shows up
    // as a corrupted layout file much later.
    ImGui::GetIO().IniFilename = m_layout_file.empty() ? nullptr : m_layout_file.c_str();
}

void imgui_layer::rebuild_fonts()
{
    // The backend makes its objects again, font texture included, on the
    // next new frame once they are gone.
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
}

bool imgui_layer::frame_open() const
{
    return m_frame_open;
}

bool imgui_layer::wants_mouse() const
{
    return ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse;
}

bool imgui_layer::wants_keyboard() const
{
    return ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard;
}

bool imgui_layer::wants_text() const
{
    return ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantTextInput;
}

} // namespace nle
