/**
 * @file skeleton.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Node hierarchy and skins loaded from a glTF file.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "../core/ref.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <vector>

namespace nle
{

/**
 * @brief One node of a glTF scene graph.
 *
 * Transforms are kept decomposed rather than as a matrix, because that is what
 * animation channels drive: a clip sets translation, rotation or scale
 * independently, and only then is a matrix composed from them.
 */
struct skeleton_node
{
    std::string name;

    int parent = -1;
    std::vector<int> children;

    glm::vec3 translation = glm::vec3(0.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    /// T * R * S for this node, relative to its parent.
    glm::mat4 local_matrix() const;
};

/**
 * @brief A set of joints that together deform one mesh.
 *
 * inverse_bind_matrices[i] takes a vertex from model space into the local space
 * of joints[i] as it was at bind time, so that the joint's animated transform
 * can then be applied to it.
 */
struct skin
{
    std::string name;

    /// indices into skeleton::nodes()
    std::vector<int> joints;

    /// one per entry of joints
    std::vector<glm::mat4> inverse_bind_matrices;
};

/**
 * @brief The rig of a model: its node hierarchy and its skins.
 *
 * Immutable once loaded and shared by every instance of a model. The per
 * instance, animated copy of the node transforms lives in animator.
 */
class skeleton
{
public:
    skeleton() = default;

    const std::vector<skeleton_node>& nodes() const { return m_nodes; }
    std::vector<skeleton_node>& nodes() { return m_nodes; }

    /// indices of the nodes that have no parent.
    const std::vector<int>& roots() const { return m_roots; }
    std::vector<int>& roots() { return m_roots; }

    const std::vector<skin>& skins() const { return m_skins; }
    std::vector<skin>& skins() { return m_skins; }

    /// Index of the first node with this name, or -1.
    int find_node(const std::string& name) const;

private:
    std::vector<skeleton_node> m_nodes;
    std::vector<int> m_roots;
    std::vector<skin> m_skins;
};

} // namespace nle
