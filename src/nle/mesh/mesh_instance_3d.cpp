#include "mesh_instance_3d.h"
#include "../scene/scene_3d.h"

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

    void mesh_instance_3d::render(render_command_buffer& command_buffer)
    {
        auto scene = std::dynamic_pointer_cast<scene_3d>(this->scene());
        if(!scene)
        {
            return;
        }

        command_buffer.set_polygon_mode(GL_FRONT_AND_BACK, static_cast<GLenum>(render_mode()));

        command_buffer.use_shader(this->shader());

        // Handle texture
        if(this->mesh()->texture())
        {
            command_buffer.set_uniform("u_texture_enabled", 1);
            command_buffer.use_texture(this->mesh()->texture());
        }
        else
        {
            command_buffer.set_uniform("u_texture_enabled", 0);
        }

        bool accept_light = true;

        // Handle material uniforms
        auto material = this->material_override() ? this->material_override() : this->mesh()->material();
        if(material)
        {
            command_buffer.set_uniform("u_material.ambient", material->ambient());
            command_buffer.set_uniform("u_material.diffuse", material->diffuse());
            command_buffer.set_uniform("u_material.specular", material->specular());
            command_buffer.set_uniform("u_material.shininess", material->shininess());
            command_buffer.set_uniform("u_material.dissolve", material->dissolve());
            command_buffer.set_uniform("u_material.accept_light", static_cast<int>(material->accept_light()));
            accept_light = material->accept_light();
        }
        
        // Handle lighting
        if (accept_light && scene->light()->enabled())
        {
            command_buffer.set_uniform("u_lighting_enabled", 1);
            command_buffer.set_uniform("u_directional_light.color", scene->light()->color());
            command_buffer.set_uniform("u_directional_light.ambient", scene->light()->ambient());
            command_buffer.set_uniform("u_directional_light.diffuse", scene->light()->diffuse());
            command_buffer.set_uniform("u_directional_light.specular", scene->light()->specular());
            command_buffer.set_uniform("u_directional_light.direction", scene->light()->front());
        }
        else
        {
            command_buffer.set_uniform("u_lighting_enabled", 0);
        }

        // Calculate matrices
        glm::mat4 model = glm::mat4(1.0f);
        float aspect_ratio = scene->target_resolution().x / scene->target_resolution().y;
        glm::mat4 projection = glm::perspective(scene->camera()->field_of_view(), aspect_ratio, scene->camera()->near(), scene->camera()->far());

        model = glm::translate(model, this->position());
        model = glm::rotate(model, glm::radians(this->rotation().x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(this->rotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(this->rotation().z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, this->scale());

        // Set matrix uniforms
        command_buffer.set_uniform("u_model", model);
        command_buffer.set_uniform("u_projection", projection);
        command_buffer.set_uniform("u_view", scene->camera()->view_matrix());
        command_buffer.set_uniform("u_eye_position", scene->camera()->position());

        // Draw the mesh
        command_buffer.draw_elements(static_cast<GLenum>(this->primitive_type()), this->mesh()->m_vao, this->mesh()->m_ebo, this->mesh()->indices().size());
    }
} // namespace nle
