/**
 * @file mesh_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "../core/ref.h"
#include "../renderer/vertex.h"
#include "../renderer/shader.h"
#include "../renderer/texture.h"
#include "../renderer/material.h"

#include <vector>

namespace nle
{

class mesh_3d
{
public:

    mesh_3d(const std::vector<struct vertex>& vertices, const std::vector<uint32_t>& indices, ref<class texture> texture);
    virtual ~mesh_3d();

    void set_shader(ref<class shader> shader);
    ref<class shader> shader();

    void set_material(ref<class material> material);
    ref<class material> material();
    void clear_material_override();

    const std::vector<struct vertex>& vertices();

    /**
     * @brief Rewrites a run of vertices in place, without rebuilding anything.
     *
     * For geometry that changes shape but not size -- terrain under a brush,
     * where a stroke moves a few hundred vertices of a hundred thousand and
     * rebuilding the mesh to say so costs more than the stroke did.
     *
     * @return false if the run falls outside the mesh.
     */
    bool update_vertices(size_t first, const struct vertex* data, size_t count);

    const std::vector<uint32_t>& indices();

    void set_texture(ref<class texture> texture);
    ref<class texture> texture();

    /// GL object names, for recording draw commands.
    unsigned int vao() const;
    unsigned int ebo() const;

    /**
     * @brief How far the furthest vertex is from the mesh's own origin.
     *
     * The radius of a sphere around the geometry, in the mesh's own space,
     * for asking whether the camera could see it before going to the
     * trouble of drawing it. Worked out on the first ask and kept, since
     * geometry is built far more rarely than it is looked at.
     */
    float bounding_radius() const;
private:

    /// vertices
    std::vector<struct vertex> m_vertices;

    /// indices
    std::vector<uint32_t> m_indices;

    /// textures
    ref<class texture> m_texture;

    /// element buffer object
    unsigned int m_ebo;

    ///  vertex buffer object
    unsigned int m_vbo;

    /// vertex array object
    unsigned int m_vao;

    ref<class shader> m_shader;

    ref<class material> m_material;

    ref<class material> m_material_override;

    /// Negative until somebody asks. Sculpting the vertices puts it back.
    mutable float m_bounding_radius = -1.0f;

    void load();

    friend class mesh_instance_3d;
    friend class gltf_instance_3d;
};

} // namespace nle
