/**
 * @file mesh_instance_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "mesh_3d.h"
#include "../renderer/render_object_3d.h"

namespace nle
{

class mesh_instance_3d : public render_object_3d
{
public:
    mesh_instance_3d(ref<class mesh_3d> mesh);
    virtual ~mesh_instance_3d();

    ref<class mesh_3d> mesh(); 

    /**
     * @brief Swaps the geometry, keeping the transform and the material.
     *
     * For something whose shape is one of a small set built in advance --
     * the same circle in each school's colour, say -- where rebuilding the
     * instance would lose where it was and whether it was showing.
     */
    void set_mesh(ref<class mesh_3d> mesh);

    void render(render_command_buffer& command_buffer, const render_context& context) override;

    bool see_through() override;
protected:
    /**
     * @brief Uniforms recorded after the shared ones and before the draw.
     *
     * A subclass that sets uniforms around render() finds them overwritten:
     * the shared surface uniforms are recorded inside it, and the draw is the
     * last thing recorded. This is the gap between the two, and so the only
     * place a subclass's own uniforms survive to reach the draw.
     */
    virtual void record_extra_uniforms(render_command_buffer&, const render_context&) {}

    /// Subclasses such as terrain_3d rebuild their own geometry.
    ref<class mesh_3d> m_mesh;
};

} // namespace nle
