#include "terrain_3d.h"

#include "../mesh/mesh_3d.h"
#include "../model/model.hpp"

#include <FastNoiseLite.h>

#include <algorithm>
#include <cmath>
#include <random>
#include <cstdint>

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

    // A new description of the surface replaces the sculpted one, which is
    // the only sensible reading of being handed a new one.
    m_heights.clear();

    rebuild();
}

void terrain_3d::set_noise(const terrain_noise& noise)
{
    m_noise = noise;
    m_height = make_noise_height(noise, m_size);
    m_generated = true;

    // Regenerating is starting again: whatever was sculpted described the old
    // ground and means nothing on the new.
    m_heights.clear();

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

int terrain_3d::samples() const
{
    return m_resolution + 1;
}

const std::vector<float>& terrain_3d::heightmap() const
{
    return m_heights;
}

bool terrain_3d::sculpted() const
{
    return !m_heights.empty();
}

bool terrain_3d::set_heightmap(std::vector<float> heights)
{
    const auto wanted = static_cast<size_t>(samples()) * static_cast<size_t>(samples());

    if(heights.size() != wanted)
    {
        return false;
    }

    m_heights = std::move(heights);
    rebuild();

    return true;
}

void terrain_3d::bake_heightmap()
{
    const int n = samples();
    const float half = m_size * 0.5f;
    const float step = m_size / static_cast<float>(m_resolution);

    std::vector<float> baked(static_cast<size_t>(n) * static_cast<size_t>(n), 0.0f);

    for(int z = 0; z < n; ++z)
    {
        for(int x = 0; x < n; ++x)
        {
            const float wx = static_cast<float>(x) * step - half;
            const float wz = static_cast<float>(z) * step - half;

            // Deliberately the function, not local_height: this is the moment
            // the function stops being the answer and the samples start.
            baked[static_cast<size_t>(z) * static_cast<size_t>(n) + static_cast<size_t>(x)] =
                m_height ? m_height(wx, wz) : 0.0f;
        }
    }

    m_heights = std::move(baked);
}

void terrain_3d::clear_heightmap()
{
    m_heights.clear();
    rebuild();
}

float terrain_3d::sample_heightmap(float x, float z) const
{
    const int n = samples();
    const auto expected = static_cast<size_t>(n) * static_cast<size_t>(n);

    if(m_heights.size() != expected)
    {
        return 0.0f;
    }

    // Not finite, and therefore not a position on this or any terrain.
    //
    // Worth refusing rather than trusting the clamp below: std::clamp passes
    // a NaN straight through, because every comparison against one is false,
    // and casting that to an int is undefined -- in practice INT_MIN, which
    // then indexes the heightfield somewhere off in memory. Callers really do
    // hand this NaNs: a screen ray built from a cursor that has left the
    // window is made of them.
    if(!std::isfinite(x) || !std::isfinite(z))
    {
        return 0.0f;
    }

    const float half = m_size * 0.5f;
    const float step = m_size / static_cast<float>(m_resolution);

    // Into grid coordinates, where a whole number lands on a sample.
    const float gx = std::clamp((x + half) / step, 0.0f, static_cast<float>(m_resolution));
    const float gz = std::clamp((z + half) / step, 0.0f, static_cast<float>(m_resolution));

    // Clamped at both ends. The upper end is the interpolation needing a
    // sample after this one; the lower end costs nothing and means no
    // arithmetic above can put this out of range whatever it is given.
    const int x0 = std::clamp(static_cast<int>(gx), 0, m_resolution - 1);
    const int z0 = std::clamp(static_cast<int>(gz), 0, m_resolution - 1);
    const int x1 = x0 + 1;
    const int z1 = z0 + 1;

    const float tx = std::clamp(gx - static_cast<float>(x0), 0.0f, 1.0f);
    const float tz = std::clamp(gz - static_cast<float>(z0), 0.0f, 1.0f);

    auto at = [&](int sx, int sz) {
        return m_heights[static_cast<size_t>(sz) * static_cast<size_t>(n)
                       + static_cast<size_t>(sx)];
    };

    // Bilinear, which is what makes the surface continuous between samples --
    // and therefore what makes height_at() agree with what is drawn.
    const float bottom = at(x0, z0) * (1.0f - tx) + at(x1, z0) * tx;
    const float top = at(x0, z1) * (1.0f - tx) + at(x1, z1) * tx;

    return bottom * (1.0f - tz) + top * tz;
}

bool terrain_3d::sculpt(const glm::vec3& center, float radius, float strength,
                        sculpt_mode mode, float delta_time, float level)
{
    if(radius <= 0.0f || delta_time <= 0.0f)
    {
        return false;
    }

    if(m_heights.empty())
    {
        bake_heightmap();
    }

    const int n = samples();
    const glm::vec3 origin = position();
    const float half = m_size * 0.5f;
    const float step = m_size / static_cast<float>(m_resolution);

    // Only the samples the brush actually covers, so the cost of a stroke is
    // the size of the brush rather than the size of the map.
    const float cx = center.x - origin.x;
    const float cz = center.z - origin.z;

    const int min_x = std::max(0, static_cast<int>(std::floor((cx - radius + half) / step)));
    const int max_x = std::min(n - 1, static_cast<int>(std::ceil((cx + radius + half) / step)));
    const int min_z = std::max(0, static_cast<int>(std::floor((cz - radius + half) / step)));
    const int max_z = std::min(n - 1, static_cast<int>(std::ceil((cz + radius + half) / step)));

    if(min_x > max_x || min_z > max_z)
    {
        return false;
    }

    auto at = [&](int sx, int sz) -> float& {
        return m_heights[static_cast<size_t>(sz) * static_cast<size_t>(n)
                       + static_cast<size_t>(sx)];
    };

    // Smoothing reads its neighbours, so it cannot read a half-smoothed grid.
    std::vector<float> before;
    if(mode == sculpt_mode::smooth)
    {
        before = m_heights;
    }

    auto sampled = [&](const std::vector<float>& from, int sx, int sz) {
        const int cx_ = std::clamp(sx, 0, n - 1);
        const int cz_ = std::clamp(sz, 0, n - 1);
        return from[static_cast<size_t>(cz_) * static_cast<size_t>(n)
                  + static_cast<size_t>(cx_)];
    };

    bool moved = false;

    for(int sz = min_z; sz <= max_z; ++sz)
    {
        for(int sx = min_x; sx <= max_x; ++sx)
        {
            const float wx = static_cast<float>(sx) * step - half;
            const float wz = static_cast<float>(sz) * step - half;

            const float distance = std::sqrt((wx - cx) * (wx - cx) + (wz - cz) * (wz - cz));

            if(distance >= radius)
            {
                continue;
            }

            // Smooth falloff to the rim, so overlapping strokes build up into
            // a hill rather than stacking visible discs.
            const float t = 1.0f - distance / radius;
            const float weight = t * t * (3.0f - 2.0f * t);

            float& height = at(sx, sz);
            const float was = height;

            switch(mode)
            {
                case sculpt_mode::raise:
                    height += strength * weight * delta_time;
                    break;

                case sculpt_mode::lower:
                    height -= strength * weight * delta_time;
                    break;

                case sculpt_mode::smooth:
                {
                    const float average =
                        (sampled(before, sx - 1, sz) + sampled(before, sx + 1, sz)
                       + sampled(before, sx, sz - 1) + sampled(before, sx, sz + 1)
                       + sampled(before, sx, sz)) / 5.0f;

                    const float rate = std::clamp(strength * weight * delta_time, 0.0f, 1.0f);
                    height += (average - height) * rate;
                    break;
                }

                case sculpt_mode::flatten:
                {
                    const float rate = std::clamp(strength * weight * delta_time, 0.0f, 1.0f);
                    height += (level - height) * rate;
                    break;
                }
            }

            if(height != was)
            {
                moved = true;
            }
        }
    }

    if(moved)
    {
        refresh_region(min_x, max_x, min_z, max_z);
    }

    return moved;
}

void terrain_3d::set_paint_layers(std::vector<terrain_paint_layer> layers)
{
    const size_t was = m_paint_layers.size();
    const size_t now = layers.size();

    m_paint_layers = std::move(layers);

    if(!m_paint.empty() && was != now)
    {
        // Keep the weights that still have a layer to belong to. Renaming or
        // recolouring a layer must not wipe what has been painted with it,
        // and dropping the last one should not wipe the others.
        const auto count = static_cast<size_t>(samples()) * static_cast<size_t>(samples());

        std::vector<uint8_t> moved(count * now, 0);

        for(size_t sample = 0; sample < count; ++sample)
        {
            for(size_t layer = 0; layer < std::min(was, now); ++layer)
            {
                moved[sample * now + layer] = m_paint[sample * was + layer];
            }
        }

        m_paint = std::move(moved);
    }
    else if(m_paint.empty())
    {
        m_paint.clear();
    }

    rebuild();
}

void terrain_3d::set_base_texture(ref<class texture> picture, float tiling)
{
    m_base_texture = std::move(picture);
    m_base_tiling = std::max(0.01f, tiling);

    // The ground is drawn by the material now, so the mesh has to carry it.
    rebuild();
}

ref<class texture> terrain_3d::base_texture() const
{
    return m_base_texture;
}

void terrain_3d::set_layer_texture(size_t layer, ref<class texture> picture)
{
    if(layer >= MOST_TEXTURED_LAYERS)
    {
        return;
    }

    m_layer_textures[layer] = std::move(picture);
    m_splat_stale = true;

    rebuild();
}

ref<class texture> terrain_3d::splatmap()
{
    if(m_splat_stale)
    {
        refresh_splatmap();
    }

    return m_splatmap;
}

void terrain_3d::render(render_command_buffer& command_buffer, const render_context& context)
{
    // Built here rather than on every brush stroke: painting is a drag of the
    // mouse and touches the same ground many times a second, and only the last
    // of those is ever seen.
    splatmap();

    mesh_instance_3d::render(command_buffer, context);
}

glm::vec3 terrain_3d::lighting_reference(const render_context& context) const
{
    // The ground is one mesh the size of the map, so its middle says nothing
    // about which lamps reach the corner anybody is standing in. The part of
    // it worth lighting is the part under the camera.
    return context.eye_position;
}

void terrain_3d::record_extra_uniforms(render_command_buffer& command_buffer,
                                       const render_context&)
{
    // Here rather than around render(), because the shared surface uniforms
    // are recorded inside it and one of them is this one: every other surface
    // says it is not ground, and a value set before them would be overwritten
    // by the surface that comes after it. This runs between them and the draw.
    command_buffer.set_uniform("u_terrain_extent", static_cast<float>(m_resolution));

    // Nought is the mesh's own texture and the shadow map sits at seven, so
    // the ground's pictures start at two, with room between.
    constexpr unsigned int BASE_UNIT = 2;
    constexpr unsigned int SPLAT_UNIT = 3;
    constexpr unsigned int FIRST_LAYER_UNIT = 4;

    if(m_base_texture)
    {
        command_buffer.use_texture(m_base_texture, BASE_UNIT);
        command_buffer.set_uniform("u_terrain_base", static_cast<int>(BASE_UNIT));
        command_buffer.set_uniform("u_terrain_base_enabled", 1);
        command_buffer.set_uniform("u_terrain_base_tiling", m_base_tiling);
    }
    else
    {
        command_buffer.set_uniform("u_terrain_base_enabled", 0);
    }

    int which = 0;
    glm::vec4 tiling(24.0f);

    for(size_t layer = 0; layer < MOST_TEXTURED_LAYERS; ++layer)
    {
        if(!m_layer_textures[layer])
        {
            continue;
        }

        const unsigned int unit = FIRST_LAYER_UNIT + static_cast<unsigned int>(layer);

        command_buffer.use_texture(m_layer_textures[layer], unit);
        command_buffer.set_uniform("u_terrain_layer_" + std::to_string(layer),
                                   static_cast<int>(unit));

        which |= 1 << layer;

        if(layer < m_paint_layers.size())
        {
            tiling[static_cast<int>(layer)] = std::max(0.01f, m_paint_layers[layer].tiling);
        }
    }

    command_buffer.set_uniform("u_terrain_layers_enabled", which);
    command_buffer.set_uniform("u_terrain_layer_tiling", tiling);

    if(m_splatmap && which != 0)
    {
        command_buffer.use_texture(m_splatmap, SPLAT_UNIT);
        command_buffer.set_uniform("u_terrain_splat", static_cast<int>(SPLAT_UNIT));
        command_buffer.set_uniform("u_terrain_splat_enabled", 1);
    }
    else
    {
        command_buffer.set_uniform("u_terrain_splat_enabled", 0);
    }
}

void terrain_3d::refresh_splatmap()
{
    // Only worth building when something textured is going to read it.
    bool wanted = false;

    for(const auto& one : m_layer_textures)
    {
        if(one)
        {
            wanted = true;
            break;
        }
    }

    if(!wanted || m_paint.empty() || m_paint_layers.empty())
    {
        m_splatmap = nullptr;
        m_splat_stale = false;
        return;
    }

    const int side = m_resolution + 1;
    const size_t layers = m_paint_layers.size();

    // One texel per sample, four channels, one layer to a channel. The same
    // numbers the paint brush already writes -- this is only them arranged
    // the way a sampler wants to read them.
    std::vector<uint8_t> pixels(static_cast<size_t>(side) * side * 4, 0);

    for(int z = 0; z < side; ++z)
    {
        for(int x = 0; x < side; ++x)
        {
            const size_t sample = static_cast<size_t>(z) * side + x;
            const size_t from = sample * layers;
            const size_t to = sample * 4;

            for(size_t layer = 0; layer < MOST_TEXTURED_LAYERS && layer < layers; ++layer)
            {
                pixels[to + layer] = m_paint[from + layer];
            }
        }
    }

    m_splatmap = make_ref<class texture>(pixels.data(), side, side, 4, false);

    m_splat_stale = false;
}

const std::vector<terrain_paint_layer>& terrain_3d::paint_layers() const
{
    return m_paint_layers;
}

const std::vector<uint8_t>& terrain_3d::paintmap() const
{
    return m_paint;
}

bool terrain_3d::painted() const
{
    return !m_paint.empty();
}

size_t terrain_3d::paint_index(int x, int z) const
{
    return (static_cast<size_t>(z) * static_cast<size_t>(samples())
          + static_cast<size_t>(x)) * m_paint_layers.size();
}

bool terrain_3d::set_paintmap(std::vector<uint8_t> weights)
{
    const auto wanted = static_cast<size_t>(samples()) * static_cast<size_t>(samples())
                      * m_paint_layers.size();

    if(m_paint_layers.empty() || weights.size() != wanted)
    {
        return false;
    }

    m_paint = std::move(weights);
    rebuild();

    return true;
}

int terrain_3d::paint_at(float x, float z) const
{
    if(m_paint.empty() || m_paint_layers.empty())
    {
        return -1;
    }

    const glm::vec3 origin = position();
    const float half = m_size * 0.5f;
    const float step = m_size / static_cast<float>(m_resolution);

    if(!std::isfinite(x) || !std::isfinite(z))
    {
        return -1;
    }

    // std::lround of a NaN is as undefined as a cast is, so the check above
    // has to come first here too.
    const int sx = std::clamp(static_cast<int>(std::lround((x - origin.x + half) / step)),
                              0, m_resolution);
    const int sz = std::clamp(static_cast<int>(std::lround((z - origin.z + half) / step)),
                              0, m_resolution);

    const size_t base = paint_index(sx, sz);

    int strongest = -1;
    uint8_t best = 0;

    for(size_t layer = 0; layer < m_paint_layers.size(); ++layer)
    {
        if(m_paint[base + layer] > best)
        {
            best = m_paint[base + layer];
            strongest = static_cast<int>(layer);
        }
    }

    return strongest;
}

bool terrain_3d::paint(const glm::vec3& center, float radius, int layer, float strength,
                       float delta_time, bool erase)
{
    if(radius <= 0.0f || delta_time <= 0.0f || m_paint_layers.empty())
    {
        return false;
    }

    if(layer < 0 || static_cast<size_t>(layer) >= m_paint_layers.size())
    {
        return false;
    }

    const int n = samples();
    const size_t layers = m_paint_layers.size();

    if(m_paint.empty())
    {
        m_paint.assign(static_cast<size_t>(n) * static_cast<size_t>(n) * layers, 0);
    }

    const glm::vec3 origin = position();
    const float half = m_size * 0.5f;
    const float step = m_size / static_cast<float>(m_resolution);

    const float cx = center.x - origin.x;
    const float cz = center.z - origin.z;

    // Only the samples the brush covers, so a stroke costs the size of the
    // brush rather than the size of the map.
    const int min_x = std::max(0, static_cast<int>(std::floor((cx - radius + half) / step)));
    const int max_x = std::min(n - 1, static_cast<int>(std::ceil((cx + radius + half) / step)));
    const int min_z = std::max(0, static_cast<int>(std::floor((cz - radius + half) / step)));
    const int max_z = std::min(n - 1, static_cast<int>(std::ceil((cz + radius + half) / step)));

    if(min_x > max_x || min_z > max_z)
    {
        return false;
    }

    bool changed = false;

    for(int sz = min_z; sz <= max_z; ++sz)
    {
        for(int sx = min_x; sx <= max_x; ++sx)
        {
            const float wx = static_cast<float>(sx) * step - half;
            const float wz = static_cast<float>(sz) * step - half;

            const float distance = std::sqrt((wx - cx) * (wx - cx) + (wz - cz) * (wz - cz));

            if(distance >= radius)
            {
                continue;
            }

            // Same falloff as sculpting, so a painted edge and a sculpted one
            // look like they were made by the same hand.
            const float t = 1.0f - distance / radius;
            const float weight = t * t * (3.0f - 2.0f * t);

            const float amount = strength * weight * delta_time * 255.0f;

            uint8_t& value = m_paint[paint_index(sx, sz) + static_cast<size_t>(layer)];
            const uint8_t was = value;

            const float target = erase ? static_cast<float>(value) - amount
                                       : static_cast<float>(value) + amount;

            value = static_cast<uint8_t>(std::clamp(target, 0.0f, 255.0f));

            if(value != was)
            {
                changed = true;
            }
        }
    }

    if(changed)
    {
        refresh_region(min_x, max_x, min_z, max_z);
    }

    return changed;
}

float terrain_3d::local_height(float x, float z) const
{
    // Sculpted ground answers from its samples; everything else from whatever
    // function generated it.
    if(!m_heights.empty())
    {
        return sample_heightmap(x, z);
    }

    return m_height ? m_height(x, z) : 0.0f;
}

bool terrain_3d::has_surface() const
{
    return static_cast<bool>(m_height) || !m_heights.empty();
}

glm::vec3 terrain_3d::local_normal(float x, float z) const
{
    if(!has_surface())
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

glm::vec3 terrain_3d::ground_tone(int tile_x, int tile_z, float local_y,
                                  const glm::vec3& normal) const
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

glm::vec3 terrain_3d::painted_tone(int tile_x, int tile_z, const glm::vec3& over,
                                   bool skip_textured) const
{
    glm::vec3 color = over;

    // Paint goes over everything else, in order, so a road laid after grass
    // covers it. tile_x and tile_z are sample indices in the grid builder,
    // which is the only builder painting is shown by.
    if(!m_paint.empty() && !m_paint_layers.empty()
       && tile_x >= 0 && tile_x < samples() && tile_z >= 0 && tile_z < samples())
    {
        const size_t base = paint_index(tile_x, tile_z);

        for(size_t layer = 0; layer < m_paint_layers.size(); ++layer)
        {
            // A layer with a picture is mixed in by the shader, from the same
            // weights. Mixing its flat colour here as well would show through
            // its picture and tint it. A map wants the opposite -- it is made
            // of colours and has no pictures to show through -- which is what
            // skip_textured is for.
            if(skip_textured && layer < MOST_TEXTURED_LAYERS && m_layer_textures[layer])
            {
                continue;
            }

            const float weight = static_cast<float>(m_paint[base + layer]) / 255.0f;

            if(weight > 0.0f)
            {
                color = glm::mix(color, m_paint_layers[layer].color, weight);
            }
        }
    }

    return color;
}

glm::vec3 terrain_3d::surface_color(int tile_x, int tile_z, float local_y,
                                    const glm::vec3& normal) const
{
    // A picture of ground is a colour of ground. Tinting it with the
    // checkerboard as well would show the checkerboard through it, which is
    // the one thing a picture is chosen to be rid of.
    const glm::vec3 under = m_base_texture ? glm::vec3(1.0f)
                                           : ground_tone(tile_x, tile_z, local_y, normal);

    return painted_tone(tile_x, tile_z, under, true);
}

std::vector<uint8_t> terrain_3d::overhead_image(int side) const
{
    side = std::clamp(side, 16, 1024);

    std::vector<uint8_t> pixels(static_cast<size_t>(side) * side * 4, 0);

    const int n = samples();
    const float half = m_size * 0.5f;
    const float step = m_size / static_cast<float>(m_resolution);

    for(int y = 0; y < side; ++y)
    {
        for(int x = 0; x < side; ++x)
        {
            // Nearest sample rather than a filtered one: the map is small,
            // the samples are many, and a road one sample wide should stay a
            // road rather than be averaged away.
            const int sx = std::clamp(x * n / side, 0, n - 1);
            const int sz = std::clamp(y * n / side, 0, n - 1);

            const float px = static_cast<float>(sx) * step - half;
            const float pz = static_cast<float>(sz) * step - half;

            const float height = local_height(px, pz);
            const glm::vec3 normal = local_normal(px, pz);

            // Always the ground's own colours, never white: a base picture
            // leaves the mesh white because the shader puts the picture back,
            // and there is no shader here to put anything back.
            glm::vec3 colour = ground_tone(sx, sz, height, normal);

            // And every paint layer, pictures included. That is the whole
            // point of a map: a road is a road whether it is drawn as a
            // colour or as gravel.
            colour = painted_tone(sx, sz, colour, false);

            // A little relief, from how the ground faces. Without it a map of
            // a hilly place is a flat wash the same colour throughout, and
            // the one thing a map is for is telling one place from another.
            const float lit = std::clamp(0.62f + 0.38f * normal.y, 0.0f, 1.0f);

            colour *= lit;

            const size_t at = (static_cast<size_t>(y) * side + x) * 4;

            pixels[at + 0] = static_cast<uint8_t>(std::clamp(colour.r, 0.0f, 1.0f) * 255.0f);
            pixels[at + 1] = static_cast<uint8_t>(std::clamp(colour.g, 0.0f, 1.0f) * 255.0f);
            pixels[at + 2] = static_cast<uint8_t>(std::clamp(colour.b, 0.0f, 1.0f) * 255.0f);
            pixels[at + 3] = 255;
        }
    }

    return pixels;
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

ref<class mesh_3d> terrain_3d::build_grid_mesh() const
{
    const int n = samples();
    const float half = m_size * 0.5f;
    const float step = m_size / static_cast<float>(m_resolution);

    std::vector<vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(static_cast<size_t>(n) * static_cast<size_t>(n));
    indices.reserve(static_cast<size_t>(m_resolution) * static_cast<size_t>(m_resolution) * 6);

    for(int z = 0; z < n; ++z)
    {
        for(int x = 0; x < n; ++x)
        {
            const float px = static_cast<float>(x) * step - half;
            const float pz = static_cast<float>(z) * step - half;

            const float y = local_height(px, pz);
            const glm::vec3 normal = local_normal(px, pz);

            vertices.push_back({ { px, y, pz },
                                 normal,
                                 surface_color(x, z, y, normal),
                                 { static_cast<float>(x), static_cast<float>(z) } });
        }
    }

    for(int z = 0; z < m_resolution; ++z)
    {
        for(int x = 0; x < m_resolution; ++x)
        {
            const auto row = static_cast<uint32_t>(z) * static_cast<uint32_t>(n);
            const auto next = row + static_cast<uint32_t>(n);
            const auto column = static_cast<uint32_t>(x);

            const uint32_t bottom_left = row + column;
            const uint32_t bottom_right = row + column + 1;
            const uint32_t top_left = next + column;
            const uint32_t top_right = next + column + 1;

            // counter-clockwise seen from above, so the face normal is +Y
            indices.insert(indices.end(), { top_left, top_right, bottom_right,
                                            top_left, bottom_right, bottom_left });
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

void terrain_3d::refresh_region(int min_x, int max_x, int min_z, int max_z)
{
    // Anything that redraws the ground may have repainted it, and the splat
    // map is that painting arranged for a sampler. Marked rather than rebuilt:
    // a brush stroke is a drag of the mouse and touches the same ground many
    // times a second, and uploading a texture nobody has seen yet each time is
    // work for its own sake. The next draw pays for it, once.
    m_splat_stale = true;

    if(!m_mesh || !m_mesh_is_grid)
    {
        rebuild();
        return;
    }

    const int n = samples();

    // A vertex's normal comes from the heights on either side of it, so the
    // ring just outside the stroke has moved too even though its height has
    // not.
    min_x = std::max(0, min_x - 1);
    min_z = std::max(0, min_z - 1);
    max_x = std::min(n - 1, max_x + 1);
    max_z = std::min(n - 1, max_z + 1);

    if(min_x > max_x || min_z > max_z)
    {
        return;
    }

    const float half = m_size * 0.5f;
    const float step = m_size / static_cast<float>(m_resolution);
    const auto span = static_cast<size_t>(max_x - min_x + 1);

    std::vector<vertex> row;
    row.reserve(span);

    // A row of the grid is contiguous in the buffer, so each one goes up as a
    // single write rather than a vertex at a time.
    for(int z = min_z; z <= max_z; ++z)
    {
        row.clear();

        for(int x = min_x; x <= max_x; ++x)
        {
            const float px = static_cast<float>(x) * step - half;
            const float pz = static_cast<float>(z) * step - half;

            const float y = local_height(px, pz);
            const glm::vec3 normal = local_normal(px, pz);

            row.push_back({ { px, y, pz },
                            normal,
                            surface_color(x, z, y, normal),
                            { static_cast<float>(x), static_cast<float>(z) } });
        }

        const auto first = static_cast<size_t>(z) * static_cast<size_t>(n)
                         + static_cast<size_t>(min_x);

        if(!m_mesh->update_vertices(first, row.data(), row.size()))
        {
            // The mesh is not the shape we thought it was, so start again.
            rebuild();
            return;
        }
    }
}

void terrain_3d::rebuild()
{
    // Anything that redraws the ground may have repainted it, and the splat
    // map is that painting arranged for a sampler. Marked rather than rebuilt:
    // a brush stroke is a drag of the mouse and touches the same ground many
    // times a second, and uploading a texture nobody has seen yet each time is
    // work for its own sake. The next draw pays for it, once.
    m_splat_stale = true;

    // The checkerboard is the only thing that needs a vertex per tile corner,
    // since its colour changes across an edge. Everything else -- which is to
    // say anything anyone would ship -- shares them, and rebuilds an order of
    // magnitude faster for it.
    m_mesh_is_grid = !(m_layers.empty() && m_heights.empty() && m_paint.empty());
    m_mesh = m_mesh_is_grid ? build_grid_mesh() : build_mesh();
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

    if(!has_surface())
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
