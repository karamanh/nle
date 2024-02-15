/**
 * @file mesh_instance_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "mesh_3d.h"
#include "../object/render_object_3d.h"

namespace nle
{

class mesh_instance_3d : public render_object_3d
{
public:
    mesh_instance_3d(ref<class mesh_3d> mesh);
    virtual ~mesh_instance_3d();

    ref<class mesh_3d> mesh(); 

    void render();
private:
    ref<class mesh_3d> m_mesh;
};

} // namespace nle
