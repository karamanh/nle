#include "Sky.h"
#include "Globals.h"
#include "resources/default_sky.h"
#include "resources/default_vert.h"
#include "resources/default_frag.h"

namespace nle
{
    Sky::Sky()
        : MeshInstance(m_mesh = new Mesh(
            {
                // front face vertices
                -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.3333333f,
                -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.6666666f,
                0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.6666666f,
                0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.3333333f,
                // back face vertices 
                -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.3333333f,
                -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.6666666f,
                0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.6666666f,
                0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.3333333f,
                // left face vertices
                -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3333333f,
                -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.6666666f,
                -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.6666666f,
                -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.3333333f,
                // right face vertices
                0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.3333333f,
                0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.6666666f,
                0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.6666666f,
                0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.3333333f,
                // bottom face vertices
                -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.0f,
                -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.3333333f,
                0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.3333333f,
                0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
                // top face vertices
                -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 1.0f,
                -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.6666666f,
                0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.6666666f,
                0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f
            },
            {
                0, 1, 2, 2, 3, 0,
                4, 5, 6, 6, 7, 4,
                8, 9, 10, 10, 11, 8,
                12, 13, 14, 14, 15, 12,
                16, 17, 18, 18, 19, 16,
                20, 21, 22, 22, 23, 20
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
        // shader, texture and materials are not deleted by mesh class instance.
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
