#pragma once

#include "Mesh.h"

namespace nle
{
    class TerrainInstance;

    class Terrain
    {
    private:
        Mesh *m_mesh;

        unsigned int m_width;
        unsigned int m_height;

        std::vector<float> m_heightmap;

        void generate_terrain(std::vector<float>& vertices, std::vector<unsigned int>& indices);

    public:
        Terrain(unsigned int width, unsigned int height);
        ~Terrain();
        TerrainInstance *create_instance();
        float get_height_at(unsigned int x, unsigned int y);
        float get_height_at(float x, float z);

        friend class TerrainInstance;
    };

    class TerrainInstance : public MeshInstance
    {
    private:
        Terrain *m_terrain;
    public:
        TerrainInstance(Terrain * terrain);
        void add_child(Object3D *child) override;
    };
} // namespace nle
