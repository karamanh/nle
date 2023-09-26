#include "Sky.h"
#include "Globals.h"
#include "resources/default_sky.h"
#include "resources/default_vert.h"
#include "resources/default_frag.h"

// vertices.push_back(v.Position.X);
// vertices.push_back(v.Position.Y);
// vertices.push_back(v.Position.Z);
// vertices.push_back(v.Normal.X);
// vertices.push_back(v.Normal.Y);
// vertices.push_back(v.Normal.Z);
// vertices.push_back(mesh.MeshMaterial.Kd.X);
// vertices.push_back(mesh.MeshMaterial.Kd.Y);
// vertices.push_back(mesh.MeshMaterial.Kd.Z);
// vertices.push_back(v.TextureCoordinate.X);
// vertices.push_back(v.TextureCoordinate.Y);
namespace nle
{
    Sky::Sky()
        : MeshInstance(m_mesh = new Mesh(
                           {
                               // Front face
                               -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.3333f, // Vertex 0 (x, y, z, u, v)
                               0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.3333f, // Vertex 1
                               0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.6666f,  // Vertex 2
                               -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.6666f,  // Vertex 3

                               // Back face
                               -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3333f, // Vertex 4
                               0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.3333f, // Vertex 5
                               0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.6666f,  // Vertex 6
                               -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.6666f,  // Vertex 7

                               // Left face
                               -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.3333f, // Vertex 8
                               -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.6666f,  // Vertex 9
                               -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.6666f,    // Vertex 10
                               -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.3333f,   // Vertex 11

                               // Right face
                               0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.3333f, // Vertex 12
                               0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.6666f,  // Vertex 13
                               0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.6666f,  // Vertex 14
                               0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.3333f, // Vertex 15

                               // Top face
                               -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 1.0f,   // Vertex 16
                               0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f,     // Vertex 17
                               0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.6666f,   // Vertex 18
                               -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.6666f, // Vertex 19

                               // Bottom face
                               -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.0f,   // Vertex 20
                               0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,     // Vertex 21
                               0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.3333f,   // Vertex 22
                               -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.3333f, // Vertex 23
                           },
                           {
                               0, 1, 2, 2, 3, 0,       // Front face
                               4, 5, 6, 6, 7, 4,       // Back face
                               8, 9, 10, 10, 11, 8,    // Left face
                               12, 13, 14, 14, 15, 12, // Right face
                               16, 17, 18, 18, 19, 16, // Top face
                               20, 21, 22, 22, 23, 20, // Bottom face
                           },
                           new Shader(std::string((const char *)default_vert_glsl, default_vert_glsl_len), std::string((const char *)default_frag_glsl, default_frag_glsl_len)),
                           new Texture(default_sky_png, default_sky_png_len),
                           new Material()))
    {
        mesh()->material()->set_accept_light(false);
    }

    Sky::Sky(Mesh *m)
        : m_mesh(m), MeshInstance(m)
    {
    }

    Sky::~Sky()
    {
        delete m_mesh->shader();
        delete m_mesh->texture();
        delete m_mesh->material();
        delete m_mesh;
    }

    void Sky::set_distance_fog_enabled(bool enabled)
    {
        m_distance_fog_enabled = enabled;
    }

    bool Sky::distance_fog_enabled() const
    {
        return m_distance_fog_enabled;
    }

    void Sky::set_distance_fog_max(float max)
    {
        m_distance_fog_max = max;
    }

    float Sky::distance_fog_max() const
    {
        return m_distance_fog_max;
    }

    void Sky::set_distance_fog_min(float min)
    {
        m_distance_fog_min = min;
    }

    float Sky::distance_fog_min() const
    {
        return m_distance_fog_min;
    }
} // namespace nle
