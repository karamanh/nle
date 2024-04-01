#include "multimesh_instance_3d.h"

namespace nle
{

    multimesh_instance_3d::multimesh_instance_3d(ref<class multimesh_3d> multimesh)
    {
        m_multimesh = multimesh;

        for(auto m3d : m_multimesh->meshes())
        {
            add_child(make_ref<mesh_instance_3d>(m3d));
        }

        update();
    }

    multimesh_instance_3d::~multimesh_instance_3d()
    {
    }

    ref<class multimesh_3d> multimesh_instance_3d::multimesh()
    {
        return m_multimesh;
    }

    void multimesh_instance_3d::render()
    {
        for(auto ro : render_objects())
        {
            ro->render();
        }
    }

    void multimesh_instance_3d::update()
    {
        object_3d::update();
        /// TODO: update model matrix
    }

} // namespace nle
