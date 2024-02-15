#include "multimesh_instance_3d.h"

namespace nle
{

    multimesh_instance_3d::multimesh_instance_3d(ref<class multimesh_3d> multimesh)
    {
        m_multimesh_3d = multimesh;
    }

    multimesh_instance_3d::~multimesh_instance_3d()
    {
    }

    ref<class multimesh_3d> multimesh_instance_3d::multimesh_3d()
    {
        return m_multimesh_3d;
    }

    void multimesh_instance_3d::render()
    {
        throw std::runtime_error("multimesh_instance_3d::render not implemented");
    }

} // namespace nle
