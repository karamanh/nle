#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vector>

#include "Shader.h"
#include "RenderObject3D.h"
#include "Texture.h"
#include "Material.h"

namespace nle
{
    class MeshInstance;

    class Mesh
    {
    private:
        unsigned int m_ebo,
            m_vbo,
            m_vao;

        unsigned int m_primitive_type = GL_TRIANGLES;

        std::vector<float> m_vertices;
        std::vector<unsigned int> m_indices;

        Shader *m_shader = nullptr;
        Texture *m_texture = nullptr;
        Material *m_material = nullptr;

        void load();

    public:
        Mesh(std::vector<float> vertices, std::vector<unsigned int> indices, Shader *shader, Texture *texture = nullptr);
        virtual ~Mesh();
        std::vector<float> *vertices();
        std::vector<unsigned int> *indices();

        void set_texture(Texture *texture);
        Texture *texture();
        Shader *shader();
        void set_material(Material *material);
        Material *material();
        MeshInstance *create_instance();

        friend class Renderer3D;
    };

    class MeshInstance : public RenderObject3D
    {
    private:
        Mesh *m_mesh;

        // RenderLayer m_render_layer;
        // int m_render_mode;
        // bool m_visible;

    public:
        MeshInstance(Mesh *mesh);
        Mesh *mesh();

        friend class Renderer3D;
    };

} // namespace nle

#endif