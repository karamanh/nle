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

    void mesh_instance_3d::render()
    {
        auto scene = std::dynamic_pointer_cast<scene_3d>(this->scene());
        if(!scene)
        {
            return;
        }

        glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(render_mode()));

        this->shader()->use();

        // matrices and stuff
        GLuint unf_model = this->shader()->uniform_location("u_model");
        GLuint unf_proj = this->shader()->uniform_location("u_projection");
        GLuint unf_view = this->shader()->uniform_location("u_view");
        GLuint unf_eye_position = this->shader()->uniform_location("u_eye_position");

        // light
        GLuint unf_light_color = this->shader()->uniform_location("u_directional_light.color");
        GLuint unf_light_ambient = this->shader()->uniform_location("u_directional_light.ambient");
        GLuint unf_light_diffuse = this->shader()->uniform_location("u_directional_light.diffuse");
        GLuint unf_light_specular = this->shader()->uniform_location("u_directional_light.specular");
        GLuint unf_light_direction = this->shader()->uniform_location("u_directional_light.direction");
        GLuint unf_light_enabled = this->shader()->uniform_location("u_lighting_enabled");
        // material
        GLuint unf_material_shininess = this->shader()->uniform_location("u_material.shininess");
        GLuint unf_material_accept_light = this->shader()->uniform_location("u_material.accept_light");
        GLuint unf_material_ambient = this->shader()->uniform_location("u_material.ambient");
        GLuint unf_material_diffuse = this->shader()->uniform_location("u_material.diffuse");
        GLuint unf_material_specular = this->shader()->uniform_location("u_material.specular");
        GLuint unf_material_dissolve = this->shader()->uniform_location("u_material.dissolve");
        // other
        GLuint unf_texture_enabled = this->shader()->uniform_location("u_texture_enabled");
        /// TODO: implement a sky class, then re-enable and set following uniforms.
        // GLuint unf_sky_distance_fog_enabled = this->shader()->uniform_location("u_sky.distance_fog_enabled");
        // GLuint unf_sky_distance_fog_near = this->shader()->uniform_location("u_sky.distance_fog_near");
        // GLuint unf_sky_distance_fog_far = this->shader()->uniform_location("u_sky.distance_fog_far");

        /// TODO: For now i don't know how, but later we might use multiple textures
        /// needs to be implemented though. Until then, we only use textures()[0]
        if (!mesh()->textures().empty())
        {
            this->mesh()->textures()[0].use();
            glUniform1i(unf_texture_enabled, 1);
        }
        else
        {
            glUniform1i(unf_texture_enabled, 0);
        }

        /// TODO: render sky

        bool accept_light = true;

        if(this->mesh()->material())
        {
            glUniform3f(unf_material_ambient, this->mesh()->material()->ambient().x, this->mesh()->material()->ambient().y, this->mesh()->material()->ambient().z);
            glUniform3f(unf_material_diffuse, this->mesh()->material()->diffuse().x, this->mesh()->material()->diffuse().y, this->mesh()->material()->diffuse().z);
            glUniform3f(unf_material_specular, this->mesh()->material()->specular().x, this->mesh()->material()->specular().y, this->mesh()->material()->specular().z);
            glUniform1f(unf_material_shininess, this->mesh()->material()->shininess());
            glUniform1f(unf_material_dissolve, this->mesh()->material()->dissolve());
            glUniform1i(unf_material_accept_light, static_cast<int>(this->mesh()->material()->accept_light()));
            accept_light = this->mesh()->material()->accept_light();
        }
        
        if (accept_light && scene->light()->enabled())
        {
            glUniform1i(unf_light_enabled, 1);
            glUniform3f(unf_light_color, scene->light()->color().x, scene->light()->color().y, scene->light()->color().z);
            glUniform3f(unf_light_ambient, scene->light()->ambient().x, scene->light()->ambient().y, scene->light()->ambient().z);
            glUniform3f(unf_light_diffuse, scene->light()->diffuse().x, scene->light()->diffuse().y, scene->light()->diffuse().z);
            glUniform3f(unf_light_specular, scene->light()->specular().x, scene->light()->specular().y, scene->light()->specular().z);
            glUniform3f(unf_light_direction, scene->light()->front().x, scene->light()->front().y, scene->light()->front().z);
        }
        else
        {
            glUniform1i(unf_light_enabled, 0);
        }

        glm::mat4 model = glm::mat4(1.0f);

        float aspect_ratio = scene->target_resolution().x / scene->target_resolution().y;
        glm::mat4 projection = glm::perspective(scene->camera()->field_of_view(), aspect_ratio, scene->camera()->near(), scene->camera()->far());

        model = glm::translate(model, this->position());
        model = glm::rotate(model, glm::radians(this->rotation().x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(this->rotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(this->rotation().z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, this->scale());

        glUniformMatrix4fv(unf_model, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(unf_proj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(unf_view, 1, GL_FALSE, glm::value_ptr(scene->camera()->view_matrix()));
        glUniform3f(unf_eye_position, scene->camera()->position().x, scene->camera()->position().y, scene->camera()->position().z);

        glBindVertexArray(this->mesh()->m_vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->mesh()->m_ebo);
        glDrawElements(static_cast<GLenum>(this->primitive_type()), this->mesh()->indices().size(), GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        this->shader()->unuse();
    }
} // namespace nle
