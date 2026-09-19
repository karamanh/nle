/**
 * @file gltf_instance_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief A placed, animatable instance of a glTF model.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "model_gltf.h"

#include "../animation/animator.h"
#include "../mesh/multimesh_instance_3d.h"

namespace nle
{

/**
 * @brief An instance of a model_gltf, with its own animation playback state.
 *
 * Geometry, materials and the rig are shared with the model; the pose is not,
 * so two instances of the same model can play different clips at different
 * points in time.
 *
 * Playback is advanced from render(), using the frame's delta time, so an
 * instance animates simply by being in a scene:
 *
 *     auto knight = model->create_gltf_instance();
 *     knight->animator()->play("walk");
 *     scene->add_child(knight);
 *
 * A consequence worth knowing: an instance that is culled, hidden or on a
 * disabled layer is not rendered and therefore does not advance. Drive
 * animator()->update() yourself if you need it to keep time regardless.
 */
class gltf_instance_3d : public multimesh_instance_3d
{
public:
    explicit gltf_instance_3d(ref<model_gltf> model);
    virtual ~gltf_instance_3d();

    ref<class animator> animator();

    ref<model_gltf> model() const;

    /// Whether render() advances the animator. On by default.
    void set_auto_advance(bool auto_advance);
    bool auto_advance() const;

    void render(render_command_buffer& command_buffer, const render_context& context) override;

private:
    ref<model_gltf> m_model;
    ref<class animator> m_animator;
    bool m_auto_advance = true;
};

} // namespace nle
