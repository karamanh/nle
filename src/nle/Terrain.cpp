#include "Terrain.h"
#include "Globals.h"

#include "vendor/FastNoiseLite.h"
#include "stb_image.h"
#include <glm/gtc/random.hpp>

// static void map_value_to_color(float value, float min_value, float max_value, float& red, float& green, float& blue) {
//     if (value < min_value) value = min_value;
//     if (value > max_value) value = max_value;

//     // Calculate the normalized position of the value within the green range (80%)
//     float t = (value - min_value) / (0.8f * (max_value - min_value));

//     // Map the value to an RGB color within the specified range
//     red = 0.6f + 0.4f * t;  // Red component (from 0.6 to 1.0)
//     green = 0.8f;           // Green component (fixed at 0.8 for green)
//     blue = 0.2f;    
// }

static void map_value_to_color(float value, float min_value, float max_value, float& red, float& green, float& blue) {
    if (value < min_value) value = min_value;
    if (value > max_value) value = max_value;

    float range = max_value - min_value;
    float value_normalized = value - min_value;

    if(value_normalized <= range * 0.1)
    {
        red = 0.290f; green = 0.290f; blue = 0.290f;
    }
    else if(value_normalized <= range * 0.9)
    {
        red = 0.0f; green = 0.478f; blue = 0.105f;
    }
    else
    {
        red = 0.478f; green = 0.352f; blue = 0.0f;
    }

    // // Calculate the normalized position of the value within the green range (80%)
    // float t = (value - min_value) / (0.8f * (max_value - min_value));

    // // Map the value to an RGB color within the specified range
    // red = 0.6f + 0.4f * t;  // Red component (from 0.6 to 1.0)
    // green = 0.8f;           // Green component (fixed at 0.8 for green)
    // blue = 0.2f;    
}

namespace nle
{

   Terrain::Terrain(int width, int height, float height_multiplier)
        : m_width(width), m_height(height), m_height_multiplier(height_multiplier)
    {
        m_heightmap.resize(width * height);

        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        int index = 0;
        for (int y = 0; y < m_height; y++)
        {
            for (int x = 0; x < m_width; x++)
            {
                m_heightmap[index++] = noise.GetNoise((float)x, (float)y) * m_height_multiplier;
            }
        }
        generate_terrain_mesh();
    }

    Terrain::Terrain(int width, int height, const std::vector<float> &heightmap, float height_multiplier)
        : m_width(width), m_height(height), m_height_multiplier(height_multiplier)
    {
        generate_terrain_mesh();
    }

    Terrain::Terrain(const std::string &image_path, float height_multiplier)
        : m_height_multiplier(height_multiplier)
    {
        int bit_depth;
        unsigned char *data = stbi_load(image_path.c_str(), &m_width, &m_height, &bit_depth, STBI_rgb_alpha);
        if (!data)
        {
            prerr("Terrain(): nothing here %s", image_path.c_str());
            return;
        }

        for(int i = 0; i < m_width * m_height; i++)
        {
            m_heightmap.push_back(static_cast<float>(data[i*4]) / 255 * m_height_multiplier); 
        }

        stbi_image_free(data);
        generate_terrain_mesh();
    }

    Terrain::~Terrain()
    {
        delete m_mesh->shader();
        delete m_mesh;
    }

    TerrainInstance *Terrain::create_instance()
    {
        auto * ti = new TerrainInstance(this);
        ti->set_position({-static_cast<float>(m_width)/2, 0.0f, -static_cast<float>(m_height)/2});
        return ti;
    }

    float Terrain::get_height_at(unsigned int x, unsigned int y)
    {
        return m_heightmap[y*m_width + x];
    }

    float Terrain::get_height_at(float x, float z)
    {
        int x0 = static_cast<int>(x);
        int z0 = static_cast<int>(z);

        int x1 = x0 + 1;
        int z1 = z0 + 1;

        // Ensure that the indices are within bounds
        if (x0 < 0 || x1 >= m_width || z0 < 0 || z1 >= m_height) {
            return 0.0f; // Return a default value (0.0) for out-of-bounds coordinates
        }

        // Perform bilinear interpolation
        float q11 = m_heightmap[m_width * z0 + x0]; //heightmap[z0][x0];
        float q12 = m_heightmap[m_width * z1 + x0];//heightmap[z1][x0];
        float q21 = m_heightmap[m_width * z0 + x1];//heightmap[z0][x1];
        float q22 = m_heightmap[m_width * z1 + x1];//heightmap[z1][x1];

        float fx1 = x - static_cast<float>(x0);
        float fz1 = z - static_cast<float>(z0);

        float interpolated_height = (1.0f - fx1) * ((1.0f - fz1) * q11 + fz1 * q12) + fx1 * ((1.0f - fz1) * q21 + fz1 * q22);

        return interpolated_height;
    }

