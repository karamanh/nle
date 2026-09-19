#include "terrain_3d.h"

#include "../mesh/mesh_3d.h"

#include <algorithm>
#include <cmath>

namespace nle
{

namespace
{
    /// Step used when walking a ray across uneven ground, and the number of
    /// bisection passes used to tighten the hit afterwards.
    constexpr float MARCH_STEP = 0.35f;
    constexpr float MARCH_LIMIT = 2000.0f;
    constexpr int BISECTION_STEPS = 20;
}

terrain_3d::terrain_3d(float size, int resolution)
    : mesh_instance_3d(nullptr),
      m_size(std::max(size, 1.0f)),
      m_resolution(std::max(resolution, 1))
{
    rebuild();
}

terrain_3d::~terrain_3d()
{
}

void terrain_3d::set_height_function(std::function<float(float, float)> height)
{
    m_height = std::move(height);
    rebuild();
}

void terrain_3d::set_colors(const glm::vec3& first, const glm::vec3& second)
{
    m_first_color = first;
    m_second_color = second;
    rebuild();
}

float terrain_3d::size() const
{
    return m_size;
}

int terrain_3d::resolution() const
{
    return m_resolution;
}

float terrain_3d::local_height(float x, float z) const
{
    return m_height ? m_height(x, z) : 0.0f;
}

float terrain_3d::height_at(float x, float z) const
{
    const glm::vec3 origin = position();
    return origin.y + local_height(x - origin.x, z - origin.z);
}

bool terrain_3d::contains(float x, float z) const
{
    const glm::vec3 origin = position();
    const float half = m_size * 0.5f;

    return std::abs(x - origin.x) <= half && std::abs(z - origin.z) <= half;
}

glm::vec3 terrain_3d::place_on_surface(const glm::vec3& p) const
{
    return { p.x, height_at(p.x, p.z), p.z };
}

ref<class mesh_3d> terrain_3d::build_mesh() const
{
    std::vector<vertex> vertices;
    std::vector<uint32_t> indices;

    const auto tiles = static_cast<size_t>(m_resolution);
    vertices.reserve(tiles * tiles * 4);
    indices.reserve(tiles * tiles * 6);

    const float half = m_size * 0.5f;
    const float step = m_size / static_cast<float>(m_resolution);

    // Central differences over a fraction of a tile: close enough to the real
    // gradient for shading, and it needs nothing but the height function.
    const float epsilon = step * 0.25f;

    auto normal_at = [&](float x, float z) {
        if(!m_height)
        {
            return glm::vec3(0.0f, 1.0f, 0.0f);
        }

        const float dx = local_height(x + epsilon, z) - local_height(x - epsilon, z);
        const float dz = local_height(x, z + epsilon) - local_height(x, z - epsilon);

        return glm::normalize(glm::vec3(-dx, 2.0f * epsilon, -dz));
    };

    for(int z = 0; z < m_resolution; ++z)
    {
        for(int x = 0; x < m_resolution; ++x)
        {
            const float x0 = static_cast<float>(x) * step - half;
            const float z0 = static_cast<float>(z) * step - half;
            const float x1 = x0 + step;
            const float z1 = z0 + step;

            const glm::vec3 color = ((x + z) % 2 == 0) ? m_first_color : m_second_color;
            const auto base = static_cast<uint32_t>(vertices.size());

            const float u = static_cast<float>(x);
            const float v = static_cast<float>(z);

            // counter-clockwise seen from above, so the face normal is +Y
            vertices.push_back({ { x0, local_height(x0, z1), z1 }, normal_at(x0, z1), color, { u, v + 1.0f } });
            vertices.push_back({ { x1, local_height(x1, z1), z1 }, normal_at(x1, z1), color, { u + 1.0f, v + 1.0f } });
            vertices.push_back({ { x1, local_height(x1, z0), z0 }, normal_at(x1, z0), color, { u + 1.0f, v } });
            vertices.push_back({ { x0, local_height(x0, z0), z0 }, normal_at(x0, z0), color, { u, v } });

            indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
        }
    }

    auto mesh = make_ref<mesh_3d>(vertices, indices, nullptr);

    // Ground is diffuse; a strong specular term on a whole field of it reads
    // as wet plastic.
    auto surface = make_ref<material>();
    surface->set_ambient(glm::vec3(0.30f));
    surface->set_diffuse(glm::vec3(0.85f));
    surface->set_specular(glm::vec3(0.04f));
    surface->set_shininess(8.0f);
    surface->set_dissolve(1.0f);
    mesh->set_material(surface);

    return mesh;
}

void terrain_3d::rebuild()
{
    m_mesh = build_mesh();
}

bool terrain_3d::raycast(const ray& r, glm::vec3& hit) const
{
    const glm::vec3 origin = position();

    if(!m_height)
    {
        // Flat ground: solve it directly rather than marching towards it.
        float distance = 0.0f;
        if(!intersect_horizontal_plane(r, origin.y, distance))
        {
            return false;
        }

        const glm::vec3 point = r.at(distance);
        if(!contains(point.x, point.z))
        {
            return false;
        }

        hit = point;
        return true;
    }

    // Uneven ground: walk forward until the ray is under the surface, then
    // bisect the last step. Only a ray coming down can enter the ground.
    float previous = 0.0f;
    bool previous_above = r.origin.y >= height_at(r.origin.x, r.origin.z);

    for(float distance = MARCH_STEP; distance < MARCH_LIMIT; distance += MARCH_STEP)
    {
        const glm::vec3 point = r.at(distance);
        const bool above = point.y >= height_at(point.x, point.z);

        if(previous_above && !above)
        {
            float low = previous;
            float high = distance;

            for(int step = 0; step < BISECTION_STEPS; ++step)
            {
                const float middle = (low + high) * 0.5f;
                const glm::vec3 sample = r.at(middle);

                if(sample.y >= height_at(sample.x, sample.z))
                {
                    low = middle;
                }
                else
                {
                    high = middle;
                }
            }

            const glm::vec3 result = r.at((low + high) * 0.5f);
            if(!contains(result.x, result.z))
            {
                return false;
            }

            hit = result;
            return true;
        }

        previous = distance;
        previous_above = above;
    }

    return false;
}

} // namespace nle
