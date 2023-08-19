#include "Mesh.h"
#include <stdexcept>
#include <limits>

namespace nle
{
    Mesh::Mesh(std::vector<float> vertices, std::vector<unsigned int> indices, Shader *shader, Texture *texture)
        : m_vertices(vertices), m_indices(indices), m_shader(shader), m_texture(texture)
    {
        // compute aabb here.
        unsigned int block_size = 11U; // 3 vertices, 3 normals, 2 texture coords
        unsigned int range = vertices.size() / block_size;

        m_aabb_max = glm::vec3(std::numeric_limits<float>::min());
        m_aabb_min = glm::vec3(std::numeric_limits<float>::max());

        for (unsigned int i = 0; i < range; i++)
        {
            float &vx = m_vertices[block_size * i];
            float &vy = m_vertices[block_size * i + 1];
            float &vz = m_vertices[block_size * i + 2];

            if (vx < m_aabb_min.x)
            {
                m_aabb_min.x = vx;
            }
            else if (vx > m_aabb_max.x)
            {
                m_aabb_max.x = vx;
            }
            if (vy < m_aabb_min.y)
            {
                m_aabb_min.y = vy;
            }
            else if (vy > m_aabb_max.y)
            {
                m_aabb_max.y = vy;
            }
            if (vz < m_aabb_min.z)
            {
                m_aabb_min.z = vz;
            }
            else if (vz > m_aabb_max.z)
            {
                m_aabb_max.z = vz;
            }
        }

        load();
    }

    Mesh::~Mesh()
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

    void Mesh::load()
    {
        // vertex array object
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        // vertex buffer object
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(m_vertices[0]), m_vertices.data(), GL_STATIC_DRAW);

        // element buffer offset (index buffer object)
        glGenBuffers(1, &m_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(m_indices[0]), m_indices.data(), GL_STATIC_DRAW);

        unsigned int cluster_size = 11;

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(m_vertices[0]) * cluster_size, (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(m_vertices[0]) * cluster_size, (void *)(sizeof(m_vertices[0]) * 3));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(m_vertices[0]) * cluster_size, (void *)(sizeof(m_vertices[0]) * 6));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(m_vertices[0]) * cluster_size, (void *)(sizeof(m_vertices[0]) * 9));
        glEnableVertexAttribArray(3);

        glBindVertexArray(0);
    }

    std::vector<float> *Mesh::vertices()
    {
        return &m_vertices;
    }

    std::vector<unsigned int> *Mesh::indices()
    {
        return &m_indices;
    }

    void Mesh::set_texture(Texture *texture)
    {
        m_texture = texture;
    }

    Texture *Mesh::texture()
    {
        return m_texture;
    }

    Shader *Mesh::shader()
    {
        return m_shader;
    }

    void Mesh::set_material(Material *material)
    {
        m_material = material;
    }

    Material *Mesh::material()
    {
        return m_material;
    }

    MeshInstance *Mesh::create_instance()
    {
        MeshInstance *mi = new MeshInstance(this);
        return mi;
    }

    glm::vec3 Mesh::aabb_min() const
    {
        return m_aabb_min;
    }

    glm::vec3 Mesh::aabb_max() const
    {
        return m_aabb_max;
    }

    MeshInstance::MeshInstance(Mesh *mesh)
        : m_mesh(mesh)
    {
    }

    Mesh *MeshInstance::mesh()
    {
        return m_mesh;
    }

} // namespace nle