    glm::vec3 Terrain::get_normal_at(float x, float z)
    {
        if (x < 0 || z < 0 || x >= m_height - 1 || z >= m_width - 1) {
            return glm::vec3(0.0f, 1.0f, 0.0f);
        }

        int i00 = static_cast<int>(z) * m_width + static_cast<int>(x);
        int i10 = static_cast<int>(z) * m_width + static_cast<int>(x+1);
        int i01 = static_cast<int>(z + 1) * m_width + static_cast<int>(x);

        float height00 = m_heightmap[i00];
        float height10 = m_heightmap[i10];
        float height01 = m_heightmap[i01];

        float dx = height10 - height00;
        float dy = height01 - height00;

        glm::vec3 normal(-dx, -dy, 1.0f);
        return glm::normalize(normal);
    }

    glm::vec3 Terrain::get_random_position() const
    {
        return glm::vec3();
    }

    void Terrain::generate_terrain(std::vector<float> &vertices, std::vector<unsigned int> &indices)
    {
        unsigned int nvertices = m_width * m_height;
        unsigned int ntriangles = (m_width - 1) * (m_height - 1) * 2;

        vertices.resize(nvertices * 11);
        indices.resize(ntriangles * 3);

        for(unsigned int i = 0; i < m_height; i++)
        {
            for(unsigned int j = 0; j < m_width; j++)
            {
                unsigned int vert_chunk_size = 11;
                unsigned int index = (i * m_width + j) * vert_chunk_size;

                float r, g, b;
                float height = get_height_at(j, i);
                map_value_to_color(height, 0.0f, m_height_multiplier, r, g, b);

                glm::vec3 normal = get_normal_at(j, i);

                vertices[index] = static_cast<float>(j);
                vertices[index+1] = height;
                vertices[index+2] = static_cast<float>(i);
                vertices[index+3] = normal.x;
                vertices[index+4] = normal.y;
                vertices[index+5] = normal.z;
                vertices[index+6] = r;
                vertices[index+7] = g;
                vertices[index+8] = b;
                vertices[index+9] = 0.0f;
                vertices[index+10] = 0.0f;
            }
        }

        int index = 0;
        for(int i = 0; i < m_height - 1; i++)
        {
            for(int j = 0; j < m_width - 1; j++)
            {
                int topl = i * m_width + j;
                int topr = topl + 1;
                int botl = (i + 1) * m_width + j;
                int botr = botl + 1;

                indices[index++] = topl;
                indices[index++] = botl;
                indices[index++] = topr;

                indices[index++] = topr;
                indices[index++] = botl;
                indices[index++] = botr;
            }
        }
    }

    void Terrain::generate_terrain_mesh()
    {
        std::vector<float> vert;
        std::vector<unsigned int> ind;

        generate_terrain(vert, ind);

        m_mesh = new Mesh(vert, ind, new Shader("shader/default_vert.glsl", "shader/default_frag.glsl", true), nullptr, new Material(*globals::DEFAULT_MATERIAL));
        m_mesh->material()->set_specular({0.0f, 0.0f, 0.0f});
    }

    TerrainInstance::TerrainInstance(Terrain * terrain)
        : MeshInstance(terrain->m_mesh), m_terrain(terrain)
    {
    }

    void TerrainInstance::add_child(Object3D *child)
    {
        Object3D::add_child(child);
        add_terrain_feature(child, 7.0f, 20.f);
    }

    void TerrainInstance::add_terrain_feature(Object3D *child, float from_height, float to_height)
    {
        glm::vec2 bounds_min = {this->position().x, this->position().z};

        glm::vec2 bounds_max = {
            bounds_min.x + static_cast<float>(m_terrain->m_width),
            bounds_min.y + static_cast<float>(m_terrain->m_height)
        };

        // auto pos = glm::circularRand(static_cast<float>(m_terrain->m_width));
        auto pos = glm::linearRand(bounds_min, bounds_max);
        auto pos_no_offset = pos - bounds_min;
        float height = m_terrain->get_height_at(pos_no_offset.x, pos_no_offset.y);
        
        if(height >= from_height && height <= to_height)
        {
            child->set_position(glm::vec3(pos.x, height, pos.y));
        }
        else
        {
            add_terrain_feature(child, from_height, to_height);
        }
    }

} // namespace nle
