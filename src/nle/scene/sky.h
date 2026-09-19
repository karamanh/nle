/**
 * @file sky.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-04-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "../mesh/mesh_instance_3d.h"
#include "../mesh/box_mesh.h"

namespace nle
{

    class sky : public mesh_instance_3d
    {
    public:
        sky(ref<mesh_3d> mesh = make_ref<boxmesh>());
        virtual ~sky();

        void render(render_command_buffer& command_buffer, const render_context& context) override;

        void set_distance_fog_enabled(bool enabled);
        bool distance_fog_enabled() const;

        void set_distance_fog_far(float far);
        float distance_fog_far() const;

        void set_distance_fog_near(float near);
        float distance_fog_near() const;

        void set_distance_fog_color(glm::vec3 color);
        glm::vec3 distance_fog_color() const;

    private:
        bool m_distance_fog_enabled = false;
        float m_distance_fog_near = 0.0f;
        float m_distance_fog_far = 1000.0f;
        glm::vec3 m_distance_fog_color = glm::vec3(1.0f);
    };

} // namespace nle
