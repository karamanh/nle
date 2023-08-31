#include "RenderObject3D.h"

namespace nle
{

void RenderObject3D::set_render_mode(RenderMode render_mode)
{
    m_render_mode = render_mode;
    for (auto *c : this->children())
    {
        if (RenderObject3D *r = dynamic_cast<RenderObject3D *>(c))
        {
            r->set_render_mode(render_mode);
        }
    }
}

RenderMode RenderObject3D::render_mode()
{
    return m_render_mode;
}

void RenderObject3D::set_visible(bool visible)
{
    m_visible = visible;
}

bool RenderObject3D::visible() const
{
    return m_visible;
}

void RenderObject3D::set_render_layer(RenderLayer layer)
{
    m_render_layer = layer;
}

RenderLayer RenderObject3D::render_layer() const
{
    return m_render_layer;
}

bool RenderObject3D::mouse_pickable() const
{
    return m_mouse_pickable;
}

void RenderObject3D::set_mouse_pickable(bool mouse_pickable)
{
    m_mouse_pickable = mouse_pickable;
}

} // namespace nle