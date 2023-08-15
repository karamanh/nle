#include "MultiMesh.h"

namespace nle
{

MultiMesh::MultiMesh()
{
}

MultiMesh::~MultiMesh()
{
    for(auto * i : m_meshes)
        delete i;
}

std::vector<Mesh*> &MultiMesh::meshes()
{
    return m_meshes;
}

MultiMeshInstance *MultiMesh::create_instance()
{
    return new MultiMeshInstance(this);
}

MultiMeshInstance::MultiMeshInstance(MultiMesh *mm)
    : m_multimesh(mm)
{
    for(auto* c : m_multimesh->meshes())
    {
        add_child(c->create_instance());
    }
}

MultiMeshInstance::~MultiMeshInstance()
{
}

MultiMesh *MultiMeshInstance::multimesh() const
{
    return m_multimesh;
}

void MultiMeshInstance::set_material_for_meshes(Material *material)
{
    for(auto* c : m_multimesh->meshes())
    {
        c->set_material(material);
    }
}

} // namespace nle
