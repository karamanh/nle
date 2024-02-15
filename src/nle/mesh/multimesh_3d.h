/**
 * @file multimesh_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "mesh_3d.h"

namespace nle
{

class multimesh_3d
{
public:
    multimesh_3d();
    virtual ~multimesh_3d();

    std::vector<ref<mesh_3d>>& meshes();
private:
    std::vector<ref<mesh_3d>> m_meshes;
};

} // namespace nle
