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
        void set_material_for_meshes(Material *material);
    public:
        MultiMeshInstance(MultiMesh *mm);
        ~MultiMeshInstance();
        MultiMesh *multimesh() const;
        void set_material(Material *material);
        Material *material() const;
    };
} // namespace nle
