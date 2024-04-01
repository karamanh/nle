#include "mesh_3d.h"

namespace nle
{

    mesh_3d::mesh_3d(const std::vector<struct vertex> &vertices, const std::vector<uint32_t> &indices, const std::vector<class texture>& textures)
        : m_vertices(vertices),
        m_indices(indices),
        m_textures(textures)
    {
        load();
    }

    mesh_3d::~mesh_3d()
    {
        if (m_ebo != 0)
        {
            glDeleteBuffers(1, &m_ebo);
        }
        if (m_vbo != 0)
        {
            glDeleteBuffers(1, &m_vbo);
        }
        if (m_vao != 0)
        {
            glDeleteBuffers(1, &m_vao);
        }
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
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(vertex), m_vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(m_indices[0]), m_indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));

        glBindVertexArray(0);
    }

} // namespace nle
