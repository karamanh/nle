#include "nle.h"

namespace nle
{

nle::nle()
{
    m_window = make_ref<class window_glfw>(480, 480, "nle");
    m_renderer = make_ref<class renderer_3d>(m_window);
}

nle::~nle()
{
}

ref<window_glfw> nle::window_glfw()
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
