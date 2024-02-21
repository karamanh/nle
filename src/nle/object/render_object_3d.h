/**
 * @file render_object_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "object_3d.h"
#include "../renderer/shader.h"

#include <GL/gl.h>

namespace nle
{

enum class render_mode
{
    POINT = GL_POINT,
    LINE = GL_LINE,
    FILL = GL_FILL 
};

enum class render_layer
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

class render_object_3d : public object_3d
{
public:
    render_object_3d(const std::string &id = "");
    virtual ~render_object_3d();

    virtual void render() = 0;

    virtual void set_render_mode(enum render_mode rm);
    virtual enum render_mode render_mode() const;

    virtual void set_render_layer(enum render_layer rl);
    virtual enum render_layer render_layer() const;

    virtual void set_visible(bool visible);
    virtual bool visible() const;

    virtual void set_shader(ref<class shader> shader);
    virtual ref<class shader> shader();

    ref<class object_3d> scene();
    
    nlohmann::json to_json() const override;
    void from_json(const nlohmann::json& j) override;
    
private:
    ref<class shader> m_shader;
    ref<object_3d> m_scene;

    enum render_layer m_render_layer;
    enum render_mode m_render_mode;

    bool m_visible;

    void set_scene(ref<class object_3d> scene);

    friend class scene_3d;
    friend class renderer_3d;
};

} // namespace nle
