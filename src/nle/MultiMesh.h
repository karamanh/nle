#pragma once

#include "Mesh.h"
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
    public:
        MultiMeshInstance(MultiMesh *mm);
        ~MultiMeshInstance();
        MultiMesh *multimesh() const;
        void set_material_for_meshes(Material *material);
    };
} // namespace nle
