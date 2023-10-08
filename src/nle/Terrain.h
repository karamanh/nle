#pragma once

#include "Mesh.h"

namespace nle
{
    class TerrainInstance;
    class TerrainFeatureLayer;

    class Terrain
    {
    private:
        Mesh *m_mesh;

        int m_width;
        int m_height;
        float m_height_multiplier = 1.0f;

        std::vector<float> m_heightmap;

        void generate_terrain(std::vector<float>& vertices, std::vector<unsigned int>& indices);
        void generate_terrain_mesh();

    public:
        Terrain(int width, int height, float height_multiplier = 10.0f);
        Terrain(int width, int height, const std::vector<float>& heightmap, float height_multiplier = 1.0f);
        Terrain(const std::string& image_path, float height_multiplier = 1.0f);
        ~Terrain();
        TerrainInstance *create_instance();
        float get_height_at(unsigned int x, unsigned int y);
        float get_height_at(float x, float z);
        glm::vec3 get_normal_at(float x, float z);

        glm::vec3 get_random_position() const;

        friend class TerrainInstance;
    };

    class TerrainInstance : public MeshInstance
    {
    private:
        Terrain *m_terrain;
    public:
        TerrainInstance(Terrain * terrain);
        void add_child(Object3D *child) override;
        void add_terrain_feature(Object3D *feature, float from_height = 0.0f, float to_height = 10.f);

        nlohmann::json to_json() override;
        void from_json(const nlohmann::json& j) override;
    };

    // class TerrainFeatureLayers
    // {
    // private:
    //     typedef struct {
    //         Mesh mesh;
    //         std::vector<Object3D*> instances;
    //     } feature_t;

    //     Terrain *m_terrain;
    //     std::vector<Mesh> m_features;
    // public:
    //     TerrainFeatureLayers(Terrain *terrain);
    // };
} // namespace nle
