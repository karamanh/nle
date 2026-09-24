#include "mesh_instance_3d.h"

#include "../renderer/surface_uniforms.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace nle
{
    mesh_instance_3d::mesh_instance_3d(ref<class mesh_3d> mesh)
    {
        m_mesh = mesh;
    }

    mesh_instance_3d::~mesh_instance_3d()
    {
    }

    ref<class mesh_3d> mesh_instance_3d::mesh()
    {
        return m_mesh;
    }

    bool mesh_instance_3d::see_through()
    {
        const auto& used = this->material_override() ? this->material_override()
                         : (m_mesh ? m_mesh->material() : nullptr);

        return used && used->dissolve() < 1.0f;
    }

    void mesh_instance_3d::set_mesh(ref<class mesh_3d> mesh)
    {
        m_mesh = std::move(mesh);
    }

    void mesh_instance_3d::render(render_command_buffer& command_buffer, const render_context& context)
    {
        if(!m_mesh)
        {
            return;
        }

        command_buffer.set_polygon_mode(GL_FRONT_AND_BACK, static_cast<GLenum>(render_mode()));

        command_buffer.use_shader(this->shader());

        record_surface_uniforms(command_buffer, context, this->mesh()->texture(),
                                this->material_override() ? this->material_override() : this->mesh()->material());

        // this mesh is not skinned.
        command_buffer.set_uniform("u_skinning_enabled", 0);

        command_buffer.set_uniform("u_model", this->transform_matrix());

        // A material that says it is not fully opaque is drawn as though it
        // means it. Without this, dissolve reaches the shader, comes out in
        // the alpha channel, and is thrown away by a pipeline with blending
        // off -- so a wash meant to be a third visible painted solid over
        // whatever was underneath it.
        //
        // Depth writes go off with it, or a transparent thing hides what is
        // behind it as effectively as an opaque one would.
        const auto& used = this->material_override() ? this->material_override()
                                                     : this->mesh()->material();

        const bool see_through = used && used->dissolve() < 1.0f;

        if(see_through)
        {
            command_buffer.set_blending(used->blending());
            command_buffer.set_depth_mask(false);
        }

        // The last word before the draw, for whatever a subclass needs to say
        // that the shared surface uniforms above would otherwise overwrite.
        record_extra_uniforms(command_buffer, context);

        // Draw the mesh
        command_buffer.draw_elements(static_cast<GLenum>(this->primitive_type()), this->mesh()->m_vao, this->mesh()->m_ebo, this->mesh()->indices().size());

        if(see_through)
        {
            command_buffer.set_depth_mask(true);
            command_buffer.set_blending(blend_mode::none);
        }
    }
} // namespace nle
