/**
 * @file surface_uniforms.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Per-draw material and texture uniforms shared by every mesh draw.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "render_command.h"
#include "render_context.h"

namespace nle
{

class material;
class texture;

/**
 * @brief Records the texture and material state for one surface.
 *
 * Every mesh draw needs the same block of uniforms, so both mesh_instance_3d
 * and the glTF instance go through here rather than keeping two copies in step.
 * Frame constants (camera, the light values themselves) are not recorded; the
 * backend uploads those once per shader program per frame.
 *
 * The caller is still responsible for u_model, the skinning uniforms and the
 * draw itself.
 */
void record_surface_uniforms(render_command_buffer& command_buffer,
                             const render_context& context,
                             const ref<class texture>& texture,
                             const ref<class material>& material,
                             const glm::vec3& where = glm::vec3(0.0f));

} // namespace nle
