/**
 * @file multimesh_instance_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "multimesh_3d.h"
#include "mesh_instance_3d.h"
#include "../object/render_object_3d.h"

namespace nle
{

class multimesh_instance_3d : public render_object_3d
{
public:
    multimesh_instance_3d(ref<multimesh_3d> multimesh);
    virtual ~multimesh_instance_3d();
    
    ref<class multimesh_3d> multimesh();

    void render();

private:
    ref<class multimesh_3d> m_multimesh;

    void update() override;
};

} // namespace nle
