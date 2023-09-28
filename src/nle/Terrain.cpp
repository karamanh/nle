#include "Terrain.h"
#include "Globals.h"

#include "vendor/FastNoiseLite.h"

static void map_value_to_color(float value, float min_value, float max_value, float& red, float& green, float& blue) {
    if (value < min_value) value = min_value;
    if (value > max_value) value = max_value;

    // Calculate the normalized position of the value within the green range (80%)
    float t = (value - min_value) / (0.8f * (max_value - min_value));

    // Map the value to an RGB color within the specified range
    red = 0.6f + 0.4f * t;  // Red component (from 0.6 to 1.0)
    green = 0.8f;           // Green component (fixed at 0.8 for green)
    blue = 0.2f;    
}

namespace nle
{

   Terrain::Terrain(unsigned int width, unsigned int height)
        : m_width(width), m_height(height)
    {
        m_heightmap.resize(width * height);

        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        int index = 0;
        for (int y = 0; y < m_height; y++)
        {
            for (int x = 0; x < m_width; x++)
            {
                m_heightmap[index++] = noise.GetNoise((float)x, (float)y);
            }
        }

        std::vector<float> vert;
        std::vector<unsigned int> ind;
        generate_terrain(vert, ind);
        m_mesh = new Mesh(vert, ind, globals::DEFAULT_SHADER, nullptr, new Material(*globals::DEFAULT_MATERIAL));
        m_mesh->material()->set_specular({0.0f, 0.0f, 0.0f});
    }

    Terrain::~Terrain()
    {
        delete m_mesh->material();
    }

    TerrainInstance *Terrain::create_instance()
    {
        // return m_mesh->create_instance();
        return new TerrainInstance(this);
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
                map_value_to_color(height, 0.0f, 1.0f, r, g, b);

                // Calculate the normal vector for this vertex
                glm::vec3 normal(0.0f, 1.0f, 0.0f); // Initialize with an "up" direction

                if (j > 0 && j < m_width - 1 && i > 0 && i < m_height - 1) {
                    // Calculate the normal based on neighboring vertices
                    glm::vec3 v1(vertices[index - vert_chunk_size], vertices[index - (vert_chunk_size -1)], vertices[index - (vert_chunk_size - 2)]);
                    glm::vec3 v2(vertices[index + vert_chunk_size], vertices[index + (vert_chunk_size + 1)], vertices[index + (vert_chunk_size + 2)]);
                    glm::vec3 v3(vertices[index - m_width * vert_chunk_size], vertices[index - m_width * vert_chunk_size + 1], vertices[index - m_width * vert_chunk_size + 2]);
                    glm::vec3 v4(vertices[index + m_width * vert_chunk_size], vertices[index + m_width * vert_chunk_size + 1], vertices[index + m_width * vert_chunk_size + 2]);

                    // Calculate the cross product of two triangle edges to find the normal
                    glm::vec3 normal1 = glm::cross(v2 - v1, v3 - v1);
                    glm::vec3 normal2 = glm::cross(v4 - v1, v2 - v1);

                    // Average the two normals to get the vertex normal
                    normal = glm::normalize((normal1 + normal2) / 2.0f);
                }

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

    TerrainInstance::TerrainInstance(Terrain * terrain)
        : MeshInstance(terrain->m_mesh), m_terrain(terrain)
    {
    }

    void TerrainInstance::add_child(Object3D *child)
    {
        Object3D::add_child(child);
        auto cpos = child->position();
        child->set_position(this->position() + glm::vec3(cpos.x, this->position().y + m_terrain->get_height_at(cpos.x, cpos.z), cpos.z));
    }

} // namespace nle
