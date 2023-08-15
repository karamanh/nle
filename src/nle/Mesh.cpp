#include "Mesh.h"
#include <stdexcept>
#include <limits>

namespace nle
{
    Mesh::Mesh(std::vector<float> vertices, std::vector<unsigned int> indices, Shader *shader, Texture *texture)
        : m_vertices(vertices), m_indices(indices), m_shader(shader), m_texture(texture)
    {
        // if (1)
        // {
        //     // compute aabb here.
        //     unsigned int block_size = 8U; // 3 vertices, 3 normals, 2 texture coords
        //     unsigned int range = vertices.size() / block_size;

        //     glm::vec3 aabb_max = glm::vec3(std::numeric_limits<float>::min());
        //     glm::vec3 aabb_min = glm::vec3(std::numeric_limits<float>::max());

        //     for (unsigned int i = 0; i < range; i++)
        //     {
        //         float &vx = m_vertices[block_size * i];
        //         float &vy = m_vertices[block_size * i + 1];
        //         float &vz = m_vertices[block_size * i + 2];

        //         if (vx < aabb_min.x)
        //         {
        //             aabb_min.x = vx;
        //         }
        //         else if (vx > aabb_max.x)
        //         {
        //             aabb_max.x = vx;
        //         }
        //         if (vy < aabb_min.y)
        //         {
        //             aabb_min.y = vy;
        //         }
        //         else if (vy > aabb_max.y)
        //         {
        //             aabb_max.y = vy;
        //         }
        //         if (vz < aabb_min.z)
        //         {
        //             aabb_min.z = vz;
        //         }
        //         else if (vz > aabb_max.z)
        //         {
        //             aabb_max.z = vz;
        //         }
        //     }

        //     // 0 - 1 - 5 - 4 - 0 - 3 - 7 - 4 - 5 - 6 - 7 - 3 - 2 - 6 - 5 - 1 - 2 
        //     // -----------------   ---------   ---------  ----------   ---------
        //     //     front		      lef        btm         bck        right
        //     // m_aabb_Mesh = new Mesh(
        //     //     {
        //     //         m_aabb_min.x, m_aabb_max.y, m_aabb_max.z, 0.f, 0.f, 0.f, 0.f, 0.f, // top left front,
        //     //         m_aabb_max.x, m_aabb_max.y, m_aabb_max.z, 0.f, 0.f, 0.f, 0.f, 0.f, // top right front
        //     //         m_aabb_max.x, m_aabb_max.y, m_aabb_min.z, 0.f, 0.f, 0.f, 0.f, 0.f, // top right back
        //     //         m_aabb_min.x, m_aabb_max.y, m_aabb_min.z, 0.f, 0.f, 0.f, 0.f, 0.f, // top left back
        //     //         m_aabb_min.x, m_aabb_min.y, m_aabb_max.z, 0.f, 0.f, 0.f, 0.f, 0.f, // bottom left front
        //     //         m_aabb_max.x, m_aabb_min.y, m_aabb_max.z, 0.f, 0.f, 0.f, 0.f, 0.f, // bottom right front
        //     //         m_aabb_max.x, m_aabb_min.y, m_aabb_min.z, 0.f, 0.f, 0.f, 0.f, 0.f, // bottom right back
        //     //         m_aabb_min.x, m_aabb_min.y, m_aabb_min.z, 0.f, 0.f, 0.f, 0.f, 0.f  // bottom left back
        //     //     },
        //     //     {
        //     //         0, 1, 5, 4, 0, 3, 7, 4, 5, 6, 7, 3, 2, 6, 5, 1, 2
        //     //     },
        //     //     m_shader,
        //     //     nullptr,
        //     //     true);

        //     // m_aabb_Mesh->m_primitive_type = GL_LINE_STRIP;
        //     // prdbg("aabb max %f aabb min %f.", aabb_max.y, aabb_min.y);
        // }

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

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(m_vertices[0]) * 8, (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(m_vertices[0]) * 8, (void *)(sizeof(m_vertices[0]) * 3));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(m_vertices[0]) * 8, (void *)(sizeof(m_vertices[0]) * 6));
        glEnableVertexAttribArray(2);

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

    MeshInstance::MeshInstance(Mesh *mesh)
        : m_mesh(mesh) //, m_visible(true), m_render_layer(RenderLayer::_0)
    {
    }

    Mesh *MeshInstance::mesh()
    {
        return m_mesh;
    }

} // namespace nle
