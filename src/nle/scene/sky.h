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

    /**
     * @brief The box drawn behind everything else.
     *
     * Distance fog used to live here and now lives on scene_3d. Fog is a
     * property of the air in a scene rather than of the backdrop, and keeping
     * it here meant a scene could not have fog without also having a skybox.
     */
    class sky : public mesh_instance_3d
    {
    public:
        sky(ref<mesh_3d> mesh = make_ref<boxmesh>());
        virtual ~sky();

        void render(render_command_buffer& command_buffer, const render_context& context) override;
    };

} // namespace nle
