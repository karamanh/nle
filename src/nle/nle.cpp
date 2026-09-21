#include "nle.h"

namespace nle
{

nle::nle(unsigned int width, unsigned int height, const std::string& title)
{
    m_window = make_ref<class window_glfw>(width, height, title);
    m_renderer = make_ref<class renderer_3d>(m_window);
}

nle::~nle()
{
}

ref<window_glfw> nle::window()
{
    return m_window;
}

ref<class renderer_3d> nle::renderer_3d()
{
    return m_renderer;
}

void nle::run()
{
    m_window->display();
}

} // namespace nle
