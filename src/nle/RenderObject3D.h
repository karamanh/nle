#pragma once

#include <GL/gl.h>

#include "Object3D.h"

namespace nle
{
enum RenderMode
{
    Point = GL_POINT,
    Line = GL_LINE,
    Fill = GL_FILL
};

enum class RenderLayer
{
    _0,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7
};

class RenderObject3D : public Object3D
{
protected:
    RenderLayer m_render_layer = RenderLayer::_0;
    RenderMode m_render_mode = RenderMode::Fill;
    bool m_visible = true;
    bool m_mouse_pickable = true;
public:
    RenderObject3D() = default;

    void set_render_mode(RenderMode render_mode);
    RenderMode render_mode();

    void set_visible(bool visible);
    bool visible() const;

    void set_render_layer(RenderLayer layer);
    RenderLayer render_layer() const;

    void set_mouse_pickable(bool mouse_pickable);
    bool mouse_pickable() const;
};

} // namespace nle
