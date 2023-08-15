#include "Renderer3D.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define NLE_INPUT_PROCESS_SLEEP_TIME            1
#define NLE_RENDER_MAX_ELAPSED_TIME_SECONDS     0.016666

namespace nle
{
    Renderer3D::Renderer3D(Window *render_target)
        : m_parent_window(render_target),
        m_max_render_distance(10000.f),
        m_time_strict_mode(false),
        m_imgui(new ImGUI_GLFW(render_target->m_handle))
    {
        m_grid = new Shader("shader/grid_vert.glsl", "shader/grid_frag.glsl", true);

        for (auto &it : m_render_layer_attributes)
        {
            it.second.render_distance = m_max_render_distance;
        }

        initialize();
        m_parent_window->set_draw_callback(
            [this]()
            {
                glViewport(0, 0, this->m_parent_window->m_width, this->m_parent_window->m_height);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                this->render_scene(this->m_root_scene);
                this->gui()->render();
                m_render_timestamp = get_time_sec();
            });
    }

    Renderer3D::~Renderer3D()
    {
        // if (m_thr_input.joinable())
        //     m_thr_input.join();
    }

    void Renderer3D::render_recursively(Object3D *root)
    {
        if (!root)
        {
            prerr("root spatial is null");
            return;
        }

        if(m_time_strict_mode)
        {
            double now = get_time_sec();

            if((now - m_render_timestamp) > NLE_RENDER_MAX_ELAPSED_TIME_SECONDS)
            {
                m_render_timestamp = now;
                return;
            }
        }

        if (MeshInstance *m = dynamic_cast<MeshInstance *>(root))
        {
            if (m->visible())
            {
                render(m);
            }
        }

        for (auto *c : root->children())
        {
            render_recursively(c);
        }
    }

    void Renderer3D::render_scene(Scene *scene)
    {
        for(auto *i : scene->m_render_objects)
        {
            if(i)
            {
                MeshInstance *mi;
                if((mi = dynamic_cast<MeshInstance*>(i)))
                {
                    render(mi);
                }
            }
        }
    }

    void Renderer3D::initialize()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glDepthFunc(GL_LEQUAL);
        glClearColor(0.0f, 0.24f, 0.38f, 1.f);
    }

    ImGUI_GLFW *Renderer3D::gui() const
    {
        return m_imgui;
    }

    void Renderer3D::render(MeshInstance *mi)
    {
        if (!mi)
        {
            return;
        }

        if (!m_render_layer_attributes[mi->m_render_layer].visible)
        {
            return;
        }

        if (glm::distance(static_cast<Scene *>(mi->root())->camera()->position(), mi->position()) >
            m_render_layer_attributes[mi->m_render_layer].render_distance)
        {
            return;
        }

        Scene *s = static_cast<Scene *>(mi->root());
        if (!s)
        {
            return;
        }

        glPolygonMode(GL_FRONT_AND_BACK, mi->m_render_mode);

        mi->mesh()->shader()->use();

        GLuint unf_model = mi->mesh()->shader()->uniform_location("model");
        GLuint unf_proj = mi->mesh()->shader()->uniform_location("projection");
        GLuint unf_view = mi->mesh()->shader()->uniform_location("view");
        GLuint unf_ambient_intensity = mi->mesh()->shader()->uniform_location("directionalLight.ambientIntensity");
        GLuint unf_ambient_color = mi->mesh()->shader()->uniform_location("directionalLight.color");
        GLuint unf_diffuse_direction = mi->mesh()->shader()->uniform_location("directionalLight.direction");
        GLuint unf_diffuse_intensity = mi->mesh()->shader()->uniform_location("directionalLight.diffuseIntensity");
        GLuint unf_light_enabled = mi->mesh()->shader()->uniform_location("lightingEnabled");
        GLuint unf_texture_enabled = mi->mesh()->shader()->uniform_location("textureEnabled");
        GLuint unf_specular_intensity = mi->mesh()->shader()->uniform_location("material.specularIntensity");
        GLuint unf_shininess = mi->mesh()->shader()->uniform_location("material.shininess");
        GLuint unf_eye_position = mi->mesh()->shader()->uniform_location("eyePosition");

        if (mi->mesh()->texture())
        {
            mi->mesh()->texture()->use();
            glUniform1i(unf_texture_enabled, 1);
        }
        else
        {
            glUniform1i(unf_texture_enabled, 0);
        }

        if (s->light()->enabled())
        {
            glUniform1i(unf_light_enabled, 1);
            s->light()->use(unf_ambient_intensity, unf_ambient_color, unf_diffuse_intensity, unf_diffuse_direction);
        }
        else
        {
            glUniform1i(unf_light_enabled, 0);
        }

        if(mi->mesh()->material())
        {
            mi->mesh()->material()->use(unf_specular_intensity, unf_shininess);
        }

        glm::mat4 model = glm::mat4(1.f);
        glm::mat4 projection = glm::perspective(45.f, (GLfloat)m_parent_window->m_width / (GLfloat)m_parent_window->m_height, 0.1f, m_max_render_distance);

        model = glm::translate(model, mi->position());
        model = glm::rotate(model, glm::radians(mi->rotation().x), glm::vec3(1.f, 0.f, 0.f));
        model = glm::rotate(model, glm::radians(mi->rotation().y), glm::vec3(0.f, 1.f, 0.f));
        model = glm::rotate(model, glm::radians(mi->rotation().z), glm::vec3(0.f, 0.f, 1.f));
        model = glm::scale(model, mi->scale());

        glUniformMatrix4fv(unf_model, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(unf_proj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(unf_view, 1, GL_FALSE, glm::value_ptr(s->camera()->get_view_matrix()));
        glUniform3f(unf_eye_position, s->camera()->position().x, s->camera()->position().y, s->camera()->position().z);

        glBindVertexArray(mi->mesh()->m_vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mi->mesh()->m_ebo);
        glDrawElements(mi->mesh()->m_primitive_type, mi->mesh()->indices()->size(), GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glUseProgram(0);

        if (mi->mesh()->texture())
            mi->mesh()->texture()->unuse();
    }

    void Renderer3D::set_root_scene(Scene *root)
    {
        m_root_scene = root;

        m_parent_window->input_handler()->mouse_moved().unbind_all();
        m_parent_window->input_handler()->key_pressed().unbind_all();
        
        m_parent_window->input_handler()->mouse_moved().bind_callback(
            std::bind(&Camera::on_mouse_motion, root->camera(), std::placeholders::_1, std::placeholders::_2)
        );
        m_parent_window->input_handler()->key_pressed().bind_callback(
            std::bind(&Camera::on_key_pressed, root->camera(), std::placeholders::_1)
        );
    }

    Scene *Renderer3D::root_scene() const
    {
        return m_root_scene;
    }

    void Renderer3D::set_render_layer_attributes(RenderLayer layer, RenderLayerAttributes attributes)
    {
        m_render_layer_attributes[layer] = attributes;
    }

    RenderLayerAttributes Renderer3D::render_layer_attributes(RenderLayer layer)
    {
        return m_render_layer_attributes[layer];
    } 

} // namespace nle
