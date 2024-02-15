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

    mesh_3d(const std::vector<struct vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<class texture>& textures = {});
    virtual ~mesh_3d();

    void set_shader(ref<class shader> shader);
    ref<class shader> shader();

    void set_material(ref<class material> material);
    ref<class material> material();
    void clear_material_override();

    const std::vector<struct vertex>& vertices();

    const std::vector<uint32_t>& indices();
    
    const std::vector<class texture>& textures();
private:

    /// vertices
    std::vector<struct vertex> m_vertices;

    /// indices
    std::vector<uint32_t> m_indices;

    /// textures
    std::vector<class texture> m_textures;

    /// element buffer object
    unsigned int m_ebo;

    ///  vertex buffer object
    unsigned int m_vbo;

    /// vertex array object
    unsigned int m_vao;

    /// triangles, lines or dots?
    unsigned int m_primitive_type;

    ref<class shader> m_shader;

    ref<class material> m_material;

    ref<class material> m_material_override;

    void load();
};

} // namespace nle
