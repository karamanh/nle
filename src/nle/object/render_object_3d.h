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

enum class primitive_type
{
    points = GL_POINTS,
    lines = GL_LINES,
    triangles = GL_TRIANGLES
};

enum class render_mode
{
    point = GL_POINT,
    line = GL_LINE,
    fill = GL_FILL 
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

    virtual void set_primitive_type(enum primitive_type pt);
    virtual enum primitive_type primitive_type();

    virtual void set_visible(bool visible);
    virtual bool visible() const;

    virtual void set_shader(ref<class shader> shader);
    virtual ref<class shader> shader();

    ref<class render_object_3d> scene();

    std::set<ref<class render_object_3d>, std::owner_less<ref<class render_object_3d>>> render_objects();

    virtual void add_child(ref<object_3d> child) override;
    virtual void delete_child(ref<object_3d> child) override;
    
    nlohmann::json to_json() const override;
    void from_json(const nlohmann::json& j) override;
    
private:
    ref<class shader> m_shader;
    ref<class render_object_3d> m_scene;
    std::set<ref<class render_object_3d>, std::owner_less<ref<class render_object_3d>>> m_render_objects;

    enum render_layer m_render_layer;
    enum render_mode m_render_mode;
    enum primitive_type m_primitive_type;

    bool m_visible;

    void set_scene(ref<class render_object_3d> scene);

    friend class scene_3d;
    friend class renderer_3d;
};

} // namespace nle
