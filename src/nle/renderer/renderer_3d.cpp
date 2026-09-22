#include "renderer_3d.h"

#include <glm/gtc/matrix_transform.hpp>

namespace nle
{
    renderer_3d::renderer_3d(ref<window_glfw> render_target)
        : m_render_target(render_target)
    {
        glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        render_target->render_3d() = [this](){
            this->main_routine();
        };
    }

    renderer_3d::~renderer_3d()
    {
    }

    void renderer_3d::set_current_scene(ref<scene_3d> scene)
    {
        m_current_scene = scene;
    }

    ref<scene_3d> renderer_3d::current_scene()
    {
        return m_current_scene;
    }

    class bloom& renderer_3d::bloom()
    {
        return m_bloom;
    }

    ref<window_glfw> renderer_3d::render_target()
    {
        return m_render_target;
    }

    void renderer_3d::set_render_layer_attribute(enum render_layer layer, const struct render_layer_attribute& attribute)
    {
        m_render_layer_attributes[layer] = attribute;
    }

    render_layer_attribute renderer_3d::render_layer_attribute_of(enum render_layer layer)
    {
        return m_render_layer_attributes[layer];
    }

    const opengl_backend::frame_statistics& renderer_3d::statistics() const
    {
        return m_opengl_backend.statistics();
    }

    float renderer_3d::delta_time() const
    {
        return m_delta_time;
    }

    void renderer_3d::set_clear_color(const glm::vec3& color)
    {
        m_clear_color = color;
        glClearColor(color.r, color.g, color.b, 1.0f);
    }

    glm::vec3 renderer_3d::clear_color() const
    {
        return m_clear_color;
    }

    render_context renderer_3d::build_render_context(ref<scene_3d> scene)
    {
        render_context context;

        auto camera = scene->camera();

        context.resolution = scene->target_resolution();
        const float aspect_ratio = context.resolution.y > 0.0f
            ? context.resolution.x / context.resolution.y
            : 1.0f;

        context.projection = glm::perspective(camera->field_of_view(), aspect_ratio, camera->near(), camera->far());
        context.view = camera->view_matrix();
        context.eye_position = camera->position();
        context.time = m_time;
        context.delta_time = m_delta_time;

        context.directional_light = scene->light()->to_directional_light_data();
        context.point_lights = scene->collect_point_lights(context.eye_position);

        context.fog = scene->fog();

        return context;
    }

    bool renderer_3d::is_visible(const ref<render_object_3d>& ro, const glm::vec3& eye)
    {
        if(!ro->visible())
        {
            return false;
        }

        const auto& attribute = m_render_layer_attributes[ro->render_layer()];
        if(!attribute.visible)
        {
            return false;
        }

        return glm::distance(ro->position(), eye) < attribute.render_distance;
    }

    class shadow_map& renderer_3d::shadows()
    {
        return m_shadows;
    }

    void renderer_3d::set_depth_shader(ref<class shader> shader)
    {
        m_depth_shader = std::move(shader);
    }

    void renderer_3d::render_scene(ref<scene_3d> scene)
    {
        render_context context = build_render_context(scene);

        // What the sun can see, drawn first. Everything is recorded exactly
        // as it would be for the lit pass -- same objects, same skinning,
        // same model matrices -- and drawn with one shader that keeps only
        // depth, so a character's shadow is the shape the character is
        // actually in rather than the shape of its bind pose.
        const bool casting = m_shadows.enabled() && m_depth_shader
                          && context.directional_light.enabled;

        if(casting)
        {
            context.light_space = m_shadows.aim(context.eye_position,
                                                context.directional_light.direction);
            context.shadows_enabled = true;

            if(m_shadows.begin())
            {
                render_context from_the_sun = context;

                // The light's box, as one matrix. The depth shader still
                // multiplies projection by view, so one of them is enough
                // and the other is nothing.
                from_the_sun.projection = context.light_space;
                from_the_sun.view = glm::mat4(1.0f);

                m_shadow_commands.clear();

                for(auto ro : scene->render_objects())
                {
                    // Everything casts, whatever the eye can see: a thing
                    // behind you is exactly what puts a shadow in front of
                    // you. The sky does not, having no shape to speak of.
                    ro->render(m_shadow_commands, from_the_sun);
                }

                m_opengl_backend.force_shader(m_depth_shader);
                m_opengl_backend.execute_commands(m_shadow_commands);
                m_opengl_backend.force_shader(nullptr);

                m_shadows.end(m_render_target->width(), m_render_target->height());
            }
            else
            {
                context.shadows_enabled = false;
            }
        }

        if(context.shadows_enabled)
        {
            m_shadows.bind_for_reading(context.shadow_texture_unit);
        }

        m_command_buffer.clear();
        m_opengl_backend.begin_frame(context);

        if(scene->sky())
        {
            scene->sky()->set_position(context.eye_position);
            scene->sky()->render(m_command_buffer, context);
        }

        for(auto ro : scene->render_objects())
        {
            if(is_visible(ro, context.eye_position))
            {
                ro->render(m_command_buffer, context);
            }
        }

        // Execute all commands through OpenGL backend
        m_opengl_backend.execute_commands(m_command_buffer);
    }

    void renderer_3d::main_routine()
    {
        const int64_t now_us = m_clock.elapsed_time_us();
        m_delta_time = static_cast<float>(now_us - m_last_frame_us) / 1000000.0f;
        m_last_frame_us = now_us;
        m_time = static_cast<float>(now_us) / 1000000.0f;

        const int width = m_render_target->width();
        const int height = m_render_target->height();

        // The scene goes into bloom's own target when it is on, and straight
        // to the window when it is not. Either way it is cleared once, here.
        if(!m_bloom.begin(width, height))
        {
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        if(m_current_scene)
        {
            m_current_scene->set_target_resolution(glm::vec2(width, height));
            render_scene(m_current_scene);
        }

        // Nothing if bloom did not begin, so the interface drawn after this
        // lands on the window either way.
        m_bloom.end();

        // glfw specific functions like glfwSwapBuffers are called
        // from window_glfw::display().
        // we only have gl specific function calls here.
    }

} // namespace nle
