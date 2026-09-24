#include "mesh_3d.h"

#include <algorithm>
#include <cmath>

namespace nle
{

    mesh_3d::mesh_3d(const std::vector<struct vertex> &vertices, const std::vector<uint32_t> &indices, ref<class texture> texture)
        : m_vertices(vertices),
        m_indices(indices),
        m_texture(texture),
        m_material(make_ref<class material>())
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
            glDeleteVertexArrays(1, &m_vao);
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

    void mesh_3d::set_texture(ref<class texture> texture)
    {
        m_texture = texture;
    }

    ref<class texture> mesh_3d::texture()
    {
        return m_texture;
    }

    unsigned int mesh_3d::vao() const
    {
        return m_vao;
    }

    unsigned int mesh_3d::ebo() const
    {
        return m_ebo;
    }

    float mesh_3d::bounding_radius() const
    {
        if(m_bounding_radius >= 0.0f)
        {
            return m_bounding_radius;
        }

        float furthest = 0.0f;

        // Squared while looking, rooted once at the end: the comparison does
        // not need the root and a mesh has a great many vertices.
        for(const auto& one : m_vertices)
        {
            const float away = glm::dot(one.position, one.position);

            if(away > furthest)
            {
                furthest = away;
            }
        }

        m_bounding_radius = std::sqrt(furthest);

        return m_bounding_radius;
    }

    bool mesh_3d::update_vertices(size_t first, const struct vertex* data, size_t count)
    {
        if(data == nullptr || count == 0 || first + count > m_vertices.size())
        {
            return false;
        }

        std::copy(data, data + count, m_vertices.begin() + static_cast<long>(first));

        // The shape has changed, so what was worked out about its size no
        // longer holds. Sculpting terrain is exactly this, and a radius kept
        // from before the hill was raised would cull the hill.
        m_bounding_radius = -1.0f;

        if(m_vbo == 0)
        {
            return false;
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>(first * sizeof(struct vertex)),
                        static_cast<GLsizeiptr>(count * sizeof(struct vertex)),
                        data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return true;
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
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
        // glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4,4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, joints));

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5,4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, weights));

        glBindVertexArray(0);
    }

} // namespace nle
