#include "terrain_3d.h"

#include "../mesh/mesh_3d.h"
#include "../model/model.hpp"

#include <FastNoiseLite.h>

#include <algorithm>
#include <cmath>
#include <random>

namespace nle
{

namespace
{
    /// Step used when walking a ray across uneven ground, and the number of
    /// bisection passes used to tighten the hit afterwards.
    constexpr float MARCH_STEP = 0.35f;
    constexpr float MARCH_LIMIT = 2000.0f;
    constexpr int BISECTION_STEPS = 20;

    /// Hermite fade, so colour bands and the flat centre blend without a seam.
    float smooth01(float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    /**
     * @brief Builds the height function for a set of noise settings.
     *
     * The generator is captured by shared_ptr rather than by value: height_at()
     * runs per frame for everything standing on the ground, so the generator is
     * built once here and not once per sample.
     */
    std::function<float(float, float)> make_noise_height(const terrain_noise& settings, float size)
    {
        auto generator = std::make_shared<FastNoiseLite>();

        generator->SetSeed(settings.seed);
        generator->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        generator->SetFractalType(FastNoiseLite::FractalType_FBm);
        generator->SetFractalOctaves(std::max(settings.octaves, 1));
        generator->SetFractalLacunarity(settings.lacunarity);
        generator->SetFractalGain(settings.gain);
        generator->SetFrequency(settings.frequency);

        const float half = size * 0.5f;

        return [generator, settings, half](float x, float z) {
            float height = generator->GetNoise(x, z) * settings.amplitude;

            // A level circle at the centre, faded out over flat_falloff so the
            // hills do not start with a step.
            if(settings.flat_radius > 0.0f)
            {
                const float distance = std::sqrt(x * x + z * z);
                const float falloff = std::max(settings.flat_falloff, 0.001f);

                height *= smooth01((distance - settings.flat_radius) / falloff);
            }

            // Pull the outside edge back down to the base, so the patch ends in
            // a rim rather than in a wall of cliff.
            if(settings.edge_falloff > 0.0f)
            {
                const float edge = half - std::max(std::abs(x), std::abs(z));
                height *= smooth01(edge / settings.edge_falloff);
            }

            return height;
        };
    }
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
    m_generated = false;
    rebuild();
}

void terrain_3d::set_noise(const terrain_noise& noise)
{
    m_noise = noise;
    m_height = make_noise_height(noise, m_size);
    m_generated = true;
    rebuild();
}

const terrain_noise& terrain_3d::noise() const
{
    return m_noise;
}

bool terrain_3d::generated() const
{
    return m_generated;
}

void terrain_3d::set_colors(const glm::vec3& first, const glm::vec3& second)
{
    m_first_color = first;
    m_second_color = second;
    rebuild();
}

void terrain_3d::set_layers(std::vector<terrain_layer> layers)
{
    std::sort(layers.begin(), layers.end(),
              [](const terrain_layer& a, const terrain_layer& b) { return a.height < b.height; });

    m_layers = std::move(layers);
    rebuild();
}

const std::vector<terrain_layer>& terrain_3d::layers() const
{
    return m_layers;
}

void terrain_3d::set_cliff(const glm::vec3& color, float angle, float blend)
{
    m_has_cliff = true;
    m_cliff_color = color;
    m_cliff_angle = angle;
    m_cliff_blend = std::max(blend, 0.001f);
    rebuild();
}

void terrain_3d::clear_cliff()
{
    m_has_cliff = false;
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

glm::vec3 terrain_3d::local_normal(float x, float z) const
{
    if(!m_height)
    {
        return glm::vec3(0.0f, 1.0f, 0.0f);
    }

    // Central differences over a fraction of a tile: close enough to the real
    // gradient for shading, and it needs nothing but the height function.
    const float epsilon = (m_size / static_cast<float>(m_resolution)) * 0.25f;

    const float dx = local_height(x + epsilon, z) - local_height(x - epsilon, z);
    const float dz = local_height(x, z + epsilon) - local_height(x, z - epsilon);

    return glm::normalize(glm::vec3(-dx, 2.0f * epsilon, -dz));
}

float terrain_3d::height_at(float x, float z) const
{
    const glm::vec3 origin = position();
    return origin.y + local_height(x - origin.x, z - origin.z);
}

glm::vec3 terrain_3d::normal_at(float x, float z) const
{
    const glm::vec3 origin = position();
    return local_normal(x - origin.x, z - origin.z);
}

float terrain_3d::slope_at(float x, float z) const
{
    const glm::vec3 normal = normal_at(x, z);
    return glm::degrees(std::acos(std::clamp(normal.y, -1.0f, 1.0f)));
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

glm::vec3 terrain_3d::surface_color(int tile_x, int tile_z, float local_y, const glm::vec3& normal) const
{
    glm::vec3 color;

    if(m_layers.empty())
    {
        color = ((tile_x + tile_z) % 2 == 0) ? m_first_color : m_second_color;
    }
    else if(local_y <= m_layers.front().height)
    {
        color = m_layers.front().color;
    }
    else if(local_y >= m_layers.back().height)
    {
        color = m_layers.back().color;
    }
    else
    {
        color = m_layers.back().color;

        for(size_t i = 1; i < m_layers.size(); ++i)
        {
            if(local_y > m_layers[i].height)
            {
                continue;
            }

            const terrain_layer& lower = m_layers[i - 1];
            const terrain_layer& upper = m_layers[i];
            const float span = std::max(upper.height - lower.height, 0.001f);

            color = glm::mix(lower.color, upper.color,
                             smooth01((local_y - lower.height) / span));
            break;
        }
    }

    if(m_has_cliff)
    {
        const float slope = glm::degrees(std::acos(std::clamp(normal.y, -1.0f, 1.0f)));
        const float lower = m_cliff_angle - m_cliff_blend;

        color = glm::mix(color, m_cliff_color,
                         smooth01((slope - lower) / (m_cliff_blend * 2.0f)));
    }

    return color;
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

    // Vertices are not shared between tiles, which is what lets the checker
    // pattern have hard edges. Normals still come out smooth: they are taken
    // from the height function rather than from the triangles, so the copies
    // of a shared corner all agree.
    auto push = [&](std::vector<vertex>& out, int tile_x, int tile_z,
                    float x, float z, float u, float v) {
        const float y = local_height(x, z);
        const glm::vec3 normal = local_normal(x, z);

        out.push_back({ { x, y, z }, normal, surface_color(tile_x, tile_z, y, normal), { u, v } });
    };

    for(int z = 0; z < m_resolution; ++z)
    {
        for(int x = 0; x < m_resolution; ++x)
        {
            const float x0 = static_cast<float>(x) * step - half;
            const float z0 = static_cast<float>(z) * step - half;
            const float x1 = x0 + step;
            const float z1 = z0 + step;

            const auto base = static_cast<uint32_t>(vertices.size());

            const float u = static_cast<float>(x);
            const float v = static_cast<float>(z);

            // counter-clockwise seen from above, so the face normal is +Y
            push(vertices, x, z, x0, z1, u, v + 1.0f);
            push(vertices, x, z, x1, z1, u + 1.0f, v + 1.0f);
            push(vertices, x, z, x1, z0, u + 1.0f, v);
            push(vertices, x, z, x0, z0, u, v);

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

void terrain_3d::set_scene(ref<render_object_3d> scene)
{
    render_object_3d::set_scene(scene);

    // Props scattered before there was a scene have been waiting for this.
    if(scene)
    {
        for(const auto& prop : m_decorations)
        {
            scene->add_child(prop);
        }
    }
}

void terrain_3d::attach(ref<render_object_3d> prop)
{
    m_decorations.push_back(prop);

    // No scene yet: set_scene() hands it over when the terrain joins one.
    if(auto host = scene())
    {
        host->add_child(prop);
    }
}

void terrain_3d::decorate(ref<render_object_3d> prop, const glm::vec3& position)
{
    if(!prop)
    {
        return;
    }

    prop->set_position(place_on_surface(position));
    attach(prop);
}

const std::vector<ref<render_object_3d>>& terrain_3d::decorations() const
{
    return m_decorations;
}

void terrain_3d::clear_decorations()
{
    if(auto host = scene())
    {
        for(const auto& prop : m_decorations)
        {
            host->delete_child(prop);
        }
    }

    m_decorations.clear();
}

std::vector<ref<render_object_3d>> terrain_3d::scatter(ref<model> prop, const terrain_scatter& settings)
{
    if(!prop)
    {
        return {};
    }

    return scatter([prop](int) -> ref<render_object_3d> { return prop->create_instance(); }, settings);
}

std::vector<ref<render_object_3d>> terrain_3d::scatter(
    const std::function<ref<render_object_3d>(int)>& factory, const terrain_scatter& settings)
{
    std::vector<ref<render_object_3d>> placed;

    if(!factory || settings.count <= 0)
    {
        return placed;
    }

    std::mt19937 random(static_cast<uint32_t>(settings.seed));

    const glm::vec3 origin = position();
    const float half = std::max(m_size * 0.5f - std::max(settings.margin, 0.0f), 0.0f);

    std::uniform_real_distribution<float> across(-half, half);
    std::uniform_real_distribution<float> yaw(0.0f, 360.0f);
    std::uniform_real_distribution<float> sizing(settings.scale_range.x, settings.scale_range.y);

    const int attempts = std::max(settings.attempts_per_prop, 1);

    placed.reserve(static_cast<size_t>(settings.count));

    for(int i = 0; i < settings.count; ++i)
    {
        for(int attempt = 0; attempt < attempts; ++attempt)
        {
            const float local_x = across(random);
            const float local_z = across(random);

            // Rejected placements are retried rather than clamped, so a tight
            // filter thins the scatter instead of packing props against it.
            if(settings.clear_radius > 0.0f &&
               std::sqrt(local_x * local_x + local_z * local_z) < settings.clear_radius)
            {
                continue;
            }

            const glm::vec3 normal = local_normal(local_x, local_z);
            const float slope = glm::degrees(std::acos(std::clamp(normal.y, -1.0f, 1.0f)));

            if(slope > settings.max_slope)
            {
                continue;
            }

            const float y = origin.y + local_height(local_x, local_z);

            if(y < settings.min_height || y > settings.max_height)
            {
                continue;
            }

            auto instance = factory(i);
            if(!instance)
            {
                break;
            }

            instance->set_position({ origin.x + local_x, y, origin.z + local_z });
            instance->set_scale(instance->scale() * sizing(random));

            glm::vec3 rotation(0.0f);

            if(settings.random_yaw)
            {
                rotation.y = yaw(random);
            }

            if(settings.align_to_normal)
            {
                rotation.x = glm::degrees(std::atan2(normal.z, normal.y));
                rotation.z = glm::degrees(-std::atan2(normal.x, normal.y));
            }

            instance->set_rotation(rotation);

            attach(instance);
            placed.push_back(instance);
            break;
        }
    }

    return placed;
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
