/**
 * @file terrain_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Ground: a grid mesh you can stand on, pick with the mouse and decorate.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "../core/ray.h"
#include "../mesh/mesh_instance_3d.h"

#include <functional>

namespace nle
{

/**
 * @brief A square patch of ground, centred on the object's position.
 *
 * Flat by default. Give it a height function to make it uneven; the mesh, its
 * normals and the results of height_at() all follow from that one function, so
 * what you see and what you pick cannot disagree.
 *
 * Decoration is ordinary parenting: add_child() a model and use
 * place_on_surface() to sit it on the ground.
 *
 *     auto tree = model->create_instance();
 *     tree->set_position(terrain->place_on_surface({12.0f, 0.0f, -4.0f}));
 *     terrain->add_child(tree);
 */
class terrain_3d : public mesh_instance_3d
{
public:
    /**
     * @param size       world units along each edge
     * @param resolution tiles along each edge; vertices are not shared between
     *                   tiles, so the checker pattern has hard edges
     */
    terrain_3d(float size = 100.0f, int resolution = 50);
    virtual ~terrain_3d();

    /**
     * @brief Sets the surface height, sampled in the terrain's local XZ space,
     *        and rebuilds the mesh.
     *
     * Pass nullptr to go back to flat, which also makes picking exact rather
     * than iterative.
     */
    void set_height_function(std::function<float(float, float)> height);

    /// Alternating tile colours.
    void set_colors(const glm::vec3& first, const glm::vec3& second);

    void rebuild();

    float size() const;
    int resolution() const;

    /// Height of the surface under a world-space XZ position.
    float height_at(float x, float z) const;

    /// Whether a world-space XZ position is over the patch at all.
    bool contains(float x, float z) const;

    /// Copy of @p position moved vertically onto the surface.
    glm::vec3 place_on_surface(const glm::vec3& position) const;

    /**
     * @brief Where @p r first meets the ground.
     *
     * Flat terrain is solved directly. An uneven one is marched along the ray
     * until the ray drops below the surface, then bisected -- good enough for
     * picking a spot to aim a spell at, and it needs no acceleration structure.
     *
     * @return false if the ray never meets the patch.
     */
    bool raycast(const ray& r, glm::vec3& hit) const;

private:
    float m_size;
    int m_resolution;

    glm::vec3 m_first_color = glm::vec3(0.42f, 0.44f, 0.38f);
    glm::vec3 m_second_color = glm::vec3(0.33f, 0.35f, 0.30f);

    std::function<float(float, float)> m_height;

    /// Local height, before the object's own transform.
    float local_height(float x, float z) const;

    ref<class mesh_3d> build_mesh() const;
};

} // namespace nle
