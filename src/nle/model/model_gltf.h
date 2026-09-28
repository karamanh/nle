/**
 * @file model_gltf.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief glTF 2.0 (.gltf / .glb) model loading, including skins and animations.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "model.hpp"

#include "../animation/animation.h"
#include "../animation/skeleton.h"

namespace nle
{

class gltf_instance_3d;

/**
 * @brief One drawable primitive of a glTF mesh.
 *
 * glTF separates what to draw (a mesh primitive) from where to draw it (the
 * node referencing that mesh). Loading flattens the two: one entry per
 * (node, primitive) pair, which is also the order they are drawn in.
 */
struct gltf_primitive
{
    ref<mesh_3d> mesh;

    /// index into skeleton::nodes(); positions the primitive when not skinned.
    int node = -1;

    /// index into skeleton::skins(), or -1 when the primitive is rigid.
    int skin = -1;
};

/**
 * @brief A glTF 2.0 model: geometry, materials, a rig and its animations.
 *
 * Loads both the JSON (.gltf) and binary (.glb) containers. The loaded data is
 * immutable and shared; call create_instance() for something you can place in
 * a scene, once per copy you want.
 *
 * Supported: triangle primitives, base colour textures and factors, node
 * hierarchies, skins with up to MAX_JOINTS joints, and translation/rotation/
 * scale animation channels with step, linear and cubic spline interpolation.
 *
 * Not supported: morph targets, cameras, draco compression, and the metallic
 * roughness model proper -- metallic/roughness are approximated onto the
 * engine's Phong material.
 */
class model_gltf : public model
{
public:
    /// @throws std::runtime_error if the file cannot be read or parsed.
    explicit model_gltf(const std::string& path);
    virtual ~model_gltf();

    /// A placeable, animatable instance of this model.
    ref<multimesh_instance_3d> create_instance() override;

    /// As create_instance(), typed so that animator() is reachable.
    ref<gltf_instance_3d> create_gltf_instance();

    const std::vector<gltf_primitive>& primitives() const;

    /// The shared rig. Never null, but may have no nodes.
    ref<class skeleton> skeleton() const;

    const std::vector<ref<animation_clip>>& animations() const;
    std::vector<std::string> animation_names() const;

    /**
     * @brief Takes on another model's clips, retargeted onto this rig.
     *
     * For a family of models rigged alike but animated only once -- five
     * suits of armour on one body, of which only the first came with more
     * than a walk. Bones are matched by the shape of the two rigs (see
     * match_humanoid_bones), so the two need not agree about names.
     *
     * Only affects instances created afterwards: an instance copies the
     * clip list when it is made.
     *
     * @param replace  whether a clip this model already has is replaced by
     *                 the donor's. Off by default: a model's own clip was
     *                 made for it, and is the better of the two.
     * @return how many clips were added or replaced.
     */
    size_t borrow_animations(const model_gltf& donor, bool replace = false);

    /// True when the file contains at least one skin with joints.
    bool skinned() const;

    std::string name() const;
    const std::string& path() const;

private:
    void load(const std::string& path) override;

    std::string m_path;

    std::vector<gltf_primitive> m_primitives;
    ref<class skeleton> m_skeleton;
    std::vector<ref<animation_clip>> m_animations;
};

} // namespace nle
