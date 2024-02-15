#include "mesh_instance_3d.h"

namespace nle
{
    mesh_instance_3d::mesh_instance_3d(ref<class mesh_3d> mesh)
    {
        m_mesh = mesh;
    }

    mesh_instance_3d::~mesh_instance_3d()
    {
    }

    ref<class mesh_3d> mesh_instance_3d::mesh()
    {
        return m_mesh;
    }

    void mesh_instance_3d::render()
    {
        throw std::runtime_error("mesh_instance_3d::render not implemented");
    }
} // namespace nle
