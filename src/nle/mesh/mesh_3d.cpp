#include "mesh_3d.h"

namespace nle
{

    mesh_3d::mesh_3d(const std::vector<struct vertex> &vertices, const std::vector<uint32_t> &indices, const std::vector<class texture>& textures)
        : m_vertices(vertices),
        m_indices(indices),
        m_textures(textures)
    {
        
    }

    mesh_3d::~mesh_3d()
    {
    }

    void mesh_3d::set_shader(ref<class shader> shader)
    {
        m_shader = shader;
    }

    ref<class shader> mesh_3d::shader()
    {
        return m_shader;
    }

    void mesh_3d::set_material(ref<class material> material)
    {
        m_material = material;
    }

    ref<class material> mesh_3d::material()
    {
        return m_material_override == nullptr ? m_material : m_material_override;
    }

    void mesh_3d::clear_material_override()
    {
        m_material_override.reset();
    }

    const std::vector<struct vertex> &mesh_3d::vertices()
    {
        return m_vertices;
    }

    const std::vector<uint32_t> &mesh_3d::indices()
    {
        return m_indices;
    }

    const std::vector<class texture> &mesh_3d::textures()
    {
        return m_textures;
    }

    void mesh_3d::load()
    {
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    }

} // namespace nle
