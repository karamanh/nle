#include "render_object_3d.h"

namespace nle
{
    render_object_3d::render_object_3d(const std::string &id)
        : object_3d(id)
    {
        m_visible = true;
        m_render_layer = render_layer::_0;
        m_render_mode = render_mode::FILL;
    }

    render_object_3d::~render_object_3d()
    {
    }

    void render_object_3d::set_render_mode(enum render_mode rm)
    {
        m_render_mode = rm;
    }

    render_mode render_object_3d::render_mode() const
    {
        return m_render_mode;
    }

    void render_object_3d::set_render_layer(enum render_layer rl)
    {
        m_render_layer = rl;
    }

    enum render_layer render_object_3d::render_layer() const
    {
        return m_render_layer;
    }

    void render_object_3d::set_visible(bool visible)
    {
        m_visible = visible;
    }

    bool render_object_3d::visible() const
    {
        return m_visible;
    }

    void render_object_3d::set_shader(ref<class shader> shader)
    {
        m_shader = shader;
    }

    ref<class shader> render_object_3d::shader()
    {
        return m_shader;
    }

    nlohmann::json render_object_3d::to_json() const
    {
        auto j = object_3d::to_json();
        j["render_layer"] = static_cast<int>(m_render_layer);
        j["render_mode"] = static_cast<int>(m_render_mode);
        return j;
    }

    void render_object_3d::from_json(const nlohmann::json &j)
    {
        object_3d::from_json(j);
        m_render_layer = static_cast<enum render_layer>(j["render_layer"]);
        m_render_mode = static_cast<enum render_mode>(j["render_mode"]);
    }

    void render_object_3d::set_scene(ref<class object_3d> scene)
    {
        m_scene = scene;
        for(auto child : children())
        {
            if(auto ro = std::dynamic_pointer_cast<render_object_3d>(child))
            {
                ro->set_scene(scene);
            }
        }
    }

    ref<class object_3d> render_object_3d::scene()
    {
        return m_scene;
    }
} // namespace nle
