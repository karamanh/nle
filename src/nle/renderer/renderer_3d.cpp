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

        if(auto sky = scene->sky())
        {
            context.fog.enabled = sky->distance_fog_enabled();
            context.fog.near_distance = sky->distance_fog_near();
            context.fog.far_distance = sky->distance_fog_far();
            context.fog.color = sky->distance_fog_color();
        }

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

    void renderer_3d::render_scene(ref<scene_3d> scene)
    {
        const render_context context = build_render_context(scene);

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

        glViewport(0, 0, m_render_target->width(), m_render_target->height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(m_current_scene)
        {
            m_current_scene->set_target_resolution(glm::vec2(m_render_target->width(), m_render_target->height()));
            render_scene(m_current_scene);
        }

        // glfw specific functions like glfwSwapBuffers are called
        // from window_glfw::display().
        // we only have gl specific function calls here.
    }

} // namespace nle
