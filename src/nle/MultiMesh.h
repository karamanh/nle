#pragma once

#include "Mesh.h"
#include "Material.h"
#include "RenderObject3D.h"
#include "AABB.h"

namespace nle
{
    class MultiMeshInstance;

    class MultiMesh
    {
    private:
        std::vector<Mesh *> m_meshes;
        AABB m_aabb;

    public:
        MultiMesh();
        ~MultiMesh();
        std::vector<Mesh *> &meshes();
        void add_mesh(Mesh *m);
        MultiMeshInstance *create_instance();
        float bounding_sphere_radius() const;
        AABB aabb() const;
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
        AABB m_aabb;
        void set_material_for_meshes(Material *material);
        void update() override;

    public:
        MultiMeshInstance(MultiMesh *mm);
        ~MultiMeshInstance();
        MultiMesh *multimesh() const;
        void set_material(Material *material);
        Material *material() const;
        nlohmann::json to_json() override;
        void from_json(const nlohmann::json &j) override;
        AABB aabb() const;
        glm::mat4 model_matrix() const;

        friend class MultiMesh;
        friend class Model;
    };
} // namespace nle
