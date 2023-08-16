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
        std::vector<Mesh*> m_meshes;
    public:
        MultiMesh();
        ~MultiMesh();
        std::vector<Mesh*>& meshes();
        MultiMeshInstance *create_instance();
    };

    class MultiMeshInstance : public RenderObject3D
    {
    private:
        MultiMesh *m_multimesh;
        Material *m_material = nullptr;
        std::string m_source;
        void set_material_for_meshes(Material *material);
    public:
        MultiMeshInstance(MultiMesh *mm);
        ~MultiMeshInstance();
        MultiMesh *multimesh() const;
        void set_material(Material *material);
        Material *material() const;
        nlohmann::json to_json() override;
        void from_json(const nlohmann::json& j) override;

        friend class MultiMesh;
        friend class Model;
    };
} // namespace nle
