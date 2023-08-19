#pragma once

#include "Mesh.h"
#include "Material.h"
#include "RenderObject3D.h"

namespace nle
{
    class MultiMeshInstance;

    class MultiMesh
    {
    private:
        std::vector<Mesh *> m_meshes;
        float m_bounding_sphere_radius;
        glm::vec3 m_aabb_max;
        glm::vec3 m_aabb_min;

    public:
        MultiMesh();
        ~MultiMesh();
        std::vector<Mesh *> &meshes();
        void add_mesh(Mesh *m);
        MultiMeshInstance *create_instance();
        float bounding_sphere_radius() const;
        friend class Model;
        friend class MultiMeshInstance;
    };

    class MultiMeshInstance : public RenderObject3D
    {
    private:
        glm::mat4 m_model_matrix;
        MultiMesh *m_multimesh;
        Material *m_material = nullptr;
        std::string m_source;
        void set_material_for_meshes(Material *material);
        void update_model_matrix();

    public:
        MultiMeshInstance(MultiMesh *mm);
        ~MultiMeshInstance();
        MultiMesh *multimesh() const;
        void set_material(Material *material);
        Material *material() const;
        nlohmann::json to_json() override;
        void from_json(const nlohmann::json &j) override;
        void set_position(glm::vec3 position) override;
        void set_rotation(glm::vec3 rotation) override;
        void set_scale(glm::vec3 scale) override;
        float scaled_radius() const;
        glm::vec3 aabb_max() const;
        glm::vec3 aabb_min() const;
        glm::mat4 model_matrix() const;

        friend class MultiMesh;
        friend class Model;
    };
} // namespace nle
