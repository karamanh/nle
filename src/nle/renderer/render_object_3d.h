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

#include "../object/object_3d.h"
#include "../renderer/shader.h"
#include "render_command.h"
#include "render_context.h"

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

    /**
     * @brief Records the draw for this object into @p command_buffer.
     *
     * @p context carries everything that is constant for the frame (camera,
     * lights, fog). Objects must not re-upload those; the backend does it once
     * per shader program per frame.
     */
    virtual void render(render_command_buffer& command_buffer, const render_context& context) = 0;

    /**
     * @brief Whether anything it draws is see-through.
     *
     * Something see-through does not write depth -- it must not, or it would
     * hide what is behind it -- which means anything opaque drawn after it
     * paints straight over it. So the renderer draws these last, and this is
     * how it knows which they are.
     *
     * False by default: a thing that says nothing is drawn as though it were
     * solid, which is the safe way round. Drawing an opaque thing late costs
     * nothing; drawing a see-through one early loses it.
     */
    virtual bool see_through() { return false; }

    /**
     * @brief The radius of a sphere around this, in world units.
     *
     * For asking whether the camera could see it at all. Zero means it does
     * not know its own size, and nothing that says zero is ever culled by
     * shape -- which is the safe way round, since drawing something need-
     * lessly costs a draw and skipping something wrongly is a hole in the
     * world.
     */
    virtual float bounding_radius() const { return 0.0f; }

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

    virtual void set_material_override(ref<class material> material_override);
    virtual ref<class material> material_override();

    ref<class render_object_3d> scene();

    std::set<ref<class render_object_3d>, std::owner_less<ref<class render_object_3d>>> render_objects();

    virtual void add_child(ref<object_3d> child) override;
    virtual void delete_child(ref<object_3d> child) override;
    
    nlohmann::json to_json() const override;
    void from_json(const nlohmann::json& j) override;

protected:
    /**
     * @brief Tells this object, and everything under it, which scene it is in.
     *
     * Called by scene_3d as objects are added. Override it to react to joining
     * a scene -- terrain_3d uses it to hand over props that were scattered
     * before there was a scene to put them in -- and call the base version.
     */
    virtual void set_scene(ref<class render_object_3d> scene);

private:
    ref<class shader> m_shader;
    ref<class render_object_3d> m_scene;
    ref<class material> m_material_override;

    std::set<ref<class render_object_3d>, std::owner_less<ref<class render_object_3d>>> m_render_objects;

    enum render_layer m_render_layer;
    enum render_mode m_render_mode;
    enum primitive_type m_primitive_type;

    bool m_visible;

    friend class scene_3d;
    friend class renderer_3d;
};

} // namespace nle
