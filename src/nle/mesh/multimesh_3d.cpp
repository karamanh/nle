#include "multimesh_3d.h"

namespace nle
{
    multimesh_3d::multimesh_3d()
    {
    }

    multimesh_3d::~multimesh_3d()
    {
    }

    std::vector<ref<mesh_3d>> &multimesh_3d::meshes()
    {
        return m_meshes;
    }
} // namespace nle
