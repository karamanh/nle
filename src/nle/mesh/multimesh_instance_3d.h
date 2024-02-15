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
#include "../object/render_object_3d.h"

namespace nle
{

class multimesh_instance_3d : public render_object_3d
{
public:
    multimesh_instance_3d(ref<class multimesh_3d> multimesh);
    virtual ~multimesh_instance_3d();
    
    ref<class multimesh_3d> multimesh_3d();

    void render();

private:
    ref<class multimesh_3d> m_multimesh_3d;
};

} // namespace nle
