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

namespace nle
{

    class sky : public mesh_instance_3d
    {
    public:
        sky();
        virtual ~sky();

        void render() override;

        void set_distance_fog_far(float far);
        float distance_fog_far();

        void set_distance_fog_near(float near);
        float distance_fog_near();

    private:
        float m_distance_fog_near;
        float m_distance_fog_far;
    };

} // namespace nle
