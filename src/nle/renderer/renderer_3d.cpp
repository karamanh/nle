#include "renderer_3d.h"

namespace nle
{
    renderer_3d::renderer_3d(ref<window_glfw> render_target)
        : m_render_target(render_target)
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);

        render_target->render_routine() = [&](){
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

    void renderer_3d::render_scene(ref<scene_3d> scene)
    {
        /// TODO: disable depth buffer / render sky / enable depth buffer

        for(auto ro : scene->m_render_objects)
        {
            bool visible = m_render_layer_attributes[ro->render_layer()].visible || 
                glm::distance(ro->position(), scene->camera()->position()) < m_render_layer_attributes[ro->render_layer()].render_distance ||
                ro->visible();

            if(visible)
            {
                ro->render();
            }
        }
    }

    void renderer_3d::render([[maybe_unused]] ref<render_object_3d> ro)
    {
        throw std::runtime_error("renderer_3d::render() not implemented");
    }

    void renderer_3d::load_scene([[maybe_unused]] ref<scene_3d> scene)
    {
        throw std::runtime_error("renderer_3d::load_scene() not implemented");
    }

    void renderer_3d::main_routine()
    {
        glViewport(0, 0, this->m_render_target->width(), this->m_render_target->height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(m_current_scene)
        {
            render_scene(m_current_scene);
        }
    }

} // namespace nle
