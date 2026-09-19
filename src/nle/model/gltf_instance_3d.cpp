#include "gltf_instance_3d.h"

#include "../renderer/surface_uniforms.h"

namespace nle
{

gltf_instance_3d::gltf_instance_3d(ref<model_gltf> model)
    : multimesh_instance_3d(model ? model->multimesh() : make_ref<multimesh_3d>()),
      m_model(model)
{
    if(m_model)
    {
        m_animator = make_ref<class animator>(m_model->skeleton(), m_model->animations());
    }
}

gltf_instance_3d::~gltf_instance_3d()
{
}

ref<class animator> gltf_instance_3d::animator()
{
    return m_animator;
}

ref<model_gltf> gltf_instance_3d::model() const
{
    return m_model;
}

void gltf_instance_3d::set_auto_advance(bool auto_advance)
{
    m_auto_advance = auto_advance;
}

bool gltf_instance_3d::auto_advance() const
{
    return m_auto_advance;
}

void gltf_instance_3d::render(render_command_buffer& command_buffer, const render_context& context)
{
    if(!m_model || !m_animator)
    {
        return;
    }

    if(m_auto_advance)
    {
        m_animator->update(context.delta_time);
    }

    command_buffer.set_polygon_mode(GL_FRONT_AND_BACK, static_cast<GLenum>(render_mode()));
    command_buffer.use_shader(this->shader());

    const glm::mat4 instance_transform = this->transform_matrix();
    const auto& override_material = this->material_override();

    for(const auto& primitive : m_model->primitives())
    {
        if(!primitive.mesh)
        {
            continue;
        }

        record_surface_uniforms(command_buffer, context, primitive.mesh->texture(),
                                override_material ? override_material : primitive.mesh->material());

        if(primitive.skin >= 0)
        {
            const auto& palette = m_animator->joint_matrices(static_cast<size_t>(primitive.skin));

            if(!palette.empty())
            {
                // glTF poses a skinned mesh entirely through its joints, so the
                // node's own transform is not part of the model matrix here.
                command_buffer.set_uniform("u_skinning_enabled", 1);
                command_buffer.set_uniform("u_joint_matrices", palette);
                command_buffer.set_uniform("u_model", instance_transform);
            }
            else
            {
                command_buffer.set_uniform("u_skinning_enabled", 0);
                command_buffer.set_uniform("u_model", instance_transform);
            }
        }
        else
        {
            command_buffer.set_uniform("u_skinning_enabled", 0);
            command_buffer.set_uniform("u_model",
                                       instance_transform * m_animator->node_world_matrix(primitive.node));
        }

        command_buffer.draw_elements(static_cast<GLenum>(this->primitive_type()),
                                     primitive.mesh->vao(),
                                     primitive.mesh->ebo(),
                                     primitive.mesh->indices().size());
    }
}

} // namespace nle
